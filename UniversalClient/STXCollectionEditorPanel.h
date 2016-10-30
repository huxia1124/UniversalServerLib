#pragma once

#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "Resource.h"
#include "STXAnchor.h"
#include <set>


class CSTXCollectionEditorPanel : public CDialogImpl<CSTXCollectionEditorPanel>
{
public:
	CSTXCollectionEditorPanel();
	~CSTXCollectionEditorPanel();

public:
	enum { IDD = IDD_PANEL_COLLECTION_EDITOR };

protected:
	CSTXAnchor *_anchor = nullptr;

	std::vector<std::wstring> _wstrVector;
	std::vector<int64_t> _intVector;
	std::vector<double> _doubleVector;

	std::set<std::wstring> _wstrSet;
	std::set<int64_t> _intSet;
	std::set<double> _doubleSet;

	int _dataType = -1;

protected:
	BEGIN_MSG_MAP(CSTXCollectionEditorPanel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_REMOVE, OnRemoveClicked)
		COMMAND_HANDLER(IDC_LIST_DATA, LBN_DBLCLK, OnListBoxDblClick)

	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnAddClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnRemoveClicked(WORD, UINT, HWND, BOOL&);

	LRESULT OnListBoxDblClick(WORD, UINT, HWND, BOOL&);

protected:
	void ReloadListData();
	void UpdateDataTypeIndicator();

public:
	void FillCollectionValues(std::vector<std::wstring> &values, int dataType);
};

