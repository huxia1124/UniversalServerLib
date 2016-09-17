#pragma once
#include <thread>
#include <future>
#include <concurrent_unordered_map.h>

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

