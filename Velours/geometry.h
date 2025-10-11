/*
	math.h - different math utilities like VlRect and VlRGBA
*/
#ifndef VELOURS_GEOMETRY_H
#define VELOURS_GEOMETRY_H

#define _USE_MATH_DEFINES
#include <math.h>

#define VL_MATH_NO_RESULT -65536

#define VL_MIN(A, B) (((A) < (B)) ? (A) : (B))
#define VL_MAX(A, B) (((A) < (B)) ? (B) : (A))

typedef struct {
	float x1, y1;
	float x2, y2;
} VlRect;

typedef struct {
	float x, y;
} VlVec2;

typedef struct {
	float r, g, b, a;
} VlRGBA;

#define VL_RECT(X1, Y1, X2, Y2) ((VlRect) {.x1 = (float) (X1), .y1 = (float) (Y1), .x2 = (float) (X2), .y2 = (float) (Y2)})
#define VL_VEC2(X, Y) ((VlVec2) {.x = (float) (X), .y = (float) (Y)})
#define VL_RGBA(R, G, B, A) ((VlRGBA) {.r = (float) (R), .g = (float) (G), .b = (float) (B), .a = (float) (A)})

#define VL_ABS(X) (((X) < 0) ? -(X) : (X))
#define VL_CLAMP(X, A, B) VL_MAX(A, VL_MIN(X, B))

VlVec2 vl_math_line_line_intersection(VlVec2 p1, VlVec2 p2, VlVec2 p3, VlVec2 p4);

#endif // VELOURS_GEOMETRY_H