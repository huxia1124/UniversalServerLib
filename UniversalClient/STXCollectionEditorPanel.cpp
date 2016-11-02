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
#include "STXCollectionEditorPanel.h"
#include <algorithm>
#include "STXServerDataDialog.h"
#include <iterator>

CSTXCollectionEditorPanel::CSTXCollectionEditorPanel()
{
}


CSTXCollectionEditorPanel::~CSTXCollectionEditorPanel()
{
}

LRESULT CSTXCollectionEditorPanel::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	_anchor = new CSTXAnchor(m_hWnd);

	_anchor->AddItem(IDC_BUTTON_ADD, STXANCHOR_RIGHT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_EDIT_ADD, STXANCHOR_LEFT | STXANCHOR_RIGHT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_LIST_DATA, STXANCHOR_ALL);
	_anchor->AddItem(IDC_EDIT_DATA_TYPE, STXANCHOR_LEFT | STXANCHOR_RIGHT | STXANCHOR_BOTTOM);

	_listBoxData.Attach(GetDlgItem(IDC_LIST_DATA));

	return 1; // Let dialog manager set initial focus
}

LRESULT CSTXCollectionEditorPanel::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (_anchor)
	{
		delete _anchor;
		_anchor = NULL;
	}

	_listBoxData.DestroyWindow();

	return 0;
}

LRESULT CSTXCollectionEditorPanel::OnAddClicked(WORD, UINT, HWND, BOOL&)
{
	CString inputText;
	GetDlgItemText(IDC_EDIT_ADD, inputText);
	inputText.Replace(_T("\\"), _T("\\\\"));
	inputText.Replace(_T("\r\n"), _T("\\n"));

	auto result = GetParent().SendMessage(WM_ADD_STRING_DATA, (WPARAM)(LPCTSTR)inputText);
	if(result == 0)//==0 : succeed
	{
		GetDlgItemText(IDC_EDIT_ADD, inputText);

		switch (_dataType)
		{
		case 9:
			_wstrVector.push_back((LPCTSTR)inputText);
			break;
		case 10:
			_wstrSet.insert((LPCTSTR)inputText);
			break;
		case 11:
			_intVector.push_back(_ttoi64((LPCTSTR)inputText));
			break;
		case 12:
			_intSet.insert(_ttoi64((LPCTSTR)inputText));
			break;
		case 13:
			_doubleVector.push_back(_tcstod((LPCTSTR)inputText, nullptr));
			break;
		case 14:
			_doubleSet.insert(_tcstod((LPCTSTR)inputText, nullptr));
			break;
		default:
			break;
		}

		ReloadListData();
	}
	return 0;
}

LRESULT CSTXCollectionEditorPanel::OnRemoveClicked(WORD, UINT, HWND, BOOL&)
{
	CString inputText;
	GetDlgItemText(IDC_EDIT_ADD, inputText);
	inputText.Replace(_T("\\"), _T("\\\\"));
	inputText.Replace(_T("\r\n"), _T("\\n"));

	auto result = GetParent().SendMessage(WM_REMOVE_STRING_DATA, (WPARAM)(LPCTSTR)inputText);
	if (result == 0)//==0 : succeed
	{
		GetDlgItemText(IDC_EDIT_ADD, inputText);

		switch (_dataType)
		{
		case 9:
			_wstrVector.erase(std::remove(_wstrVector.begin(), _wstrVector.end(), (LPCTSTR)inputText), _wstrVector.end());
			break;
		case 10:
			_wstrSet.erase((LPCTSTR)inputText);
			break;
		case 11:
			_intVector.erase(std::remove(_intVector.begin(), _intVector.end(), _ttoi64((LPCTSTR)inputText)), _intVector.end());
			break;
		case 12:
			_intSet.erase(_ttoi64((LPCTSTR)inputText));
			break;
		//case 13:
		//	_doubleVector.push_back(_tcstod((LPCTSTR)inputText, nullptr));
		//	break;
		//case 14:
		//	_doubleSet.insert(_tcstod((LPCTSTR)inputText, nullptr));
		//	break;
		default:
			break;
		}

		ReloadListData();
	}
	return 0;
}

