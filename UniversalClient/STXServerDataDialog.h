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
#include <atlapp.h>
#include <atlctrls.h>
#include <vector>
#include <string>

#include "Resource.h"
#include "STXAnchor.h"
#include "STXAnimatedTreeCtrlNS.h"
#include "STXCollectionEditorPanel.h"

#define WM_ADD_STRING_DATA		(WM_USER + 5)
#define WM_REMOVE_STRING_DATA	(WM_USER + 6)

struct TreeNodeData
{
	int status = 0;
	int dataType = 0;
	unsigned flags = 0;
	std::wstring nodeName;
	std::wstring nodeFullPathName;
	bool dirty = false;
};

class CSTXServerDataDialog : public CDialogImpl<CSTXServerDataDialog>
{
public:
	CSTXServerDataDialog();
	~CSTXServerDataDialog();

public:
	enum {IDD = IDD_DIALOG_SERVER_DATA};

protected:
	CSTXAnchor *_anchor;
	WTL::CComboBox _cbRPCHost;
	std::wstring _host;
	std::wstring _port;
	CSTXAnimatedTreeCtrlNS _tree;

	HWND _hWndCurrentEditor = NULL;
	CSTXCollectionEditorPanel _collectionEditorPanel;

protected:
	BEGIN_MSG_MAP(CSTXServerDataDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_ID_HANDLER(IDC_BUTTON_REFRESH, OnRefreshClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_SAVE_DATA, OnSaveDataClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_UNREGISTER, OnUnregisterClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_SORT, OnSortClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_SORT_AS_NUMBER, OnSortAsNumbersClicked)
		NOTIFY_CODE_HANDLER(STXATVN_ITEMEXPANDING, OnTreeItemExpanding)
		NOTIFY_CODE_HANDLER(STXATVN_POSTDELETEITEM, OnTreeItemPostDelete)
		NOTIFY_CODE_HANDLER(STXATVN_SELECTEDITEMCHANGED, OnTreeSelectedItemChanged)
		MESSAGE_HANDLER(WM_ADD_STRING_DATA, OnAddStringData)
		MESSAGE_HANDLER(WM_REMOVE_STRING_DATA, OnRemoveStringData)

	END_MSG_MAP()


protected:


	void InitializeRPCHostCombobox();
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnAddStringData(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnRemoveStringData(UINT, WPARAM, LPARAM, BOOL&);


	LRESULT OnRefreshClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnSaveDataClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnUnregisterClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnSortClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnSortAsNumbersClicked(WORD, UINT, HWND, BOOL&);

	LRESULT OnTreeItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTreeItemPostDelete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnTreeSelectedItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void GetErrorText(RPC_STATUS NTStatusMessage, CString &err);
	void CreateDataTree();
	void CreateDataEditor(HSTXTREENODE treeNode);
	CString GetSelectedItemFullPath();
	CString GetTreeNodeFullPath(HSTXTREENODE treeNode);
	void DeleteAllChildNodes(HSTXTREENODE parentNode);
	void SetTreeNodeAppearance(HSTXTREENODE treeNode, TreeNodeData *nodeData);
	CString GenerateTreeNodeText(LPCTSTR lpszOriginalName, TreeNodeData *nodeData, size_t siblingNodeCount);
	CString CombineNodePath(LPCTSTR pathLeft, LPCTSTR pathRight);
	void UpdateNodeFullPathIndicator(HSTXTREENODE treeNode);
	bool IsCollectionType(int dataType);

protected:
	void GetSharedDataTreeNodes(LPCTSTR lpszPath, std::vector<std::wstring>* pNodeNames, std::vector<int>* pNodeTypes, std::vector<unsigned long>* pNodeFlags, CString &err);
	void GetSharedDataTreeNodeStringValue(LPCTSTR lpszPath, CString &value, CString &err);
	void RunServerScriptString(LPCTSTR lpszScript, CString &result, CString &err);
	void GetSharedDataTreeNodeValues(LPCTSTR lpszPath, std::vector<std::wstring> &result, CString &err);
private:
	void RefreshNodeText(HSTXTREENODE treeNode);
};

