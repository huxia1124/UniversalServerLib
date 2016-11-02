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

#include "stdafx.h"
#include "MainContainerWindow.h"
#include "STXServerDataDialog.h"
#include "STXReceivedMessageDialog.h"
#include <cassert>

IStream* LoadImageFromResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
	HRSRC hRC = FindResource(hModule, lpName, lpType);
	if (hRC == NULL)
		return NULL;

	HGLOBAL hPkg = LoadResource(hModule, hRC);
	if (hPkg == NULL)
		return NULL;

	DWORD dwSize = SizeofResource(hModule, hRC);
	LPVOID pData = LockResource(hPkg);

	HGLOBAL hImage = GlobalAlloc(GMEM_FIXED, dwSize);
	LPVOID pImageBuf = GlobalLock(hImage);
	memcpy(pImageBuf, pData, dwSize);
	GlobalUnlock(hImage);

	UnlockResource(hPkg);

	IStream *pStream = NULL;
	CreateStreamOnHGlobal(hImage, TRUE, &pStream);

	return pStream;
}

void DrawTitleBar(HDC hDC, LPCTSTR pszTitle, RECT &rcArea)
{
	TRIVERTEX        vert[2];
	GRADIENT_RECT    gRect;

	CDCHandle dc;
	dc.Attach(hDC);

	COLORREF colorBK = GetSysColor(COLOR_BTNFACE);

	vert[0].x = rcArea.left;
	vert[0].y = rcArea.top;
	vert[0].Red = MAKEWORD(0, 64);
	vert[0].Green = MAKEWORD(0, 64);
	vert[0].Blue = MAKEWORD(0, 64);
	vert[0].Alpha = 0;

	vert[1].x = rcArea.right;
	vert[1].y = rcArea.bottom;
	vert[1].Red = MAKEWORD(0, GetRValue(colorBK));
	vert[1].Green = MAKEWORD(0, GetGValue(colorBK));
	vert[1].Blue = MAKEWORD(0, GetBValue(colorBK));
	vert[1].Alpha = 0;

	gRect.UpperLeft = 0;
	gRect.LowerRight = 1;

	GradientFill(hDC, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	auto oldBkMode = dc.GetBkMode();
	auto oldTextColor = dc.GetTextColor();
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(255, 255, 255));

	rcArea.left += 4;
	dc.DrawText(pszTitle, -1, &rcArea, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
	rcArea.left -= 4;	//restore

	dc.SetBkMode(oldBkMode);
	dc.SetTextColor(oldTextColor);
}


CMainContainerWindow::CMainContainerWindow()
{
	_anchor = NULL;
}

CMainContainerWindow::~CMainContainerWindow()
{
}

LRESULT CMainContainerWindow::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	ModifyStyle(0, WS_CLIPCHILDREN);
	RECT rcWnd;
	GetClientRect(&rcWnd);

	_anchor = new CSTXAnchor(m_hWnd);

	CreateMasterTree();

	InitializeTreeItems();

	_anchor->AddItem(IDC_STATIC_CONTENT, STXANCHOR_ALL);
	_anchor->AddItem(IDC_STATIC_TREE, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_BOTTOM);

	//_anchor->AddItem(IDC_STATIC_TIPS_TEXT, STXANCHOR_BOTTOM | STXANCHOR_LEFT);

	return 0;
}

LRESULT CMainContainerWindow::OnNewScriptWindow(UINT, WPARAM, LPARAM, BOOL&)
{
	CreateScriptWindow(_tree.GetSelectedItem());
	return 0;
}

LRESULT CMainContainerWindow::OnAppendReceivedMessage(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	auto dlgBase = _masterDialogs.find(_T("_MessageReceiveWindow"));
	assert(dlgBase != _masterDialogs.end());
	
	auto dlg = (CSTXReceivedMessageDialog*)(dlgBase->second.get());
	assert(dlg != nullptr);

	dlg->AppendReceivedMessage((LPCTSTR)wParam);

	return 0;
}

LRESULT CMainContainerWindow::OnUpdateReceivedMessageCount(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	CString text;
	if (wParam == 0)
		text = _T("Received Message");
	else
		text.Format(_T("Received Message [%d]"), wParam);

	_tree.Internal_SetItemText(_nodeMessageReceive, text);
	return 0;
}

LRESULT CMainContainerWindow::OnSwitchToProtocolTest(UINT, WPARAM, LPARAM, BOOL&)
{
	_tree.Internal_SelectItem(_nodeProtocolTest);
	return 0;
}

LRESULT CMainContainerWindow::OnTreeSelectedItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);

	if (pNM->hNode == _nodeProtocolTest)
	{
		CreateProtocolTestWindow();
	}
	else if (pNM->hNode == _nodeMessageReceive)
	{
		CreateMessageReceiveWindow(TRUE);
	}
	else if (_tree.Internal_GetParentItem(pNM->hNode) == _nodeServerScriptParent)
	{
		ShowScriptWindow(pNM->hNode);
	}
	else if (pNM->hNode == _nodeServerData)
	{
		CreateServerDataWindow();
	}
	return 0;
}

