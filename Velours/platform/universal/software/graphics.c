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

static VlRGBA vl_software_graphics_brush_sample_solid(VlGraphicsBrush brush, float x, float y) {
	VL_UNUSED(x);
	VL_UNUSED(y);
	return ((VlSoftwareBrushSolid*) brush)->color;
}

VlGraphicsBrush vl_software_graphics_brush_solid_new(VlGraphics graphics, VlRGBA rgba) {
	VlSoftwareBrushSolid *brush = VL_MALLOC(sizeof(VlSoftwareBrushSolid));
	if (!brush) return NULL;

	brush->base.base.type = VL_GRAPHICS_BRUSH_SOLID;
	brush->base.base.owner = graphics;
	brush->base.sample = vl_software_graphics_brush_sample_solid;
	brush->color = rgba;

	return (VlGraphicsBrush) brush;
}

VlResult vl_software_graphics_brush_free(VlGraphicsBrush brush) {
	if (!brush) return VL_ERROR;

	VL_FREE(brush);
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
	/* for (int y = software->y1; y < software->y2; y++) {
		for (int x = software->x1; x < software->x2; x++) {
			size_t __pixel_index = ((y) * software->front->row) + (x) * software->front->pixel_size;
			software->front->data[__pixel_index] = (u8)(color.r * 255);
			if (software->front->channels >= 2) software->front->data[__pixel_index + 1] = (u8) (color.g * 255);
			if (software->front->channels >= 3) software->front->data[__pixel_index + 2] = (u8) (color.b * 255);
			if (software->front->channels >= 4) software->front->data[__pixel_index + 3] = (u8) (color.a * 255);
		}
	} */
	union {
		u32 rgba;
		struct {
			unsigned char r, g, b, a;
		} split;
	} compound = { 0 };
	compound.split.r = (unsigned char) (color.r * 255);
	compound.split.g = (unsigned char) (color.g * 255);
	compound.split.b = (unsigned char) (color.b * 255);
	compound.split.a = (unsigned char) (color.a * 255);
	memset(software->front->data, compound.rgba, software->w * software->h * software->front->pixel_size);
	return VL_SUCCESS;
}

static inline void draw_brush(VlSoftwarePixelBuffer *front, int cx, int cy, int radius, VlRGBA color) {
	if (radius <= 1) {
		SET_PIXEL(front, cx, cy, color);
		return;
	}
	int r2 = radius * radius;
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			if (dx * dx + dy * dy <= r2) {          // inside circle
				SET_PIXEL(front, cx + dx, cy + dy, color);
			}
		}
	}
}

static inline unsigned char clamp255(int x) {
	return (unsigned char) VL_CLAMP(x, 0, 255);
}

static inline char blend_pixel(VlSoftwarePixelBuffer *front, int x, int y, VlRGBA color, double wb) {
	if (x < 0 || x >= front->w || y < 0 || y >= front->h) return 1;

	size_t index = front->row * y + x * front->pixel_size;
	unsigned char *dst = front->data + index;

	double a = wb * color.a;
	double inv = 1.0 - a;

	VlRGBA res = VL_RGBA(
		(a * color.r + inv * (dst[0] / 255.0f)),
		(a * color.g + inv * (dst[1] / 255.0f)),
		(a * color.b + inv * (dst[2] / 255.0f)),
		(a * color.a + inv * (dst[3] / 255.0f))
	);

	SET_PIXEL(front, x, y, res);

	return 0;
}

static inline void draw_soft_brush(VlSoftwarePixelBuffer* front, int cx, int cy, int radius,
	VlRGBA color, double intensity) {
	if (radius <= 1) {
		blend_pixel(front, cx, cy, color, intensity);
		return;
	}
	int r2 = radius * radius;
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			int dist2 = dx * dx + dy * dy;
			if (dist2 > r2) continue;                 // outside circle

			/* quadratic fall‑off: (1 - d/r)^2  */
			double d = sqrt((double)dist2);
			double w = 1.0 - d / radius;
			w = w * intensity;                         // smooth edge
			if (w <= 0.0) continue;

			if (blend_pixel(front, cx + dx, cy + dy, color, w)) break;
		}
	}
}

static VlResult vl_software_graphics_bresenham_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlGraphicsBrush brush, int thickness) {
	VL_UNUSED(thickness);
	if (!graphics || !brush) return VL_ERROR;
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	VlSoftwareBrush *sw_brush = (VlSoftwareBrush*) brush;
	if (!software->front) return VL_ERROR;
	int x1 = (int)p2.x;
	int x0 = (int)p1.x;
	int y1 = (int)p2.y;
	int y0 = (int)p1.y;
	if (!(x0 >= software->x1 && x0 <= software->x2) && !(y0 >= software->y1 && y0 <= software->y2) &&
		!(x1 >= software->x1 && x1 <= software->x2) && !(y1 >= software->y1 && y1 <= software->y2)) return VL_SUCCESS;
	VlRGBA sample = sw_brush->sample(brush, 0, 0);
	int radius = thickness;
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

	if (steep) {
		for (int x = x0; x <= x1; ++x) {
			sample = sw_brush->sample(brush, 0, 0);
			draw_brush(software->front, y, x, radius, sample);

			err -= dy;
			if (err < 0) {
				y += ystep;
				err += dx;
			}
		}
	} else {
		for (int x = x0; x <= x1; ++x) {
			sample = sw_brush->sample(brush, 0, 0);
			draw_brush(software->front, x, y, radius, sample);

			err -= dy;
			if (err < 0) {
				y += ystep;
				err += dx;
			}
		}
	}
	return VL_SUCCESS;
}

