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