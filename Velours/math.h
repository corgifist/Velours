/*
	math.h - different math utilities like VlRect and VlRGBA
*/
#ifndef VELOURS_MATH_H
#define VELOURS_MATH_H

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

int abs(int x);
double fmod(double f, double x);
double floor(double x);
float floorf(float f);
double sin(double a);
double cos(double a);
double tan(double a);
double sqrt(double x);
float fmodf(float f, float x);

#define VL_PI 3.14159265358979323846

#define VL_RECT(X1, Y1, X2, Y2) ((VlRect) {.x1 = (float) (X1), .y1 = (float) (Y1), .x2 = (float) (X2), .y2 = (float) (Y2)})
#define VL_VEC2(X, Y) ((VlVec2) {.x = (float) (X), .y = (float) (Y)})
#define VL_RGBA(R, G, B, A) ((VlRGBA) {.r = (float) (R), .g = (float) (G), .b = (float) (B), .a = (float) (A)})

#define VL_ABS(X) (((X) < 0) ? -(X) : (X))
#define VL_CLAMP(X, A, B) VL_MAX(A, VL_MIN(X, B))

VlVec2 vl_math_line_line_intersection(VlVec2 p1, VlVec2 p2, VlVec2 p3, VlVec2 p4);

#endif // VELOURS_MATH_H