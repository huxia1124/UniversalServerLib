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

#include "stdafx.h"
#include "UniversalIOCPServer.h"
#include <Shlwapi.h>
#include "UniversalServer.h"
#include "UniversalServerInternal.h"
#include "STXProtocol.h"
#include "STXProtocolLua.h"
#include "Cryptographic.h"
#include <iterator>
#include <atlexcept.h>
#include <atlconv.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_set.h>

#include "UniversalServerRPC_h.h"
#include <atlcomcli.h>
#pragma comment(lib, "Rpcrt4.lib")

#pragma comment(lib, "Crypt32.lib")

CUniversalIOCPServer *_s_server = NULL;

_declspec(thread) static Concurrency::concurrent_unordered_set<std::wstring> g_loadedLuaModules;
extern "C"
{
	void add_loaded_entry(const char *module_name, void *data)
	{
		USES_CONVERSION;
		Concurrency::concurrent_unordered_set<std::wstring> *pModulesArray = (Concurrency::concurrent_unordered_set<std::wstring>*)data;
		pModulesArray->insert((LPCWSTR)ATL::CA2W(module_name));
	}
	void* get_data_for_add_loaded_entry()
	{
		return &g_loadedLuaModules;
	}
typedef void(*pfn_add_loaded_entry)(const char *module_name, void *data);
extern pfn_add_loaded_entry g_pfnAddLoadedModuleEntry;
typedef void*(*pfn_get_date_for_add_loaded_entry)();
extern pfn_get_date_for_add_loaded_entry g_pfnGetDataForAddLoadedModuleEntry;
}

namespace LuaIntf
{
	LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}

CUniversalIOCPServer::CUniversalIOCPServer()
{
	_logMonitorCount = 0;
	_debugMonitorCount = 0;
	_hRPCThread = NULL;
	g_pfnAddLoadedModuleEntry = add_loaded_entry;
	g_pfnGetDataForAddLoadedModuleEntry = get_data_for_add_loaded_entry;
	memset(&_serverInitializationInfo, 0, sizeof(_serverInitializationInfo));
	_serverInitializationInfo.cbSize = sizeof(_serverInitializationInfo);
	_serverInitializationInfo.dwMask = STXSM_OPERATION_TIMEOUT | STXSM_BUFFER | STXSM_LOG_FLAGS | STXSM_ACCEPT | STXSM_TIMER_INTERVAL;
	_serverInitializationInfo.dwBufferInitialCount = 60000;
	_serverInitializationInfo.dwBufferMaxCount = 80000;
	_serverInitializationInfo.dwBufferSize = 1024;
	_serverInitializationInfo.dwDefaultOperationTimeout = 180000;
	//_serverInitializationInfo.dwLogFlags = STXLF_TIME | STXLF_THREADID | STXLF_ERROR_LEVEL | STXLF_CALLBACK_LOG | STXLF_CALLBACK_DEBUG;
	_serverInitializationInfo.dwLogFlags = STXLF_TIME | STXLF_THREADID | STXLF_ERROR_LEVEL | STXLF_CALLBACK_LOG;
	_serverInitializationInfo.dwAcceptPost = 60000;
	_serverInitializationInfo.dwTimerInterval = STXS_DEFAULT_TIMER_INTERVAL;
	_serverInitializationInfo.dwServerFlags = STXSF_IPV6;
	_serverInitializationInfo.dwLogBufferSize = 65536;
	_serverInitializationInfo.dwFolderMonitorBufferSize = 65536 * 2;

	m_nRPCServerPort = 0;


	// 	init.uTcpServerCount = 3;
	// 	UINT arrPorts[3] = { 8900, 8901, 8902 };
	// 	init.pTcpServerPorts = arrPorts;
	// 	DWORD_PTR arrParams[3] = { TcpServerTypeHttp, TcpServerTypeStream, TcpServerTypeBinaryHeaderV };
	// 	init.pTcpServerParameters = arrParams;

	// 	init.uUdpServerCount = 1;
	// 	UINT arrPortsUdp[1] = { 8911 };
	// 	init.pUdpServerPorts = arrPortsUdp;
	// 	init.pUdpServerParameters = arrParams;


	_s_server = this;
}


CUniversalIOCPServer::~CUniversalIOCPServer()
{
}

void CUniversalIOCPServer::OnWorkerThreadInitialize(LPVOID pStoragePtr)
{
	UINT threadIndex = m_nThreadIndexBase++;
	_workerThreadActionScripts.push_back(Concurrency::concurrent_queue<std::shared_ptr<std::wstring>>());

	CUniversalServerWorkerThreadData *pData = (CUniversalServerWorkerThreadData*)pStoragePtr;
	pData->_threadIndex = threadIndex;

	auto threadDataCopy = std::make_shared<CUniversalServerWorkerThreadData>();
	_workerThreadData[threadIndex] = threadDataCopy;

	memcpy(threadDataCopy.get(), pData, sizeof(CUniversalServerWorkerThreadData));

	lua_State *L;
	L = GetLuaStateForCurrentThread();
	luaopen_base(L);
	luaL_openlibs(L);

	LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetServer", [&] {
		// you can use C++11 lambda expression here too
		std::shared_ptr<CServerController> sp = _pServer->_spController;
		return sp;
	}).addFunction("GetThreadIndex", [=] {
		return threadIndex;
	}).endModule();

	LuaBindClasses(L, _pServer);
	LuaBindSTXProtocolClasses(L);

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

}

void CUniversalIOCPServer::OnWorkerThreadUninitialize(LPVOID pStoragePtr)
{
	lua_State *L;
	L = GetLuaStateForCurrentThread();
	lua_close(L);

	CoUninitialize();
}

void CUniversalIOCPServer::OnWorkerThreadPreOperationProcess(LPVOID pStoragePtr)
{
	CUniversalServerWorkerThreadData *pData = (CUniversalServerWorkerThreadData*)pStoragePtr;
	auto &scriptQueue = _workerThreadActionScripts[pData->_threadIndex];

	size_t queuedScriptCount = 0;
	if ((queuedScriptCount = scriptQueue.unsafe_size()) > 0)
	{
		STXTRACELOGE(_T("[i] %d scripts queued for current thread [idx=%d]. now pop and run the scripts..."), queuedScriptCount, pData->_threadIndex);
		USES_CONVERSION;
		lua_State *L;
		L = GetLuaStateForCurrentThread();
		while (scriptQueue.unsafe_size())
		{
			std::shared_ptr<std::wstring> script;
			if (scriptQueue.try_pop(script))
			{
				std::string buf = (LPCSTR)ATL::CW2A(script->c_str());
				int nResult = luaL_loadstring(L, buf.c_str());
				if (nResult == 0)
				{
					nResult = lua_pcall(L, 0, LUA_MULTRET, 0);
				}

				if (nResult == 0)
				{
					//Success
				}
				else
				{
					const char *pLuaLoadError = lua_tostring(L, -1);
					LPCTSTR pszError = GetLuaErrorName(nResult);
					STXTRACELOGE(_T("[r][i]\tfailed: luaL_loadstring result = %d [%s]"), nResult, script->c_str());
					STXTRACELOGE(_T("[r][i]\t\t%S"), pLuaLoadError);
				}
			}
		}
	}
}

UINT g_maxTcpPackageLength = 1024 * 2048;

inline const char* stristr(const char* s, const char* p, int maxlen)
{
	while (maxlen != 0 && *s != '\0') {
		int i;
		for (i = 0; (s[i] & ~('a' - 'A')) == (p[i] & ~('a' - 'A')) && p[i] != '\0'; i++);
		if (p[i] == '\0') {
			return s;
		}
		s += 1;
		maxlen--;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CUniversalIOCPServer

void CUniversalIOCPServer::OnClientDisconnect(CSTXIOCPServerClientContext *pClientContext)
{
	__super::OnClientDisconnect(pClientContext);

	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);
	
	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpClientDisconnected(pClient->GetServerContext()->GetServerParamString().c_str(), pClient->_uid, &bSkipScript);

	if (pClient->GetRole() == UniversalTcpClientRole_LogMonitor)
	{
		InterlockedDecrement(&_logMonitorCount);
	}
	else if (pClient->GetRole() == UniversalTcpClientRole_DebugMonitor)
	{
		InterlockedDecrement(&_debugMonitorCount);
	}

	_mapClients.erase(pClient->_uid);
	_mapDebugMonitorClients.erase(pClient->_uid);
	_mapLogMonitorClients.erase(pClient->_uid);

	if (!bSkipScript)
	{
		//Run script
		auto serverContext = pClient->GetServerContext();
		auto scriptCache = GetTcpServerClientDisconnectedScript(dynamic_cast<CUniversalIOCPTcpServerContext*>(serverContext.get()));
		if (scriptCache)
		{
			RunScriptCache(*scriptCache.get());
		}
	}
}

CSTXServerContextBase* CUniversalIOCPServer::OnCreateServerContext()
{
	return new CUniversalIOCPTcpServerContext();
}

CSTXIOCPServerClientContext * CUniversalIOCPServer::OnCreateClientContext(tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext)
{
	static __int64 nUIDBase = 0;
	CUniversalIOCPServerClientContext *pNewClient = new CUniversalIOCPServerClientContext();
	if (pNewClient == NULL)
	{
		STXTRACELOGFE(_T("CUniversalIOCPServer::OnCreateClientContext: not enough memory."));
		return NULL;
	}

	pNewClient->_uid = nUIDBase++;
	pNewClient->_serverType = (TcpServerType)pServerContext->GetServerParam();

	_mapClients[pNewClient->_uid] = pNewClient;

	return pNewClient;
}

CSTXIOCPTcpConnectionContext* CUniversalIOCPServer::OnCreateTcpConnectionContext()
{
	CUniversalIOCPTcpConnectionContext *pNewConnection = new CUniversalIOCPTcpConnectionContext();
	if (pNewConnection == NULL)
	{
		STXTRACELOGFE(_T("CUniversalIOCPServer::OnCreateTcpConnectionContext: not enough memory."));
		return NULL;
	}

	return pNewConnection;
}

BOOL CUniversalIOCPServer::OnAccepted(CSTXIOCPServerClientContext *pClientContext)
{
	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);
	if (_statisticsEnabled)
	{
		_totalConnected++;
	}

	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpClientConnected(pClient->GetServerContext()->GetServerParamString().c_str(), pClient->_uid, &bSkipScript);

	if (!bSkipScript)
	{
		//Lua script preparation. The following methods can be used in lua script:
		// utils.GetClientUid()
		// utils.GetServerParam()

		lua_State *L = GetLuaStateForCurrentThread();
		LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetClientUid", [&] {
			return pClient->_uid;
		}).addFunction("GetServerParam", [&] {
			USES_CONVERSION;
			std::string serverParam = (LPCSTR)ATL::CW2A(pClient->GetServerContext()->GetServerParamString().c_str());
			return serverParam;
		}).endModule();

		
		//Run script
		auto serverContext = pClientContext->GetServerContext();
		auto scriptCache = GetTcpServerConnectedScript(dynamic_cast<CUniversalIOCPTcpServerContext*>(serverContext.get()));
		if (scriptCache)
		{
			RunScriptCache(*scriptCache.get());
		}
	}

	return TRUE;
}

static inline uint32_t swap32(uint32_t a)
{
	return _byteswap_ulong(a);
}
/*
static inline uint32_t swap32(uint32_t a)
{
	return (a << 24) | ((a & 0xff00) << 8) | ((a >> 8) & 0xff00) | (a >> 24);
}
*/

static inline uint64_t swap64(uint64_t a)
{
	return _byteswap_uint64(a);
}

/*
static inline uint64_t swap64(uint64_t a)
{
	return ((uint64_t)swap32((unsigned int)(a >> 32))) |
		(((uint64_t)swap32((unsigned int)a)) << 32);
}
*/

