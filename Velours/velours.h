#ifndef VELOURS_H
#define VELOURS_H

#ifdef __cplusplus
#define VELOURS_EXTERN_C extern "C"
#else
#define VELOURS_EXTERN_C
#endif

#if defined(WIN32) && defined(VELOURS_SHARED)
#ifdef VELOURS_EXPORT
#define VELOURS_API VELOURS_EXTERN_C __declspec(dllexport)
#else
#define VELOURS_API VELOURS_EXTERN_C __declspec(dllimport)
#endif
#else
#define VELOURS_API
#endif

#endif