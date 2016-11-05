#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>

class CSTXCommon
{
public:
	static BOOL GradientFill(HDC hDC, LPCRECT lprcRect, COLORREF clrBegin, COLORREF clrEnd, BOOL bHorizontal);
	static BOOL GradientFill(Gdiplus::Graphics *pGraphics, LPCRECT lprcRect, COLORREF clrBegin, COLORREF clrEnd, BOOL bHorizontal);
	static BOOL GradientFillEx(HDC hDC, LPCRECT lprcRect, COLORREF clrMainColor);
	static BOOL GradientFillEx(Gdiplus::Graphics *pGraphics, LPCRECT lprcRect, COLORREF clrMainColor);
	static void DrawTitleBar(HDC hDC, LPCTSTR pszTitle, RECT &rcArea);
	static IStream* LoadImageFromResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);
};