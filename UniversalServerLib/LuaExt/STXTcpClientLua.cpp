#include "stdafx.h"
#include "STXTcpClientLua.h"
#include <atlexcept.h>
#include <atlconv.h>

std::string CSTXTcpClientLua::GetInternalName()
{
	return (LPCSTR)ATL::CW2A(_clientContext->GetClientIP());
}
