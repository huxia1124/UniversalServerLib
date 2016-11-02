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
#include "STXServerDataDialog.h"
#include <atldlgs.h>
#include <atlsafe.h>

#include "../UniversalServerLib/UniversalServerRPC_h.h"
#include <iterator>
#include "Common.h"

#pragma comment(lib,"rpcrt4")
#pragma comment(lib,"ole32")

void* __RPC_USER midl_user_allocate(size_t size);
void __RPC_USER midl_user_free(void* p);

CSTXServerDataDialog::CSTXServerDataDialog()
{
	_anchor = NULL;
}


CSTXServerDataDialog::~CSTXServerDataDialog()
{
}

void CSTXServerDataDialog::InitializeRPCHostCombobox()
{
	int index = -1;
	_cbRPCHost.ResetContent();
	index = _cbRPCHost.AddString(_T("localhost:4047"));
	index = _cbRPCHost.AddString(_T("127.0.0.1:137"));
	index = _cbRPCHost.AddString(_T("localhost:3399"));
	_cbRPCHost.SetCurSel(index);
}

LRESULT CSTXServerDataDialog::OnRefreshClicked(WORD, UINT, HWND, BOOL&)
{
	::EnableWindow(GetDlgItem(IDC_BUTTON_REFRESH), FALSE);
	UpdateWindow();

	int nLenHost = _cbRPCHost.GetWindowTextLength();
	TCHAR *pTextHost = new TCHAR[nLenHost + 2];
	pTextHost[nLenHost + 1] = 0;
	_cbRPCHost.GetWindowText(pTextHost, nLenHost + 1);

	TCHAR *pszSep = _tcschr(pTextHost, _T(':'));
	if (pszSep == NULL)
	{
		_host = pTextHost;
		_port = _T("4747");
	}
	else
	{
		*pszSep = 0;
		_host = pTextHost;
		_port = pszSep + 1;
	}

	delete[]pTextHost;

	auto selectedTreeNode = _tree.GetSelectedItem();
	CString selectedTreeNodePath = GetSelectedItemFullPath();
	if (selectedTreeNode == NULL)
	{
		selectedTreeNode = STXTVI_ROOT;
	}
	else
	{
		RefreshNodeText(selectedTreeNode);
		CreateDataEditor(selectedTreeNode);
	}

	//_tree.Internal_DeleteItem(selectedTreeNode);

	DeleteAllChildNodes(selectedTreeNode);

	std::vector<std::wstring> nodeNames;
	std::vector<int> nodeTypes;
	std::vector<unsigned long> nodeFlags;
	CString error;
	GetSharedDataTreeNodes(selectedTreeNodePath, &nodeNames, &nodeTypes, &nodeFlags, error);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING | MB_OK);
	}
	else
	{
		size_t count = nodeNames.size();
		for (size_t i = 0; i < count; i++)
		{
			TreeNodeData *itemData = new TreeNodeData();
			itemData->dataType = nodeTypes[i];
			itemData->flags = nodeFlags[i];
			itemData->nodeName = nodeNames[i];
			itemData->nodeFullPathName = (LPCTSTR)CombineNodePath(GetTreeNodeFullPath(selectedTreeNode), nodeNames[i].c_str());
			auto treeNode = _tree.Internal_InsertItem(GenerateTreeNodeText(nodeNames[i].c_str(), itemData, count), selectedTreeNode, STXTVI_LAST);
			_tree.Internal_SetItemData(treeNode, (DWORD_PTR)itemData);
			itemData->nodeFullPathName = GetTreeNodeFullPath(treeNode);
			SetTreeNodeAppearance(treeNode, itemData);
			_tree.Internal_Expand(treeNode, STXATVE_COLLAPSE);
			
			if(itemData->flags & 0x40000000)	//Not empty, children exist
				_tree.Internal_ModifyItemStyle(treeNode, STXTVIS_FORCE_SHOW_EXPANDER, 0);
		}
	}


	::EnableWindow(GetDlgItem(IDC_BUTTON_REFRESH), TRUE);

	return 0;
}


