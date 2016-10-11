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

#include "STXServerBase.h"
#include <shlwapi.h>
//#include "Global.h"

//#define INCLUDE_INTEL_VTUNE_AMPLIFIER_CODE

//////////////////////////////////////////////////////////////////////////
CSTXLog g_LogGlobal;

//////////////////////////////////////////////////////////////////////////
int SplitString( const TCHAR *pszSrc, TCHAR chSplit, std::vector<std::wstring> &arrResult )
{
	const TCHAR *pValue = pszSrc;
	size_t nLen = _tcslen(pszSrc);
	if(nLen > 0)
	{
		arrResult.clear();
		TCHAR *pValueCopy = new TCHAR[nLen + 2];
		_tcscpy_s(pValueCopy, nLen + 2, pValue);

		TCHAR seps[2] = {0};
		seps[0] = chSplit;
		seps[1] = 0;

		TCHAR *pStart = pValueCopy, *pEnd = pStart + 1;
		while(pStart && *pStart)
		{
			pEnd = _tcschr(pStart, chSplit);
			if(pEnd == 0)
			{
				break;
				pEnd = pValueCopy + nLen;
			}

			TCHAR chEnd = *pEnd;
			TCHAR *pStartCopy = pStart;
			pStart = pEnd + 1;
			*pEnd = 0;

			arrResult.push_back(pStartCopy);

			if(chEnd == 0)
				break;

		}
		arrResult.push_back(pStart);
		delete []pValueCopy;
	}
	return (int)arrResult.size();
}

void CSTXMemoryVariableNode::RegisterVariable(std::wstring strPathName, STXVariableTreeNodeType nType, LPVOID pAddress)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	if(arrParts.size() == 0)
		return;

	if(arrParts.size() == 1)
	{
		CSTXMemoryVariableNode &newVariable = _mapContent[strPathName];
		newVariable._type = nType;
		newVariable._ptr = pAddress;
		newVariable._parentNode = this;
		newVariable._name = strPathName;
		return;
	}

	std::wstring firstPart = arrParts[0];
	std::map<std::wstring, CSTXMemoryVariableNode>::iterator it = _mapContent.find(firstPart);
	if(it == _mapContent.end(firstPart))
	{
		CSTXMemoryVariableNode &newVariable = _mapContent[firstPart];
		newVariable._parentNode = this;
		newVariable._name = firstPart;
	}
	it = _mapContent.find(firstPart);
	
	it->second.RegisterVariable(strPathName.substr(firstPart.size() + 1), nType, pAddress);
}

CSTXMemoryVariableNode::CSTXMemoryVariableNode()
{
	_type = STXVariableTreeNodeType_Invalid;
	_ptr = NULL;
	_parentNode = NULL;
}

LPVOID CSTXMemoryVariableNode::GetVariablePtr(std::wstring strPathName)
{
	CSTXMemoryVariableNode *pNode = GetVariableNode(strPathName);
	if(pNode == NULL)
		return NULL;

	return pNode->_ptr;
}

CSTXMemoryVariableNode * CSTXMemoryVariableNode::GetVariableNode(std::wstring strPathName)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	if(arrParts.size() == 0)
		return NULL;

	std::wstring firstPart = arrParts[0];
	std::map<std::wstring, CSTXMemoryVariableNode>::iterator it = _mapContent.find(firstPart);
	if(it == _mapContent.end(firstPart))
	{
		return NULL;
	}

	if(arrParts.size() == 1)
	{
		return &it->second;
	}

	return it->second.GetVariableNode(strPathName.substr(firstPart.size() + 1));
}

int CSTXMemoryVariableNode::GetVariableType(std::wstring strPathName)
{
	CSTXMemoryVariableNode *pNode = GetVariableNode(strPathName);
	if(pNode == NULL)
		return -1;

	return pNode->_type;
}

void CSTXMemoryVariableNode::UnregisterVariable(std::wstring strPathName)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	if(arrParts.size() == 0)
		return;

	if(arrParts.size() == 1)
	{
		_mapContent.erase(strPathName);
		return;
	}

	std::wstring firstPart = arrParts[0];
	std::map<std::wstring, CSTXMemoryVariableNode>::iterator it = _mapContent.find(firstPart);
	if(it == _mapContent.end(firstPart))
	{
		return;
	}

	it->second.UnregisterVariable(strPathName.substr(firstPart.size() + 1));
}

