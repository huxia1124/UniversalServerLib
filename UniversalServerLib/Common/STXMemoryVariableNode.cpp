#include "stdafx.h"
#include "STXMemoryVariableNode.h"
#include <tchar.h>
#include <iterator>


//////////////////////////////////////////////////////////////////////////
int SplitString(const TCHAR *pszSrc, TCHAR chSplit, std::vector<std::wstring> &arrResult)
{
	const TCHAR *pValue = pszSrc;
	size_t nLen = _tcslen(pszSrc);
	if (nLen > 0)
	{
		arrResult.clear();
		TCHAR *pValueCopy = new TCHAR[nLen + 2];
		_tcscpy_s(pValueCopy, nLen + 2, pValue);

		TCHAR seps[2] = { 0 };
		seps[0] = chSplit;
		seps[1] = 0;

		TCHAR *pStart = pValueCopy, *pEnd = pStart + 1;
		while (pStart && *pStart)
		{
			pEnd = _tcschr(pStart, chSplit);
			if (pEnd == 0)
			{
				break;
				pEnd = pValueCopy + nLen;
			}

			TCHAR chEnd = *pEnd;
			TCHAR *pStartCopy = pStart;
			pStart = pEnd + 1;
			*pEnd = 0;

			arrResult.push_back(pStartCopy);

			if (chEnd == 0)
				break;

		}
		arrResult.push_back(pStart);
		delete[]pValueCopy;
	}
	return (int)arrResult.size();
}

bool CSTXMemoryVariableNode::IsNewTypeAcceptable(int newType)
{
	return _type == STXVariableTreeNodeType_Invalid || _type == newType;
}

std::shared_ptr<CSTXMemoryVariableNode> CSTXMemoryVariableNode::RegisterVariable(std::wstring strPathName, STXVariableTreeNodeType nType, void* pAddress, bool managed)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	std::shared_ptr<CSTXMemoryVariableNode> result;
	if (arrParts.size() == 0)
		return result;

	if (arrParts.size() == 1)
	{
		_mapContent.lock(strPathName);
		auto it = _mapContent.find(strPathName);
		std::shared_ptr<CSTXMemoryVariableNode> newVariable;
		if (it == _mapContent.end(strPathName))
		{
			newVariable = std::make_shared<CSTXMemoryVariableNode>();
			_mapContent[strPathName] = newVariable;
			newVariable->_type = nType;
			newVariable->_ptr = pAddress;
			newVariable->_parentNode = this;
			newVariable->_name = strPathName;
			newVariable->_managedValue = managed;
		}
		else
		{
			newVariable = it->second;
		}
		_mapContent.unlock(strPathName);

		return newVariable;
	}

	std::wstring firstPart = arrParts[0];
	_mapContent.lock(firstPart);
	auto it = _mapContent.find(firstPart);
	if (it == _mapContent.end(firstPart))
	{
		auto newVariable = std::make_shared<CSTXMemoryVariableNode>();
		_mapContent[firstPart] = newVariable;
		newVariable->_parentNode = this;
		newVariable->_name = firstPart;
	}
	it = _mapContent.find(firstPart);
	auto nodeFound = it->second;
	_mapContent.unlock(firstPart);

	return nodeFound->RegisterVariable(strPathName.substr(firstPart.size() + 1), nType, pAddress, managed);
}

CSTXMemoryVariableNode::CSTXMemoryVariableNode()
{
	_type = STXVariableTreeNodeType_Invalid;
	_ptr = NULL;
}

CSTXMemoryVariableNode::CSTXMemoryVariableNode(std::wstring name)
{
	_type = STXVariableTreeNodeType_Invalid;
	_ptr = NULL;
	_name = name;
}

