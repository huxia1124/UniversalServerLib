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

#include <process.h>

#include <WinSock2.h>
#include <WTypes.h>
#include <tchar.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#include <process.h>
#include <Windows.h>
#include <time.h>

#include <set>
#include <map>
#include <queue>
using namespace std;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#define WM_STXSOCKET_SERVER_EVENT			(WM_USER + 210)
#define WM_STXSOCKET_CLIENT_EVENT			(WM_USER + 211)
#define WM_STXSOCKET_CONNECTION_EVENT		(WM_USER + 212)
#define WM_STXSOCKET_CREATESERVER_EVENT		(WM_USER + 213)

//////////////////////////////////////////////////////////////////////////
typedef BOOL(CALLBACK*PFNSTXSOCKETISWAITMESSAGE)(DWORD_PTR /*dwUserData*/, LPVOID /*lpRecvData*/, int /*cbRecvData*/);

//////////////////////////////////////////////////////////////////////////
class CSTXSocket;

//////////////////////////////////////////////////////////////////////////

#define STXIOCP_INTERNAL_OP_UPLOAD				1		//上传操作
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

#define STXIOCP_INTERNAL_OPCODE_INITIALIZATION		0		// 初始化
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK			1		// 数据块
#define STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH		2		// 结束通知

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
	DWORD dwOffset;
	DWORD dwContentSize;
}STXIOCPSERVERPASSIVEDOWNLOADFILE, *LPSTXIOCPSERVERPASSIVEDOWNLOADFILE;

//////////////////////////////////////////////////////////////////////////
class CSTXSocketBuffer
{
public:
	CSTXSocketBuffer();
	virtual ~CSTXSocketBuffer();

protected:
	CRITICAL_SECTION m_cs;
	char* m_pBuffer;
	DWORD m_dwBufferLength;		//Full length of buffer
	DWORD m_dwContentLength;	//Length of data
	int m_nLock;

	DWORD m_dwFlags;
	DWORD_PTR m_dwUserData;

public:
	//重新为这个 Buffer 对象分配空间
	BOOL ReallocateBuffer(DWORD dwBufferSize);

	//锁定 Buffer 对象，返回锁定后的总锁定次数
	int LockBuffer();

	//解锁 Buffer 对象，返回解锁后的总锁定次数
	int UnlockBuffer();

	//向 Buffer 中写入数据，返回实际写入的数据大小(单位:字节)
	//此方法向对象中追加数据
	DWORD WriteBuffer(LPVOID pData, DWORD cbDataLen);

	//从 Buffer 中读取数据并保存到 pOutBuffer 所指向的存储空间中，返回实际读取的数据大小(单位:字节)
	DWORD ReadBuffer(LPVOID pOutBuffer, DWORD cbOutBufferLen);

	//获得 Buffer 空间的地址
	LPVOID GetBufferPtr();

	//获得 Buffer 空间的大小(单位:字节)
	DWORD GetBufferLength();

	//获得 Buffer 中实际数据的大小(单位:字节)
	DWORD GetDataLength();

	//设置 Buffer 中实际数据的大小(单位:字节)。
	DWORD SetDataLength(DWORD cbBufferDataLen);

	//释放这个 Buffer 空间，从此这个 Buffer 对象不再可用，直到再次成功调用 ReallocateBuffer 为止
	BOOL ClearBuffer();

	//设置用户自定义数据
	void SetUserData(DWORD_PTR dwUserData);

	//获取用户自定义数据
	DWORD_PTR GetUserData();

	//从另一个对象中复制数据
	DWORD CopyData(CSTXSocketBuffer *pSrcBuffer);
	BOOL WriteToFile( LPCTSTR lpszFile );
};

//////////////////////////////////////////////////////////////////////////
class CSTXSocketObjectBase
{
	friend class CSTXSocket;

