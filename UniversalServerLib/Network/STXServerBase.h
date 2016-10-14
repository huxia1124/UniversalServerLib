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

#define WIN32_LEAN_AND_MEAN
//#define _WIN32_WINNT 0x0500		// Windows 2000 or later

#include <WTypes.h>
//#include <Mstcpip.h>
#include <WinSock2.h>
#include <tchar.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#include "STXLog.h"

#include <process.h>
#include <vector>
#include "STXUtility.h"
#include "STXLog.h"
#include "STXMemoryVariableNode.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

//////////////////////////////////////////////////////////////////////////

#define STXS_DEFAULT_TIMER_INTERVAL			60000
#define STXS_DEFAULT_ACCEPT_POST				64
#define STXS_DEFAULT_BUFFER_SIZE				8192
#define STXS_DEFAULT_BUFFER_COUNT			100
#define STXS_DEFAULT_BUFFER_MAX_COUNT		100000
#define STXS_DEFAULT_OPERATION_TIMEOUT		60000
#define STXS_DEFAULT_ACCEPT_BUFFER_SIZE		1024
#define STXS_DEFAULT_LOG_FLAGS				(STXLF_TIME | STXLF_THREADID | STXLF_CALLBACK_DEBUG | STXLF_CALLBACK_LOG)

#define STXSM_TIMER_INTERVAL				0x00000001		//指定 STXSERVERINIT.dwTimerInterval 有效
#define STXSM_OPERATION_TIMEOUT			0x00000002		//指定 STXSERVERINIT.dwDefaultOperationTimeout 有效
#define STXSM_BUFFER						0x00000004		//指定 STXSERVERINIT.dwBufferSize/dwBufferInitialCount/dwBufferMaxCount 有效
#define STXSM_ACCEPT						0x00000008		//指定 STXSERVERINIT.dwAcceptPost 有效
#define STXSM_LOG_FLAGS					0x00000010		//指定 STXSERVERINIT.dwLogFlags 有效

#define STXSF_IPV6						0x00000001		//服务器特性，指定此特性以支持IPv6

#define STXSS_NORMAL				0x00000000
#define STXSS_TERMINATE			0x10000000		// 这个标志用于中止工作线程
#define STXSS_PRETERMINATE		0x20000000		// 有这个标志的时候，所有新的请求将不再受理


