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
#include "UniversalServerInternal.h"
#include <atlconv.h>
#include <atlexcept.h>
#include "STXLog.h"
#include "STXIOCPBuffer.h"

CSTXLog g_LogGlobalInitial;

CUniversalServer::CUniversalServer() : _spController(new CServerController(""))
{
	_luaVersion = _T("");
	_tickStart = GetTickCount64();
	_callback = NULL;
	_spController->_pServer = this;
	L = luaL_newstate();  /* create state */
	InitializeCriticalSection(&_csMainScript);

	TCHAR szFile[MAX_PATH];
	GetModuleFileName(NULL, szFile, MAX_PATH);
	g_LogGlobalInitial.Initialize(szFile, NULL, STXLF_TIME | STXLF_THREADID | STXLF_ERROR_LEVEL, 32768);
}

CUniversalServer::~CUniversalServer()
{
	g_LogGlobalInitial.Uninitialize();
	DeleteCriticalSection(&_csMainScript);
	lua_close(L);
}

int CUniversalServer::ProcessException(LPEXCEPTION_POINTERS pExp)
{
	CONTEXT c;
	if (pExp != NULL && pExp->ExceptionRecord != NULL)
	{
		memcpy(&c, pExp->ContextRecord, sizeof(CONTEXT));
		//c.ContextFlags = CONTEXT_FULL;

		TCHAR szExceptionText[1024];
		switch (pExp->ExceptionRecord->ExceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:
			_stprintf_s(szExceptionText, _T("Access violation %s location 0x%p"), (pExp->ExceptionRecord->ExceptionInformation[0] ? _T("writing to") : _T("reading from")), (void*)pExp->ExceptionRecord->ExceptionInformation[1]);
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			_stprintf_s(szExceptionText, _T("Accessed array out of bounds")); break;
			break;
		case EXCEPTION_BREAKPOINT:
			_stprintf_s(szExceptionText, _T("Hit breakpoint")); break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			_stprintf_s(szExceptionText, _T("Data misaligned")); break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			_stprintf_s(szExceptionText, _T("FPU denormal operand")); break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			_stprintf_s(szExceptionText, _T("FPU divide by zero")); break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			_stprintf_s(szExceptionText, _T("FPU inexact result")); break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			_stprintf_s(szExceptionText, _T("FPU invalid operation")); break;
		case EXCEPTION_FLT_OVERFLOW:
			_stprintf_s(szExceptionText, _T("FPU overflow")); break;
		case EXCEPTION_FLT_STACK_CHECK:
			_stprintf_s(szExceptionText, _T("FPU stack fault")); break;
		case EXCEPTION_FLT_UNDERFLOW:
			_stprintf_s(szExceptionText, _T("FPU underflow")); break;
		case EXCEPTION_GUARD_PAGE:
			_stprintf_s(szExceptionText, _T("Attempt to access guard page")); break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			_stprintf_s(szExceptionText, _T("Attempt to execeute illegal instruction")); break;
		case EXCEPTION_IN_PAGE_ERROR:
			_stprintf_s(szExceptionText, _T("Page fault")); break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			_stprintf_s(szExceptionText, _T("Integer divide by zero")); break;
		case EXCEPTION_INT_OVERFLOW:
			_stprintf_s(szExceptionText, _T("Integer overflow")); break;
		case EXCEPTION_INVALID_DISPOSITION:
			_stprintf_s(szExceptionText, _T("Badly handled exception")); break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			_stprintf_s(szExceptionText, _T("Exception during exception handling")); break;
		case EXCEPTION_PRIV_INSTRUCTION:
			_stprintf_s(szExceptionText, _T("Attempt to execute a privileged instruction")); break;
		case EXCEPTION_SINGLE_STEP:
			_stprintf_s(szExceptionText, _T("Processor in single step mode")); break;
		case EXCEPTION_STACK_OVERFLOW:
			_stprintf_s(szExceptionText, _T("Stack overflow")); break;
		default:
			_stprintf_s(szExceptionText, _T("Unknown exception code")); break;
		}

		g_LogGlobalInitial.Lock();
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]**Exception** - %s"), szExceptionText);
		ShowExceptionCallStack(&c, &g_LogGlobalInitial);
		g_LogGlobalInitial.Unlock();
	}
	else
	{
		g_LogGlobalInitial.Lock();
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]Unknown exception!"));
		ShowExceptionCallStack(NULL, &g_LogGlobalInitial);
		g_LogGlobalInitial.Unlock();
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void CUniversalServer::CallFunctionImpl(LPCTSTR lpszFunctionName)
{
	USES_CONVERSION;
	std::string buf = (LPCSTR)ATL::CW2A(lpszFunctionName);
	LuaIntf::LuaRef luaFunc(L, buf.c_str());
	luaFunc();
}

