add_subdirectory(universal)

if(WIN32)
	add_subdirectory(win)
endif()

set(Velours_Sources 
	${Velours_Sources}
	platform/instance.c
	platform/instance.h
	platform/window.h
	platform/main.h
	PARENT_SCOPE
)