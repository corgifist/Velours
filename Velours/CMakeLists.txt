cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Velours LANGUAGES C CXX)

set(Velours_Sources
	velours.c
	memory.c
	ht.c
	file.c
	geometry.c
)

set(Velours_Headers
	velours.h
	da.h
	file.h
	utf.h
	ht.h
	memory.h 
)

add_subdirectory(xml)
add_subdirectory(platform)

if(BUILD_SHARED_LIBS)
	add_library(Velours SHARED ${Velours_Sources} ${Velours_Headers})
else()
	add_library(Velours STATIC ${Velours_Sources} ${Velours_Headers})
endif()

target_compile_definitions(Velours PRIVATE VELOURS_EXPORT)

if(ENABLE_SOFTWARE_BACKEND)
	target_compile_definitions(Velours PRIVATE VL_SOFTWARE_BACKEND_ENABLED)
endif()

if(BUILD_SHARED_LIBS)
	target_compile_definitions(Velours PRIVATE VELOURS_SHARED)
endif()

# MSVC-specific compilation flags
# TODO: add support for gcc / clang here
if(WIN32)
	target_compile_definitions(Velours PRIVATE UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS)
	target_compile_options(Velours PRIVATE /utf-8 /Wall /WX)
	target_compile_options(Velours PUBLIC /wd4820 /wd5045 /wd4710 /wd4711 /wd5039)
endif()

# explicitly linking math library is not required on windows
if(NOT WIN32)
	target_link_libraries(Velours PRIVATE m)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(Velours PUBLIC VELOURS_MANAGED_MALLOC)
endif()

target_include_directories(Velours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})