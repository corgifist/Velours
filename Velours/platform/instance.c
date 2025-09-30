#include "instance.h"

static VlInstance *s_instance = NULL;

VL_API VlInstance *vl_instance_get(void) {
	return s_instance;
}

VL_API void vl_instance_set(VlInstance *instance) {
	s_instance = instance;
}