void CSTXMemoryVariableNode::SetValue(std::wstring strPathName, std::wstring strValue)
{
	CSTXMemoryVariableNode *pNode = GetVariableNode(strPathName);
	if(pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		*((__int32*)pNode->_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		*((__int64*)pNode->_ptr) = _ttoi64(strValue.c_str());
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		*((std::wstring*)pNode->_ptr) = strValue;
		break;
	case STXVariableTreeNodeType_Int:		//int
		*((int*)pNode->_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Float:		//float
		*((float*)pNode->_ptr) = (float)_ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Double:		//double
		*((double*)pNode->_ptr) = _ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Word:		//WORD
		*((WORD*)pNode->_ptr) = (WORD)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	case STXVariableTreeNodeType_DWord:		//DWORD
		*((DWORD*)pNode->_ptr) = (DWORD)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	default:
		break;
	}
}

std::wstring CSTXMemoryVariableNode::GetValue(std::wstring strPathName)
{
	CSTXMemoryVariableNode *pNode = GetVariableNode(strPathName);
	if(pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return _T("");

	TCHAR szTmp[MAX_PATH];
	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		_itot_s(*((__int32*)pNode->_ptr), szTmp, MAX_PATH, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		_i64tot_s(*((__int64*)pNode->_ptr), szTmp, MAX_PATH, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		return *((std::wstring*)pNode->_ptr);
		break;
	case STXVariableTreeNodeType_Int:		//int
		_itot_s(*((int*)pNode->_ptr), szTmp, MAX_PATH, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Float:		//float
		_stprintf_s(szTmp, _T("%f"), *((float*)pNode->_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Double:		//double
		_stprintf_s(szTmp, _T("%lf"), *((double*)pNode->_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Word:		//WORD
		_ultot_s(*((WORD*)pNode->_ptr), szTmp, MAX_PATH, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_DWord:		//DWORD
		_ultot_s(*((DWORD*)pNode->_ptr), szTmp, MAX_PATH, 10);
		return szTmp;
		break;

	default:
		break;
	}

	return _T("");
}

int CSTXMemoryVariableNode::GetChildrenNames(std::vector<std::wstring> *pArrNames)
{
	if(pArrNames == NULL)
		return _mapContent.size();

	pArrNames->clear();
	_mapContent.foreach([&](std::pair<const std::wstring, CSTXMemoryVariableNode> item)
	{
		pArrNames->push_back(item.first);
	});

	return (int)pArrNames->size();
}

std::wstring CSTXMemoryVariableNode::GetFullPath()
{
	std::wstring path = _name;
	CSTXMemoryVariableNode *pParent = _parentNode;
	while (pParent)
	{
		path = pParent->_name + _T("\\") + path;
		pParent = pParent->_parentNode;
	}
	return path;
}

void CSTXMemoryVariableNode::RegisterInt32Variable(std::wstring strPathName, __int32 *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int32, pAddress);
}

void CSTXMemoryVariableNode::RegisterInt64Variable(std::wstring strPathName, __int64 *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int64, pAddress);
}

void CSTXMemoryVariableNode::RegisterIntStringVariable(std::wstring strPathName, std::wstring *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_WString, pAddress);
}

void CSTXMemoryVariableNode::RegisterIntVariable(std::wstring strPathName, int *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int, pAddress);
}

void CSTXMemoryVariableNode::RegisterFloatVariable(std::wstring strPathName, int *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Float, pAddress);
}

void CSTXMemoryVariableNode::RegisterDoubleVariable(std::wstring strPathName, int *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Double, pAddress);
}

void CSTXMemoryVariableNode::RegisterWordVariable(std::wstring strPathName, WORD *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Word, pAddress);
}

void CSTXMemoryVariableNode::RegisterDWordVariable(std::wstring strPathName, DWORD *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_DWord, pAddress);
}


//////////////////////////////////////////////////////////////////////////
// CSTXServerBase
CSTXHashMap<LONGLONG, CSTXServerObjectPtr<CSTXServerBase::TimerParam>, 16> CSTXServerBase::m_mapTimerDataAlt;
CSTXServerBase *CSTXServerBase::s_pThis = NULL;;

CSTXServerBase::CSTXServerBase(void)
{
	s_pThis = this;
	m_hTimerThread = INVALID_HANDLE_VALUE;
	m_uTimerThreadID = 0;
	m_hTimerThreadEvent = NULL;
	m_hTimerThreadIntervalChangeEvent = NULL;
	m_hTimerQueue = 0;
	m_nTimerQueueRef = 0;
	m_nTimerAlternativeIDBase = 0;

	m_BaseServerInfo.cbSize = sizeof(m_BaseServerInfo);
	m_BaseServerInfo.dwAcceptPost = STXS_DEFAULT_ACCEPT_POST;
	m_BaseServerInfo.dwBufferInitialCount = STXS_DEFAULT_BUFFER_COUNT;
	m_BaseServerInfo.dwBufferMaxCount = STXS_DEFAULT_BUFFER_MAX_COUNT;
	m_BaseServerInfo.dwBufferSize = STXS_DEFAULT_BUFFER_SIZE;
	m_BaseServerInfo.dwDefaultOperationTimeout = STXS_DEFAULT_OPERATION_TIMEOUT;
	m_BaseServerInfo.dwAcceptBufferSize = STXS_DEFAULT_ACCEPT_BUFFER_SIZE;
	m_BaseServerInfo.dwLogFlags = STXS_DEFAULT_LOG_FLAGS;
	m_BaseServerInfo.dwServerFlags = 0;
	m_BaseServerInfo.dwTimerInterval = STXS_DEFAULT_TIMER_INTERVAL;
	m_BaseServerInfo.dwStatus = STXSS_NORMAL;
}

CSTXServerBase::~CSTXServerBase(void)
{
	if(m_hTimerThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hTimerThreadEvent);
		WaitForSingleObject(m_hTimerThread, INFINITE);
		CloseHandle(m_hTimerThread);
	}
	if(m_hTimerThreadEvent)
		CloseHandle(m_hTimerThreadEvent);
	if (m_hTimerThreadIntervalChangeEvent)
		CloseHandle(m_hTimerThreadIntervalChangeEvent);
}

BOOL CSTXServerBase::OnInitialization()
{
	m_mapTimerDataAlt.clear();
	m_hTimerQueue = CreateTimerQueue();
	AddTimerQueueRef();
	if(m_hTimerQueue == 0)
	{
		STXTRACELOGE(_T("[r][i]CreateTimerQueue failed !"));
	}
	RegisterServerBaseVariablesForVariableTree();
	return OnInitializeServerLog();
}

void CSTXServerBase::OnShutDown()
{
	//STXTRACELOGE(_T("Clear TimerQueueTimers and delete TimerQueue..."));
	m_mapTimerDataAlt.foreach([&](std::pair<LONGLONG, CSTXServerObjectPtr<CSTXServerBase::TimerParam> > item)
	{
		LONGLONG nRef = ReleaseTimerQueueRef();
		if (nRef == 0)
		{
			DeleteTimerQueueEx(m_hTimerQueue, NULL);
			m_hTimerQueue = NULL;
			//STXTRACELOGE(_T("TimerQueue deleted."));
			m_mapTimerDataAlt.clear();
		}
	});

	LONGLONG nRefOuter = ReleaseTimerQueueRef();
	if (nRefOuter == 0)
	{
		DeleteTimerQueueEx(m_hTimerQueue, NULL);
		m_hTimerQueue = NULL;
		//STXTRACELOGE(_T("TimerQueue deleted."));
		m_mapTimerDataAlt.clear();
	}
}

void CSTXServerBase::OnFinalShutDown()
{
	m_mapTimerDataAlt.clear();
	g_LogGlobal.Uninitialize();
}

CSTXServerClientContextBase* CSTXServerBase::OnCreateClientContext(CSTXServerContextBase *pServerContext)
{
	return new CSTXServerClientContextBase();
}

void CSTXServerBase::OnClientContextInitialized(CSTXServerClientContextBase *pClientContext)
{

}

CSTXTcpServerContextBase* CSTXServerBase::OnCreateTcpServerContext(DWORD_PTR dwServerParam)
{
	return new CSTXTcpServerContextBase(dwServerParam);
}

CSTXUdpServerContextBase* CSTXServerBase::OnCreateUdpServerContext(DWORD_PTR dwServerParam)
{
	return new CSTXUdpServerContextBase(dwServerParam);
}

BOOL CSTXServerBase::OnInitializeServerLog()
{
	TCHAR szLogFilePath[MAX_PATH];
	if(m_BaseServerInfo.pszLogFilePath == NULL)
	{
		_tcscpy_s(szLogFilePath, MAX_PATH, m_szModuleFilePathName);
		_tcscat_s(szLogFilePath, _T(".log"));
	}
	else
	{
		_tcscpy_s(szLogFilePath, m_BaseServerInfo.pszLogFilePath);
	}
	if(!g_LogGlobal.Initialize(m_pszModuleFileName, szLogFilePath, m_BaseServerInfo.dwLogFlags, m_BaseServerInfo.dwLogBufferSize >= 1024 ? m_BaseServerInfo.dwLogBufferSize : 1024))
		return FALSE;

	g_LogGlobal.SetNotifyWindowName(m_BaseServerInfo.pszLogOutputWindowTitle);
	g_LogGlobal.SetCallback(this);

	return TRUE;
}

void CSTXServerBase::OutputLog(LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	g_LogGlobal.OutputLogV(lpszLogFmt, marker);
	va_end( marker );              /* Reset variable arguments.      */
}

void CSTXServerBase::OutputLogDirect(LPCTSTR lpszLogFmt, ...)
{
	va_list marker;
	va_start( marker, lpszLogFmt );     /* Initialize variable arguments. */
	g_LogGlobal.OutputLogDirectV(lpszLogFmt, marker);
	va_end( marker );              /* Reset variable arguments.      */
}

BOOL CSTXServerBase::Initialize(HMODULE hModuleHandle, LPSTXSERVERINIT lpInit)
{
	if(m_hTimerThread != INVALID_HANDLE_VALUE || m_hTimerThreadEvent)
		return FALSE;

	if(lpInit == NULL)
		return FALSE;

	m_hModuleHandle = (hModuleHandle == NULL ? ::GetModuleHandle(NULL) : hModuleHandle);

	if(m_hModuleHandle)
	{
		::GetModuleFileName(m_hModuleHandle, m_szModuleFilePathName, sizeof(m_szModuleFilePathName) / sizeof(TCHAR));
		::GetFullPathName(m_szModuleFilePathName, sizeof(m_szModuleFilePathName) / sizeof(TCHAR)
			, m_szModuleFilePath, &m_pszModuleFileName);

		// Point to the end
		LPCTSTR pszPosStart = m_szModuleFilePathName + _tcslen(m_szModuleFilePathName);
		LPCTSTR pszPosEnd = pszPosStart;

		//Find filename start
		while(*pszPosStart != _T('\\'))
			pszPosStart--;
		pszPosStart++;

		//Find fileTitle end
		while(*pszPosEnd != _T('\\') && *pszPosEnd != _T('.'))
			pszPosEnd--;
		if(*pszPosEnd == _T('\\'))	//No extension
			_tcscpy_s(m_szModuleFileTitle, sizeof(m_szModuleFileTitle) / sizeof(TCHAR), pszPosStart);
		else
		{
			_tcsncpy_s(m_szModuleFileTitle, pszPosStart, pszPosEnd - pszPosStart);
			m_szModuleFileTitle[pszPosEnd - pszPosStart] = _T('\0');
		}

		m_szModuleFilePath[pszPosStart - m_szModuleFilePathName] = _T('\0');

		if(lpInit->pszIniFilePathName == NULL)
		{
			_tcscpy_s(m_szIniFilePath, sizeof(m_szIniFilePath) / sizeof(TCHAR), m_szModuleFilePath);
			_tcscat_s(m_szIniFilePath, sizeof(m_szIniFilePath) / sizeof(TCHAR), m_szModuleFileTitle);
			_tcscat_s(m_szIniFilePath, sizeof(m_szIniFilePath) / sizeof(TCHAR), _T(".INI"));
		}
		else
		{
			_tcscpy_s(m_szIniFilePath, sizeof(m_szIniFilePath) / sizeof(TCHAR), lpInit->pszIniFilePathName);
		}

		if(lpInit->pszXmlFilePathName == NULL)
		{
			_tcscpy_s(m_szXmlFilePath, sizeof(m_szXmlFilePath) / sizeof(TCHAR), m_szModuleFilePath);
			_tcscat_s(m_szXmlFilePath, sizeof(m_szXmlFilePath) / sizeof(TCHAR), m_szModuleFileTitle);
			_tcscat_s(m_szXmlFilePath, sizeof(m_szXmlFilePath) / sizeof(TCHAR), _T(".xml"));
		}
		else
		{
			_tcscpy_s(m_szXmlFilePath, sizeof(m_szXmlFilePath) / sizeof(TCHAR), lpInit->pszXmlFilePathName);
		}
	}

	if(lpInit->dwMask & STXSM_TIMER_INTERVAL)
		m_BaseServerInfo.dwTimerInterval = lpInit->dwTimerInterval;

	if(lpInit->dwMask & STXSM_ACCEPT)
		m_BaseServerInfo.dwAcceptPost = lpInit->dwAcceptPost;

	if(lpInit->dwMask & STXSM_BUFFER)
	{
		m_BaseServerInfo.dwBufferInitialCount = lpInit->dwBufferInitialCount;
		m_BaseServerInfo.dwBufferMaxCount = lpInit->dwBufferMaxCount;
		m_BaseServerInfo.dwBufferSize = lpInit->dwBufferSize;
	}

	if(lpInit->dwMask & STXSM_OPERATION_TIMEOUT)
		m_BaseServerInfo.dwDefaultOperationTimeout = lpInit->dwDefaultOperationTimeout;

	if(lpInit->dwMask & STXSM_LOG_FLAGS)
		m_BaseServerInfo.dwLogFlags = lpInit->dwLogFlags;

	m_BaseServerInfo.pszLogOutputWindowTitle = lpInit->pszLogOutputWindowTitle;
	m_BaseServerInfo.pszIniFilePathName = lpInit->pszIniFilePathName;
	m_BaseServerInfo.pszXmlFilePathName = lpInit->pszXmlFilePathName;
	m_BaseServerInfo.pszLogFilePath = lpInit->pszLogFilePath;
	m_BaseServerInfo.dwServerFlags = lpInit->dwServerFlags;
	m_BaseServerInfo.dwLogBufferSize = lpInit->dwLogBufferSize;
	m_BaseServerInfo.dwFolderMonitorBufferSize = lpInit->dwFolderMonitorBufferSize;

	if(!OnInitialization())
	{
		STXTRACELOGE(_T("[r][i]OnInitialization Failed! Terminate server..."));
		InternalTerminate();
		return FALSE;
	}

	if(m_BaseServerInfo.dwServerFlags & STXSF_IPV6)
		STXTRACELOGE(_T("[b][i]Server support for IPv6 enabled."));

	if (m_BaseServerInfo.dwLogBufferSize < 1024)
	{
		m_BaseServerInfo.dwLogBufferSize = 1024;
		STXTRACELOGE(_T("[r][g][i]Server log buffer size %d is too small. now using 1024."), m_BaseServerInfo.dwLogBufferSize);
	}

	if (m_BaseServerInfo.dwLogBufferSize >= 1024)
	{
		TCHAR szTemp[MAX_PATH];
		StrFormatByteSize64(m_BaseServerInfo.dwLogBufferSize, szTemp, MAX_PATH);
		STXTRACELOGE(_T("[b][i]Server log buffer size: %d\t(%s)"), m_BaseServerInfo.dwLogBufferSize, szTemp);
	}

	int nSubServerCreated = 0;
	if(lpInit->uTcpServerCount > 0)
	{
		for(UINT i=0;i<lpInit->uTcpServerCount;i++)
		{
			DWORD_PTR dwParam = lpInit->pTcpServerParameters ? lpInit->pTcpServerParameters[i] : 0;
			LPCTSTR pszParamString = lpInit->pszTcpServerParameterStrings ? lpInit->pszTcpServerParameterStrings[i] : NULL;
			if (!BeginTcpServer(lpInit->pTcpServerPorts[i], dwParam, pszParamString))
			{
				STXTRACELOGE(_T("[r][i]BeginTcpServer Failed for port %d [TCP] !"), lpInit->pTcpServerPorts[i]);
			}
			else
				nSubServerCreated++;
		}
	}

	if(lpInit->uUdpServerCount > 0)
	{
		for(UINT i=0;i<lpInit->uUdpServerCount;i++)
		{
			if(!BeginUdpServer(lpInit->pUdpServerPorts[i], lpInit->pUdpServerParameters[i], _T("")))
			{
				STXTRACELOGE(_T("[r][i]BeginUdpServer Failed for port %d [UDP] !"), lpInit->pUdpServerPorts[i]);
			}
			else
				nSubServerCreated++;
		}
	}

 	if(nSubServerCreated == 0)
 	{
 		STXTRACELOGE(_T("[r][g][i] No Sub-Server created !"));
 		//InternalTerminate();
 		//return FALSE;
 	}

	//Create Timer Thread and etc.
	m_hTimerThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hTimerThreadIntervalChangeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hTimerThread = (HANDLE)_beginthreadex(NULL, 0, ServerTimerThread, this, 0, &m_uTimerThreadID);
	if(m_hTimerThread == INVALID_HANDLE_VALUE)
		STXTRACELOGE(_T("[r][i]Failed to create Timer thread! There will be no build-in timer events."));

	return TRUE;
}

BOOL CSTXServerBase::BeginTcpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

CSTXServerContextBase* CSTXServerBase::BeginUdpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return NULL;
}

