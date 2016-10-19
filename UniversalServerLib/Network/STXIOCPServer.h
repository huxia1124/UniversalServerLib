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

#include "STXServerBase.h"
#include "STXIOCPBuffer.h"
#include "STXUtility.h"

#include <wininet.h>

#pragma warning(disable:4786)
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <memory>
#include <concurrentqueue.h>
#include <mutex>

using namespace std;

//////////////////////////////////////////////////////////////////////////

#define STXIOCPSERVER_BUFFER_COUNT				10					// Initial Buffer Count - ��ȡ�������ĳ�ʼ������
#define STXIOCPSERVER_BUFFER_SIZE				(8 * 1024)			// Buffer Initial Size
#define STXIOCPSERVER_BUFFER_MAXCOUNT			100000				// Maximum Buffer Count
#define STXIOCP_FOLDER_MONITOR_BUFFER_SIZE		2048				// Default Buffer size for folder monitoring

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//Forward declaration

class CSTXIOCPServer;
class CSTXIOCPServerClientContext;
class CSTXIOCPServerContext;
class CSTXIOCPTcpServerContext;
class CSTXIOCPUdpServerContext;
class CSTXIOCPTcpConnectionContext;
class CSTXIOCPFolderMonitorContext;

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPServer

// Socket Type (ContextKey Type), See STXIOCPCONTEXTKEY::nSocketType
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_CLIENT				1		//Accepted client socket (TCP)
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_TCPSERVER			2		//Tcp listener socket
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_UDPSERVER			3		//Udp receiver socket
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_TCP_CONNECTION		4		//Tcp connection socket to other server
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_FILE					5		//File to read
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_MONITORED_DIR		6		//Monitored Folder
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_FAKE					8		//A Fake socket for custom operation
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_HTTP					16		//HTTP Request
#define STXIOCP_CONTEXTKEY_SOCKET_TYPE_TERMINATE			10		//(Virtual)Temp Socket for Terminate

// IO Operation Type, See STXIOCPOPERATION::nOpType
#define STXIOCP_OPERATION_ACCEPT					1				// [Tcp Listening Socket]	AcceptEx
#define STXIOCP_OPERATION_READ						2				// [TCP Client Socket, UDP Socket]	WSARecv WSARecvFrom
#define STXIOCP_OPERATION_WRITE						3				// [TCP Client Socket, UDP Socket]	WSASend WSASendTo
#define STXIOCP_OPERATION_CONNECT					4				// [Tcp Connection Socket]	ConnectEx
#define STXIOCP_OPERATION_DISCONNECT				5				// [TCP Client Socket]	DisconnectEx
#define STXIOCP_OPERATION_MONITOR					6				// [Monitored Dir]
#define STXIOCP_OPERATION_CUSTOM					200				// Custom operation
#define STXIOCP_OPERATION_FILE_CHANGE_NOTIFY		201				// Internal Custom operation, used for file change notification
#define STXIOCP_OPERATION_TERMINAT					10				// [Working Thread Terminate Signal]

#define STXIOCP_SERVER_CONNECTION_FLAG_KEEPCONNECT		0x00000001		// TCP connection flags, indicate that this connection should be kept and try reconnect when disconnected

#define STXIOCP_CUSTOM_OPERATION_QUEUED				0		//�������ڶ����еȴ����
#define STXIOCP_CUSTOM_OPERATION_COMPLETE			1		//�����ɹ����
#define STXIOCP_CUSTOM_OPERATION_FAILED				2		//����ʧ��
#define STXIOCP_CUSTOM_OPERATION_CANCELED			3		//�����Ѿ���ȡ��
#define STXIOCP_CUSTOM_OPERATION_TIMEOUT			5		//������ʱ

#define STXIOCP_SOCKET_IPV4							4
#define STXIOCP_SOCKET_IPV6							6

// STXIOCP_ACCEPT_ ������ IssueAccept() �����ϣ�ָ�� IssueAccept() ���ĸ�Э����Ͷ�� Accept ����
#define STXIOCP_ACCEPT_IPV4						0x0001
#define STXIOCP_ACCEPT_IPV6						0x0002
#define STXIOCP_ACCEPT_ALL						(STXIOCP_ACCEPT_IPV4 | STXIOCP_ACCEPT_IPV6)

//////////////////////////////////////////////////////////////////////////
//

#define STXIOCP_INTERNAL_OP_UPLOAD					1		//�ϴ�����
#define STXIOCP_INTERNAL_OP_DOWNLOAD				2		//���ز���
#define STXIOCP_INTERNAL_OP_DOWNLOAD_PASSIVE		3		//�������ز���

// �ڲ�ʵ���ϴ����������ݵ���Ϣͷ
typedef struct tagSTXIOCPSERVERMESSAGEHEADER
{
	WORD wHeaderSize;		// ��Ϣͷ�Ĵ�С(�ֽ�)����������ֵ��Ϊ sizeof(STXIOCPSERVERMESSAGEHEADER)
	DWORD dwMagic;			// Magic Number
	WORD wOpType;			// ʹ�� STXIOCP_INTERNAL_OP_ ��
	DWORD dwContentSize;	// dwContentSize ����ָʾ�ڴ���Ϣͷ֮��β�����ݵĴ�С��
}STXIOCPSERVERMESSAGEHEADER, *LPSTXIOCPSERVERMESSAGEHEADER;

#define STXIOCP_INTERNAL_OPCODE_ACK					0x8000

#define STXIOCP_INTERNAL_OPCODE_INITIALIZATION			0		// ��ʼ��
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK			1		// ���ݿ�
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH			2		// ����֪ͨ

// �ڲ�ʵ���ϴ����ݵ���Ϣͷ���� STXIOCPSERVERMESSAGEHEADER �ṹ��ʼ
typedef struct tagSTXIOCPSERVERUPLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//ʹ�� STXIOCP_INTERNAL_OPCODE_*** ��

	// dwCookie �����ڴ�������б���һ����Ա��δ���Ķ��б�š����������ڷ��������ɵġ�
	// �� wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION ʱ���ͻ����ڷ�����Ϣ�л�õ����ϵͳ�����ı�š�
	// �� wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK �� wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH ʱ
	//    ͨѶ˫����Ӧ��ʹ�������Ž��б�ʶ
	DWORD dwCookie;

	// dwUserData �ǿͻ��˿�������ָ����һ���û��Զ���ֵ��
	// �� wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION ʱ���ͻ�����Ҫָ�����ֵ������һ�δ�������в��ٸı����ֵ
	// �ں��������ݴ����У����ֵ��һֱ�����ڴ���Ϣͷ�С��ͻ���Ӧ����֤���͵�ÿһ�����ݰ���ָ�������ֵ
	DWORD dwUserData;

	// dwContentSize ����ָʾ�ڴ���Ϣͷ֮��β�����ݵĴ�С(�ֽ�)��
	DWORD dwContentSize;

	// dwTransferSize ���ڷ�����Ϣͷ�У�����ָʾʵ�ʴ����˶����ֽڵ�����
	// dwTransferSize ���� dwContentSize ��Ӧ�ġ�ͨ������ dwContentSize ����ͬ��ֵ
	//      �����Ƿ�����δ�ܳɹ����� dwContentSize �ֽڵ�����
	DWORD dwTransferSize;
}STXIOCPSERVERUPLOADFILE, *LPSTXIOCPSERVERUPLOADFILE;