LRESULT CSTXServerDataDialog::OnSaveDataClicked(WORD, UINT, HWND, BOOL&)
{
	auto selectedNode = _tree.GetSelectedItem();
	if (selectedNode == NULL)
		return 0;

	TreeNodeData *nodeData = (TreeNodeData*)_tree.Internal_GetItemData(selectedNode);

	CString scriptResult;
	CString error;
	CString scriptText;
	CString fullPath;
	CString inputText;

	GetDlgItemText(IDC_EDIT_DATA, inputText);

	// 1: int32
	// 2: int64
	// 3: wstring
	// 4: int (c++ standard int)
	// 5: float (c++ standard float)
	// 6: double (c++ standard double)
	// 7: WORD
	// 8: DWORD
	// 9: vector<wstring>
	// 10: set<wstring>
	// 11: vector<int64_t>
	// 12: set<int64_t>
	// 13: vector<double>
	// 14: set<double>
	// 15: IntFunction
	// 16: WstringFunction
	// 30000: Custom value

	switch (nodeData->dataType)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:		/*STXVariableTreeNodeType_DWord*/
	case 15:		/*STXVariableTreeNodeType_IntFunction*/
	case 16:		/*STXVariableTreeNodeType_WStringFunction*/
		fullPath = nodeData->nodeFullPathName.c_str();
		fullPath.Replace(_T("\\"), _T("\\\\"));
		inputText.Replace(_T("\\"), _T("\\\\"));
		inputText.Replace(_T("\r\n"), _T("\\n"));
		scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:SetStringValue(\"%s\", \"%s\")"), (LPCTSTR)fullPath, (LPCTSTR)inputText);
		RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);
		break;
	}

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"));
	}

	RefreshNodeText(selectedNode);

	return 0;
}

LRESULT CSTXServerDataDialog::OnUnregisterClicked(WORD, UINT, HWND, BOOL&)
{
	auto selectedNode = _tree.GetSelectedItem();
	if (selectedNode == NULL)
		return 0;

	TreeNodeData *nodeData = (TreeNodeData*)_tree.Internal_GetItemData(selectedNode);
	CString fullPath = nodeData->nodeFullPathName.c_str();

	if (MessageBox(_T("Warning: Once a node is unregistered, all of its children will also be deleted!\n\nYou are going to unregister the following node:\n") + fullPath
		, _T("Warning"), MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3) != IDYES)
	{
		return 0;
	}

	fullPath.Replace(_T("\\"), _T("\\\\"));

	CString scriptText;
	CString scriptResult;
	CString error;
	scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:UnregisterVariable(\"%s\")"), (LPCTSTR)fullPath);
	RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"));
	}
	else
	{
		_tree.Internal_DeleteItem(selectedNode);
	}
	return 0;
}

LRESULT CSTXServerDataDialog::OnSortClicked(WORD, UINT, HWND, BOOL&)
{
	auto selectedNode = _tree.GetSelectedItem();

	static int iFactor = 1;
	_tree.Internal_SetAnimationDuration(400);
	_tree.Internal_SortChildrenByItem(selectedNode, [&](HSTXTREENODE hItem1, HSTXTREENODE hItem2, LPARAM lParamSort) {
		TCHAR szBuf1[1024];
		TCHAR szBuf2[1024];
		_tree.Internal_GetItemText(hItem1, szBuf1, 1024);
		_tree.Internal_GetItemText(hItem2, szBuf2, 1024);
		return iFactor * _tcscmp(szBuf1, szBuf2);
	}, 0);
	_tree.Internal_SetAnimationDuration(200);

	iFactor = iFactor * (-1);

	return 0;
}

