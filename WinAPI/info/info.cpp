#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

// Функция, возвращающая максимальную высоту окна (в пикселях)
extern "C" _declspec(dllexport) int MaxHeight()
{
	return GetSystemMetrics(SM_CYFULLSCREEN);
}

// Функция, возвращающая информацию о наличии поддержки технологии SSE
extern "C" _declspec(dllexport) bool SSESupport()
{
	unsigned result = 0;
	int value = 0x00000001;
	
	// Вызов инструкции cpuid с параметром "1" в EAX
	__asm
	{
		mov eax, value
		cpuid
		mov result, edx
	}

	// Получение 25 бита регистра EDX
	result >>= 25;
	bool sse = result & 1;

	return sse;
}
