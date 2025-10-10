#include "timer.h"

#include "memory.h"
#include "utf.h"

static DWORD WINAPI vl_timer_proc(LPVOID param) {
	VlWinTimer *timer = (VlWinTimer*) param;
	while (1) {
		if (WaitForSingleObject(timer->h, (DWORD)timer->base.milliseconds) == WAIT_OBJECT_0) {
			timer->base.fn((VlTimer)timer);
			if (!timer->reset) return 0;
		}
	}
	return 1;
}

VOID CALLBACK vl_timer_apc_proc(
	LPVOID lpArg,               // Data value
	DWORD dwTimerLowValue,      // Timer low value
	DWORD dwTimerHighValue)    // Timer high value

{
	// Formal parameters not used in this example.
	UNREFERENCED_PARAMETER(dwTimerLowValue);
	UNREFERENCED_PARAMETER(dwTimerHighValue);

	VlWinTimer *timer = (VlWinTimer*) lpArg;
	timer->base.fn((VlTimer) timer);
}

VL_API VlTimer vl_timer_new(u8* name, u64 milliseconds, VlTimerType type, VlTimerFunction fn) {
	VlWinTimer *timer = VL_MALLOC(sizeof(VlWinTimer));
	if (!timer) return NULL;
	memset(timer, 0, sizeof(VlWinTimer));

	timer->base.name = name;
	timer->base.milliseconds = milliseconds;
	timer->base.type = type;
	timer->base.fn = fn;
	timer->name16 = utf8_to_utf16(name);
	timer->reset = 0;
	timer->h = CreateWaitableTimerEx(
		NULL,
		NULL,
		type == VL_TIMER_PRECISE ? CREATE_WAITABLE_TIMER_HIGH_RESOLUTION : 0,
		TIMER_ALL_ACCESS
	);
	if (!timer->h) {
		VL_FREE(timer);
		return NULL;
	}

	if (vl_timer_reset((VlTimer)timer)) {
		vl_timer_free((VlTimer)timer);
		return NULL;
	}

	timer->t = CreateThread(
		NULL,
		0,
		vl_timer_proc,
		timer,
		0,
		NULL
	);
	if (!timer->t) {
		printf("oopsie-doopsie\n");
		return NULL;

	}

	return (VlTimer) timer;
}

VL_API VlResult vl_timer_wait(VlTimer t, i64 milliseconds) {
	if (!t) return VL_ERROR;
	VlWinTimer* timer = (VlWinTimer*)t;
	if (WaitForSingleObject(timer->t, milliseconds <= 0 ? INFINITE : (DWORD)milliseconds) != WAIT_OBJECT_0) {
		printf("oh fuck\n");
	};
	timer->reset = 0;
	return VL_SUCCESS;
}

VL_API VlResult vl_timer_reset(VlTimer t) {
	if (!t) return VL_ERROR;
	VlWinTimer *timer = (VlWinTimer*) t;
	LARGE_INTEGER due = { 0 };
	i64 ns = (i64) (-((double)t->milliseconds) * 10000);
	due.LowPart = (DWORD) (ns & 0xFFFFFFFF);
	due.HighPart = (LONG) (ns >> 32);

	if (!SetWaitableTimer(
		timer->h,
		&due,
		0,
		NULL,
		NULL,
		FALSE
	)) {
		return VL_ERROR;
	}

	timer->reset = 1;

	return VL_SUCCESS;
}

VL_API VlResult vl_timer_free(VlTimer t) {
	if (!t) return VL_ERROR;
	VlWinTimer *timer = (VlWinTimer*) t;
	if (timer->h) CloseHandle(timer->h);
	if (timer->t) CloseHandle(timer->t);
	VL_FREE(timer->name16);
	VL_FREE(timer);
	return VL_SUCCESS;
}