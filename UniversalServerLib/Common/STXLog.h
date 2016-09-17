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

#include <stdio.h>
#include <WTypes.h>
#include <tchar.h>
#include <wincon.h>

#include "StackWalker.h"

#pragma warning(disable:4786)
#include <string>
#include <map>
using namespace std;

#ifndef STLSTRING
#ifdef UNICODE
#define STLSTRING wstring
#else
#define STLSTRING string
#endif
#endif


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Log Flags
#define STXLF_LOGNAME			0x00000001		//Log Name
#define STXLF_TIME				0x00000002		//Date and Time
#define STXLF_PROCESSID			0x00000004		//Process ID
#define STXLF_THREADID			0x00000008		//Thread ID
#define STXLF_ERROR_LEVEL		0x00000010		//Log Error Level
#define STXLF_CALLBACK_DEBUG	0x00000040		//Enable Debug String Output Callback
#define STXLF_CALLBACK_LOG		0x00000080		//Enable Log Output Callback

#define STXLF_CONSOLE			0x01000000		//Create a console window to output the debug-string
#define STXLF_NOTIFY_WINDOW		0x02000000		//Output the debug-string to all the windows with the caption text given to SetNotifyWindowName()

#define STXLF_ALL				0x00000FFF

// Prefix Types. Lower WORD is reserved for Console Text Attribute
#define STXLPX_ERROR_POST		0x00010000		//Post an Error Text at the end of the output. the error text is from GetLastError



