#ifndef VELOURS_PLATFORM_WIN_MAIN_H
#define VELOURS_PLATFORM_WIN_MAIN_H

#include <Windows.h>
#include <shellapi.h>

// for VlWinInstance struct
#include "instance.h"

// for void vl_instance_set(VlInstance *instance)
#include "platform/instance.h" 

// for re-encoding utf-16 string to utf-8 strings
#include "utf.h"
#include "da.h"

#ifndef NDEBUG
#define CREATE_CONSOLE() \
	do { \
		AllocConsole(); \
		SetConsoleTitle(TEXT("Velours Debug Console")); \
		FILE* pCout; \
		FILE* pCin; \
		FILE* pCerr; \
		freopen_s(&pCout, "CONOUT$", "w", stdout); \
		freopen_s(&pCin, "CONIN$", "r", stdin); \
		freopen_s(&pCerr, "CONOUT$", "w", stderr); \
	} while (0)
#else
#define CREATE_CONSOLE() do {} while (0)
#endif

#ifndef NDEBUG
#define FREE_CONSOLE() \
	do { \
		FreeConsole(); \
	} while (0)
#else
#define FREE_CONSOLE() do {} while (0)
#endif

#define main(...) \
	main(__VA_ARGS__); \
	int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR pCmdLine, _In_ int nCmdShow) { \
		VL_UNUSED(hPrevInstance); \
		VL_UNUSED(pCmdLine); \
		VL_UNUSED(nCmdShow); \
		HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0); \
		CREATE_CONSOLE(); \
		VlWinInstance win; \
		win.hInstance = hInstance; \
		win.nCmdShow = nCmdShow; \
		vl_instance_set((VlInstance*) &win); \
		LPWSTR cmd = GetCommandLine(); \
		int argc = 1; \
		LPWSTR *argv = CommandLineToArgvW(cmd, &argc); \
		VL_DA(u8*) converted = NULL; \
		VL_DA_NEW_WITH_ALLOCATOR(converted, u8*, malloc); \
		for (int i = 0; i < argc; i++) { \
			VL_DA_APPEND_CONST_WITH_ALLOCATOR(converted, u8*, utf16_to_utf8(argv[i]), realloc); \
		} \
		int res = main(argc, (char**) converted); \
		VL_DA_FOREACH(converted, i) { \
			VL_DA_FREE_WITH_ALLOCATOR(converted[i], free); \
		} \
		VL_DA_FREE_WITH_ALLOCATOR(converted, free); \
		FREE_CONSOLE(); \
		return res; \
	} \
	int main(__VA_ARGS__)

#endif