//#define STXIOCP_INTERNAL_OPCODE_INITIALIZATION		0	// ��ʼ�����������ϴ�����ͬһ����
#define STXIOCP_INTERNAL_OPCODE_DOWNLOAD_BLOCK		1	// ���ݿ�
#define STXIOCP_INTERNAL_OPCODE_DOWNLOAD_FINISH		2	// ����֪ͨ���������أ��ͻ��˿ɲ����� FINISH ֪ͨ

// �ڲ�ʵ���������ݵ���Ϣͷ���� STXIOCPSERVERMESSAGEHEADER �ṹ��ʼ
typedef struct tagSTXIOCPSERVERDOWNLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//ʹ�� STXIOCP_INTERNAL_OPCODE_ ��
	DWORD dwCookie;
	DWORD dwUserData;
	DWORD dwContentSize;

	// �ͻ����������������ݿ�ʱ����Ҫָ�� dwOffset ��Ϊ��ȡ���ݵ���ʼƫ����(�ֽ�)��
	DWORD dwOffset;
	DWORD dwTransferSize;
}STXIOCPSERVERDOWNLOADFILE, *LPSTXIOCPSERVERDOWNLOADFILE;

// �ڲ�ʵ�ֱ����������ݵ���Ϣͷ���� STXIOCPSERVERMESSAGEHEADER �ṹ��ʼ
typedef struct tagSTXIOCPSERVERPASSIVEDOWNLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//ʹ�� STXIOCP_INTERNAL_OPCODE_ ��
	DWORD dwCookie;
	DWORD dwUserData;
	WORD wResult;			//0 ��ʾ�ɹ��������ʾʧ�ܴ���
	__int64 dwOffset;
	DWORD dwContentSize;
}STXIOCPSERVERPASSIVEDOWNLOADFILE, *LPSTXIOCPSERVERPASSIVEDOWNLOADFILE;

//STXIOCPCONTEXTKEY �˽ṹ��Ӧ��ɶ˿��ϵ� Context Key
typedef struct tagSTXIOCPCONTEXTKEY
{
	TCHAR szDescription[32];		// A Description about the socket type
	SOCKET sock;					// Socket
	SOCKET sock6;							//Socket of IPv6
	UINT nSocketType;				// Socket Type

	CSTXServerObjectPtr<CSTXIOCPServerClientContext> pClientContext;
	tr1::shared_ptr<CSTXIOCPTcpServerContext> pTcpServerContext;
	CSTXServerObjectPtr<CSTXIOCPUdpServerContext> pUdpServerContext;
	CSTXServerObjectPtr<CSTXIOCPTcpConnectionContext> pTcpConnectionContext;
	CSTXServerObjectPtr<CSTXIOCPFolderMonitorContext> pFolderMonitorContext;
	HANDLE hMonitoredDir;
	TCHAR szMonitoredFolder[MAX_PATH];		//�������ļ���ȡʱ������Ϊ�ļ���
	DWORD dwMonitorNotifyFilter;			//�������ļ���ȡʱ������Ϊ�����־
	HANDLE hFile;
	DWORD dwCookie;
	DWORD dwUserData;
	DWORD_PTR dwFileTag;
	__int64 nBytesRead;		//�Ѿ���ȡ�˶����ֽڡ�����ָʾ�´ζ�ȡʱ��ƫ����

	LONG nAcceptPending;	//���� TCP Server, ��Ҫ���ٻ��ж��ٸ�Accept��Ȼû�д����꣬���ܾ����Ƿ���ɾ���˶���

}STXIOCPCONTEXTKEY, *LPSTXIOCPCONTEXTKEY;


// Operation flags, used in STXIOCPOPERATION
#define STXIOCP_OPF_CANCELED					0x00000001
#define STXIOCP_OPF_PROCESSED					0x00000002


//STXIOCPOPERATION �ṹ���ڱ�������Ͷ������ɶ˿��ϵĲ�����ʹ���� OVERLAPPED β������
struct tagSTXIOCPOPERATION
{
	OVERLAPPED ov;			// �˲����� OVERLAPPED �ṹ
	SOCKET sock;			// Socket
	UINT nOpType;			// ��������.�� STXIOCP_OPERATION_ ��

	
	//TCP Client:
	CSTXServerObjectPtr<CSTXIOCPServerClientContext> pClientContext;

	//TCP Server:
	SOCKET sockNewAccept;
	char *pszOutputBuffer;			//Buffer of peer address

	//UDP Server
	char szUdpAddressBuffer[256];		//buffer allocated for recvfrom address (IPv4/IPv6)
	INT nUdpAddressLen;					//address len

	//TCP Connection
	CSTXServerObjectPtr<CSTXIOCPTcpConnectionContext> pTcpConnectionContext;

	//Folder Monitoring
	HANDLE hMonitoredDir;
	char *pMonitorBuffer;
	INT nMonitorBufferLength;
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;

	// Custom Operation
	DWORD_PTR dwOperationID;
	DWORD_PTR dwUserData;
	DWORD_PTR dwUserData2;
	DWORD_PTR dwCompleteType;		//See STXIOCP_CUSTOM_OPERATION_* Macros

	//Common Properties
	DWORD dwSubmitTime;		// �˲����ύʱ��ϵͳ Tick ֵ 
	DWORD dwTimeOut;		// �˲�����ʱ��
	DWORD dwOriginalTimeOut;		// �˲�����ʱ��
	UINT nSocketType;		// �˲�����Ӧ�� Socket ����
	int nIPVer;				// �˲�����Ӧ�� Socket ��ʹ�õ� IP Э��汾�š� 4 �� 6
	DWORD dwFlags;			// �˲�����Flags.
	CSTXIOCPBuffer *pBuffer;	// Ϊ�˲�������Ļ���������ָ��.�����������͵Ĳ�������Ҫ�������������������Ҫ���������� pBuffer Ϊ NULL

	std::recursive_mutex lock;
	LONG m_nRef;
	tr1::shared_ptr<STXIOCPCONTEXTKEY> pContextKey;		//��ContextKey������

	HANDLE hTimerHandle;	//��ʱ��Handle,���ڳ�ʱ���
	LONGLONG nTimerAltId;	//Timer alternative ID. as a key in timer map