LRESULT CSTXServerDataDialog::OnSortAsNumbersClicked(WORD, UINT, HWND, BOOL&)
{
	auto selectedNode = _tree.GetSelectedItem();

	static int iFactor = 1;
	_tree.Internal_SetAnimationDuration(400);
	_tree.Internal_SortChildrenByItem(selectedNode, [&](HSTXTREENODE hItem1, HSTXTREENODE hItem2, LPARAM lParamSort) {
		TCHAR szBuf1[1024];
		TCHAR szBuf2[1024];
		_tree.Internal_GetItemText(hItem1, szBuf1, 1024);
		_tree.Internal_GetItemText(hItem2, szBuf2, 1024);
		int result = 0;
		auto v1 = _tcstod(szBuf1, nullptr);
		auto v2 = _tcstod(szBuf2, nullptr);
		if (v1 < v2)	result = -1;
		else if (v1 > v2)	result = 1;
		return iFactor * result;
	}, 0);
	_tree.Internal_SetAnimationDuration(200);

	iFactor = iFactor * (-1);

	return 0;
}

LRESULT CSTXServerDataDialog::OnTreeItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);

	TreeNodeData *existingItemData = (TreeNodeData*)_tree.Internal_GetItemData(pNM->hNode);
	if (existingItemData->status)		//if status is not zero, it has been expanded and data retrieved. no need to retrieve again
	{
		return 0;
	}


	CString fullPath;
	HSTXTREENODE currentNode = pNM->hNode;
	while (currentNode != STXTVI_ROOT)
	{
		int size = _tree.Internal_GetItemText(currentNode, nullptr, 0);
		CString strNodeText;
		_tree.Internal_GetItemText(currentNode, strNodeText.GetBufferSetLength(size + 1), size + 1);

		fullPath.Insert(0, strNodeText);
		fullPath.Insert(0, _T("\\"));

		currentNode = _tree.Internal_GetParentItem(currentNode);
	}
	fullPath.TrimLeft(_T('\\'));

	std::vector<std::wstring> nodeNames;
	std::vector<int> nodeTypes;
	std::vector<unsigned long> nodeFlags;
	CString error;
	GetSharedDataTreeNodes(fullPath, &nodeNames, &nodeTypes, &nodeFlags, error);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING | MB_OK);
	}
	else
	{
		size_t count = nodeNames.size();
		for (size_t i = 0; i < count; i++)
		{
			TreeNodeData *itemData = new TreeNodeData();
			itemData->dataType = nodeTypes[i];
			itemData->flags = nodeFlags[i];
			itemData->nodeName = nodeNames[i];
			itemData->nodeFullPathName = (LPCTSTR)CombineNodePath(GetTreeNodeFullPath(pNM->hNode), nodeNames[i].c_str());
			auto treeNode = _tree.Internal_InsertItem(GenerateTreeNodeText(nodeNames[i].c_str(), itemData, count), pNM->hNode, STXTVI_LAST);
			_tree.Internal_SetItemData(treeNode, (DWORD_PTR)itemData);
			SetTreeNodeAppearance(treeNode, itemData);
			_tree.Internal_Expand(treeNode, STXATVE_COLLAPSE);

			if (itemData->flags & 0x40000000)	//Not empty, children exist
				_tree.Internal_ModifyItemStyle(treeNode, STXTVIS_FORCE_SHOW_EXPANDER, 0);
		}
	}

	existingItemData->status = 1;
	_tree.Internal_ModifyItemStyle(pNM->hNode, 0, STXTVIS_FORCE_SHOW_EXPANDER);

	return 0;
}

LRESULT CSTXServerDataDialog::OnTreeItemPostDelete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);
	
	delete (TreeNodeData*)pNM->dwItemData;

	return 0;
}

