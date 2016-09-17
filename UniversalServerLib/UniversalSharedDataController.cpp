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