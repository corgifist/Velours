#ifndef VELOURS_H
#define VELOURS_H

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
#define VL_EXTERN_C extern "C"
#else
#define VL_EXTERN_C
#endif // __cplusplus

#if defined(_WIN32) || defined(_WIN64)
#define WIN
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define IOS
#elif TARGET_OS_IPHONE == 1
#define IOS
#elif TARGET_OS_MAC == 1
#define MACOS
#endif 
#define DARWIN
#elif defined(__linux__)
#define LINUX
#endif


// does nothing, like literally nothing
// should be used to chain preprocessor macros together
// check da.h to see how VL_BASE is used
#define VL_BASE(X) X

#define PLATFORM(PLATFORM) VL_BASE(VELOURS_PLATFORM_##PLATFORM)

#if defined(WIN) && defined(VELOURS_SHARED)
#ifdef VELOURS_EXPORT
#define VL_API VL_EXTERN_C __declspec(dllexport)
#else
#define VL_API VL_EXTERN_C __declspec(dllimport)
#endif // VELOURS_EXPPORT
#else
#define VL_API
#endif // defined(WIN) && defined(VELOURS_SHARED)

#include "platform/main.h"

#define VL_SUCCESS 0
#define VL_ERROR 1

#define VL_UNUSED(x) ((void*) &x)

#define VL_STRINGIFY(X) u8#X
#define VL_STRINGIFY_EXPAND(X) VL_STRINGIFY(X)
#define VL_STRINGIFY_VARIADIC(...) VL_STRINGIFY_EXPAND(__VA_ARGS__)

#define VL_LOG_ERROR(...) fprintf(stderr, __FILE__ "(" VL_STRINGIFY_EXPAND(__LINE__) "): " __VA_ARGS__)

typedef char VlResult;

#endif