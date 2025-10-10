#ifndef VELOURS_PLATFORM_TIMER_H
#define VELOURS_PLATFORM_TIMER_H

#include "velours.h"
#include "da.h"

struct VlTimer;
typedef void (*VlTimerFunction)(struct VlTimer*);

typedef enum {
	VL_TIMER_NORMAL = 0,
	VL_TIMER_PRECISE = 1
} VlTimerType;

struct VlTimer {
	// timer name, using vl_timer_get(u8 *name) you can retrieve the timer with specified name
	VL_DA(u8) name;

	// amount of milliseconds the timer waits
	size_t milliseconds;

	VlTimerType type;

	VlTimerFunction fn;
};

typedef struct VlTimer* VlTimer;

VL_API VlTimer vl_timer_new(u8 *name, u64 milliseconds, VlTimerType type, VlTimerFunction fn);

// milliseconds <= 0 means infinite amount of time 
VL_API char vl_timer_wait(VlTimer timer, i64 milliseconds);
VL_API VlResult vl_timer_reset(VlTimer timer);
VL_API VlResult vl_timer_free(VlTimer timer);

#endif // VELOURS_PLATFORM_TIMER_H