typedef struct tagSTXSERVERINIT
{
	DWORD cbSize;								//Size in byte of this structure
	DWORD dwMask;								//各个配置值的Mask. 详见 STXSM_XXX 宏
	DWORD dwDefaultOperationTimeout;			//TCP 超时时间（Milliseconds）. 在上一次收到数据之后的此时间内，必须有新的数据到来，否则此TCP连接会自动被断开
	DWORD dwTimerInterval;						//自带的定时器时间间隔（Milliseconds）.默认为 STXS_DEFAULT_TIMER_INTERVAL。服务器启动之后，CSTXServerBase::OnTimer 会被周期性调用，此变量可以指定这个周期
	DWORD dwLogFlags;							//服务器日志输出项。详见 STXLF_XXX 宏
	LPCTSTR pszLogOutputWindowTitle;			//服务器日志将通过SendMessage方式发送到含有此标题的窗口。
	DWORD dwServerFlags;						//服务器特性。目前只有一个值可用 : STXSF_IPV6. 默认没有此标志(只开启 IPv4)
	DWORD dwLogBufferSize;						//标准日志缓冲区大小（字符数）
	DWORD dwFolderMonitorBufferSize;			//目录内容变更监控读取缓冲区大小（字节数，取值至少1024）
	LPCTSTR pszIniFilePathName;					//如果指定了此变量，则 CSTXServerBase::GetServerModuleIniPathName 将返回这里指定的值。否则 CSTXServerBase::GetServerModuleIniPathName 返回默认的 INI 文件的全路径
	LPCTSTR pszXmlFilePathName;					//如果指定了此变量，则 CSTXServerBase::GetServerModuleXmlPathName 将返回这里指定的值。否则 CSTXServerBase::GetServerModuleXmlPathName 返回默认的 XML 文件的全路径
	LPCTSTR pszLogFilePath;						//如果指定了此变量，则服务器日志将被写到这里指定的文件中。否则服务器日志写到默认的日志文件中
	DWORD dwBufferSize;							//数据读取缓冲区大小（字节）。默认为 STXS_DEFAULT_BUFFER_SIZE 
	DWORD dwBufferInitialCount;					//数据读取缓冲区初始个数。默认为 STXS_DEFAULT_BUFFER_COUNT 
	DWORD dwBufferMaxCount;						//数据读取缓冲区最大个数。默认为 STXS_DEFAULT_BUFFER_MAX_COUNT 
	DWORD dwAcceptPost;							//对每一个TCP子服务器，初始投递的 Accept 操作个数
	DWORD dwAcceptBufferSize;					//为每个Accept操作分配的缓存大小

	//TCP 子服务器可以后续通过 CSTXServerBase::BeginTcpServer 来建立
	UINT uTcpServerCount;				//初始 TCP 子服务器个数，默认为 0
	LPUINT pTcpServerPorts;				//初始 TCP 子服务器监听端口列表。此数组的个数即 uTcpServerCount 中指定的数量
	DWORD_PTR* pTcpServerParameters;	//初始 TCP 子服务器自定义参数值。此数组的个数即 uTcpServerCount 中指定的数量，每个元素的与 pTcpServerPorts 中一一对应
	LPCTSTR* pszTcpServerParameterStrings;	//初始 TCP 子服务器自定义字符串参数值。此数组的个数即 uTcpServerCount 中指定的数量，每个元素的与 pTcpServerPorts 中一一对应

	//UDP 子服务器可以后续通过 CSTXServerBase::BeginUdpServer 来建立
	UINT uUdpServerCount;				//初始 UDP 子服务器个数，默认为 0
	LPUINT pUdpServerPorts;				//初始 UDP 子服务器绑定端口列表。此数组的个数即 uTcpServerCount 中指定的数量
	DWORD_PTR* pUdpServerParameters;	//初始 UDP 子服务器自定义参数值。此数组的个数即 uTcpServerCount 中指定的数量，每个元素的与 pTcpServerPorts 中一一对应

	//TCP 外连接可以后续通过 CSTXServerBase::CreateTcpConnection 来建立
	int nTcpConnection;					//初始 TCP 外连接个数，默认为 0。 外连接是指此服务器主动连接到其它地址。
	SOCKADDR* pAddrTcpConnect;			//初始 TCP 外连接目标地址
	int nConnectAddrLen;				//初始 TCP 外连接目标地址结构字节数

	tagSTXSERVERINIT()
	{
		memset(this, sizeof(tagSTXSERVERINIT), 0);
		cbSize = sizeof(tagSTXSERVERINIT);
	}
}STXSERVERINIT, *LPSTXSERVERINIT;

typedef struct tagSTXSERVERINFORMATION
{
	DWORD cbSize;								//Size in byte of this structure
	DWORD dwDefaultOperationTimeout;
	DWORD dwTimerInterval;
	DWORD dwLogFlags;
	LPCTSTR pszLogOutputWindowTitle;
	DWORD dwServerFlags;
	DWORD dwAcceptBufferSize;
	LPCTSTR pszIniFilePathName;
	LPCTSTR pszXmlFilePathName;
	LPCTSTR pszLogFilePath;
	DWORD dwStatus;								// 服务器模块状态
	DWORD dwBufferSize;
	DWORD dwBufferInitialCount;
	DWORD dwBufferMaxCount;
	DWORD dwAcceptPost;
	DWORD dwLogBufferSize;
	DWORD dwFolderMonitorBufferSize;			//minimum is 1024
	tagSTXSERVERINFORMATION()
	{
		memset(this, sizeof(tagSTXSERVERINFORMATION), 0);
		cbSize = sizeof(tagSTXSERVERINFORMATION);
	}
}STXSERVERINFORMATION, *LPSTXSERVERINFORMATION;

//////////////////////////////////////////////////////////////////////////

//extern LPVOID g_pThreadTlsUserData;

extern CSTXLog g_LogGlobal;		//Global Log

//////////////////////////////////////////////////////////////////////////
class CSTXServerBase;
class CSTXServerClientContextBase;
class CSTXServerContextBase;
class CSTXTcpServerContextBase;
class CSTXUdpServerContextBase;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CSTXServerObjectPtr

template<class T>
class CSTXServerObjectPtr
{
protected:
	T* _p;
public:
	CSTXServerObjectPtr()
	{
		_p = NULL;
	}
	CSTXServerObjectPtr(T* pObj)
	{
		_p = pObj;
		if(pObj)
		{
			pObj->AddRef();
		}

	}

