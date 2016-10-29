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
#include "STXIOCPServer.h"
#include "SimpleHttpServer.h"
#include <atomic>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>
#include <concurrent_queue.h>
#include "UniversalServerInternal.h"
#include "UniversalStringCache.h"
#include <functional>
#include "UniversalSharedDataController.h"

extern "C"
{
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}


#include "LuaIntf/LuaIntf.h"
#include "StatisticsBuffer.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////


class CUniversalServer;

////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TcpServerType
{
	TcpServerTypeStream = 1,				//Stream data. Whenever data is coming from client, it will trigger message handler 
	TcpServerTypeHttp = 2,					//HTTP server
	TcpServerTypeBinaryHeader2 = 3,			//Package starts with 2 bytes header
	TcpServerTypeBinaryHeader4 = 4,			//Package starts with 4 bytes header
	TcpServerTypeBinaryHeaderV = 5			//Package starts with variable length header
};

enum UniversalTcpClientRole
{
	UniversalTcpClientRole_Default = 0,					//Normal client. nothing special
	UniversalTcpClientRole_DebugMonitor = 1800,			//This client will receive debug output
	UniversalTcpClientRole_LogMonitor = 2000,			//This client will receive debug output and log output
	UniversalTcpClientRole_Admin = 8000,				//This client will receive debug output and log output
														//, and can perform some low level operations on the server [not implemented]
};

enum TcpConnectionType
{
	TcpConnectionTypeStream = 1,			//Stream data. Whenever data is coming, it will trigger message handler 
	TcpConnectionTypeBinaryHeader2 = 3,		//Package starts with 2 bytes header. message handler will only be triggered when a single complete package is received
	TcpConnectionTypeBinaryHeader4 = 4,		//Package starts with 4 bytes header. message handler will only be triggered when a single complete package is received
	TcpConnectionTypeBinaryHeaderV = 5		//Package starts with variable length header. message handler will only be triggered when a single complete package is received
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

// CUniversalServerWorkerThreadData
// To store TLS(thread local storage) data for worker threads
struct CUniversalServerWorkerThreadData
{
	//Used to track the version of the lua scripts being using in current thread
	//The size of this array indicates the maximum number of custom scripts
	LONGLONG _scriptVersions[64];

	//Lua environment of current thread
	lua_State *_pLuaState;

	//User-defined data for current thread. This field is currently not used and reserved for the future
	LPVOID _pUserData;

	//The zero-based index of current thread
	//Each worker thread has a unique, zero-based index number.
	UINT _threadIndex;
};

//////////////////////////////////////////////////////////////////////////

class CUniversalIOCPTcpServerContext : public CSTXIOCPTcpServerContext
{
	friend class CUniversalIOCPServer;
protected:

	//Scripts configuration
	//These members are copies from the content of CUniversalIOCPServer::_mapTcpServer***Scripts.
	//just for quick access when script is set, for better performance
	std::shared_ptr<CUniversalStringCache> _tcpServerRecvScript;
	std::shared_ptr<CUniversalStringCache> _tcpServerConnectedScript;
	std::shared_ptr<CUniversalStringCache> _tcpServerClientDisconnectedScript;

};

//////////////////////////////////////////////////////////////////////////

class CUniversalIOCPUdpServerContext : public CSTXIOCPUdpServerContext
{
	friend class CUniversalIOCPServer;
public:
	CUniversalIOCPUdpServerContext(DWORD_PTR dwServeParam) : CSTXIOCPUdpServerContext(dwServeParam)
	{

	}

protected:

