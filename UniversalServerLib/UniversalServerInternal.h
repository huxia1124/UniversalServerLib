#pragma once

#include <wtypes.h>
extern "C"
{
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}


#include "LuaIntf/LuaIntf.h"

#include "ServerController.h"
#include "UniversalServer.h"




class CUniversalServer : public IUniversalServer
{
protected:

public:
	lua_State *L;
	std::shared_ptr<CServerController> _spController;
	IUniversalServerCallback *_callback;
	CRITICAL_SECTION _csMainScript;
	ULONGLONG _tickStart;
	std::wstring _luaVersion;

public:
	CUniversalServer();
	virtual ~CUniversalServer();

protected:
	int ProcessException(LPEXCEPTION_POINTERS pExp);

	void CallFunctionImpl(LPCTSTR lpszFunctionName);

public:
	virtual int Run(LPCTSTR lpszScriptFile);
	virtual void CallFunction(LPCTSTR lpszFunctionName);
	virtual LPVOID GetMainLuaState();
	virtual int RunScriptString(LPCTSTR lpszScriptString);
	virtual void SetServerCallback(IUniversalServerCallback *pCallback);
	virtual ULONGLONG GetRunTime();
	virtual LPCTSTR GetLuaVersion();
	virtual void Terminate();

public:
	void Initialize();
	void BindClasses();

};

void LuaBindClasses(lua_State *pLuaState, CUniversalServer *pServer);
