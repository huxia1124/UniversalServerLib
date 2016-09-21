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

#include <wtypes.h>
#include <queue>
#include <string>
#include <set>
#include <iterator>
#include <tchar.h>
#include "FastSpinlock.h"
#include "STXUtility.h"
#include <concurrent_vector.h>

#define DEFAULT_STRING_CACHE_HISTORY_MAX_COUNT		3

class CUniversalStringCache
{
public:
	CUniversalStringCache()
	{
		_globalIndex = -1;
		_referenceModules = NULL;
		_byteCode = NULL;
		_stringName = _T("");
		_ver = 0;
	}
	~CUniversalStringCache()
	{
		if (_stringInUse)
		{
			delete _stringInUse;
		}
		if (_stringAnsiInUse)
		{
			delete _stringAnsiInUse;
		}
		while (_stringHistory.size())
		{
			delete _stringHistory.front();
			_stringHistory.pop();
		}
		while (_byteCodeHistory.size())
		{
			delete _byteCodeHistory.front();
			_byteCodeHistory.pop();
		}
		if (_byteCode)
		{
			delete[]_byteCode;
		}
		while (_referenceModulesHistory.size())
		{
			delete _referenceModulesHistory.front();
			_referenceModulesHistory.pop();
		}
		if (_referenceModules)
		{
			delete _referenceModules;
		}

	}

	struct ByteCodeInfo
	{
		char *byteCode;
		UINT byteCodeLen;
		UINT byteCodeBufferLen;
	};

protected:
	struct _CNoCaseLess
	{
		bool operator()(const std::wstring& str1, const std::wstring& str2) const
		{
			return _tcsicmp(str1.c_str(), str2.c_str()) < 0;
		}
	};

	volatile bool _needUpdate = false;
	std::wstring *_stringInUse = NULL;
	std::string *_stringAnsiInUse = NULL;
	std::queue<std::wstring*> _stringHistory;
	std::queue<ByteCodeInfo *> _byteCodeHistory;
	std::wstring _stringName;
	std::set<std::wstring, _CNoCaseLess> *_referenceModules;
	std::queue<std::set<std::wstring, _CNoCaseLess>*> _referenceModulesHistory;
	LONGLONG _ver;

	FastSpinlock _lock;

