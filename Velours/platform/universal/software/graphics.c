#include "graphics.h"
#include "memory.h"

// VlSoftwarePixelBuffer *vl_software_graphics_pixel_buffer_new(int w, int h, char channels, char depth)
// allocates pixel buffer with specified parameters
// arguments:
//     w - selected width for new pixel buffer
//     h - selected height for new pixel buffer
//     channels - amount of channels per color (4 for RGBA, 3 for RGB and so on)
//     depth - amount of bytes of color channel occupies (for example 1 for RGBA8, 2 for RGBA16 and so on)
static VlSoftwarePixelBuffer *vl_software_graphics_pixel_buffer_new(int w, int h, char channels, char depth) {
	VlSoftwarePixelBuffer* result = VL_MALLOC(sizeof(VlSoftwarePixelBuffer));
	if (!result) return NULL;
	result->w = w;
	result->h = h;
	result->channels = channels;
	result->depth = depth;
	result->pixel_size = channels * depth;
	result->row = w * result->pixel_size;
	result->data = VL_MALLOC(w * h * channels * depth);
	if (!result->data) {
		VL_FREE(result);
		return NULL;
	}
	return result;
}

VlResult vl_software_graphics_initialize(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}

VlResult vl_software_graphics_set_antialiasing_mode(VlGraphics graphics, VlGraphicsAntialiasingMode mode) {
	graphics->antialias = mode;
	return VL_SUCCESS;
}

VlResult vl_software_graphics_begin(VlGraphics graphics) {
	VL_UNUSED(graphics);

	// not used by this graphics backend

	return VL_SUCCESS;
}

#define SET_PIXEL(FRONT, X, Y, COLOR) \
	do { \
		if (X >= FRONT->w || Y >= FRONT->h || X < 0 || Y < 0) break; \
		size_t __pixel_index = ((Y) * FRONT->row) + (X) * FRONT->pixel_size; \
		FRONT->data[__pixel_index] = (u8) (COLOR.r * 255); \
		if (FRONT->channels >= 2) FRONT->data[__pixel_index + 1] = (u8) (COLOR.g * 255); \
		if (FRONT->channels >= 3) FRONT->data[__pixel_index + 2] = (u8) (COLOR.b * 255); \
		if (FRONT->channels >= 4) FRONT->data[__pixel_index + 3] = (u8) (COLOR.a * 255); \
	} while (0)

VlResult vl_software_graphics_clear(VlGraphics graphics, VlRGBA color) {
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics *software = (VlSoftwareGraphics*) graphics;
	if (!software->front) return VL_ERROR;
	for (int y = software->y1; y < software->y2; y++) {
		for (int x = software->x1; x < software->x2; x++) {
			SET_PIXEL(software->front, x, y, color);
		}
	}
	return VL_SUCCESS;
}

static void draw_brush(VlSoftwarePixelBuffer *front, int cx, int cy, int radius, unsigned char *cl) {
	int r2 = radius * radius;
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			if (dx * dx + dy * dy <= r2) {          // inside circle
				int x = cx + dx;
				int y = cy + dy;
				if (x >= front->w || y >= front->h || x < 0 || y < 0) break; 
				size_t __pixel_index = ((y)*front->row) + (x)*front->pixel_size; 
				front->data[__pixel_index] = cl[0];
				if (front->channels >= 2) front->data[__pixel_index + 1] = cl[1];
				if (front->channels >= 3) front->data[__pixel_index + 2] = cl[2];
				if (front->channels >= 4) front->data[__pixel_index + 3] = cl[3];
			}
		}
	}
}

