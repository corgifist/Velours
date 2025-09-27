#ifndef VELOURS_H
#define VELOURS_H

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
#define VL_EXTERN_C extern "C"
#else
#define VL_EXTERN_C
#endif // __cplusplus

#if defined(WIN32) && defined(VELOURS_SHARED)
#ifdef VELOURS_EXPORT
#define VL_API VL_EXTERN_C __declspec(dllexport)
#else
#define VL_API VL_EXTERN_C __declspec(dllimport)
#endif // VELOURS_EXPPORT
#else
#define VL_API
#endif // defined(WIN32) && defined(VELOURS_SHARED)

#define VL_SUCCESS 0
#define VL_ERROR 1

#define VL_UNUSED(x) ((void*) &x)

// does nothing, like literally nothing
// should be used to chain preprocessor macros together
// check da.h to see how VL_BASE is used
#define VL_BASE(X) X

#define VL_STRINGIFY(X) u8#X
#define VL_STRINGIFY_EXPAND(X) VL_STRINGIFY(X)
#define VL_STRINGIFY_VARIADIC(...) VL_STRINGIFY_EXPAND(__VA_ARGS__)

typedef char VlResult;

#endif