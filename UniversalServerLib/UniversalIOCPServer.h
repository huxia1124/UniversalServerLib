#pragma once
#include "STXIOCPServer.h"
#include "SimpleHttpServer.h"
#include <atomic>
#include <concurrent_unordered_map.h>
#include "UniversalServerInternal.h"
#include "UniversalStringCache.h"
#include <functional>

extern "C"
{
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}


#include "LuaIntf/LuaIntf.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////


class CUniversalServer;

////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TcpServerType
{
	TcpServerTypeStream = 1,
	TcpServerTypeHttp = 2,
	TcpServerTypeBinaryHeader2 = 3,
	TcpServerTypeBinaryHeader4 = 4,
	TcpServerTypeBinaryHeaderV = 5
};

enum UniversalTcpClientRole
{
	UniversalTcpClientRole_Default = 0,
	UniversalTcpClientRole_DebugMonitor = 1800,
	UniversalTcpClientRole_LogMonitor = 2000,
	UniversalTcpClientRole_Admin = 8000,
};

enum TcpConnectionType
{
	TcpConnectionTypeStream = 1,
	TcpConnectionTypeBinaryHeader2 = 3,
	TcpConnectionTypeBinaryHeader4 = 4,
	TcpConnectionTypeBinaryHeaderV = 5
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

// CUniversalServerWorkerThreadData
// To store TLS(thread local storage) data for worker threads
struct CUniversalServerWorkerThreadData
{
	LONGLONG _scriptVersions[16];
	lua_State *_pLuaState;
	LPVOID _pUserData;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUniversalIOCPTcpConnectionContext : public CSTXIOCPTcpConnectionContext
{
	friend class CUniversalIOCPServer;
public:
	CUniversalIOCPTcpConnectionContext();
	virtual ~CUniversalIOCPTcpConnectionContext();

public:
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
	TcpServerType _serverType;
	__int64 _uid;
	UniversalTcpClientRole _role = UniversalTcpClientRole_Default;
	concurrency::concurrent_unordered_map<std::wstring, std::wstring> _userDataStringMap;
	BOOL _webSocketProtocolMode;

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
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUniversalIOCPServer : public CSTXIOCPServer
{
	friend class CUniversalIOCPServerClientContext;
public:
	CUniversalIOCPServer();
	virtual ~CUniversalIOCPServer();

protected:
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 256, 8> _mapClients;
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 4, 8> _mapLogMonitorClients;
	CSTXHashMap<__int64, CUniversalIOCPServerClientContext*, 4, 8> _mapDebugMonitorClients;
	volatile LONG _logMonitorCount;
	volatile LONG _debugMonitorCount;

	CSTXHashMap<UINT, lua_State *> _mapLuaState;
	CSTXHashMap<std::wstring, std::set<CUniversalStringCache*>, 4, 1, CSTXNoCaseWStringHash<4, 1> > _mapLuaModuleReference;

public:
	STXSERVERINIT _serverInitializationInfo;
	CUniversalServer *_pServer;
	HANDLE _hRPCThread;
	UINT m_nRPCServerPort;

	CUniversalStringCache _scriptCacheClientConnected;
	CUniversalStringCache _scriptCacheClientReceived;
	CUniversalStringCache _scriptCacheTcpConnectionReceived;

protected:
	std::atomic<__int64> _totalSentBytes;
	std::atomic<__int64> _totalReceivedBytes;
	std::atomic<__int64> _totalConnected;
	std::atomic<__int64> _totalSentCount;
	std::atomic<__int64> _totalReceivedCount;

protected:
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
	virtual void OnWorkerThreadInitialize();
	virtual void OnWorkerThreadUninitialize();
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
	void RunScriptCache(CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread);
	int RunScript(LPCTSTR lpszScriptFile);
	void SetRPCServerPort(UINT nPort);
	void CreateServerRPCThread();
	void StopServerRPC();

protected:
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
	void LuaBindSTXProtocolClasses(lua_State *L);
	lua_State *GetLuaStateForCurrentThread();
	CUniversalServerWorkerThreadData *GetCurrentThreadData();
	DWORD IsClientDataReadableWebSocket(CSTXIOCPServerClientContext *pClientContext);

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

		LuaIntf::CppObjectSharedPtr<std::shared_ptr<T>, T>* pSp = (LuaIntf::CppObjectSharedPtr<std::shared_ptr<T>, T>*)p;
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

public:
	void SetUserData(LPVOID pUserData);
	__int64 GetTotalSentBytes() const;
	__int64 GetTotalReceivedBytes() const;
	void SetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, LPCTSTR lpszValue);
	BOOL GetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, std::wstring& valueOut);

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
	


};