DWORD CUniversalIOCPServer::IsClientDataReadableWebSocket(CSTXIOCPServerClientContext *pClientContext)
{
	DWORD_PTR dwMsgDataLen = pClientContext->GetBufferedMessageLength();
	if (dwMsgDataLen < 2)
		return 0;

	BYTE *pData = (BYTE *)pClientContext->GetMessageBasePtr();
	const BYTE &b1 = pData[0];
	const BYTE &b2 = pData[1];

	BOOL bFin = (b1 & 0x80);
	BOOL bMask = (b2 & 0x80);
	BYTE nPayloadLen = (b2 & 0x7F);
	BYTE opCode = (b1 & 0x0F);

	uint64_t nPayloadLenActual = nPayloadLen;

	BYTE *pMaskingKeyPtr = pData + 2;

	DWORD_PTR dwPackageLen = 2;

	if (nPayloadLen <= 125)
	{
		dwPackageLen += nPayloadLen;
	}
	else if (nPayloadLen == 126)
	{
		char szLen[2];
		szLen[0] = pData[3];
		szLen[1] = pData[2];
		WORD lenWord = *((WORD*)szLen);

		dwPackageLen += (2 + lenWord);
		pMaskingKeyPtr += 2;

		nPayloadLenActual = lenWord;
	}
	else if (nPayloadLen == 127)
	{
		uint64_t len = swap64(*((uint64_t*)(pData + 2)));
		dwPackageLen += (8 + len);
		pMaskingKeyPtr += 8;

		nPayloadLenActual = len;
	}

	BYTE *pPayloadDataPtr = pMaskingKeyPtr;
	if (bMask)
	{
		dwPackageLen += 4;
		pPayloadDataPtr += 4;
	}

	if (dwMsgDataLen < dwPackageLen)
		return 0;

	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);
	if (pClient)
	{
		if (opCode == 10)		//Pong
		{
			OnWebSocketClientPong(pClient);
		}
		else if (opCode == 9)		//Ping
		{
			OnWebSocketClientPing(pClient);
		}
		else
		{
			for (auto i = 0; i < nPayloadLenActual; i++)
			{
				pPayloadDataPtr[i] = (byte)(pPayloadDataPtr[i] ^ pMaskingKeyPtr[i % 4]);
			}

			pClient->AppendWebSocketRecvData((LPVOID)pPayloadDataPtr, nPayloadLenActual);
			pClient->SetLastWebSocketPackageFin(bFin);
		}
	}

	return dwPackageLen;

}

std::shared_ptr<CUniversalStringCache> CUniversalIOCPServer::GetTcpServerReceiveScript(CUniversalIOCPTcpServerContext *pServerContext)
{
	if (pServerContext->_tcpServerRecvScript)
		return pServerContext->_tcpServerRecvScript;

	std::shared_ptr<CUniversalStringCache> result;
	auto port = pServerContext->GetListeningPort();
	_mapTcpServerRecvScripts.lock(port);
	auto it = _mapTcpServerRecvScripts.find(port);
	if (it != _mapTcpServerRecvScripts.end(port))
	{
		result = it->second;
	}
	_mapTcpServerRecvScripts.unlock(port);
	
	if (result)
		pServerContext->_tcpServerRecvScript = result;

	return result;
}

std::shared_ptr<CUniversalStringCache> CUniversalIOCPServer::GetTcpConnectionReceiveScript(CUniversalIOCPTcpConnectionContext *pConnectionContext)
{
	if (pConnectionContext->_tcpConnectionRecvScript)
		return pConnectionContext->_tcpConnectionRecvScript;

	std::shared_ptr<CUniversalStringCache> result;
	auto connectionID = pConnectionContext->GetConnectionID();
	_mapTcpConnectionRecvScripts.lock(connectionID);
	auto it = _mapTcpConnectionRecvScripts.find(connectionID);
	if (it != _mapTcpConnectionRecvScripts.end(connectionID))
	{
		result = it->second;
	}
	_mapTcpConnectionRecvScripts.unlock(connectionID);

	if (result)
		pConnectionContext->_tcpConnectionRecvScript = result;

	return result;
}

std::shared_ptr<CUniversalStringCache> CUniversalIOCPServer::GetTcpConnectionDisconnectedScript(CUniversalIOCPTcpConnectionContext *pConnectionContext)
{
	if (pConnectionContext->_tcpConnectionDisconnectedScript)
		return pConnectionContext->_tcpConnectionDisconnectedScript;

	std::shared_ptr<CUniversalStringCache> result;
	auto connectionID = pConnectionContext->GetConnectionID();
	_mapTcpConnectionDisconnectedScripts.lock(connectionID);
	auto it = _mapTcpConnectionDisconnectedScripts.find(connectionID);
	if (it != _mapTcpConnectionDisconnectedScripts.end(connectionID))
	{
		result = it->second;
	}
	_mapTcpConnectionDisconnectedScripts.unlock(connectionID);

	if (result)
		pConnectionContext->_tcpConnectionDisconnectedScript = result;

	return result;
}

std::shared_ptr<CUniversalStringCache> CUniversalIOCPServer::GetTcpServerConnectedScript(CUniversalIOCPTcpServerContext *pServerContext)
{
	if (pServerContext->_tcpServerConnectedScript)
		return pServerContext->_tcpServerConnectedScript;

	std::shared_ptr<CUniversalStringCache> result;
	auto port = pServerContext->GetListeningPort();
	_mapTcpServerConnectedScripts.lock(port);
	auto it = _mapTcpServerConnectedScripts.find(port);
	if (it != _mapTcpServerConnectedScripts.end(port))
	{
		result = it->second;
	}
	_mapTcpServerConnectedScripts.unlock(port);

	if (result)
		pServerContext->_tcpServerConnectedScript = result;

	return result;
}

std::shared_ptr<CUniversalStringCache> CUniversalIOCPServer::GetTcpServerClientDisconnectedScript(CUniversalIOCPTcpServerContext *pServerContext)
{
	if (pServerContext->_tcpServerClientDisconnectedScript)
		return pServerContext->_tcpServerClientDisconnectedScript;

	std::shared_ptr<CUniversalStringCache> result;
	auto port = pServerContext->GetListeningPort();
	_mapTcpServerClientDisconnectedScripts.lock(port);
	auto it = _mapTcpServerClientDisconnectedScripts.find(port);
	if (it != _mapTcpServerClientDisconnectedScripts.end(port))
	{
		result = it->second;
	}
	_mapTcpServerClientDisconnectedScripts.unlock(port);

	if (result)
		pServerContext->_tcpServerClientDisconnectedScript = result;

	return result;
}

DWORD CUniversalIOCPServer::IsClientDataReadable(CSTXIOCPServerClientContext *pClientContext)
{
	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);
	if (pClient->GetBufferedMessageLength() > g_maxTcpPackageLength)
	{
		STXTRACELOGE(_T("TCP Client [%s] message package is too large! Disconnect it!"), pClientContext->GetClientIP());
		DisconnectClient(pClientContext);
		return 0;
	}

	switch (pClient->_serverType)
	{
	case TcpServerTypeStream:
		return pClient->GetBufferedMessageLength();
	case TcpServerTypeHttp:
	{
		DWORD_PTR dwMsgDataLen = pClientContext->GetBufferedMessageLength();
		if (pClient->_webSocketProtocolMode)
		{
			return IsClientDataReadableWebSocket(pClientContext);
		}
		else
		{
			if (dwMsgDataLen < 4)
				return 0;

			const char *pHeader = (const char*)pClient->GetMessageBasePtr();
			const char *pEndCRLF = NULL;
			if (pEndCRLF = stristr(pHeader, "\r\n\r\n", dwMsgDataLen))
			{
				const char* lenptr = stristr(pHeader, "content-length: ", dwMsgDataLen);
				if (lenptr)
				{
					DWORD_PTR dwAppendLen = atoi(lenptr + 15);
					DWORD_PTR dwExpectedSize = pEndCRLF - pHeader + 4 + dwAppendLen;
					if (dwMsgDataLen >= dwExpectedSize)
					{
						return dwExpectedSize;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return pEndCRLF - pHeader + 4;
				}
			}
			else
			{
				return 0;
			}

			return dwMsgDataLen;
		}
	}
	break;
	case TcpServerTypeBinaryHeader2:
	{
		DWORD_PTR dwMsgDataLen = pClientContext->GetBufferedMessageLength();

		if (dwMsgDataLen < sizeof(WORD))
			return 0;

		WORD wSize = *((WORD*)pClientContext->GetMessageBasePtr());
		if (wSize <= dwMsgDataLen)
			return wSize;
	}
	break;
	case TcpServerTypeBinaryHeader4:
	{
		DWORD_PTR dwMsgDataLen = pClientContext->GetBufferedMessageLength();

		if (dwMsgDataLen < sizeof(DWORD))
			return 0;

		DWORD dwSize = *((DWORD*)pClientContext->GetMessageBasePtr());

		if (dwSize > g_maxTcpPackageLength)
		{
			STXTRACELOGE(_T("TCP Client [%s] message package is too large! Disconnect it!"), pClientContext->GetClientIP());
			DisconnectClient(pClientContext);
			return 0;
		}

		if (dwSize <= dwMsgDataLen)
			return dwSize;
	}
	break;
	case TcpServerTypeBinaryHeaderV:
	{
		DWORD_PTR dwMsgDataLen = pClientContext->GetBufferedMessageLength();
		if (dwMsgDataLen == 0)
			return 0;

		BYTE nLengthBytes = 0;
		LONG nLength = CSTXProtocol::DecodeCompactInteger(pClientContext->GetMessageBasePtr(), &nLengthBytes);

		if (nLength < 0)
			return 0;

		if ((UINT)nLength > g_maxTcpPackageLength)
		{
			STXTRACELOGE(_T("TCP Client [%s] message package is too large! Disconnect it!"), pClientContext->GetClientIP());
			DisconnectClient(pClientContext);
			return 0;
		}

		if ((DWORD)(nLength + nLengthBytes) <= dwMsgDataLen)
			return nLengthBytes + nLength;
	}
	break;
	}
	return 0;
}

void CUniversalIOCPServer::OnHttpRequestInternal(const char *pszHeader, void* pUserData)
{
	Parse(pszHeader, pUserData, NULL);
}

void CUniversalIOCPServer::OnHttpRequestError(const char *pszReason, void* pUserData)
{
	if (pszReason && pUserData)
	{
		SendHttpResponseData((CSTXIOCPServerClientContext*)pUserData, 500, "text", (LPVOID)pszReason, strlen(pszReason));
	}
}

void CUniversalIOCPServer::OnMessage(std::map<std::string, std::string> &mapRequest, void* pUserData, bool bPost)
{
	OnHttpRequest((CSTXIOCPServerClientContext*)pUserData, mapRequest, bPost);
}

void CUniversalIOCPServer::OnWebSocketHandShake(const char *pSecKey, const char *pszPath, void* pUserData)
{
	CSHA1Hash hash;
	hash.Init();
	hash.Create();
	std::string responseHashSrc = pSecKey;
	responseHashSrc += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	hash.HashData((LPBYTE)responseHashSrc.c_str(), responseHashSrc.size());
	BYTE hashResult[128];
	DWORD dwHashResultLen = 128;
	hash.GetHashValue(hashResult, &dwHashResultLen);
	char szBase64[128];
	DWORD dwBase64Len = 128;
	CryptBinaryToStringA(hashResult, dwHashResultLen, CRYPT_STRING_BASE64, szBase64, &dwBase64Len);

	OnHttpWebSocketHandShake((CSTXIOCPServerClientContext*)pUserData, pszPath, pSecKey, szBase64);
}

void CUniversalIOCPServer::OnHttpWebSocketHandShake(CSTXIOCPServerClientContext *pClientContext, const char *pszPath, const char *pSecKey, const char *pSecKeyResponse)
{
	/*
	HTTP/1.1 101 Switching Protocols
	Upgrade: websocket
	Connection: Upgrade
	Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
	*/
	
	CUniversalIOCPServerClientContext* pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);
	if (pClient)
	{
		pClient->BeginWebSocketProtocolMode();
	}

	std::string responseHeader = "HTTP/1.1 101 Web Socket Protocol Handshake";
	responseHeader += "\r\nAccess-Control-Allow-Credentials: true\r\nAccess-Control-Allow-Headers: content-type\r\nAccess-Control-Allow-Headers: authorization\r\nAccess-Control-Allow-Headers: x-websocket-extensions\r\nAccess-Control-Allow-Headers: x-websocket-version\r\nAccess-Control-Allow-Headers: x-websocket-protocol\r\nAccess-Control-Allow-Origin : null";
	responseHeader += "\r\nConnection: Upgrade";
	responseHeader += "\r\nSec-WebSocket-Accept: ";
	responseHeader += pSecKeyResponse;	//pSecKeyResponse includes a CRLF at the end
	responseHeader += "Server:Test\r\nUpgrade: websocket";
	responseHeader += "\r\n\r\n";

	SendClientData(pClientContext, (LPVOID)responseHeader.c_str(), responseHeader.size());

}

void CUniversalIOCPServer::OnHttpRequest(CSTXIOCPServerClientContext *pClientContext, std::map<std::string, std::string> &mapRequest, bool bPost)
{
	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);

	STXTRACE(3, _T("Http request on server [port=%d]: %S"), pClient->GetServerContext()->GetListeningPort(), mapRequest["*page"].c_str());
}