LRESULT CSTXServerDataDialog::OnTreeSelectedItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPSTXATVNITEM pNM = reinterpret_cast<LPSTXATVNITEM>(pnmh);

	if (_hWndCurrentEditor)
	{
		_anchor->DeleteItem(_hWndCurrentEditor);
		::DestroyWindow(_hWndCurrentEditor);
		_hWndCurrentEditor = NULL;
	}

	if (pNM->hNode)
	{
		CreateDataEditor(pNM->hNode);
	}
	UpdateNodeFullPathIndicator(pNM->hNode);
	return 0;
}

LRESULT CSTXServerDataDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	//CenterWindow();

	_anchor = new CSTXAnchor(m_hWnd);

	_cbRPCHost.Attach(GetDlgItem(IDC_COMBO_RPC_HOST));
	InitializeRPCHostCombobox();

	CreateDataTree();

	_anchor->AddItem(IDC_BUTTON_SAVE_DATA, STXANCHOR_RIGHT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_STATIC_DATA, STXANCHOR_ALL);
	_anchor->AddItem(IDC_EDIT_FULL_PATH, STXANCHOR_LEFT | STXANCHOR_RIGHT | STXANCHOR_BOTTOM);
	_anchor->AddItem(IDC_BUTTON_UNREGISTER, STXANCHOR_LEFT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_STATIC_TITLE, STXANCHOR_LEFT | STXANCHOR_RIGHT | STXANCHOR_TOP);

	return 1; // Let dialog manager set initial focus
}


LRESULT CSTXServerDataDialog::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	PAINTSTRUCT ps;
	BeginPaint(&ps);

	RECT rcTitleBar;
	GetDlgItem(IDC_STATIC_TITLE).GetWindowRect(&rcTitleBar);
	ScreenToClient(&rcTitleBar);
	rcTitleBar.right = rcClient.right;			//Fix the minimize/restore drawing problem
	CString titleStr;
	GetWindowText(titleStr);

	DrawTitleBar(ps.hdc, titleStr, rcTitleBar);

	EndPaint(&ps);
	return 0;
}

LRESULT CSTXServerDataDialog::OnAddStringData(UINT, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	CString fullPath = GetSelectedItemFullPath();
	fullPath.Replace(_T("\\"), _T("\\\\"));
	CString scriptText;
	CString scriptResult;
	CString error;
	scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:AddStringValue(\"%s\", \"%s\")"), (LPCTSTR)fullPath, (LPCTSTR)wParam);
	RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);

	handled = TRUE;
	return !error.IsEmpty();
}

LRESULT CSTXServerDataDialog::OnRemoveStringData(UINT, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	CString fullPath = GetSelectedItemFullPath();
	fullPath.Replace(_T("\\"), _T("\\\\"));
	CString scriptText;
	CString scriptResult;
	CString error;
	scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:RemoveStringValue(\"%s\", \"%s\")"), (LPCTSTR)fullPath, (LPCTSTR)wParam);
	RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);

	handled = TRUE;
	return !error.IsEmpty();
}

void CSTXServerDataDialog::GetErrorText(RPC_STATUS NTStatusMessage, CString &err)
{
	LPVOID lpMessageBuffer;
	HMODULE Hand = LoadLibraryA("NTDLL.DLL");

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_FROM_HMODULE,
		Hand,
		NTStatusMessage,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMessageBuffer,
		0,
		NULL);

	// Now display the string.

	// Free the buffer allocated by the system.
	err = (LPCTSTR)lpMessageBuffer;
	LocalFree(lpMessageBuffer);
	FreeLibrary(Hand);
}

void CSTXServerDataDialog::CreateDataTree()
{
	auto treePlaceholder = GetDlgItem(IDC_STATIC_DATA_TREE);
	RECT rcTree;
	treePlaceholder.GetWindowRect(&rcTree);
	this->ScreenToClient(&rcTree);

	CSTXAnimatedTreeCtrlNS::RegisterAnimatedTreeCtrlClass();
	_tree.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | STXTVS_NO_EXPANDER_FADE, rcTree.left, rcTree.top, rcTree.right - rcTree.left, rcTree.bottom - rcTree.top, m_hWnd, IDC_MASTER_TREE);
	_tree.Internal_SetAnimationDuration(200);
	_anchor->AddItem(IDC_MASTER_TREE, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_BOTTOM);
}


