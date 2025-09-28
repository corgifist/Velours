cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(MiniBrowser LANGUAGES C)

set(MiniBrowser_Sources
	minibrowser.c
)

add_executable(MiniBrowser ${MiniBrowser_Sources})
target_link_libraries(MiniBrowser PRIVATE Velours)

if(WIN32)
	target_compile_definitions(MiniBrowser PRIVATE PROPERTIES UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
	# MSVC compilation flags breakdown:
	# /utf-8  - makes sure utf-8 is up and running in this build
	# /Wall   - enable all warnings, even the most pedantic ones
	# /WX     - treat all warnings as errors, the code should be perfect 
	# /wd5040 - disable Spectre migitations warning, they're pretty annoying and useless

	target_compile_options(MiniBrowser PRIVATE /utf-8 /Wall /WX)
endif()

configure_file(30mb.xml ${CMAKE_BINARY_DIR}/30mb.xml COPYONLY)