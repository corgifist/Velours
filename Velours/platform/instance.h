#ifndef VELOURS_PLATFORM_INSTANCE_H
#define VELOURS_PLATFORM_INSTANCE_H

#include "velours.h"

// VlInstance is a pointer to platform-specific data
// e.g on windows VlInstance would be a pointer to VlWinInstance struct,
// on linux it would point to VlLinuxInstance and so on
typedef void* VlInstance;

VL_API VlInstance *vl_instance_get(void);
VL_API void vl_instance_set(VlInstance *instance);

#endif // VELOURS_PLATFORM_INSTANCE_H