LRESULT CMainContainerWindow::OnTreeItemClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);

	if (GetKeyState(VK_CONTROL) & 0xFF00)
	{
		if (_tree.Internal_GetParentItem(pNM->hNode) == _nodeServerScriptParent)
		{
			std::wstring windowKey = _T("Server Script Window ");
			TCHAR szNumber[16];
			_stprintf_s(szNumber, _T(" %d"), pNM->dwItemData);
			windowKey += szNumber;

			auto it = _masterDialogs.find(windowKey);
			if (it != _masterDialogs.end())
			{
				it->second->DestroyWindow();
				_masterDialogs.erase(it);
			}

			_tree.Internal_DeleteItem(pNM->hNode);
		}
	}

	return 0;
}

LRESULT CMainContainerWindow::OnTreeItemDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);

	if (_tree.Internal_GetParentItem(pNM->hNode) == _nodeServerScriptParent)
	{
		CreateScriptWindow(pNM->hNode);
	}

	return 0;
}

void CMainContainerWindow::CreateMasterTree()
{
	auto treePlaceholder = GetDlgItem(IDC_STATIC_TREE);
	RECT rcTree;
	treePlaceholder.GetWindowRect(&rcTree);
	this->ScreenToClient(&rcTree);

	CSTXAnimatedTreeCtrlNS::RegisterAnimatedTreeCtrlClass();
	_tree.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | STXTVS_NO_EXPANDER_FADE, rcTree.left, rcTree.top, rcTree.right - rcTree.left, rcTree.bottom - rcTree.top, m_hWnd, IDC_MASTER_TREE);
	_tree.Internal_SetAnimationDuration(400);
	_anchor->AddItem(IDC_MASTER_TREE, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_BOTTOM);
}

void CMainContainerWindow::InitializeTreeItems()
{
	auto nodeMessage = _tree.Internal_InsertItem(_T("Message"));
	_nodeProtocolTest = _tree.Internal_InsertItem(_T("Protocol Test"), nodeMessage);
	CComPtr<IStream> spMessageImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_MESSAGE_32), _T("PNG"));
	_tree.SetItemImage(_nodeProtocolTest, spMessageImage, TRUE);
	_nodeMessageReceive = _tree.Internal_InsertItem(_T("Message Received"), nodeMessage);
	CComPtr<IStream> spReceivedMessageImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_MESSAGE_RECV_32), _T("PNG"));
	_tree.SetItemImage(_nodeMessageReceive, spReceivedMessageImage, TRUE);

	auto nodeScripts = _tree.Internal_InsertItem(_T("Scripts"));
	auto scriptChildNode = _tree.Internal_InsertItem(_T("Server Scripts"), nodeScripts);
	_nodeServerScriptParent = nodeScripts;
	CComPtr<IStream> spScriptImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_LUA_32), _T("PNG"));
	_tree.SetItemImage(scriptChildNode, spScriptImage, TRUE);

	auto nodeData = _tree.Internal_InsertItem(_T("Data"));
	_nodeServerData = _tree.Internal_InsertItem(_T("Server Data Viewer"), nodeData);
	CComPtr<IStream> spTreeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_TREE_32), _T("PNG"));
	_tree.SetItemImage(_nodeServerData, spTreeImage, TRUE);
}

void CMainContainerWindow::CreateServerDataWindow()
{
	if (_currentMasterDialog && _currentMasterDialog->IsWindow())
	{
		_currentMasterDialog->ShowWindow(SW_HIDE);
	}

	LPCTSTR windowKey = _T("_ServerDataWindow");

	auto contentPlaceholder = GetDlgItem(IDC_STATIC_CONTENT);
	RECT rcContent;
	contentPlaceholder.GetWindowRect(&rcContent);
	this->ScreenToClient(&rcContent);

	auto it = _masterDialogs.find(windowKey);
	if (it != _masterDialogs.end())
	{
		_currentMasterDialog = it->second;
		_currentMasterDialog->ShowWindow(SW_SHOW);
		return;
	}

	auto dlg = std::make_shared<CSTXServerDataDialog>();
	_currentMasterDialog = dlg;

	dlg->Create(m_hWnd, rcContent);
	dlg->SetWindowText(_T("Server Data Viewer"));
	dlg->MoveWindow(&rcContent);
	dlg->ShowWindow(SW_SHOW);

	_masterDialogs[windowKey] = dlg;

	_anchor->AddItem(dlg->m_hWnd, STXANCHOR_ALL);
}