void CSTXServerDataDialog::CreateDataEditor(HSTXTREENODE treeNode)
{
	TreeNodeData *existingItemData = (TreeNodeData*)_tree.Internal_GetItemData(treeNode);
	int dataType = existingItemData->dataType;

	if (existingItemData->flags & 0x00000001 || IsCollectionType(dataType) || dataType < 0)		//Read-Only or CollectionType or InvalidType(no data)
		::EnableWindow(GetDlgItem(IDC_BUTTON_SAVE_DATA), FALSE);
	else
		::EnableWindow(GetDlgItem(IDC_BUTTON_SAVE_DATA), TRUE);

	// 1: int32
	// 2: int64
	// 3: wstring
	// 4: int (c++ standard int)
	// 5: float (c++ standard float)
	// 6: double (c++ standard double)
	// 7: WORD
	// 8: DWORD
	// 9: vector<wstring>
	// 10: set<wstring>
	// 11: vector<int64_t>
	// 12: set<int64_t>
	// 13: vector<double>
	// 14: set<double>
	// 15: IntFunction
	// 16: WstringFunction
	// 30000: Custom value

	auto dataPlaceholder = GetDlgItem(IDC_STATIC_DATA);
	RECT rcData;
	dataPlaceholder.GetWindowRect(&rcData);
	this->ScreenToClient(&rcData);

	if (_hWndCurrentEditor)
	{
		_anchor->DeleteItem(_hWndCurrentEditor);
		::DestroyWindow(_hWndCurrentEditor);
		_hWndCurrentEditor = NULL;
	}
	HWND hWndEditor = NULL;

	CString editorValue;
	CString error;
	GetSharedDataTreeNodeStringValue(GetSelectedItemFullPath(), editorValue, error);
	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_OK | MB_ICONERROR);
		return;
	}
	editorValue.Replace(_T("\n"), _T("\r\n"));

	DWORD wsExtra = 0;

	switch (dataType)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 15:
	case 16:
		if (existingItemData->flags & 0x00000001)		//Read-Only
		{
			wsExtra |= ES_READONLY;
		}
		hWndEditor = CreateWindow(_T("edit"), editorValue
			, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | wsExtra
			, rcData.left, rcData.top, rcData.right - rcData.left, rcData.bottom - rcData.top, m_hWnd, (HMENU)IDC_EDIT_DATA, NULL, 0);
		::SendMessage(hWndEditor, WM_SETFONT, SendMessage(WM_GETFONT), 0);
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	{
		_collectionEditorPanel.Create(m_hWnd);
		_collectionEditorPanel.MoveWindow(&rcData);
		_collectionEditorPanel.ShowWindow(SW_SHOW);
		hWndEditor = _collectionEditorPanel.m_hWnd;
		std::vector<std::wstring> collectionValues;
		GetSharedDataTreeNodeValues(GetSelectedItemFullPath(), collectionValues, error);
		_collectionEditorPanel.FillCollectionValues(collectionValues, dataType);
	}
		break;
	}

	_hWndCurrentEditor = hWndEditor;
	if (hWndEditor)
	{
		_anchor->AddItem(hWndEditor, STXANCHOR_ALL);
	}
}

CString CSTXServerDataDialog::GetSelectedItemFullPath()
{
	return GetTreeNodeFullPath(_tree.GetSelectedItem());
}

