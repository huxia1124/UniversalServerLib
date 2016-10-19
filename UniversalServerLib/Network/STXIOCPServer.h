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

#define STXIOCPSERVER_BUFFER_COUNT				10					// Initial Buffer Count - 读取缓冲区的初始个数。
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

#define STXIOCP_CUSTOM_OPERATION_QUEUED				0		//操作正在队列中等待完成
#define STXIOCP_CUSTOM_OPERATION_COMPLETE			1		//操作成功完成
#define STXIOCP_CUSTOM_OPERATION_FAILED				2		//操作失败
#define STXIOCP_CUSTOM_OPERATION_CANCELED			3		//操作已经被取消
#define STXIOCP_CUSTOM_OPERATION_TIMEOUT			5		//操作超时

#define STXIOCP_SOCKET_IPV4							4
#define STXIOCP_SOCKET_IPV6							6

// STXIOCP_ACCEPT_ 宏用在 IssueAccept() 方法上，指定 IssueAccept() 在哪个协议上投递 Accept 操作
#define STXIOCP_ACCEPT_IPV4						0x0001
#define STXIOCP_ACCEPT_IPV6						0x0002
#define STXIOCP_ACCEPT_ALL						(STXIOCP_ACCEPT_IPV4 | STXIOCP_ACCEPT_IPV6)

//////////////////////////////////////////////////////////////////////////
//

#define STXIOCP_INTERNAL_OP_UPLOAD					1		//上传操作
#define STXIOCP_INTERNAL_OP_DOWNLOAD				2		//下载操作
#define STXIOCP_INTERNAL_OP_DOWNLOAD_PASSIVE		3		//被动下载操作

// 内部实现上传或下载数据的消息头
typedef struct tagSTXIOCPSERVERMESSAGEHEADER
{
	WORD wHeaderSize;		// 消息头的大小(字节)。必须把这个值设为 sizeof(STXIOCPSERVERMESSAGEHEADER)
	DWORD dwMagic;			// Magic Number
	WORD wOpType;			// 使用 STXIOCP_INTERNAL_OP_ 宏
	DWORD dwContentSize;	// dwContentSize 用于指示在此消息头之后尾随数据的大小。
}STXIOCPSERVERMESSAGEHEADER, *LPSTXIOCPSERVERMESSAGEHEADER;

#define STXIOCP_INTERNAL_OPCODE_ACK					0x8000

#define STXIOCP_INTERNAL_OPCODE_INITIALIZATION			0		// 初始化
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK			1		// 数据块
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH			2		// 结束通知

// 内部实现上传数据的消息头，以 STXIOCPSERVERMESSAGEHEADER 结构开始
typedef struct tagSTXIOCPSERVERUPLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//使用 STXIOCP_INTERNAL_OPCODE_*** 宏

	// dwCookie 用于在传输过程中保持一个针对本次传输的独有编号。这个编号是在服务器生成的。
	// 当 wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION 时，客户端在反馈消息中会得到这个系统产生的编号。
	// 当 wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK 或 wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH 时
	//    通讯双方都应该使用这个编号进行标识
	DWORD dwCookie;

	// dwUserData 是客户端可以随意指定的一个用户自定义值。
	// 当 wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION 时，客户端需要指定这个值，并在一次传输过程中不再改变这个值
	// 在后续的数据传输中，这个值会一直保持在此消息头中。客户端应当保证发送的每一个数据包都指定了这个值
	DWORD dwUserData;

	// dwContentSize 用于指示在此消息头之后尾随数据的大小(字节)。
	DWORD dwContentSize;

	// dwTransferSize 用在反馈消息头中，用于指示实际处理了多少字节的数据
	// dwTransferSize 是与 dwContentSize 对应的。通常它与 dwContentSize 有相同的值
	//      ，除非服务器未能成功处理 dwContentSize 字节的数据
	DWORD dwTransferSize;
}STXIOCPSERVERUPLOADFILE, *LPSTXIOCPSERVERUPLOADFILE;

//#define STXIOCP_INTERNAL_OPCODE_INITIALIZATION		0	// 初始化。与数据上传共用同一个宏
#define STXIOCP_INTERNAL_OPCODE_DOWNLOAD_BLOCK		1	// 数据块
#define STXIOCP_INTERNAL_OPCODE_DOWNLOAD_FINISH		2	// 结束通知。对于下载，客户端可不发送 FINISH 通知

