#ifndef VELOURS_PLATFORM_WIN_MAIN_H
#define VELOURS_PLATFORM_WIN_MAIN_H

#include <Windows.h>
#include <shellapi.h>

// for VlWinInstance struct
#include "instance.h"

// for void vl_instance_set(VlInstance *instance)
#include "platform/instance.h" 

// why is this ifdef not in windows.h?...
#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#else
#define CommandLineToArgv CommandLineToArgvA
#endif // UNICODE

#define main(...) \
	main(__VA_ARGS__); \
	int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR pCmdLine, _In_ int nCmdShow) { \
		VL_UNUSED(hPrevInstance); \
		VL_UNUSED(pCmdLine); \
		VL_UNUSED(nCmdShow); \
		VlWinInstance win; \
		win.hInstance = hInstance; \
		win.nCmdShow = nCmdShow; \
		vl_instance_set((VlInstance*) &win); \
		LPWSTR cmd = GetCommandLine(); \
		int argc = 1; \
		LPWSTR *argv = CommandLineToArgvW(cmd, &argc); \
		return main(argc, (char**) argv); \
	} \
	int main(__VA_ARGS__)

#endif