	//Scripts configuration
	//These members are copies from the content of CUniversalIOCPServer::_mapTcpServer***Scripts.
	//just for quick access when script is set, for better performance
	std::shared_ptr<CUniversalStringCache> _udpServerReceivedScript;

};
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUniversalIOCPTcpConnectionContext : public CSTXIOCPTcpConnectionContext
{
	friend class CUniversalIOCPServer;
public:
	CUniversalIOCPTcpConnectionContext();
	virtual ~CUniversalIOCPTcpConnectionContext();

protected:
	//Scripts configuration
	//These members are copies from the content of CUniversalIOCPServer::_mapTcpConnection***Scripts.
	//just for quick access when script is set, for better performance
	std::shared_ptr<CUniversalStringCache> _tcpConnectionRecvScript;
	std::shared_ptr<CUniversalStringCache> _tcpConnectionDisconnectedScript;

public:

	//Connection ID
	__int64 _uid;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUniversalIOCPServerClientContext : public CSTXIOCPServerClientContext, public CHTTPRequestParserEventHandler
{
	friend class CUniversalIOCPServer;
public:
	CUniversalIOCPServerClientContext();
	virtual ~CUniversalIOCPServerClientContext();

protected:
	CRITICAL_SECTION m_csHttpParser;
	CHTTPRequestParser m_reqParser;
public:
	ULONGLONG _connectedTick = 0;		//the time this client connected
	TcpServerType _serverType;
	__int64 _uid;
	UniversalTcpClientRole _role = UniversalTcpClientRole_Default;
	concurrency::concurrent_unordered_map<std::wstring, std::wstring> _userDataStringMap;
	BOOL _webSocketProtocolMode;

	//Used to cache a SharedData node for quick access. Performance consideration.
	std::shared_ptr<CUniversalSharedDataTree> _clientSharedDataRootNode;

	//Statistics
	int64_t _totalReceivedPackageCount = 0;
	int64_t _totalReceivedBytes = 0;

protected:
	//for HTTP
	int _iStage;
	std::vector<char> _arrContent;

	//For websocket
	char *m_pWSRecvBuffer;
	LONG m_cbWSBufferSize;
	LONG m_cbWSWriteOffset;
	BOOL m_bLastWebSocketPacketFin;

protected:
	virtual void OnHttpRequestInternal(const char *pszHeader, void* pUserData);
	virtual void OnHttpRequestError(const char *pszReason, void* pUserData);

	BOOL AppendChar(char ch, void* pUserData, char *pCharLocation);
	void OnHttpMessageEnd(void* pUserData, const char *pHeaderEnd);
	void BeginWebSocketProtocolMode();

	//For websocket
	void AppendWebSocketRecvData(LPVOID lpData, DWORD cbDataLen);
	void SkipWebSocketRecvBuffer(LONG cbSkip);
	DWORD GetBufferedWebSocketMessageLength();
	void SetLastWebSocketPackageFin(BOOL bFin);
	LPVOID GetWebSocketMessageBasePtr();

public:
	UniversalTcpClientRole SetRole(UniversalTcpClientRole newRole);
	UniversalTcpClientRole GetRole();
	ULONGLONG GetOnlineTime();
	void AddTotalReceivedPackageCount(int32_t delta);
	void AddTotalReceivedBytes(int64_t delta);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUniversalIOCPServer : public CSTXIOCPServer
{
	friend class CUniversalIOCPServerClientContext;
public:
	CUniversalIOCPServer();
	virtual ~CUniversalIOCPServer();

protected:
	static std::atomic<__int64> _s_nClientUIDBase;
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 256, 8> _mapClients;
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 4, 8> _mapLogMonitorClients;
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 4, 8> _mapDebugMonitorClients;
	volatile LONG _logMonitorCount;
	volatile LONG _debugMonitorCount;
	LONGLONG _defaultFolderMonitorID = -1;			//Folder monitoring id
	lua_State *_pLuaStateForTimer = nullptr;
	LONGLONG _timerScriptVersion = 0;

	CSTXHashMap<UINT, lua_State *> _mapLuaState;
	CSTXHashMap<std::wstring, std::set<CUniversalStringCache*>, 4, 1, CSTXNoCaseWStringHash<4, 1> > _mapLuaModuleReference;

	//Scripts configuration
	//Runs whenever new action was posted for worker threads
	Concurrency::concurrent_vector<Concurrency::concurrent_queue<std::shared_ptr<std::wstring>>> _workerThreadActionScripts;

	//A copy of TLS data of worker threads
	Concurrency::concurrent_vector<std::shared_ptr<CUniversalServerWorkerThreadData>> _workerThreadData;

	CSTXHashMap<std::wstring, std::set<CUniversalStringCache*>, 4, 1, CSTXNoCaseWStringHash<4, 1> > _mapScriptFileCaches;	//ScriptFileName -> cache set
	CSTXHashMap<UINT, std::shared_ptr<CUniversalStringCache>> _mapTcpServerRecvScripts;					//Port -> ScriptCache
	CSTXHashMap<UINT, std::shared_ptr<CUniversalStringCache>> _mapTcpServerConnectedScripts;			//Port -> ScriptCache
	CSTXHashMap<UINT, std::shared_ptr<CUniversalStringCache>> _mapTcpServerClientDisconnectedScripts;		//Port -> ScriptCache
	CSTXHashMap<LONG, std::shared_ptr<CUniversalStringCache>> _mapTcpConnectionRecvScripts;					//ConnectionID -> ScriptCache
	CSTXHashMap<LONG, std::shared_ptr<CUniversalStringCache>> _mapTcpConnectionDisconnectedScripts;			//ConnectionID -> ScriptCache
	CSTXHashMap<UINT, std::shared_ptr<CUniversalStringCache>> _mapUdpServerReceivedScripts;				//Port -> ScriptCache
	std::shared_ptr<CUniversalStringCache> _timerScript;				//executed in OnTimer
	std::shared_ptr<CUniversalStringCache> _fileChangedScript;				//executed when monitored files changed
	std::string _workerThreadInitializationScript;

public:
	STXSERVERINIT _serverInitializationInfo;
	CUniversalServer *_pServer;
	HANDLE _hRPCThread;
	UINT m_nRPCServerPort;
	std::atomic<UINT> m_nThreadIndexBase = 0;		//used to generate zero-based index identifier for worker threads

protected:
	//Statistics members
	UINT _statisticsEnabled = 1;					//Main switch for statistics data gathering
	std::atomic<long long> _totalSentBytes = 0;
	std::atomic<long long> _totalReceivedBytes = 0;
	std::atomic<long long> _totalConnected = 0;
	std::atomic<long long> _totalSentCount = 0;
	std::atomic<long long> _totalReceivedCount = 0;

	CStatisticsBuffer<long long, 20> _statisticsSentBytes;
	CStatisticsBuffer<long long, 20> _statisticsSentCount;
	CStatisticsBuffer<long long, 20> _statisticsReceivedBytes;
	CStatisticsBuffer<long long, 20> _statisticsReceiveCount;

protected:
	virtual CSTXServerContextBase* OnCreateServerContext();
	virtual CSTXUdpServerContextBase* OnCreateUdpServerContext(DWORD_PTR dwServerParam);
	virtual CSTXIOCPServerClientContext *OnCreateClientContext(tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext);
	virtual CSTXIOCPTcpConnectionContext*OnCreateTcpConnectionContext();
	virtual BOOL OnAccepted(CSTXIOCPServerClientContext *pClientContext);
	virtual DWORD IsClientDataReadable(CSTXIOCPServerClientContext *pClientContext);
	virtual BOOL OnClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual BOOL OnWebSocketClientReceived(CUniversalIOCPServerClientContext *pClientContext, BYTE *pDataBuffer, DWORD cbDataLen);		//this method is specific to CUniversalIOCPServer
	virtual BOOL OnWebSocketClientPing(CUniversalIOCPServerClientContext *pClientContext);		//this method is specific to CUniversalIOCPServer
	virtual BOOL OnWebSocketClientPong(CUniversalIOCPServerClientContext *pClientContext);		//this method is specific to CUniversalIOCPServer
	virtual void OnClientSent(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);
	virtual void OnClientDisconnect(CSTXIOCPServerClientContext *pClientContext);
	virtual void OnWorkerThreadInitialize(LPVOID pStoragePtr);
	virtual void OnWorkerThreadUninitialize(LPVOID pStoragePtr);
	virtual void OnWorkerThreadPreOperationProcess(LPVOID pStoragePtr);
	virtual void OnUdpServerReceived(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer, SOCKADDR *pFromAddr, INT nAddrLen);
	virtual DWORD IsTcpDataReadable(CSTXIOCPTcpConnectionContext *pTcpConnCtx);
	virtual void OnTcpReceived(CSTXIOCPTcpConnectionContext *pTcpConnCtx, CSTXIOCPBuffer *pBuffer);
	virtual void OnTcpConnect(CSTXIOCPTcpConnectionContext *pTcpConnectionContext);
	virtual void OnTcpDisconnected(CSTXIOCPTcpConnectionContext *pTcpConnCtx, DWORD dwError);
	virtual void OnUrlDownloadComplete(LPSTXIOCPSERVERHTTPCONTEXT pContext);
	virtual void OnShutDown();
	virtual void OnServerInitialized();
	virtual void OnFileChangedFiltered(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName);
	virtual LPVOID OnAllocateWorkerThreadLocalStorage();
	virtual void OnFreeWorkerThreadLocalStorage(LPVOID pStoragePtr);
	virtual DWORD OnQueryWorkerThreadCount();
	virtual void OnTimer(DWORD dwInterval);
	virtual void OnTimerInitialize();
	virtual void OnTimerUninitialize();
	virtual void OnTcpSubServerInitialized(CSTXIOCPTcpServerContext *pServerContext);
	virtual void OnTcpSubServerDestroyed(CSTXIOCPTcpServerContext *pServerContext);
	virtual void OnUdpSubServerInitialized(CSTXIOCPUdpServerContext *pServerContext);
	virtual void OnUdpSubServerDestroyed(CSTXIOCPUdpServerContext *pServerContext);

	virtual LPCTSTR OnGetUserDefinedExceptionName(DWORD dwExceptionCode);
	virtual DWORD OnParseUserDefinedExceptionArgument(DWORD dwExceptionCode, DWORD nArguments, ULONG_PTR *pArgumentArray, LPTSTR lpszBuffer, UINT cchBufferSize);

	virtual void OnOutputDebugString(LPCTSTR lpszContent, BOOL bFinalCall);
	virtual void OnOutputLog(LPCTSTR lpszContent, BOOL bFinalCall);


protected:
	//Derived class should override this method to handle HTTP request
	virtual void OnHttpRequest(CSTXIOCPServerClientContext *pClientContext, std::map<std::string, std::string> &mapRequest, bool bPost);

protected:
	void OnHttpRequestInternal(const char *pszHeader, void* pUserData);
	void OnHttpRequestError(const char *pszReason, void* pUserData);

public:
	int RunScriptString(LPCTSTR lpszScript);
	void RunScriptCache(CUniversalStringCache &cache);
	void RunScriptCache(CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread, lua_State *pLuaState = nullptr);
	int RunScript(LPCTSTR lpszScriptFile);
	void SetRPCServerPort(UINT nPort);
	void CreateServerRPCThread(UINT nPort = 0);
	void StopServerRPC();

protected:
	void InitializeServerDataForShareDataTree();
	void InitializeTcpClientDataForSharedDataTree(CUniversalIOCPServerClientContext *pClientContext);
	void UninitializeTcpClientdataForSharedDataTree(CUniversalIOCPServerClientContext *pClientContext);
	int LoadScriptCache(lua_State *pLuaState, CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread);
	void UpdateLuaModuleReference(lua_State *pLuaState, CUniversalStringCache &cache);
	void SetLuaCacheDirtyForModuleChange(LPCTSTR lpszScriptModule);
	void Parse(const char *pszHeader, void* pUserData, const char *pHeaderEnd);
	void CheckLuaEnvironment(lua_State *L);
	void RestoreLuaEnvironment(lua_State *L);
	LPCTSTR GetLuaErrorName(int nError);
	void ProcessInternalScriptFileChange(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName);
	void UpdateAndRunScriptCache(CUniversalStringCache &cache, lua_State *pLuaState, int &nResult, LONGLONG *pScriptVersionInThread);
	int CheckLuaStateUpdateForThread(lua_State *pLuaState, CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread);
	lua_State *GetLuaStateForCurrentThread();
	CUniversalServerWorkerThreadData *GetCurrentThreadData();
	DWORD IsClientDataReadableWebSocket(CSTXIOCPServerClientContext *pClientContext);
	std::shared_ptr<CUniversalStringCache> GetTcpServerReceiveScript(CUniversalIOCPTcpServerContext *pServerContext);
	std::shared_ptr<CUniversalStringCache> GetTcpConnectionReceiveScript(CUniversalIOCPTcpConnectionContext *pConnectionContext);
	std::shared_ptr<CUniversalStringCache> GetTcpConnectionDisconnectedScript(CUniversalIOCPTcpConnectionContext *pConnectionContext);
	std::shared_ptr<CUniversalStringCache> GetTcpServerConnectedScript(CUniversalIOCPTcpServerContext *pServerContext);
	std::shared_ptr<CUniversalStringCache> GetTcpServerClientDisconnectedScript(CUniversalIOCPTcpServerContext *pServerContext);
	std::shared_ptr<CUniversalStringCache> GetUdpServerReceivedScript(CUniversalIOCPUdpServerContext *pServerContext);

	template<class T>
	BOOL CheckLuaObject(lua_State *L, const char *name)
	{
		LuaIntf::LuaRef obj(L, name);
		LuaIntf::LuaTypeID type = obj.type();
		if (type != LuaIntf::LuaTypeID::USERDATA && type != LuaIntf::LuaTypeID::LIGHTUSERDATA)
		{
			STXTRACELOGE(_T("[r][i][Lua] '%S' is no longer a user defined object. it must have been assigned incorrectly !"), name);
			return FALSE;
		}
		void *p = obj.toPtr();
		if (p == NULL)
		{
			STXTRACELOGE(_T("[r][i][Lua] '%S' has no pointer value now. it must have been assigned incorrectly !"), name);
			return FALSE;
		}

		return TRUE;
	}


protected:
	void OnMessage(std::map<std::string, std::string> &mapRequest, void* pUserData, bool bPost);
	void OnWebSocketHandShake(const char *pSecKey, const char *pszPath, void* pUserData);
	void OnHttpWebSocketHandShake(CSTXIOCPServerClientContext *pClientContext, const char *pszPath, const char *pSecKey, const char *pSecKeyResponse);

protected:
	static UINT WINAPI UniversalServerRPCThread(LPVOID lpParameter);


public:
	void SendWebSocketResponseData(CSTXIOCPServerClientContext *pClientContext, LPVOID pData, unsigned int nDataLen);
	void SendWebSocketResponseData(__int64 nClientUID, LPVOID pData, unsigned int nDataLen);
	void SendHttpResponseData(CSTXIOCPServerClientContext *pClientContext, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen);
	void SendHttpResponseData(__int64 nClientUID, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen);
	void SendRawResponseData(__int64 nClientUID, LPVOID pData, unsigned int nDataLen);
	void SendRawResponseDataToAdminClients(LPVOID pData, unsigned int nDataLen);
	void SendRawResponseDataToLogMonitors(LPVOID pData, unsigned int nDataLen);
	void SendRawResponseDataToDebugMonitors(LPVOID pData, unsigned int nDataLen);
	void SetTcpClientRole(__int64 nClientUID, UniversalTcpClientRole role);
	void SetTcpClientTimeout(__int64 nClientUID, DWORD dwTimeout);
	UniversalTcpClientRole GetTcpClientRole(__int64 nClientUID);
	void DisconnectTcpClient(__int64 nClientUID);
	void EnqueueWorkerThreadScript(LPCTSTR lpszScriptString);

public:
	static void LuaBindSTXProtocolClasses(lua_State *L);
	void UpdateScriptTrackingInfo(const std::wstring &originalName, LPCTSTR lpszScriptFile, std::shared_ptr<CUniversalStringCache> &scriptCache);
	void SetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, LPCTSTR lpszValue);
	BOOL GetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, std::wstring& valueOut);
	void SetTcpServerReceiveScript(UINT nPort, LPCTSTR lpszScriptFile);
	void SetTcpConnectionReceiveScript(LONG nConnectionID, LPCTSTR lpszScriptFile);
	void SetTcpConnectionDisconnectedScript(LONG nConnectionID, LPCTSTR lpszScriptFile);
	void SetTcpServerClientConnectedScript(UINT nPort, LPCTSTR lpszScriptFile);
	void SetTcpServerClientDisconnectedScript(UINT nPort, LPCTSTR lpszScriptFile);
	void SetTimerScript(LPCTSTR lpszScriptFile);
	void SetWorkerThreadInitializationScript(LPCTSTR lpszScriptFile);
	void ChangeTimerInterval(DWORD interval);
	long GetTcpClientCount(UINT nPort);
	void SetLogLevel(int level);
	void SetDebugOutputLevel(int level);
	long long GetDefaultFolderMonitorId();
	void SetFileChangedScript(LPCTSTR lpszScriptFile);
	size_t GetWorkerThreadScriptCapacity();
	size_t GetWorkerThreadScriptUsage();
	void SetUdpServerReceivedScript(UINT nPort, LPCTSTR lpszScriptFile);

public:
	//Statistics data. can be enabled/disabled through
	void SetStatisticsLevel(UINT level);
	UINT GetStatisticsLevel();
	long long GetSentBytesPerSecond();
	long long GetSentCountPerSecond();
	long long GetReceivedBytesPerSecond();
	long long GetReceivedCountPerSecond();
	long long GetTotalSentBytes() const;
	long long GetTotalReceivedBytes() const;
	long long GetTotalSentCount() const;			//Package count
	long long GetTotalReceivedCount() const;		//Package count


public:
	template<typename T>
	void EnumerateTcpClients(T pfn)
	{
		_mapClients.foreach([&](std::pair<__int64, CUniversalIOCPServerClientContext*> item)
		{
			pfn(item.second->_uid);
		});
	}

protected:
	virtual void OnServerFileChanged(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName);

public:
	static std::wstring GetFileContentText(LPCTSTR lpszFile, BOOL *pbResult = NULL);
	static __int64 GetNextClientUID();
	


};