// 内部实现下载数据的消息头，以 STXIOCPSERVERMESSAGEHEADER 结构开始
typedef struct tagSTXIOCPSERVERDOWNLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//使用 STXIOCP_INTERNAL_OPCODE_ 宏
	DWORD dwCookie;
	DWORD dwUserData;
	DWORD dwContentSize;

	// 客户端在请求下载数据块时，需要指定 dwOffset 作为读取数据的起始偏移量(字节)。
	DWORD dwOffset;
	DWORD dwTransferSize;
}STXIOCPSERVERDOWNLOADFILE, *LPSTXIOCPSERVERDOWNLOADFILE;

// 内部实现被动下载数据的消息头，以 STXIOCPSERVERMESSAGEHEADER 结构开始
typedef struct tagSTXIOCPSERVERPASSIVEDOWNLOADFILE
{
	STXIOCPSERVERMESSAGEHEADER header;
	WORD wOpCode;			//使用 STXIOCP_INTERNAL_OPCODE_ 宏
	DWORD dwCookie;
	DWORD dwUserData;
	WORD wResult;			//0 表示成功，否则表示失败代码
	__int64 dwOffset;
	DWORD dwContentSize;
}STXIOCPSERVERPASSIVEDOWNLOADFILE, *LPSTXIOCPSERVERPASSIVEDOWNLOADFILE;

//STXIOCPCONTEXTKEY 此结构对应完成端口上的 Context Key
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
	TCHAR szMonitoredFolder[MAX_PATH];		//在用于文件读取时，此项为文件名
	DWORD dwMonitorNotifyFilter;			//在用于文件读取时，此项为特殊标志
	HANDLE hFile;
	DWORD dwCookie;
	DWORD dwUserData;
	DWORD_PTR dwFileTag;
	__int64 nBytesRead;		//已经读取了多少字节。用于指示下次读取时的偏移量

	LONG nAcceptPending;	//对于 TCP Server, 需要跟踪还有多少个Accept仍然没有处理完，才能决定是否能删除此对象

}STXIOCPCONTEXTKEY, *LPSTXIOCPCONTEXTKEY;


// Operation flags, used in STXIOCPOPERATION
#define STXIOCP_OPF_CANCELED					0x00000001
#define STXIOCP_OPF_PROCESSED					0x00000002


//STXIOCPOPERATION 结构用于保存所有投递于完成端口上的操作。使用了 OVERLAPPED 尾随数据
struct tagSTXIOCPOPERATION
{
	OVERLAPPED ov;			// 此操作的 OVERLAPPED 结构
	SOCKET sock;			// Socket
	UINT nOpType;			// 操作类型.见 STXIOCP_OPERATION_ 宏

	
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
	DWORD dwSubmitTime;		// 此操作提交时的系统 Tick 值 
	DWORD dwTimeOut;		// 此操作的时限
	DWORD dwOriginalTimeOut;		// 此操作的时限
	UINT nSocketType;		// 此操作对应的 Socket 类型
	int nIPVer;				// 此操作对应的 Socket 所使用的 IP 协议版本号。 4 或 6
	DWORD dwFlags;			// 此操作的Flags.
	CSTXIOCPBuffer *pBuffer;	// 为此操作分配的缓冲区对象指针.不是所有类型的操作都需要缓冲区。如果操作不需要缓冲区，则 pBuffer 为 NULL

	std::recursive_mutex lock;
	LONG m_nRef;
	tr1::shared_ptr<STXIOCPCONTEXTKEY> pContextKey;		//对ContextKey的引用

