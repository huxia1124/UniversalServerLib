#include "stdafx.h"
#include "TitleBar.h"
#include "Common.h"


CTitleBar::CTitleBar()
{
}


CTitleBar::~CTitleBar()
{
}

LRESULT CTitleBar::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	PAINTSTRUCT ps;
	BeginPaint(&ps);

	CString titleStr;
	GetParent().GetWindowText(titleStr);

	CSTXCommon::DrawTitleBar(ps.hdc, titleStr, rcClient);

	EndPaint(&ps);
	return 0;
}

LRESULT CTitleBar::OnEraseBackground(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	//Skip default background drawing
	return 0;
}
