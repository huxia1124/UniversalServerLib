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
