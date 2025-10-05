add_subdirectory(universal)

if(WIN32)
	add_subdirectory(win)
endif()

set(Velours_Sources 
	${Velours_Sources}
	platform/instance.c
	PARENT_SCOPE
)

set(Velours_Headers
	${Velours_Headers}
	platform/instance.h
	platform/window.h
	platform/main.h
	platform/graphics.h
	PARENT_SCOPE
)