	CSTXServerObjectPtr(const CSTXServerObjectPtr<T>&val)
	{
		_p = NULL;
		*this = val;
	}
	~CSTXServerObjectPtr()
	{
		if(_p)
		{
			_p->Release();
			_p = NULL;
		}
	}
	CSTXServerObjectPtr& operator=(T* pObj)
	{
		T* pOriginal = _p;

		_p = pObj;
		if(pObj)
		{
			pObj->AddRef();
		}

		if(pOriginal)
		{
			pOriginal->Release();
		}

		return *this;
	}
	CSTXServerObjectPtr& operator=(const CSTXServerObjectPtr<T>&val)
	{
		T* pOriginal = _p;

		_p = val._p;
		if(val._p)
		{
			val._p->AddRef();
		}

		if(pOriginal)
		{
			pOriginal->Release();
		}
		return *this;
	}
	operator T*()
	{
		return _p;
	}
	T* operator->()
	{
		return _p;
	}
};


//////////////////////////////////////////////////////////////////////////
// CSTXServerBase

class CSTXServerBase : public ISTXLogCallback
{
public:
	CSTXServerBase(void);
	virtual ~CSTXServerBase(void);

protected:
	typedef struct tagTimerParam
	{
		CSTXServerBase *pThis;
		LPVOID pTimerParam;
		LONGLONG nAltID;
		HANDLE hTimer;
		tagTimerParam()
		{
			m_nRef = 1;
		}
		virtual LONG AddRef()
		{
			return InterlockedIncrement(&m_nRef);
		}
		virtual LONG Release()
		{
			LONG nRef = InterlockedDecrement(&m_nRef);
			if (nRef == 0)
			{
				delete this;
			}
			return nRef;
		}
	protected:
		LONG m_nRef;
	}TimerParam;

protected:
	HMODULE m_hModuleHandle;
	TCHAR m_szModuleFilePathName[MAX_PATH];		//Full path with file name
	LPTSTR m_pszModuleFileName;					//File Name only, with extension
	TCHAR m_szModuleFileTitle[MAX_PATH];		//File Name only, without extension;
	TCHAR m_szModuleFilePath[MAX_PATH];			//Path only. with '\' at the end
	TCHAR m_szIniFilePath[MAX_PATH];			//INI filepath, Full Path with filename
	TCHAR m_szXmlFilePath[MAX_PATH];			//XML filepath, Full Path with filename

	UINT m_uTimerThreadID;
	HANDLE m_hTimerThread;
	HANDLE m_hTimerThreadEvent;					//Signaled when timer thread needed to be terminated
	HANDLE m_hTimerThreadIntervalChangeEvent;	//Signaled when timer interval changed

	volatile LONGLONG m_nTimerAlternativeIDBase;		//Timer 的唯一ID。 64位整数可以在每秒1000万次的速度下，使用292年
	volatile LONGLONG m_nTimerQueueRef;

	STXSERVERINFORMATION m_BaseServerInfo;
	HANDLE m_hTimerQueue;
	static CSTXHashMap<LONGLONG, CSTXServerObjectPtr<TimerParam>, 16> m_mapTimerDataAlt;
	static CSTXServerBase *s_pThis;

	CSTXMemoryVariableNode m_variableTreeRoot;

protected:
	virtual CSTXServerClientContextBase* OnCreateClientContext(CSTXServerContextBase *pServerContext);
	virtual CSTXTcpServerContextBase* OnCreateTcpServerContext(DWORD_PTR dwServerParam);
	virtual CSTXUdpServerContextBase* OnCreateUdpServerContext(DWORD_PTR dwServerParam);
	virtual void OnClientContextInitialized(CSTXServerClientContextBase *pClientContext);
	virtual BOOL Initialize(HMODULE hModuleHandle, LPSTXSERVERINIT lpInit);
	virtual BOOL OnInitialization();
	virtual void OnShutDown();
	virtual void OnFinalShutDown();
	virtual void OnTimer(DWORD dwInterval);
	virtual void OnTimerInitialize();
	virtual void OnTimerUninitialize();
	virtual void OnTimerTimeout(LPVOID lpTimerParam);
	virtual LRESULT Run() = 0;

private:
	static UINT WINAPI ServerTimerThread(LPVOID lpParameter);
	static void CALLBACK ServerBaseTimerCallback(__in  PVOID lpParameter, __in  BOOLEAN TimerOrWaitFired);
	static int ProcessException(LPEXCEPTION_POINTERS pExp);


	virtual void InternalTerminate();
	void OnTimerTimeoutWrapper(LPVOID lpTimerParam);

public:
	virtual void StartServer();
	virtual BOOL Terminate();
	virtual void OutputLog(LPCTSTR lpszLogFmt, ...);
	virtual void OutputLogDirect(LPCTSTR lpszLogFmt, ...);

