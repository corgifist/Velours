#include "geometry.h"

VlVec2 vl_math_line_line_intersection(VlVec2 p1, VlVec2 p2, VlVec2 p3, VlVec2 p4) {
	VlVec2 res = { .x = (float) VL_MATH_NO_RESULT, .y = (float) VL_MATH_NO_RESULT };

	float xd = ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
	float yd = ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));

	if (xd == 0 || yd == 0) return res;

	res.x =
		((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) /
		xd;

	res.y =
		((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y * p2.y) * (p3.x * p4.y - p3.y * p4.x)) /
		yd;

	return res;
}