void CMainContainerWindow::ShowScriptWindow(HSTXTREENODE currentNode)
{
	int targetWindowId = 0;
	if (currentNode)
	{
		targetWindowId = (int)_tree.Internal_GetItemData(currentNode);
	}
	else
	{
		return;
	}

	if (_currentMasterDialog && _currentMasterDialog->IsWindow())
	{
		_currentMasterDialog->ShowWindow(SW_HIDE);
	}

	std::wstring windowKey = _T("Server Script Window ");
	TCHAR szNumber[16];
	_stprintf_s(szNumber, _T(" %d"), targetWindowId);
	windowKey += szNumber;

	auto contentPlaceholder = GetDlgItem(IDC_STATIC_CONTENT);
	RECT rcContent;
	contentPlaceholder.GetWindowRect(&rcContent);
	this->ScreenToClient(&rcContent);

	auto it = _masterDialogs.find(windowKey);
	if (it != _masterDialogs.end())
	{
		_currentMasterDialog = it->second;
		_currentMasterDialog->ShowWindow(SW_SHOW);
		return;
	}
	else
	{
		CreateScriptWindow(currentNode, targetWindowId);
	}
}

void CMainContainerWindow::CreateProtocolTestWindow()
{
	if (_currentMasterDialog && _currentMasterDialog->IsWindow())
	{
		_currentMasterDialog->ShowWindow(SW_HIDE);
	}

	LPCTSTR windowKey = _T("_ProtocolTestWindow");

	auto contentPlaceholder = GetDlgItem(IDC_STATIC_CONTENT);
	RECT rcContent;
	contentPlaceholder.GetWindowRect(&rcContent);
	this->ScreenToClient(&rcContent);

	auto it = _masterDialogs.find(windowKey);
	if (it != _masterDialogs.end())
	{
		_currentMasterDialog = it->second;
		_currentMasterDialog->ShowWindow(SW_SHOW);
		return;
	}

	auto dlg = std::make_shared<CSTXProtocolDialog>();
	_currentMasterDialog = dlg;

	dlg->Create(m_hWnd, rcContent);
	dlg->SetWindowText(_T("Protocol Test"));
	dlg->MoveWindow(&rcContent);
	dlg->ShowWindow(SW_SHOW);

	_masterDialogs[windowKey] = dlg;

	_anchor->AddItem(dlg->m_hWnd, STXANCHOR_ALL);

	CreateMessageReceiveWindow(FALSE);
}

void CMainContainerWindow::CreateMessageReceiveWindow(BOOL activate)
{
	CreateStandardContainedWindow<CSTXReceivedMessageDialog>(_T("_MessageReceiveWindow"), _T("Received Message"), activate);
	if (activate)
	{
		auto dlgBase = _masterDialogs.find(_T("_MessageReceiveWindow"));
		assert(dlgBase != _masterDialogs.end());

		auto dlg = (CSTXReceivedMessageDialog*)(dlgBase->second.get());
		assert(dlg != nullptr);

		dlg->ClearMessageCount();
		dlg->UpdateNodeText();
	}
}

void CMainContainerWindow::CreateScriptWindow(HSTXTREENODE currentNode, int windowId)
{
	static int windowIdBase = 0;

	int targetWindowId = windowId;
	if (windowId < 0)
		targetWindowId = ++windowIdBase;

	if (_currentMasterDialog && _currentMasterDialog->IsWindow())
	{
		_currentMasterDialog->ShowWindow(SW_HIDE);
	}

	std::wstring windowKey = _T("Server Script Window ");
	TCHAR szNumber[16];
	_stprintf_s(szNumber, _T(" %d"), targetWindowId);
	windowKey += szNumber;

	auto contentPlaceholder = GetDlgItem(IDC_STATIC_CONTENT);
	RECT rcContent;
	contentPlaceholder.GetWindowRect(&rcContent);
	this->ScreenToClient(&rcContent);

	auto dlg = std::make_shared<CSTXScriptDialog>();
	_currentMasterDialog = dlg;

	dlg->Create(m_hWnd, rcContent);
	dlg->SetWindowText(windowKey.c_str());
	dlg->MoveWindow(&rcContent);
	dlg->ShowWindow(SW_SHOW);

	_masterDialogs[windowKey] = dlg;

	if (targetWindowId > 0)
	{
		std::wstring itemText = _T("Server Scripts");
		itemText += szNumber;

		if (windowId < 0)
		{
			auto treeNode = _tree.Internal_InsertItem(itemText.c_str(), _nodeServerScriptParent);
			CComPtr<IStream> spScriptImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_LUA_32), _T("PNG"));
			_tree.SetItemImage(treeNode, spScriptImage, TRUE);
			_tree.Internal_SetItemData(treeNode, targetWindowId);
			_tree.Internal_SelectItem(treeNode);
		}
		else
		{
			_tree.Internal_SetItemData(currentNode, targetWindowId);
		}
	}

	_anchor->AddItem(dlg->m_hWnd, STXANCHOR_ALL);
}