	HANDLE hTimerHandle;	//定时器Handle,用于超时检测
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

// HTTP请求的用户自定义数据(Context)
typedef struct tagSTXIOCPSERVERHTTPCONTEXT
{
	CSTXIOCPServer *pServer;
	DWORD dwRequestType;		// 操作类型，见 HTTP_CONTEXT_宏
	HINTERNET hInternet;
	HINTERNET hConnect;
	HINTERNET hRequest;
	TCHAR szUrl[256];
	INTERNET_BUFFERS buff;
	DWORD_PTR dwUserData;		// 用户数据
	DWORD dwHttpResponseCode;
	CSTXIOCPBuffer bufferContent;
	STLSTRING stringParam;		// 用户数据 String
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


	//READPENDING 因缓冲区不足而被排队的读操作。READPENDING 的 4 个成员对应于 IssueRead 的 4 个参数
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
	DWORD m_dwIOWait;												//工作线程的空闲超时时间
	HANDLE m_hIOCP;													//完成端口句柄
	map<UINT, tr1::shared_ptr<CSTXIOCPTcpServerContext> > m_mapTcpServers;	//TCP 子服务器 map	<端口号, 子服务器信息>
	map<UINT, CSTXServerObjectPtr<CSTXIOCPUdpServerContext> > m_mapUdpServers;	//UDP 子服务器 map	<端口号, 子服务器信息>
	map<LONG, CSTXServerObjectPtr<CSTXIOCPTcpConnectionContext> > m_mapConnections;		//对外 TCP 连接 map	<连接ID, 连接信息>
	queue<READPENDING> m_queueRead;									//因缓冲区不足而被排队的读操作队列

	vector<HANDLE> m_arrWorkerThreads;								//工作线程句柄数组
	CSTXHashSet<UINT, 8> m_setWorkerThreadId;						
	CSTXIOCPBufferCollection m_Buffers;								//缓冲区集合对象,管理缓冲区的分配

	CSTXHashMap<std::wstring, CSTXServerObjectPtr<CSTXIOCPServerClientContext>, 128, 4, CSTXDefaultWStringHash<128, 4> > m_mapClientContext;	//TCP 客户端 Context map <SOCKET, 客户端 Context 指针>
	CSTXHashSet<LPSTXIOCPOPERATION, 128, sizeof(void*)> m_setOperation;							//所有投递到完成端口上的操作的集合
	CSTXHashMap<DWORD_PTR, LPSTXIOCPOPERATION, 64, 1> m_mapCustomOperation;				//所有用户自定义操作
	CSTXHashMap<DWORD_PTR, LPSTXIOCPOPERATION, 64, 1> m_mapInternalCustomOperation;				//所有内部自定义操作

	CRITICAL_SECTION m_csServersMap;								// m_mapTcpServers 和 m_mapUdpServers
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

	// 设置缓冲区信息
	BOOL SetBufferConfig(LONG nBufferSize = STXIOCPSERVER_BUFFER_SIZE, LONG nInitBufferCount = STXIOCPSERVER_BUFFER_COUNT, LONG nMaxBufferCount = STXIOCPSERVER_BUFFER_MAXCOUNT);

	// 启动整个服务器模块。此方法在整个服务器模块停止工作时才返回。
	// 在此之前必须先调用 Initialize 方法来初始化一些参数。
	virtual void StartServer();

	// 停止整个服务器模块
	virtual BOOL Terminate();

	// 启动一个 TCP 子服务器，在端口 uPort 上监听。dwServerParam 为用户自定义的子服务器参数
	virtual BOOL BeginTcpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString, UINT nPostAcceptCount = 0, LONG64 nLimitClientCount = 0);

	// 启动一个 UDP 子服务器，绑定在端口 uPort 上。dwServerParam 为用户自定义的子服务器参数
	virtual CSTXServerContextBase* BeginUdpServer(UINT uPort, DWORD_PTR dwServerParam, LPCTSTR lpszServerParamString);

	// 停止在端口 uPort 上监听的那一个 TCP 子服务器。
	BOOL KillTcpServer(UINT uPort);
	// 停止绑定在端口 uPort 上的那一个 UDP 子服务器。
	BOOL KillUdpServer(UINT uPort);

	// 获得目前正在运行的子服务器的总数，即当前正在运行的 TCP 子服务器个数 + UDP 子服务器的个数
	int GetRunningServerCount();

	// 停止所有的子服务器。此操作不影响服务器模块的运行
	// 此方法仅关闭 socket, 不回收资源
	BOOL KillAllServers();

	// 初始化整个服务器模块。hModuleHandle 为进程句柄，表示这个服务器模块是由 hModuleHandle 进行启动的，主要用于 Log
	BOOL Initialize(HMODULE hModuleHandle, LPSTXSERVERINIT lpInit);

