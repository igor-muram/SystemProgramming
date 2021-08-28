#define _CRT_SECURE_NO_WARNINGS
#define UNICODE
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define SIZE 256	// Максимальный размер строки, которая выводится в окно

WCHAR info[SIZE];	// Строка, которая выводится в окне

// Функция для проверки поддержки SSE
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

// Функция для загрузки динамической библиотеки и вызова функций из нее
DWORD WINAPI TreadFunc(void* args)
{
	typedef int (*HeightFunction)();
	typedef bool (*SSEFunction)();

	// Указатели на функции из динамической библиотеки
	HeightFunction DLLMaxHeight;
	SSEFunction DLLSSESupport;

	// Загрузка динамической библиотеки
	HINSTANCE hinstLib = LoadLibrary(TEXT("info.dll"));
	// Получение адресов функций из библиотеки 
	DLLMaxHeight = (HeightFunction)GetProcAddress(hinstLib, "MaxHeight");
	DLLSSESupport = (SSEFunction)GetProcAddress(hinstLib, "SSESupport");

	int height = DLLMaxHeight();
	bool sse = DLLSSESupport();

	// Освобождение динамической библиотеки
	FreeLibrary(hinstLib);


	// Запись полученной информации в строку, которая будет выведена в динамическом окне
	if (height != 0)
	{
		wsprintf(info, L"Max height of window: %dpx, SSE support: %s", height, sse ? L"true" : L"false");
	}
	else
	{
		wsprintf(info, L"Failed to determine max height of window, SSE support: %s", sse ? L"true" : L"false");
	}

	return 0;
}

// Функция обработки сообщений графического окна
LRESULT CALLBACK WindowFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThread;
	DWORD threadID;

	PAINTSTRUCT ps;
	HDC hDC;

	switch (msg)
	{
	case WM_CREATE:
		// Создание потока, выполняющего функцию для получения информации
		hThread = CreateThread(NULL, 0, TreadFunc, NULL, 0, &threadID);
		// Ожидание завершения выполнения потока
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		break;

	case WM_DESTROY:
		// Запрос на завершение работы приложения с кодом 0
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		// Вывод текста в окне
		hDC = BeginPaint(hWnd, &ps);
		TextOut(hDC, 10, 10, info, wcslen(info));
		EndPaint(hWnd, &ps);
		break;

	default:
		// Стандартная функция обработки сообщений
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

// Основная функция графического приложения Windows
int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str, int nWinMode)
{
	// Создание класса окна
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

	// Регистрация класса окна
	RegisterClass(&wc);

	// Создание окна Windows
	HWND hWnd = CreateWindow(
		L"MainWindow",
		L"Window max client height and SSE",
		WS_OVERLAPPED | WS_SYSMENU,
		400, 400,
		400, 80,
		HWND_DESKTOP,
		NULL,
		hThisInst,
		NULL);

	// Отображение и обновление окна
	ShowWindow(hWnd, nWinMode);
	UpdateWindow(hWnd);

	// Обработка сообщений
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);	// Перевод сообщений виртуальных клавиш в символьные сообщения
		DispatchMessage(&msg);	// Отправка сообщений в функцию обработки сообщений
	}

	return msg.wParam;
}