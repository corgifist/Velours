#ifndef VELOURS_PLATFORM_WIN_INSTANCE_H
#define VELOURS_PLATFORM_WIN_INSTANCE_H

#include "velours.h"

#include <Windows.h>

typedef struct {
	HINSTANCE hInstance;
	int nCmdShow;
} VlWinInstance;

#endif // VELOURS_PLATFORM_WIN_INSTANCE_H