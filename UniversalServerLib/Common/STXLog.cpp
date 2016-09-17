//#include "Global.h"
#include "STXLog.h"
#include <io.h>
#include <fcntl.h>
#include <time.h>

//#pragma init_seg(compiler)

//////////////////////////////////////////////////////////////////////////

StackWalkerToGlobalLog g_stackWalker;


//////////////////////////////////////////////////////////////////////////
// CSTXLog

CSTXLog::CSTXLog(void)
{
	m_pCallback = NULL;
	m_dwLogOptions = 0;
	m_cchLogBufferLen = 2048;
	m_pLogBuffer = new TCHAR[m_cchLogBufferLen];
	m_bFirstWrite = TRUE;
	m_hConsoleStdout = INVALID_HANDLE_VALUE;
	m_bOwnedConsole = FALSE;
	m_szOutputWindowName[0] = 0;
	TCHAR szLogFile[MAX_PATH];
	::GetModuleFileName(NULL, szLogFile, MAX_PATH);
	lstrcat(szLogFile, _T(".log"));
	m_strLogFilePath = szLogFile;
	InitializeCriticalSection(&m_cs);

	m_nDebugLevel = 3;
	m_nLogLevel = 3;
}

CSTXLog::~CSTXLog(void)
{
	Uninitialize();
	DeleteCriticalSection(&m_cs);
}

BOOL CSTXLog::Uninitialize()
{
	if(m_pLogBuffer == NULL)
	{
		return FALSE;
	}

	if(m_pLogBuffer != NULL)
	{
		delete []m_pLogBuffer;
		m_pLogBuffer = NULL;
	}
	return TRUE;
}

void CSTXLog::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CSTXLog::Unlock()
{
	LeaveCriticalSection(&m_cs);
}

BOOL CSTXLog::Initialize(LPCTSTR lpszLogName, LPCTSTR lpszLogFile, DWORD dwLogOptions /* = STXLOG_OPTION_ALL */, LONG cchLogBufferLen /* = 1024 */)
{
	m_cchLogBufferLen = cchLogBufferLen;
	if(lpszLogFile)
		m_strLogFilePath = lpszLogFile;

	m_strLogName = lpszLogName;
	m_dwLogOptions = dwLogOptions;
	if(m_pLogBuffer != NULL)
	{
		delete []m_pLogBuffer;
		m_pLogBuffer = NULL;
	}
	m_pLogBuffer = new TCHAR[m_cchLogBufferLen];

	if(m_dwLogOptions & STXLF_CONSOLE)
	{
		m_bOwnedConsole = AllocConsole();
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleStdout = hStdout;
		GetConsoleScreenBufferInfo(m_hConsoleStdout, &m_ConsoleBufferInfo);
	}
	return TRUE;
}

void CSTXLog::OutputLogFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputLogFullV(lpszSourceFile, nLine, lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputLogFull( int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ... )
{
	if(m_nLogLevel >= nLevel)
	{
		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputLogFullV(nLevel, lpszSourceFile, nLine, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputLogFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputLogFullV(m_nLogLevel, lpszSourceFile, nLine, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputLogFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	LPTSTR pszFmt = new TCHAR[m_cchLogBufferLen + MAX_PATH];
#ifdef UNICODE
	WCHAR szSourceFileW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpszSourceFile, -1, szSourceFileW, MAX_PATH);
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), szSourceFileW, nLine, lpszLogFmt);
	OutputLogV(nLevel, pszFmt, _ArgList);
#else
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), lpszSourceFile, nLine, lpszLogFmt);
	OutputLogV(nLevel, pszFmt, _ArgList);
#endif
	delete []pszFmt;
}

