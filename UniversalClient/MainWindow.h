#pragma once

#include <atlwin.h>

#include "STXProtocolDialog.h"
#include "STXAnchor.h"

class CMainWindow : public CWindowImpl<CMainWindow, CWindow, CFrameWinTraits>
{
public:
	CMainWindow();
	~CMainWindow();

protected:
	BEGIN_MSG_MAP(CMainWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()


protected:
	CSTXProtocolDialog _dlg;
	CSTXAnchor *_anchor;

protected:
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		_dlg.DoOnClose();
		_dlg.DestroyWindow();
		DestroyWindow();
		return 0;
	}
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (_anchor)
		{
			delete _anchor;
			_anchor = NULL;
		}

		PostQuitMessage(0);
		return 0;
	}
	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		ModifyStyle(0, WS_CLIPCHILDREN);
		RECT rcWnd;
		GetClientRect(&rcWnd);
		_dlg.Create(m_hWnd);
		_dlg.MoveWindow(&rcWnd);
		_dlg.ShowWindow(SW_SHOW);

		_anchor = new CSTXAnchor(m_hWnd);

		_anchor->AddItem(_dlg.m_hWnd, STXANCHOR_ALL);

		//_anchor->AddItem(IDC_STATIC_TIPS_TEXT, STXANCHOR_BOTTOM | STXANCHOR_LEFT);
		
		return 0;
	}

public:
	void RunServerScriptFile(LPCTSTR lpszScriptFile);
	void RunServerScriptString(LPCTSTR lpszScript);

};