CString CSTXServerDataDialog::GetTreeNodeFullPath(HSTXTREENODE treeNode)
{
	CString fullPath;
	HSTXTREENODE currentNode = treeNode;
	if (currentNode == NULL)
		return _T("");
	while (currentNode != STXTVI_ROOT)
	{
		TreeNodeData *nodeData = (TreeNodeData*)_tree.Internal_GetItemData(currentNode);

		fullPath.Insert(0, nodeData->nodeName.c_str());
		fullPath.Insert(0, _T("\\"));

		currentNode = _tree.Internal_GetParentItem(currentNode);
	}
	fullPath.TrimLeft(_T('\\'));
	return fullPath;
}

void CSTXServerDataDialog::DeleteAllChildNodes(HSTXTREENODE parentNode)
{
	auto child = _tree.Internal_GetNextItem(parentNode, STXATVGN_CHILD);

	while (child)
	{
		auto tmp = _tree.Internal_GetNextSiblingItem(child);
		_tree.Internal_DeleteItem(child);
		child = tmp;
	}
}

void CSTXServerDataDialog::SetTreeNodeAppearance(HSTXTREENODE treeNode, TreeNodeData *nodeData)
{
	// 1: int32
	// 2: int64
	// 3: wstring
	// 4: int (c++ standard int)
	// 5: float (c++ standard float)
	// 6: double (c++ standard double)
	// 7: WORD
	// 8: DWORD
	// 9: vector<wstring>
	// 10: set<wstring>
	// 11: vector<int64_t>
	// 12: set<int64_t>
	// 13: vector<double>
	// 14: set<double>
	// 15: IntFunction
	// 16: WstringFunction
	// 30000: Custom value

	CComPtr<IStream> spNodeImage;

	switch (nodeData->dataType)
	{
	case 3:
	case 16:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_STRING_32), _T("PNG")); break;
	case 5:
	case 6:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_FLOAT_32), _T("PNG")); break;
	case 1:
	case 2:
	case 4:
	case 7:
	case 8:
	case 15:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_INTEGER_32), _T("PNG")); break;
	case 9:
	case 10:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_STRING_COLLECTION_32), _T("PNG")); break;
	case 11:
	case 12:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_INTEGER_COLLECTION_32), _T("PNG")); break;
	case 13:
	case 14:
		spNodeImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_FLOAT_COLLECTION_32), _T("PNG")); break;
	}

	if (spNodeImage)
	{
		_tree.Internal_SetItemHeight(treeNode, 20);
		_tree.SetItemImage(treeNode, spNodeImage, TRUE);
	}

	if (nodeData->flags & 0x00000001)		//Read-Only
	{
		CComPtr<IStream> spReadOnlyImage = LoadImageFromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG_READONLY_16), _T("PNG"));
		_tree.SetItemSubImage(treeNode, spReadOnlyImage);
	}
}

CString CSTXServerDataDialog::GenerateTreeNodeText(LPCTSTR lpszOriginalName, TreeNodeData *nodeData, size_t siblingNodeCount)
{
	if (siblingNodeCount > 100)		//Avoid long-time waiting. if too many nodes, we don't retrieve the node data at this time
		return lpszOriginalName;

	CString nodeName = lpszOriginalName;
	CString scriptResult;
	CString error;
	CString scriptText;
	CString fullPath;
	switch (nodeData->dataType)
	{
	case 3:		//wstring
	case 16:		/*STXVariableTreeNodeType_WStringFunction*/
		fullPath = nodeData->nodeFullPathName.c_str();
		fullPath.Replace(_T("\\"), _T("\\\\"));
		scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:GetStringValue(\"%s\")"), (LPCTSTR)fullPath);
		RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);
		nodeName += _T(" [");
		nodeName += scriptResult;
		nodeName += _T("]");
		break;
	case 5:		//float
	case 6:		//double
		fullPath = nodeData->nodeFullPathName.c_str();
		fullPath.Replace(_T("\\"), _T("\\\\"));
		scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:GetDoubleValue(\"%s\")"), (LPCTSTR)fullPath);
		RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);
		nodeName += _T(" [");
		nodeName += scriptResult;
		nodeName += _T("]");
		break;
	case 1:
	case 2:
	case 4:
	case 7:
	case 8:		/*STXVariableTreeNodeType_DWord*/
	case 15:		/*STXVariableTreeNodeType_IntFunction*/
		fullPath = nodeData->nodeFullPathName.c_str();
		fullPath.Replace(_T("\\"), _T("\\\\"));
		scriptText.Format(_T("local ____sharedDataObj = SharedData()\r\nresult=____sharedDataObj:GetIntegerValue(\"%s\")"), (LPCTSTR)fullPath);
		RunServerScriptString((LPCTSTR)scriptText, scriptResult, error);
		nodeName += _T(" [");
		nodeName += scriptResult;
		nodeName += _T("]");
		break;
	}
	return nodeName;
}