LONG CSTXServerBase::CreateTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, int nAddressFamily, BOOL bKeepConnect)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return -1;
}

UINT WINAPI CSTXServerBase::ServerTimerThread(LPVOID lpParameter)
{
	CSTXServerBase *pThis = (CSTXServerBase*)lpParameter;

	pThis->OnTimerInitialize();

	HANDLE waitObjects[2] = { pThis->m_hTimerThreadEvent , pThis->m_hTimerThreadIntervalChangeEvent };
	

	while(TRUE)
	{
		//auto waitResult = ::WaitForSingleObject(pThis->m_hTimerThreadEvent, pThis->m_BaseServerInfo.dwTimerInterval);
		auto waitResult = ::WaitForMultipleObjects(2, waitObjects, FALSE, pThis->m_BaseServerInfo.dwTimerInterval);
		if (waitResult == WAIT_OBJECT_0)		//Signal of termination
		{
			break;
		}
		else if (waitResult == WAIT_OBJECT_0 + 1)		//Interval Changed
		{
			STXTRACELOGE(_T("Timer interval changed to %dms"), pThis->m_BaseServerInfo.dwTimerInterval);
			ResetEvent(pThis->m_hTimerThreadIntervalChangeEvent);
			continue;
		}
		pThis->OnTimer(pThis->m_BaseServerInfo.dwTimerInterval);
	}

	pThis->OnTimerUninitialize();

	return 0;
}

