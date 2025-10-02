cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Velours LANGUAGES C CXX)

set(Velours_Sources
	velours.c
	velours.h
	da.h
	memory.c
	memory.h 
	ht.c
	ht.h
	file.c
	file.h
	utf.h)

add_subdirectory(xml)
add_subdirectory(platform)

add_library(Velours ${Velours_Sources})
target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_EXPORT)

if(BUILD_SHARED_LIBS)
	target_compile_definitions(Velours PRIVATE PROPERTIES VELOURS_SHARED)
endif()

if(WIN32)
	target_compile_definitions(Velours PRIVATE UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS)
	target_compile_options(Velours PRIVATE /utf-8 /Wall /WX)
	target_compile_options(Velours PUBLIC /wd4820 /wd5045 /wd4710 /wd4711 /wd5039)
	target_link_libraries(Velours PRIVATE d2d1)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(Velours PUBLIC VELOURS_MANAGED_MALLOC)
endif()

target_include_directories(Velours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})