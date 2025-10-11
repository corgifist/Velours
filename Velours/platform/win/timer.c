#include "timer.h"

#include "memory.h"
#include "utf.h"

static DWORD WINAPI vl_timer_proc(LPVOID param) {
	LARGE_INTEGER start, end;
	LARGE_INTEGER freq;
	VlWinTimer *timer = (VlWinTimer*) param;
	i64 base_milliseconds = (i64) timer->base.milliseconds;

	QueryPerformanceFrequency(&freq);
	while (timer->running) {
		if (timer->should_fire_now || WaitForSingleObject(timer->h, INFINITE) == WAIT_OBJECT_0) {
			timer->should_fire_now = 0;
			QueryPerformanceCounter(&start);
			timer->base.fn((VlTimer) timer);
			QueryPerformanceCounter(&end);
			if (!timer->reset) return 0;
		} else {
			printf("failed to wait on timer '%s'\n", timer->base.name);
			break;
		}

		i64 elapsed_milliseconds = (end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart;
		// printf("%lld - %lld = %lld\n", base_milliseconds, elapsed_milliseconds, base_milliseconds - elapsed_milliseconds);
		if (elapsed_milliseconds > base_milliseconds) {
			timer->should_fire_now = 1;
			timer->internal_milliseconds = base_milliseconds;
			continue;
		}
		timer->internal_milliseconds = base_milliseconds - elapsed_milliseconds;
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
	timer->reset = 0;
	timer->should_fire_now = 0;
	timer->running = 1;
	timer->internal_milliseconds = milliseconds;

	VL_DA(u16) name16 = utf8_to_utf16(name);
	timer->h = CreateWaitableTimerEx(
		NULL,
		NULL,
		type & VL_TIMER_PRECISE ? CREATE_WAITABLE_TIMER_HIGH_RESOLUTION : 0,
		TIMER_ALL_ACCESS
	);
	VL_DA_FREE(name16);

	if (!timer->h) {
		timer->h = CreateWaitableTimer(
			NULL,
			FALSE,
			NULL
		);
		if (!timer->h) {
			VL_FREE(timer);
			printf("error: %i\n", (int)GetLastError());
			return NULL;
		}
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

	int priority = -69;
	if (type & VL_TIMER_LOW_PRIORITY) priority = THREAD_PRIORITY_LOWEST;
	if (type & VL_TIMER_HIGH_PRIORITY) priority = THREAD_PRIORITY_ABOVE_NORMAL;
	if (type & VL_TIMER_CRITICAL_PRIORITY) priority = THREAD_PRIORITY_TIME_CRITICAL;
	if (priority != -69) SetThreadPriority(timer->t, priority);

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
	i64 ns = (i64) (-((double)timer->internal_milliseconds) * 10000);
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
	timer->running = 0;
	vl_timer_wait(t, 0);
	if (timer->t) CloseHandle(timer->t);
	if (timer->h) CloseHandle(timer->h);
	VL_FREE(timer);
	return VL_SUCCESS;
}
VL_API u64 vl_timer_get_milliseconds(void) {
	LARGE_INTEGER li, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&li);
	return li.QuadPart * 1000 / freq.QuadPart;
}