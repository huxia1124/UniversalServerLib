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

CUniversalSharedDataController::CUniversalSharedDataController(std::string)
{
}


CUniversalSharedDataController::~CUniversalSharedDataController()
{
}

void CUniversalSharedDataController::PutString(std::wstring key, std::wstring value)
{
	_datamap.insert(std::pair<std::wstring, std::wstring>(key, value));

	//_datamap[key] = value;
}

std::wstring CUniversalSharedDataController::GetString(std::wstring key)
{
	auto it = _datamap.find(key);
	if (it == _datamap.end())
		return _T("");

	return it->second;
}