void CSTXServerBase::OnTimer(DWORD dwInterval)
{

}

void CSTXServerBase::OnTimerInitialize()
{

}

void CSTXServerBase::OnTimerUninitialize()
{

}

LPCTSTR CSTXServerBase::GetServerModuleFilePathName()
{
	return (LPCTSTR)m_szModuleFilePathName;
}

LPCTSTR CSTXServerBase::GetServerModuleFileName()
{
	return (LPCTSTR)m_pszModuleFileName;
}

LPCTSTR CSTXServerBase::GetServerModuleFilePath()
{
	return (LPCTSTR)m_szModuleFilePath;
}

LPCTSTR CSTXServerBase::GetServerModuleFileTitle()
{
	return (LPCTSTR)m_szModuleFileTitle;
}

LPCTSTR CSTXServerBase::GetServerModuleIniPathName()
{
	if(m_BaseServerInfo.pszIniFilePathName)
		return m_BaseServerInfo.pszIniFilePathName;

	return (LPCTSTR)m_szIniFilePath;
}

LPCTSTR CSTXServerBase::GetServerModuleXmlPathName()
{
	if(m_BaseServerInfo.pszXmlFilePathName)
		return m_BaseServerInfo.pszXmlFilePathName;

	return (LPCTSTR)m_szXmlFilePath;
}

