#ifndef VELOURS_PLATFORM_WIN_TIMER_H
#define VELOURS_PLATFORM_WIN_TIMER_H

#include "platform/timer.h"

typedef struct {
	struct VlTimer base;
	VL_DA(u16) name16;
	HANDLE h;
	HANDLE t;
	char reset;
} VlWinTimer;

#endif // VELOURS_PLATFORM_WIN_TIMER_H