#pragma once

#include <string>
#include "UniversalIOCPServer.h"

class CSTXTcpClientLua;


//////////////////////////////////////////////////////////////////////////
// CSTXTcpClientLua

class CSTXTcpClientLua
{
	friend class CUniversalIOCPServer;
public:
	CSTXTcpClientLua(void) { _clientContext = NULL; };
	CSTXTcpClientLua(std::string s) { _clientContext = NULL; };
	virtual ~CSTXTcpClientLua(void) {};


protected:
	CUniversalIOCPServerClientContext *_clientContext;

public:
	std::string GetInternalName();

};