void CSTXServerBase::EnableConsoleOutput(BOOL bEnabled)
{
	if(bEnabled)
		g_LogGlobal.ModifyLogOption(0, STXLF_CONSOLE);
	else
		g_LogGlobal.ModifyLogOption(STXLF_CONSOLE, 0);
}

void CSTXServerBase::StartServer()
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

BOOL CSTXServerBase::Terminate()
{
	//Terminate the Timer Thread
	::SetEvent(m_hTimerThreadEvent);
	::WaitForSingleObject(m_hTimerThread, INFINITE);
	return TRUE;
}

UINT CSTXServerBase::GetServerProfileString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPTSTR lpszBuf, DWORD cchBufLen, LPCTSTR lpszDefault)
{
	return ::GetPrivateProfileString(lpszSection, lpszKey, lpszDefault, lpszBuf, cchBufLen, GetServerModuleIniPathName());
}

UINT CSTXServerBase::GetServerProfileInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT nDefault)
{
	return ::GetPrivateProfileInt(lpszSection, lpszKey, nDefault, GetServerModuleIniPathName());
}

DWORD CSTXServerBase::GetTimerInterval()
{
	return m_BaseServerInfo.dwTimerInterval;
}

void CSTXServerBase::GetBaseServerInfo( LPSTXSERVERINFORMATION pInfo )
{
	memcpy(pInfo, &m_BaseServerInfo, sizeof(m_BaseServerInfo));
}

