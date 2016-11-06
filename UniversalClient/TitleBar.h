#pragma once
#include <atlwin.h>

class CTitleBar : public CWindowImpl<CTitleBar>
{
public:
	CTitleBar();
	~CTitleBar();

protected:
	BEGIN_MSG_MAP(CTitleBar)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	END_MSG_MAP()


	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBackground(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

};