void CUniversalIOCPServer::SendRawResponseData(__int64 nClientUID, LPVOID pData, unsigned int nDataLen)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		SendClientData(it->second, pData, nDataLen);
	}
	_mapClients.unlock(nClientUID);
}

void CUniversalIOCPServer::SendRawResponseDataToAdminClients(LPVOID pData, unsigned int nDataLen)
{
	_mapClients.foreach([&](std::pair<__int64, CUniversalIOCPServerClientContext*> item)
	{
		if (item.second->GetRole() == UniversalTcpClientRole_Admin)
			SendClientData(item.second, pData, nDataLen);
	});
}

void CUniversalIOCPServer::SendRawResponseDataToLogMonitors(LPVOID pData, unsigned int nDataLen)
{
	_mapLogMonitorClients.foreach([&](std::pair<__int64, CUniversalIOCPServerClientContext*> item)
	{
		SendClientData(item.second, pData, nDataLen);
	});
}

void CUniversalIOCPServer::SendRawResponseDataToDebugMonitors(LPVOID pData, unsigned int nDataLen)
{
	_mapDebugMonitorClients.foreach([&](std::pair<__int64, CUniversalIOCPServerClientContext*> item)
	{
		SendClientData(item.second, pData, nDataLen);
	});
}

void CUniversalIOCPServer::SendWebSocketResponseData(__int64 nClientUID, LPVOID pData, unsigned int nDataLen)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		SendWebSocketResponseData(it->second, pData, nDataLen);
	}
	_mapClients.unlock(nClientUID);
}

void CUniversalIOCPServer::SendWebSocketResponseData(CSTXIOCPServerClientContext *pClientContext, LPVOID pData, unsigned int nDataLen)
{
	CSTXIOCPBuffer buffer;
	BYTE nHead = 0x81;		//Fin=1, Opcode=1:Text Frame
	buffer.WriteBuffer(&nHead, 1);

	if (nDataLen <= 125)
	{
		BYTE nLen = (BYTE)nDataLen;
		nLen &= 0x7F;					//Mask = 0
		buffer.WriteBuffer(&nLen, 1);
	}
	else if (nDataLen >= 126 && nDataLen <= 65535)
	{
		BYTE nLen = 126;
		nLen &= 0x7F;					//Mask = 0
		buffer.WriteBuffer(&nLen, 1);

		WORD nExtLen = (WORD)nDataLen;

		//Swap bytes for network byte ordering
		buffer.WriteBuffer(((BYTE*)&nExtLen) + 1, 1);
		buffer.WriteBuffer(((BYTE*)&nExtLen), 1);
	}
	else if (nDataLen >= 65536)
	{
		BYTE nLen = 127;
		nLen &= 0x7F;					//Mask = 0
		buffer.WriteBuffer(&nLen, 1);

		uint64_t nExtLen = (uint64_t)nDataLen;

		//Swap bytes for network byte ordering
		nExtLen = swap64(nExtLen);
		buffer.WriteBuffer(&nExtLen, sizeof(nExtLen));
	}

	buffer.WriteBuffer(pData, nDataLen);
	SendClientData(pClientContext, buffer.GetBufferPtr(), buffer.GetDataLength());
}

void CUniversalIOCPServer::SendHttpResponseData(CSTXIOCPServerClientContext *pClientContext, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen)
{
	char szResponseHeader[256];
	int nLen = sprintf_s(szResponseHeader, "HTTP/1.1 %d OK\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", nResponseCode, "UniversalServer", pszContentType, nDataLen);

	if (nLen > 0)
	{
		SendClientData(pClientContext, (void*)szResponseHeader, nLen);

		if (nDataLen > 0)
		{
			SendClientData(pClientContext, pData, nDataLen);
		}
	}
}

void CUniversalIOCPServer::SendHttpResponseData(__int64 nClientUID, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		SendHttpResponseData(it->second, nResponseCode, pszContentType, pData, nDataLen);
	}
	_mapClients.unlock(nClientUID);
}

void CUniversalIOCPServer::Parse(const char *pszHeader, void* pUserData, const char *pHeaderEnd)
{
	std::map<std::string, std::string> mapRequestContext;
	int nLen = strlen(pszHeader);

	if (nLen < 4)
		return;

	bool bPost = false;
	if (_strnicmp(pszHeader, "GET", 3) == 0)
	{

	}
	else if (_strnicmp(pszHeader, "POST", 4) == 0)
	{
		bPost = true;
	}

	const char *pszFirstSpace = strstr(pszHeader, " ");
	if (pszFirstSpace == NULL)
	{
		OnMessage(mapRequestContext, pUserData, bPost);
		return;
	}
	const char *pszSecondSpace = strstr(pszFirstSpace + 1, " ");
	if (pszSecondSpace == NULL)
	{
		OnMessage(mapRequestContext, pUserData, bPost);
		return;
	}

	int nParamLen = pszSecondSpace - pszFirstSpace - 1;
	char *pszParam = new char[nParamLen + 2];
	if (pszParam == NULL)
	{
		STXTRACELOGFE(_T("CUniversalIOCPServer::Parse: not enough memory."));
		return;
	}
	strncpy_s(pszParam, nParamLen + 2, pszFirstSpace + 1, nParamLen);


	char *pszPostParam = NULL;
	const char *pszStart = strchr(pszParam, '?');
	if (pszStart == NULL)
	{
		mapRequestContext["*page"] = pszParam;

		if (bPost)
		{
			const char* lenptr = stristr(pszHeader, "content-length: ", -1);
			if (lenptr)
			{
				DWORD dwAppendLen = atoi(lenptr + 15);
				pszPostParam = new char[dwAppendLen + 2];
				if (pszPostParam == NULL)
				{
					STXTRACELOGFE(_T("CUniversalIOCPServer::Parse: not enough memory."));
					return;
				}
				strncpy_s(pszPostParam, dwAppendLen + 2, pHeaderEnd, dwAppendLen);
				pszStart = pszPostParam - 1;	//-1 to fix later +=1
			}
		}
	}
	else
	{
		int nPageLen = pszStart - pszParam;
		char *pszPage = new char[nPageLen + 2];
		if (pszPage == NULL)
		{
			STXTRACELOGFE(_T("CUniversalIOCPServer::Parse: not enough memory."));
			return;
		}
		strncpy_s(pszPage, nPageLen + 2, pszParam, nPageLen);
		mapRequestContext["*page"] = pszPage;
		delete[]pszPage;
	}

	if (pszStart)
	{
		pszStart += 1;

		std::vector<std::string> arrParts;
		SplitString(pszStart, '&', arrParts);

		std::vector<std::string>::iterator it = arrParts.begin();
		char szUnescape[1024];
		DWORD dwUnescapeLen = 1024;
		for (; it != arrParts.end(); it++)
		{
			dwUnescapeLen = 1024;
			std::vector<std::string> arrKeyValue;
			SplitString((*it).c_str(), '=', arrKeyValue);
			if (arrKeyValue.size() < 2)
				continue;

			UrlUnescapeA((char*)arrKeyValue[1].c_str(), szUnescape, &dwUnescapeLen, 0);

			mapRequestContext[arrKeyValue[0]] = szUnescape;
		}
	}

	delete[]pszParam;

	if (pszPostParam)
	{
		delete[]pszPostParam;
	}

	//Now detecting for WebSocket connection request
	const char* upgradePtr = stristr(pszHeader, "Upgrade: ", -1);
	if (upgradePtr)
	{
		const char* upgradeValuePtr = upgradePtr + 8;
		while (*upgradeValuePtr && *upgradeValuePtr <= 0x20)
			upgradeValuePtr++;

		if (_strnicmp(upgradeValuePtr, "websocket", 9) == 0)
		{
			const char* connectionPtr = stristr(pszHeader, "Connection: ", -1);
			if (connectionPtr)
			{
				const char* connectionValuePtr = connectionPtr + 11;
				while (*connectionValuePtr && *connectionValuePtr <= 0x20)
					connectionValuePtr++;

				if (_strnicmp(connectionValuePtr, "Upgrade", 7) == 0)
				{
					const char* webSocketKeyPtr = stristr(pszHeader, "Sec-WebSocket-Key: ", -1);
					if (webSocketKeyPtr)
					{
						const char* webSocketKeyValuePtr = webSocketKeyPtr + 18;
						while (*webSocketKeyValuePtr && *webSocketKeyValuePtr <= 0x20)
							webSocketKeyValuePtr++;

						std::string webSocketKey;
						while (*webSocketKeyValuePtr && *webSocketKeyValuePtr != 0x0A && *webSocketKeyValuePtr != 0x0D)
						{
							webSocketKey += *webSocketKeyValuePtr;
							webSocketKeyValuePtr++;
						}

						OnWebSocketHandShake(webSocketKey.c_str(), mapRequestContext["*page"].c_str(), pUserData);
						return;
					}
				}
			}
		}
	}

	OnMessage(mapRequestContext, pUserData, bPost);
}

extern "C"
static int UniversalIOCPServer_lua_writer(lua_State *L, const void *p, size_t size, void *u)
{
	CSTXIOCPBuffer *pBuffer = (CSTXIOCPBuffer*)u;
	pBuffer->WriteBuffer((LPVOID)p, size);
	return 0;
}

extern "C" static const char * UniversalIOCPServer_lua_reader(lua_State *L,
	void *data,
	size_t *size)
{
	CSTXIOCPBuffer *pBuffer = (CSTXIOCPBuffer*)data;
	*size = pBuffer->GetDataLength();
	return (const char *)pBuffer->GetBufferPtr();
}

extern "C" static const char * UniversalIOCPServer_lua_reader_bc(lua_State *L,
	void *data,
	size_t *size)
{
	CUniversalStringCache::ByteCodeInfo *pByteCodeInfo = (CUniversalStringCache::ByteCodeInfo*)data;
	*size = pByteCodeInfo->byteCodeLen;
	return (const char *)pByteCodeInfo->byteCode;
}

BOOL CUniversalIOCPServer::OnWebSocketClientPing(CUniversalIOCPServerClientContext *pClientContext)
{
	return TRUE;
}

BOOL CUniversalIOCPServer::OnWebSocketClientPong(CUniversalIOCPServerClientContext *pClientContext)
{
	return TRUE;
}

BOOL CUniversalIOCPServer::OnWebSocketClientReceived(CUniversalIOCPServerClientContext *pClientContext, BYTE *pDataBuffer, DWORD cbDataLen)
{
	//Lua script preparation. The following methods can be used in lua script:
	// utils.GetClientUid()
	// utils.GetServerParam()
	// utils.GetMessageBase64()
	// utils.GetMessage() - text message

	lua_State *L = GetLuaStateForCurrentThread();

	LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetClientUid", [&] {
		return pClientContext->_uid;
	}).addFunction("GetServerParam", [&] {
		USES_CONVERSION;
		std::string serverParam = (LPCSTR)ATL::CW2A(pClientContext->GetServerContext()->GetServerParamString().c_str());
		return serverParam;
	}).endModule();


	LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetMessage", [&] {
		std::string strMsg;
		strMsg.append((const char*)pDataBuffer, (const char*)pDataBuffer + cbDataLen);
		return strMsg;

	}).addFunction("GetMessageBase64", [&] {
		std::string strMsg;
		DWORD dwBase64Len = (cbDataLen / 3 + 4) * 5;
		strMsg.resize(dwBase64Len);
		CryptBinaryToStringA(pDataBuffer, cbDataLen, CRYPT_STRING_BASE64, (char*)strMsg.c_str(), &dwBase64Len);
		return strMsg;
	}).endModule();

	//Run script
	auto serverContext = pClientContext->GetServerContext();
	auto scriptCache = GetTcpServerReceiveScript(dynamic_cast<CUniversalIOCPTcpServerContext*>(serverContext.get()));
	if (scriptCache)
	{
		RunScriptCache(*scriptCache.get());
	}

	return TRUE;
}