void CSTXServerBase::InternalTerminate()
{

}

void CALLBACK CSTXServerBase::ServerBaseTimerCallback( __in PVOID lpParameter, __in BOOLEAN TimerOrWaitFired )
{
	LONGLONG nTimerAltId = (LONGLONG)lpParameter;
	m_mapTimerDataAlt.lock(nTimerAltId);
	auto itAlt = m_mapTimerDataAlt.find(nTimerAltId);
	if (itAlt == m_mapTimerDataAlt.end(nTimerAltId))
	{
		STXTRACE(_T("in ServerBaseTimerCallback, Timer not found in alt-map. altID = %I64d"), nTimerAltId);
		m_mapTimerDataAlt.unlock(nTimerAltId);
		return;
	}
	if (itAlt->second == NULL)
	{
		STXTRACE(_T("Timer Deleting Error NULL"));
		m_mapTimerDataAlt.unlock(nTimerAltId);
		return;
	}

	s_pThis->OnTimerTimeoutWrapper(itAlt->second->pTimerParam);

 	m_mapTimerDataAlt.unlock(nTimerAltId);
}

void CSTXServerBase::OnTimerTimeoutWrapper(LPVOID lpTimerParam)
{
	__try
	{
		__try
		{
			OnTimerTimeout(lpTimerParam);
		}
		__except (ProcessException(GetExceptionInformation()))
		{

		}
	}
	__finally
	{

	}
}