CString CSTXServerDataDialog::CombineNodePath(LPCTSTR pathLeft, LPCTSTR pathRight)
{
	if (pathLeft == 0 || pathLeft[0] == 0)
		return pathRight;

	if (pathRight == 0 || pathRight[0] == 0)
		return pathLeft;

	return CString(pathLeft) + _T("\\") + CString(pathRight);
}

void ExtractSafeArrayBSTR(SAFEARRAY *psa, std::vector<std::wstring>* pNodeNames)
{
	ATL::CComSafeArray<BSTR> sa;
	sa.Attach(psa);

	auto d = sa.GetDimensions();
	auto l = sa.GetLowerBound();
	auto u = sa.GetUpperBound();

	for (auto i = l; i <= u; i++)
	{
		auto v = sa.GetAt(i);
		pNodeNames->push_back((LPCTSTR)v);
	}

	//sa.Detach();
}

void ExtractSafeArrayInt(SAFEARRAY *psa, std::vector<int>* pNodeTypes)
{
	ATL::CComSafeArray<int> sa;
	sa.Attach(psa);

	auto d = sa.GetDimensions();
	auto l = sa.GetLowerBound();
	auto u = sa.GetUpperBound();

	for (auto i = l; i <= u; i++)
	{
		auto v = sa.GetAt(i);
		pNodeTypes->push_back(v);
	}

	//sa.Detach();
}

void ExtractSafeArrayULong(SAFEARRAY *psa, std::vector<unsigned long>* pNodeFlags)
{
	ATL::CComSafeArray<unsigned long> sa;
	sa.Attach(psa);

	auto d = sa.GetDimensions();
	auto l = sa.GetLowerBound();
	auto u = sa.GetUpperBound();

	for (auto i = l; i <= u; i++)
	{
		auto v = sa.GetAt(i);
		pNodeFlags->push_back(v);
	}

	//sa.Detach();
}

void CSTXServerDataDialog::GetSharedDataTreeNodes(LPCTSTR lpszPath, std::vector<std::wstring>* pNodeNames, std::vector<int>* pNodeTypes, std::vector<unsigned long>* pNodeFlags, CString &err)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")),
		(RPC_WSTR)(_host.c_str()),
		(RPC_WSTR)(_port.c_str()),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status)
	{
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
	SAFEARRAY *psa = NULL;
	SAFEARRAY *psaTypes = NULL;
	SAFEARRAY *psaFlags = NULL;
	::GetSharedDataTreeNodes(hwBinding, lpszPath, &psa, &psaTypes, &psaFlags);
	if (psa)
	{
		ExtractSafeArrayBSTR(psa, pNodeNames);
		ExtractSafeArrayInt(psaTypes, pNodeTypes);
		ExtractSafeArrayULong(psaFlags, pNodeFlags);
		//SafeArrayDestroy(psa);
	}
	}
		RpcExcept(1)
	{
		err.Format(_T("Runtime reported exception: %d, except=%d\n"), GetLastError(), RpcExceptionCode());
		GetErrorText(RpcExceptionCode(), err);
		printf("Runtime reported exception: %d, except=%d\n", GetLastError(), RpcExceptionCode());
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

		if (status)
		{
			printf("Bind free failed\n");
			exit(status);
		}
}

