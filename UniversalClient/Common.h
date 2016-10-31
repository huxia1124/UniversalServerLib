#pragma once

#include <Windows.h>

IStream* LoadImageFromResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);

void DrawTitleBar(HDC hDC, LPCTSTR pszTitle, RECT &rcArea);