BOOL CUniversalIOCPServer::OnClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer)
{
	if (_statisticsEnabled)
	{
		_totalReceivedCount++;
		_totalReceivedBytes += pBuffer->GetDataLength();

		_statisticsReceiveBytes.AddValue(pBuffer->GetDataLength());
		_statisticsReceiveCount.AddValue(1);
	}

	CUniversalIOCPServerClientContext *pClient = dynamic_cast<CUniversalIOCPServerClientContext*>(pClientContext);

	if (pClient)
	{
		if (pClient->m_bLastWebSocketPacketFin)
		{
			DWORD dwLen = pClient->GetBufferedWebSocketMessageLength();

			//dwLen can be 0 because Websocket will generate special message like Ping and Pong
			//We only proceed when dwLen is greater than 0 (means actual data was received)
			if (dwLen > 0)
			{
				BYTE *pDataBuffer = (BYTE*)pClient->GetWebSocketMessageBasePtr();
				BOOL bWSRecvResult = OnWebSocketClientReceived(pClient, pDataBuffer, dwLen);
				pClient->SkipWebSocketRecvBuffer(dwLen);
				return bWSRecvResult;
			}
			return TRUE;
		}
	}

	//Event handler callback
	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpClientReceived(pClient->GetServerContext()->GetServerParamString().c_str(), pClient->_uid, pBuffer->GetBufferPtr(), pBuffer->GetDataLength(), &bSkipScript);


	if (!bSkipScript &&
		(pClient->_serverType == TcpServerTypeStream				//Raw Data
		|| pClient->_serverType == TcpServerTypeBinaryHeaderV		//VariableHeader Server
		|| pClient->_serverType == TcpServerTypeBinaryHeader4		//4 bytes header size prefix
		|| pClient->_serverType == TcpServerTypeBinaryHeader2))		//2 bytes header size prefix
	{
		long nBufferLen = pBuffer->GetDataLength();
		BYTE *pDataBuffer = (BYTE*)pBuffer->GetBufferPtr();

		//Lua script preparation. The following methods can be used in lua script:
		// utils.GetClientUid()
		// utils.GetServerParam()
		// utils.GetMessageBase64()

		lua_State *L = GetLuaStateForCurrentThread();
		LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetClientUid", [&] {
			return pClient->_uid;
		}).addFunction("GetServerParam", [&] {
			USES_CONVERSION;
			std::string serverParam = (LPCSTR)ATL::CW2A(pClient->GetServerContext()->GetServerParamString().c_str());
			return serverParam;
		}).addFunction("GetMessageBase64", [&] {
			std::string strMsg;
			DWORD dwBase64Len = (nBufferLen / 3 + 4) * 5;
			strMsg.resize(dwBase64Len);
			CryptBinaryToStringA(pDataBuffer, nBufferLen, CRYPT_STRING_BASE64, (char*)strMsg.c_str(), &dwBase64Len);
			return strMsg;
		}).endModule();

		if (pClient->_serverType == TcpServerTypeBinaryHeaderV)
		{
			//For variable header server:
			// utils.GetMessage()
			LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetMessage", [&] {
				size_t nLenParsed = 0;
				std::shared_ptr<CSTXProtocolLua> spInner(new CSTXProtocolLua());
				spInner->_protocol.Decode(pDataBuffer, &nLenParsed);

				return spInner;
			}).endModule();
		}
		

		auto serverContext = pClient->GetServerContext();
		auto scriptCache = GetTcpServerReceiveScript(dynamic_cast<CUniversalIOCPTcpServerContext*>(serverContext.get()));
		if (scriptCache)
		{
			RunScriptCache(*scriptCache.get());
		}

	}
	else if (pClient->_serverType == TcpServerTypeHttp)		//HTTP Server
	{
		char *pszBuffer = (char*)pBuffer->GetBufferPtr();
		for (DWORD i = 0; i < pBuffer->GetDataLength(); i++)
		{
			pClient->AppendChar(pszBuffer[i], pClientContext, pszBuffer + i);
		}
	}

	return TRUE;
}

void CUniversalIOCPServer::OnUdpServerReceived(CSTXIOCPUdpServerContext *pUdpServerContext, CSTXIOCPBuffer *pBuffer, SOCKADDR *pFromAddr, INT nAddrLen)
{
	TCHAR szAddress[MAX_PATH];
	DWORD dwAddressLen = MAX_PATH;
	WSAAddressToString(pFromAddr, nAddrLen, NULL, szAddress, &dwAddressLen);

	long nBufferLen = pBuffer->GetDataLength();
	BYTE *pDadaBuffer = (BYTE*)pBuffer->GetBufferPtr();
}

void CUniversalIOCPServer::OnTcpReceived(CSTXIOCPTcpConnectionContext *pTcpConnCtx, CSTXIOCPBuffer *pBuffer)
{
	CUniversalIOCPTcpConnectionContext *pConnection = dynamic_cast<CUniversalIOCPTcpConnectionContext*>(pTcpConnCtx);

	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpConnectionReceived(pTcpConnCtx->GetServerParamString().c_str(), pTcpConnCtx->GetConnectionID(), pBuffer->GetBufferPtr(), pBuffer->GetDataLength(), &bSkipScript);

	if (!bSkipScript &&
		(pTcpConnCtx->GetServerParam() == TcpConnectionTypeStream					//Raw Data
			|| pTcpConnCtx->GetServerParam() == TcpConnectionTypeBinaryHeaderV		//VariableHeader Server
			|| pTcpConnCtx->GetServerParam() == TcpConnectionTypeBinaryHeader4		//4 bytes header size prefix
			|| pTcpConnCtx->GetServerParam() == TcpConnectionTypeBinaryHeader2))		//2 bytes header size prefix
	{
		long nBufferLen = pBuffer->GetDataLength();
		BYTE *pDataBuffer = (BYTE*)pBuffer->GetBufferPtr();

		//Lua script preparation. The following methods can be used in lua script:
		// utils.GetConnectionId()
		// utils.GetConnectionParam()
		// utils.GetMessageBase64()
		lua_State *L = GetLuaStateForCurrentThread();

		LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetConnectionId", [&] {
			return pConnection->GetConnectionID();
		}).addFunction("GetConnectionParam", [&] {
			USES_CONVERSION;
			std::string connParam = (LPCSTR)ATL::CW2A(pConnection->GetServerParamString().c_str());
			return connParam;
		}).addFunction("GetMessageBase64", [&] {
			std::string strMsg;
			DWORD dwBase64Len = (nBufferLen / 3 + 4) * 5;
			strMsg.resize(dwBase64Len);
			CryptBinaryToStringA(pDataBuffer, nBufferLen, CRYPT_STRING_BASE64, (char*)strMsg.c_str(), &dwBase64Len);
			return strMsg;
		}).endModule();

		if (pTcpConnCtx->GetServerParam() == TcpConnectionTypeBinaryHeaderV)
		{
			//For variable header server:
			// utils.GetMessage()

			LuaIntf::LuaBinding(L).beginModule("utils").addFunction("GetMessage", [&] {
				size_t nLenParsed = 0;
				std::shared_ptr<CSTXProtocolLua> spInner(new CSTXProtocolLua());
				spInner->_protocol.Decode(pDataBuffer, &nLenParsed);

				return spInner;
			}).endModule();
		}

		//Run script 
		auto scriptCache = GetTcpConnectionReceiveScript(dynamic_cast<CUniversalIOCPTcpConnectionContext*>(pTcpConnCtx));
		if (scriptCache)
		{
			RunScriptCache(*scriptCache.get());
		}
	}
}

void CUniversalIOCPServer::OnTcpConnect(CSTXIOCPTcpConnectionContext *pTcpConnCtx)
{
	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpConnectionConnected(pTcpConnCtx->GetServerParamString().c_str(), pTcpConnCtx->GetConnectionID(), &bSkipScript);

}

void CUniversalIOCPServer::OnTcpDisconnected(CSTXIOCPTcpConnectionContext *pTcpConnCtx, DWORD dwError)
{
	BOOL bSkipScript = FALSE;
	if (_pServer && _pServer->_callback)
		_pServer->_callback->OnTcpConnectionDisconnected(pTcpConnCtx->GetServerParamString().c_str(), pTcpConnCtx->GetConnectionID(), &bSkipScript);

}

DWORD CUniversalIOCPServer::IsTcpDataReadable(CSTXIOCPTcpConnectionContext *pTcpConnCtx)
{
	switch (pTcpConnCtx->GetServerParam())
	{
	case TcpConnectionTypeStream:
		return pTcpConnCtx->GetBufferedMessageLength();
	case TcpConnectionTypeBinaryHeader2:
	{
		DWORD dwMsgDataLen = pTcpConnCtx->GetBufferedMessageLength();

		if (dwMsgDataLen < sizeof(WORD))
			return 0;

		WORD wSize = *((WORD*)pTcpConnCtx->GetMessageBasePtr());
		if (wSize <= dwMsgDataLen)
			return wSize;
	}
	break;
	case TcpConnectionTypeBinaryHeader4:
	{
		DWORD dwMsgDataLen = pTcpConnCtx->GetBufferedMessageLength();

		if (dwMsgDataLen < sizeof(DWORD))
			return 0;

		DWORD dwSize = *((DWORD*)pTcpConnCtx->GetMessageBasePtr());
		if (dwSize <= dwMsgDataLen)
			return dwSize;
	}
	break;
	case TcpConnectionTypeBinaryHeaderV:
	{
		DWORD dwMsgDataLen = pTcpConnCtx->GetBufferedMessageLength();
		if (dwMsgDataLen == 0)
			return 0;

		BYTE nLengthBytes = 0;
		LONG nLength = CSTXProtocol::DecodeCompactInteger(pTcpConnCtx->GetMessageBasePtr(), &nLengthBytes);

		if (nLength < 0)
			return 0;

		if ((DWORD)(nLength + nLengthBytes) <= dwMsgDataLen)
			return nLengthBytes + nLength;
	}
	break;
	}
	return 0;

}

void CUniversalIOCPServer::OnUrlDownloadComplete(LPSTXIOCPSERVERHTTPCONTEXT pContext)
{
	long nBufferLen = pContext->bufferContent.GetDataLength();
	BYTE *pDadaBuffer = (BYTE*)pContext->bufferContent.GetBufferPtr();
}

void CUniversalIOCPServer::SetTcpClientTimeout(__int64 nClientUID, DWORD dwTimeout)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		it->second->SetOperationTimeout(dwTimeout);
	}
	_mapClients.unlock(nClientUID);
}

void CUniversalIOCPServer::SetTcpClientRole(__int64 nClientUID, UniversalTcpClientRole role)
{
	CUniversalIOCPServerClientContext *pClient = NULL;
	UniversalTcpClientRole oldRole = UniversalTcpClientRole_Default;

	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		oldRole = it->second->SetRole(role);
		pClient = it->second;
	}
	_mapClients.unlock(nClientUID);

	if (pClient)
	{
		_mapLogMonitorClients.lock(nClientUID);
		if (role == UniversalTcpClientRole_LogMonitor)
		{
			_mapLogMonitorClients[nClientUID] = pClient;
		}
		else
		{
			_mapLogMonitorClients.erase(nClientUID);
		}
		_mapLogMonitorClients.unlock(nClientUID);

		_mapDebugMonitorClients.lock(nClientUID);
		if (role == UniversalTcpClientRole_DebugMonitor)
		{
			_mapDebugMonitorClients[nClientUID] = pClient;
		}
		else
		{
			_mapDebugMonitorClients.erase(nClientUID);
		}
		_mapDebugMonitorClients.unlock(nClientUID);

		if (oldRole != role)
		{
			if (role == UniversalTcpClientRole_LogMonitor)
			{
				InterlockedIncrement(&_logMonitorCount);
			}
			else if(oldRole == UniversalTcpClientRole_LogMonitor)
			{
				InterlockedDecrement(&_logMonitorCount);
			}

			if (role == UniversalTcpClientRole_DebugMonitor)
			{
				InterlockedIncrement(&_debugMonitorCount);
			}
			else if(oldRole == UniversalTcpClientRole_DebugMonitor)
			{
				InterlockedDecrement(&_debugMonitorCount);
			}
		}
	}
}

void CUniversalIOCPServer::OnClientSent(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer)
{
	if (_statisticsEnabled)
	{
		_totalSentCount++;
		_totalSentBytes += pBuffer->GetDataLength();

		_statisticsSentBytes.AddValue(pBuffer->GetDataLength());
		_statisticsSentCount.AddValue(1);
	}
}

long long CUniversalIOCPServer::GetTotalSentBytes() const
{
	return _totalSentBytes;
}

long long CUniversalIOCPServer::GetTotalReceivedBytes() const
{
	return _totalReceivedBytes;
}

long long CUniversalIOCPServer::GetTotalSentCount() const
{
	return _totalSentCount;
}

long long CUniversalIOCPServer::GetTotalReceivedCount() const
{
	return _totalReceivedCount;
}

