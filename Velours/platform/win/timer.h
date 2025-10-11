#ifndef VELOURS_PLATFORM_WIN_TIMER_H
#define VELOURS_PLATFORM_WIN_TIMER_H

#include "platform/timer.h"

#include <Windows.h>

typedef struct {
	struct VlTimer base;
	HANDLE h;
	HANDLE t;
	i64 internal_milliseconds;
	char reset;
	char should_fire_now;
	char running;
} VlWinTimer;

#endif // VELOURS_PLATFORM_WIN_TIMER_H