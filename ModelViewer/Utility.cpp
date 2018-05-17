#include "pch.h"
#include "Utility.h"
#include <strsafe.h>

const wchar_t *newline = L"\n";

void Utility::Out(LPCTSTR sFormat, ...)
{
#ifndef NO_OUTPUT
	va_list argptr;
	va_start(argptr, sFormat);
	wchar_t buffer[2000];
	HRESULT hr = StringCbVPrintf(buffer, sizeof(buffer), sFormat, argptr);
	if (STRSAFE_E_INSUFFICIENT_BUFFER == hr || S_OK == hr)
	{
		wcsncat_s(buffer, newline, (rsize_t)sizeof(buffer));
		OutputDebugString(buffer);
	}
	else
		OutputDebugString(L"StringCbVPrintf error.");
#endif
}