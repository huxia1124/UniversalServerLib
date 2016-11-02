#include "stdafx.h"
#include "STXReceivedMessageDialog.h"
#include "MainContainerWindow.h"
#include <atldlgs.h>

#include "Common.h"
#include <atltime.h>


CSTXReceivedMessageDialog::CSTXReceivedMessageDialog()
{
	_anchor = NULL;
}


CSTXReceivedMessageDialog::~CSTXReceivedMessageDialog()
{
}

LRESULT CSTXReceivedMessageDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	//CenterWindow();

	_anchor = new CSTXAnchor(m_hWnd);

	_edtMessage.Attach(GetDlgItem(IDC_EDIT_MESSAGE_RECEIVED));

	_anchor->AddItem(IDC_EDIT_MESSAGE_RECEIVED, STXANCHOR_ALL);
	_anchor->AddItem(IDC_BUTTON_BACK, STXANCHOR_RIGHT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_BUTTON_CLEAR, STXANCHOR_RIGHT | STXANCHOR_TOP);
	_anchor->AddItem(IDC_STATIC_TITLE, STXANCHOR_LEFT | STXANCHOR_RIGHT | STXANCHOR_TOP);

	CenterWindow(::GetParent(m_hWnd));

	return 1; // Let dialog manager set initial focus
}

LRESULT CSTXReceivedMessageDialog::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
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

LRESULT CSTXReceivedMessageDialog::OnBackClicked(WORD, UINT, HWND, BOOL&)
{
	GetParent().SendMessage(WM_SWITCH_TO_PROTOCOL_TEST);
	return 0;
}

LRESULT CSTXReceivedMessageDialog::OnClearClicked(WORD, UINT, HWND, BOOL&)
{
	_edtMessage.SetWindowText(_T(""));
	return 0;
}

void CSTXReceivedMessageDialog::UpdateNodeText()
{
	GetParent().SendMessage(WM_UPDATE_RECEIVED_MESSAGE_COUNT, (WPARAM)_messageCount);
}

void CSTXReceivedMessageDialog::ClearMessageCount()
{
	_messageCount = 0;
}

void CSTXReceivedMessageDialog::AppendReceivedMessage(LPCTSTR lpszMessage)
{
	_messageCount++;
	UpdateNodeText();

	CString strTimeStamp;
	ATL::CTime tm = ATL::CTime::GetCurrentTime();
	strTimeStamp = tm.Format(_T("[ %m/%d/%Y %H:%M:%S ]\r\n"));
	_edtMessage.AppendText(strTimeStamp, FALSE, FALSE);
	_edtMessage.AppendText(lpszMessage, FALSE, FALSE);
	_edtMessage.AppendText(_T("\r\n\r\n"), FALSE, FALSE);

	int len = _edtMessage.GetWindowTextLength();

	_edtMessage.SetSel(len, len);
	_edtMessage.Scroll(SB_BOTTOM);
}