extern "C"
static int UniversalServer_lua_writer(lua_State *L, const void *p, size_t size, void *u)
{
	CSTXIOCPBuffer *pBuffer = (CSTXIOCPBuffer*)u;
	pBuffer->WriteBuffer((LPVOID)p, size);
	return 0;
}

extern "C" static const char * UniversalServer_lua_reader(lua_State *L,
	void *data,
	size_t *size)
{
	CSTXIOCPBuffer *pBuffer = (CSTXIOCPBuffer*)data;
	*size = pBuffer->GetDataLength();
	return (const char *)pBuffer->GetBufferPtr();
}

int CUniversalServer::Run(LPCTSTR lpszScriptFile)
{
	USES_CONVERSION;
	std::string buf = (LPCSTR)ATL::CW2A(lpszScriptFile);
	int nResult = luaL_loadfile(L, buf.c_str());
	if (nResult != 0)
	{
		const char *pLuaLoadError = lua_tostring(L, -1);
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]%s Script failed to run."), lpszScriptFile);
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]\t\t%S"), pLuaLoadError);
		return nResult;
	}

	//CSTXIOCPBuffer b;
	//lua_dump(L, UniversalServer_lua_writer, &b, 0);
	//lua_load(L, UniversalServer_lua_reader, &b, "script", "b");

	nResult = lua_pcall(L, 0, LUA_MULTRET, 0);
	return nResult;
}

void CUniversalServer::CallFunction(LPCTSTR lpszFunctionName)
{
	__try
	{
		__try
		{

			CallFunctionImpl(lpszFunctionName);
		}
		__except (ProcessException(GetExceptionInformation()))
		{

		}
	}
	__finally
	{

	}
}

LPVOID CUniversalServer::GetMainLuaState()
{
	return L;
}

int CUniversalServer::RunScriptString(LPCTSTR lpszScriptString)
{
	USES_CONVERSION;
	std::string buf = (LPCSTR)ATL::CW2A(lpszScriptString);

	int nResult = 0;
	EnterCriticalSection(&_csMainScript);
	nResult = luaL_dostring(L, buf.c_str());
	if (nResult)
	{
		const char *pLuaLoadError = lua_tostring(L, -1);
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]Script text failed to run."));
		g_LogGlobalInitial.OutputLogDebugString(_T("[r][i]\t\t%S"), pLuaLoadError);
	}
	LeaveCriticalSection(&_csMainScript);
	return nResult;
}

void CUniversalServer::SetServerCallback(IUniversalServerCallback * pCallback)
{
	_callback = pCallback;
}

void CUniversalServer::Initialize()
{
	luaopen_base(L);
	luaL_openlibs(L);

	USES_CONVERSION;

	_luaVersion = ATL::CA2W(LUA_RELEASE);
}

void CUniversalServer::BindClasses()
{
	LuaBindClasses(L, this);
}

ULONGLONG CUniversalServer::GetRunTime()
{
	return GetTickCount64() - _tickStart;
}

LPCTSTR CUniversalServer::GetLuaVersion()
{
	return _luaVersion.c_str();
}

void CUniversalServer::Terminate()
{
	_spController->Terminate();
}