	struct _CMessageRecv
	{
		DWORD cbBufferLength;
		LPVOID pBuffer;
		HANDLE hWaitEvent;
		HWND hWndNotify;
		UINT uMsgNotify;
		UINT_PTR nTimerID;
		DWORD_PTR dwUserData;
		struct _CMessageRecv& operator =(const struct _CMessageRecv& val)
		{
			cbBufferLength = val.cbBufferLength;
			pBuffer = val.pBuffer;
			hWaitEvent = val.hWaitEvent;
			hWndNotify = val.hWndNotify;
			uMsgNotify = val.uMsgNotify;
			nTimerID = val.nTimerID;
			dwUserData = val.dwUserData;
			return *this;
		}
	};

public:
	CSTXSocketObjectBase();
	virtual ~CSTXSocketObjectBase();

protected:
	LONG m_nRef;
	SOCKET m_sock;
	DWORD m_dwEventID;
	
	DWORD_PTR m_dwUploadUserData;
	DWORD_PTR m_dwDownloadUserData;
	
	BOOL m_bUploading;
	DWORD m_dwcbUploadTransferred;
	BOOL m_bDownloading;
	DWORD m_dwcbDownloadTransferred;
	BOOL m_bPassiveDownloadBeginCalled;
	CSTXSocketBuffer m_FileDownloadBuffer;

	TCHAR m_szFileUploading[MAX_PATH];
	HANDLE m_hFileUploading;
	HWND m_hWndFileUploadNotify;
	UINT m_uMsgFileUploadNotify;

	char *m_pRecvBuffer;
	LONG m_cbBufferSize;
	LONG m_cbWriteOffset;
	LONG m_cbReadOffset;

	TCHAR m_szPeerIP[256];
	UINT m_nPeerPort;
	SOCKADDR_IN m_addrPeer;
	DWORD_PTR m_dwUserData;
	CSTXSocket *m_pMainSocket;
	PFNSTXSOCKETISWAITMESSAGE m_pfnIsWaitMessage;

	queue <pair<char*, int> > m_quePendingSendData;
	CRITICAL_SECTION m_csWaitMap;
	map <pair<HANDLE, DWORD_PTR>, _CMessageRecv> m_mapWaitObjects;
	map <UINT_PTR, _CMessageRecv> m_mapTimerToWaitObject;
	HANDLE m_hEvtUpload;
	HANDLE m_hEvtDownload;
	DWORD_PTR m_dwTag1;
	DWORD_PTR m_dwTag2;

	HWND m_hWndIsDataReadableNotify;
	UINT m_uMsgIsDataReadableNotify;

protected:
	void AppendRecvData(LPVOID lpData, DWORD cbDataLen);
	LPVOID GetMessageBasePtr();
	DWORD GetBufferedMessageLength();
	void SkipRecvBuffer(LONG cbSkip);
	void OnPreReceived(LPVOID lpDataRecv, DWORD cbRecvLen);
	void OnPreSendReady();
	int SendHelper(LPVOID lpData, int cbDataLen, BOOL bAutoCache, int *pWsaError = NULL);
	void ClearPendingMessages();
	void OnTimeout(UINT_PTR nTimerID);
	static BOOL CALLBACK DefIsWaitMessage(DWORD_PTR dwUserData, LPVOID lpRecvData, int cbRecvData);

protected:
	virtual DWORD IsDataReadable();
	virtual DWORD IsInternalDataReadable();
	virtual void OnReceived(LPVOID lpDataRecv, DWORD cbRecvLen);
	virtual void OnInternalReceived(LPVOID lpDataRecv, DWORD cbRecvLen);
	virtual void OnInternalDataTransfer(LPVOID lpDataRecv, DWORD cbRecvLen);
	virtual void OnInternalDownload(LPVOID lpDataRecv, DWORD cbRecvLen);
	virtual void OnInternalPassiveDownload(LPVOID lpDataRecv, DWORD cbRecvLen);
	virtual void OnSendReady();

