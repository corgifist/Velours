cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(VeloursWebView LANGUAGES C)

set(Velours_Sources
	"velours.c"
	"velours.h"
)

add_library(Velours ${Velours_Sources})
target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_EXPORT)

if(BUILD_SHARED_LIBS)
	target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_SHARED)
endif()

if(WIN32)
	target_compile_definitions(Velours PRIVATE PROPERTIES UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_include_directories(Velours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})