#pragma once

#include <wtypes.h>

class IUniversalServerCallback
{
public:
	virtual void OnServerFileChanged(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName, BOOL *pSkipScript) {};

	virtual void OnTcpClientConnected(LPCTSTR lpszServerParam, LONGLONG clientID, BOOL *pSkipScript) {}
	virtual void OnTcpClientDisconnected(LPCTSTR lpszServerParam, LONGLONG clientID, BOOL *pSkipScript) {}
	virtual void OnTcpClientReceived(LPCTSTR lpszServerParam, LONGLONG clientID, LPVOID lpData, ULONGLONG cbDataSize, BOOL *pSkipScript) {}

	virtual void OnTcpConnectionConnected(LPCTSTR lpszConnectionParam, LONGLONG connectionID, BOOL *pSkipScript) {}
	virtual void OnTcpConnectionDisconnected(LPCTSTR lpszConnectionParam, LONGLONG connectionID, BOOL *pSkipScript) {}
	virtual void OnTcpConnectionReceived(LPCTSTR lpszConnectionParam, LONGLONG connectionID, LPVOID lpData, ULONGLONG cbDataSize, BOOL *pSkipScript) {}

	virtual void OnOutputDebugString(LPCTSTR lpszContent, BOOL *pSkipScript) {};
	virtual void OnOutputLog(LPCTSTR lpszContent, BOOL *pSkipScript) {};
};


class IUniversalServer
{

public:
	virtual int Run(LPCTSTR lpszScriptFile) = 0;
	virtual int RunScriptString(LPCTSTR lpszScriptString) = 0;
	virtual void CallFunction(LPCTSTR lpszFunctionName) = 0;
	virtual LPVOID GetMainLuaState() = 0;
	virtual void SetServerCallback(IUniversalServerCallback *pCallback) = 0;
	virtual ULONGLONG GetRunTime() = 0;
	virtual LPCTSTR GetLuaVersion() = 0;
	virtual void Terminate() = 0;

};


IUniversalServer *CreateUniversalServer();