void CSTXServerBase::OnTimerTimeout( LPVOID lpTimerParam )
{

}

HANDLE CSTXServerBase::AddTimerObject( LPVOID lpTimerParam, DWORD dwDueTime, LONGLONG *pAltId )
{
	if(m_hTimerQueue == NULL)
		return NULL;

	TimerParam *pData = new TimerParam();
	if (pData == NULL)
	{
		STXTRACELOGE(_T("TimerParam allocate failed!. Maybe not enough memory."));
		return NULL;
	}

	AddTimerQueueRef();

	CSTXServerObjectPtr<TimerParam> pDataPtr = pData;
	pData->Release();


	LONGLONG nAltID = InterlockedIncrement64(&m_nTimerAlternativeIDBase);
	pDataPtr->pThis = this;
	pDataPtr->pTimerParam = lpTimerParam;
	pDataPtr->nAltID = nAltID;
	pDataPtr->hTimer = INVALID_HANDLE_VALUE;

	m_mapTimerDataAlt[nAltID] = pDataPtr;

	HANDLE hTimer = NULL;
	if(!CreateTimerQueueTimer(&hTimer, m_hTimerQueue, ServerBaseTimerCallback, (LPVOID)nAltID, dwDueTime, 0, WT_EXECUTEINTIMERTHREAD|WT_EXECUTEONLYONCE))
	{
		m_mapTimerDataAlt.erase(nAltID);
		LONGLONG nRef = ReleaseTimerQueueRef();
		if (nRef == 0)
		{
			DeleteTimerQueueEx(m_hTimerQueue, NULL);
			m_hTimerQueue = NULL;
			//STXTRACELOGE(_T("TimerQueue deleted."));
			m_mapTimerDataAlt.clear();
		}
		return NULL;
	}

	pData->hTimer = hTimer;
	if (pAltId)
		*pAltId = nAltID;

	return hTimer;
}

int CSTXServerBase::ProcessException(LPEXCEPTION_POINTERS pExp)
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

		g_LogGlobal.Lock();
		STXTRACELOGE(1, _T("[r][i]**Exception** - %s"), szExceptionText);
		ShowExceptionCallStack(&c);
		g_LogGlobal.Unlock();
	}
	else
	{
		g_LogGlobal.Lock();
		STXTRACELOGE(1, _T("[r][i]Unknown exception!"));
		ShowExceptionCallStack();
		g_LogGlobal.Unlock();
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

#ifdef INCLUDE_INTEL_VTUNE_AMPLIFIER_CODE
#include "D:\Intel\VTune Amplifier XE 2015\include\ittnotify.h"
#pragma comment(lib, "D:\\Intel\\VTune Amplifier XE 2015\\lib64\\libittnotify.lib ")
#endif

BOOL CSTXServerBase::DeleteTimerObject( HANDLE hTimer, LONGLONG nAltId)
{
	if (m_hTimerQueue == NULL)
	{
		return TRUE;
	}

	m_mapTimerDataAlt.lock(nAltId);
	if (m_hTimerQueue == NULL)
	{
		m_mapTimerDataAlt.unlock(nAltId);
		return TRUE;
	}
	auto itAlt = m_mapTimerDataAlt.find(nAltId);
	if (itAlt == m_mapTimerDataAlt.end(nAltId))
	{
		STXTRACE(_T("Timer not found in alt-map. timer = %p, altID = %I64d"), hTimer, nAltId);
		m_mapTimerDataAlt.unlock(nAltId);
		return FALSE;
	}
	if (itAlt->second == NULL)
	{
		STXTRACE(_T("Timer Deleting Error NULL"));
		m_mapTimerDataAlt.unlock(nAltId);
		return FALSE;
	}

	HANDLE hTimerInternal = itAlt->second->hTimer;
	if (hTimerInternal == NULL)
	{
		STXTRACE(_T("Timer Deleting Error, Timer Handle IS NULL"));
		m_mapTimerDataAlt.unlock(nAltId);
		return FALSE;
	}

	BOOL bResult = FALSE;

#ifdef INCLUDE_INTEL_VTUNE_AMPLIFIER_CODE
	__itt_pause();
	bResult = DeleteTimerQueueTimer(m_hTimerQueue, hTimerInternal, NULL);
	__itt_resume();
#else
	bResult = DeleteTimerQueueTimer(m_hTimerQueue, hTimerInternal, NULL);
#endif

	itAlt->second->hTimer = NULL;

	int iResult = WSAGetLastError();
	if (bResult || (!bResult && iResult == ERROR_IO_PENDING))
	{
		bResult = TRUE;
	}
	else
	{
		STXTRACE(_T("DeleteTimerObject DeleteTimerQueueTimer failed. timer = %p, altID = %I64d, error = %d"), hTimer, nAltId, iResult);
	}

	m_mapTimerDataAlt.erase(nAltId);
	m_mapTimerDataAlt.unlock(nAltId);

	LONGLONG nRef = ReleaseTimerQueueRef();
	if (nRef == 0)
	{
		DeleteTimerQueueEx(m_hTimerQueue, NULL);
		m_hTimerQueue = NULL;
		//STXTRACELOGE(_T("TimerQueue deleted."));
		m_mapTimerDataAlt.clear();
	}

	return bResult;
}

void CSTXServerBase::SetTcpClientOperationDefaultTimeOut(DWORD dwTimeoutMS)
{
	m_BaseServerInfo.dwDefaultOperationTimeout = dwTimeoutMS;
}

LONG64 CSTXServerBase::AddTimerQueueRef()
{
	return InterlockedIncrement64(&m_nTimerQueueRef);
}

LONG64 CSTXServerBase::ReleaseTimerQueueRef()
{
	return InterlockedDecrement64(&m_nTimerQueueRef);
}

void CSTXServerBase::RegisterServerBaseVariablesForVariableTree()
{
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\AcceptPost"), &m_BaseServerInfo.dwAcceptPost);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\BufferInitialCount"), &m_BaseServerInfo.dwBufferInitialCount);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\BufferMaxCount"), &m_BaseServerInfo.dwBufferMaxCount);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\BufferSize"), &m_BaseServerInfo.dwBufferSize);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\DefaultOperationTimeout"), &m_BaseServerInfo.dwDefaultOperationTimeout);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\AcceptBufferSize"), &m_BaseServerInfo.dwAcceptBufferSize);
	m_variableTreeRoot.RegisterDWordVariable(_T("ServerBase\\Defaults\\TimerInterval"), &m_BaseServerInfo.dwTimerInterval);
}

