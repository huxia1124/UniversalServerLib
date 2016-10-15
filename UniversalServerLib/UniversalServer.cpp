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
#include <Shlwapi.h>
#include "UniversalServer.h"
#include "UniversalIOCPServer.h"
#include "ServerController.h"
#include "UniversalServerInternal.h"
#include <string>
#include <codecvt>
#include "UniversalSharedDataController.h"
#include <atlexcept.h>
#include <atlconv.h>

extern "C"
{
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#define USE_INTERNAL_FPCONV
#define ENABLE_CJSON_GLOBAL
#include "extensions/luacjson/lua_cjson.h"

#include "extensions/lsqlite3/lsqlite3.c"
}


#include "LuaIntf/LuaIntf.h"


namespace LuaIntf
{

	template <>
	struct LuaTypeMapping <std::wstring>
	{
		static void push(lua_State* L, const std::wstring& str)
		{
			if (str.empty()) {
				lua_pushliteral(L, "");
			}
			else {
				std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
				std::string buf = conv.to_bytes(str);
				lua_pushlstring(L, buf.data(), buf.length());
			}
		}

		static std::wstring get(lua_State* L, int index)
		{
			size_t len;
			const char* p = luaL_checklstring(L, index, &len);
			//std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
			//return conv.from_bytes(p, p + len);

			USES_CONVERSION;
			return (LPCTSTR)ATL::CA2W(p, CP_UTF8);
		}

		static std::wstring opt(lua_State* L, int index, const std::wstring& def)
		{
			return lua_isnoneornil(L, index) ? def : get(L, index);
		}
	};

} // namespace LuaIntf


IUniversalServer *CreateUniversalServer()
{
	CUniversalServer *pServer = (new CUniversalServer());
	pServer->Initialize();

	//utils.GetServer
	LuaIntf::LuaBinding(pServer->L).beginModule("utils").addFunction("GetServer", [&] {
		std::shared_ptr<CServerController> sp = pServer->_spController;
		return sp;
	}).endModule();

	pServer->BindClasses();


	return pServer;
}

int setLuaPath(lua_State* L, const char* path)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
	std::string cur_path = lua_tostring(L, -1); // grab path string from top of stack
	cur_path.append(";"); // do your path magic here
	cur_path.append(path);
	lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
	lua_pushstring(L, cur_path.c_str()); // push the new one
	lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
	lua_pop(L, 1); // get rid of package table from top of stack
	return 0; // all done!
}

