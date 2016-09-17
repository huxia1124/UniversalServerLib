// UniversalClient.cpp : 定义应用程序的入口点。
//

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
		WSADATA wsd;
		if (WSAStartup(MAKEWORD(1, 1), &wsd) == 0)
		{
			CMainWindow wnd;
			wnd.Create(NULL, NULL, _T("UniversalClient"));
			wnd.ShowWindow(SW_SHOW);

			RunMessageLoop();

			WSACleanup();
		}
		else
		{
			//DebugBreak();
		}

		return 0;
	}
};