	virtual LONG OnQueryUploadData(LPVOID lpDataBuf, DWORD cbBufferLen, DWORD cbTransferred, DWORD_PTR dwUserData);
	virtual LONG OnQueryUploadFinalPaddingData(LPVOID lpDataBuf, DWORD cbBufferLen, DWORD_PTR dwUserData);
	virtual void OnUploadComplete(DWORD_PTR dwUserData);
	virtual void OnDownloadBlock(DWORD dwDownloadTransferred, LPVOID lpDataDownload, DWORD cbDownloadLen);
	virtual void OnDownloadComplete(DWORD_PTR dwUserData);
	virtual void OnPassiveDownloadBegin(DWORD dwCookie, DWORD dwUserData, WORD wServerResult);
	virtual void OnPassiveDownloadBlock(DWORD dwCookie, DWORD dwUserData, DWORD dwOffset, LPVOID lpDataDownload, DWORD cbDownloadLen);
	virtual void OnPassiveDownloadComplete(DWORD dwCookie, DWORD dwUserData, WORD wServerResult);

public:
	virtual LONG AddRef();
	virtual LONG Release();

public:
	int Send(LPVOID lpData, int cbDataLen, BOOL bAutoCache = FALSE);
	int SendRecv(LPVOID lpSendData, int cbSendLen, LPVOID lpRecvBuf, int cbRecvBufLen, DWORD_PTR dwUserData, DWORD dwTimeOut);
	BOOL BeginSendRecv(LPVOID lpSendData, int cbSendLen, int cbRecvBufLenReserve, DWORD_PTR dwUserData, DWORD dwTimeOut, HWND hWndNotify, UINT uMsgNotify);
	int Receive(LPVOID lpData, int cbDataLen);
	BOOL BeginUpload(DWORD_PTR dwUserData, DWORD dwUserDataRemote, DWORD dwCookieDesire, LPVOID lpPaddingData, DWORD cbPaddingDataLen);
	BOOL BeginUploadFile(LPCTSTR lpszFile, DWORD_PTR dwUserData, DWORD dwUserDataRemote, DWORD dwCookieDesire, LPVOID lpPaddingData, DWORD cbPaddingDataLen, HWND hWndNotify = NULL, UINT uMsgNotify = 0);
	BOOL BeginDownload(DWORD_PTR dwUserData, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpPaddingData, DWORD cbPaddingDataLen);
	BOOL BeginPassiveDownload(DWORD_PTR dwUserData, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpPaddingData, DWORD cbPaddingDataLen);
	CSTXSocketBuffer *GetFileDownloadBuffer();
	BOOL WaitForUploadComplete(DWORD dwMilliseconds);
	BOOL WaitForDownloadComplete(DWORD dwMilliseconds);
	void SetTag1(DWORD_PTR dwTag);
	DWORD_PTR GetTag1();
	void SetTag2(DWORD_PTR dwTag);
	DWORD_PTR GetTag2();

	LPCTSTR GetPeerIP();
	UINT GetPeerPort();
	void Close();
	DWORD_PTR GetUserData();
	void ClearReceiveBuffer();

	void SetIsDataReadableNotify(HWND hWndNotify, UINT uMsgNotify);
};

//////////////////////////////////////////////////////////////////////////
class CSTXSocketClientContext : public CSTXSocketObjectBase
{
	friend class CSTXSocket;
public:
	CSTXSocketClientContext();
	virtual ~CSTXSocketClientContext();

protected:

public:

};

//////////////////////////////////////////////////////////////////////////
class CSTXSocketConnectionContext : public CSTXSocketObjectBase
{
	friend class CSTXSocket;
public:
	CSTXSocketConnectionContext();
	virtual ~CSTXSocketConnectionContext();

protected:
	HANDLE m_hConnectSignal;
	BOOL m_bConnected;
	HWND m_hWndConnectNotify;
	UINT m_uMsgConnectNotify;
	UINT m_uMsgDisconnectNotify;

public:
	BOOL IsConnected();

};


//////////////////////////////////////////////////////////////////////////

