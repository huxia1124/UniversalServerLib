#include "stdafx.h"
#include "STXScriptDialog.h"
#include <atldlgs.h>

#include "../UniversalServerLib/UniversalServerRPC_h.h"
#include <iterator>

#pragma comment(lib,"rpcrt4")
#pragma comment(lib,"ole32")

void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}
// Memory deallocation function for RPC.

void __RPC_USER midl_user_free(void* p)
{
	free(p);
}

CSTXScriptDialog::CSTXScriptDialog()
{
	_anchor = NULL;
}


CSTXScriptDialog::~CSTXScriptDialog()
{
}

void CSTXScriptDialog::InitializeRPCHostCombobox()
{
	int index = -1;
	_cbRPCHost.ResetContent();
	index = _cbRPCHost.AddString(_T("localhost:4047"));
	index = _cbRPCHost.AddString(_T("127.0.0.1:137"));
	index = _cbRPCHost.AddString(_T("localhost:3399"));
	_cbRPCHost.SetCurSel(index);
}

LRESULT CSTXScriptDialog::OnRunScriptClicked(WORD, UINT, HWND, BOOL&)
{

	::EnableWindow(GetDlgItem(IDOK), FALSE);
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

	int nLen = _edtScript.GetWindowTextLength();
	//if (nLen == 0)
	//	return 0;

	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_edtScript.GetWindowText(pText, nLen + 1);


	_scriptToRun = pText;

	delete[]pText;


	::SetFocus(GetDlgItem(IDC_EDIT_SCRIPT));
	//_edtScript.SetSelAll();

	CString result;
	CString error;
	RunServerScriptString(_scriptToRun.c_str(), result, error);

	::EnableWindow(GetDlgItem(IDOK), TRUE);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING|MB_OK);
	}
	if (!result.IsEmpty())
	{
		MessageBox(result, _T("Result"), MB_ICONINFORMATION | MB_OK);
	}


	return 0;
}

LRESULT CSTXScriptDialog::OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (!CheckSave())
		return 0;

	DestroyWindow();
	return 0;
}

LRESULT CSTXScriptDialog::OnCancelClicked(WORD, UINT, HWND, BOOL&)
{
	if (!CheckSave())
		return 0;

	DestroyWindow();
	return 0;
}

LRESULT CSTXScriptDialog::OnOpenFileClicked(WORD, UINT, HWND, BOOL&)
{
	CFileDialog fd(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("*.lua\0*.lua\0*.*\0*.*\0\0"), m_hWnd);
	if (fd.DoModal(m_hWnd) == IDOK)
	{
		BOOL bResult = FALSE;
		std::wstring content = GetFileContentText(fd.m_ofn.lpstrFile, &bResult);

		if(bResult)
			_edtScript.SetWindowText(content.c_str());
	}

	return 0;
}

LRESULT CSTXScriptDialog::OnScheduleWorkerThreadScriptClicked(WORD, UINT, HWND, BOOL&)
{
	::EnableWindow(GetDlgItem(IDOK), FALSE);
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



	int nLen = _edtScript.GetWindowTextLength();
	//if (nLen == 0)
	//	return 0;

	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_edtScript.GetWindowText(pText, nLen + 1);


	_scriptToRun = pText;

	delete[]pText;


	::SetFocus(GetDlgItem(IDC_EDIT_SCRIPT));
	//_edtScript.SetSelAll();

	CString error;
	EnqueueServerWorkerThreadScriptString(_scriptToRun.c_str(), error);

	::EnableWindow(GetDlgItem(IDOK), TRUE);

	if (!error.IsEmpty())
	{
		MessageBox(error, _T("Error"), MB_ICONWARNING | MB_OK);
	}

	return 0;
}

/*
LRESULT CSTXScriptDialog::OnHistoryClicked(WORD, UINT, HWND, BOOL&)
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
*/

LRESULT CSTXScriptDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	//CenterWindow();

	_anchor = new CSTXAnchor(m_hWnd);

	_cbRPCHost.Attach(GetDlgItem(IDC_COMBO_RPC_HOST));
	InitializeRPCHostCombobox();
	_edtScript.Attach(GetDlgItem(IDC_EDIT_SCRIPT));

	_anchor->AddItem(IDC_EDIT_SCRIPT, STXANCHOR_ALL);
	_anchor->AddItem(IDOK, STXANCHOR_BOTTOM | STXANCHOR_RIGHT);
	_anchor->AddItem(IDCANCEL, STXANCHOR_BOTTOM | STXANCHOR_RIGHT);
	_anchor->AddItem(IDC_BUTTON_LOAD_FILE, STXANCHOR_BOTTOM | STXANCHOR_LEFT);
	_anchor->AddItem(IDC_BUTTON_ENQUEUE_WORKER_THREAD_SCRIPT, STXANCHOR_BOTTOM | STXANCHOR_RIGHT);

	// Copy the string from the data member
	// to the child control (DDX)
	//SetDlgItemText(IDC_STRING, m_sz);

	_originalDefaultScript = _T("--This lua script can be executed at server\r\n"
		"print(\"This message will be print on the server console\")\r\n"
		"result = \"Put anything to the result variable as return value.\"");

	_edtScript.SetWindowText(_originalDefaultScript.c_str());

	LoadHistory();

	CenterWindow(::GetParent(m_hWnd));

	return 1; // Let dialog manager set initial focus
}

	/*

LRESULT CSTXScriptDialog::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
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
	*/