CSTXMemoryVariableNode::~CSTXMemoryVariableNode()
{
	if (_managedValue)
	{
		switch (_type)
		{
		case STXVariableTreeNodeType_Int32:		//int32_t
			delete (int32_t*)_ptr;
			break;
		case STXVariableTreeNodeType_Int64:		//int64_t
			delete (int64_t*)_ptr;
			break;
		case STXVariableTreeNodeType_WString:		//wstring
			delete (std::wstring*)_ptr;
			break;
		case STXVariableTreeNodeType_Int:		//int
			delete (int*)_ptr;
			break;
		case STXVariableTreeNodeType_Float:		//float
			delete (float*)_ptr;
			break;
		case STXVariableTreeNodeType_Double:		//double
			delete (double*)_ptr;
			break;
		case STXVariableTreeNodeType_Word:		//uint16_t
			delete (uint16_t*)_ptr;
			break;
		case STXVariableTreeNodeType_DWord:		//uint32_t
			delete (uint32_t*)_ptr;
			break;
		case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
			delete (std::vector<std::wstring>*)_ptr;
			break;
		case STXVariableTreeNodeType_WStringSet:		//set<wstring>
			delete (std::set<std::wstring>*)_ptr;
			break;
		case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
			delete (std::vector<int64_t>*)_ptr;
			break;
		case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
			delete (std::set<int64_t>*)_ptr;
			break;
		case STXVariableTreeNodeType_DoubleVector:		//vector<double>
			delete (std::vector<double>*)_ptr;
			break;
		case STXVariableTreeNodeType_DoubleSet:		//set<double>
			delete (std::set<double>*)_ptr;
			break;

		default:
			break;
		}
	}
}

void* CSTXMemoryVariableNode::GetVariablePtr(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL)
		return NULL;

	return pNode->_ptr;
}

std::shared_ptr<CSTXMemoryVariableNode> CSTXMemoryVariableNode::GetVariableNode(std::wstring strPathName)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	if (arrParts.size() == 0)
		return NULL;

	std::wstring firstPart = arrParts[0];
	_mapContent.lock(firstPart);
	auto it = _mapContent.find(firstPart);
	if (it == _mapContent.end(firstPart))
	{
		_mapContent.unlock(firstPart);
		return NULL;
	}
	auto nodeFound = it->second;
	_mapContent.unlock(firstPart);

	if (arrParts.size() == 1)
	{
		return nodeFound;
	}

	return nodeFound->GetVariableNode(strPathName.substr(firstPart.size() + 1));
}

int CSTXMemoryVariableNode::GetVariableType(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL)
		return -1;

	return pNode->_type;
}

int CSTXMemoryVariableNode::GetThisVariableType()
{
	return _type;
}

void CSTXMemoryVariableNode::UnregisterVariable(std::wstring strPathName)
{
	std::vector<std::wstring> arrParts;
	SplitString(strPathName.c_str(), _T('\\'), arrParts);

	if (arrParts.size() == 0)
		return;

	if (arrParts.size() == 1)
	{
		_mapContent.erase(strPathName);
		return;
	}

	std::wstring firstPart = arrParts[0];
	_mapContent.lock(firstPart);
	auto it = _mapContent.find(firstPart);
	if (it == _mapContent.end(firstPart))
	{
		_mapContent.unlock(firstPart);
		return;
	}

	auto nodeFound = it->second;
	_mapContent.unlock(firstPart);

	nodeFound->UnregisterVariable(strPathName.substr(firstPart.size() + 1));
}

void CSTXMemoryVariableNode::SetStringValue(std::wstring strPathName, std::wstring strValue)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		*((int32_t*)pNode->_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		*((int64_t*)pNode->_ptr) = _ttoi64(strValue.c_str());
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		*((std::wstring*)pNode->_ptr) = strValue;
		break;
	case STXVariableTreeNodeType_Int:		//int
		*((int*)pNode->_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Float:		//float
		*((float*)pNode->_ptr) = (float)_ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Double:		//double
		*((double*)pNode->_ptr) = _ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		*((uint16_t*)pNode->_ptr) = (uint16_t)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		*((uint32_t*)pNode->_ptr) = (uint32_t)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	default:
		break;
	}
}

std::wstring CSTXMemoryVariableNode::GetStringValue(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return _T("");

	pNode->LockValue();
	return GetStringValue(pNode->_ptr, pNode->_type);
	pNode->UnlockValue();
}

