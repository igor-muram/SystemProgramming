#define _CRT_SECURE_NO_WARNINGS
#define UNICODE
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define SIZE 256

WCHAR info[SIZE];

bool SSESupport()
{
	unsigned result = 0;
	int value = 0x00000001;
	__asm
	{
		mov eax, value
		cpuid
		mov result, edx
	}


	result >>= 25;
	bool sse = result & 1;

	return sse;
}

DWORD WINAPI TreadFunc(void* args)
{
	// Получение функции из 
	typedef int (*HeightFunction)();
	typedef bool (*SSEFunction)();

	HeightFunction DLLMaxHeight;
	SSEFunction DLLSSESupport;

	HINSTANCE hinstLib = LoadLibrary(TEXT("info.dll"));
	DLLMaxHeight = (HeightFunction)GetProcAddress(hinstLib, "MaxHeight");
	DLLSSESupport = (SSEFunction)GetProcAddress(hinstLib, "SSESupport");

	int height = DLLMaxHeight();
	bool sse = DLLSSESupport();

	wsprintf(info, L"Max-height of window: %dpx, SSE support: %s", height, sse ? L"true" : L"false");
	FreeLibrary(hinstLib);
	return 0;
}

LRESULT CALLBACK WindowFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThread;
	DWORD threadID;

	PAINTSTRUCT ps;
	HDC hDC;

	switch (msg)
	{
	case WM_CREATE:
		hThread = CreateThread(NULL, 0, TreadFunc, NULL, 0, &threadID);
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		TextOut(hDC, 10, 10, info, wcslen(info));
		EndPaint(hWnd, &ps);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str, int nWinMode)
{
	WNDCLASS wc;
	wc.hInstance = hThisInst;
	wc.lpszClassName = L"MainWindow";
	wc.lpfnWndProc = WindowFunc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
		L"MainWindow",
		L"Window max-height and SSE",
		WS_OVERLAPPEDWINDOW,
		400, 400,
		700, 80,
		HWND_DESKTOP,
		NULL,
		hThisInst,
		NULL);

	ShowWindow(hWnd, nWinMode);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}