void LuaBindClasses(lua_State *pLuaState, CUniversalServer *pServer)
{
	CHAR szPath[MAX_PATH];
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	LPSTR lpszName = PathFindFileNameA(szPath);
	*lpszName = 0;

	CHAR szPathToSet[MAX_PATH];
	//strcpy_s(szPathToSet, szPath);
	//strcat_s(szPathToSet, "?.lua");
	//setLuaPath(pLuaState, szPathToSet);

	strcpy_s(szPathToSet, szPath);
	strcat_s(szPathToSet, "scripts\\?.lua");
	setLuaPath(pLuaState, szPathToSet);

	LuaIntf::LuaBinding(pLuaState).beginClass<CServerController>("UniversalServer")
		//Constructor
		.addConstructor(LUA_SP(std::shared_ptr<CServerController>), LUA_ARGS(LuaIntf::_opt<std::string>))

		//Functions
			//.addStaticProperty("home_url", &Web::home_url, &Web::set_home_url)
			//.addStaticFunction("GetServer", &CUniversalIOCPServer::GetServer)
			//.addProperty("url", &Web::url, &Web::set_url)
		.addFunction("StartServer", &CServerController::StartServer, LUA_ARGS(LuaIntf::_def<int, 0>, LuaIntf::_def<int, 1000>, LuaIntf::_def<int, 5000>, LuaIntf::_def<int, 8192>))
		.addFunction("Terminate", &CServerController::Terminate)
		.addFunction("BeginTcpStreamServer", &CServerController::BeginTcpStreamServer, LUA_ARGS(int, LuaIntf::_opt<std::wstring>, LuaIntf::_def<int, 0>, LuaIntf::_def<long long, 0>))
		.addFunction("BeginTcpServer2", &CServerController::BeginTcpServer2, LUA_ARGS(int, LuaIntf::_opt<std::wstring>, LuaIntf::_def<int, 0>, LuaIntf::_def<long long, 0>))
		.addFunction("BeginTcpServer4", &CServerController::BeginTcpServer4, LUA_ARGS(int, LuaIntf::_opt<std::wstring>, LuaIntf::_def<int, 0>, LuaIntf::_def<long long, 0>))
		.addFunction("BeginTcpServerV", &CServerController::BeginTcpServerV, LUA_ARGS(int, LuaIntf::_opt<std::wstring>, LuaIntf::_def<int, 0>, LuaIntf::_def<long long, 0>))
		.addFunction("BeginHttpServer", &CServerController::BeginHttpServer, LUA_ARGS(int, LuaIntf::_opt<std::wstring>, LuaIntf::_def<int, 0>, LuaIntf::_def<long long, 0>))
		.addFunction("DestroyTcpServer", &CServerController::DestroyTcpServer, LUA_ARGS(int))
		.addFunction("CreateTcpStreamConnection", &CServerController::CreateTcpStreamConnection, LUA_ARGS(std::wstring, int, LuaIntf::_opt<std::wstring>))
		.addFunction("PendingTcpStreamConnection", &CServerController::PendingTcpStreamConnection, LUA_ARGS(std::wstring, int, LuaIntf::_opt<std::wstring>))
		.addFunction("PendingTcpConnectionV", &CServerController::PendingTcpConnectionV, LUA_ARGS(std::wstring, int, LuaIntf::_opt<std::wstring>))
		.addFunction("SendStringToClient", &CServerController::SendStringToClient, LUA_ARGS(__int64, std::string))
		.addFunction("SendWebSocketStringToClient", &CServerController::SendWebSocketStringToClient, LUA_ARGS(__int64, std::string))
		.addFunction("SendStringToConnection", &CServerController::SendStringToConnection, LUA_ARGS(long, std::string))
		.addFunction("SendPackageToClient", &CServerController::SendPackageToClient, LUA_ARGS(__int64, std::shared_ptr<CSTXProtocolLua>))
		.addFunction("SendPackageToConnection", &CServerController::SendPackageToConnection, LUA_ARGS(long, std::shared_ptr<CSTXProtocolLua>))
		.addFunction("PutString", &CServerController::PutString, LUA_ARGS(std::wstring, std::wstring))
		.addFunction("GetString", &CServerController::GetString, LUA_ARGS(std::wstring))
		.addFunction("Log", &CServerController::Log, LUA_ARGS(std::wstring, LuaIntf::_def<int, 3>))
		.addFunction("DisconnectClient", &CServerController::DisconnectClient, LUA_ARGS(__int64))
		.addFunction("SetTcpClientRole", &CServerController::SetTcpClientRole, LUA_ARGS(__int64, int))
		.addFunction("SetTcpClientTimeout", &CServerController::SetTcpClientTimeout, LUA_ARGS(__int64, unsigned int))
		.addFunction("SetRPCPort", &CServerController::SetRPCPort, LUA_ARGS(unsigned int))
		.addFunction("StartRPC", &CServerController::StartRPC, LUA_ARGS(LuaIntf::_def<int, -1>))
		.addFunction("StopRPC", &CServerController::StopRPC)
		.addFunction("SetTcpServerReceivedScript", &CServerController::SetTcpServerReceivedScript, LUA_ARGS(int, LuaIntf::_opt<std::wstring>))
		.addFunction("SetTcpConnectionReceivedScript", &CServerController::SetTcpConnectionReceivedScript, LUA_ARGS(long, LuaIntf::_opt<std::wstring>))
		.addFunction("SetTcpConnectionDisconnectedScript", &CServerController::SetTcpConnectionDisconnectedScript, LUA_ARGS(long, LuaIntf::_opt<std::wstring>))
		.addFunction("SetTcpServerClientConnectedScript", &CServerController::SetTcpServerClientConnectedScript, LUA_ARGS(int, LuaIntf::_opt<std::wstring>))
		.addFunction("SetTcpServerClientDisconnectedScript", &CServerController::SetTcpServerClientDisconnectedScript, LUA_ARGS(int, LuaIntf::_opt<std::wstring>))
		.addFunction("GetTcpClientCount", &CServerController::GetTcpClientCount, LUA_ARGS(LuaIntf::_def<int, 0>))
		.addFunction("GetSentBytesPerSecond", &CServerController::GetSentBytesPerSecond)
		.addFunction("GetSentCountPerSecond", &CServerController::GetSentCountPerSecond)
		.addFunction("GetReceivedBytesPerSecond", &CServerController::GetReceivedBytesPerSecond)
		.addFunction("GetReceivedCountPerSecond", &CServerController::GetReceivedCountPerSecond)
		.addFunction("SetStatisticsLevel", &CServerController::SetStatisticsLevel, LUA_ARGS(LuaIntf::_def<unsigned int, 1>))
		.addFunction("GetStatisticsLevel", &CServerController::GetStatisticsLevel)
		.addFunction("GetTotalReceivedBytes", &CServerController::GetTotalReceivedBytes)
		.addFunction("GetTotalSentBytes", &CServerController::GetTotalSentBytes)
		.addFunction("GetTotalReceivedCount", &CServerController::GetTotalReceivedCount)
		.addFunction("GetTotalSentCount", &CServerController::GetTotalSentCount)
		.addFunction("SetLogLevel", &CServerController::SetLogLevel, LUA_ARGS(int))
		.addFunction("SetDebugOutputLevel", &CServerController::SetDebugOutputLevel, LUA_ARGS(int))
		.addFunction("AddFolderMonitorIgnoreFileExtension", &CServerController::AddFolderMonitorIgnoreFileExtension, LUA_ARGS(long long, std::wstring))
		.addFunction("RemoveFolderMonitorIgnoreFileExtension", &CServerController::RemoveFolderMonitorIgnoreFileExtension, LUA_ARGS(long long, std::wstring))
		.addFunction("GetDefaultFolderMonitorId", &CServerController::GetDefaultFolderMonitorId)
		.addFunction("SetTimerScript", &CServerController::SetTimerScript, LUA_ARGS(std::wstring))
		.addFunction("SetWorkerThreadInitializationScript", &CServerController::SetWorkerThreadInitializationScript, LUA_ARGS(std::wstring))
		.addFunction("SetTimerInterval", &CServerController::ChangeTimerInterval, LUA_ARGS(unsigned int))

		//Properties
		.addPropertyReadOnly("DefaultFolderMonitorId", &CServerController::GetDefaultFolderMonitorId, &CServerController::GetDefaultFolderMonitorId)
		.addPropertyReadOnly("SentBytesPerSecond", &CServerController::GetSentBytesPerSecond, &CServerController::GetSentBytesPerSecond)
		.addPropertyReadOnly("SentCountPerSecond", &CServerController::GetSentCountPerSecond, &CServerController::GetSentCountPerSecond)
		.addPropertyReadOnly("ReceivedBytesPerSecond", &CServerController::GetReceivedBytesPerSecond, &CServerController::GetReceivedBytesPerSecond)
		.addPropertyReadOnly("ReceivedCountPerSecond", &CServerController::GetReceivedCountPerSecond, &CServerController::GetReceivedCountPerSecond)
		.addPropertyReadOnly("TotalReceivedBytes", &CServerController::GetTotalReceivedBytes, &CServerController::GetTotalReceivedBytes)
		.addPropertyReadOnly("TotalSentBytes", &CServerController::GetTotalSentBytes, &CServerController::GetTotalSentBytes)
		.addPropertyReadOnly("TotalReceivedCount", &CServerController::GetTotalReceivedCount, &CServerController::GetTotalReceivedCount)
		.addPropertyReadOnly("TotalSentCount", &CServerController::GetTotalSentCount, &CServerController::GetTotalSentCount)
		.addProperty("StatisticsLevel", &CServerController::GetStatisticsLevel, &CServerController::GetStatisticsLevel, &CServerController::SetStatisticsLevel)
		.addProperty("TimerInterval", &CServerController::GetTimerInterval, &CServerController::GetTimerInterval, &CServerController::SetTimerInterval)
		//*/
		//.addFunction("Terminate", &CUniversalIOCPServer::Initialize)
		//.addFunction("load", &Web::load, LUA_ARGS(LuaIntf::_opt<std::string>, LuaIntf::_opt<int>))
		//.addFunction("loadw", &Web::loadw, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		//.addStaticFunction("lambda", [] {
		// you can use C++11 lambda expression here too
		//return "yes";
		//	}
		.endClass()
		.beginClass<CUniversalSharedDataController>("_data")
		.addConstructor(LUA_SP(std::shared_ptr<CUniversalSharedDataController>), LUA_ARGS(LuaIntf::_opt<std::string>))
		.addStaticFunction("get", &CUniversalSharedDataController::GetString, LUA_ARGS(std::wstring))
		.addStaticFunction("put", &CUniversalSharedDataController::PutString, LUA_ARGS(std::wstring, std::wstring))
		.endClass();


	auto pfnGetCurrentThreadId = [] {
		return GetCurrentThreadId();
	};

	LuaIntf::LuaBinding(pLuaState).beginModule("utils").addFunction("Sleep", [] (int dwTimeMS) {
			Sleep(dwTimeMS);
		}, LUA_ARGS(int))
		.addFunction("GetThreadId", pfnGetCurrentThreadId)
		.addFunction("SetLogLevel", [&](int nLogLevel) {
			g_LogGlobal.SetLogLevel(nLogLevel, nLogLevel);
		}).endModule();

		//LuaIntf::LuaBinding(pLuaState).beginModule("_G").addFunction("TestG", [](int dwTimeMS) {
		//	// you can use C++11 lambda expression here too
		//	Sleep(dwTimeMS);
		//}, LUA_ARGS(int));

	LuaIntf::LuaBinding(pLuaState).beginClass<CUniversalSharedDataTree>("SharedData")
		.addConstructor(LUA_SP(std::shared_ptr<CUniversalSharedDataTree>), LUA_ARGS(LuaIntf::_opt<std::string>))
		.addFunction("GetNodes", &CUniversalSharedDataTree::GetNodes)
		.addFunction("GetValues", &CUniversalSharedDataTree::GetValues)
		.addFunction("GetStringValues", &CUniversalSharedDataTree::GetValues)
		.addFunction("GetIntegerValues", &CUniversalSharedDataTree::GetIntegerValues)
		.addFunction("GetDoubleValues", &CUniversalSharedDataTree::GetDoubleValues)
		.addFunction("RegisterStringVectorVariable", &CUniversalSharedDataTree::RegisterStringVectorVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterStringSetVariable", &CUniversalSharedDataTree::RegisterStringSetVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterIntegerVectorVariable", &CUniversalSharedDataTree::RegisterIntegerVectorVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterIntegerSetVariable", &CUniversalSharedDataTree::RegisterIntegerSetVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterDoubleVectorVariable", &CUniversalSharedDataTree::RegisterDoubleVectorVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterDoubleSetVariable", &CUniversalSharedDataTree::RegisterDoubleSetVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterIntegerVariable", &CUniversalSharedDataTree::RegisterIntegerVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("RegisterDoubleVariable", &CUniversalSharedDataTree::RegisterDoubleVariable, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("AddStringValue", &CUniversalSharedDataTree::AddStringValue, LUA_ARGS(std::wstring, std::wstring))
		.addFunction("AddIntegerValue", &CUniversalSharedDataTree::AddIntegerValue, LUA_ARGS(std::wstring, int64_t))
		.addFunction("AddDoubleValue", &CUniversalSharedDataTree::AddDoubleValue, LUA_ARGS(std::wstring, double))
		.addFunction("GetStringValue", &CUniversalSharedDataTree::GetStringValue, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("SetStringValue", &CUniversalSharedDataTree::SetStringValue, LUA_ARGS(std::wstring, std::wstring))
		.addFunction("GetIntegerValue", &CUniversalSharedDataTree::GetIntegerValue, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("SetIntegerValue", &CUniversalSharedDataTree::SetIntegerValue, LUA_ARGS(std::wstring, int64_t))
		.addFunction("GetDoubleValue", &CUniversalSharedDataTree::GetDoubleValue, LUA_ARGS(LuaIntf::_opt<std::wstring>))
		.addFunction("SetDoubleValue", &CUniversalSharedDataTree::SetDoubleValue, LUA_ARGS(std::wstring, double))
		.addFunction("GetNode", &CUniversalSharedDataTree::GetNode, LUA_ARGS(std::wstring))
		.addFunction("UnregisterVariable", &CUniversalSharedDataTree::UnregisterVariable, LUA_ARGS(std::wstring))
		.addFunction("IncreaseIntegerValue", &CUniversalSharedDataTree::IncreaseIntegerValue, LUA_ARGS(std::wstring, int64_t))
		.addFunction("RemoveStringValue", &CUniversalSharedDataTree::RemoveStringValue, LUA_ARGS(std::wstring, std::wstring))
		.addFunction("RemoveIntegerValue", &CUniversalSharedDataTree::RemoveIntegerValue, LUA_ARGS(std::wstring, int64_t))
		.addFunction("IsValueExists", &CUniversalSharedDataTree::IsValueExists, LUA_ARGS(std::wstring))
		.addFunction("IsContainStringValue", &CUniversalSharedDataTree::IsContainStringValue, LUA_ARGS(std::wstring, std::wstring))
		.addFunction("IsContainIntegerValue", &CUniversalSharedDataTree::IsContainIntegerValue, LUA_ARGS(std::wstring, int64_t))
		.addPropertyReadOnly("Name", &CUniversalSharedDataTree::GetName)
		.addPropertyReadOnly("FullPath", &CUniversalSharedDataTree::GetFullPath)
		.endClass();


	//Additional libraries
	luaopen_cjson(pLuaState);
	luaopen_lsqlite3(pLuaState);

	int n = luaL_dostring(pLuaState, "server=utils.GetServer()");

	//int n = luaL_dostring(pLuaState, "server=utils.GetServer()\r\n"
	//"SharedData.___class.__index = SharedData.___class.get\r\n"
	//	"SharedData.___class.__newindex = SharedData.___class.set");

}