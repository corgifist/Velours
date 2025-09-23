cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(MiniBrowser LANGUAGES C)

set(MiniBrowser_Sources
	"minibrowser.c"
)

add_executable(MiniBrowser ${MiniBrowser_Sources})
target_link_libraries(MiniBrowser PRIVATE Velours)

if(WIN32)
	target_compile_definitions(MiniBrowser PRIVATE PROPERTIES UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
	target_link_options(MiniBrowser PRIVATE "/SUBSYSTEM:WINDOWS")
endif()