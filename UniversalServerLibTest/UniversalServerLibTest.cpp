// UniversalServerLibTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "UniversalServer.h"
#include <future>
#include <thread>
#include <Commctrl.h>

extern "C"
{
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

}

#include <LuaIntf/LuaIntf.h>

#pragma comment(lib, "Comctl32.lib")

IUniversalServer *g_pServer = NULL;		//Global pointer to the server object

//////////////////////////////////////////////////////////////////////////
// CMyCallback - Callback to respond with server events

class CMyCallback : public IUniversalServerCallback
{
public:
	virtual void OnServerFileChanged(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName, BOOL *pSkipScript)
	{
		static LPCTSTR actionText[10] = { 0, _T("Added"), _T("Removed"), _T("Modified"), _T("Renaming"), _T("Renamed"), 0, 0, _T("Refreshed"), 0 };
		printf("------OnServerFileChanged Callback Called----%S: %S\n", actionText[dwAction], lpszRelativePathName);

	}
	virtual void OnOutputDebugString(LPCTSTR lpszContent, BOOL *pSkipScript)
	{
	};
	virtual void OnOutputLog(LPCTSTR lpszContent, BOOL *pSkipScript)
	{
	};

};

//////////////////////////////////////////////////////////////////////////
// ConsoleHandler to respond the event when the user click tne close box of the console window

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
	g_pServer->CallFunction(_T("stop"));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// TaskDialog

HRESULT CALLBACK TaskDialogCallbackProc(
	_In_ HWND     hwnd,
	_In_ UINT     uNotification,
	_In_ WPARAM   wParam,
	_In_ LPARAM   lParam,
	_In_ LONG_PTR dwRefData
	)
{
	if (uNotification == TDN_TIMER)
	{

		TCHAR szTime[MAX_PATH];
		StrFromTimeInterval(szTime, MAX_PATH, (DWORD)g_pServer->GetRunTime(), 6);
		TCHAR szText[MAX_PATH];
		_stprintf_s(szText, _T("Server is running for %s"), szTime);
		::SendMessage(hwnd, TDM_SET_ELEMENT_TEXT, TDE_MAIN_INSTRUCTION, (LPARAM)szText);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Entry point

int _tmain(int argc, _TCHAR* argv[])
{
	//Get the path of the executable file, saved in szPath
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	LPTSTR lpszName = PathFindFileName(szPath);
	*lpszName = 0;

	//Set the working directory to szPath
	SetCurrentDirectory(szPath);

	//Set the console window event handler
	SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);

	//Create the server
	IUniversalServer *pServer = CreateUniversalServer();
	g_pServer = pServer;

	//Set callback for server events
	CMyCallback callback;
	pServer->SetServerCallback(&callback);

	//Lua version
	printf("Lua version: %S\n", pServer->GetLuaVersion());

	LPCTSTR pszStartupScriptFile = _T("scripts/server_test.lua");
	LPCTSTR pszStartupFunction = _T("start");

	//Execute a lua script
	int nLuaResult = pServer->Run(pszStartupScriptFile);
	if (nLuaResult == LUA_ERRSYNTAX)
	{
		printf("Error: Syntax error!\n");
	}
	if (nLuaResult)
	{
		return nLuaResult;
	}

	//Execute the function in lua previously loaded
	pServer->CallFunction(pszStartupFunction);


	std::future<int> f2 = std::async(std::launch::async, [&]() {
		
		int nButtonPressed = 0;

		HRESULT hr;
		TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };

		std::wstring content = _T("This server is configured as following:\n");
		content += _T("\nStartup script file:  ");
		content += pszStartupScriptFile;
		content += _T("\nEntry function:  ");
		content += pszStartupFunction;

		tdc.hwndParent = NULL;
		tdc.dwCommonButtons = TDCBF_CLOSE_BUTTON;
		tdc.pszWindowTitle = _T("UniversalServerLibTest");
		tdc.pszMainIcon = TD_INFORMATION_ICON;
		tdc.pszMainInstruction = _T("Server is running");
		tdc.pszContent = content.c_str();
		tdc.pfCallback = TaskDialogCallbackProc;
		tdc.dwFlags = TDF_CALLBACK_TIMER;
		tdc.pszFooterIcon = TD_WARNING_ICON;
		tdc.pszFooter = _T("Close this dialog to terminate the server.");

		hr = TaskDialogIndirect(&tdc, &nButtonPressed, NULL, NULL);

		/*
		if (SUCCEEDED(hr) && IDYES == nButtonPressed)
		{
			// download the update...
		}

		if (IDOK == nButtonPressed)
		{
			// OK button pressed
		}
		else if (IDCANCEL == nButtonPressed)
		{
			// Cancel pressed
		}
		*/


		pServer->Terminate();
		return 0;

	});
	f2.wait();
	
	return 0;
}

