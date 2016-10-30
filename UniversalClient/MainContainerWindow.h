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

		NOTIFY_CODE_HANDLER(STXATVN_SELECTEDITEMCHANGED, OnTreeSelectedItemChanged)
		NOTIFY_CODE_HANDLER(STXATVN_ITEMDBLCLICK, OnTreeItemDblClick)
	END_MSG_MAP()


protected:
	CSTXAnchor *_anchor;
	CSTXAnimatedTreeCtrlNS _tree;

	HSTXTREENODE _nodeProtocolTest = nullptr;
	HSTXTREENODE _nodeServerScriptParent = nullptr;
	HSTXTREENODE _nodeServerData = nullptr;

	std::shared_ptr<CWindow> _currentMasterDialog;
	std::map<std::wstring, std::shared_ptr<CWindow>> _masterDialogs;

protected:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnTreeSelectedItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
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
	void CreateServerDataWindow();
	void ShowScriptWindow(HSTXTREENODE currentNode);
	void CreateScriptWindow(HSTXTREENODE currentNode, int windowId = -1);

};