	// 创建一个到其它地址(服务器)的 TCP 连接。返回所创建的 TCP 连接的 ID(非负整数)，在调用 SendTcpData 发数据时使用。返回 -1 表示连接失败
	virtual LONG PendingTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, DWORD_PTR dwConnectionParam, LPCTSTR lpszServerParamString, int nAddressFamily, BOOL bKeepConnect, CSTXIOCPTcpConnectionContext *pKeepTcpConnCtx);
	virtual LONG CreateTcpConnection(LPCTSTR lpszTargetHostAddress, UINT uTargetPort, DWORD_PTR dwConnectionParam, LPCTSTR lpszServerParamString, int nAddressFamily, BOOL bKeepConnect);
	virtual CSTXIOCPTcpConnectionContext *GetTcpConnectionContext(LONG nConnectionID);
	virtual LONG GetTcpConnectionCount();

	// 发送数据给指定的 TCP 客户端
	LONG SendClientData(CSTXIOCPServerClientContext *pClientContext, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData = 0);

	// 通过指定 ID 的 TCP 连接(由 CreateTcpConnection 创建)发送数据。
	BOOL SendTcpData(LONG nConnectionID, LPVOID lpData, DWORD cbDataLen, DWORD_PTR dwUserData = 0);

	// 通过绑定在指定端口上的 UDP 子服务器向指定的目标地址及目标端口发送数据。该 UDP 子服务器必须是一个正在运行中的 UDP 子服务器
	LONG SendUdpData(UINT uUdpSocketPort, LPVOID lpData, DWORD cbDataLen, LPCTSTR lpszTargetHostAddress, UINT uTargetPort, int nAddressFamily = AF_INET);

	// 投递一个自定义操作
	BOOL PostCustomOperation(DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD dwTimeOut);
	BOOL PostCustomOperationUsingSprcifiedOperationType(UINT nOperationType, DWORD_PTR dwOperationID, DWORD_PTR dwUserData, DWORD_PTR dwUserData2, DWORD dwTimeOut);

	// 完成一个自定义操作
	BOOL DoCustomOperation(DWORD_PTR dwOperationID, DWORD dwCompleteType = STXIOCP_CUSTOM_OPERATION_COMPLETE);

	// 断开一个客户端
	void DisconnectClient(CSTXIOCPServerClientContext *pClientContext);
	BOOL DisconnectClientByTimeout(CSTXIOCPServerClientContext *pClientContext);

	// 增加一个监视目录
	LONGLONG MonitorFolder(LPCTSTR lpszFolder, DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL);

	//关闭所有监控目录
	void CloseAllMonitoredFolder();

	// 开始异步读取文件
	BOOL IssueReadFile(LPCTSTR lpszFile, CSTXIOCPServerClientContext *pClientContext, DWORD_PTR dwTag);

	// 向内部文件列表中增加和删除文件引用
	// 用于内置的文件下载功能
	BOOL AddFileReference(DWORD dwCookie, DWORD dwUserData, LPCTSTR lpszFile);
	BOOL RemoveFileReference(DWORD dwCookie, DWORD dwUserData);

	//开始一个异步 URL 下载请求
	BOOL BeginDownloadURL(LPCTSTR lpszServer, WORD wPort, LPCTSTR lpszURL, DWORD_PTR dwUserData = 0, LPCTSTR lpszUserDataString = NULL);

	//设置和取消监控目录对特定文件扩展名的忽略
	BOOL AddFolderMonitorIgnoreFileExtension(LONGLONG nFolderMonitorID, LPCTSTR lpszExt);
	BOOL RemoveFolderMonitorIgnoreFileExtension(LONGLONG nFolderMonitorID, LPCTSTR lpszExt);

	void RaiseCustomException(DWORD dwExceptionCode, DWORD nArguments = 0, ULONG_PTR *pArguments = NULL);

private:
	// 以下方法用于辅助实现服务器，在派生类中慎用。

	BOOL IssueReadFile(LPCTSTR lpszFile, CSTXIOCPServerClientContext *pClientContext, DWORD_PTR dwTag, BOOL bInternalUse, DWORD dwCookie, DWORD dwUserDataRemote);

	// 为指定的 socket 应用 Keep-Alive 属性
	BOOL KeepAlive(SOCKET sock, DWORD dwFirstTime, DWORD dwInterval);

	// 判断工作线程是否可以安全中止
	BOOL CanTerminateWorkThreads();

	// 关闭所有的客户 Socket。此方法仅关闭 socket, 不回收资源
	void KillAllClient();

	//列出指定目录下所有文件（含所有子目录中的文件）
	void EnumFiles(LPCTSTR lpszFilter, const std::function<BOOL(LPCTSTR, DWORD_PTR)>& pfnEnumFunc, DWORD_PTR dwUserData);
	BOOL GetFileLastModifyTime(LPCTSTR lpszFile, LPFILETIME lpFileTime);

	//获取SOCKET扩展函数
	BOOL GetSocketExtensionFunctions(SOCKET sock);