void CSTXServerDataDialog::GetSharedDataTreeNodeStringValue(LPCTSTR lpszPath, CString &value, CString &err)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")),
		(RPC_WSTR)(_host.c_str()),
		(RPC_WSTR)(_port.c_str()),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status)
	{
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		BSTR pstrValue = NULL;
	::GetSharedDataTreeNodeStringValue(hwBinding, lpszPath, &pstrValue);
	if (pstrValue)
	{
		value = pstrValue;
		SysFreeString(pstrValue);
	}
	}
		RpcExcept(1)
	{
		err.Format(_T("Runtime reported exception: %d, except=%d\n"), GetLastError(), RpcExceptionCode());
		GetErrorText(RpcExceptionCode(), err);
		printf("Runtime reported exception: %d, except=%d\n", GetLastError(), RpcExceptionCode());
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

		if (status)
		{
			printf("Bind free failed\n");
			exit(status);
		}
}

void CSTXServerDataDialog::RunServerScriptString(LPCTSTR lpszScript, CString &result, CString &err)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")),
		(RPC_WSTR)(_host.c_str()),
		(RPC_WSTR)(_port.c_str()),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status)
	{
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		BSTR pStrResult = NULL;
	RunScriptString(hwBinding, lpszScript, &pStrResult);
	if (pStrResult)
	{
		result = pStrResult;
		SysFreeString(pStrResult);
	}
	}
		RpcExcept(1)
	{
		err.Format(_T("Runtime reported exception: %d, except=%d\n"), GetLastError(), RpcExceptionCode());
		GetErrorText(RpcExceptionCode(), err);
		printf("Runtime reported exception: %d, except=%d\n", GetLastError(), RpcExceptionCode());
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

		if (status)
		{
			printf("Bind free failed\n");
			exit(status);
		}
}

void CSTXServerDataDialog::GetSharedDataTreeNodeValues(LPCTSTR lpszPath, std::vector<std::wstring> &result, CString &err)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")),
		(RPC_WSTR)(_host.c_str()),
		(RPC_WSTR)(_port.c_str()),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status)
	{
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		SAFEARRAY *psa = NULL;
		::GetSharedDataTreeNodeValues(hwBinding, lpszPath, &psa);

		ExtractSafeArrayBSTR(psa, &result);
	}
		RpcExcept(1)
	{
		err.Format(_T("Runtime reported exception: %d, except=%d\n"), GetLastError(), RpcExceptionCode());
		GetErrorText(RpcExceptionCode(), err);
		printf("Runtime reported exception: %d, except=%d\n", GetLastError(), RpcExceptionCode());
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

		if (status)
		{
			printf("Bind free failed\n");
			exit(status);
		}
}

void CSTXServerDataDialog::RefreshNodeText(HSTXTREENODE treeNode)
{
	if (treeNode != NULL && treeNode != STXTVI_ROOT)
	{
		TreeNodeData *itemData = (TreeNodeData*)_tree.Internal_GetItemData(treeNode);
		_tree.Internal_SetItemText(treeNode, GenerateTreeNodeText(itemData->nodeName.c_str(), itemData, 0));
	}
}

void CSTXServerDataDialog::UpdateNodeFullPathIndicator(HSTXTREENODE treeNode)
{
	if (treeNode == NULL)
		SetDlgItemText(IDC_EDIT_FULL_PATH, _T(""));
	else
	{
		TreeNodeData *nodeData = (TreeNodeData*)_tree.Internal_GetItemData(treeNode);
		SetDlgItemText(IDC_EDIT_FULL_PATH, nodeData->nodeFullPathName.c_str());
	}
}

bool CSTXServerDataDialog::IsCollectionType(int dataType)
{
	return dataType >= 9 && dataType <= 14;
}
