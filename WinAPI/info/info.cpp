#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

extern "C" _declspec(dllexport) int MaxHeight()
{
	return GetSystemMetrics(SM_CYFULLSCREEN);
}


extern "C" _declspec(dllexport) bool SSESupport()
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
