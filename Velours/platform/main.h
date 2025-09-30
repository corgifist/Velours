#ifndef VELOURS_PLATFORM_MAIN_H
#define VELOURS_PLATFORM_MAIN_H

#include "velours.h"

#ifdef WIN
#include "win/main.h"
#else 
#include "universal/main.h"
#endif // WIN

#endif // VELOURS_PLATFORM_MAIN_H