CSTXMemoryVariableNode& CSTXServerBase::GetVariableTreeRoot()
{
	return m_variableTreeRoot;
}

//////////////////////////////////////////////////////////////////////////
// CSTXServerClientContextBase

CSTXServerClientContextBase::CSTXServerClientContextBase()
{
	m_nRef = 1;
}

CSTXServerClientContextBase::~CSTXServerClientContextBase()
{
}

void CSTXServerClientContextBase::OnDestroy()
{

}

BOOL CSTXServerClientContextBase::OnInitialize()
{
	return TRUE;
}

LONG CSTXServerClientContextBase::Send(LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData)
{
	STXTRACELOGE(_T("[r][g][i]CSTXServerClientContextBase::Send is not implemented."));
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return 0;
}

BOOL CSTXServerClientContextBase::Clone(CSTXServerClientContextBase *pClientContext)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// CSTXServerContextBase

CSTXServerContextBase::CSTXServerContextBase()
{
	m_nRef = 1;
	m_dwServerParam = 0;
	m_nCreateTick = GetTickCount64();
}

CSTXServerContextBase::CSTXServerContextBase(DWORD_PTR dwServerParam)
{
	m_nRef = 1;
	m_dwServerParam = dwServerParam;
}

CSTXServerContextBase::~CSTXServerContextBase()
{
}

void CSTXServerContextBase::OnDestroy()
{

}

BOOL CSTXServerContextBase::OnInitialize()
{
	return TRUE;
}

void CSTXServerContextBase::SetServerParam(DWORD_PTR dwParam)
{
	m_dwServerParam = dwParam;
}

DWORD_PTR CSTXServerContextBase::GetServerParam()
{
	return m_dwServerParam;
}

void CSTXServerContextBase::SetServerParamString(LPCTSTR lpszStringParam)
{
	if (lpszStringParam)
		m_strServerParamString = lpszStringParam;
	else
		m_strServerParamString.clear();
}

STLSTRING CSTXServerContextBase::GetServerParamString()
{
	return m_strServerParamString;
}

ULONGLONG CSTXServerContextBase::GetRunTime()
{
	return GetTickCount64() - m_nCreateTick;
}

//////////////////////////////////////////////////////////////////////////
// CSTXTcpServerContextBase

CSTXTcpServerContextBase::CSTXTcpServerContextBase(DWORD_PTR dwServeParam) : CSTXServerContextBase(dwServeParam)
{

}

//////////////////////////////////////////////////////////////////////////
// CSTXUdpServerContextBase

CSTXUdpServerContextBase::CSTXUdpServerContextBase(DWORD_PTR dwServeParam) : CSTXServerContextBase(dwServeParam)
{

}