void CUniversalIOCPServer::OnShutDown()
{
	StopServerRPC();

	TCHAR szTemp[MAX_PATH];
	if (!_statisticsEnabled)
	{
		STXTRACELOGE(_T("Statistics data gathering is currently disabled!. _statisticsEnabled = %d"), _statisticsEnabled);
	}
	STXTRACELOGE(_T("Total Connected: \t%I64d"), __int64(_totalConnected));
	StrFormatByteSize64(_totalSentBytes, szTemp, MAX_PATH);
	STXTRACELOGE(_T("Total Sent: \t%I64d Times \t%I64d Bytes (%s)"), __int64(_totalSentCount), __int64(_totalSentBytes), szTemp);
	StrFormatByteSize64(_totalReceivedBytes, szTemp, MAX_PATH);
	STXTRACELOGE(_T("Total Received: \t%I64d Times \t%I64d Bytes (%s)"), __int64(_totalReceivedCount), __int64(_totalReceivedBytes), szTemp);
	STXTRACELOGE(_T("Total Graceful Disconnect: %d,  \t Total Unexpected Disconnect: %d"), _numGracefulDisconnect, _numUnexpectDisconnect);

	CSTXIOCPServer::OnShutDown();
}

extern "C"
{
	void RunScriptFile(handle_t IDL_handle, const WCHAR *pszScriptFile)
	{
		_s_server->_pServer->Run(pszScriptFile);
	}
	void RunScriptString(handle_t IDL_handle, const WCHAR *pszScript, BSTR *pstrResult)
	{
		std::wstring strResult;
		_s_server->_pServer->RunScriptString(pszScript, &strResult);
		if (!strResult.empty())
		{
			*pstrResult = SysAllocString(CComBSTR(strResult.c_str()));
		}
	}
	void EnqueueWorkerThreadScriptString(handle_t IDL_handle, const WCHAR *pszScript)
	{
		_s_server->_pServer->EnqueueWorkerThreadScriptString(pszScript);
	}
	void* /*__RPC_FAR**/ __RPC_USER midl_user_allocate(size_t len)
	{
		return(malloc(len));
	}
	void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
	{
		free(ptr);
	}
}

UINT WINAPI CUniversalIOCPServer::UniversalServerRPCThread(LPVOID lpParameter)
{
	CUniversalIOCPServer *pServer = (CUniversalIOCPServer*)lpParameter;

	TCHAR szPort[64];
	_itot_s(pServer->m_nRPCServerPort, szPort, 10);

	RPC_STATUS status;
	status = RpcServerUseProtseqEp(
		(RPC_WSTR)(_T("ncacn_ip_tcp")), // Use TCP/IP protocol.
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // Backlog queue length for TCP/IP.
		(RPC_WSTR)(szPort), // TCP/IP port to use.
		NULL); // No security.

	if (status)
	{
		STXTRACELOGE(3, _T("[r][i]RpcServerUseProtseqEp failed @ port %d ! status = %d"), pServer->m_nRPCServerPort, status);
		return 0;
	}

	status = RpcServerRegisterIfEx(
		UniversalServerRPC_v1_0_s_ifspec, // Interface to register.
		NULL,
		NULL, // Use the MIDL generated entry-point vector.
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
		0,
		NULL);

	if (status)
	{
		STXTRACELOGE(3, _T("[r][i]RpcServerRegisterIfEx failed! status = %d"), status);
		return 0;
	}

	// This call will not return until
	// RpcMgmtStopServerListening is called.

	STXTRACELOGE(_T("RPC Server will listen at port %d, Status = 0x%X, Thread ID = 0x%X"), pServer->m_nRPCServerPort, status, GetCurrentThreadId());

	status = RpcServerListen(
		1, // Recommended minimum number of threads.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Recommended maximum number of threads.
		FALSE); // Start listening now.

	if (status)
	{
		STXTRACELOGE(3, _T("[r][i]RpcServerListen failed! status = %d"), status);
		return 0;
	}

	STXTRACELOGE(_T("RPC thread terminated."));
	return 0;
}

void CUniversalIOCPServer::OnServerInitialized()
{
	ULONGLONG tick = GetTickCount64();
	STXTRACELOGE(3, _T("Loading file modify/write time for all files in the server directory..."));

	_defaultFolderMonitorID = MonitorFolder(this->GetServerModuleFilePath());
	if (_defaultFolderMonitorID < 0)
	{
		STXTRACELOGE(_T("[r][i]MonitorFolder failed for %s"), this->GetServerModuleFilePath());
	}
	else
	{
		this->AddFolderMonitorIgnoreFileExtension(_defaultFolderMonitorID, _T(".log"));
	}

	CreateServerRPCThread();
}

void CUniversalIOCPServer::CreateServerRPCThread(UINT nPort)
{
	if (nPort != 0)
	{
		m_nRPCServerPort = nPort;
	}

	if (m_nRPCServerPort == 0)
	{
		STXTRACELOGE(_T("RPC port is not specified. RPC server will not be created."));
		return;
	}

	if (_hRPCThread != NULL)
	{
		STXTRACELOGE(_T("[r][g][i]RPC thread is running, RPC port is %d. It must be stopped before create a new RPC thread."), m_nRPCServerPort);
		return;
	}

	STXTRACELOGE(_T("Now starting RPC thread..."));

	UINT uThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, UniversalServerRPCThread, this, 0, &uThreadID);
	if (hThread == NULL)
	{
		STXTRACELOGE(_T("[r][i]Server RPC thread create failed!"));
	}
	_hRPCThread = hThread;
}

void CUniversalIOCPServer::StopServerRPC()
{
	if (_hRPCThread)
	{
		RpcMgmtStopServerListening(NULL);
		RpcServerUnregisterIf(NULL, NULL, FALSE);

		WaitForSingleObject(_hRPCThread, INFINITE);
		_hRPCThread = NULL;
	}
}

void CUniversalIOCPServer::OnFileChangedFiltered(CSTXIOCPFolderMonitorContext *pFolderMonitorContext, DWORD dwAction, LPCTSTR lpszFileName, LPCTSTR lpszFileFullPathName)
{
	LPCTSTR lpszRelativePathName = lpszFileFullPathName + _tcslen(GetServerModuleFilePath());
	LPCTSTR lpszName = PathFindFileName(lpszRelativePathName);

	static LPCTSTR actionText[8] = { 0, _T("Added"), _T("Removed"), _T("Modified"), _T("Renaming"), _T("Renamed") };

	std::wstring fileName = lpszName;
	LPTSTR lpszExt = PathFindExtension(fileName.c_str());
	if (lpszExt)
	{
		*lpszExt = 0;
	}

	if (_tcsicmp(lpszExt + 1, _T("lua")) == 0)
	{
		SetLuaCacheDirtyForModuleChange(fileName.c_str());
	}

	OnServerFileChanged(dwAction, lpszRelativePathName, lpszFileFullPathName);
}


void CUniversalIOCPServer::SetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, LPCTSTR lpszValue)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		it->second->_userDataStringMap[lpszKey] = lpszValue;
	}
	_mapClients.unlock(nClientUID);
}

BOOL CUniversalIOCPServer::GetClientUserDataString(__int64 nClientUID, LPCTSTR lpszKey, std::wstring& valueOut)
{
	BOOL bResult = FALSE;
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		auto itUDM = it->second->_userDataStringMap.find(lpszKey);
		if (itUDM != it->second->_userDataStringMap.end())
		{
			valueOut = itUDM->second;
			bResult = TRUE;
		}
	}
	_mapClients.unlock(nClientUID);
	return bResult;
}

void CUniversalIOCPServer::SetTcpServerReceiveScript(UINT nPort, LPCTSTR lpszScriptFile)
{
	std::shared_ptr<CUniversalStringCache> scriptCache;
	std::wstring originalName;
	_mapTcpServerRecvScripts.lock(nPort);
	auto it = _mapTcpServerRecvScripts.find(nPort);
	if (it == _mapTcpServerRecvScripts.end(nPort))
	{
		scriptCache = std::make_shared<CUniversalStringCache>();
		originalName = scriptCache->GetStringName();
		scriptCache->SetNeedUpdate(true);
		scriptCache->EnableTraceThreadVersion();
		scriptCache->SetStringName(lpszScriptFile);
		_mapTcpServerRecvScripts[nPort] = scriptCache;
	}
	else
	{
		it->second->SetStringName(lpszScriptFile);
		it->second->SetNeedUpdate(true);
		scriptCache = it->second;
	}
	_mapTcpServerRecvScripts.unlock(nPort);

	LockServersMap();
	auto itServer = m_mapTcpServers.find(nPort);
	if (itServer != m_mapTcpServers.end())
	{
		CUniversalIOCPTcpServerContext *serverContext = dynamic_cast<CUniversalIOCPTcpServerContext*>(itServer->second.get());
		if (serverContext)
		{
			serverContext->_tcpServerRecvScript = scriptCache;
		}
	}
	UnlockServersMap();

	_mapScriptFileCaches.lock(lpszScriptFile);
	auto itCache = _mapScriptFileCaches.find(lpszScriptFile);
	if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
	{
		itCache->second.insert(scriptCache.get());
	}
	else
	{
		_mapScriptFileCaches[lpszScriptFile].insert(scriptCache.get());
	}

	//If the script file name changed, remove the previous mapping
	if (originalName.size() > 0)
	{
		itCache = _mapScriptFileCaches.find(originalName);
		if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
		{
			itCache->second.erase(scriptCache.get());
		}
	}
	_mapScriptFileCaches.unlock(lpszScriptFile);
}

void CUniversalIOCPServer::SetTcpConnectionReceiveScript(LONG nConnectionID, LPCTSTR lpszScriptFile)
{
	std::shared_ptr<CUniversalStringCache> scriptCache;
	std::wstring originalName;
	_mapTcpConnectionRecvScripts.lock(nConnectionID);
	auto it = _mapTcpConnectionRecvScripts.find(nConnectionID);
	if (it == _mapTcpConnectionRecvScripts.end(nConnectionID))
	{
		scriptCache = std::make_shared<CUniversalStringCache>();
		originalName = scriptCache->GetStringName();
		scriptCache->SetNeedUpdate(true);
		scriptCache->EnableTraceThreadVersion();
		scriptCache->SetStringName(lpszScriptFile);
		_mapTcpConnectionRecvScripts[nConnectionID] = scriptCache;
	}
	else
	{
		it->second->SetStringName(lpszScriptFile);
		it->second->SetNeedUpdate(true);
		scriptCache = it->second;
	}
	_mapTcpConnectionRecvScripts.unlock(nConnectionID);

	LockConnectionMap();
	auto itConnection = m_mapConnections.find(nConnectionID);
	if (itConnection != m_mapConnections.end())
	{
		CUniversalIOCPTcpConnectionContext *connectionContext = dynamic_cast<CUniversalIOCPTcpConnectionContext*>((CSTXIOCPTcpConnectionContext*)itConnection->second);
		if (connectionContext)
		{
			connectionContext->_tcpConnectionRecvScript = scriptCache;
		}
	}
	LockConnectionMap();

	_mapScriptFileCaches.lock(lpszScriptFile);
	auto itCache = _mapScriptFileCaches.find(lpszScriptFile);
	if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
	{
		itCache->second.insert(scriptCache.get());
	}
	else
	{
		_mapScriptFileCaches[lpszScriptFile].insert(scriptCache.get());
	}

	//If the script file name changed, remove the previous mapping
	if (originalName.size() > 0)
	{
		itCache = _mapScriptFileCaches.find(originalName);
		if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
		{
			itCache->second.erase(scriptCache.get());
		}
	}
	_mapScriptFileCaches.unlock(lpszScriptFile);
}

void CUniversalIOCPServer::SetTcpConnectionDisconnectedScript(LONG nConnectionID, LPCTSTR lpszScriptFile)
{
	std::shared_ptr<CUniversalStringCache> scriptCache;
	std::wstring originalName;
	_mapTcpConnectionDisconnectedScripts.lock(nConnectionID);
	auto it = _mapTcpConnectionDisconnectedScripts.find(nConnectionID);
	if (it == _mapTcpConnectionDisconnectedScripts.end(nConnectionID))
	{
		scriptCache = std::make_shared<CUniversalStringCache>();
		originalName = scriptCache->GetStringName();
		scriptCache->SetNeedUpdate(true);
		scriptCache->EnableTraceThreadVersion();
		scriptCache->SetStringName(lpszScriptFile);
		_mapTcpConnectionDisconnectedScripts[nConnectionID] = scriptCache;
	}
	else
	{
		it->second->SetStringName(lpszScriptFile);
		it->second->SetNeedUpdate(true);
		scriptCache = it->second;
	}
	_mapTcpConnectionDisconnectedScripts.unlock(nConnectionID);

	LockConnectionMap();
	auto itConnection = m_mapConnections.find(nConnectionID);
	if (itConnection != m_mapConnections.end())
	{
		CUniversalIOCPTcpConnectionContext *connectionContext = dynamic_cast<CUniversalIOCPTcpConnectionContext*>((CSTXIOCPTcpConnectionContext*)itConnection->second);
		if (connectionContext)
		{
			connectionContext->_tcpConnectionDisconnectedScript = scriptCache;
		}
	}
	LockConnectionMap();

	_mapScriptFileCaches.lock(lpszScriptFile);
	auto itCache = _mapScriptFileCaches.find(lpszScriptFile);
	if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
	{
		itCache->second.insert(scriptCache.get());
	}
	else
	{
		_mapScriptFileCaches[lpszScriptFile].insert(scriptCache.get());
	}

	//If the script file name changed, remove the previous mapping
	if (originalName.size() > 0)
	{
		itCache = _mapScriptFileCaches.find(originalName);
		if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
		{
			itCache->second.erase(scriptCache.get());
		}
	}
	_mapScriptFileCaches.unlock(lpszScriptFile);
}