void CSTXLog::OutputLog(LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputLogV(lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputLog( int nLevel, LPCTSTR lpszLogFmt, ... )
{
	if(m_nLogLevel >= nLevel)
	{
		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputLogV(nLevel, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputLogV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	if(lpszLogFmt == NULL)
		return;

	DWORD dwLastError = ::GetLastError();

	EnterCriticalSection(&m_cs);

	FILE* pLogFile = NULL;
	_tfopen_s(&pLogFile, m_strLogFilePath.c_str(), _T("ab+"));
	if(pLogFile == NULL)
	{
		LeaveCriticalSection(&m_cs);
		return;
	}

	DWORD dwExtraPrefix = AnalyseFmtPrefix(&lpszLogFmt);

/*	int nOffset = 0;
	if(lpszLogFmt[0] == 0 || lpszLogFmt[1] == 0 || lpszLogFmt[2] == 0)
		nOffset = 0;
	else if(lpszLogFmt[0] == '[' && lpszLogFmt[1] == 'E' && lpszLogFmt[2] == ']')
		nOffset = 3;
*/

	TCHAR szErrBuffer[MAX_PATH] = _T("");
	if(dwExtraPrefix & STXLPX_ERROR_POST)
	{
		TCHAR szErrBufferUnit[MAX_PATH];
		GetErrorText(szErrBufferUnit, MAX_PATH, dwLastError);
		_stprintf_s(szErrBuffer, MAX_PATH, _T(" * 0x%.8X (%d) : %s\r\n"), dwLastError, dwLastError, szErrBufferUnit);
		::OutputDebugString(szErrBuffer);
	}

#ifdef UNICODE
	if(m_bFirstWrite)
	{
		//Write UNICODE text prefix
		fputc(0xFF, pLogFile);
		fputc(0xFE, pLogFile);
		m_bFirstWrite = FALSE;
	}
#endif

	if(m_dwLogOptions & STXLF_TIME)
	{
		TCHAR szDate[64], szTime[64];
		_tstrdate_s(szDate, 64);
		_fputts(szDate, pLogFile);
		_fputtc(_T(' '), pLogFile);
		_tstrtime_s(szTime, 64);
		_fputts(szTime, pLogFile);
	}

 	if(m_dwLogOptions & STXLF_ERROR_LEVEL)
 	{
		_fputts(_T("[EL:"), pLogFile);
		_ftprintf(pLogFile, _T("%d"), nLevel);
		_fputtc(_T(']'), pLogFile);
 	}

	if(m_dwLogOptions & STXLF_LOGNAME)
	{
		_fputts(_T("[N:"), pLogFile);
		_fputts(m_strLogName.c_str(), pLogFile);
		_fputtc(_T(']'), pLogFile);
	}

	if(m_dwLogOptions & STXLF_PROCESSID)
	{
		_fputts(_T("[P:"), pLogFile);
		_ftprintf(pLogFile, _T("0x%X"), ::GetCurrentProcessId());
		_fputtc(_T(']'), pLogFile);
	}

	if(m_dwLogOptions & STXLF_THREADID)
	{
		_fputts(_T("[T:"), pLogFile);
		_ftprintf(pLogFile, _T("0x%X"), ::GetCurrentThreadId());
		_fputtc(_T(']'), pLogFile);
	}

	if(m_dwLogOptions != 0)
		_fputts(_T(" "), pLogFile);

	_vftprintf(pLogFile, lpszLogFmt, _ArgList);

	if(!(dwExtraPrefix & STXLPX_ERROR_POST) || ((dwExtraPrefix & STXLPX_ERROR_POST) && lpszLogFmt[0] != 0))
		_fputts(_T("\r\n"), pLogFile);

	if(dwExtraPrefix & STXLPX_ERROR_POST)
		_fputts(szErrBuffer, pLogFile);

	if(pLogFile != NULL)
	{
		fclose(pLogFile);
		pLogFile = NULL;
	}


	if ((m_dwLogOptions & STXLF_CALLBACK_LOG) && m_pCallback)
	{
		if (m_dwLogOptions & STXLF_TIME)
		{
			TCHAR szDate[64], szTime[64];
			_tstrdate_s(szDate, 64);
			m_pCallback->OnOutputLog(szDate, FALSE);
			m_pCallback->OnOutputLog(_T(" "), FALSE);
			_tstrtime_s(szTime, 64);
			m_pCallback->OnOutputLog(szTime, FALSE);
		}

		if (m_dwLogOptions & STXLF_ERROR_LEVEL)
		{
			TCHAR szLogLevel[8];
			_itot_s(nLevel, szLogLevel, 10);
			m_pCallback->OnOutputLog(_T("[EL:"), FALSE);
			m_pCallback->OnOutputLog(szLogLevel, FALSE);
			m_pCallback->OnOutputLog(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_LOGNAME)
		{
			m_pCallback->OnOutputLog(_T("[N:"), FALSE);
			m_pCallback->OnOutputLog(m_strLogName.c_str(), FALSE);
			m_pCallback->OnOutputLog(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_PROCESSID)
		{
			TCHAR szProcessID[32];
			_itot_s(::GetCurrentProcessId(), szProcessID, 16);
			m_pCallback->OnOutputLog(_T("[P:0x"), FALSE);
			m_pCallback->OnOutputLog(szProcessID, FALSE);
			m_pCallback->OnOutputLog(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_THREADID)
		{
			TCHAR szThreadID[32];
			_itot_s(::GetCurrentThreadId(), szThreadID, 16);
			m_pCallback->OnOutputLog(_T("[T:0x"), FALSE);
			m_pCallback->OnOutputLog(szThreadID, FALSE);
			m_pCallback->OnOutputLog(_T("]"), FALSE);
		}

		_vstprintf_s(m_pLogBuffer, m_cchLogBufferLen, lpszLogFmt, _ArgList);
		m_pCallback->OnOutputLog(m_pLogBuffer, !(dwExtraPrefix & STXLPX_ERROR_POST));
		if(!(dwExtraPrefix & STXLPX_ERROR_POST) || ((dwExtraPrefix & STXLPX_ERROR_POST) && lpszLogFmt[0] != 0))
			m_pCallback->OnOutputLog(_T("\r\n"), FALSE);

		if(dwExtraPrefix & STXLPX_ERROR_POST)
			m_pCallback->OnOutputLog(szErrBuffer, TRUE);
	}

	LeaveCriticalSection(&m_cs);

}

void CSTXLog::OutputLogV(LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputLogV(m_nLogLevel, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputLogDirect(LPCTSTR lpszLogFmt, ...)
{
	va_list marker;

	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputLogDirectV(lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputLogDirect( int nLevel, LPCTSTR lpszLogFmt, ... )
{
	if(m_nLogLevel >= nLevel)
	{
		va_list marker;

		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputLogDirectV(nLevel, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}
void CSTXLog::OutputLogDirectV(LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputLogDirectV(m_nLogLevel, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputLogDirectV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	EnterCriticalSection(&m_cs);

	FILE* pLogFile = NULL;
	_tfopen_s(&pLogFile, m_strLogFilePath.c_str(), _T("ab+"));
	if(pLogFile == NULL)
	{
		LeaveCriticalSection(&m_cs);
		return;
	}

#ifdef UNICODE
	if(m_bFirstWrite)
	{
		//Write UNICODE text prefix
		fputc(0xFF, pLogFile);
		fputc(0xFE, pLogFile);
		m_bFirstWrite = FALSE;
	}
#endif

	_vftprintf(pLogFile, lpszLogFmt, _ArgList);

	_fputts(_T("\r\n"), pLogFile);

	if(pLogFile != NULL)
	{
		fclose(pLogFile);
		pLogFile = NULL;
	}

	LeaveCriticalSection(&m_cs);
}

void CSTXLog::OutputDebugStringFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputDebugStringFullV(lpszSourceFile, nLine, lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputDebugStringFull( int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ... )
{
	if(m_nDebugLevel >= nLevel)
	{
		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputDebugStringFullV(nLevel, lpszSourceFile, nLine, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputDebugStringFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputDebugStringFullV(m_nDebugLevel, lpszSourceFile, nLine, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputDebugStringFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	LPTSTR pszFmt = new TCHAR[m_cchLogBufferLen + MAX_PATH];
#ifdef UNICODE
	WCHAR szSourceFileW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpszSourceFile, -1, szSourceFileW, MAX_PATH);
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), szSourceFileW, nLine, lpszLogFmt);
	OutputDebugStringV(nLevel, pszFmt, _ArgList);
#else
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), lpszSourceFile, nLine, lpszLogFmt);
	OutputDebugStringV(nLevel, pszFmt, _ArgList);
#endif
	delete []pszFmt;
}

void CSTXLog::OutputDebugString(LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputDebugStringV(lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputDebugString( int nLevel, LPCTSTR lpszLogFmt, ... )
{
	if(m_nDebugLevel >= nLevel)
	{
		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputDebugStringV(nLevel, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputDebugStringC(WORD wConsoleTextAttribute, LPCTSTR lpszLogFmt, ...)
{
	BOOL bSetConsoleAttribute = SetConsoleAttribute(wConsoleTextAttribute);

	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputDebugStringV(lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */

	if(bSetConsoleAttribute)
		SetConsoleAttribute(m_ConsoleBufferInfo.wAttributes);
}

void CSTXLog::OutputDebugStringC( int nLevel, WORD wConsoleTextAttribute, LPCTSTR lpszLogFmt, ... )
{
	if(m_nDebugLevel >= nLevel)
	{
		BOOL bSetConsoleAttribute = SetConsoleAttribute(wConsoleTextAttribute);

		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputDebugStringV(nLevel, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */

		if(bSetConsoleAttribute)
			SetConsoleAttribute(m_ConsoleBufferInfo.wAttributes);
	}
}

DWORD CSTXLog::AnalyseFmtPrefix(LPCTSTR *lplpszLogFmt)
{
	if(lplpszLogFmt == NULL)
		return 0;

	static TCHAR szColorMap[][4]={_T("[b]"), _T("[g]"), _T("[r]"), _T("[i]"), _T("[B]"), _T("[G]"), _T("[R]"), _T("[I]")};

	DWORD dwResult = 0;
	WORD wConsoleAttr = 0;
	LPCTSTR pszTemp = *lplpszLogFmt;
	BOOL bContinue = TRUE;
	while(bContinue)
	{
		if(pszTemp[0] == 0 || pszTemp[1] == 0 || pszTemp[2] == 0)
			break;
		else if(pszTemp[0] == '[' && pszTemp[1] == 'E' && pszTemp[2] == ']')
		{
			dwResult |= STXLPX_ERROR_POST;
			pszTemp += 3;
			continue;
		}
		else
		{
			int i = 0;
			for(i=0;i<sizeof(szColorMap) / sizeof(szColorMap[0]);i++)
			{
				if(pszTemp[0] == szColorMap[i][0] && pszTemp[1] == szColorMap[i][1] && pszTemp[2] == szColorMap[i][2])
				{
					wConsoleAttr |= (1 << i);
					break;
				}
			}
			if(i == sizeof(szColorMap) / sizeof(szColorMap[0]))
				bContinue = FALSE;
			else
			{
				pszTemp += 3;
				continue;
			}
		}
		bContinue = FALSE;
	}
	*lplpszLogFmt = pszTemp;
	return (dwResult | wConsoleAttr);
}

void CSTXLog::OutputDebugStringV(LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputDebugStringV(m_nLogLevel, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputDebugStringV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	if(lpszLogFmt == NULL)
		return;

	DWORD dwLastError = ::GetLastError();

	EnterCriticalSection(&m_cs);

	TCHAR szCRLF[4] = _T("\r\n");

	DWORD dwExtraPrefix = AnalyseFmtPrefix(&lpszLogFmt);

	SetConsoleAttribute(LOWORD(dwExtraPrefix));

	if((dwExtraPrefix & STXLPX_ERROR_POST) && lpszLogFmt[0] == 0)
		szCRLF[0] = 0;


	_vstprintf_s(m_pLogBuffer, m_cchLogBufferLen, lpszLogFmt, _ArgList);

	if(dwExtraPrefix & STXLPX_ERROR_POST)
	{
		TCHAR szErrBufferUnit[MAX_PATH];
		GetErrorText(szErrBufferUnit, MAX_PATH, dwLastError);

		size_t nLen = _tcslen(m_pLogBuffer);
		_stprintf_s(m_pLogBuffer + nLen, m_cchLogBufferLen - nLen, _T("%s * 0x%.8X (%d) : %s\r\n"), szCRLF, dwLastError, dwLastError, szErrBufferUnit);
	}
	else
	{
		size_t nLen = _tcslen(m_pLogBuffer);
		_stprintf_s(m_pLogBuffer + nLen, m_cchLogBufferLen - nLen, _T("%s"), szCRLF);
	}

	TCHAR szPrefix[32];
	szPrefix[0] = 0;
	if(m_dwLogOptions & STXLF_ERROR_LEVEL)
	{
		_stprintf_s(szPrefix, _T("[EL:%d] "), nLevel);
		_tprintf_s(_T("%s"), szPrefix);
	}

	if (m_dwLogOptions & STXLF_THREADID)
	{
		TCHAR szThreadID[32];
		_stprintf_s(szThreadID, _T("[T:0x%X] "), ::GetCurrentThreadId());
		_tprintf_s(_T("%s"), szThreadID);
		::OutputDebugString(szThreadID);
	}

	::OutputDebugString(szPrefix);
	::OutputDebugString(m_pLogBuffer);
	_tprintf_s(_T("%s"), m_pLogBuffer);

	if ((m_dwLogOptions & STXLF_CALLBACK_DEBUG) && m_pCallback)
	{
		if (m_dwLogOptions & STXLF_TIME)
		{
			TCHAR szDate[64], szTime[64];
			_tstrdate_s(szDate, 64);
			m_pCallback->OnOutputDebugString(szDate, FALSE);
			m_pCallback->OnOutputDebugString(_T(" "), FALSE);
			_tstrtime_s(szTime, 64);
			m_pCallback->OnOutputDebugString(szTime, FALSE);
		}

		if (m_dwLogOptions & STXLF_ERROR_LEVEL)
		{
			TCHAR szLogLevel[8];
			_itot_s(nLevel, szLogLevel, 10);
			m_pCallback->OnOutputDebugString(_T("[EL:"), FALSE);
			m_pCallback->OnOutputDebugString(szLogLevel, FALSE);
			m_pCallback->OnOutputDebugString(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_LOGNAME)
		{
			m_pCallback->OnOutputDebugString(_T("[N:"), FALSE);
			m_pCallback->OnOutputDebugString(m_strLogName.c_str(), FALSE);
			m_pCallback->OnOutputDebugString(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_PROCESSID)
		{
			TCHAR szProcessID[32];
			_itot_s(::GetCurrentProcessId(), szProcessID, 16);
			m_pCallback->OnOutputDebugString(_T("[P:0x"), FALSE);
			m_pCallback->OnOutputDebugString(szProcessID, FALSE);
			m_pCallback->OnOutputDebugString(_T("]"), FALSE);
		}

		if (m_dwLogOptions & STXLF_THREADID)
		{
			TCHAR szThreadID[32];
			_itot_s(::GetCurrentThreadId(), szThreadID, 16);
			m_pCallback->OnOutputDebugString(_T("[T:0x"), FALSE);
			m_pCallback->OnOutputDebugString(szThreadID, FALSE);
			m_pCallback->OnOutputDebugString(_T("]"), FALSE);
		}

		m_pCallback->OnOutputDebugString(szPrefix, FALSE);
		m_pCallback->OnOutputDebugString(m_pLogBuffer, TRUE);
	}

	if(m_hConsoleStdout != INVALID_HANDLE_VALUE)
	{
		DWORD dwWrite = 0;
		if(szPrefix[0] != 0)
		{
			::WriteConsole(m_hConsoleStdout, szPrefix, (DWORD)_tcslen(szPrefix), &dwWrite, NULL);
		}
		::WriteConsole(m_hConsoleStdout, m_pLogBuffer, (DWORD)_tcslen(m_pLogBuffer), &dwWrite, NULL);
	}
	if((m_dwLogOptions & STXLF_NOTIFY_WINDOW) && m_szOutputWindowName[0] != 0)
	{
		ENUMDEBUGOUTWINDOW info;
		info.pLogObject = this;
		info.lpszDebugString = m_pLogBuffer;
		COPYDATASTRUCT cd;
		cd.cbData = (lstrlen(m_pLogBuffer) + 1) * sizeof(TCHAR);
		cd.dwData = 0;
		cd.lpData = (LPVOID)m_pLogBuffer;
		info.pCD = &cd;
		EnumWindows(EnumDebugOutputWindowsProc, (LPARAM)&info);
	}
	SetConsoleAttribute(m_ConsoleBufferInfo.wAttributes);
	LeaveCriticalSection(&m_cs);

}

BOOL CALLBACK CSTXLog::EnumDebugOutputWindowsProc(HWND hwnd, LPARAM lParam)
{
	LPENUMDEBUGOUTWINDOW pInfo = (LPENUMDEBUGOUTWINDOW)lParam;
	CSTXLog *pThis = pInfo->pLogObject;

	int nCaptionLen = ::GetWindowTextLength(hwnd);
	if(nCaptionLen > 0)
	{
		LPTSTR pszText = new TCHAR[nCaptionLen + 2];
		::GetWindowText(hwnd, pszText, nCaptionLen + 2);
		if(lstrcmp(pszText, pThis->m_szOutputWindowName) == 0)
		{
			::SendMessage(hwnd, WM_COPYDATA, NULL, (LPARAM)pInfo->pCD);
		}
	}

	return TRUE;
}


void CSTXLog::OutputLogDebugStringFull(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ...)
{
	if(lpszLogFmt == NULL || lpszSourceFile == NULL)
		return;

	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputLogDebugStringFullV(lpszSourceFile, nLine, lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputLogDebugStringFull( int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, ... )
{
	if(m_nLogLevel >= nLevel)
	{
		if(lpszLogFmt == NULL || lpszSourceFile == NULL)
			return;

		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputLogDebugStringFullV(nLevel, lpszSourceFile, nLine, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputLogDebugStringFullV(LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputLogDebugStringFullV(m_nLogLevel, lpszSourceFile, nLine, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputLogDebugStringFullV(int nLevel, LPCSTR lpszSourceFile, LONG nLine, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	LPTSTR pszFmt = new TCHAR[m_cchLogBufferLen + MAX_PATH];
#ifdef UNICODE
	WCHAR szSourceFileW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpszSourceFile, -1, szSourceFileW, MAX_PATH);
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), szSourceFileW, nLine, lpszLogFmt);
	OutputLogDebugStringV(nLevel, pszFmt, _ArgList);
#else
	_stprintf_s(pszFmt, m_cchLogBufferLen + MAX_PATH, _T("%s(%d) %s"), lpszSourceFile, nLine, lpszLogFmt);
	OutputLogDebugStringV(nLevel, pszFmt, _ArgList);
#endif
	delete []pszFmt;
}

void CSTXLog::OutputLogDebugString(LPCTSTR lpszLogFmt, ...)
{
	if(lpszLogFmt == NULL)
		return;

	DWORD dwLastError = GetLastError();
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	OutputLogDebugStringV(lpszLogFmt, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputLogDebugString( int nLevel, LPCTSTR lpszLogFmt, ... )
{
	if(m_nLogLevel >= nLevel)
	{
		if(lpszLogFmt == NULL)
			return;

		DWORD dwLastError = GetLastError();
		va_list marker;
		va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
		OutputLogDebugStringV(nLevel, lpszLogFmt, marker);
		va_end( marker );					/* Reset variable arguments.      */
	}
}

void CSTXLog::OutputLogDebugStringV(LPCTSTR lpszLogFmt, va_list _ArgList)
{
	OutputLogDebugStringV(m_nLogLevel, lpszLogFmt, _ArgList);
}

void CSTXLog::OutputLogDebugStringV(int nLevel, LPCTSTR lpszLogFmt, va_list _ArgList)
{
	EnterCriticalSection(&m_cs);
	OutputDebugStringV(nLevel, lpszLogFmt, _ArgList);
	OutputLogV(nLevel, lpszLogFmt, _ArgList);
	LeaveCriticalSection(&m_cs);
}

void CSTXLog::OutputDebugStringDirect(LPCTSTR lpszLogFmt, const UINT cchBufferLength, ...)
{
	va_list marker;
	va_start( marker, cchBufferLength );     /* Initialize variable arguments. */
	OutputDebugStringDirectV(lpszLogFmt, cchBufferLength, marker);
	va_end( marker );					/* Reset variable arguments.      */
}

void CSTXLog::OutputDebugStringDirectV(LPCTSTR lpszLogFmt, const UINT cchBufferLength, va_list _ArgList)
{
	LPTSTR pszOutput = new TCHAR[cchBufferLength];
	wvsprintf(pszOutput, lpszLogFmt, _ArgList);
	::OutputDebugString(pszOutput);
	::OutputDebugString(_T("\n"));
	delete []pszOutput;
}

BOOL CSTXLog::ModifyLogOption(DWORD dwRemove, DWORD dwAdd)
{
	dwAdd &= (~dwRemove);

	if((dwRemove & STXLF_CONSOLE)
		&& (m_dwLogOptions & STXLF_CONSOLE))
	{
		if(m_bOwnedConsole)
		{
			FreeConsole();
			m_bOwnedConsole = FALSE;
			m_hConsoleStdout = INVALID_HANDLE_VALUE;
		}
	}
	else if((dwAdd & STXLF_CONSOLE)
		&& (m_dwLogOptions & STXLF_CONSOLE) == 0)
	{
		m_bOwnedConsole = AllocConsole();
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleStdout = hStdout;
		GetConsoleScreenBufferInfo(m_hConsoleStdout, &m_ConsoleBufferInfo);
	}

	m_dwLogOptions &= (~dwRemove);
	m_dwLogOptions |= dwAdd;

	return TRUE;
}

void CSTXLog::SetLogFilePathName(LPCTSTR lpszPathName)
{
	if(lpszPathName == NULL)
		return;

	EnterCriticalSection(&m_cs);
	m_strLogFilePath = lpszPathName;
	LeaveCriticalSection(&m_cs);
}

DWORD CSTXLog::GetLastErrorText(LPTSTR lpszBuf, DWORD cchBufLen)
{
	return GetErrorText(lpszBuf, cchBufLen, GetLastError());
}

DWORD CSTXLog::GetErrorText(LPTSTR lpszBuf, DWORD cchBufLen, DWORD dwErrorCode)
{
	return FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lpszBuf, cchBufLen, NULL);
}


void CSTXLog::SetNotifyWindowName(LPCTSTR lpszWindowCaption)
{
	if(lpszWindowCaption == NULL)
		return;
	lstrcpyn(m_szOutputWindowName, lpszWindowCaption, sizeof(m_szOutputWindowName) / sizeof(TCHAR));
}

BOOL CSTXLog::SetConsoleAttribute(WORD wAttribute)
{
	if(m_hConsoleStdout != INVALID_HANDLE_VALUE)
	{
		if(wAttribute)
			return SetConsoleTextAttribute(m_hConsoleStdout, wAttribute);
		else
			return SetConsoleTextAttribute(m_hConsoleStdout, m_ConsoleBufferInfo.wAttributes);
	}

	return FALSE;
}

void CSTXLog::SetCallback(ISTXLogCallback *pCallback)
{
	m_pCallback = pCallback;
}

void CSTXLog::SetLogLevel( int nLogLevel /*= -1*/, int nDebugLevel /*= -1*/ )
{
	if(nLogLevel >= 0)
		m_nLogLevel = nLogLevel;

	if(nDebugLevel >= 0)
		m_nDebugLevel = nDebugLevel;
}