void CSTXScriptDialog::SaveHistory()
{
	/*
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
	*/
}

void CSTXScriptDialog::AddToHistoryCache()
{
	/*
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
	*/
}

void CSTXScriptDialog::LoadHistory()
{
	/*
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
	*/
}


BOOL CSTXScriptDialog::CheckSave()
{
	int nLen = _edtScript.GetWindowTextLength();
	TCHAR *pText = new TCHAR[nLen + 2];
	pText[nLen + 1] = 0;
	_edtScript.GetWindowText(pText, nLen + 1);
	_scriptToRun = pText;
	delete[]pText;

	if (_scriptToRun.empty())
		return TRUE;
	if (_scriptToRun == _originalDefaultScript)
		return TRUE;

	auto result = MessageBox(_T("Save the script to file ?"), _T("Save script"), MB_YESNOCANCEL|MB_ICONWARNING);
	if (result == IDYES)
	{
		CFileDialog fd(FALSE, _T("*.lua"), _T("myscript.lua"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("*.lua\0*.lua\0*.*\0*.*\0\0"), m_hWnd);
		if (fd.DoModal(m_hWnd) == IDOK)
		{
			WriteToFile(fd.m_ofn.lpstrFile, _scriptToRun.c_str());
			return TRUE;
		}
	}
	else if (result == IDNO)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CSTXScriptDialog::WriteToFile(LPCTSTR lpszFile, LPCTSTR lpszText)
{
	HANDLE hFile = CreateFile(lpszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	USES_CONVERSION;
	std::string strAnsi = (LPCSTR)CW2A(lpszText);

	DWORD dwLength = strAnsi.size();
	DWORD dwWritePos = 0;
	while (dwLength > 0)
	{
		DWORD nBytesToWrite = min(dwLength, 65536);
		DWORD dwBytesWritten = 0;
		WriteFile(hFile, strAnsi.c_str() + dwWritePos, nBytesToWrite, &dwBytesWritten, NULL);
		dwLength -= dwBytesWritten;
		dwWritePos += dwBytesWritten;
	}

	CloseHandle(hFile);
	return TRUE;
}

void CSTXScriptDialog::GetErrorText(RPC_STATUS NTStatusMessage, CString &err)
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

void CSTXScriptDialog::RunServerScriptString(LPCTSTR lpszScript, CString &result, CString &err)
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

void CSTXScriptDialog::RunServerScriptFile(LPCTSTR lpszScriptFile)
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
	if (status) {
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		RunScriptFile(hwBinding, lpszScriptFile);
	}
		RpcExcept(1)
	{
		printf("Runtime reported exception:%d,except=%d\n", GetLastError(), RpcExceptionCode());
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

void CSTXScriptDialog::EnqueueServerWorkerThreadScriptString(LPCTSTR lpszScript, CString &err)
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
		EnqueueWorkerThreadScriptString(hwBinding, lpszScript);
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

std::wstring CSTXScriptDialog::GetFileContentText(LPCTSTR lpszFile, BOOL *pbResult)
{
	if (pbResult)
	{
		*pbResult = FALSE;
	}
	USES_CONVERSION;
	HANDLE file = INVALID_HANDLE_VALUE;
	file = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return _T("");

	LARGE_INTEGER size;
	BOOL bSuccess = GetFileSizeEx(file, &size);

	std::vector<unsigned char> vData;
	const int bufferSize = 16384 * 40;
	char *pszBuffer = new char[bufferSize];
	if (pszBuffer == NULL)
	{
		if (pbResult)
		{
			*pbResult = TRUE;
		}
		return _T("");
	}
	DWORD dwRead = 0;
	while (TRUE)
	{
		bSuccess = ReadFile(file, pszBuffer, (DWORD)bufferSize, &dwRead, NULL);
		if (!bSuccess || dwRead == 0)
			break;

		std::copy(pszBuffer, pszBuffer + dwRead, std::back_inserter(vData));
	}

	delete[]pszBuffer;

	vData.push_back('\0');
	vData.push_back('\0');

	std::wstring strFileContent;

	CloseHandle(file);

	if (pbResult)
	{
		*pbResult = TRUE;
	}

	if (vData[0] == 0xFF && vData[1] == 0xFE)	//Unicode
	{
		strFileContent = (LPCWSTR)&vData[2];
		return strFileContent;
	}
	else if (vData.size() >= 3)
	{
		//EF¡¢BB¡¢BF
		if (vData[0] == 0xEF && vData[1] == 0xBB && vData[2] == 0xBF)	//UTF-8
		{
			//Todo: Convert UTF-8 to Unicode
			strFileContent = A2W_CP((LPCSTR)&vData[2], CP_UTF8);
			return strFileContent;
		}
		else
		{
			//Todo: Convert ANSI to Unicode
			strFileContent = A2W((LPCSTR)&vData[0]);
			return strFileContent;
		}
	}
	else
	{
		//Todo: Convert ANSI to Unicode
		strFileContent = A2W((LPCSTR)&vData[0]);
		return strFileContent;
	}

	return strFileContent;
}