#include "stdafx.h"
#include "STXProtocolDialog.h"
#include "STXScriptDialog.h"


CSTXProtocolDialog::CSTXProtocolDialog()
{
	_connectionContext = NULL;
	_anchor = NULL;
	_dirty = FALSE;
	_encryptionEnabled = FALSE;
}


CSTXProtocolDialog::~CSTXProtocolDialog()
{
}

void CSTXProtocolDialog::InitializeDataTypeCombobox()
{
	int index = -1;
	_cbDataType.ResetContent();
	index = _cbDataType.AddString(_T("Byte")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_BYTE);
	index = _cbDataType.AddString(_T("Word")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_WORD);
	index = _cbDataType.AddString(_T("DWord")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_DWORD);
	index = _cbDataType.AddString(_T("Int64")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_I64);
	index = _cbDataType.AddString(_T("UTF8 String")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_UTF8);
	index = _cbDataType.AddString(_T("UNICODE String")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_UNICODE);
	index = _cbDataType.AddString(_T("GUID")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_GUID);
	index = _cbDataType.AddString(_T("*Object")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_OBJECT);
	index = _cbDataType.AddString(_T("RAW")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_RAW);
	index = _cbDataType.AddString(_T("Float")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_FLOAT);
	index = _cbDataType.AddString(_T("Double")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_DOUBLE);
	index = _cbDataType.AddString(_T("UTF8-UTF8 Pair")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_UTF8_PAIR);
	index = _cbDataType.AddString(_T("UNICODE Pair")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_UNICODE_PAIR);
	index = _cbDataType.AddString(_T("UTF8-DWord Pair")); _cbDataType.SetItemData(index, STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR);

	_cbDataType.SetCurSel(0);
}

void CSTXProtocolDialog::InitializeServerCombobox()
{
	int index = -1;
	_cbServers.ResetContent();
	index = _cbServers.AddString(_T("127.0.0.1:9000"));
	index = _cbServers.AddString(_T("127.0.0.1:6800"));
	index = _cbServers.AddString(_T("127.0.0.1:9111"));
	_cbServers.SetCurSel(index);
}

void CSTXProtocolDialog::BuildPackage(CSTXProtocol &p)
{
	int n = _lstData.GetItemCount();
	for (int i = 0; i < n; i++)
	{
		ATL::CString strType;
		_lstData.GetItemText(i, 1, strType);
		ATL::CString strData;
		//_lstData.GetItemText(i, 2, strData);
		strData = (LPCTSTR)_lstData.GetItemData(i);

		AddPackageData(p, strType, strData);
	}
}

