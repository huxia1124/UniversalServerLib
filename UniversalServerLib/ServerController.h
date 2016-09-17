#pragma once

#include <thread>
#include <future>
#include <concurrent_unordered_map.h>
#include "STXProtocolLua.h"


class CUniversalIOCPServer;
class CUniversalServer;

class CServerController
{
public:
	CServerController(std::string name);
	virtual ~CServerController();

public:
	CUniversalIOCPServer *_server;
	CUniversalServer *_pServer;

	std::future<int> _future;
	concurrency::concurrent_unordered_map<std::wstring, std::wstring> _datamap;

public:
	void StartServer(int nDefaultTcpTimeout, int nInitialBufferCount, int nMaxBufferCount, int nBufferSize);
	void Terminate();
	void BeginTcpStreamServer(int nPort, std::wstring userParam, int nAcceptPost, long long nLimitClientCount);
	void BeginHttpServer(int nPort, std::wstring userParam, int nAcceptPost, long long nLimitClientCount);
	void BeginTcpServer2(int nPort, std::wstring userParam, int nAcceptPost, long long nLimitClientCount);
	void BeginTcpServer4(int nPort, std::wstring userParam, int nAcceptPost, long long nLimitClientCount);
	void BeginTcpServerV(int nPort, std::wstring userParam, int nAcceptPost, long long nLimitClientCount);
	long CreateTcpStreamConnection(std::wstring address, int nPort, std::wstring userParam);
	long PendingTcpStreamConnection(std::wstring address, int nPort, std::wstring userParam);
	long PendingTcpConnectionV(std::wstring address, int nPort, std::wstring userParam);
	void SendStringToConnection(long nConnectionID, std::string data);
	void SendStringToClient(__int64 nClientUID, std::string data);
	void SendWebSocketStringToClient(__int64 nClientUID, std::string data);
	void SendPackageToClient(__int64 nClientUID, std::shared_ptr<CSTXProtocolLua> spData);
	void SendPackageToConnection(long nConnectionID, std::shared_ptr<CSTXProtocolLua> spData);
	void DisconnectClient(__int64 nClientUID);
	void SetTcpClientRole(__int64 nClientUID, int nRole);
	void SetTcpClientTimeout(__int64 nClientUID, unsigned int nTimeout);
	void SetRPCPort(unsigned int nPort);
	void StartRPC();
	void StopRPC();
	void PutString(std::wstring key, std::wstring value);
	std::wstring GetString(std::wstring key);
	void Log(std::wstring logText, int nLogLevel);

};