	//Source Information. For Debugging
	LPCTSTR pszSrcFile;		//Source file in which this operation object was created(allocated)
	int nLine;				//At which line this operation object was created

public:
	tagSTXIOCPOPERATION::tagSTXIOCPOPERATION(LPCTSTR lpszSourceFile, int nLine)
	{
		memset(&ov, 0, sizeof(ov));
		sock = INVALID_SOCKET;
		hMonitoredDir = INVALID_HANDLE_VALUE;
		pMonitorBuffer = NULL;
		nMonitorBufferLength = 0;
		nOpType = 0;
		dwSubmitTime = 0;
		dwTimeOut = 0;
		dwOriginalTimeOut = 0;
		nSocketType = 0;
		nIPVer = 0;
		dwFlags = 0;
		pBuffer = NULL;
		lpCompletionRoutine = NULL;
		m_nRef = 1;
		hTimerHandle = NULL;
		nTimerAltId = -1;

		pszSrcFile = lpszSourceFile;
		nLine = nLine;
	}
	tagSTXIOCPOPERATION::~tagSTXIOCPOPERATION()
	{
		//STXTRACE(_T("STXIOCPOPERATION Delete \t%p"), this);
		//ShowCallStack();

		if(pMonitorBuffer)
			delete []pMonitorBuffer;
	}

public:
	void Lock()
	{
		lock.lock();
	}
	void Unlock()
	{
		lock.unlock();
	}
	LONG AddRef()
	{
		return InterlockedIncrement(&m_nRef);
	}
	LONG Release()
	{
		LONG nRef = InterlockedDecrement(&m_nRef);
		if(nRef == 0)
		{
			/*
			TCHAR *szSockTypes[] = 
			{
				_T(""),
				_T("Client"),
				_T("TCP Server"),
				_T("UDP Server"),
				_T("TCP Connection"),
				_T("File"),
				_T("Monitor Dir"),
				_T(""),
				_T("Fake(Custom)"),
				_T(""),
				_T("Terminate")
			};


			TCHAR *szOpTypes[] = 
			{
				_T(""),
				_T("Accept"),
				_T("Read"),
				_T("Write"),
				_T("Connect"),
				_T("Disconnect"),
				_T("Monitor"),
				_T(""),
				_T("Custom"),
				_T(""),
				_T("Terminate")
			};

			TCHAR *szIpVer[] = 
			{
				_T(""),
				_T(""),
				_T(""),
				_T(""),
				_T("IPv4"),
				_T(""),
				_T("IPv6"),
			};

			STXTRACE(_T("[*]<%s> Operation %p Deleted : Socket Type = %s, OpType = %s  @Line = %d"), szIpVer[nIPVer], this, szSockTypes[nSocketType], szOpTypes[nOpType], nLine);
			//*/
			delete this;
		}
		return nRef;
	}
	void MarkProcessed()
	{
#ifdef _DEBUG
		if(dwFlags & STXIOCP_OPF_PROCESSED)
			DebugBreak();
#endif
		dwFlags |= STXIOCP_OPF_PROCESSED;
	}
	void MarkCanceled()
	{
#ifdef _DEBUG
		if(dwFlags & STXIOCP_OPF_CANCELED)
			DebugBreak();
#endif
		dwFlags |= STXIOCP_OPF_CANCELED;
	}
};

typedef struct tagSTXIOCPOPERATION STXIOCPOPERATION, *LPSTXIOCPOPERATION;

#define HTTP_DOWNLOAD_BUFFER_SIZE			16384

#define HTTP_CONTEXT_CONNECT				1
#define HTTP_CONTEXT_REQUEST				2
#define HTTP_CONTEXT_DOWNLOAD				3

#define HTTP_OPERATION_CONNECT				1
#define HTTP_OPERATION_REQUEST				2
#define HTTP_OPERATION_REQUEST_SEND			3
#define HTTP_OPERATION_REQUEST_READ			4
#define HTTP_OPERATION_REQUEST_FINISHED		5

// HTTP������û��Զ�������(Context)
typedef struct tagSTXIOCPSERVERHTTPCONTEXT
{
	CSTXIOCPServer *pServer;
	DWORD dwRequestType;		// �������ͣ��� HTTP_CONTEXT_��
	HINTERNET hInternet;
	HINTERNET hConnect;
	HINTERNET hRequest;
	TCHAR szUrl[256];
	INTERNET_BUFFERS buff;
	DWORD_PTR dwUserData;		// �û�����
	DWORD dwHttpResponseCode;
	CSTXIOCPBuffer bufferContent;
	STLSTRING stringParam;		// �û����� String
	tagSTXIOCPSERVERHTTPCONTEXT()
	{
		pServer = 0;
		hInternet = 0;
		hConnect = 0;
		hRequest = 0;
		buff.lpvBuffer = 0;
		dwHttpResponseCode = 0;
	}

	~tagSTXIOCPSERVERHTTPCONTEXT()
	{
		if(buff.lpvBuffer)
			delete []buff.lpvBuffer;

		if(hRequest)
			InternetCloseHandle(hRequest);
		if(hConnect)
			InternetCloseHandle(hConnect);
		if(hInternet)
			InternetCloseHandle(hInternet);
	}
}STXIOCPSERVERHTTPCONTEXT, *LPSTXIOCPSERVERHTTPCONTEXT;

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPServer

class CSTXIOCPServer : public CSTXServerBase
{

protected:


	//READPENDING �򻺳�����������ŶӵĶ�������READPENDING �� 4 ����Ա��Ӧ�� IssueRead �� 4 ������
	typedef struct tagREADPENDING
	{
		SOCKET sock;					//Socket
		UINT nSocketType;				//Socket Type
		DWORD_PTR dwExtraData;			
		DWORD_PTR dwExtraData2;			
		DWORD dwTimeOut;				//TimeOut
		int nAddressFamily;
	}READPENDING, *LPREADPENDING;

public:
	CSTXIOCPServer(void);
	virtual ~CSTXIOCPServer(void);

	friend class CSTXIOCPServerClientContext;
	friend class CSTXIOCPTcpConnectionContext;

protected:
	LPFN_DISCONNECTEX _lpfnDisconnectEx;
	LPFN_CONNECTEX _lpfnConnectEx;

protected:
	DWORD m_dwIOWait;												//�����̵߳Ŀ��г�ʱʱ��
	HANDLE m_hIOCP;													//��ɶ˿ھ��
	map<UINT, tr1::shared_ptr<CSTXIOCPTcpServerContext> > m_mapTcpServers;	//TCP �ӷ����� map	<�˿ں�, �ӷ�������Ϣ>
	map<UINT, CSTXServerObjectPtr<CSTXIOCPUdpServerContext> > m_mapUdpServers;	//UDP �ӷ����� map	<�˿ں�, �ӷ�������Ϣ>
	map<LONG, CSTXServerObjectPtr<CSTXIOCPTcpConnectionContext> > m_mapConnections;		//���� TCP ���� map	<����ID, ������Ϣ>
	queue<READPENDING> m_queueRead;									//�򻺳�����������ŶӵĶ���������

