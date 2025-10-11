#ifndef VELOURS_H
#define VELOURS_H

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

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
#define VL_API VL_EXTERN_C
#endif // defined(WIN) && defined(VELOURS_SHARED)

#define VL_SUCCESS 0
#define VL_ERROR 1

#define VL_UNUSED(x) ((void*) &x)

#define VL_STRINGIFY(X) u8#X
#define VL_STRINGIFY_EXPAND(X) VL_STRINGIFY(X)
#define VL_STRINGIFY_VARIADIC(...) VL_STRINGIFY_EXPAND(__VA_ARGS__)

#define VL_LOG_ERROR(...) fprintf(stderr, __FILE__ "(" VL_STRINGIFY_EXPAND(__LINE__) "): " __VA_ARGS__)

typedef char VlResult;

typedef char	 u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef short i16;
typedef int i32;
typedef int64_t i64;

// u8 *vl_format(const u8 *format, ...)
// automatically allocates memory and snprintf's into it
// NOTICE:
//     after you're done with the string vl_format returns
//     free it using VL_FREE macro, otherwise you're going to leak memory!
VL_API u8 *vl_format(const u8 *format, ...);

#endif