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
#include "STXAnimatedTreeCtrlNS.h"
#include <set>
#include <map>
#include <memory>
#include "Common.h"

#define WM_NEW_SCRIPT_WINDOW				(WM_USER + 101)
#define WM_APPEND_RECEIVED_MESSAGE			(WM_USER + 102)
#define WM_UPDATE_RECEIVED_MESSAGE_COUNT	(WM_USER + 103)
#define WM_SWITCH_TO_PROTOCOL_TEST			(WM_USER + 104)


class CMainContainerWindow : public CDialogImpl<CMainContainerWindow>
{
public:
	CMainContainerWindow();
	~CMainContainerWindow();

	enum { IDD = IDD_DIALOG_MAIN_CONTAINER };
protected:


	BEGIN_MSG_MAP(CMainContainerWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		MESSAGE_HANDLER(WM_NEW_SCRIPT_WINDOW, OnNewScriptWindow)
		MESSAGE_HANDLER(WM_APPEND_RECEIVED_MESSAGE, OnAppendReceivedMessage)
		MESSAGE_HANDLER(WM_UPDATE_RECEIVED_MESSAGE_COUNT, OnUpdateReceivedMessageCount)
		MESSAGE_HANDLER(WM_SWITCH_TO_PROTOCOL_TEST, OnSwitchToProtocolTest)

		NOTIFY_CODE_HANDLER(STXATVN_SELECTEDITEMCHANGED, OnTreeSelectedItemChanged)
		NOTIFY_CODE_HANDLER(STXATVN_ITEMDBLCLICK, OnTreeItemDblClick)
		NOTIFY_CODE_HANDLER(STXATVN_ITEMCLICK, OnTreeItemClick)
	END_MSG_MAP()


protected:
	CSTXAnchor *_anchor;
	CSTXAnimatedTreeCtrlNS _tree;

	HSTXTREENODE _nodeProtocolTest = nullptr;
	HSTXTREENODE _nodeMessageReceive = nullptr;
	HSTXTREENODE _nodeServerScriptParent = nullptr;
	HSTXTREENODE _nodeServerData = nullptr;

	std::shared_ptr<CWindow> _currentMasterDialog;
	std::map<std::wstring, std::shared_ptr<CWindow>> _masterDialogs;

protected:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnNewScriptWindow(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnAppendReceivedMessage(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnUpdateReceivedMessageCount(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSwitchToProtocolTest(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT OnTreeSelectedItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTreeItemClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTreeItemDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{

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


	void CreateMasterTree();
	void InitializeTreeItems();

	void CreateProtocolTestWindow();
	void CreateMessageReceiveWindow(BOOL activate);
	void CreateServerDataWindow();

	template<class WindowClassType>
	void CreateStandardContainedWindow(LPCTSTR lpszKey, LPCTSTR lpszTitle, BOOL activate)
	{
		if (activate)
		{
			if (_currentMasterDialog && _currentMasterDialog->IsWindow())
			{
				_currentMasterDialog->ShowWindow(SW_HIDE);
			}
		}

		LPCTSTR windowKey = lpszKey;

		auto contentPlaceholder = GetDlgItem(IDC_STATIC_CONTENT);
		RECT rcContent;
		contentPlaceholder.GetWindowRect(&rcContent);
		this->ScreenToClient(&rcContent);

		auto it = _masterDialogs.find(windowKey);
		if (it != _masterDialogs.end())
		{
			if (activate)
			{
				_currentMasterDialog = it->second;
				_currentMasterDialog->ShowWindow(SW_SHOW);
			}
			return;
		}

		auto dlg = std::make_shared<WindowClassType>();
		if (activate)
			_currentMasterDialog = dlg;

		dlg->Create(m_hWnd, rcContent);
		dlg->SetWindowText(lpszTitle);
		dlg->MoveWindow(&rcContent);
		if (activate)
		{
			dlg->ShowWindow(SW_SHOW);
		}
		else
		{
			dlg->ShowWindow(SW_HIDE);
		}

		_masterDialogs[windowKey] = dlg;

		_anchor->AddItem(dlg->m_hWnd, STXANCHOR_ALL);
	}

	void ShowScriptWindow(HSTXTREENODE currentNode);
	void CreateScriptWindow(HSTXTREENODE currentNode, int windowId = -1);

};

