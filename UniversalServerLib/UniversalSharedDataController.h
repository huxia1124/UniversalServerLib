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
#include "STXServerBase.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaIntf/LuaIntf.h"
#include <memory>
#include <atlexcept.h>
#include <atlconv.h>

namespace LuaIntf
{
	LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}

//////////////////////////////////////////////////////////////////////////
// CSTXMemoryVariableValueIterator

template<class ElemType>
void pushValueToStack(lua_State* L, ElemType value);


template<class ElemType>
class CSTXMemoryVariableValueIterator : public LuaIntf::CppFunctor
{
public:
	CSTXMemoryVariableValueIterator(std::vector<ElemType> &&values, int elemType)
	{
		_values = std::move(values);
		_elemType = elemType;
	}

protected:
	std::vector<ElemType> _values;
	int _elemType = -1;
	size_t _index = 0;

public:
	virtual int run(lua_State* L) override
	{
		USES_CONVERSION;
		if (_index < _values.size())
		{
			pushValueToStack(L, _values[_index]);
			//lua_pushstring(L, (LPCSTR)ATL::CW2A(_values[_index].c_str()));
			_index++;
			return 1;
		}
		else
		{
			return 0;
		}
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// CUniversalSharedDataTree
// Cross-thread tree storage for multiple types of data

class CUniversalSharedDataTree : public LuaIntf::CppFunctor
{
public:
	CUniversalSharedDataTree(std::string dummyStr)
	{
		_rootNode = _s_rootNode;
	}
	CUniversalSharedDataTree(std::string dummyStr, std::shared_ptr<CSTXMemoryVariableNode> rootNode)
	{
		_rootNode = rootNode;
	}
	CUniversalSharedDataTree(std::shared_ptr<CSTXMemoryVariableNode> rootNode)
	{
		_rootNode = rootNode;
		_rootNode->GetChildren(&_childNodes);
	}
	virtual ~CUniversalSharedDataTree()
	{
		
	}
protected:
	size_t _index = 0;
	std::shared_ptr<CSTXMemoryVariableNode> _rootNode;
	std::vector<std::shared_ptr<CSTXMemoryVariableNode>> _childNodes;

	static std::shared_ptr<CSTXMemoryVariableNode> _s_rootNode;

public:

	virtual int run(lua_State* L) override
	{
		if (_index < _childNodes.size())
		{
			LuaIntf::Lua::pushNew<CUniversalSharedDataTree>(L, "", _childNodes[_index]);
			_index++;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	//Get child nodes
	int GetNodes(lua_State* L)
	{
		return LuaIntf::CppFunctor::make<CUniversalSharedDataTree>(L, _rootNode); // ... is constructor auguments
	}
	int GetValues(lua_State* L)
	{
		std::vector<std::wstring> values;
		_rootNode->GetThisValues(&values);
		return LuaIntf::CppFunctor::make<CSTXMemoryVariableValueIterator<std::wstring>>(L, std::move(values), _rootNode->GetThisVariableType()); // ... is constructor auguments
	}

	int GetIntegerValues(lua_State* L)
	{
		std::vector<int32_t> values;
		_rootNode->GetThisValues(&values);
		return LuaIntf::CppFunctor::make<CSTXMemoryVariableValueIterator<int32_t>>(L, std::move(values), _rootNode->GetThisVariableType()); // ... is constructor auguments
	}

public:
	std::wstring GetName();
	std::wstring GetFullPath();
	void RegisterStringVectorVariable(std::wstring strPathName);
	void RegisterStringSetVariable(std::wstring strPathName);
	void RegisterIntegerVariable(std::wstring strPathName);
	void RegisterDoubleVariable(std::wstring strPathName);

	void AddStringValue(std::wstring strPathName, std::wstring value);

	std::wstring GetStringValue(std::wstring strPathName);
	void SetStringValue(std::wstring strPathName, std::wstring value);

	int32_t GetIntegerValue(std::wstring strPathName);
	void SetIntegerValue(std::wstring strPathName, int32_t value);

	double GetDoubleValue(std::wstring strPathName);
	void SetDoubleValue(std::wstring strPathName, double value);

	std::shared_ptr<CUniversalSharedDataTree> GetNode(std::wstring strPathName);
};

//////////////////////////////////////////////////////////////////////////
// CUniversalSharedDataController
// Simple cross-thread key-value storage for strings

class CUniversalSharedDataController
{
public:
	CUniversalSharedDataController(std::string);
	~CUniversalSharedDataController();

protected:
	static concurrency::concurrent_unordered_map<std::wstring, std::wstring> _datamap;
	//static concurrency::concurrent_unordered_map<std::wstring, concurrency::concurrent_unordered_map<std::wstring, std::wstring>> _datamap2;

public:
	static void PutString(std::wstring key, std::wstring value);
	static std::wstring GetString(std::wstring key);

};

