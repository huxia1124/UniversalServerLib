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
#include "TitleBar.h"

class CSTXScriptDialog : public CDialogImpl<CSTXScriptDialog>
{
public:
	CSTXScriptDialog();
	~CSTXScriptDialog();

public:
	enum {IDD = IDD_DIALOG_SERVER_SCRIPT};

protected:
	CSTXAnchor *_anchor;

	WTL::CComboBox _cbRPCHost;
	WTL::CEdit _edtScript;
	std::wstring _host;
	std::wstring _port;
	std::wstring _scriptToRun;
	std::wstring _originalDefaultScript;

	CTitleBar _titleBar;

protected:
	BEGIN_MSG_MAP(CSTXScriptDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnClose)


		COMMAND_ID_HANDLER(IDOK, OnRunScriptClicked)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancelClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_NEW_SCRIPT, OnNewScriptClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_FILE, OnOpenFileClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_ENQUEUE_WORKER_THREAD_SCRIPT, OnScheduleWorkerThreadScriptClicked)

		//COMMAND_HANDLER(IDC_COMBO_TYPE, CBN_SELCHANGE, OnCbHostSelChange)

	END_MSG_MAP()


protected:
	std::wstring GetFileContentText(LPCTSTR lpszFile, BOOL *pbResult);


	void InitializeRPCHostCombobox();
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);


	//LRESULT OnCbHostSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);


	LRESULT OnRunScriptClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnNewScriptClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnCancelClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnOpenFileClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnScheduleWorkerThreadScriptClicked(WORD, UINT, HWND, BOOL&);


	void AddToHistoryCache();
	void SaveHistory();
	void LoadHistory();
	BOOL CheckSave();

	BOOL WriteToFile(LPCTSTR lpszFile, LPCTSTR lpszText);
	void GetErrorText(RPC_STATUS NTStatusMessage, CString &err);
	void CreateTitleBar();

protected:
	void RunServerScriptFile(LPCTSTR lpszScriptFile);
	void RunServerScriptString(LPCTSTR lpszScript, CString &result, CString &err);
	void EnqueueServerWorkerThreadScriptString(LPCTSTR lpszScript, CString &err);

};

