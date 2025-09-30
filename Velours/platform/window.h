#ifndef VELOURS_PLATFORM_WINDOW_H
#define VELOURS_PLATFORM_WINDOW_H

#include "velours.h"

typedef struct {
	void* platform_handle;
} VlWindow;

VL_API VlWindow* vl_window_new(const char *title, int w, int h);

#endif // VELOURS_PLATFORM_WINDOW_H