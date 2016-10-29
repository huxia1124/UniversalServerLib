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

	return 1; // Let dialog manager set initial focus
}

LRESULT CSTXCollectionEditorPanel::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (_anchor)
	{
		delete _anchor;
		_anchor = NULL;
	}

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

		//auto listBox = GetDlgItem(IDC_LIST_DATA);
		//::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)inputText);

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

		//auto listBox = GetDlgItem(IDC_LIST_DATA);
		//::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)inputText);

		ReloadListData();
	}
	return 0;
}

void CSTXCollectionEditorPanel::ReloadListData()
{
	auto listBox = GetDlgItem(IDC_LIST_DATA);
	::SendMessage(listBox.m_hWnd, LB_RESETCONTENT, 0, 0);

	switch (_dataType)
	{
	case 9:
		std::for_each(_wstrVector.begin(), _wstrVector.end(), [&](auto item) {
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)item.c_str());
		});
		break;
	case 10:
		std::for_each(_wstrSet.begin(), _wstrSet.end(), [&](auto item) {
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)item.c_str());
		});
		break;
	case 11:
		std::for_each(_intVector.begin(), _intVector.end(), [&](auto item) {
			CString str;
			str.Format(_T("%I64d"), item);
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		});
		break;
	case 12:
		std::for_each(_intSet.begin(), _intSet.end(), [&](auto item) {
			CString str;
			str.Format(_T("%I64d"), item);
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		});
		break;
	case 13:
		std::for_each(_doubleVector.begin(), _doubleVector.end(), [&](auto item) {
			CString str;
			str.Format(_T("%lf"), item);
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		});
		break;
	case 14:
		std::for_each(_doubleSet.begin(), _doubleSet.end(), [&](auto item) {
			CString str;
			str.Format(_T("%lf"), item);
			::SendMessage(listBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		});
		break;
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
}
