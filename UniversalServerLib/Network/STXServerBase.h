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

#define STXSM_TIMER_INTERVAL				0x00000001		//ָ�� STXSERVERINIT.dwTimerInterval ��Ч
#define STXSM_OPERATION_TIMEOUT			0x00000002		//ָ�� STXSERVERINIT.dwDefaultOperationTimeout ��Ч
#define STXSM_BUFFER						0x00000004		//ָ�� STXSERVERINIT.dwBufferSize/dwBufferInitialCount/dwBufferMaxCount ��Ч
#define STXSM_ACCEPT						0x00000008		//ָ�� STXSERVERINIT.dwAcceptPost ��Ч
#define STXSM_LOG_FLAGS					0x00000010		//ָ�� STXSERVERINIT.dwLogFlags ��Ч

#define STXSF_IPV6						0x00000001		//���������ԣ�ָ����������֧��IPv6

#define STXSS_NORMAL				0x00000000
#define STXSS_TERMINATE			0x10000000		// �����־������ֹ�����߳�
#define STXSS_PRETERMINATE		0x20000000		// �������־��ʱ�������µ����󽫲�������


typedef struct tagSTXSERVERINIT
{
	DWORD cbSize;								//Size in byte of this structure
	DWORD dwMask;								//��������ֵ��Mask. ��� STXSM_XXX ��
	DWORD dwDefaultOperationTimeout;			//TCP ��ʱʱ�䣨Milliseconds��. ����һ���յ�����֮��Ĵ�ʱ���ڣ��������µ����ݵ����������TCP���ӻ��Զ����Ͽ�
	DWORD dwTimerInterval;						//�Դ��Ķ�ʱ��ʱ������Milliseconds��.Ĭ��Ϊ STXS_DEFAULT_TIMER_INTERVAL������������֮��CSTXServerBase::OnTimer �ᱻ�����Ե��ã��˱�������ָ���������
	DWORD dwLogFlags;							//��������־������� STXLF_XXX ��
	LPCTSTR pszLogOutputWindowTitle;			//��������־��ͨ��SendMessage��ʽ���͵����д˱���Ĵ��ڡ�
	DWORD dwServerFlags;						//���������ԡ�Ŀǰֻ��һ��ֵ���� : STXSF_IPV6. Ĭ��û�д˱�־(ֻ���� IPv4)
	DWORD dwLogBufferSize;						//��׼��־��������С���ַ�����
	DWORD dwFolderMonitorBufferSize;			//Ŀ¼���ݱ����ض�ȡ��������С���ֽ�����ȡֵ����1024��
	LPCTSTR pszIniFilePathName;					//���ָ���˴˱������� CSTXServerBase::GetServerModuleIniPathName ����������ָ����ֵ������ CSTXServerBase::GetServerModuleIniPathName ����Ĭ�ϵ� INI �ļ���ȫ·��
	LPCTSTR pszXmlFilePathName;					//���ָ���˴˱������� CSTXServerBase::GetServerModuleXmlPathName ����������ָ����ֵ������ CSTXServerBase::GetServerModuleXmlPathName ����Ĭ�ϵ� XML �ļ���ȫ·��
	LPCTSTR pszLogFilePath;						//���ָ���˴˱��������������־����д������ָ�����ļ��С������������־д��Ĭ�ϵ���־�ļ���
	DWORD dwBufferSize;							//���ݶ�ȡ��������С���ֽڣ���Ĭ��Ϊ STXS_DEFAULT_BUFFER_SIZE 
	DWORD dwBufferInitialCount;					//���ݶ�ȡ��������ʼ������Ĭ��Ϊ STXS_DEFAULT_BUFFER_COUNT 
	DWORD dwBufferMaxCount;						//���ݶ�ȡ��������������Ĭ��Ϊ STXS_DEFAULT_BUFFER_MAX_COUNT 
	DWORD dwAcceptPost;							//��ÿһ��TCP�ӷ���������ʼͶ�ݵ� Accept ��������
	DWORD dwAcceptBufferSize;					//Ϊÿ��Accept��������Ļ����С

	//TCP �ӷ��������Ժ���ͨ�� CSTXServerBase::BeginTcpServer ������
	UINT uTcpServerCount;				//��ʼ TCP �ӷ�����������Ĭ��Ϊ 0
	LPUINT pTcpServerPorts;				//��ʼ TCP �ӷ����������˿��б�������ĸ����� uTcpServerCount ��ָ��������
	DWORD_PTR* pTcpServerParameters;	//��ʼ TCP �ӷ������Զ������ֵ��������ĸ����� uTcpServerCount ��ָ����������ÿ��Ԫ�ص��� pTcpServerPorts ��һһ��Ӧ
	LPCTSTR* pszTcpServerParameterStrings;	//��ʼ TCP �ӷ������Զ����ַ�������ֵ��������ĸ����� uTcpServerCount ��ָ����������ÿ��Ԫ�ص��� pTcpServerPorts ��һһ��Ӧ

	//UDP �ӷ��������Ժ���ͨ�� CSTXServerBase::BeginUdpServer ������
	UINT uUdpServerCount;				//��ʼ UDP �ӷ�����������Ĭ��Ϊ 0
	LPUINT pUdpServerPorts;				//��ʼ UDP �ӷ������󶨶˿��б�������ĸ����� uTcpServerCount ��ָ��������
	DWORD_PTR* pUdpServerParameters;	//��ʼ UDP �ӷ������Զ������ֵ��������ĸ����� uTcpServerCount ��ָ����������ÿ��Ԫ�ص��� pTcpServerPorts ��һһ��Ӧ

	//TCP �����ӿ��Ժ���ͨ�� CSTXServerBase::CreateTcpConnection ������
	int nTcpConnection;					//��ʼ TCP �����Ӹ�����Ĭ��Ϊ 0�� ��������ָ�˷������������ӵ�������ַ��
	SOCKADDR* pAddrTcpConnect;			//��ʼ TCP ������Ŀ���ַ
	int nConnectAddrLen;				//��ʼ TCP ������Ŀ���ַ�ṹ�ֽ���

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
	DWORD dwStatus;								// ������ģ��״̬
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

	volatile LONGLONG m_nTimerAlternativeIDBase;		//Timer ��ΨһID�� 64λ����������ÿ��1000��ε��ٶ��£�ʹ��292��
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

	// ����һ�� TCP �ӷ��������ڶ˿� uPort �ϼ�����dwServerParam Ϊ�û��Զ�����ӷ���������
	virtual BOOL BeginTcpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString);

	// ����һ�� UDP �ӷ����������ڶ˿� uPort �ϡ�dwServerParam Ϊ�û��Զ�����ӷ���������
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