void CUniversalIOCPServer::SetTcpServerClientConnectedScript(UINT nPort, LPCTSTR lpszScriptFile)
{
	std::shared_ptr<CUniversalStringCache> scriptCache;
	std::wstring originalName;
	_mapTcpServerConnectedScripts.lock(nPort);
	auto it = _mapTcpServerConnectedScripts.find(nPort);
	if (it == _mapTcpServerConnectedScripts.end(nPort))
	{
		scriptCache = std::make_shared<CUniversalStringCache>();
		originalName = scriptCache->GetStringName();
		scriptCache->SetNeedUpdate(true);
		scriptCache->EnableTraceThreadVersion();
		scriptCache->SetStringName(lpszScriptFile);
		_mapTcpServerConnectedScripts[nPort] = scriptCache;
	}
	else
	{
		it->second->SetStringName(lpszScriptFile);
		it->second->SetNeedUpdate(true);
		scriptCache = it->second;
	}
	_mapTcpServerConnectedScripts.unlock(nPort);

	LockServersMap();
	auto itServer = m_mapTcpServers.find(nPort);
	if (itServer != m_mapTcpServers.end())
	{
		CUniversalIOCPTcpServerContext *serverContext = dynamic_cast<CUniversalIOCPTcpServerContext*>(itServer->second.get());
		if (serverContext)
		{
			serverContext->_tcpServerConnectedScript = scriptCache;
		}
	}
	UnlockServersMap();

	_mapScriptFileCaches.lock(lpszScriptFile);
	auto itCache = _mapScriptFileCaches.find(lpszScriptFile);
	if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
	{
		itCache->second.insert(scriptCache.get());
	}
	else
	{
		_mapScriptFileCaches[lpszScriptFile].insert(scriptCache.get());
	}

	//If the script file name changed, remove the previous mapping
	if (originalName.size() > 0)
	{
		itCache = _mapScriptFileCaches.find(originalName);
		if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
		{
			itCache->second.erase(scriptCache.get());
		}
	}
	_mapScriptFileCaches.unlock(lpszScriptFile);
}

void CUniversalIOCPServer::SetTcpServerClientDisconnectedScript(UINT nPort, LPCTSTR lpszScriptFile)
{
	std::shared_ptr<CUniversalStringCache> scriptCache;
	std::wstring originalName;
	_mapTcpServerClientDisconnectedScripts.lock(nPort);
	auto it = _mapTcpServerClientDisconnectedScripts.find(nPort);
	if (it == _mapTcpServerClientDisconnectedScripts.end(nPort))
	{
		scriptCache = std::make_shared<CUniversalStringCache>();
		originalName = scriptCache->GetStringName();
		scriptCache->SetNeedUpdate(true);
		scriptCache->EnableTraceThreadVersion();
		scriptCache->SetStringName(lpszScriptFile);
		_mapTcpServerClientDisconnectedScripts[nPort] = scriptCache;
	}
	else
	{
		it->second->SetStringName(lpszScriptFile);
		it->second->SetNeedUpdate(true);
		scriptCache = it->second;
	}
	_mapTcpServerClientDisconnectedScripts.unlock(nPort);

	LockServersMap();
	auto itServer = m_mapTcpServers.find(nPort);
	if (itServer != m_mapTcpServers.end())
	{
		CUniversalIOCPTcpServerContext *serverContext = dynamic_cast<CUniversalIOCPTcpServerContext*>(itServer->second.get());
		if (serverContext)
		{
			serverContext->_tcpServerClientDisconnectedScript = scriptCache;
		}
	}
	UnlockServersMap();

	_mapScriptFileCaches.lock(lpszScriptFile);
	auto itCache = _mapScriptFileCaches.find(lpszScriptFile);
	if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
	{
		itCache->second.insert(scriptCache.get());
	}
	else
	{
		_mapScriptFileCaches[lpszScriptFile].insert(scriptCache.get());
	}

	//If the script file name changed, remove the previous mapping
	if (originalName.size() > 0)
	{
		itCache = _mapScriptFileCaches.find(originalName);
		if (itCache != _mapScriptFileCaches.end(lpszScriptFile))
		{
			itCache->second.erase(scriptCache.get());
		}
	}
	_mapScriptFileCaches.unlock(lpszScriptFile);
}

long CUniversalIOCPServer::GetTcpClientCount(UINT nPort)
{
	return m_mapClientContext.size();
}

void CUniversalIOCPServer::SetLogLevel(int level)
{
	g_LogGlobal.SetLogLevel(level, -1);
}

void CUniversalIOCPServer::SetDebugOutputLevel(int level)
{
	g_LogGlobal.SetLogLevel(-1, level);
}

long long CUniversalIOCPServer::GetDefaultFolderMonitorId()
{
	return _defaultFolderMonitorID;
}

void CUniversalIOCPServer::SetStatisticsLevel(UINT level)
{
	_statisticsEnabled = level;
}

UINT CUniversalIOCPServer::GetStatisticsLevel()
{
	return _statisticsEnabled;
}

long long CUniversalIOCPServer::GetSentBytesPerSecond()
{
	return _statisticsSentBytes.GetAverage();
}

long long CUniversalIOCPServer::GetSentCountPerSecond()
{
	return _statisticsSentCount.GetAverage();
}

long long CUniversalIOCPServer::GetReceivedBytesPerSecond()
{
	return _statisticsReceiveBytes.GetAverage();

}

long long CUniversalIOCPServer::GetReceivedCountPerSecond()
{
	return _statisticsReceiveCount.GetAverage();
}

UniversalTcpClientRole CUniversalIOCPServer::GetTcpClientRole(__int64 nClientUID)
{
	UniversalTcpClientRole role = UniversalTcpClientRole_Default;
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		role = it->second->GetRole();
	}
	_mapClients.unlock(nClientUID);

	return role;
}

void CUniversalIOCPServer::DisconnectTcpClient(__int64 nClientUID)
{
	_mapClients.lock(nClientUID);
	auto it = _mapClients.find(nClientUID);
	if (it != _mapClients.end(nClientUID))
	{
		DisconnectClient(it->second);
	}
	_mapClients.unlock(nClientUID);
}

void CUniversalIOCPServer::EnqueueWorkerThreadScript(LPCTSTR lpszScriptString)
{
	auto n = _workerThreadActionScripts.size();
	for (int i = 0; i < n; i++)
	{
		_workerThreadActionScripts[i].push(std::make_shared<std::wstring>(lpszScriptString));
	}
	STXTRACELOGE(_T("Script for worker thread schedued. length of the scheduled script: %d"), _tcslen(lpszScriptString));
}

int CUniversalIOCPServer::CheckLuaStateUpdateForThread(lua_State *pLuaState, CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread)
{
	int nResult = 0;
	if (cache.IsNeedUpdateForThreads(pScriptVersionInThread))
	{
		STXTRACE(_T("[r]\tForce current thread %d to reload module [%s] and all dependencies."), GetCurrentThreadId(), cache.GetStringName());

		std::vector<std::wstring> refreshModules;
		cache.GetCleanupModules(&refreshModules);

		for (std::wstring moduleName : refreshModules)
		{
			char szClear[MAX_PATH];
			sprintf_s(szClear, "package.loaded[\"%S\"] = nil", moduleName.c_str());
			nResult = luaL_dostring(pLuaState, szClear);
			STXTRACE(_T("[r]\t\t\t[%s] cleared and ready to be reloaded."), moduleName.c_str());
		}
	}

	return nResult;
}

lua_State *CUniversalIOCPServer::GetLuaStateForCurrentThread()
{
	CUniversalServerWorkerThreadData *pData = (CUniversalServerWorkerThreadData*)GetThreadUserStorage();
	return pData->_pLuaState;
}

CUniversalServerWorkerThreadData *CUniversalIOCPServer::GetCurrentThreadData()
{
	return (CUniversalServerWorkerThreadData*)GetThreadUserStorage();
}

void CUniversalIOCPServer::LuaBindSTXProtocolClasses(lua_State *L)
{
	LuaIntf::LuaBinding(L).beginClass<CSTXProtocolLua>("STXProtocol")
		.addConstructor(LUA_SP(std::shared_ptr<CSTXProtocolLua>), LUA_ARGS(LuaIntf::_opt<std::string>))
		//.addStaticProperty("home_url", &Web::home_url, &Web::set_home_url)
		//.addStaticFunction("GetServer", &CUniversalIOCPServer::GetServer)
		.addFunction("GetNextString", &CSTXProtocolLua::GetNextString)
		.addFunction("GetNextInteger", &CSTXProtocolLua::GetNextInteger)
		.addFunction("GetNextFloat", &CSTXProtocolLua::GetNextFloat)
		.addFunction("GetNextGUID", &CSTXProtocolLua::GetNextGUID)
		.addFunction("SeekReadToBegin", &CSTXProtocolLua::SeekReadToBegin)
		.addFunction("AppendByte", &CSTXProtocolLua::AppendByte, LUA_ARGS(byte))
		.addFunction("AppendWord", &CSTXProtocolLua::AppendWord, LUA_ARGS(WORD))
		.addFunction("AppendDWord", &CSTXProtocolLua::AppendDWord, LUA_ARGS(DWORD))
		.addFunction("AppendI64", &CSTXProtocolLua::AppendI64, LUA_ARGS(__int64))
		.addFunction("AppendString", &CSTXProtocolLua::AppendString, LUA_ARGS(std::string))
		.addFunction("SkipNextField", &CSTXProtocolLua::SkipNextField)
		.addFunction("IncreaseDWORDAtOffset", &CSTXProtocolLua::IncreaseDWORDAtOffset, LUA_ARGS(LONG))
		.addPropertyReadOnly("NextFieldTypeString", &CSTXProtocolLua::GetNextFieldTypeString, &CSTXProtocolLua::GetNextFieldTypeString)
		.addPropertyReadOnly("NextFieldType", &CSTXProtocolLua::GetNextFieldType, &CSTXProtocolLua::GetNextFieldType)
		.addPropertyReadOnly("DataSize", &CSTXProtocolLua::GetDataSize, &CSTXProtocolLua::GetDataSize)
		.endClass();
}

int CUniversalIOCPServer::LoadScriptCache(lua_State *pLuaState, CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread)
{
	CheckLuaStateUpdateForThread(pLuaState, cache, pScriptVersionInThread);

	LPCTSTR pszLoadType = _T("");
	int nResult = 0;
	auto byteCode = cache.GetByteCode();
	if (byteCode.byteCode)
	{
		pszLoadType = _T("[byte code]");
		nResult = lua_load(pLuaState, UniversalIOCPServer_lua_reader_bc, &byteCode, "custom_script", "b");
	}
	else
	{
		pszLoadType = _T("[script text]");
		LPCTSTR lpszScriptString = cache.GetString();
		if (lpszScriptString == NULL)
		{
			nResult = LUA_ERRERR;
			STXTRACELOGE(_T("[r]\tLoadScriptCache: byte code is NULL, and cache.GetString() returns NULL for %s, schedule reloading."), cache.GetStringName());
			cache.SetNeedUpdate(TRUE);
		}
		else
		{
			nResult = luaL_loadstring(pLuaState, ATL::CW2A(cache.GetString()));
		}
	}

	if (nResult)
	{
		LPCTSTR pszError = GetLuaErrorName(nResult);
		STXTRACELOGE(_T("[r][i]\tLoadScriptCache for %s failed with error coce = %d [%s]"), pszLoadType, nResult, pszError);
	}

	return nResult;
}

