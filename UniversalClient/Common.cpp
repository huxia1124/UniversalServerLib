#include "stdafx.h"
#include "Common.h"


BOOL CSTXCommon::GradientFill(HDC hDC, LPCRECT lprcRect, COLORREF clrBegin, COLORREF clrEnd, BOOL bHorizontal)
{
	TRIVERTEX vertex[2];
	GRADIENT_RECT grect;
	grect.UpperLeft = 0;
	grect.LowerRight = 1;
	vertex[0].x = lprcRect->left;
	vertex[0].y = lprcRect->top;
	vertex[0].Red = MAKEWORD(0, GetRValue(clrBegin));
	vertex[0].Green = MAKEWORD(0, GetGValue(clrBegin));
	vertex[0].Blue = MAKEWORD(0, GetBValue(clrBegin));
	vertex[0].Alpha = 255;
	vertex[1].x = lprcRect->right;
	vertex[1].y = lprcRect->bottom;
	vertex[1].Red = MAKEWORD(0, GetRValue(clrEnd));
	vertex[1].Green = MAKEWORD(0, GetGValue(clrEnd));
	vertex[1].Blue = MAKEWORD(0, GetBValue(clrEnd));
	vertex[1].Alpha = 255;

	return ::GradientFill(hDC, vertex, 2, &grect, 1, bHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
}

BOOL CSTXCommon::GradientFillEx(HDC hDC, LPCRECT lprcRect, COLORREF clrMainColor)
{
	COLORREF clrTopColor =
		RGB(GetRValue(clrMainColor) + (255 - GetRValue(clrMainColor)) * 8 / 10
			, GetGValue(clrMainColor) + (255 - GetGValue(clrMainColor)) * 8 / 10
			, GetBValue(clrMainColor) + (255 - GetBValue(clrMainColor)) * 8 / 10);

	COLORREF clrTopCenterColor =
		RGB(GetRValue(clrMainColor) + (255 - GetRValue(clrMainColor)) / 8
			, GetGValue(clrMainColor) + (255 - GetGValue(clrMainColor)) / 8
			, GetBValue(clrMainColor) + (255 - GetBValue(clrMainColor)) / 8);

	COLORREF clrBottomCenterColor =
		RGB(GetRValue(clrMainColor) * 8 / 10
			, GetGValue(clrMainColor) * 8 / 10
			, GetBValue(clrMainColor) * 8 / 10);

	COLORREF clrBottomColor =
		RGB(GetRValue(clrMainColor) / 3
			, GetGValue(clrMainColor) / 3
			, GetBValue(clrMainColor) / 3);

	RECT rcArea = *lprcRect;
	RECT rcUpper = *lprcRect; rcUpper.bottom = rcArea.top + (rcArea.bottom - rcArea.top) / 2;
	RECT rcLower = *lprcRect; rcLower.top = rcUpper.bottom;

	CSTXCommon::GradientFill(hDC, &rcUpper, clrTopColor, clrTopCenterColor, FALSE);
	CSTXCommon::GradientFill(hDC, &rcLower, clrBottomCenterColor, clrBottomColor, FALSE);

	return TRUE;
}

BOOL CSTXCommon::GradientFillEx(Gdiplus::Graphics *pGraphics, LPCRECT lprcRect, COLORREF clrMainColor)
{
	COLORREF clrTopColor =
		RGB(GetRValue(clrMainColor) + (255 - GetRValue(clrMainColor)) * 8 / 10
			, GetGValue(clrMainColor) + (255 - GetGValue(clrMainColor)) * 8 / 10
			, GetBValue(clrMainColor) + (255 - GetBValue(clrMainColor)) * 8 / 10);

	COLORREF clrTopCenterColor =
		RGB(GetRValue(clrMainColor) + (255 - GetRValue(clrMainColor)) / 8
			, GetGValue(clrMainColor) + (255 - GetGValue(clrMainColor)) / 8
			, GetBValue(clrMainColor) + (255 - GetBValue(clrMainColor)) / 8);

	COLORREF clrBottomCenterColor =
		RGB(GetRValue(clrMainColor) * 8 / 10
			, GetGValue(clrMainColor) * 8 / 10
			, GetBValue(clrMainColor) * 8 / 10);

	COLORREF clrBottomColor =
		RGB(GetRValue(clrMainColor) / 3
			, GetGValue(clrMainColor) / 3
			, GetBValue(clrMainColor) / 3);

	RECT rcArea = *lprcRect;
	RECT rcUpper = *lprcRect; rcUpper.bottom = rcArea.top + (rcArea.bottom - rcArea.top) / 2;
	RECT rcLower = *lprcRect; rcLower.top = rcUpper.bottom;

	CSTXCommon::GradientFill(pGraphics, &rcUpper, clrTopColor, clrTopCenterColor, FALSE);
	CSTXCommon::GradientFill(pGraphics, &rcLower, clrBottomCenterColor, clrBottomColor, FALSE);

	return TRUE;
}

BOOL CSTXCommon::GradientFill(Gdiplus::Graphics *pGraphics, LPCRECT lprcRect, COLORREF clrBegin, COLORREF clrEnd, BOOL bHorizontal)
{
	Gdiplus::Point pt1(lprcRect->left, lprcRect->top), ptH(lprcRect->right, lprcRect->top), ptV(lprcRect->left, lprcRect->bottom);
	if (bHorizontal)
	{
		Gdiplus::LinearGradientBrush brush(pt1, ptH,
			Gdiplus::Color(LOBYTE(clrBegin >> 24), LOBYTE(clrBegin), LOBYTE(clrBegin >> 8), LOBYTE(clrBegin >> 16)),
			Gdiplus::Color(LOBYTE(clrEnd >> 24), LOBYTE(clrEnd), LOBYTE(clrEnd >> 8), LOBYTE(clrEnd >> 16)));

		pGraphics->FillRectangle(&brush, lprcRect->left, lprcRect->top, lprcRect->right - lprcRect->left, lprcRect->bottom - lprcRect->top);
	}
	else
	{
		Gdiplus::LinearGradientBrush brush(pt1, ptV,
			Gdiplus::Color(255, LOBYTE(clrBegin), LOBYTE(clrBegin >> 8), LOBYTE(clrBegin >> 16)),
			Gdiplus::Color(255, LOBYTE(clrEnd), LOBYTE(clrEnd >> 8), LOBYTE(clrEnd >> 16)));

		pGraphics->FillRectangle(&brush, lprcRect->left, lprcRect->top, lprcRect->right - lprcRect->left, lprcRect->bottom - lprcRect->top);
	}

	return TRUE;
}