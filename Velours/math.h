/*
	math.h - different math utilities like VlRect and VlRGBA
*/
#ifndef VELOURS_MATH_H
#define VELOURS_MATH_H

typedef struct {
	float x1, y1;
	float x2, y2;
} VlRect;

typedef struct {
	float r, g, b, a;
} VlRGBA;

#define VL_RECT(X1, Y1, X2, Y2) ((VlRect) {.x1 = (float) (X1), .y1 = (float) (Y1), .x2 = (float) (X2), .y2 = (float) (Y2)})
#define VL_RGBA(R, G, B, A) ((VlRGBA) {.r = (float) (R), .g = (float) (G), .b = (float) (B), .a = (float) (A)})

#endif // VELOURS_MATH_H