void CUniversalIOCPServer::SetLuaCacheDirtyForModuleChange(LPCTSTR lpszScriptModule)
{
	//lua_State *L = GetLuaStateForCurrentThread();
	//char szClear[MAX_PATH];
	//sprintf_s(szClear, "package.loaded[\"%S\"] = nil", lpszScriptModule);
	//luaL_dostring(L, szClear);

	STXTRACELOGE(_T("Lua module changed : [%s]"), lpszScriptModule);

	auto it = _mapLuaModuleReference.find(lpszScriptModule);
	if (it == _mapLuaModuleReference.end(lpszScriptModule))
	{
		STXTRACELOGE(_T("\tThere is no other module that referenced [%s]"), lpszScriptModule);
		return;
	}

	for (CUniversalStringCache *pCache : it->second)
	{
		STXTRACELOGE(_T("\tSchedule [%s] to reload at next access. because [%s] changed."), pCache->GetStringName(), lpszScriptModule);
		pCache->SetNeedUpdate(TRUE);
	}
}

void CUniversalIOCPServer::UpdateLuaModuleReference(lua_State *pLuaState, CUniversalStringCache &cache)
{
	for (std::wstring moduleName : g_loadedLuaModules)
	{
		_mapLuaModuleReference[moduleName].insert(&cache);
	}

	std::vector<std::wstring> refAdded;
	cache.GetReferenceAdded(&refAdded, &g_loadedLuaModules);
	for (std::wstring moduleName : refAdded)
	{
		STXTRACELOGE(_T("\t\t%s : now have new dependence on %s. +>> dependence added!"), cache.GetStringName(), moduleName.c_str());
	}

	std::vector<std::wstring> refRemoved;
	cache.GetReferenceRemove(&refRemoved, &g_loadedLuaModules);
	for (std::wstring moduleName : refRemoved)
	{
		_mapLuaModuleReference[moduleName].erase(&cache);
		STXTRACELOGE(_T("\t\t%s : now have no dependence on %s. <<- dependence removed."), cache.GetStringName(), moduleName.c_str());
		luaL_dostring(pLuaState, "");
	}
	cache.SetReferenceModule(&g_loadedLuaModules);
}

void CUniversalIOCPServer::UpdateAndRunScriptCache(CUniversalStringCache &cache, lua_State *pLuaState, int &nResult, LONGLONG *pScriptVersionInThread)
{
	BOOL bNeedUpdateModuleReference = FALSE;

	if (cache.IsNeedUpdateDirect())
	{
		BOOL bLoadFileResult = FALSE;
		std::wstring &strFileContent = GetFileContentText(cache.GetStringName(), &bLoadFileResult);

		if (!bLoadFileResult)
		{
			STXTRACELOGE(_T("[r][i]\t%s reload failed. Failed to read content of file! use existing cache now."), cache.GetStringName());
			nResult = LoadScriptCache(pLuaState, cache, pScriptVersionInThread);
		}
		else
		{
			std::wstring *pScriptPtr = new std::wstring(strFileContent);
			STXTRACELOGE(_T("\t[%s] loaded from file. script text length = %d."), cache.GetStringName(), pScriptPtr->size());

			std::string buf = (LPCSTR)ATL::CW2A(pScriptPtr->c_str());
			nResult = luaL_loadstring(pLuaState, buf.c_str());
			if (nResult == 0)
			{
				CSTXIOCPBuffer b;
				lua_dump(pLuaState, UniversalIOCPServer_lua_writer, &b, 0);
				cache.SetByteCode(b.GetBufferPtr(), b.GetDataLength());
				cache.SetStringPtr(pScriptPtr);
				bNeedUpdateModuleReference = TRUE;
				g_loadedLuaModules.clear();
				STXTRACELOGE(_T("\t\t\t script loaded and bytecode generated . length = %d."), b.GetDataLength());
			}
			else
			{
				const char *pLuaLoadError = lua_tostring(pLuaState, -1);
				LPCTSTR pszError = GetLuaErrorName(nResult);
				STXTRACELOGE(_T("[r][i]\t%s reload failed: luaL_loadstring result = %d [%s]"), cache.GetStringName(), nResult, pszError);
				STXTRACELOGE(_T("[r][i]\t\t%S"), pLuaLoadError);
			}
		}
	}
	else
	{
		nResult = LoadScriptCache(pLuaState, cache, pScriptVersionInThread);
	}

	if (nResult == 0)
	{
		cache.SetNeedUpdate(FALSE);
	}
	else
	{
		if (cache.IsEmpty())
		{
			// No historical values. waiting for reload at next request
		}
		else
		{
			STXTRACELOGE(_T("[r][i]\t%s now using previous working version."), cache.GetStringName());
			cache.SetNeedUpdate(FALSE);
		}
	}

	if (nResult == 0)
	{
		CheckLuaStateUpdateForThread(pLuaState, cache, pScriptVersionInThread);

		nResult = lua_pcall(pLuaState, 0, LUA_MULTRET, 0);
		if (bNeedUpdateModuleReference)
		{
			UpdateLuaModuleReference(pLuaState, cache);
		}
	}
}

void CUniversalIOCPServer::RunScriptCache(CUniversalStringCache &cache)
{
	int nScriptVersionStockIndex = cache.GetGlobalIndex();
	if (nScriptVersionStockIndex >= 0)
	{
		int nMaxIndexCount = sizeof(GetCurrentThreadData()->_scriptVersions) / sizeof(LONGLONG);
		if (nScriptVersionStockIndex >= nMaxIndexCount)
		{
			STXTRACELOGE(_T("[r][i]\t%s try to use version index at %d, but the maximum index is %d !!"), cache.GetStringName(), nScriptVersionStockIndex, nMaxIndexCount - 1);
			RunScriptCache(cache, NULL);
		}
		else
		{
			RunScriptCache(cache, &GetCurrentThreadData()->_scriptVersions[nScriptVersionStockIndex]);
		}
	}
	else
	{
		RunScriptCache(cache, NULL);
	}
}

void CUniversalIOCPServer::RunScriptCache(CUniversalStringCache &cache, LONGLONG *pScriptVersionInThread)
{
	int nResult = 0;
	USES_CONVERSION;
	lua_State *L = GetLuaStateForCurrentThread();

	if (cache.IsNeedUpdate())
	{
		//STXTRACEE(_T("%s need to update"), cache.GetStringName());

		cache.Lock();
		__try
		{
			__try
			{
				UpdateAndRunScriptCache(cache, L, nResult, pScriptVersionInThread);
			}
			__except (ProcessException(GetExceptionInformation()))
			{

			}
		}
		__finally
		{
			cache.Unlock();
		}
	}
	else
	{
		nResult = LoadScriptCache(L, cache, pScriptVersionInThread);
		if (nResult == 0)
		{
			nResult = lua_pcall(L, 0, LUA_MULTRET, 0);
		}
	}

	if (nResult)
	{
		const char *pLuaLoadError = lua_tostring(L, -1);
		LPCTSTR pszError = GetLuaErrorName(nResult);
		STXTRACELOGE(_T("[r][i]%s in RunScriptCache failed to run with error coce = %d [%s]"), cache.GetStringName(), nResult, pszError);
		STXTRACELOGE(_T("[r][i]\t\t%S"), pLuaLoadError);
	}
	//else
	//{
	//	LuaIntf::LuaRef reg = LuaIntf::LuaRef::registry(L);

	//	LuaIntf::LuaRef table = reg.get<LuaIntf::LuaRef>("_LOADED");

	//	//LuaIntf::LuaRef table(L, "package.loaded");
	//	for (auto& e : table) {
	//		std::string key = e.key<std::string>();
	//		LuaIntf::LuaRef value = e.value<LuaIntf::LuaRef>();
	//		STXTRACEE(_T("[r][i]\t\t%S"), key.c_str());

	//		//for (auto &f : value)
	//		//{
	//		//	std::string key2 = f.key<std::string>();
	//		//	STXTRACEE(_T("[r][i]\t\t\t%S"), key2.c_str());
	//		//}

	//	}
	//}

	//TODO: check environment reliablilty here
	CheckLuaEnvironment(L);
}

int CUniversalIOCPServer::RunScriptString(LPCTSTR lpszScript)
{
	USES_CONVERSION;

	lua_State *L = GetLuaStateForCurrentThread();

	int nResult = 0;

	nResult = luaL_loadstring(L, ATL::CW2A(lpszScript));
	if (nResult == 0)
	{
		nResult = lua_pcall(L, 0, LUA_MULTRET, 0);
	}

	if (nResult)
	{
		const char *pLuaLoadError = lua_tostring(L, -1);
		LPCTSTR pszError = GetLuaErrorName(nResult);
		STXTRACELOGE(_T("ScriptString failed to run with error coce = %d [%s]"), nResult, pszError);
		STXTRACELOGE(_T("[r][i]\t\t%S"), pLuaLoadError);
	}

	//TODO: check environment reliablilty here
	CheckLuaEnvironment(L);

	return nResult;
}

void  CUniversalIOCPServer::SetRPCServerPort(UINT nPort)
{
	m_nRPCServerPort = nPort;
}

int CUniversalIOCPServer::RunScript(LPCTSTR lpszScriptFile)
{
	USES_CONVERSION;

	lua_State *L = GetLuaStateForCurrentThread();

	int nResult = 0;
	BOOL bLoadFileResult = FALSE;
	std::wstring &strFileContent = GetFileContentText(lpszScriptFile, &bLoadFileResult);
	if (!bLoadFileResult)
	{
		STXTRACELOGE(_T("[r][i]\t%s load failed. Failed to read content of file!"), lpszScriptFile);
		nResult = LUA_ERRERR;
	}
	else
	{
		nResult = RunScriptString(strFileContent.c_str());
		if (nResult)
		{
			LPCTSTR pszError = GetLuaErrorName(nResult);
			STXTRACELOGE(_T("%s RunScript failed to run with error coce = %d [%s]"), lpszScriptFile, nResult, pszError);
		}
	}

	//TODO: check environment reliablilty here
	//CheckLuaEnvironment(L);		//Done in RunScriptString

	return nResult;
}

void CUniversalIOCPServer::CheckLuaEnvironment(lua_State *L)
{
	BOOL bError = FALSE;
	bError = bError || !CheckLuaObject<CServerController>(L, "server");

	if (bError)
	{
		RestoreLuaEnvironment(L);
	}
}

void CUniversalIOCPServer::RestoreLuaEnvironment(lua_State *L)
{
	luaL_dostring(L, "server = utils.GetServer()");

	STXTRACELOGE(_T("[r][i][Lua] Lua Environment is restored."));
}

LPCTSTR CUniversalIOCPServer::GetLuaErrorName(int nError)
{
	switch (nError)
	{
	case LUA_OK:
		return _T("No Error");
	case LUA_YIELD:
		return _T("Yield Error");
	case LUA_ERRRUN:
		return _T("Run Error");
	case LUA_ERRSYNTAX:
		return _T("Syntax Error");
	case LUA_ERRMEM:
		return _T("Memory Error");
	case LUA_ERRGCMM:
		return _T("GCMM error");
	case LUA_ERRERR:
		return _T("Error");
	}
	return _T("Unknown Error");
}

void CUniversalIOCPServer::ProcessInternalScriptFileChange(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName)
{
	ULONGLONG tick = GetTickCount64();
	if (dwAction != FILE_ACTION_RENAMED_OLD_NAME)
	{
		std::wstring strRelativePathName = lpszRelativePathName;
		std::replace(strRelativePathName.begin(), strRelativePathName.end(), '\\', '/');
		LPCTSTR pszUnifiedRelativePathName = strRelativePathName.c_str();
		//Refresh script caches
		_mapScriptFileCaches.lock(pszUnifiedRelativePathName);
		auto itCache = _mapScriptFileCaches.find(pszUnifiedRelativePathName);
		if (itCache != _mapScriptFileCaches.end(pszUnifiedRelativePathName))
		{
			STXTRACELOGE(_T("%s changed. this file was cached before. schedule reloading..."), pszUnifiedRelativePathName);
			std::for_each(itCache->second.begin(), itCache->second.end(), [](auto cache) {cache->SetNeedUpdate(TRUE); });
		}
		_mapScriptFileCaches.unlock(pszUnifiedRelativePathName);

		std::replace(strRelativePathName.begin(), strRelativePathName.end(), '/', '\\');
		pszUnifiedRelativePathName = strRelativePathName.c_str();
		_mapScriptFileCaches.lock(pszUnifiedRelativePathName);
		itCache = _mapScriptFileCaches.find(pszUnifiedRelativePathName);
		if (itCache != _mapScriptFileCaches.end(pszUnifiedRelativePathName))
		{
			STXTRACELOGE(_T("%s changed. this file was cached before. schedule reloading..."), pszUnifiedRelativePathName);
			std::for_each(itCache->second.begin(), itCache->second.end(), [](auto cache) {cache->SetNeedUpdate(TRUE); });
		}
		_mapScriptFileCaches.unlock(pszUnifiedRelativePathName);

	}
}