//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define STXTRACELOG		g_LogGlobal.OutputLogDebugString							//Output both Debug String and Log
#define STXLOG			g_LogGlobal.OutputLog										//Output Log only
#define STXTRACET		g_LogGlobal.OutputDebugStringC								//Output Debug String only, [T]specifying text attribute
#define STXTRACE			g_LogGlobal.OutputDebugString								//Output Debug String only
#define STXTRACELOGE		g_LogGlobal.OutputLogDebugString							//Output both Debug String and Log [E]in both Debug and Release
#define STXLOGE			g_LogGlobal.OutputLog										//Output Log [E]in both Debug and Release
#define STXTRACEE		g_LogGlobal.OutputDebugString								//Output Debug String [E]in both Debug and Release
#define STXTRACELOGF		CSTXLogDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)		//Output both Debug String and Log in both Debug and Release
#define STXLOGF			CSTXLogFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)					//Output Log in both Debug and Release
#define STXTRACEF		CSTXServerDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)			//Output Debug String in both Debug and Release
#define STXTRACELOGFE	CSTXLogDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)		//Output both Debug String and Log in both Debug and Release
#define STXLOGFE			CSTXLogFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)					//Output Log in both Debug and Release
#define STXTRACEFE		CSTXServerDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)			//Output Debug String in both Debug and Release
#define STXLOGC			CSTXLogFileAndLineInfo2(__FILE__, __LINE__)								//Output Log in both Debug and Release. [C]Specifying a Log object to output to
#define STXLOGASSERT(x)\
	if(!(x))STXTRACELOGFE(_T("Assert Failed! %s\r\n%s(%d)"), _T(#x), _T(__FILE__), __LINE__);
#define STXLOGVERIFY(x)\
	if(!(x))STXTRACELOGFE(_T("Assert Failed! %s\r\n%s(%d)"), _T(#x), _T(__FILE__), __LINE__);
#else
#define STXTRACE			__noop
#define STXLOG			__noop
#define STXTRACELOG		__noop
#define STXTRACELOGE		g_LogGlobal.OutputLogDebugString
#define STXLOGE			g_LogGlobal.OutputLog
#define STXTRACEE		g_LogGlobal.OutputDebugString
#define STXTRACELOGF		__noop
#define STXLOGF			__noop
#define STXTRACEF		__noop
#define STXTRACELOGFE	CSTXLogDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)		//Output both Debug String and Log in both Debug and Release
#define STXLOGFE			CSTXLogFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)					//Output Log in both Debug and Release
#define STXTRACEFE		CSTXServerDebugStringFileAndLineInfo(&g_LogGlobal, __FILE__, __LINE__)			//Output Debug String in both Debug and Release
#define STXLOGC			CSTXLogFileAndLineInfo2(__FILE__, __LINE__)								//Output Log in both Debug and Release. Specifing a Log object to output to
#define STXLOGASSERT(x)	__noop
#define STXLOGVERIFY(x)	x
#endif

//////////////////////////////////////////////////////////////////////////
// ISTXLogCallback

class ISTXLogCallback
{
public:
	virtual void OnOutputDebugString(LPCTSTR lpszContent, BOOL bFinalCall) {};
	virtual void OnOutputLog(LPCTSTR lpszContent, BOOL bFinalCall) {};
};

//////////////////////////////////////////////////////////////////////////
// CSTXLog

class CSTXLog
{
	typedef struct tagENUMDEBUGOUTWINDOW
	{
		CSTXLog *pLogObject;
		LPCTSTR lpszDebugString;
		COPYDATASTRUCT* pCD;
	}ENUMDEBUGOUTWINDOW, *LPENUMDEBUGOUTWINDOW;

public:
	CSTXLog(void);
	virtual ~CSTXLog(void);

protected:
	DWORD m_dwLogOptions;			//Log Options. See STXLF_ macros
	CRITICAL_SECTION m_cs;
	LONG m_cchLogBufferLen;
	LPTSTR m_pLogBuffer;
	BOOL m_bFirstWrite;
	HANDLE m_hConsoleStdout;
	BOOL m_bOwnedConsole;
	TCHAR m_szOutputWindowName[MAX_PATH];
	CONSOLE_SCREEN_BUFFER_INFO m_ConsoleBufferInfo;
	ISTXLogCallback *m_pCallback;

#ifdef UNICODE
	wstring m_strLogName;
	wstring m_strLogFilePath;
#else
	string m_strLogName;
	string m_strLogFilePath;
#endif

	int m_nLogLevel;		//Log Level to show
	int m_nDebugLevel;		//Debug Level to show

private:
	DWORD AnalyseFmtPrefix(LPCTSTR *lplpszLogFmt);	//[in][out] lplpszLogFmt

public:
	void Lock();
	void Unlock();
	BOOL Uninitialize();
	BOOL Initialize(LPCTSTR lpszLogName, LPCTSTR lpszLogFile, DWORD dwLogOptions = STXLF_ALL, LONG cchLogBufferLen = 1024);
	void OutputLog(LPCTSTR lpszLogFmt, ...);
	void OutputLog(int nLevel, LPCTSTR lpszLogFmt, ...);
	void OutputLogV(LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogDirect(LPCTSTR lpszLogFmt, ...);
	void OutputLogDirect(int nLevel, LPCTSTR lpszLogFmt, ...);
	void OutputLogDirectV(LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogDirectV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputLogFull(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputLogFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);
	BOOL ModifyLogOption(DWORD dwRemove, DWORD dwAdd);
	void SetLogFilePathName(LPCTSTR lpszPathName);
	void SetNotifyWindowName(LPCTSTR lpszWindowCaption);
	BOOL SetConsoleAttribute(WORD wAttribute);
	void SetCallback(ISTXLogCallback *pCallback);

public:
	void OutputDebugStringC(WORD wConsoleTextAttribute, LPCTSTR lpszLogFmt, ...);
	void OutputDebugStringC(int nLevel, WORD wConsoleTextAttribute, LPCTSTR lpszLogFmt, ...);
	void OutputDebugString(LPCTSTR lpszLogFmt, ...);
	void OutputDebugString(int nLevel, LPCTSTR lpszLogFmt, ...);
	void OutputDebugStringV(LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputDebugStringV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputDebugStringFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputDebugStringFull(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputDebugStringFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputDebugStringFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);

	void OutputLogDebugString(LPCTSTR lpszLogFmt, ...);
	void OutputLogDebugString(int nLevel, LPCTSTR lpszLogFmt, ...);
	void OutputLogDebugStringV(LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogDebugStringV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogDebugStringFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputLogDebugStringFull(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...);
	void OutputLogDebugStringFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);
	void OutputLogDebugStringFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList);

	static void OutputDebugStringDirect(LPCTSTR lpszLogFmt, const UINT cchBufferLength, ...);
	static void OutputDebugStringDirectV(LPCTSTR lpszLogFmt, const UINT cchBufferLength, va_list _ArgList);

	static DWORD GetLastErrorText(LPTSTR lpszBuf, DWORD cchBufLen);
	static DWORD GetErrorText(LPTSTR lpszBuf, DWORD cchBufLen, DWORD dwErrorCode);

	void SetLogLevel(int nLogLevel = -1, int nDebugLevel = -1);
	int GetLogLevel()
	{
		return m_nLogLevel;
	}
	int GetDebugLevel()
	{
		return m_nDebugLevel;
	}

protected:
	static BOOL CALLBACK EnumDebugOutputWindowsProc(HWND hwnd, LPARAM lParam);

};

extern CSTXLog g_LogGlobal;		//Global Log

//////////////////////////////////////////////////////////////////////////
// CSTXLogDebugStringFileAndLineInfo

class CSTXLogDebugStringFileAndLineInfo
{
private:
	LPCSTR m_pszFileName;
	const int m_nLineNo;
	CSTXLog *m_pLogObject;

public:
	CSTXLogDebugStringFileAndLineInfo(CSTXLog* pLogObject, LPCSTR pszFileName, int nLineNo)
		: m_pLogObject(pLogObject), m_pszFileName(pszFileName), m_nLineNo(nLineNo)
	{}

	void __cdecl operator()(LPCTSTR pszFmt, ...) const
	{
		va_list ptr; va_start(ptr, pszFmt);
		m_pLogObject->OutputLogDebugStringFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
		va_end(ptr);
	}
	void __cdecl operator()(int nLevel, LPCTSTR pszFmt, ...) const
	{
		if(m_pLogObject->GetLogLevel() >= nLevel)
		{
			va_list ptr; va_start(ptr, pszFmt);
			m_pLogObject->OutputLogDebugStringFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
			va_end(ptr);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CSTXLogFileAndLineInfo

class CSTXLogFileAndLineInfo
{
private:
	LPCSTR m_pszFileName;
	const int m_nLineNo;
	CSTXLog *m_pLogObject;

public:
	CSTXLogFileAndLineInfo(CSTXLog* pLogObject, LPCSTR pszFileName, int nLineNo)
		: m_pLogObject(pLogObject), m_pszFileName(pszFileName), m_nLineNo(nLineNo)
	{}

	void __cdecl operator()(LPCTSTR pszFmt, ...) const
	{
		va_list ptr; va_start(ptr, pszFmt);
		m_pLogObject->OutputLogFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
		va_end(ptr);
	}

	void __cdecl operator()(int nLevel, LPCTSTR pszFmt, ...) const
	{
		if(m_pLogObject->GetLogLevel() >= nLevel)
		{
			va_list ptr; va_start(ptr, pszFmt);
			m_pLogObject->OutputLogFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
			va_end(ptr);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CSTXLogFileAndLineInfo2

class CSTXLogFileAndLineInfo2
{
private:
	LPCSTR m_pszFileName;
	const int m_nLineNo;

public:
	CSTXLogFileAndLineInfo2(LPCSTR pszFileName, int nLineNo)
		: m_pszFileName(pszFileName), m_nLineNo(nLineNo)
	{}

	void __cdecl operator()(CSTXLog& LogObject, LPCTSTR pszFmt, ...) const
	{
		va_list ptr; va_start(ptr, pszFmt);
		LogObject.OutputLogFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
		va_end(ptr);
	}
	void __cdecl operator()(int nLevel, CSTXLog& LogObject, LPCTSTR pszFmt, ...) const
	{
		if(LogObject.GetLogLevel() >= nLevel)
		{
			va_list ptr; va_start(ptr, pszFmt);
			LogObject.OutputLogFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
			va_end(ptr);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CSTXServerDebugStringFileAndLineInfo

class CSTXServerDebugStringFileAndLineInfo
{
private:
	LPCSTR m_pszFileName;
	const int m_nLineNo;
	CSTXLog *m_pLogObject;

public:
	CSTXServerDebugStringFileAndLineInfo(CSTXLog* pLogObject, LPCSTR pszFileName, int nLineNo)
		: m_pLogObject(pLogObject), m_pszFileName(pszFileName), m_nLineNo(nLineNo)
	{}

	void __cdecl operator()(LPCTSTR pszFmt, ...) const
	{
		va_list ptr; va_start(ptr, pszFmt);
		m_pLogObject->OutputDebugStringFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
		va_end(ptr);
	}
	void __cdecl operator()(int nLevel, LPCTSTR pszFmt, ...) const
	{
		if(m_pLogObject->GetLogLevel() >= nLevel)
		{
			va_list ptr; va_start(ptr, pszFmt);
			m_pLogObject->OutputDebugStringFullV(m_pszFileName, m_nLineNo, pszFmt, ptr);
			va_end(ptr);
		}
	}
};


//
// StackWalkerToGlobalLog

class StackWalkerToGlobalLog : public StackWalker
{
public:
	StackWalkerToGlobalLog()
	{
		m_nLogLevel = 3;
	}

protected:
	int m_nLogLevel;

public:
	void SetLogLevel(int nLogLevel = 3)
	{
		m_nLogLevel = nLogLevel;
	}

protected:
	virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
	{

	}
	virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
	{

	}
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
	{
		CHAR buffer[STACKWALK_MAX_NAMELEN] = "";
		if ( (eType != lastEntry) && (entry.offset != 0) )
		{
			if (entry.name[0] == 0)
				strcpy_s(entry.name, STACKWALK_MAX_NAMELEN, "(function-name not available)");
			if (entry.undName[0] != 0)
				strcpy_s(entry.name, STACKWALK_MAX_NAMELEN, entry.undName);
			if (entry.undFullName[0] != 0)
				strcpy_s(entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName);
			if (entry.lineFileName[0] == 0)
			{
				//strcpy_s(entry.lineFileName, STACKWALK_MAX_NAMELEN, "(filename not available)");
				if (entry.moduleName[0] == 0)
					strcpy_s(entry.moduleName, STACKWALK_MAX_NAMELEN, "(module-name not available)");
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s): %s: %s", (LPVOID) entry.offset, entry.moduleName, entry.lineFileName, entry.name);
			}
			else
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s (%d): %s", entry.lineFileName, entry.lineNumber, entry.name);
			buffer[STACKWALK_MAX_NAMELEN-1] = 0;
			g_LogGlobal.OutputLogDebugString(m_nLogLevel, _T("\t%S"), buffer);
		}
	}
};

extern StackWalkerToGlobalLog g_stackWalker;
/*
inline void ShowExceptionCallStack(CONTEXT *c = 0)
{
	g_LogGlobal.Lock();
	g_LogGlobal.OutputLogDebugString(_T("-------------------Exception CallStack Begin------------------"));
	g_stackWalker.ShowCallstack(GetCurrentThread(), c);
	g_LogGlobal.OutputLogDebugString(_T("------------------------CallStack End-------------------------\n"));
	g_LogGlobal.Unlock();
}
*/

inline void ShowExceptionCallStack(CONTEXT *c = 0, CSTXLog *pLog = NULL, int nLogLevel = 3)
{
	if (pLog == NULL)
		pLog = &g_LogGlobal;

	pLog->Lock();
	pLog->OutputLogDebugString(nLogLevel, _T("-------------------Exception CallStack Begin------------------"));
	g_stackWalker.SetLogLevel(nLogLevel);
	g_stackWalker.ShowCallstack(GetCurrentThread(), c);
	pLog->OutputLogDebugString(nLogLevel, _T("------------------------CallStack End-------------------------\n"));
	pLog->Unlock();
}

inline void ShowUserCallStack(int nLogLevel = 3)
{
	g_LogGlobal.Lock();
	STXTRACELOGE(nLogLevel, _T("[r][i] User requested callstack!"));
	g_LogGlobal.OutputLogDebugString(nLogLevel, _T("---------------------User CallStack Begin---------------------"));
	g_stackWalker.SetLogLevel(nLogLevel);
	g_stackWalker.ShowCallstack(GetCurrentThread());
	g_LogGlobal.OutputLogDebugString(nLogLevel, _T("------------------------CallStack End-------------------------\n"));
	g_LogGlobal.Unlock();
}

inline void ResetStackWalkerModuleList()
{
	g_stackWalker.ResetModuleListCache();
}