static unsigned char clamp255(int v) {
	return (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

/* Blend a source colour into a destination pixel using a weight w∈[0,1] */
static void blend_pixel(VlSoftwarePixelBuffer *front, int x, int y, unsigned char *src, double wb) {
	if (x < 0 || x >= front->w || y < 0 || y >= front->h) return;

	size_t index = front->row * y + x * front->pixel_size;
	unsigned char *dst = front->data + index;

	unsigned char srcR = src[0];
	unsigned char srcG = src[1];
	unsigned char srcB = src[2];
	unsigned char srcA = src[3];

	unsigned char dstR = dst[0];
	unsigned char dstG = dst[1];
	unsigned char dstB = dst[2];
	unsigned char dstA = dst[3];

	/* simple “over” blend using the weight as alpha */
	double a = wb * (srcA / 255.0);
	double inv = 1.0 - a;

	unsigned char outA = clamp255((int)(a * srcA + inv * dstA));
	unsigned char outR = clamp255((int)(a * srcR + inv * dstR));
	unsigned char outG = clamp255((int)(a * srcG + inv * dstG));
	unsigned char outB = clamp255((int)(a * srcB + inv * dstB));

	front->data[index] = outR;
	front->data[index + 1] = outG;
	front->data[index + 2] = outB;
	front->data[index + 3] = outA;
}

static void draw_soft_brush(VlSoftwarePixelBuffer* front, int cx, int cy, int radius,
	unsigned char *brush, double intensity) {
	int r2 = radius * radius;
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			int dist2 = dx * dx + dy * dy;
			if (dist2 > r2) continue;                 // outside circle

			/* quadratic fall‑off: (1 - d/r)^2  */
			double d = sqrt((double)dist2);
			double w = 1.0 - d / radius;
			w = w * w * intensity;                         // smooth edge
			if (w <= 0.0) continue;

			blend_pixel(front, cx + dx, cy + dy, brush, w);
		}
	}
}

VlResult vl_software_graphics_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlRGBA brush, int thickness) {
	VL_UNUSED(thickness);
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	if (!software->front) return VL_ERROR;
	int x1 = (int) p2.x;
	int x0 = (int) p1.x;
	int y1 = (int) p2.y;
	int y0 = (int) p1.y;
	if (!(x0 >= software->x1 && x0 <= software->x2) && !(y0 >= software->y1 && y0 <= software->y2) &&
		!(x1 >= software->x1 && x1 <= software->x2) && !(y1 >= software->y1 && y1 <= software->y2)) return VL_SUCCESS;
	int radius = thickness / 2;
	char steep = VL_ABS(y1 - y0) > VL_ABS(x1 - x0);
	if (steep) {
		/* swap x and y */
		int tmp;
		tmp = x0; x0 = y0; y0 = tmp;
		tmp = x1; x1 = y1; y1 = tmp;
	}
	if (x0 > x1) {
		/* ensure left‑to‑right drawing */
		int tmp;
		tmp = x0; x0 = x1; x1 = tmp;
		tmp = y0; y0 = y1; y1 = tmp;
	}

	int dx = x1 - x0;
	int dy = abs(y1 - y0);
	int err = dx / 2;
	int ystep = (y0 < y1) ? 1 : -1;
	int y = y0;

	unsigned char cl[4];
	cl[0] = (unsigned char) (brush.r * 255);
	cl[1] = (unsigned char) (brush.g * 255);
	cl[2] = (unsigned char) (brush.b * 255);
	cl[3] = (unsigned char) (brush.a * 255);

	if (radius == 0) {
		for (int x = x0; x <= x1; ++x) {
			if (steep)
				SET_PIXEL(software->front, y, x, brush);
			else
				SET_PIXEL(software->front, x, y, brush);

			err -= dy;
			if (err < 0) {
				y += ystep;
				err += dx;
			}
		}
	} else {
		for (int x = x0; x <= x1; ++x) {
			if (graphics->antialias == VL_GRAPHICS_ANTIALIASING_ON) {
				if (steep)
					draw_soft_brush(software->front, y, x, radius, cl, 1.5);
				else
					draw_soft_brush(software->front, x, y, radius, cl, 1.5);
			} else {
				if (steep)
					draw_brush(software->front, y, x, radius, cl);
				else
					draw_brush(software->front, x, y, radius, cl);
			}

			err -= dy;
			if (err < 0) {
				y += ystep;
				err += dx;
			}
		}
	}
	return VL_SUCCESS;
}

VlResult vl_software_graphics_end(VlGraphics graphics) {
	VL_UNUSED(graphics);

	// not used by this graphics backend

	return VL_SUCCESS;
}

VlResult vl_software_graphics_terminate(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}