static VlResult vl_software_graphics_wu_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlGraphicsBrush brush, int thickness) {
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	VlSoftwareBrush *sw_brush = (VlSoftwareBrush*) brush;
	if (!software->front) return VL_ERROR;
	VlRGBA sample = sw_brush->sample(brush, 0, 0);
	int x1 = (int)p2.x;
	int x0 = (int)p1.x;
	int y1 = (int)p2.y;
	int y0 = (int)p1.y;
	if (!(x0 >= software->x1 && x0 <= software->x2) && !(y0 >= software->y1 && y0 <= software->y2) &&
		!(x1 >= software->x1 && x1 <= software->x2) && !(y1 >= software->y1 && y1 <= software->y2)) return VL_SUCCESS;
	int radius = thickness / 2;          // floor division

	char steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		/* swap x ↔ y */
		int tmp;
		tmp = x0; x0 = y0; y0 = tmp;
		tmp = x1; x1 = y1; y1 = tmp;
	}
	if (x0 > x1) {
		/* swap start ↔ end */
		int tmp;
		tmp = x0; x0 = x1; x1 = tmp;
		tmp = y0; y0 = y1; y1 = tmp;
	}

	float dx = (float)(x1 - x0);
	float dy = (float)(y1 - y0);
	float gradient = dx == 0.0f ? 1.0f : dy / dx;

	/* ---- first endpoint ---- */
	int xend = x0;
	float yend = y0 + gradient * (xend - x0);
	int xpxl1 = xend;
	int ypxl1 = (int)floorf(yend);
	float xgap = 1.0f - fmodf((float)x0 + 0.5f, 1.0f);
	float alpha1 = (1.0f - fmodf(yend, 1.0f)) * xgap;
	float alpha2 = fmodf(yend, 1.0f) * xgap;

	if (steep) {
		draw_soft_brush(software->front, ypxl1, xpxl1, radius, sample, alpha1);
		draw_soft_brush(software->front, ypxl1 + 1, xpxl1, radius, sample, alpha2);
	}
	else {
		draw_soft_brush(software->front, xpxl1, ypxl1, radius, sample, alpha1);
		draw_soft_brush(software->front, xpxl1, ypxl1 + 1, radius, sample, alpha2);
	}

	float intery = yend + gradient;   // first y-intersection for the main loop

	/* ---- second endpoint ---- */
	xend = x1;
	yend = y1 + gradient * (xend - x1);
	int xpxl2 = xend;
	int ypxl2 = (int)floorf(yend);
	xgap = fmodf((float)x1 + 0.5f, 1.0f);
	alpha1 = (1.0f - fmodf(yend, 1.0f)) * xgap;
	alpha2 = fmodf(yend, 1.0f) * xgap;

	sample = sw_brush->sample(brush, 1, 0);

	if (steep) {
		draw_soft_brush(software->front, ypxl2, xpxl2, radius, sample, alpha1);
		draw_soft_brush(software->front, ypxl2 + 1, xpxl2, radius, sample, alpha2);
	}
	else {
		draw_soft_brush(software->front, xpxl2, ypxl2, radius, sample, alpha1);
		draw_soft_brush(software->front, xpxl2, ypxl2 + 1, radius, sample, alpha2);
	}

	/* ---- main loop ---- */
	if (steep) {
		for (int x = xpxl1 + 1; x < xpxl2; ++x) {
			int y = (int)intery;
			float f = intery - y;               /* fractional part */
			sample = sw_brush->sample(brush, 0, 0);
			draw_soft_brush(software->front, y, x, radius, sample, 1.0f - f);
			draw_soft_brush(software->front, y + 1, x, radius, sample, f);
			intery += gradient;
		}
	}
	else {
		for (int x = xpxl1 + 1; x < xpxl2; ++x) {
			int y = (int)intery;
			float f = intery - y;
			sample = sw_brush->sample(brush, 0, 0);
			draw_soft_brush(software->front, x, y, radius, sample, 1.0f - f);
			draw_soft_brush(software->front, x, y + 1, radius, sample, f);
			intery += gradient;
		}
	}

	return VL_SUCCESS;
}



VlResult vl_software_graphics_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlGraphicsBrush brush, int thickness) {
	return
		graphics->antialias == VL_GRAPHICS_ANTIALIASING_ON
		? vl_software_graphics_wu_line(graphics, p1, p2, brush, thickness)
	    : vl_software_graphics_bresenham_line(graphics, p1, p2, brush, thickness);
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