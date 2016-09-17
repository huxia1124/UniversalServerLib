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

