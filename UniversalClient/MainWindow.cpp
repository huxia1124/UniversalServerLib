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
