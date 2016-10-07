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

#include "Resource.h"
#include "STXProtocol.h"
#include "UniversalClientSocket.h"
#include "STXAnchor.h"
#include "STXProtocolHistory.h"

#define WM_SOCKET_CONNECT_NOTIFY		(WM_USER + 10)
#define WM_SOCKET_DISCONNECT_NOTIFY		(WM_USER + 11)

class CSTXProtocolDialog : public CDialogImpl<CSTXProtocolDialog>
{
public:
	CSTXProtocolDialog();
	~CSTXProtocolDialog();

public:
	enum {IDD = IDD_DIALOG_STXPROTOCOL};

public:
	BOOL _encryptionEnabled;

protected:
	CUniversalClientSocket _socket;
	CSTXSocketConnectionContext *_connectionContext;
	CSTXAnchor *_anchor;

	WTL::CComboBox _cbDataType;
	WTL::CComboBox _cbServers;
	WTL::CEdit _edtContent;
	WTL::CEdit _edtContent2;
	WTL::CListViewCtrl _lstData;

	BOOL _dirty;
	std::vector<CSTXProtocolHistory> _histories;

protected:
	BEGIN_MSG_MAP(CSTXProtocolDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)

		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SOCKET_CONNECT_NOTIFY, OnConnectNotify)
		MESSAGE_HANDLER(WM_SOCKET_DISCONNECT_NOTIFY, OnDisconnectNotify)

		COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_SEND, OnSendClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_CONNECT, OnConnectClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_HISTORY, OnHistoryClicked)

		COMMAND_HANDLER(IDC_COMBO_TYPE, CBN_SELCHANGE, OnCbTypeSelChange)

		NOTIFY_HANDLER(IDC_LIST_VALUES, LVN_DELETEITEM, OnListViewItemDelete)
		NOTIFY_HANDLER(IDC_LIST_VALUES, NM_DBLCLK, OnListViewDblClk)
	END_MSG_MAP()


protected:
	void InitializeDataTypeCombobox();
	void InitializeServerCombobox();
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		DoOnClose();

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
	LRESULT OnListViewItemDelete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnListViewDblClk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCbTypeSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);
	LRESULT OnConnectNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDisconnectNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnAddClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnSendClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnConnectClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnHistoryClicked(WORD, UINT, HWND, BOOL&);

	void AddContentToList(LPCTSTR lpszValue);
	LPVOID GetDataFromHexString(LPCTSTR lpszHexValue, DWORD* pdwLen);

	void AddToHistoryCache();
	void SaveHistory();
	void LoadHistory();
	void UpdateAddButtonEnabled();
	void UpdateEnterFields();
	void BuildPackage(CSTXProtocol &p);
	void AddPackageData(CSTXProtocol &p, ATL::CString type, ATL::CString value);
	void AddPackageDataByte(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataWord(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataDWord(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataI64(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataFloat(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataDouble(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataUTF8(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataUnicode(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataGUID(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataRAW(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataUTF8Pair(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataUnicodePair(CSTXProtocol &p, ATL::CString value);
	void AddPackageDataUTF8DWordPair(CSTXProtocol &p, ATL::CString value);


public:
	void DoOnClose();
	void AppendResponseMsg(LPCTSTR lpszMsg);
};