	vector<HANDLE> m_arrWorkerThreads;								//�����߳̾������
	CSTXHashSet<UINT, 8> m_setWorkerThreadId;						
	CSTXIOCPBufferCollection m_Buffers;								//���������϶���,���������ķ���

	CSTXHashMap<std::wstring, CSTXServerObjectPtr<CSTXIOCPServerClientContext>, 128, 4, CSTXDefaultWStringHash<128, 4> > m_mapClientContext;	//TCP �ͻ��� Context map <SOCKET, �ͻ��� Context ָ��>
	CSTXHashSet<LPSTXIOCPOPERATION, 128, sizeof(void*)> m_setOperation;							//����Ͷ�ݵ���ɶ˿��ϵĲ����ļ���
	CSTXHashMap<DWORD_PTR, LPSTXIOCPOPERATION, 64, 1> m_mapCustomOperation;				//�����û��Զ������
	CSTXHashMap<DWORD_PTR, LPSTXIOCPOPERATION, 64, 1> m_mapInternalCustomOperation;				//�����ڲ��Զ������

	CRITICAL_SECTION m_csServersMap;								// m_mapTcpServers �� m_mapUdpServers
	CRITICAL_SECTION m_csPending;									// m_queueRead
	CRITICAL_SECTION m_csConnections;								// m_mapConnections
	CRITICAL_SECTION m_csKillAllConnectionsLock;					// Used to prevent killing connection while creating connection

	static LONG s_nTcpConnectionIDBase;					//ID seed for TCP connection
	HANDLE m_hServerUnlockEvent;

	CRITICAL_SECTION m_csMonitoredDirSet;
	set<HANDLE> m_setMonitoredDir;
	CSTXHashSet<LPSTXIOCPCONTEXTKEY, 4, 8> m_setMonitoredDirToRemove;
	CSTXHashMap<HANDLE, LPSTXIOCPCONTEXTKEY, 8, 4> m_mapMonitoredDir;
	CSTXHashMap<LONGLONG, LPSTXIOCPCONTEXTKEY, 8, 4> m_mapIDtoMonitoredDir;

	CRITICAL_SECTION m_csFileDictionary;
	CSTXHashMap<__int64, STLSTRING> m_mapFileDictionary;		// LARGE_INTEGER->FilePathName. Internal implementation for file download

	STXIOCPCONTEXTKEY m_httpContextKey;
	LONG _numGracefulDisconnect = 0;
	LONG _numUnexpectDisconnect = 0;


private:
	static UINT WINAPI WorkThreadProc(LPVOID lpParam);				//IO Completion Port worker thread

public:
	static int SockaddrFromHost(LPCTSTR lpszHostAddress, UINT uPort, SOCKADDR *pAddr, int nAddressFamily = AF_INET);

	// ���û�������Ϣ
	BOOL SetBufferConfig(LONG nBufferSize = STXIOCPSERVER_BUFFER_SIZE, LONG nInitBufferCount = STXIOCPSERVER_BUFFER_COUNT, LONG nMaxBufferCount = STXIOCPSERVER_BUFFER_MAXCOUNT);

	// ��������������ģ�顣�˷���������������ģ��ֹͣ����ʱ�ŷ��ء�
	// �ڴ�֮ǰ�����ȵ��� Initialize ��������ʼ��һЩ������
	virtual void StartServer();

	// ֹͣ����������ģ��
	virtual BOOL Terminate();

	// ����һ�� TCP �ӷ��������ڶ˿� uPort �ϼ�����dwServerParam Ϊ�û��Զ�����ӷ���������
	virtual BOOL BeginTcpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString, UINT nPostAcceptCount = 0, LONG64 nLimitClientCount = 0);

	// ����һ�� UDP �ӷ����������ڶ˿� uPort �ϡ�dwServerParam Ϊ�û��Զ�����ӷ���������
	virtual CSTXServerContextBase* BeginUdpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString);

	// ֹͣ�ڶ˿� uPort �ϼ�������һ�� TCP �ӷ�������
	BOOL KillTcpServer(UINT uPort);
	// ֹͣ���ڶ˿� uPort �ϵ���һ�� UDP �ӷ�������
	BOOL KillUdpServer(UINT uPort);

	// ���Ŀǰ�������е��ӷ�����������������ǰ�������е� TCP �ӷ��������� + UDP �ӷ������ĸ���
	int GetRunningServerCount();

	// ֹͣ���е��ӷ��������˲�����Ӱ�������ģ�������
	// �˷������ر� socket, ��������Դ
	BOOL KillAllServers();

	// ��ʼ������������ģ�顣hModuleHandle Ϊ���̾������ʾ���������ģ������ hModuleHandle ���������ģ���Ҫ���� Log
	BOOL Initialize(HMODULE hModuleHandle, LPSTXSERVERINIT lpInit);

	// ����һ����������ַ(������)�� TCP ���ӡ������������� TCP ���ӵ� ID(�Ǹ�����)���ڵ��� SendTcpData ������ʱʹ�á����� -1 ��ʾ����ʧ��
	virtual LONG PendingTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, DWORD_PTR dwConnectionParam, LPCTSTR lpszServerParamString, int nAddressFamily, BOOL bKeepConnect, CSTXIOCPTcpConnectionContext *pKeepTcpConnCtx);
	virtual LONG CreateTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, DWORD_PTR dwConnectionParam, LPCTSTR lpszServerParamString, int nAddressFamily, BOOL bKeepConnect);
	virtual CSTXIOCPTcpConnectionContext *GetTcpConnectionContext(LONG nConnectionID);
	virtual LONG GetTcpConnectionCount();

	// �������ݸ�ָ���� TCP �ͻ���
	LONG SendClientData(CSTXIOCPServerClientContext *pClientContext, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData = 0);

	// ͨ��ָ�� ID �� TCP ����(�� CreateTcpConnection ����)�������ݡ�
	BOOL SendTcpData(LONG nConnectionID, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData = 0);

	// ͨ������ָ���˿��ϵ� UDP �ӷ�������ָ����Ŀ���ַ��Ŀ��˿ڷ������ݡ��� UDP �ӷ�����������һ�����������е� UDP �ӷ�����
	LONG SendUdpData(UINT uUdpSocketPort, LPVOID lpData, DWORD cbDataLen, LPCTSTR lpszTargetHostAddress, UINT uTargetPort, int nAddressFamily = AF_INET);

	// Ͷ��һ���Զ������
	BOOL PostCustomOperation(DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD dwTimeOut);
	BOOL PostCustomOperationUsingSprcifiedOperationType(UINT nOperationType, DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD_PTR dwUserData2, DWORD dwTimeOut);

	// ���һ���Զ������
	BOOL DoCustomOperation(DWORD_PTR dwOperationID, DWORD dwCompleteType = STXIOCP_CUSTOM_OPERATION_COMPLETE);

	// �Ͽ�һ���ͻ���
	void DisconnectClient(CSTXIOCPServerClientContext *pClientContext);
	BOOL DisconnectClientByTimeout(CSTXIOCPServerClientContext *pClientContext);

	// ����һ������Ŀ¼
	LONGLONG MonitorFolder(LPCTSTR lpszFolder, DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL);

	//�ر����м��Ŀ¼
	void CloseAllMonitoredFolder();

	// ��ʼ�첽��ȡ�ļ�
	BOOL IssueReadFile(LPCTSTR lpszFile, CSTXIOCPServerClientContext *pClientContext, DWORD_PTR dwTag);

	// ���ڲ��ļ��б������Ӻ�ɾ���ļ�����
	// �������õ��ļ����ع���
	BOOL AddFileReference(DWORD dwCookie, DWORD dwUserData, LPCTSTR lpszFile);
	BOOL RemoveFileReference(DWORD dwCookie, DWORD dwUserData);

	//��ʼһ���첽 URL ��������
	BOOL BeginDownloadURL(LPCTSTR lpszServer, WORD wPort, LPCTSTR lpszURL, DWORD_PTR dwUserData = 0, LPCTSTR lpszUserDataString = NULL);

	//���ú�ȡ�����Ŀ¼���ض��ļ���չ���ĺ���
	BOOL AddFolderMonitorIgnoreFileExtension(LONGLONG nFolderMonitorID, LPCTSTR lpszExt);
	BOOL RemoveFolderMonitorIgnoreFileExtension(LONGLONG nFolderMonitorID, LPCTSTR lpszExt);

	void RaiseCustomException(DWORD dwExceptionCode, DWORD nArguments = 0, ULONG_PTR *pArguments = NULL);

