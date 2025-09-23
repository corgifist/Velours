#include "velours.h"
#include <Windows.h>

const wchar_t CLASS_NAME[] = L"VelourWindowClass";

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ PWSTR pCmdLine, 
	_In_ int nCmdShow) {
	greet();

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME, 
		L"Velours",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL) {
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);

	MSG msg = { 0 };
	while (GetMessage(&msg, hwnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT: {
		return 0;
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}