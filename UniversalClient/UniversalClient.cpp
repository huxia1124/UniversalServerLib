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
#include "UniversalClient.h"
#include "MainWindow.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlplus.h>

[module(exe, name="UniversalClient")]
class CUniversalClientModule
{
public:
	int WinMain(int nCmdShow)
	{
		CoInitialize(NULL);
		ULONG_PTR m_gdiplusToken = 0;
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

		WSADATA wsd;
		if (WSAStartup(MAKEWORD(1, 1), &wsd) == 0)
		{
			CMainWindow wnd;
			wnd.Create(NULL, NULL, _T("Server Management Tool"));
			wnd.ShowWindow(SW_SHOW);

			RunMessageLoop();

			WSACleanup();
		}
		else
		{
			//DebugBreak();
		}

		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		CoUninitialize();

		return 0;
	}
};

