#include "stdafx.h"

#include "STXProtocolHistory.h"


CSTXProtocolHistory::CSTXProtocolHistory()
{

}

CSTXProtocolHistory::CSTXProtocolHistory(const CSTXProtocolHistory& val)
{
	*this = val;
}

void CSTXProtocolHistory::WriteToFile(FILE * file)
{
	fwrite(&_time, sizeof(_time), 1, file);
	unsigned int nItems = (unsigned int)_items.size();
	fwrite(&nItems, sizeof(nItems), 1, file);
	for (unsigned int i = 0; i < nItems; i++)
	{
		unsigned int length = 0;
		for (int k = 0; k < 4; k++)
		{
			length = (unsigned int)(_items[i][k].size() + 1) * sizeof(WCHAR);
			fwrite(&length, sizeof(length), 1, file);
			fwrite(_items[i][k].c_str(), length, 1, file);
		}
	}

	unsigned int length = (unsigned int)(_target.size() + 1) * sizeof(WCHAR);
	fwrite(&length, sizeof(length), 1, file);
	fwrite(_target.c_str(), length, 1, file);
}

void CSTXProtocolHistory::ReadFromFile(FILE * file)
{
	fread(&_time, sizeof(_time), 1, file);
	unsigned int nItems = 0;
	fread(&nItems, sizeof(nItems), 1, file);

	for (unsigned int i = 0; i < nItems; i++)
	{
		_items.push_back(std::vector<std::wstring>());
		unsigned int length = 0;
		for (int k = 0; k < 4; k++)
		{
			length = 0;
			fread(&length, sizeof(length), 1, file);
			_items[i].push_back(std::wstring());
			_items[i][k].resize(length / sizeof(WCHAR));
			fread((void*)_items[i][k].c_str(), length, 1, file);
		}
	}

	unsigned int length = 0;
	fread(&length, sizeof(length), 1, file);
	_target.resize(length / sizeof(WCHAR));
	fread((void*)_target.c_str(), length, 1, file);
}

CSTXProtocolHistory& CSTXProtocolHistory::operator =(const CSTXProtocolHistory& val)
{
	_time = val._time;
	_items = val._items;
	_target = val._target;
	return *this;
}

