#pragma once

#include <string>
#include <wtypes.h>
#include <stdio.h>
#include <vector>

class CSTXProtocolHistory
{
public:
	SYSTEMTIME _time;
	std::wstring _target;
	std::vector<std::vector<std::wstring>> _items;

public:
	CSTXProtocolHistory();
	CSTXProtocolHistory(const CSTXProtocolHistory& val);

public:
	void WriteToFile(FILE*file);
	void ReadFromFile(FILE*file);

	CSTXProtocolHistory& operator =(const CSTXProtocolHistory& val);

};

