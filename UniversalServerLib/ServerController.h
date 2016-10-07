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
	void SetTcpServerReceiveScript(int nPort, std::wstring scriptFile);
	void SetTcpConnectionReceiveScript(long nConnectionID, std::wstring scriptFile);
	void SetTcpServerClientConnectedScript(int nPort, std::wstring scriptFile);
	void SetTcpServerClientDisconnectedScript(int nPort, std::wstring scriptFile);

};