void CUniversalIOCPServer::OnServerFileChanged(DWORD dwAction, LPCTSTR lpszRelativePathName, LPCTSTR lpszFileFullPathName)
{
	ProcessInternalScriptFileChange(dwAction, lpszRelativePathName, lpszFileFullPathName);

	BOOL bSkipScript = FALSE;
	if(_pServer && _pServer->_callback)
		_pServer->_callback->OnServerFileChanged(dwAction, lpszRelativePathName, lpszFileFullPathName, &bSkipScript);
}

std::wstring CUniversalIOCPServer::GetFileContentText(LPCTSTR lpszFile, BOOL *pbResult)
{
	if (pbResult)
	{
		*pbResult = FALSE;
	}
	USES_CONVERSION;
	HANDLE file = INVALID_HANDLE_VALUE;
	file = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return _T("");

	LARGE_INTEGER size;
	BOOL bSuccess = GetFileSizeEx(file, &size);

	std::vector<unsigned char> vData;
	const int bufferSize = 16384 * 40;
	char *pszBuffer = new char[bufferSize];
	if (pszBuffer == NULL)
	{
		STXTRACELOGFE(_T("CUniversalIOCPServer::GetFileContentText: not enough memory."));
		if (pbResult)
		{
			*pbResult = TRUE;
		}
		return _T("");
	}
	DWORD dwRead = 0;
	while (TRUE)
	{
		bSuccess = ReadFile(file, pszBuffer, (DWORD)bufferSize, &dwRead, NULL);
		if (!bSuccess || dwRead == 0)
			break;

		std::copy(pszBuffer, pszBuffer + dwRead, std::back_inserter(vData));
	}

	delete[]pszBuffer;

	vData.push_back('\0');
	vData.push_back('\0');

	std::wstring strFileContent;

	CloseHandle(file);

	if (pbResult)
	{
		*pbResult = TRUE;
	}

	if (vData[0] == 0xFF && vData[1] == 0xFE)	//Unicode
	{
		strFileContent = (LPCWSTR)&vData[2];
		return strFileContent;
	}
	else if (vData.size() >= 3)
	{
		//EF、BB、BF
		if (vData[0] == 0xEF && vData[1] == 0xBB && vData[2] == 0xBF)	//UTF-8
		{
			//Todo: Convert UTF-8 to Unicode
			strFileContent = A2W_CP((LPCSTR)&vData[2], CP_UTF8);
			return strFileContent;
		}
		else
		{
			//Todo: Convert ANSI to Unicode
			strFileContent = A2W((LPCSTR)&vData[0]);
			return strFileContent;
		}
	}
	else
	{
		//Todo: Convert ANSI to Unicode
		strFileContent = A2W((LPCSTR)&vData[0]);
		return strFileContent;
	}

	return strFileContent;
}

LPVOID CUniversalIOCPServer::OnAllocateWorkerThreadLocalStorage()
{
	CUniversalServerWorkerThreadData *pData = new CUniversalServerWorkerThreadData();
	memset(pData, 0, sizeof(CUniversalServerWorkerThreadData));

	pData->_pLuaState = luaL_newstate(); /* create state */
	return pData; 
}

void CUniversalIOCPServer::OnFreeWorkerThreadLocalStorage(LPVOID pStoragePtr)
{
	CUniversalServerWorkerThreadData *pData = (CUniversalServerWorkerThreadData*)pStoragePtr;
	lua_close(pData->_pLuaState);

	delete pData;
}

DWORD CUniversalIOCPServer::OnQueryWorkerThreadCount()
{
	DWORD dwWorkerThreadCount = __super::OnQueryWorkerThreadCount();
	_workerThreadData.resize(dwWorkerThreadCount);
	return dwWorkerThreadCount;
}

LPCTSTR CUniversalIOCPServer::OnGetUserDefinedExceptionName(DWORD dwExceptionCode)
{
	if (dwExceptionCode == 0x20000001)
	{
		return _T("Hanananana~~~");
	}

	return NULL;
}

DWORD CUniversalIOCPServer::OnParseUserDefinedExceptionArgument(DWORD dwExceptionCode, DWORD nArguments, ULONG_PTR *pArgumentArray, LPTSTR lpszBuffer, UINT cchBufferSize)
{
	if (dwExceptionCode == 0x20000001)
	{
		if (lpszBuffer != NULL)
		{
			_stprintf_s(lpszBuffer, cchBufferSize, _T("%d arguments!\r\n%s\r\n%s"), nArguments, (LPCTSTR)pArgumentArray[0], (LPCTSTR)pArgumentArray[1]);
		}
		return 100;
	}

	return 0;
}

void CUniversalIOCPServer::OnOutputDebugString(LPCTSTR lpszContent, BOOL bFinalCall)
{
	static std::wstring cacheString = _T("");
	if (!bFinalCall)
	{
		cacheString += lpszContent;
	}
	else
	{
		if (_debugMonitorCount > 0)
		{
			SendRawResponseDataToDebugMonitors((LPVOID)cacheString.c_str(), cacheString.size() * sizeof(TCHAR));
		}

		BOOL bSkipScript = FALSE;
		if (_pServer && _pServer->_callback)
			_pServer->_callback->OnOutputDebugString(lpszContent, &bSkipScript);

		cacheString.clear();
	}
}

void CUniversalIOCPServer::OnOutputLog(LPCTSTR lpszContent, BOOL bFinalCall)
{
	static std::wstring cacheString = _T("");
	if (!bFinalCall)
	{
		cacheString += lpszContent;
	}
	else
	{
		if (_logMonitorCount > 0)
		{
			SendRawResponseDataToLogMonitors((LPVOID)lpszContent, _tcslen(lpszContent) * sizeof(TCHAR));
		}

		BOOL bSkipScript = FALSE;
		if (_pServer && _pServer->_callback)
			_pServer->_callback->OnOutputLog(lpszContent, &bSkipScript);

		cacheString.clear();
	}
}


//////////////////////////////////////////////////////////////////////////

CUniversalIOCPTcpConnectionContext::CUniversalIOCPTcpConnectionContext()
{
	_uid = -1;
}

CUniversalIOCPTcpConnectionContext::~CUniversalIOCPTcpConnectionContext()
{

}

//////////////////////////////////////////////////////////////////////////

CUniversalIOCPServerClientContext::CUniversalIOCPServerClientContext() : m_reqParser(this)
{
	_webSocketProtocolMode = FALSE;
	_uid = -1;
	InitializeCriticalSection(&m_csHttpParser);
	_iStage = 0;
	_arrContent.reserve(256);	//reserve an initial space for performance optimization

	m_pWSRecvBuffer = NULL;
	m_cbWSBufferSize = 0;
	m_cbWSWriteOffset = 0;
	m_bLastWebSocketPacketFin = FALSE;

}

CUniversalIOCPServerClientContext::~CUniversalIOCPServerClientContext()
{
	DeleteCriticalSection(&m_csHttpParser);

	if (m_pWSRecvBuffer)
	{
		free(m_pWSRecvBuffer);
	}
}

void CUniversalIOCPServerClientContext::OnHttpRequestInternal(const char *pszHeader, void* pUserData)
{
	CUniversalIOCPServer *pServer = dynamic_cast<CUniversalIOCPServer*>(m_pServer);
	if (pServer)
	{
		pServer->OnHttpRequestInternal(pszHeader, pUserData);
	}
}

void CUniversalIOCPServerClientContext::OnHttpRequestError(const char *pszReason, void* pUserData)
{
	CUniversalIOCPServer *pServer = dynamic_cast<CUniversalIOCPServer*>(m_pServer);
	if (pServer)
	{
		pServer->OnHttpRequestError(pszReason, pUserData);
	}
}

BOOL CUniversalIOCPServerClientContext::AppendChar(char ch, void* pUserData, char *pCharLocation)
{
	//Parse each character until a double empty line encountered. (double empty line means HTTP header end)
	//HTTP header is stored in _arrContent

	switch (_iStage)
	{
	case 0:
	{
		if (ch >= 0x20 && ch <= 0x7E)
		{
			_iStage = 'C';
		}
		else if (ch < 0x20)
		{
			//CUniversalIOCPServer *pServer = dynamic_cast<CUniversalIOCPServer*>(m_pServer);
			//if (pServer)
			//{
			//	pServer->OnError("HTTP Request Header begins with an invalid character.", pUserData);
			//}
			return FALSE;
		}
	}
	break;
	case 'C':
	{
		if (ch >= 0x20 && ch <= 0x7E)
		{
			//Do Nothing
		}
		else if (ch == '\r')
		{
			_iStage = 'E';
		}
	}
	break;
	case 'E':
	{
		if (ch >= 0x20 && ch <= 0x7E)
		{
			_iStage = 'C';
		}
		else if (ch == '\r')
		{
			_iStage = 'F';
		}
	}
	break;
	case 'F':
	{
		if (ch >= 0x20 && ch <= 0x7E)
		{
			_arrContent.push_back(0);
			OnHttpMessageEnd(pUserData, pCharLocation + 1);
			_arrContent.clear();
			_iStage = 'C';
		}
		else if (ch == '\n')
		{
			_arrContent.push_back(ch);
			_arrContent.push_back(0);
			OnHttpMessageEnd(pUserData, pCharLocation + 1);
			_arrContent.clear();
			_iStage = 'C';
			return TRUE;
		}
		else if (ch == '\r')
		{
			_iStage = 'F';
		}
	}
	break;
	}
	_arrContent.push_back(ch);

	// 	if (_maxHeaderSize > 0 && _arrContent.size() > _maxHeaderSize)
	// 	{
	// 		OnError("HTTP Request Header is too long.", pUserData);
	// 		return FALSE;
	// 	}

	return TRUE;
}

void CUniversalIOCPServerClientContext::OnHttpMessageEnd(void* pUserData, const char *pHeaderEnd)
{
	const char *psz = (const char*)&_arrContent[0];
	if (psz == NULL || psz[0] == 0)
		return;

	CUniversalIOCPServer *pServer = dynamic_cast<CUniversalIOCPServer*>(m_pServer);
	if (pServer)
	{
		pServer->Parse(psz, pUserData, pHeaderEnd);
	}
}

void CUniversalIOCPServerClientContext::BeginWebSocketProtocolMode()
{
	_webSocketProtocolMode = TRUE;
}

void CUniversalIOCPServerClientContext::AppendWebSocketRecvData(LPVOID lpData, DWORD cbDataLen)
{
	UpdateLastDataReceiveTime();

	//由于每次处理收到的消息包完成之后才会再次调用IssueClientRead。此处不存在多线程同时进入此函数的情况
	LONG cbOldBufferSize = m_cbWSBufferSize;

	if (m_pWSRecvBuffer == NULL)
	{
		m_cbWSBufferSize = cbDataLen * 2;
	}
	else
	{
		while ((LONG)cbDataLen + m_cbWSWriteOffset > m_cbWSBufferSize)
			m_cbWSBufferSize *= 2;
	}

	if (m_cbWSBufferSize > cbOldBufferSize)
	{
		if(m_pWSRecvBuffer == NULL)
			m_pWSRecvBuffer = (char*)malloc(m_cbWSBufferSize);
		else
			m_pWSRecvBuffer = (char*)realloc(m_pWSRecvBuffer, m_cbWSBufferSize);
	}

	memcpy(m_pWSRecvBuffer + m_cbWSWriteOffset, lpData, cbDataLen);
	m_cbWSWriteOffset += cbDataLen;
}

void CUniversalIOCPServerClientContext::SkipWebSocketRecvBuffer(LONG cbSkip)
{
	//由于每次处理收到的消息包完成之后才会再次调用IssueClientRead。此处不存在多线程同时进入此函数的情况
	memmove(m_pWSRecvBuffer, m_pWSRecvBuffer + cbSkip, m_cbWSWriteOffset - cbSkip);
	m_cbWSWriteOffset -= cbSkip;
}

DWORD CUniversalIOCPServerClientContext::GetBufferedWebSocketMessageLength()
{
	return m_cbWSWriteOffset;
}

void CUniversalIOCPServerClientContext::SetLastWebSocketPackageFin(BOOL bFin)
{
	m_bLastWebSocketPacketFin = bFin;
}

LPVOID CUniversalIOCPServerClientContext::GetWebSocketMessageBasePtr()
{
	return m_pWSRecvBuffer;
}

UniversalTcpClientRole CUniversalIOCPServerClientContext::SetRole(UniversalTcpClientRole newRole)
{
	UniversalTcpClientRole oldRole = _role;
	_role = newRole;
	return oldRole;
}

UniversalTcpClientRole CUniversalIOCPServerClientContext::GetRole()
{
	return _role;
}

