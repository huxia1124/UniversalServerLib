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

void CUniversalSharedDataTree::RegisterStringVectorVariable(std::wstring strPathName)
{
	_rootNode->RegisterStringVectorVariable(strPathName);
}

void CUniversalSharedDataTree::RegisterStringSetVariable(std::wstring strPathName)
{
	_rootNode->RegisterStringSetVariable(strPathName);
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

std::wstring CUniversalSharedDataTree::GetStringValue(std::wstring strPathName)
{
	if (strPathName.size() == 0)
		return _rootNode->GetThisStringValue();

	return _rootNode->GetStringValue(strPathName);
}

void CUniversalSharedDataTree::SetStringValue(std::wstring strPathName, std::wstring value)
{
	_rootNode->SetStringValue(strPathName, value);
}

int32_t CUniversalSharedDataTree::GetIntegerValue(std::wstring strPathName)
{
	return _rootNode->GetIntegerValue(strPathName);
}

void CUniversalSharedDataTree::SetIntegerValue(std::wstring strPathName, int32_t value)
{
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

