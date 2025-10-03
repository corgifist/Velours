#ifndef VELOURS_PLATFORM_WINDOW_H
#define VELOURS_PLATFORM_WINDOW_H

#include "velours.h"

// VlWindow is a pointer to platform-specific window structure
// e.g. VlWinWindow on windows
typedef void* VlWindow;

typedef void (*VlWindowPaintFunction)(VlWindow);
typedef void (*VlWindowResizeFunction)(VlWindow, int, int);
typedef void (*VlWindowMoveFunction)(VlWindow, int, int);

VL_API VlWindow vl_window_new(const u8 *title, int x, int y, int w, int h);
VL_API void vl_window_set_visible(VlWindow window, char visible);
VL_API void vl_window_set_paint_function(VlWindow window, VlWindowPaintFunction fn);
VL_API void vl_window_set_resize_function(VlWindow window, VlWindowResizeFunction fn);
VL_API void vl_window_set_move_function(VlWindow window, VlWindowMoveFunction fn);
VL_API void vl_window_message_loop(VlWindow window);
VL_API VlResult vl_window_free(VlWindow window);

#endif // VELOURS_PLATFORM_WINDOW_H