private:
	// ���·������ڸ���ʵ�ַ��������������������á�

	BOOL IssueReadFile(LPCTSTR lpszFile, CSTXIOCPServerClientContext *pClientContext, DWORD_PTR dwTag, BOOL bInternalUse, DWORD dwCookie, DWORD dwUserDataRemote);

	// Ϊָ���� socket Ӧ�� Keep-Alive ����
	BOOL KeepAlive(SOCKET sock, DWORD dwFirstTime, DWORD dwInterval);

	// �жϹ����߳��Ƿ���԰�ȫ��ֹ
	BOOL CanTerminateWorkThreads();

	// �ر����еĿͻ� Socket���˷������ر� socket, ��������Դ
	void KillAllClient();

	//�г�ָ��Ŀ¼�������ļ�����������Ŀ¼�е��ļ���
	void EnumFiles(LPCTSTR lpszFilter, const std::function<BOOL(LPCTSTR, DWORD_PTR)>& pfnEnumFunc, DWORD_PTR dwUserData);
	BOOL GetFileLastModifyTime(LPCTSTR lpszFile, LPFILETIME lpFileTime);

	//��ȡSOCKET��չ����
	BOOL GetSocketExtensionFunctions(SOCKET sock);

protected:
	// �ر����������� Socket���˷������ر� socket, ��������Դ
	void KillAllConnections();

	// ���� 2 ������������Դ���� StartServer ����֮ǰ���ã���Ӧ���������ط����á�
	void ClearAllClient();
	void ClearAllServers();
	void ClearAllFolderMonitor();

	// ���������̡߳������߳����� = CPU ���� * 2 + 2
	int CreateWorkerThreads();

	// �����Ǹ������϶�����ٽ��������

	void LockServersMap();
	void UnlockServersMap();

	void LockConnectionMap();
	void UnlockConnectionMap();


	// �������¼�Ԥ������. Ԥ������ֻ�Ƕ�ʵ�ʴ�����̵ķ�ղ������쳣�����Ա�֤������ȶ�
	BOOL PreClientReceive(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer, tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext);		//TCP Client receive a message
	BOOL OnClientReceivedWrapper(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	void PreTcpReceive(CSTXIOCPTcpConnectionContext *pTcpConnCtx, CSTXIOCPBuffer *pBuffer);
	void OnTcpReceivedWrapper(CSTXIOCPTcpConnectionContext *pTcpConnCtx, CSTXIOCPBuffer *pBuffer);
	void PreTcpDisconnect(CSTXIOCPTcpConnectionContext *pTcpConnCtx, DWORD dwError);
	void OnUdpServerReceivedWrapper(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer, SOCKADDR *pFromAddr, INT nAddrLen);
	void OnClientSentWrapper(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	void OnTcpSentWrapper(CSTXIOCPTcpConnectionContext *pTcpConnectionContext, CSTXIOCPBuffer *pBuffer);
	void OnTcpConnectWrapper(CSTXIOCPTcpConnectionContext *pTcpConnectionContext);
	void OnTcpDisconnectedWrapper(CSTXIOCPTcpConnectionContext *pTcpConnectionContext, DWORD dwError);
	void OnUdpServerSentWrapper(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer);
	void OnCustomOperationCompleteWrapper(DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD_PTR dwCompleteType);
	void OnUrlDownloadProgressWrapper(LPSTXIOCPSERVERHTTPCONTEXT pContext, int nBytesIncrease);
	void OnUrlDownloadCompleteWrapper(LPSTXIOCPSERVERHTTPCONTEXT pContext);
	void OnUrlDownloadCleanupWrapper(LPSTXIOCPSERVERHTTPCONTEXT pContext);
	void OnFolderChangedWrapper(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, LPCTSTR lpszFolder, FILE_NOTIFY_INFORMATION *pFileNotify);
	void OnFileChangedWrapper(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName);
	DWORD IsClientDataReadableWrapper(CSTXIOCPServerClientContext *pClientContext);
	DWORD IsTcpDataReadableWrapper(CSTXIOCPTcpConnectionContext *pTcpConnCtx);
	void OnWorkerThreadInitializeWrapper(LPVOID pStoragePtr);
	void OnWorkerThreadUninitializeWrapper(LPVOID pStoragePtr);
	DWORD OnGetClientReceiveTimeOutWrapper(CSTXIOCPServerClientContext *pClientContext);
	void OnClientDisconnectWrapper(CSTXIOCPServerClientContext *pClientContext);
	void OnPostClientDisconnectWrapper(CSTXIOCPServerClientContext *pClientContext);
	LPVOID OnAllocateWorkerThreadLocalStorageWrapper();
	void OnFreeWorkerThreadLocalStorageWrapper(LPVOID pStoragePtr);
	void OnWorkerThreadPreOperationProcessWrapper(LPVOID pStoragePtr);

	// �����ǹ����߳�����Ը������͵� Socket ���еĴ���.ֻ��ʹ���ڹ����߳���.
	void WTProcessTcpServer(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessUdpServer(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessClientSocket(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessTcpConnection(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessDirMonitor(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessFileRead(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessHttpOperation(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);

	// ����������ʵ�ֺ�����
	BOOL IssueClientRead(CSTXIOCPServerClientContext *pClientContext, DWORD dwTimeOut);
	BOOL IssueUdpRead(CSTXIOCPUdpServerContext *pUdpServerContext, DWORD dwTimeOut, int nAddressFamily);
	BOOL IssueTcpConnectionRead(CSTXIOCPTcpConnectionContext *pTcpConnectionContext, DWORD dwTimeOut);
	BOOL IssueClientDisconnectEx(CSTXIOCPServerClientContext *pClientContext, DWORD dwTimeOut = 0);

	LONG IssueClientWrite(CSTXIOCPServerClientContext *pClientContext, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwBufferUserData, DWORD dwTimeOut);
	LONG IssueClientWriteForceSend(CSTXIOCPServerClientContext *pClientContext, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwBufferUserData, DWORD dwTimeOut);
	LONG IssueTcpConnectionWrite(CSTXIOCPTcpConnectionContext *pTcpConnectionContext, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwBufferUserData, DWORD dwTimeOut);
	void PendingRead(SOCKET sock, UINT nSocketType, DWORD_PTR dwExtraData, DWORD_PTR dwExtraData2, DWORD dwTimeOut);

	LONG EnqueueOperation(LPSTXIOCPOPERATION lpOperation);
	LONG DequeueOperation(LPSTXIOCPOPERATION lpOperation);

	LONG GetNextTcpConnectionID();
	BOOL CreateSocket(SOCKET *pSocket4, SOCKET *pSocket6, int nSocketType = SOCK_STREAM, LPCTSTR lpszPort = NULL);

	LPSTXIOCPCONTEXTKEY Accept(SOCKET sockClient, tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext, LPVOID lpAddrBuffer, CSTXIOCPServerClientContext **ppContextContext, LPSTXIOCPOPERATION pOperation);
	BOOL IssueAccept(LPSTXIOCPCONTEXTKEY lpListenSockContextKey, DWORD dwAcceptFlags = STXIOCP_ACCEPT_ALL,  SOCKET sockReuse4 = INVALID_SOCKET, SOCKET sockReuse6 = INVALID_SOCKET);

	BOOL ReleaseClient(CSTXIOCPServerClientContext *pClientContext);

	static void __stdcall HttpDownloadCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpStatusInfo, DWORD dwStatusInfoLen);
	// Ͷ��һ��HTTP����
	BOOL PostHttpRequestOperation(LPSTXIOCPSERVERHTTPCONTEXT pContext, DWORD dwOperationType);

	static int ProcessException(LPEXCEPTION_POINTERS pExp);
	static LONGLONG GetTimeSpanSeconds(SYSTEMTIME & tmNow, SYSTEMTIME & tmPrev);
	virtual LPVOID GetThreadUserStorage();

protected:
	virtual BOOL OnAccepted(CSTXIOCPServerClientContext *pClientContext);
	virtual BOOL OnClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual void OnUdpServerReceived(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer, SOCKADDR *pFromAddr, INT nAddrLen);
	virtual void OnUdpServerSent(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer);
	virtual void OnClientSent(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual void OnClientDisconnect(CSTXIOCPServerClientContext *pClientContext);
	virtual void OnPostClientDisconnect(CSTXIOCPServerClientContext *pClientContext);
	virtual void OnClientDestroy(CSTXIOCPServerClientContext *pClientContext);
	virtual void OnTcpDisconnected(CSTXIOCPTcpConnectionContext *pTcpConnCtx, DWORD dwError);
	virtual void OnTcpReceived(CSTXIOCPTcpConnectionContext *pTcpConnCtx, CSTXIOCPBuffer *pBuffer);
	virtual void OnTcpSent(CSTXIOCPTcpConnectionContext *pTcpConnectionContext, CSTXIOCPBuffer *pBuffer);
	virtual void OnTcpConnect(CSTXIOCPTcpConnectionContext *pTcpConnectionContext);
	virtual void OnCustomOperationComplete(DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD_PTR dwCompleteType);
	virtual void OnInternalCustomOperationComplete(UINT nOperationType, DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD_PTR dwUserData2, DWORD_PTR dwCompleteType);
	virtual BOOL OnClientContextOperationTimeout(CSTXIOCPServerClientContext *pClientContext);
	virtual void OnFolderChanged(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, LPCTSTR lpszFolder, FILE_NOTIFY_INFORMATION *pFileNotify);
	virtual void OnFileChanged(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName);
	virtual void OnFileChangedFiltered(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName);
	virtual void OnFileRead(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer, DWORD_PTR dwTag);
	virtual void OnUrlDownloadProgress(LPSTXIOCPSERVERHTTPCONTEXT pContext, int nBytesIncrease);
	virtual void OnUrlDownloadComplete(LPSTXIOCPSERVERHTTPCONTEXT pContext);
	virtual void OnUrlDownloadCleanup(LPSTXIOCPSERVERHTTPCONTEXT pContext);
	virtual DWORD OnQueryWorkerThreadCount();

	//�ж�һ���ͻ����Ƿ��Ѿ����㹻����Ϣ�ɶ�������У�Ӧ������һ����Ϣ�Ĵ�С�����򷵻� 0
	virtual DWORD IsClientDataReadable(CSTXIOCPServerClientContext *pClientContext);

	//�����ض�ͨѶЭ�飬����Ҫ�ж��Ƿ��Ѿ��յ�һ����������Ϣʱ��������д�˷�����
	//���Ѿ�����������Ϣ���ã�Ӧ�����ظ���Ϣ�Ĵ�С(�ֽ�)������Ӧ������ 0 �Եȴ���������ݵ���
	virtual DWORD IsTcpDataReadable(CSTXIOCPTcpConnectionContext *pTcpConnCtx);

	//��һ�������̳߳�ʼ��ʱ���˷��������á������ڴ˽����߳���صĳ�ʼ�����ã����ʼ�� COM ��
	virtual void OnWorkerThreadInitialize(LPVOID pStoragePtr);

	//��һ�������̼߳�������ʱ���˷��������á�
	virtual void OnWorkerThreadUninitialize(LPVOID pStoragePtr);

	virtual LPVOID OnAllocateWorkerThreadLocalStorage();
	virtual void OnFreeWorkerThreadLocalStorage(LPVOID pStoragePtr);
	virtual void OnWorkerThreadPreOperationProcess(LPVOID pStoragePtr);

	//���ش˷�������̬����ÿ��Client��Timeout. ���� Timeout ֵ(milliseconds)
	virtual DWORD OnGetClientReceiveTimeOut(CSTXIOCPServerClientContext *pClientContext);

	//���ش˷����Է����û��Զ�����쳣��������Ӧ���ı�
	virtual LPCTSTR OnGetUserDefinedExceptionName(DWORD dwExceptionCode);

	virtual DWORD OnParseUserDefinedExceptionArgument(DWORD dwExceptionCode, DWORD nArguments, ULONG_PTR *pArgumentArray, LPTSTR lpszBuffer, UINT cchBufferSize);

protected:
	virtual void InternalTerminate();
	virtual BOOL OnInitialization();
	virtual void OnServerInitialized();
	virtual LRESULT Run();
	virtual CSTXIOCPServerClientContext *OnCreateClientContext(tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext);
	virtual CSTXServerContextBase* OnCreateServerContext();
	virtual CSTXIOCPTcpConnectionContext* OnCreateTcpConnectionContext();
	virtual void OnTimerTimeout(LPVOID lpTimerParam);


	// ��������ʵ�ּ������ϴ�������
	// IsInternalClientData �����ж����Կͻ��˵���Ϣ�����Ƿ�����������Ϣ��������Ϣͷ��С�����ݴ������ϢͷΪ16�ֽڣ�
	// �����������Ϣ�����Ѿ��յ�һ����������Ϣ���ݣ�����Ҫ���ص�ǰ�Ѿ������������������Ϣ���ֽ���
	// IsInternalClientData ���� IsClientDataReadable ֮����õġ�Ҳ����˵���������ȼ������û���Ϣ����
	//   ����û��Ѿ������˸���Ϣ(IsClientDataReadable���ط���)�������Ϣ�����ٴ����� IsInternalClientData 
	// ͬʱ����� IsInternalClientData ���������ݣ������ IsClientDataReadable �����ٵõ��ⲿ������֪ͨ
	virtual DWORD IsInternalClientData(CSTXIOCPServerClientContext *pClientContext);

	// �� IsInternalClientData ���ط���ʱ����˵����Ϣ�Ѿ����ã���Ϣ�����漴���� OnInternalClientReceived
	// OnInternalClientReceived Ĭ��ʵ�ָ������Ϣ�����  OnClientInternalDataTransfer �� OnClientInternalDownload
	// ����������ϴ�����ַ��� OnClientInternalDataTransfer
	// ������������أ���ַ��� OnClientInternalDownload
	virtual BOOL OnInternalClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	
	// OnClientInternalDataTransfer �����ݴ�����ķ���, ����ֽ������ϴ����̵�3���׶�
	virtual BOOL OnClientInternalDataTransfer(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);

	// ͨ��ֻ��Ҫ��д���¼����������ɡ�
	virtual DWORD OnClientInternalDataTransferBegin(CSTXIOCPServerClientContext *pClientContext, DWORD dwUserDataRemote, DWORD dwCookieDesire, LPVOID pPaddingData, DWORD dwPaddingDataLen);
	virtual void OnClientInternalDataTransferDataBlock(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpDataBlock, DWORD dwDataBlockLen);
	virtual void OnClientInternalDataTransferPreComplete(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID pPaddingData, DWORD dwPaddingDataLen);
	virtual void OnClientInternalDataTransferComplete(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID pPaddingData, DWORD dwPaddingDataLen);

	virtual BOOL OnClientInternalDownload(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual DWORD OnClientInternalDownloadBegin(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID pPaddingData, DWORD dwPaddingDataLen);
	virtual DWORD OnClientInternalDownloadDataBlock(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, DWORD dwOffset, __out LPVOID pDataBuffer, DWORD dwDataBufferLen);

	virtual BOOL OnClientInternalPassiveDownload(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual BOOL OnClientInternalPassiveDownloadBegin(CSTXIOCPServerClientContext *pClientContext, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID pPaddingData, DWORD dwPaddingDataLen);

	virtual void OnInternalFileRead(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer, DWORD_PTR dwTag, __int64 dwOffset, DWORD dwCookie, DWORD dwUserDataRemote);
	virtual void OnInternalFileChanged(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName);

	void WaitForServerUnlock();
};

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPServerClientContext

class CSTXIOCPServerClientContext : public CSTXServerClientContextBase
{
public:
	CSTXIOCPServerClientContext();
	CSTXIOCPServerClientContext(const CSTXIOCPServerClientContext &val){}
	virtual ~CSTXIOCPServerClientContext();

	friend class CSTXIOCPServer;

protected:
	SOCKET m_socket;				//Socket
	SOCKET m_socketCopy;			//A copy of Socket. Used to search in the map. Do not use it otherwise
	CSTXIOCPServer *m_pServer;		//Server pointer

	SOCKADDR_IN6 m_addrLocal6;				//Local Address (IPv6)
	SOCKADDR_IN6 m_addrRemote6;				//Remote Address (IPv6)
	TCHAR m_szClientIP[128];				//Client IP in IPv4/IPv6
	SOCKADDR_IN m_addrLocal;				//Local Address (IPv4)
	SOCKADDR_IN m_addrRemote;				//Remote Address (IPv4)
	int m_nAddressFamily;					//Address Family, such as AF_INET or AF_INET6


	char *m_pRecvBuffer;
	LONG m_cbBufferSize;
	LONG m_cbWriteOffset;

	std::recursive_mutex _mtxLock;
	std::recursive_mutex _mtxLockSend;

	BOOL m_bDisconnected;
	DWORD m_dwOperationTimeout;

	//The following members are internal implementation for file uploading
	DWORD m_dwUploadCookieBase;
	map <__int64, CSTXIOCPBuffer*> m_mapUploadBuffers;				//Buffers for receiving uploads
	tr1::shared_ptr<CSTXIOCPTcpServerContext> m_pServerContext;
	CSTXIOCPBuffer *m_pOperationBufferRead = NULL;
	CSTXIOCPBuffer *m_pOperationBufferWrite = NULL;

	volatile LONG64 m_nBufferedSendDataLength;
	moodycamel::ConcurrentQueue<CSTXIOCPBuffer*> _queuedSendData;
	SYSTEMTIME _lastDataReceiveTime;
	BOOL _timeOutDisconnected;

public:
	BOOL m_nDisconnectEventProcessed;

protected:
	virtual BOOL OnInitialize();
	virtual void OnDestroy();
	virtual DWORD OnQueryDownloadSize(DWORD dwCookie, LPVOID pPaddingData, DWORD dwPaddingDataLen, DWORD dwUserDataRemote);
	virtual DWORD OnQueryDownloadData(DWORD dwCookie, DWORD dwOffset, LPVOID lpData, DWORD cbQueryBufferLen, DWORD dwUserDataRemote);


public:
	virtual LONG AddRef();
	virtual LONG Release();

public:
	LPCTSTR GetClientIP() const;
	void AppendRecvData(LPVOID lpData, DWORD cbDataLen);
	void SkipRecvBuffer(LONG cbSkip);
	BOOL IsSameIP(CSTXIOCPServerClientContext *pClientContext);
	SOCKET GetSocket();
	SOCKET GetSocketOriginal();
	void SetAddressFamily(int nAddressFamily);
	int GetAddressFamily();
	BOOL IsDisconnected();
	BOOL MarkDisconnected(BOOL bDisconnected = TRUE);
	LPVOID GetMessageBasePtr();
	DWORD GetBufferedMessageLength();
	void CloseSocket();
	tr1::shared_ptr<CSTXIOCPTcpServerContext> GetServerContext();
	void SetOperationTimeout(DWORD dwTimeout);

	virtual DWORD PrepareUpload(DWORD dwUserDataRemote, DWORD dwCookieDesire);
	virtual DWORD ProcessUpload(DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpData, DWORD cbDataLen);
	virtual DWORD ProcessDownload(DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpData, DWORD dwOffset, DWORD cbQueryDataLen);
	virtual BOOL IsUploadTask(DWORD dwCookie, DWORD dwUserDataRemote);
	virtual CSTXIOCPBuffer* GetUploadTaskBuffer(DWORD dwCookie, DWORD dwUserDataRemote);
	virtual DWORD GetUploadTaskSize(DWORD dwCookie, DWORD dwUserDataRemote);
	virtual BOOL CloseUpload(DWORD dwCookie, DWORD dwUserDataRemote);
	virtual LPCTSTR OnGetClientDisplayName();

	LONG64 DecreaseBufferedSendLength(LONG64 nLength);
	LONG64 IncreaseBufferedSendLength(LONG64 nLength);
	LONG64 GetBufferedSendLength();

	void Lock();
	void Unlock();

	void LockSend();
	void UnlockSend();
	void UpdateLastDataReceiveTime();

};

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPServerContext

class CSTXIOCPServerContext : public CSTXServerContextBase
{
public:
	CSTXIOCPServerContext();
	virtual ~CSTXIOCPServerContext();

protected:
	bool m_bSocketClosed;
	bool m_bSocket6Closed;
	SOCKET m_socket;
	SOCKET m_socket6;
	CSTXIOCPServer *m_pServer;
	LPSTXIOCPCONTEXTKEY m_pContextKey;

public:
	void SetSocket(SOCKET sock, SOCKET sock6);
	SOCKET GetSocket();
	SOCKET GetSocket6();
	void MarkSocketClosed();
	void MarkSocket6Closed();
	void SetServer(CSTXIOCPServer *pServer);
	void SetContextKey(LPSTXIOCPCONTEXTKEY pContextKey);
	BOOL IsSocketClear();
	void CloseSocket();
	void CloseSocket6();
};

//////////////////////////////////////////////////////////////////////////
class CSTXIOCPFolderMonitorContext : public CSTXServerContextBase
{
	friend class CSTXIOCPServer;
public:
	CSTXIOCPFolderMonitorContext();
	virtual ~CSTXIOCPFolderMonitorContext();

protected:
	LONGLONG m_nMonitorID;
	CSTXHashSet<std::wstring, 4, 1, CSTXNoCaseWStringHash<4, 1>> _ignoreFileExtensions;

	CSTXHashMap<std::wstring, FILETIME, 8, 1, CSTXDefaultWStringHash<8, 1>> _fileLastModifyTime;
	CSTXHashMap<std::wstring, DWORD, 8, 1, CSTXDefaultWStringHash<8, 1>> _fileLastChangeType;


public:
	void AddIgnoreFileExtension(LPCTSTR lpszExt);
	void RemoveIgnoreFileExtension(LPCTSTR lpszExt);
	BOOL IsIgnoreFileExtension(LPCTSTR lpszExt);

};

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPTcpServerContext

class CSTXIOCPTcpServerContext : public CSTXIOCPServerContext
{
public:
	CSTXIOCPTcpServerContext();
	virtual ~CSTXIOCPTcpServerContext();

protected:
	volatile LONG64 _currentClientCount;
	LONG64 _maximumClientCount;
	BOOL m_bClosed;
	UINT m_nListeningPort;

public:
	virtual void OnDestroy();

public:
	void SetMaximumClientCount(LONG64 nMaxClientCount);
	void MarkClosed(BOOL bClosed = TRUE);
	BOOL IsClosed();
	LONG64 IncreaseClientCount();
	LONG64 DecreaseClientCount();
	LONG64 GetCurrentClientCount();
	LONG64 GetClientCountLimit();
	BOOL IsReachedMaximumClient();
	void SetListeningPort(UINT nPort);
	UINT GetListeningPort();
};


//////////////////////////////////////////////////////////////////////////
// CSTXIOCPUdpServerContext

class CSTXIOCPUdpServerContext : public CSTXIOCPServerContext
{
public:
	CSTXIOCPUdpServerContext();
	virtual ~CSTXIOCPUdpServerContext();

protected:
	UINT m_nUdpPort;

public:
	LONG Send(LPVOID lpData, DWORD dwDataLen, LPCTSTR lpszTargetHost, UINT nTargetPort);
	void SetUdpPort(UINT nPort);
	UINT GetUdpPort();


};

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPTcpConnectionContext

class CSTXIOCPTcpConnectionContext : public CSTXServerContextBase
{
public:
	CSTXIOCPTcpConnectionContext();
	virtual ~CSTXIOCPTcpConnectionContext();

protected:
	char *m_pRecvBuffer;
	LONG m_cbBufferSize;
	LONG m_cbWriteOffset;

	BOOL m_bConnected;
	DWORD m_dwConnectionFlags;
	LONG m_nConnectionID;
	SOCKET m_socket;
	LPSTXIOCPCONTEXTKEY m_pContextKey;
	CSTXIOCPServer *m_pServer;
	int m_nAddressFamily;
	TCHAR m_szTargetHost[64];				//Target Host Address
	UINT m_uTargetPort;						//Target Port

public:
	void SetSocket(SOCKET sock);
	void SetConnectionID(LONG nID);
	LONG GetConnectionID();
	SOCKET GetSocket();
	void SetContextKey(LPSTXIOCPCONTEXTKEY pContextKey);
	void SetServer(CSTXIOCPServer *pServer);
	LPSTXIOCPCONTEXTKEY GetContextKey();
	BOOL IsConnected();
	int GetAddressFamily();
	void SetAddressFamily(int nAddressFamily);
	void AppendRecvData(LPVOID lpData, DWORD cbDataLen);
	void SkipRecvBuffer(LONG cbSkip);
	LPVOID GetMessageBasePtr();
	DWORD GetBufferedMessageLength();
	DWORD ModifyFlags(DWORD dwAdd, DWORD dwRemove);
	DWORD GetFlags() const;
	LPCTSTR GetTargetHostName();
	UINT GetTargetHostPort();
	void SetTargetHoatName(LPCTSTR lpszHostName);
	void SetTargetHoatPort(UINT uPort);

public:
	void Disconnect(BOOL bForceNoKeep = FALSE);
	void MarkConnected();
	void MarkDisonnected();

protected:

};


//////////////////////////////////////////////////////////////////////////
//

template<typename T>
class CSTXRealtimeDataObject
{
protected:
	T _value;
public:
	virtual void OnChanged();
};

//////////////////////////////////////////////////////////////////////////
//

