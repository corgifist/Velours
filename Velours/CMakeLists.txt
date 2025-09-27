cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(VeloursWebView LANGUAGES C)

set(Velours_Sources
	velours.c
	velours.h
	da.h
	utf8.h
	memory.c
	memory.h "ht.h" "ht.c")

add_subdirectory(xml)

add_library(Velours ${Velours_Sources})
target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_EXPORT)

if(BUILD_SHARED_LIBS)
	target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_SHARED)
endif()

if(WIN32)
	target_compile_definitions(Velours PRIVATE UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS)
	target_compile_options(Velours PRIVATE /utf-8 /Wall /WX /wd4820 /wd5045)
endif()

target_compile_definitions(Velours PUBLIC VELOURS_MANAGED_MALLOC)

target_include_directories(Velours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})