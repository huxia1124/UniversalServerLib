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
#include <tchar.h>
#include "UniversalSharedDataController.h"

//////////////////////////////////////////////////////////////////////////

template<>
void pushValueToStack<std::wstring>(lua_State* L, std::wstring value)
{
	lua_pushstring(L, (LPCSTR)ATL::CW2A(value.c_str()));
}

template<>
void pushValueToStack<int64_t>(lua_State* L, int64_t value)
{
	lua_pushinteger(L, value);
}

template<>
void pushValueToStack<double>(lua_State* L, double value)
{
	lua_pushnumber(L, value);
}
//////////////////////////////////////////////////////////////////////////

concurrency::concurrent_unordered_map<std::wstring, std::wstring> CUniversalSharedDataController::_datamap;
std::shared_ptr<CSTXMemoryVariableNode> CUniversalSharedDataTree::_s_rootNode = std::make_shared<CSTXMemoryVariableNode>(_T("<root>"));

std::wstring CUniversalSharedDataTree::GetName()
{
	return _rootNode->GetName();
}

std::wstring CUniversalSharedDataTree::GetFullPath()
{
	return _rootNode->GetFullPath();
}

size_t CUniversalSharedDataTree::GetChildrenCount(std::wstring strPathName)
{
	if (strPathName.size() == 0)
		return _rootNode->GetThisChildrenCount();
		
	return _rootNode->GetChildrenCount(strPathName);
}

void CUniversalSharedDataTree::RegisterStringVariable(std::wstring strPathName)
{
	_rootNode->RegisterStringVariable(strPathName, _T(""));
}

void CUniversalSharedDataTree::RegisterStringVectorVariable(std::wstring strPathName)
{
	_rootNode->RegisterStringVectorVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterStringSetVariable(std::wstring strPathName)
{
	_rootNode->RegisterStringSetVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterIntegerVectorVariable(std::wstring strPathName)
{
	_rootNode->RegisterIntegerVectorVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterIntegerSetVariable(std::wstring strPathName)
{
	_rootNode->RegisterIntegerSetVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterDoubleVectorVariable(std::wstring strPathName)
{
	_rootNode->RegisterDoubleVectorVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterDoubleSetVariable(std::wstring strPathName)
{
	_rootNode->RegisterDoubleSetVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterIntegerVariable(std::wstring strPathName)
{
	_rootNode->RegisterIntegerVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterDoubleVariable(std::wstring strPathName)
{
	_rootNode->RegisterDoubleVariable(strPathName);
}

void CUniversalSharedDataTree::AddStringValue(std::wstring strPathName, std::wstring value)
{
	_rootNode->AddStringValue(strPathName, value);
}

void CUniversalSharedDataTree::AddIntegerValue(std::wstring strPathName, int64_t value)
{
	_rootNode->AddIntegerValue(strPathName, value);
}

void CUniversalSharedDataTree::AddDoubleValue(std::wstring strPathName, double value)
{
	_rootNode->AddDoubleValue(strPathName, value);
}

void CUniversalSharedDataTree::RemoveStringValue(std::wstring strPathName, std::wstring value)
{
	if (strPathName.size() == 0)
		return _rootNode->RemoveThisStringValue(value);

	return _rootNode->RemoveStringValue(strPathName, value);
}

void CUniversalSharedDataTree::RemoveIntegerValue(std::wstring strPathName, int64_t value)
{
	if (strPathName.size() == 0)
		return _rootNode->RemoveThisIntegerValue(value);

	return _rootNode->RemoveIntegerValue(strPathName, value);
}

std::wstring CUniversalSharedDataTree::GetStringValue(std::wstring strPathName)
{
	if (strPathName.size() == 0)
		return _rootNode->GetThisStringValue();

	return _rootNode->GetStringValue(strPathName);
}

void CUniversalSharedDataTree::SetStringValue(std::wstring strPathName, std::wstring value)
{
	if (strPathName.size() == 0)
		return _rootNode->SetThisStringValue(value);

	_rootNode->SetStringValue(strPathName, value);
}

int64_t CUniversalSharedDataTree::GetIntegerValue(std::wstring strPathName)
{
	if (strPathName.size() == 0)
		return _rootNode->GetThisIntegerValue();

	return _rootNode->GetIntegerValue(strPathName);
}

void CUniversalSharedDataTree::SetIntegerValue(std::wstring strPathName, int64_t value)
{
	if (strPathName.size() == 0)
		return _rootNode->SetThisIntegerValue(value);

	_rootNode->SetIntegerValue(strPathName, value);
}

double CUniversalSharedDataTree::GetDoubleValue(std::wstring strPathName)
{
	return _rootNode->GetDoubleValue(strPathName);
}

void CUniversalSharedDataTree::SetDoubleValue(std::wstring strPathName, double value)
{
	_rootNode->SetDoubleValue(strPathName, value);
}

std::shared_ptr<CUniversalSharedDataTree> CUniversalSharedDataTree::GetNode(std::wstring strPathName)
{
	auto node = _rootNode->GetVariableNode(strPathName);
	if (node)
	{
		return std::make_shared<CUniversalSharedDataTree>("", node);
	}
	return nullptr;
}

void CUniversalSharedDataTree::UnregisterVariable(std::wstring strPathName)
{
	_rootNode->UnregisterVariable(strPathName);
}

int64_t CUniversalSharedDataTree::IncreaseIntegerValue(std::wstring strPathName, int64_t delta)
{
	if (strPathName.size() == 0)
		return _rootNode->IncreaseThisIntegerValue(delta);

	return _rootNode->IncreaseIntegerValue(strPathName, delta);
}

bool CUniversalSharedDataTree::IsValueExists(std::wstring strPathName)
{
	if (strPathName.size() == 0)
		return _rootNode->IsThisValueExists();

	return _rootNode->IsValueExists(strPathName);

}

bool CUniversalSharedDataTree::IsContainStringValue(std::wstring strPathName, std::wstring value)
{
	if (strPathName.size() == 0)
		return _rootNode->IsContainThisStringValue(value);

	return _rootNode->IsContainStringValue(strPathName, value);
}

bool CUniversalSharedDataTree::IsContainIntegerValue(std::wstring strPathName, int64_t value)
{
	if (strPathName.size() == 0)
		return _rootNode->IsContainThisIntegerValue(value);

	return _rootNode->IsContainIntegerValue(strPathName, value);
}

std::shared_ptr<CSTXMemoryVariableNode> CUniversalSharedDataTree::GetRootNode()
{
	return _s_rootNode;
}

CUniversalSharedDataController::CUniversalSharedDataController(std::string)
{
}


CUniversalSharedDataController::~CUniversalSharedDataController()
{
}

void CUniversalSharedDataController::PutString(std::wstring key, std::wstring value)
{
	_datamap[key] = value;
}

std::wstring CUniversalSharedDataController::GetString(std::wstring key)
{
	auto it = _datamap.find(key);
	if (it == _datamap.end())
		return _T("");

	return it->second;
}

