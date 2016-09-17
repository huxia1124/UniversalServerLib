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

IUniversalServer *g_pServer = NULL;
class CMyCallback : public IUniversalServerCallback
{
public:
	virtual void OnServerFileChanged(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName)
	{
		static LPCTSTR actionText[10] = { 0, _T("Added"), _T("Removed"), _T("Modified"), _T("Renaming"), _T("Renamed"), 0, 0, _T("Refreshed"), 0 };
		printf("------OnServerFileChanged Callback Called----%S: %S\n", actionText[dwAction], lpszRelativePathName);

	}
	virtual void OnOutputDebugString(LPCTSTR lpszContent)
	{
	};
	virtual void OnOutputLog(LPCTSTR lpszContent)
	{
	};

};


HANDLE hEvent = NULL;

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
	g_pServer->CallFunction(_T("stop"));
	SetEvent(hEvent);
	return TRUE;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	LPTSTR lpszName = PathFindFileName(szPath);
	*lpszName = 0;
	SetCurrentDirectory(szPath);

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
	IUniversalServer *pServer = CreateUniversalServer();
	g_pServer = pServer;
	CMyCallback callback;

	pServer->SetServerCallback(&callback);
	printf("Lua version: %S\n", pServer->GetLuaVersion());

	int nLuaResult = pServer->Run(_T("scripts/server_test.lua"));
	if (nLuaResult == LUA_ERRSYNTAX)
	{
		printf("Error: Syntax error!\n");
	}
	if (nLuaResult)
	{
		return nLuaResult;
	}


	//pServer->CallFunction(_T("show_stop_time"));
	pServer->CallFunction(_T("start"));

	
	std::future<int> f2 = std::async(std::launch::async, [&]() {
		
		int nButtonPressed = 0;

		//TaskDialog(NULL, NULL,
		//	_T("UniversalServerLibTest"),
		//	_T("Server is running"),
		//	_T("Close this dialog to terminate the server."),
		//	TDCBF_OK_BUTTON,
		//	TD_INFORMATION_ICON,
		//	&nButtonPressed);

		HRESULT hr;
		TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };

		tdc.hwndParent = NULL;
		tdc.dwCommonButtons = TDCBF_CLOSE_BUTTON;
		tdc.pszWindowTitle = _T("UniversalServerLibTest");
		tdc.pszMainIcon = TD_INFORMATION_ICON;
		tdc.pszMainInstruction = _T("Server is running");
		tdc.pszContent = _T("Close this dialog to terminate the server.");
		tdc.pfCallback = TaskDialogCallbackProc;
		tdc.dwFlags = TDF_CALLBACK_TIMER;

		hr = TaskDialogIndirect(&tdc, &nButtonPressed, NULL, NULL);

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

		//LuaIntf::LuaRef waitTime((lua_State*)pServer->GetMainLuaState(), "waitTime");
		//int nWaitTime = waitTime.toValue<int>();
		//if (nWaitTime != 0)
		//{
		//	Sleep(nWaitTime);
		//}
		//else
		//{
		//	Sleep(0x7FFFFFFF);
		//}
		pServer->Terminate();
		//pServer->CallFunction(_T("stop"));
		return 0;

	});
	f2.wait(); //output: 8
	

	//WaitForSingleObject(hEvent, INFINITE);

	return 0;
}

