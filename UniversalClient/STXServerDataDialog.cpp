#include "stdafx.h"
#include "STXServerDataDialog.h"
#include <atldlgs.h>
#include <atlsafe.h>

#include "../UniversalServerLib/UniversalServerRPC_h.h"
#include <iterator>

#pragma comment(lib,"rpcrt4")
#pragma comment(lib,"ole32")

void* __RPC_USER midl_user_allocate(size_t size);
void __RPC_USER midl_user_free(void* p);

struct TreeNodeData
{
	int status = 0;
	int dataType = 0;
};

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

	//_tree.Internal_DeleteItem(selectedTreeNode);

	DeleteAllChildNodes(selectedTreeNode);

	std::vector<std::wstring> nodeNames;
	std::vector<int> nodeTypes;
	CString error;
	GetSharedDataTreeNodes(selectedTreeNodePath, &nodeNames, &nodeTypes, error);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING | MB_OK);
	}
	else
	{
		size_t count = nodeNames.size();
		for (int i = 0; i < count; i++)
		{
			auto treeNode = _tree.Internal_InsertItem(nodeNames[i].c_str(), selectedTreeNode);
			TreeNodeData *itemData = new TreeNodeData();
			itemData->dataType = nodeTypes[i];
			_tree.Internal_SetItemData(treeNode, (DWORD_PTR)itemData);
			_tree.Internal_Expand(treeNode, STXATVE_COLLAPSE);
			_tree.Internal_ModifyItemStyle(treeNode, STXTVIS_FORCE_SHOW_EXPANDER, 0);
		}
	}


	::EnableWindow(GetDlgItem(IDC_BUTTON_REFRESH), TRUE);

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
	CString error;
	GetSharedDataTreeNodes(fullPath, &nodeNames, &nodeTypes, error);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING | MB_OK);
	}
	else
	{
		size_t count = nodeNames.size();
		for (int i = 0; i < count; i++)
		{
			auto treeNode = _tree.Internal_InsertItem(nodeNames[i].c_str(), pNM->hNode);
			TreeNodeData *itemData = new TreeNodeData();
			itemData->dataType = nodeTypes[i];
			_tree.Internal_SetItemData(treeNode, (DWORD_PTR)itemData);
			_tree.Internal_Expand(treeNode, STXATVE_COLLAPSE);
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
	if (pNM->hNode)
	{
		CreateDataEditor(pNM->hNode);
	}
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

	return 1; // Let dialog manager set initial focus
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
	_tree.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, rcTree.left, rcTree.top, rcTree.right - rcTree.left, rcTree.bottom - rcTree.top, m_hWnd, IDC_MASTER_TREE);
	_tree.Internal_SetAnimationDuration(100);
	_anchor->AddItem(IDC_MASTER_TREE, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_BOTTOM);
}


void CSTXServerDataDialog::CreateDataEditor(HSTXTREENODE treeNode)
{
	TreeNodeData *existingItemData = (TreeNodeData*)_tree.Internal_GetItemData(treeNode);
	int dataType = existingItemData->dataType;

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


	//switch (dataType)
	//{
	//case 1:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 2:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 3:
	//	editorValue=_T("");	break;
	//case 4:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 5:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 6:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 7:
	//	editorValue.Format(_T("%d"), 0);	break;
	//case 8:
	//	editorValue.Format(_T("%d"), 0);	break;
	//	break;
	//}


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
		hWndEditor = CreateWindow(_T("edit"), editorValue, WS_CHILD | WS_VISIBLE | WS_BORDER, rcData.left, rcData.top, rcData.right - rcData.left, rcData.bottom - rcData.top, m_hWnd, (HMENU)IDC_EDIT_DATA, NULL, 0);
		::SendMessage(hWndEditor, WM_SETFONT, SendMessage(WM_GETFONT), 0);
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
	CString fullPath;
	HSTXTREENODE currentNode = _tree.GetSelectedItem();
	if (currentNode == NULL)
		return _T("");
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

void CSTXServerDataDialog::GetSharedDataTreeNodes(LPCTSTR lpszPath, std::vector<std::wstring>* pNodeNames, std::vector<int>* pNodeTypes, CString &err)
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
	::GetSharedDataTreeNodes(hwBinding, lpszPath, &psa, &psaTypes);
	if (psa)
	{
		ExtractSafeArrayBSTR(psa, pNodeNames);
		ExtractSafeArrayInt(psaTypes, pNodeTypes);
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
