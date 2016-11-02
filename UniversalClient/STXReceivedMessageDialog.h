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
//#include "STXProtocolHistory.h"

class CSTXReceivedMessageDialog : public CDialogImpl<CSTXReceivedMessageDialog>
{
public:
	CSTXReceivedMessageDialog();
	~CSTXReceivedMessageDialog();

public:
	enum {IDD = IDD_DIALOG_MESSAGE_RECEIVED};

protected:
	CSTXAnchor *_anchor;

	WTL::CEdit _edtMessage;
	int _messageCount = 0;

protected:
	BEGIN_MSG_MAP(CSTXReceivedMessageDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		//MESSAGE_HANDLER(WM_CLOSE, OnClose)


		COMMAND_ID_HANDLER(IDC_BUTTON_BACK, OnBackClicked)
		COMMAND_ID_HANDLER(IDC_BUTTON_CLEAR, OnClearClicked)
	END_MSG_MAP()

protected:

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);


	LRESULT OnBackClicked(WORD, UINT, HWND, BOOL&);
	LRESULT OnClearClicked(WORD, UINT, HWND, BOOL&);


public:
	void ClearMessageCount();
	void UpdateNodeText();
	void AppendReceivedMessage(LPCTSTR lpszMessage);


};