class CSTXSocket
{
public:
	CSTXSocket(void);
	virtual ~CSTXSocket(void);

protected:
	SOCKET m_sockServer;		//Socket running as server
	HANDLE m_hThread;
	UINT m_uThreadID;
	HANDLE m_hCreateSignal;
	UINT m_uServerPort;
	HWND m_hWndSocket;
	BOOL m_bCreateSucceed;
	CRITICAL_SECTION m_csClientMap;
	CRITICAL_SECTION m_csConnectionMap;
	HANDLE m_hCreateServerSignal;
	BOOL m_bEnableInternalMessages;

	map<SOCKET, CSTXSocketClientContext*> m_mapClients;
	map<SOCKET, CSTXSocketConnectionContext*> m_mapConnections;

	CRITICAL_SECTION m_csTimerMap;
	map<UINT_PTR, CSTXSocketObjectBase*> m_mapTimers;
	UINT_PTR m_nInternalTimerIDBase;

protected:

protected:
	static LPCTSTR s_lpszClassName;
	static UINT WINAPI SocketThreadProc(LPVOID pData);
	static LRESULT CALLBACK SocketWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	BOOL CreateServerSocket();
	BOOL BindServerSocket(UINT uDesiredListeningPort, UINT nTryRandom);
	BOOL CreateSocketWindow();
	BOOL RegisterServerSocketEvents();
	void SockaddrFromHost(LPCTSTR lpszHostAddress, UINT uPort, SOCKADDR *pAddr, int nAddressFamily = AF_INET);
	void CloseAllClients();
	void CloseAllConnections();
	BOOL ProcessNewClient(SOCKET sock);
	BOOL ProcessNewConnection(SOCKET sock, int nError);
	BOOL InitBaseObjectInformation(CSTXSocketObjectBase *pBaseObject);

protected:
	virtual CSTXSocketClientContext* OnCreateClientContext(SOCKET sock);
	virtual CSTXSocketConnectionContext* OnCreateConnectionContext(DWORD_PTR dwUserData);
	virtual void OnClientDisconnect(CSTXSocketClientContext *pClientContext);
	virtual void OnConnectionClose(CSTXSocketConnectionContext *pConnectionContext);
	virtual void OnClientInit(CSTXSocketClientContext *pClientContext);
	virtual void OnConnectionInit(CSTXSocketConnectionContext *pConnectionContext);
	virtual void OnSocketThreadInitialize();
	virtual void OnSocketThreadUninitialize();
	virtual LRESULT OnUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	BOOL CreateServer(UINT uDesiredListeningPort, UINT nTryRandom = 0);
	UINT GetServerListeningPort();
	BOOL Create(UINT uServerPort = 0);
	CSTXSocketConnectionContext* Connect(LPCTSTR lpszAddress, UINT uPort, DWORD dwWaitTime = 8000, DWORD_PTR dwUserData = 0);
	CSTXSocketConnectionContext* ConnectEx(LPCTSTR lpszAddress, DWORD dwTimeout = 8000, DWORD_PTR dwUserData = 0);

	// BeginConnect BeginConnectEx 开始异步连接
	// 无论连接是否成功，BeginConnect* 都会立即返回
	// 在 hWndNotify 窗口会收到窗口消息 uMsgNotify 作为连接是否成功的通知
	void BeginConnect(LPCTSTR lpszAddress, UINT uPort, HWND hWndNotify, UINT uMsgNotify, UINT uMsgDisconnectNotify, DWORD_PTR dwUserData = 0);
	void BeginConnectEx(LPCTSTR lpszAddress, HWND hWndNotify, UINT uMsgNotify, UINT uMsgDisconnectNotify, DWORD_PTR dwUserData = 0);


	void Close();
	void EnableInternalMessages(BOOL bEnable = TRUE);
	BOOL IsInternalMessagesEnabled();

	void PostClose(CSTXSocketObjectBase *pBaseObject);
	void PostOnSendReady(CSTXSocketObjectBase *pBaseObject);

	BOOL PostUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT SendUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND GetInternalHWND();
	UINT_PTR SetInternalTimer( DWORD dwInterval, CSTXSocketObjectBase* pNotifyObject );
	void KillInternalTimer( UINT_PTR nTimerID );
};