std::wstring CSTXMemoryVariableNode::GetStringValue(void *ptr, int dataType)
{
	const int buflen = 256;
	TCHAR szTmp[buflen];
	switch (dataType)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		_itot_s(*((int32_t*)ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		_i64tot_s(*((int64_t*)ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		return *((std::wstring*)ptr);
		break;
	case STXVariableTreeNodeType_Int:		//int
		_itot_s(*((int*)ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Float:		//float
		_stprintf_s(szTmp, _T("%f"), *((float*)ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Double:		//double
		_stprintf_s(szTmp, _T("%lf"), *((double*)ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		_ultot_s(*((uint16_t*)ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		_ultot_s(*((uint32_t*)ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::vector<std::wstring>*)ptr)->begin(), ((std::vector<std::wstring>*)ptr)->end(), [&](std::wstring val) {
			strTmp += szPrefix;
			strTmp += _T("\"");
			strTmp += val;
			strTmp += _T("\"");
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::set<std::wstring>*)ptr)->begin(), ((std::set<std::wstring>*)ptr)->end(), [&](std::wstring val) {
			strTmp += szPrefix;
			strTmp += _T("\"");
			strTmp += val;
			strTmp += _T("\"");
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::vector<int64_t>*)ptr)->begin(), ((std::vector<int64_t>*)ptr)->end(), [&](int64_t val) {
			_i64tot_s(val, szTmp, buflen, 10);
			strTmp += szPrefix;
			strTmp += szTmp;
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::set<int64_t>*)ptr)->begin(), ((std::set<int64_t>*)ptr)->end(), [&](int64_t val) {
			_i64tot_s(val, szTmp, buflen, 10);
			strTmp += szPrefix;
			strTmp += szTmp;
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	case STXVariableTreeNodeType_DoubleVector:		//vector<double>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::vector<double>*)ptr)->begin(), ((std::vector<double>*)ptr)->end(), [&](double val) {
			_stprintf_s(szTmp, _T("%lf"), val);
			strTmp += szPrefix;
			strTmp += szTmp;
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	case STXVariableTreeNodeType_DoubleSet:		//set<double>
	{
		std::wstring strTmp = _T("[");
		TCHAR szPrefix[2] = { 0 };
		std::for_each(((std::set<double>*)ptr)->begin(), ((std::set<double>*)ptr)->end(), [&](double val) {
			_stprintf_s(szTmp, _T("%lf"), val);
			strTmp += szPrefix;
			strTmp += szTmp;
			szPrefix[0] = ',';
		});
		strTmp += _T("]");
		return strTmp;
		break;
	}
	default:
		break;
	}

	return _T("");
}

std::wstring CSTXMemoryVariableNode::GetThisStringValue()
{
	LockValue();
	auto result = GetStringValue(_ptr, _type);
	UnlockValue();
	return result;
}

void CSTXMemoryVariableNode::SetThisStringValue(std::wstring strValue)
{
	if (_type < 0)
		return;

	switch (_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		*((int32_t*)_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		*((int64_t*)_ptr) = _ttoi64(strValue.c_str());
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		LockValue();
		*((std::wstring*)_ptr) = strValue;
		UnlockValue();
		break;
	case STXVariableTreeNodeType_Int:		//int
		*((int*)_ptr) = _ttoi(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Float:		//float
		*((float*)_ptr) = (float)_ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Double:		//double
		*((double*)_ptr) = _ttof(strValue.c_str());
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		*((uint16_t*)_ptr) = (uint16_t)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		*((uint32_t*)_ptr) = (uint32_t)_tcstoul(strValue.c_str(), NULL, 10);
		break;
	default:
		break;
	}
}

size_t CSTXMemoryVariableNode::GetThisValues(std::vector<std::wstring> *values)
{
	const int buflen = 256;
	TCHAR szTmp[buflen];
	switch (_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		_itot_s(*((int32_t*)_ptr), szTmp, buflen, 10);
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		_i64tot_s(*((int64_t*)_ptr), szTmp, buflen, 10);
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		values->push_back(*((std::wstring*)_ptr));
		break;
	case STXVariableTreeNodeType_Int:		//int
		_itot_s(*((int*)_ptr), szTmp, buflen, 10);
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_Float:		//float
		_stprintf_s(szTmp, _T("%f"), *((float*)_ptr));
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_Double:		//double
		_stprintf_s(szTmp, _T("%lf"), *((double*)_ptr));
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		_ultot_s(*((uint16_t*)_ptr), szTmp, buflen, 10);
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		_ultot_s(*((uint32_t*)_ptr), szTmp, buflen, 10);
		values->push_back(szTmp);
		break;
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		values->insert(values->end(), ((std::vector<std::wstring>*)_ptr)->begin(), ((std::vector<std::wstring>*)_ptr)->end());
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		values->insert(values->end(), ((std::set<std::wstring>*)_ptr)->begin(), ((std::set<std::wstring>*)_ptr)->end());
		break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		std::for_each(((std::vector<int64_t>*)_ptr)->begin(), ((std::vector<int64_t>*)_ptr)->end(), [&] (int64_t val){
			_i64tot_s(val, szTmp, buflen, 10);
			values->push_back(szTmp);
		});
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		std::for_each(((std::set<int64_t>*)_ptr)->begin(), ((std::set<int64_t>*)_ptr)->end(), [&](int64_t val) {
			_i64tot_s(val, szTmp, buflen, 10);
			values->push_back(szTmp);
		});
		break;
	case STXVariableTreeNodeType_DoubleVector:		//vector<double>
		std::for_each(((std::vector<double>*)_ptr)->begin(), ((std::vector<double>*)_ptr)->end(), [&] (double val){
			_stprintf_s(szTmp, _T("%lf"), val);
			values->push_back(szTmp);
		});
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<double>
		std::for_each(((std::set<double>*)_ptr)->begin(), ((std::set<double>*)_ptr)->end(), [&](double val) {
			_stprintf_s(szTmp, _T("%lf"), val);
			values->push_back(szTmp);
		});
		break;
	case STXVariableTreeNodeType_Invalid:
		break;
	default:
		break;
	}

	return values->size();
}

size_t CSTXMemoryVariableNode::GetThisValues(std::vector<int64_t> *values)
{
	switch (_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		values->push_back(*((int32_t*)_ptr));
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		values->push_back(*((int64_t*)_ptr));
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		values->push_back(_ttoi(((std::wstring*)_ptr)->c_str()));
		break;
	case STXVariableTreeNodeType_Int:		//int
		values->push_back(*((int*)_ptr));
		break;
	case STXVariableTreeNodeType_Float:		//float
		values->push_back(*((float*)_ptr));
		break;
	case STXVariableTreeNodeType_Double:		//double
		values->push_back(*((double*)_ptr));
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		values->push_back(*((uint16_t*)_ptr));
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		values->push_back(*((uint32_t*)_ptr));
		break;
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		std::for_each(((std::vector<std::wstring>*)_ptr)->begin(), ((std::vector<std::wstring>*)_ptr)->end(), [&](std::wstring val) {
			values->push_back(_ttoi(val.c_str()));
		});
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		std::for_each(((std::set<std::wstring>*)_ptr)->begin(), ((std::set<std::wstring>*)_ptr)->end(), [&](std::wstring val) {
			values->push_back(_ttoi(val.c_str()));
		});
		break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		*values = (*((std::vector<int64_t>*)_ptr));
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		std::copy((*(std::set<int64_t>*)_ptr).begin(), (*(std::set<int64_t>*)_ptr).end(), std::back_inserter(*values));
		break;
	case STXVariableTreeNodeType_DoubleVector:		//vector<double>
		std::for_each(((std::vector<double>*)_ptr)->begin(), ((std::vector<double>*)_ptr)->end(), [&](double val) {
			values->push_back(val);
		});
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<double>
		std::for_each(((std::set<double>*)_ptr)->begin(), ((std::set<double>*)_ptr)->end(), [&](double val) {
			values->push_back(val);
		});
		break;
	case STXVariableTreeNodeType_Invalid:
		break;
	default:
		values->push_back(0);
		break;
	}

	return values->size();
}

size_t CSTXMemoryVariableNode::GetThisValues(std::vector<double> *values)
{
	switch (_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		values->push_back(*((int32_t*)_ptr));
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		values->push_back(*((int64_t*)_ptr));
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		values->push_back(_tcstod(((std::wstring*)_ptr)->c_str(), nullptr));
		break;
	case STXVariableTreeNodeType_Int:		//int
		values->push_back(*((int*)_ptr));
		break;
	case STXVariableTreeNodeType_Float:		//float
		values->push_back(*((float*)_ptr));
		break;
	case STXVariableTreeNodeType_Double:		//double
		values->push_back(*((double*)_ptr));
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		values->push_back(*((uint16_t*)_ptr));
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		values->push_back(*((uint32_t*)_ptr));
		break;
	case STXVariableTreeNodeType_DoubleVector:		//vector<double>
		*values = (*((std::vector<double>*)_ptr));
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<double>
		std::copy((*(std::set<double>*)_ptr).begin(), (*(std::set<double>*)_ptr).end(), std::back_inserter(*values));
		break;
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		std::for_each(((std::vector<std::wstring>*)_ptr)->begin(), ((std::vector<std::wstring>*)_ptr)->end(), [&](std::wstring val) {
			values->push_back(_tcstod(val.c_str(), nullptr));
		});
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		std::for_each(((std::set<std::wstring>*)_ptr)->begin(), ((std::set<std::wstring>*)_ptr)->end(), [&](std::wstring val) {
			values->push_back(_tcstod(val.c_str(), nullptr));
		});
		break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		std::for_each(((std::vector<int64_t>*)_ptr)->begin(), ((std::vector<int64_t>*)_ptr)->end(), [&](int64_t val) {
			values->push_back(val);
		});
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		std::for_each(((std::set<int64_t>*)_ptr)->begin(), ((std::set<int64_t>*)_ptr)->end(), [&](int64_t val) {
			values->push_back(val);
		});
		break;
	case STXVariableTreeNodeType_Invalid:
		break;
	default:
		values->push_back(0);
		break;
	}

	return values->size();
}

void CSTXMemoryVariableNode::AddStringValue(std::wstring strPathName, std::wstring strValue)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	pNode->LockValue();
	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		(*((std::vector<std::wstring>*)pNode->_ptr)).push_back(strValue);
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		(*((std::set<std::wstring>*)pNode->_ptr)).insert(strValue);
		break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		(*((std::vector<int64_t>*)pNode->_ptr)).push_back(_ttoi64(strValue.c_str()));
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		(*((std::set<int64_t>*)pNode->_ptr)).insert(_ttoi64(strValue.c_str()));
		break;
	case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
		(*((std::vector<double>*)pNode->_ptr)).push_back(_tcstod(strValue.c_str(), nullptr));
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
		(*((std::set<double>*)pNode->_ptr)).insert(_tcstod(strValue.c_str(), nullptr));
		break;
	}
	pNode->UnlockValue();
}

void CSTXMemoryVariableNode::AddIntegerValue(std::wstring strPathName, int64_t value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	pNode->LockValue();
	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		(*((std::vector<int64_t>*)pNode->_ptr)).push_back(value);
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		(*((std::set<int64_t>*)pNode->_ptr)).insert(value);
		break;
	case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
		(*((std::vector<double>*)pNode->_ptr)).push_back(value);
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
		(*((std::set<double>*)pNode->_ptr)).insert(value);
		break;
	}
	pNode->UnlockValue();
}

void CSTXMemoryVariableNode::AddDoubleValue(std::wstring strPathName, double value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	pNode->LockValue();
	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_DoubleVector:		//vector<double>
		(*((std::vector<double>*)pNode->_ptr)).push_back(value);
		break;
	case STXVariableTreeNodeType_DoubleSet:		//set<double>
		(*((std::set<double>*)pNode->_ptr)).insert(value);
		break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		(*((std::vector<int64_t>*)pNode->_ptr)).push_back(value);
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		(*((std::set<int64_t>*)pNode->_ptr)).insert(value);
		break;
	}
	pNode->UnlockValue();
}

void CSTXMemoryVariableNode::RemoveStringValue(std::wstring strPathName, std::wstring strValue)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	pNode->LockValue();
	RemoveStringValue(pNode->_ptr, pNode->_type, strValue);
	pNode->UnlockValue();
}

void CSTXMemoryVariableNode::RemoveStringValue(void *ptr, int dataType, std::wstring strValue)
{
	if (ptr == NULL || dataType < 0)
		return;

	switch (dataType)
	{
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		((std::vector<std::wstring>*)ptr)->erase(std::remove(((std::vector<std::wstring>*)ptr)->begin(), ((std::vector<std::wstring>*)ptr)->end(), strValue), ((std::vector<std::wstring>*)ptr)->end());
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		((std::set<std::wstring>*)ptr)->erase(strValue);
		break;
	//case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
	//	(*((std::vector<int64_t>*)ptr)).push_back(_ttoi64(strValue.c_str()));
	//	break;
	//case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
	//	(*((std::set<int64_t>*)ptr)).insert(_ttoi64(strValue.c_str()));
	//	break;
	//case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
	//	(*((std::vector<double>*)ptr)).push_back(_tcstod(strValue.c_str(), nullptr));
	//	break;
	//case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
	//	(*((std::set<double>*)ptr)).insert(_tcstod(strValue.c_str(), nullptr));
	//	break;
	}

}

void CSTXMemoryVariableNode::RemoveThisStringValue(std::wstring strValue)
{
	LockValue();
	RemoveStringValue(_ptr, _type, strValue);
	UnlockValue();
}

void CSTXMemoryVariableNode::RemoveIntegerValue(void *ptr, int dataType, int64_t value)
{
	if (ptr == NULL || dataType < 0)
		return;

	switch (dataType)
	{
	//case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
	//	std::remove(((std::vector<std::wstring>*)ptr)->begin(), ((std::vector<std::wstring>*)ptr)->end(), strValue);
	//	break;
	//case STXVariableTreeNodeType_WStringSet:		//set<wstring>
	//	(*((std::set<std::wstring>*)ptr)).erase(strValue);
	//	break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		((std::vector<int64_t>*)ptr)->erase(std::remove(((std::vector<int64_t>*)ptr)->begin(), ((std::vector<int64_t>*)ptr)->end(), value), ((std::vector<int64_t>*)ptr)->end());
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		((std::set<int64_t>*)ptr)->erase(value);
		break;
	//case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
	//	(*((std::vector<double>*)ptr)).push_back(_tcstod(strValue.c_str(), nullptr));
	//	break;
	//case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
	//	(*((std::set<double>*)ptr)).insert(_tcstod(strValue.c_str(), nullptr));
	//	break;
	}
}

void CSTXMemoryVariableNode::RemoveIntegerValue(std::wstring strPathName, int64_t value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	pNode->LockValue();
	RemoveIntegerValue(pNode->_ptr, pNode->_type, value);
	pNode->UnlockValue();
}

void CSTXMemoryVariableNode::LockValue()
{
	_value_mtx.lock();
}

void CSTXMemoryVariableNode::UnlockValue()
{
	_value_mtx.unlock();
}

void CSTXMemoryVariableNode::RemoveThisIntegerValue(int64_t value)
{
	RemoveIntegerValue(_ptr, _type, value);
}

bool CSTXMemoryVariableNode::IsContainStringValue(std::wstring strPathName, std::wstring strValue)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return false;

	pNode->LockValue();
	auto result = IsContainStringValue(pNode->_ptr, pNode->_type, strValue);
	pNode->UnlockValue();
	return result;
}

bool CSTXMemoryVariableNode::IsContainStringValue(void *ptr, int dataType, std::wstring strValue)
{
	if (ptr == NULL || dataType < 0)
		return false;

	switch (dataType)
	{
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		return std::find(((std::vector<std::wstring>*)ptr)->begin(), ((std::vector<std::wstring>*)ptr)->end(), strValue) != ((std::vector<std::wstring>*)ptr)->end();
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		return ((std::set<std::wstring>*)ptr)->find(strValue) != ((std::set<std::wstring>*)ptr)->end();
		break;
		//case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		//	(*((std::vector<int64_t>*)ptr)).push_back(_ttoi64(strValue.c_str()));
		//	break;
		//case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		//	(*((std::set<int64_t>*)ptr)).insert(_ttoi64(strValue.c_str()));
		//	break;
		//case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
		//	(*((std::vector<double>*)ptr)).push_back(_tcstod(strValue.c_str(), nullptr));
		//	break;
		//case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
		//	(*((std::set<double>*)ptr)).insert(_tcstod(strValue.c_str(), nullptr));
		//	break;
	}

	return false;
}

bool CSTXMemoryVariableNode::IsContainIntegerValue(std::wstring strPathName, int64_t value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return false;

	pNode->LockValue();
	auto result = IsContainIntegerValue(pNode->_ptr, pNode->_type, value);
	pNode->UnlockValue();
	return result;
}

bool CSTXMemoryVariableNode::IsContainIntegerValue(void *ptr, int dataType, int64_t value)
{
	if (ptr == NULL || dataType < 0)
		return false;

	switch (dataType)
	{
		//case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		//	std::remove(((std::vector<std::wstring>*)ptr)->begin(), ((std::vector<std::wstring>*)ptr)->end(), strValue);
		//	break;
		//case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		//	(*((std::set<std::wstring>*)ptr)).erase(strValue);
		//	break;
	case STXVariableTreeNodeType_IntegerVector:		//vector<int64_t>
		return std::find(((std::vector<int64_t>*)ptr)->begin(), ((std::vector<int64_t>*)ptr)->end(), value) != ((std::vector<int64_t>*)ptr)->end();
		break;
	case STXVariableTreeNodeType_IntegerSet:		//set<int64_t>
		return ((std::set<int64_t>*)ptr)->find(value) != ((std::set<int64_t>*)ptr)->end();
		break;
		//case STXVariableTreeNodeType_DoubleVector:		//vector<int64_t>
		//	(*((std::vector<double>*)ptr)).push_back(_tcstod(strValue.c_str(), nullptr));
		//	break;
		//case STXVariableTreeNodeType_DoubleSet:		//set<int64_t>
		//	(*((std::set<double>*)ptr)).insert(_tcstod(strValue.c_str(), nullptr));
		//	break;
	}
	return false;
}

bool CSTXMemoryVariableNode::IsContainThisStringValue(std::wstring strValue)
{
	LockValue();
	auto result = IsContainStringValue(_ptr, _type, strValue);
	UnlockValue();
	return result;
}

bool CSTXMemoryVariableNode::IsContainThisIntegerValue(int64_t value)
{
	LockValue();
	auto result = IsContainIntegerValue(_ptr, _type, value);
	UnlockValue();
	return result;
}

void CSTXMemoryVariableNode::GetChildren(std::vector<std::shared_ptr<CSTXMemoryVariableNode>>* children)
{
	_mapContent.foreach([&](std::pair<std::wstring, std::shared_ptr<CSTXMemoryVariableNode>> item) {
		children->push_back(item.second);
	});
}

int64_t CSTXMemoryVariableNode::GetIntegerValue(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return 0;

	if (pNode->_type != STXVariableTreeNodeType_Int64)
		return 0;

	return (*((int64_t*)pNode->_ptr));
}

void CSTXMemoryVariableNode::SetIntegerValue(std::wstring strPathName, int64_t value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	if (pNode->_type != STXVariableTreeNodeType_Int64)
		return;

	*((int64_t*)pNode->_ptr) = value;
}

int64_t CSTXMemoryVariableNode::GetThisIntegerValue()
{
	if (_type != STXVariableTreeNodeType_Int64)
		return 0;

	return (*((int64_t*)_ptr));
}

void CSTXMemoryVariableNode::SetThisIntegerValue(int64_t value)
{
	if (_type != STXVariableTreeNodeType_Int64)
		return;

	*((int64_t*)_ptr) = value;
}

double CSTXMemoryVariableNode::GetDoubleValue(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return 0;

	if (pNode->_type != STXVariableTreeNodeType_Double)
		return 0;

	return (*((double*)pNode->_ptr));
}

void CSTXMemoryVariableNode::SetDoubleValue(std::wstring strPathName, double value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	if (pNode->_type != STXVariableTreeNodeType_Double)
		return;

	*((double*)pNode->_ptr) = value;
}

int64_t CSTXMemoryVariableNode::IncreaseIntegerValue(std::wstring strPathName, int64_t delta)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return 0;

	return IncreaseIntegerValue(pNode->_ptr, pNode->_type, delta);
}

int64_t CSTXMemoryVariableNode::IncreaseIntegerValue(void *ptr, int dataType, int64_t delta)
{
	if (!ptr || dataType == STXVariableTreeNodeType_Invalid)
		return 0;

	switch (dataType)
	{
	case STXVariableTreeNodeType_Int32:
	{
		int32_t oldval = *((int32_t*)ptr);
		*((int32_t*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_Int64:
	{
		int64_t oldval = *((int64_t*)ptr);
		*((int64_t*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_Int:
	{
		int oldval = *((int*)ptr);
		*((int*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_Float:
	{
		float oldval = *((float*)ptr);
		*((float*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_Double:
	{
		double oldval = *((double*)ptr);
		*((double*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_Word:
	{
		uint16_t oldval = *((uint16_t*)ptr);
		*((uint16_t*)ptr) += delta;
		return oldval;
	}
	case STXVariableTreeNodeType_DWord:
	{
		uint32_t oldval = *((uint32_t*)ptr);
		*((uint32_t*)ptr) += delta;
		return oldval;
	}
	}
	return 0;
}

int64_t CSTXMemoryVariableNode::IncreaseThisIntegerValue(int64_t delta)
{
	return IncreaseIntegerValue(_ptr, _type, delta);
}

int CSTXMemoryVariableNode::GetChildrenNames(std::vector<std::wstring> *pArrNames)
{
	if (pArrNames == NULL)
		return _mapContent.size();

	pArrNames->clear();
	_mapContent.foreach([&](std::pair<const std::wstring, std::shared_ptr<CSTXMemoryVariableNode>> item)
	{
		pArrNames->push_back(item.first);
	});

	return (int)pArrNames->size();
}

std::wstring CSTXMemoryVariableNode::GetFullPath()
{
	std::wstring path = _name;
	auto pParent = _parentNode;
	while (pParent)
	{
		path = pParent->_name + _T("\\") + path;
		auto pNextParent = pParent->_parentNode;
		pParent = pNextParent;
	}
	return path;
}

bool CSTXMemoryVariableNode::IsManagedValue()
{
	return _managedValue;
}

bool CSTXMemoryVariableNode::IsThisValueExists()
{
	return _type != STXVariableTreeNodeType_Invalid;
}

bool CSTXMemoryVariableNode::IsValueExists(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (!nodeExists)
		return false;

	return nodeExists->IsThisValueExists();
}

void CSTXMemoryVariableNode::RegisterInt32Variable(std::wstring strPathName, int32_t *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int32, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterInt32Variable(std::wstring strPathName, int32_t value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_Int32))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	int32_t *v = new int32_t(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_Int32, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterInt64Variable(std::wstring strPathName, int64_t *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int64, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterInt64Variable(std::wstring strPathName, int64_t value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_Int64))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	int64_t *v = new int64_t(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_Int64, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterStringVariable(std::wstring strPathName, std::wstring *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_WString, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterStringVariable(std::wstring strPathName, std::wstring value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_WString))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::wstring *v = new std::wstring(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_WString, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterStringVectorVariable(std::wstring strPathName, std::vector<std::wstring> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_WStringVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<std::wstring>  *v = new std::vector<std::wstring>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_WStringVector, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterStringVectorVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_WStringVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<std::wstring>  *v = new std::vector<std::wstring>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_WStringVector, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterStringSetVariable(std::wstring strPathName, std::set<std::wstring> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_WStringSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<std::wstring>  *v = new std::set<std::wstring>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_WStringSet, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterStringSetVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_WStringSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<std::wstring>  *v = new std::set<std::wstring>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_WStringSet, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterIntegerVariable(std::wstring strPathName, int64_t value)
{
	RegisterInt64Variable(strPathName, value);

}

void CSTXMemoryVariableNode::RegisterIntegerVariable(std::wstring strPathName)
{
	RegisterInt64Variable(strPathName, (int64_t)0);
}

void CSTXMemoryVariableNode::RegisterIntegerVectorVariable(std::wstring strPathName, std::vector<int64_t> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_IntegerVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<int64_t> *v = new std::vector<int64_t>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_IntegerVector, v, true);
	var->_managedValue = true;

}

void CSTXMemoryVariableNode::RegisterIntegerVectorVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_IntegerVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<int64_t> *v = new std::vector<int64_t>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_IntegerVector, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterIntegerSetVariable(std::wstring strPathName, std::set<int64_t> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_IntegerSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<int64_t> *v = new std::set<int64_t>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_IntegerSet, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterIntegerSetVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_IntegerSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<int64_t> *v = new std::set<int64_t>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_IntegerSet, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterDoubleVectorVariable(std::wstring strPathName, std::vector<double> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_DoubleVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<double> *v = new std::vector<double>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_DoubleVector, v, true);
	var->_managedValue = true;

}

void CSTXMemoryVariableNode::RegisterDoubleVectorVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_DoubleVector))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::vector<double> *v = new std::vector<double>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_DoubleVector, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterDoubleSetVariable(std::wstring strPathName, std::set<double> value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_DoubleSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<double> *v = new std::set<double>(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_DoubleSet, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterDoubleSetVariable(std::wstring strPathName)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_DoubleSet))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	std::set<double> *v = new std::set<double>();
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_DoubleSet, v, true);
	var->_managedValue = true;
}

std::wstring CSTXMemoryVariableNode::GetName()
{
	return _name;
}

void CSTXMemoryVariableNode::RegisterIntVariable(std::wstring strPathName, int *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Int, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterFloatVariable(std::wstring strPathName, float *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Float, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterDoubleVariable(std::wstring strPathName, double *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Double, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterDoubleVariable(std::wstring strPathName, double value)
{
	auto nodeExists = GetVariableNode(strPathName);
	if (nodeExists)
	{
		if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_Double))
		{
			throw std::runtime_error("The node already has a value of different type.");
		}
		return;
	}
	double *v = new double(value);
	auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_Double, v, true);
	var->_managedValue = true;
}

void CSTXMemoryVariableNode::RegisterDoubleVariable(std::wstring strPathName)
{
	RegisterDoubleVariable(strPathName, 0.0);
}

void CSTXMemoryVariableNode::RegisterWordVariable(std::wstring strPathName, uint16_t *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_Word, pAddress, false);
}

void CSTXMemoryVariableNode::RegisterDWordVariable(std::wstring strPathName, uint32_t *pAddress)
{
	RegisterVariable(strPathName, STXVariableTreeNodeType_DWord, pAddress, false);
}
