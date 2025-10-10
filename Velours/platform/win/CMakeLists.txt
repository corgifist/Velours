if(ENABLE_SOFTWARE_BACKEND)
	add_subdirectory(software)
endif()

set(Velours_Sources 
	${Velours_Sources}
	platform/win/window.c
	platform/win/timer.c
	PARENT_SCOPE
)

set(Velours_Headers
	${Velours_Headers}
	platform/win/instance.h
	platform/win/window.h
	platform/win/timer.h
	platform/win/main.h
	PARENT_SCOPE
)