LRESULT CSTXCollectionEditorPanel::OnListBoxDblClick(WORD, UINT, HWND, BOOL&)
{
	int sel = _listBoxData.GetCurSel();	
	if (sel >= 0)
	{
		CString itemText;
		_listBoxData.GetText(sel, itemText);
		SetDlgItemText(IDC_EDIT_ADD, itemText);
	}

	return 0;
}

void CSTXCollectionEditorPanel::ReloadListData()
{
	_listBoxData.ResetContent();

	switch (_dataType)
	{
	case 9:
		std::for_each(_wstrVector.begin(), _wstrVector.end(), [&](auto item) {
			_listBoxData.AddString(item.c_str());
		});
		break;
	case 10:
		std::for_each(_wstrSet.begin(), _wstrSet.end(), [&](auto item) {
			_listBoxData.AddString(item.c_str());
		});
		break;
	case 11:
		std::for_each(_intVector.begin(), _intVector.end(), [&](auto item) {
			CString str;
			str.Format(_T("%I64d"), item);
			_listBoxData.AddString(str);
		});
		break;
	case 12:
		std::for_each(_intSet.begin(), _intSet.end(), [&](auto item) {
			CString str;
			str.Format(_T("%I64d"), item);
			_listBoxData.AddString(str);
		});
		break;
	case 13:
		std::for_each(_doubleVector.begin(), _doubleVector.end(), [&](auto item) {
			CString str;
			str.Format(_T("%lf"), item);
			_listBoxData.AddString(str);
		});
		break;
	case 14:
		std::for_each(_doubleSet.begin(), _doubleSet.end(), [&](auto item) {
			CString str;
			str.Format(_T("%lf"), item);
			_listBoxData.AddString(str);
		});
		break;
	default:
		break;
	}


}

void CSTXCollectionEditorPanel::UpdateDataTypeIndicator()
{
	switch (_dataType)
	{
	case 9:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("String Vector")); break;
	case 10:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("String Set")); break;
	case 11:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("Integer Vector")); break;
	case 12:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("Integer Set")); break;
	case 13:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("Double/Float Vector")); break;
	case 14:
		SetDlgItemText(IDC_EDIT_DATA_TYPE, _T("Double/Float Set")); break;
	default:
		break;
	}
}

void CSTXCollectionEditorPanel::FillCollectionValues(std::vector<std::wstring> &values, int dataType)
{
	_wstrVector.clear();
	_intVector.clear();
	_doubleVector.clear();

	_wstrSet.clear();
	_intSet.clear();
	_doubleSet.clear();

	_dataType = dataType;

	switch (dataType)
	{
	case 9:
		std::copy(values.begin(), values.end(), std::back_inserter(_wstrVector));
		break;
	case 10:
		std::for_each(values.begin(), values.end(), [&](auto item) {
			_wstrSet.insert(item);
		});
		break;
	case 11:
		std::for_each(values.begin(), values.end(), [&](auto item) {
			_intVector.push_back(_ttoi64(item.c_str()));
		});
		break;
	case 12:
		std::for_each(values.begin(), values.end(), [&](auto item) {
			_intSet.insert(_ttoi64(item.c_str()));
		});
		break;
	case 13:
		std::for_each(values.begin(), values.end(), [&](auto item) {
			_doubleVector.push_back(_tcstod(item.c_str(), nullptr));
		});
		break;
	case 14:
		std::for_each(values.begin(), values.end(), [&](auto item) {
			_doubleSet.insert(_tcstod(item.c_str(), nullptr));
		});
		break;
	default:
		break;
	}

	ReloadListData();

	UpdateDataTypeIndicator();
}