	ByteCodeInfo *_byteCode;	//byteCode in use
	static int _globalIndexBase;
	int _globalIndex;

public:
	bool IsNeedUpdateDirect()
	{
		return _needUpdate;
	}
	bool IsNeedUpdate()
	{
		return _needUpdate;
	}
	void SetNeedUpdate(bool bNeedUpdate)
	{
		_needUpdate = bNeedUpdate;
	}
	void SetStringName(LPCTSTR lpszName)
	{
		_stringName = lpszName;
		LPTSTR psz = (LPTSTR)_stringName.c_str();
		while (*psz)
		{
			if (*psz == _T('/'))
				*psz = _T('\\');

			psz++;
		}
	}
	LPCTSTR GetStringName()
	{
		return _stringName.c_str();
	}
	void SetStringPtr(std::wstring *pString)
	{
		if (_stringInUse)
		{
			_stringHistory.push(_stringInUse);
		}
		if (_stringAnsiInUse)
		{
			delete _stringAnsiInUse;
		}

		_stringInUse = pString;

		char *pszBuf = new char[_stringInUse->size() * 4];
		size_t num = 0;
		wcstombs_s(&num, pszBuf, _stringInUse->size() * 4, _stringInUse->c_str(), _stringInUse->size());
		pszBuf[num] = 0;

		_stringAnsiInUse = new std::string(pszBuf);
		delete[]pszBuf;

		while (_stringHistory.size() > DEFAULT_STRING_CACHE_HISTORY_MAX_COUNT)
		{
			delete _stringHistory.front();
			_stringHistory.pop();
		}

		_ver++;
	}
	LPCTSTR GetString()
	{
		if (_stringInUse == NULL)
			return NULL;

		return _stringInUse->c_str();
	}
	LPCSTR GetMBCSString()
	{
		if (_stringAnsiInUse == NULL)
			return NULL;

		return _stringAnsiInUse->c_str();
	}
	ByteCodeInfo GetByteCode()
	{
		if (_byteCode)
		{
			return *_byteCode;
		}
		ByteCodeInfo empty = { 0,0,0 };
		return empty;
	}
	void SetByteCode(LPVOID pData, size_t cbSize)
	{
		ByteCodeInfo *pByteCode = new ByteCodeInfo();
		if (pByteCode == NULL)
		{
			return;
		}

		char *pNewByteCodeBuffer = new char[cbSize];
		if (pNewByteCodeBuffer == NULL)
		{
			delete pByteCode;
			return;
		}

		if (_byteCode != NULL)
		{
			_byteCodeHistory.push(_byteCode);
		}

		memcpy(pNewByteCodeBuffer, pData, cbSize);
		pByteCode->byteCode = pNewByteCodeBuffer;
		pByteCode->byteCodeBufferLen = cbSize;
		pByteCode->byteCodeLen = cbSize;

		_byteCode = pByteCode;

		while (_byteCodeHistory.size() > DEFAULT_STRING_CACHE_HISTORY_MAX_COUNT)
		{
			delete _byteCodeHistory.front();
			_byteCodeHistory.pop();
		}
	}
	void Lock()
	{
		_lock.EnterLock();
	}
	void Unlock()
	{
		_lock.LeaveLock();
	}
	BOOL IsEmpty()
	{
		return _byteCode == NULL && _stringInUse == NULL;
	}
	void SetReferenceModule(Concurrency::concurrent_vector<std::wstring> *pModules)
	{
		if (_referenceModules)
		{
			_referenceModulesHistory.push(_referenceModules);
		}

		_referenceModules = new std::set<std::wstring, _CNoCaseLess>();

		for (std::wstring moduleName : *pModules)
		{
			_referenceModules->insert(moduleName);
		}

		while (_referenceModulesHistory.size() > DEFAULT_STRING_CACHE_HISTORY_MAX_COUNT)
		{
			delete _referenceModulesHistory.front();
			_referenceModulesHistory.pop();
		}
	}
	void GetReferenceRemove(std::vector<std::wstring> *pRemove, Concurrency::concurrent_vector<std::wstring> *pNewModules)
	{
		if (_referenceModules == NULL || pNewModules == NULL || pNewModules->size() == 0 || _referenceModules->size() == 0 || pRemove == NULL)
			return;

		std::set<std::wstring> newModules;
		for (std::wstring moduleName : *pNewModules)
		{
			newModules.insert(moduleName);
		}

		set_difference(_referenceModules->begin(), _referenceModules->end(), newModules.begin(), newModules.end(), std::back_inserter(*pRemove));
	}
	void GetReferenceAdded(std::vector<std::wstring> *pAdd, Concurrency::concurrent_vector<std::wstring> *pNewModules)
	{
		if (_referenceModules == NULL || pNewModules == NULL || pNewModules->size() == 0 || _referenceModules->size() == 0 || pAdd == NULL)
			return;

		std::set<std::wstring> newModules;
		for (std::wstring moduleName : *pNewModules)
		{
			newModules.insert(moduleName);
		}

		set_difference(newModules.begin(), newModules.end(), _referenceModules->begin(), _referenceModules->end(), std::back_inserter(*pAdd));
	}
	BOOL IsNeedUpdateForThreads(LONGLONG *pVersionInThread)
	{
		if (pVersionInThread == NULL)
			return TRUE;

		if (*pVersionInThread != _ver)
		{
			*pVersionInThread = _ver;
			return TRUE;
		}

		return FALSE;
	}
	void GetCleanupModules(std::vector<std::wstring> *pRefreshModules)
	{
		std::set<std::wstring, _CNoCaseLess> *pRef = NULL;
		if (_referenceModulesHistory.size() > 0)
		{
			pRef = _referenceModulesHistory.back();
		}
		else if (_referenceModules)
		{
			pRef = _referenceModules;
		}

		if (pRef)
		{
			for (std::wstring moduleName : *pRef)
			{
				pRefreshModules->push_back(moduleName);
			}
		}
	}
	int GetGlobalIndex()
	{
		return _globalIndex;
	}
	void EnableTraceThreadVersion()
	{
		if(_globalIndex < 0)
			_globalIndex = _globalIndexBase++;
	}
};
