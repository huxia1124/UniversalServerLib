//
//Copyright(c) 2016. Huan Xia
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
//documentation files(the "Software"), to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
//and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions
//of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
//CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.

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