	// 启动一个 TCP 子服务器，在端口 uPort 上监听。dwServerParam 为用户自定义的子服务器参数
	virtual BOOL BeginTcpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString);

	// 启动一个 UDP 子服务器，绑定在端口 uPort 上。dwServerParam 为用户自定义的子服务器参数
	virtual CSTXServerContextBase* BeginUdpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString);

	virtual LONG CreateTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, int nAddressFamily, BOOL bKeepConnect);


	LPCTSTR GetServerModuleFilePathName();		//e.g.  "D:\Folder\MyServer.exe"
	LPCTSTR GetServerModuleFileName();			//e.g.  "MyServer.exe"
	LPCTSTR GetServerModuleFilePath();			//e.g.  "D:\Folder\"
	LPCTSTR GetServerModuleFileTitle();			//e.g.  "MyServer"
	LPCTSTR GetServerModuleIniPathName();		//e.g.  "D:\Folder\MyServer.ini"
	LPCTSTR GetServerModuleXmlPathName();		//e.g.  "D:\Folder\MyServer.xml"

	void EnableConsoleOutput(BOOL bEnabled = TRUE);

	void GetBaseServerInfo(LPSTXSERVERINFORMATION pInfo);
	void SetTcpClientOperationDefaultTimeOut(DWORD dwTimeoutMS);

public:
	//INI file operations
	UINT GetServerProfileString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPTSTR lpszBuf, DWORD cchBufLen, LPCTSTR lpszDefault);
	UINT GetServerProfileInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT nDefault);
	DWORD GetTimerInterval();

protected:
	virtual BOOL OnInitializeServerLog();

protected:
	HANDLE AddTimerObject(LPVOID lpTimerParam, DWORD dwDueTime, LONGLONG *pAltID = NULL);
	BOOL DeleteTimerObject(HANDLE hTimer, LONGLONG nAltId);
	LONG64 AddTimerQueueRef();
	LONG64 ReleaseTimerQueueRef();
	void RegisterServerBaseVariablesForVariableTree();
	CSTXMemoryVariableNode& GetVariableTreeRoot();

};

//////////////////////////////////////////////////////////////////////////
// CSTXServerContextBase

class CSTXServerContextBase
{
public:
	CSTXServerContextBase();
	CSTXServerContextBase(DWORD_PTR dwServerParam);
	virtual ~CSTXServerContextBase();

protected:
	LONG m_nRef;
	DWORD_PTR m_dwServerParam;
	STLSTRING m_strServerParamString;
	ULONGLONG m_nCreateTick;

public:
	virtual BOOL OnInitialize();
	virtual void OnDestroy();

public:
	void SetServerParam(DWORD_PTR dwParam);
	DWORD_PTR GetServerParam();
	void SetServerParamString(LPCTSTR lpszStringParam);
	STLSTRING GetServerParamString();
	ULONGLONG GetRunTime();

public:
	LONG AddRef()
	{
		return InterlockedIncrement(&m_nRef);
	}
	LONG Release()
	{
		LONG nRef = InterlockedDecrement(&m_nRef);
		if(nRef == 0)
		{
			delete this;
		}
		return nRef;
	}


};

//////////////////////////////////////////////////////////////////////////
// CSTXServerClientContextBase

class CSTXServerClientContextBase
{
public:
	CSTXServerClientContextBase();
	virtual ~CSTXServerClientContextBase();

protected:
	LONG m_nRef;

protected:
	virtual BOOL OnInitialize();
	virtual void OnDestroy();

public:
	virtual LONG Send(LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData);
	virtual BOOL Clone(CSTXServerClientContextBase *pClientContext);
	
	virtual LONG AddRef()
	{
		//STXTRACELOG(_T("C-Add >> %d"), m_nRef + 1);
		//ShowCallStack();
		return InterlockedIncrement(&m_nRef);
	}
	virtual LONG Release()
	{
		//STXTRACELOG(_T("C-Rel >> %d"), m_nRef - 1);
		//ShowCallStack();
		LONG nRef = InterlockedDecrement(&m_nRef);
		if(nRef == 0)
		{
			delete this;
		}
		return nRef;
	}
};

//////////////////////////////////////////////////////////////////////////
// CSTXTcpServerContextBase

class CSTXTcpServerContextBase : public CSTXServerContextBase
{
public:
	CSTXTcpServerContextBase(DWORD_PTR dwServeParam);

};

//////////////////////////////////////////////////////////////////////////
// CSTXUdpServerContextBase

class CSTXUdpServerContextBase : public CSTXServerContextBase
{
public:
	CSTXUdpServerContextBase(DWORD_PTR dwServeParam);

};


