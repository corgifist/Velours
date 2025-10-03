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

static VL_DA(u8) __stdlib_free_utf16_to_utf8(const u16 * s) {
	VL_DA(u8) result;
	VL_DA_NEW_WITH_ALLOCATOR(result, u8, malloc);
	const u16* p = s;
	u32 cp = 0;
	while ((cp = utf16_decode(&p, NULL))) {
		u8 enc[4];
		int len = utf8_encode(cp, enc);
		if (len >= 1) VL_DA_APPEND_WITH_ALLOCATOR(result, enc[0], realloc);
		if (len >= 2) VL_DA_APPEND_WITH_ALLOCATOR(result, enc[1], realloc);
		if (len >= 3) VL_DA_APPEND_WITH_ALLOCATOR(result, enc[2], realloc);
		if (len >= 4) VL_DA_APPEND_WITH_ALLOCATOR(result, enc[3], realloc);
	}
	VL_DA_APPEND_CONST_WITH_ALLOCATOR(result, u8, 0, realloc);
	return result;
}

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
			u8 *enc = __stdlib_free_utf16_to_utf8((u16*) argv[i]); \
			VL_DA_APPEND_WITH_ALLOCATOR(converted, enc, realloc); \
		} \
		int res = main(argc, (char**) converted); \
		VL_DA_FOREACH(converted, i) { \
			VL_DA_FREE_WITH_ALLOCATOR(converted[i], free); \
		} \
		VL_DA_FREE_WITH_ALLOCATOR(converted, free); \
		vl_dump_all_allocations(); \
		printf("press any key to kill debug console\n"); \
		getchar(); \
		FREE_CONSOLE(); \
		return res; \
	} \
	int main(__VA_ARGS__)

#endif