protected:
	// 关闭所有外连接 Socket。此方法仅关闭 socket, 不回收资源
	void KillAllConnections();

	// 以下 2 个方法回收资源。在 StartServer 返回之前调用，不应该在其它地方调用。
	void ClearAllClient();
	void ClearAllServers();
	void ClearAllFolderMonitor();

	// 创建工作线程。工作线程数量 = CPU 数量 * 2 + 2
	int CreateWorkerThreads();

	// 以下是各个集合对象的临界段锁操作

	void LockServersMap();
	void UnlockServersMap();

	void LockConnectionMap();
	void UnlockConnectionMap();


	// 以下是事件预处理函数. 预处理函数只是对实际处理过程的封闭并加上异常捕获，以保证代码更稳定
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

	// 以下是工作线程中针对各种类型的 Socket 进行的处理.只能使用在工作线程中.
	void WTProcessTcpServer(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessUdpServer(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessClientSocket(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessTcpConnection(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessDirMonitor(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessFileRead(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);
	void WTProcessHttpOperation(BOOL bResult, DWORD dwLastError, DWORD dwNumReadWrite, LPSTXIOCPCONTEXTKEY lpContextKey, LPSTXIOCPOPERATION lpOperation);

	// 以下是其它实现函数。
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
	// 投递一个HTTP操作
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

	//判断一个客户端是否已经有足够的消息可读。如果有，应当返回一条消息的大小，否则返回 0
	virtual DWORD IsClientDataReadable(CSTXIOCPServerClientContext *pClientContext);

	//对于特定通讯协议，在需要判断是否已经收到一条完整的消息时，必须重写此方法。
	//若已经有完整的消息可用，应当返回该消息的大小(字节)，否则应当返回 0 以等待更多的数据到来
	virtual DWORD IsTcpDataReadable(CSTXIOCPTcpConnectionContext *pTcpConnCtx);

	//当一个工作线程初始化时，此方法被调用。可以在此进行线程相关的初始化设置，如初始化 COM 等
	virtual void OnWorkerThreadInitialize(LPVOID pStoragePtr);

	//当一个工作线程即将结束时，此方法被调用。
	virtual void OnWorkerThreadUninitialize(LPVOID pStoragePtr);

	virtual LPVOID OnAllocateWorkerThreadLocalStorage();
	virtual void OnFreeWorkerThreadLocalStorage(LPVOID pStoragePtr);
	virtual void OnWorkerThreadPreOperationProcess(LPVOID pStoragePtr);

	//重载此方法来动态设置每个Client的Timeout. 返回 Timeout 值(milliseconds)
	virtual DWORD OnGetClientReceiveTimeOut(CSTXIOCPServerClientContext *pClientContext);

	//重载此方法以返回用户自定义的异常代码所对应的文本
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


	// 以下用于实现简单数据上传及下载
	// IsInternalClientData 用于判断来自客户端的消息数据是否属于特殊消息（根据消息头大小。数据传输的消息头为16字节）
	// 如果是特殊消息并且已经收到一条完整的消息数据，则需要返回当前已经接收完的这条完整消息的字节数
	// IsInternalClientData 是在 IsClientDataReadable 之后调用的。也就是说，它的优先级低于用户消息数据
	//   如果用户已经处理了该消息(IsClientDataReadable返回非零)，则该消息不会再传达至 IsInternalClientData 
	// 同时，如果 IsInternalClientData 处理了数据，则后续 IsClientDataReadable 不会再得到这部分数据通知
	virtual DWORD IsInternalClientData(CSTXIOCPServerClientContext *pClientContext);

	// 当 IsInternalClientData 返回非零时，即说明消息已经可用，消息数据随即到达 OnInternalClientReceived
	// OnInternalClientReceived 默认实现负责把消息分配给  OnClientInternalDataTransfer 或 OnClientInternalDownload
	// 如果是数据上传，则分发给 OnClientInternalDataTransfer
	// 如果是数据下载，则分发给 OnClientInternalDownload
	virtual BOOL OnInternalClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	
	// OnClientInternalDataTransfer 是数据传输核心方法, 负责分解数据上传过程的3个阶段
	virtual BOOL OnClientInternalDataTransfer(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);

	// 通常只需要重写以下几个方法即可。
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