void CSTXProtocolDialog::AddPackageData(CSTXProtocol &p, ATL::CString type, ATL::CString value)
{
	int iSplit = type.Find(_T(','), 0);
	ATL::CString typeVal = type.Left(iSplit);

	BYTE nType = (BYTE)_ttoi(typeVal);
	switch(nType)
	{
	case STXPROTOCOL_DATA_TYPE_BYTE:
		AddPackageDataByte(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_WORD:
		AddPackageDataWord(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_DWORD:
		AddPackageDataDWord(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_I64:
		AddPackageDataI64(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_FLOAT:
		AddPackageDataFloat(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_DOUBLE:
		AddPackageDataDouble(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_UTF8:
		AddPackageDataUTF8(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_UNICODE:
		AddPackageDataUnicode(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_GUID:
		AddPackageDataGUID(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_RAW:
		AddPackageDataRAW(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
		AddPackageDataUTF8Pair(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
		AddPackageDataUnicodePair(p, value);	break;
	case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		AddPackageDataUTF8DWordPair(p, value);	break;
	}

}

void CSTXProtocolDialog::AddPackageDataByte(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((BYTE)_ttoi(value));
}

void CSTXProtocolDialog::AddPackageDataWord(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((WORD)_ttoi(value));
}

void CSTXProtocolDialog::AddPackageDataDWord(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((uint32_t)_ttoi(value));
}

void CSTXProtocolDialog::AddPackageDataI64(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData(_ttoi64(value));
}

void CSTXProtocolDialog::AddPackageDataFloat(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((float)_ttof(value));
}

void CSTXProtocolDialog::AddPackageDataDouble(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((double)_tcstold(value, NULL));
}

void CSTXProtocolDialog::AddPackageDataUTF8(CSTXProtocol &p, ATL::CString value)
{
	p.AppendData((const char16_t*)(LPCTSTR)value);
}

void CSTXProtocolDialog::AddPackageDataUTF8Pair(CSTXProtocol &p, ATL::CString value)
{
	TCHAR *next_token = NULL;
	TCHAR *sep = _tcstok_s((TCHAR*)(LPCTSTR)value, _T("\1"), &next_token);

	if (next_token)
	{
		p.AppendDataPair((const char16_t*)sep, (const char16_t*)next_token);
		*next_token = 1;	//restore
	}
}

void CSTXProtocolDialog::AddPackageDataUnicodePair(CSTXProtocol &p, ATL::CString value)
{
	TCHAR *next_token = NULL;
	TCHAR *sep = _tcstok_s((TCHAR*)(LPCTSTR)value, _T("\1"), &next_token);

	if (next_token)
	{
		p.AppendUnicodeStringPair((const char16_t*)sep, (const char16_t*)next_token);
		*next_token = 1;	//restore
	}
}

void CSTXProtocolDialog::AddPackageDataUTF8DWordPair(CSTXProtocol &p, ATL::CString value)
{
	TCHAR *next_token = NULL;
	TCHAR *sep = _tcstok_s((TCHAR*)(LPCTSTR)value, _T("\1"), &next_token);

	if (next_token)
	{
		p.AppendDataPair((const char16_t*)sep, (DWORD)_ttoi(next_token));
		*next_token = 1;	//restore
	}
}

void CSTXProtocolDialog::AddPackageDataUnicode(CSTXProtocol &p, ATL::CString value)
{
	p.AppendUnicodeString((const char16_t*)(LPCTSTR)value);
}

void CSTXProtocolDialog::AddPackageDataGUID(CSTXProtocol &p, ATL::CString value)
{
	value.Remove(_T('{'));
	value.Remove(_T('}'));
	value = _T("{") + value + _T("}");
	GUID guid;
	HRESULT hr = IIDFromString(ATL::CComBSTR((LPCTSTR)value), &guid);
	p.AppendData(guid);
}

LPVOID CSTXProtocolDialog::GetDataFromHexString(LPCTSTR lpszHexValue, DWORD* pdwLen)
{
	if (_tcsnicmp(lpszHexValue, _T("0x"), 2) != 0)
		return NULL;

	int hexLen = _tcslen(lpszHexValue);
	if (hexLen < 4)
		return NULL;

	int Len = (int)(hexLen - 2) / 2;
	BYTE* pBuf = new BYTE[Len];

	int nPos = 0;
	lpszHexValue += 2;
	while (*lpszHexValue && *(lpszHexValue + 1))
	{
		BYTE bVal = 0;
		BYTE nHI = 0, nLO = 0;
		if (*lpszHexValue >= _T('0') && *lpszHexValue <= _T('9'))
			nHI = *lpszHexValue - _T('0');
		else if ((*lpszHexValue >= _T('a') && *lpszHexValue <= _T('f')))
			nHI = *lpszHexValue - _T('a') + 10;
		else if ((*lpszHexValue >= _T('A') && *lpszHexValue <= _T('F')))
			nHI = *lpszHexValue - _T('A') + 10;

		lpszHexValue++;
		if (*lpszHexValue >= _T('0') && *lpszHexValue <= _T('9'))
			nLO = *lpszHexValue - _T('0');
		else if ((*lpszHexValue >= _T('a') && *lpszHexValue <= _T('f')))
			nLO = *lpszHexValue - _T('a') + 10;
		else if ((*lpszHexValue >= _T('A') && *lpszHexValue <= _T('F')))
			nLO = *lpszHexValue - _T('A') + 10;

		bVal = nHI * 16 + nLO;

		lpszHexValue++;

#pragma warning(suppress: 6386)
		pBuf[nPos] = bVal;
		nPos++;
	}

	*pdwLen = Len;
	return pBuf;

}

void CSTXProtocolDialog::AddPackageDataRAW(CSTXProtocol &p, ATL::CString value)
{
	DWORD dwLen = 0;
	char *pData = (char*)GetDataFromHexString(value, &dwLen);

	p.AppendRawData(pData, dwLen);
	delete[]pData;
}

LRESULT CSTXProtocolDialog::OnConnectNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	::EnableWindow(GetDlgItem(IDC_BUTTON_CONNECT), TRUE);

	if (_connectionContext)
	{
		_connectionContext->Release();
		_connectionContext = NULL;
	}

	_connectionContext = lParam == 0 ? (CSTXSocketConnectionContext*)wParam : NULL;
	if (_connectionContext)
		_connectionContext->AddRef();

	::EnableWindow(GetDlgItem(IDC_BUTTON_SEND), lParam == 0);

	InvalidateRect(NULL);
	return 0;
}

LRESULT CSTXProtocolDialog::OnDisconnectNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	::EnableWindow(GetDlgItem(IDC_BUTTON_CONNECT), TRUE);

	_connectionContext = (CSTXSocketConnectionContext*)wParam;
	if (_connectionContext)
	{
		_connectionContext->Release();
		_connectionContext = NULL;
	}

	::EnableWindow(GetDlgItem(IDC_BUTTON_SEND), FALSE);
	InvalidateRect(NULL);
	return 0;
}

LRESULT CSTXProtocolDialog::OnAddClicked(WORD, UINT, HWND, BOOL&)
{
	_dirty = TRUE;
	DWORD_PTR dwSel = _cbDataType.GetItemData(_cbDataType.GetCurSel());


	int nLen = _edtContent.GetWindowTextLength();
	int nLen2 = _edtContent2.GetWindowTextLength();
	//if (nLen == 0)
	//	return 0;

	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_edtContent.GetWindowText(pText, nLen + 1);

	TCHAR *pText2 = new TCHAR[nLen2 + 2];
	pText2[nLen2 + 1] = 0;
	_edtContent2.GetWindowText(pText2, nLen2 + 1);

	int nTotalLen = nLen + 2 + nLen2 + 2 + 2;
	TCHAR *pNewText = NULL;
	switch (dwSel)
	{
	case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
	case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
	case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		pNewText = new TCHAR[nTotalLen];
		pNewText[0] = 0;
		_tcscat_s(pNewText, nTotalLen, pText);
		_tcscat_s(pNewText, nTotalLen, _T("\1"));
		_tcscat_s(pNewText, nTotalLen, pText2);
		break;
	default:
		pNewText = new TCHAR[nLen + 2];
		pNewText[0] = 0;
		_tcscat_s(pNewText, nLen + 2, pText);
	}
	

	AddContentToList(pNewText);

	delete[]pText;
	delete[]pText2;

	::SetFocus(GetDlgItem(IDC_EDIT_VALUE));
	_edtContent.SetSelAll();

	return 0;
}

LRESULT CSTXProtocolDialog::OnSendClicked(WORD, UINT, HWND, BOOL&)
{
	if (_dirty)
	{
		_dirty = FALSE;
		AddToHistoryCache();
	}

	CSTXProtocol p;
	BuildPackage(p);

	if (_connectionContext && _connectionContext->IsConnected())
	{
		if (_encryptionEnabled)
		{
			int n = p.GetDataLen();
			char *pd = new char[n];
			p.GetEncryptedData(pd, n, 1332);
			_connectionContext->Send(pd, n, TRUE);
			delete[]pd;
		}
		else
		{
			_connectionContext->Send(p.GetBasePtr(), p.GetDataLen(), TRUE);
		}
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_BUTTON_SEND), FALSE);
		if (_connectionContext)
		{
			_connectionContext->Release();
			_connectionContext = NULL;
		}
	}

	return 0;
}

LRESULT CSTXProtocolDialog::OnConnectClicked(WORD, UINT, HWND, BOOL&)
{
	::EnableWindow(GetDlgItem(IDC_BUTTON_CONNECT), FALSE);
	InvalidateRect(NULL);
	_socket.SetDialogPtr(this);
	_socket.Close();
	_socket.Create();

	int nLen = _cbServers.GetWindowTextLength();
	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_cbServers.GetWindowText(pText, nLen + 1);

	_socket.BeginConnectEx(pText, m_hWnd, WM_SOCKET_CONNECT_NOTIFY, WM_SOCKET_DISCONNECT_NOTIFY);

	delete[]pText;

	return 0;
}

LRESULT CSTXProtocolDialog::OnHistoryClicked(WORD, UINT, HWND, BOOL&)
{
	WTL::CMenu menu;
	menu.CreatePopupMenu();

	UINT i = 0;
	for (CSTXProtocolHistory &item : _histories)
	{
		i++;
		TCHAR szText[MAX_PATH];
		MENUITEMINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_STRING | MIIM_ID;
		mi.wID = (UINT)i;
		mi.fType = MFT_STRING;
		_stprintf_s(szText, _T("History %.2d-%.2d %.2d:%.2d:%.2d (%I64d values)"), item._time.wMonth, item._time.wDay, item._time.wHour, item._time.wMinute, item._time.wSecond, (__int64)item._items.size());
		mi.cch = _tcslen(szText);
		mi.dwTypeData = szText;
		menu.InsertMenuItem(i, TRUE, &mi);
	}

	POINT ptCursor;
	GetCursorPos(&ptCursor);
	int nPos = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptCursor.x, ptCursor.y, m_hWnd);

	if (nPos > 0)
	{
		nPos -= 1;
		_lstData.DeleteAllItems();
		CSTXProtocolHistory &item = _histories[nPos];

		for (size_t k = 0; k < item._items.size(); k++)
		{
			int nIndex = _lstData.InsertItem(_lstData.GetItemCount(), item._items[k][0].c_str());
			_lstData.SetItemText(nIndex, 1, item._items[k][1].c_str());
			_lstData.SetItemText(nIndex, 2, item._items[k][2].c_str());

			TCHAR *pNewText = new TCHAR[item._items[k][3].size() + 1];
			_tcscpy_s(pNewText, item._items[k][3].size() + 1, item._items[k][3].c_str());
			_lstData.SetItemData(nIndex, (DWORD_PTR)pNewText);
		}

		_cbServers.SetWindowText(item._target.c_str());
	}

	return 0;
}

LRESULT CSTXProtocolDialog::OnShowScriptDialogClicked(WORD, UINT, HWND, BOOL&)
{
	auto dlg = std::make_shared<CSTXScriptDialog>();
	dlg->Create(m_hWnd);
	dlg->ShowWindow(SW_SHOW);

	_scriptDialogs.push_back(dlg);

	//CSTXScriptDialog dlg;
	//dlg.DoModal(this->m_hWnd);

	return 0;
}

void CSTXProtocolDialog::AddContentToList(LPCTSTR lpszValue)
{
	DWORD_PTR dwSel = _cbDataType.GetItemData(_cbDataType.GetCurSel());
	int nLen = _cbDataType.GetWindowTextLength();
	//if (nLen == 0)
	//	return 0;

	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_cbDataType.GetWindowText(pText, nLen + 1);

	TCHAR szTemp[128];
	_itot_s(_lstData.GetItemCount() + 1, szTemp, 64, 10);
	int index = _lstData.InsertItem(_lstData.GetItemCount(), szTemp);
	_itot_s(dwSel, szTemp, 128, 10);
	_tcscat_s(szTemp, _T(","));
	_tcscat_s(szTemp, pText);

	int n = sizeof(DWORD_PTR);
	_lstData.SetItemText(index, 1, szTemp);
	_lstData.SetItemText(index, 2, lpszValue);
	_lstData.SetItemData(index, (DWORD_PTR)lpszValue);
	delete[]pText;
}

LRESULT CSTXProtocolDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	//CenterWindow();

	_anchor = new CSTXAnchor(m_hWnd);

	_cbDataType.Attach(GetDlgItem(IDC_COMBO_TYPE));
	InitializeDataTypeCombobox();
	_cbServers.Attach(GetDlgItem(IDC_COMBO_SERVERS));
	InitializeServerCombobox();
	_edtContent.Attach(GetDlgItem(IDC_EDIT_VALUE));
	_edtContent2.Attach(GetDlgItem(IDC_EDIT_VALUE2));
	_lstData.Attach(GetDlgItem(IDC_LIST_VALUES));
	_lstData.InsertColumn(0, _T("Sequence"), LVCFMT_LEFT, 60);
	_lstData.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 120);
	_lstData.InsertColumn(2, _T("Content"), LVCFMT_LEFT, 480);
	_lstData.SetExtendedListViewStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	_anchor->AddItem(IDC_EDIT_VALUE, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_EDIT_VALUE2, STXANCHOR_LEFT | STXANCHOR_TOP | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_COMBO_TYPE, STXANCHOR_TOP | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_BUTTON_ADD, STXANCHOR_TOP | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_LIST_VALUES, STXANCHOR_ALL);
	_anchor->AddItem(IDC_COMBO_SERVERS, STXANCHOR_BOTTOM | STXANCHOR_LEFT);
	_anchor->AddItem(IDC_BUTTON_CONNECT, STXANCHOR_BOTTOM | STXANCHOR_LEFT);
	_anchor->AddItem(IDC_BUTTON_SEND, STXANCHOR_BOTTOM | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_BUTTON_HISTORY, STXANCHOR_TOP | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_BUTTON_SCRIPT_DIALOG, STXANCHOR_BOTTOM | STXANCHOR_LEFT);

	UpdateAddButtonEnabled();
	UpdateEnterFields();
	// Copy the string from the data member
	// to the child control (DDX)
	//SetDlgItemText(IDC_STRING, m_sz);

	LoadHistory();

	return 1; // Let dialog manager set initial focus
}

LRESULT CSTXProtocolDialog::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	PAINTSTRUCT ps;
	BeginPaint(&ps);

	CDCHandle dc;
	dc.Attach(ps.hdc);

	const int HEIGHT = 120;

	TRIVERTEX        vert[2];
	GRADIENT_RECT    gRect;
	RECT rcClient;
	GetClientRect(&rcClient);

	BOOL bConnected = ::IsWindowEnabled(GetDlgItem(IDC_BUTTON_SEND));
	BOOL bConnecting = !::IsWindowEnabled(GetDlgItem(IDC_BUTTON_CONNECT));

	COLORREF colorBK = GetSysColor(COLOR_BTNFACE);

	vert[0].x = rcClient.left;
	vert[0].y = rcClient.bottom - HEIGHT;
	vert[0].Red = MAKEWORD(0, GetRValue(colorBK));
	vert[0].Green = MAKEWORD(0, GetGValue(colorBK));
	vert[0].Blue = MAKEWORD(0, GetBValue(colorBK));
	vert[0].Alpha = 0;

	vert[1].x = rcClient.right;
	vert[1].y = rcClient.bottom;

	if (bConnected)
	{
		vert[1].Red = MAKEWORD(0, 192);
		vert[1].Green = MAKEWORD(0, 255);
		vert[1].Blue = MAKEWORD(0, 192);
	}
	else
	{
		if (bConnecting)
		{
			vert[1].Red = MAKEWORD(0, 248);
			vert[1].Green = MAKEWORD(0, 248);
			vert[1].Blue = MAKEWORD(0, 168);
		}
		else
		{
			vert[1].Red = MAKEWORD(0, 255);
			vert[1].Green = MAKEWORD(0, 192);
			vert[1].Blue = MAKEWORD(0, 192);
		}
	}

	vert[1].Alpha = 0;

	gRect.UpperLeft = 0;
	gRect.LowerRight = 1;

	GradientFill(ps.hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

	EndPaint(&ps);
	return 0;
}

LRESULT CSTXProtocolDialog::OnCbTypeSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateAddButtonEnabled();
	UpdateEnterFields();
	return 0;
}

void CSTXProtocolDialog::DoOnClose()
{
	if (_dirty)
	{
		_dirty = FALSE;
		AddToHistoryCache();
	}

	SaveHistory();
}

void CSTXProtocolDialog::AppendResponseMsg(LPCTSTR lpszMsg)
{
	MessageBox(lpszMsg, _T("Response"), MB_OK | MB_ICONINFORMATION);
}

void CSTXProtocolDialog::SaveHistory()
{
	TCHAR szHistoryFile[MAX_PATH];
	GetModuleFileName(NULL, szHistoryFile, MAX_PATH);
	LPTSTR pszFileName = PathFindFileName(szHistoryFile);
	*pszFileName = NULL;
	_tcscat_s(szHistoryFile, _T("UniversalClientHistory.dat"));

	FILE *file = NULL;
	_tfopen_s(&file, szHistoryFile, _T("wb"));
	if (file == NULL)
	{
		return;
	}

	unsigned int nItems = (unsigned int)_histories.size();
	fwrite(&nItems, sizeof(nItems), 1, file);

	for (unsigned int i = 0; i < nItems; i++)
	{
		_histories[i].WriteToFile(file);
	}

	fclose(file);
}

void CSTXProtocolDialog::AddToHistoryCache()
{
	CSTXProtocolHistory item;
	GetSystemTime(&item._time);

	int nRows = _lstData.GetItemCount();
	if (nRows == 0)
		return;


	for (int i = 0; i < nRows; i++)
	{
		item._items.push_back(std::vector<std::wstring>());
		for (int k = 0; k < 3; k++)
		{
			ATL::CString strText;
			_lstData.GetItemText(i, k, strText);
			item._items[i].push_back((LPCTSTR)strText);
		}
		ATL::CString strData;
		strData = (LPCTSTR)_lstData.GetItemData(i);

		item._items[i].push_back((LPCTSTR)strData);
	}

	int nLen = _cbServers.GetWindowTextLength();
	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_cbServers.GetWindowText(pText, nLen + 1);

	item._target = pText;

	delete[]pText;

	_histories.push_back(item);
}

void CSTXProtocolDialog::LoadHistory()
{
	TCHAR szHistoryFile[MAX_PATH];
	GetModuleFileName(NULL, szHistoryFile, MAX_PATH);
	LPTSTR pszFileName = PathFindFileName(szHistoryFile);
	*pszFileName = NULL;
	_tcscat_s(szHistoryFile, _T("UniversalClientHistory.dat"));

	FILE *file = NULL;
	_tfopen_s(&file, szHistoryFile, _T("rb"));
	if (file == NULL)
	{
		return;
	}

	unsigned int nItems = 0;
	fread(&nItems, sizeof(nItems), 1, file);

	for (unsigned int i = 0; i < nItems; i++)
	{
		CSTXProtocolHistory item;
		item.ReadFromFile(file);
		_histories.push_back(item);
	}

	fclose(file);
}

void CSTXProtocolDialog::UpdateAddButtonEnabled()
{
	int nLen = _cbDataType.GetWindowTextLength();
	//if (nLen == 0)
	//	return 0;

	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_cbDataType.GetWindowText(pText, nLen + 1);

	::EnableWindow(GetDlgItem(IDC_BUTTON_ADD), _tcschr(pText, _T('*')) == NULL);

	delete[]pText;
}

void CSTXProtocolDialog::UpdateEnterFields()
{
	DWORD_PTR dwSel = _cbDataType.GetItemData(_cbDataType.GetCurSel());
	switch (dwSel)
	{
	case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
	case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
	case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		::EnableWindow(GetDlgItem(IDC_EDIT_VALUE2), TRUE); break;
	default:
		::EnableWindow(GetDlgItem(IDC_EDIT_VALUE2), FALSE);
	}

}

LRESULT CSTXProtocolDialog::OnListViewItemDelete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMLISTVIEW pNM = (LPNMLISTVIEW)pnmh;

	if (pNM->lParam)
	{
		delete[](TCHAR*)pNM->lParam;
	}

	return 0;
}

LRESULT CSTXProtocolDialog::OnListViewDblClk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LVITEM lvitem;
	while (_lstData.GetSelectedItem(&lvitem))
	{
		_lstData.DeleteItem(lvitem.iItem);

	}

	int nCount = _lstData.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		ATL::CString itemId;
		itemId.Format(_T("%d"), i + 1);
		_lstData.SetItemText(i, 0, itemId);
	}
	
	return 0;
}
