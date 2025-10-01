#ifndef VELOURS_PLATFORM_WINDOW_H
#define VELOURS_PLATFORM_WINDOW_H

#include "velours.h"

// VlWindow is a pointer to platform-specific window structure
// e.g. VlWinWindow on windows
typedef void *VlWindow;

VL_API VlWindow* vl_window_new(const char *title, int w, int h);
VL_API void vl_window_message_loop(VlWindow *window);
VL_API VlResult vl_window_free(VlWindow *window);

#endif // VELOURS_PLATFORM_WINDOW_H