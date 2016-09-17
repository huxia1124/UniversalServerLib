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
#include "MainWindow.h"
#include "../UniversalServerLib/UniversalServerRPC_h.h"

#pragma comment(lib,"rpcrt4")
#pragma comment(lib,"ole32")

void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}
// Memory deallocation function for RPC.

void __RPC_USER midl_user_free(void* p)
{
	free(p);
}

CMainWindow::CMainWindow()
{
	_anchor = NULL;
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::RunServerScriptString(LPCTSTR lpszScript)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(//建立一个String Binding句柄，并不连接
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")),
		(RPC_WSTR)(_T("localhost")),
		(RPC_WSTR)(_T("4747")),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status)
	{
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		RunScriptString(hwBinding, lpszScript);
	}
	RpcExcept(1)
	{
		printf("Runtime reported exception:%d,except=%d\n", GetLastError(), RpcExceptionCode()/*RPC_S_ACCESS_DENIED==5L*/);//这里返回了5
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

	if (status)
	{
		printf("Bind free failed\n");
		exit(status);
	}
}

void CMainWindow::RunServerScriptFile(LPCTSTR lpszScriptFile)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE hwBinding;
	WCHAR* szStringBinding = NULL;

	status = RpcStringBindingCompose(//建立一个String Binding句柄，并不连接
		NULL,
		(RPC_WSTR)(_T("ncacn_ip_tcp")) ,
		(RPC_WSTR)(_T("localhost")),
		(RPC_WSTR)(_T("4747")),
		NULL,
		(RPC_WSTR*)&szStringBinding);

	if (status)
	{
		printf("StringBinding failed\n");
		exit(status);
	}
	printf("szString=%S\n", szStringBinding);

	status = RpcBindingFromStringBinding(
		(RPC_WSTR)szStringBinding,
		&hwBinding);
	if (status) {
		printf("Bind from String failed:%d\n", GetLastError());
		exit(status);
	}
	RpcTryExcept
	{
		RunScriptFile(hwBinding, lpszScriptFile);
	}
	RpcExcept(1)
	{
		printf("Runtime reported exception:%d,except=%d\n", GetLastError(), RpcExceptionCode()/*RPC_S_ACCESS_DENIED==5L*/);//这里返回了5
	}
	RpcEndExcept
	{
		status = RpcBindingFree(&hwBinding); // Frees the binding handle.
	}

	if (status)
	{
		printf("Bind free failed\n");
		exit(status);
	}
}
