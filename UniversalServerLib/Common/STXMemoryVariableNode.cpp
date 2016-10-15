#include "stdafx.h"
#include "STXMemoryVariableNode.h"
#include <tchar.h>


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

	const int buflen = 256;
	TCHAR szTmp[buflen];
	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		_itot_s(*((int32_t*)pNode->_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		_i64tot_s(*((int64_t*)pNode->_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		return *((std::wstring*)pNode->_ptr);
		break;
	case STXVariableTreeNodeType_Int:		//int
		_itot_s(*((int*)pNode->_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Float:		//float
		_stprintf_s(szTmp, _T("%f"), *((float*)pNode->_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Double:		//double
		_stprintf_s(szTmp, _T("%lf"), *((double*)pNode->_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		_ultot_s(*((uint16_t*)pNode->_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		_ultot_s(*((uint32_t*)pNode->_ptr), szTmp, buflen, 10);
		return szTmp;
		break;

	default:
		break;
	}

	return _T("");
}

std::wstring CSTXMemoryVariableNode::GetThisStringValue()
{
	const int buflen = 256;
	TCHAR szTmp[buflen];
	switch (_type)
	{
	case STXVariableTreeNodeType_Int32:		//int32
		_itot_s(*((int32_t*)_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Int64:		//int64
		_i64tot_s(*((int64_t*)_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_WString:		//wstring
		return *((std::wstring*)_ptr);
		break;
	case STXVariableTreeNodeType_Int:		//int
		_itot_s(*((int*)_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_Float:		//float
		_stprintf_s(szTmp, _T("%f"), *((float*)_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Double:		//double
		_stprintf_s(szTmp, _T("%lf"), *((double*)_ptr));
		return szTmp;
		break;
	case STXVariableTreeNodeType_Word:		//uint16_t
		_ultot_s(*((uint16_t*)_ptr), szTmp, buflen, 10);
		return szTmp;
		break;
	case STXVariableTreeNodeType_DWord:		//uint32_t
		_ultot_s(*((uint32_t*)_ptr), szTmp, buflen, 10);
		return szTmp;
		break;

	default:
		break;
	}

	return _T("");
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
		*((std::wstring*)_ptr) = strValue;
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

	default:
		break;
	}

	return values->size();
}

size_t CSTXMemoryVariableNode::GetThisValues(std::vector<int32_t> *values)
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

	switch (pNode->_type)
	{
	case STXVariableTreeNodeType_WStringVector:		//vector<wstring>
		(*((std::vector<std::wstring>*)pNode->_ptr)).push_back(strValue);
		break;
	case STXVariableTreeNodeType_WStringSet:		//set<wstring>
		(*((std::set<std::wstring>*)pNode->_ptr)).insert(strValue);
		break;
	}
}

void CSTXMemoryVariableNode::GetChildren(std::vector<std::shared_ptr<CSTXMemoryVariableNode>>* children)
{
	_mapContent.foreach([&](std::pair<std::wstring, std::shared_ptr<CSTXMemoryVariableNode>> item) {
		children->push_back(item.second);
	});
}

int32_t CSTXMemoryVariableNode::GetIntegerValue(std::wstring strPathName)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return 0;

	if (pNode->_type != STXVariableTreeNodeType_Int32)
		return 0;

	return (*((int32_t*)pNode->_ptr));
}

void CSTXMemoryVariableNode::SetIntegerValue(std::wstring strPathName, int32_t value)
{
	auto pNode = GetVariableNode(strPathName);
	if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
		return;

	if (pNode->_type != STXVariableTreeNodeType_Int32)
		return;

	*((int32_t*)pNode->_ptr) = value;
}

int32_t CSTXMemoryVariableNode::GetThisIntegerValue()
{
	if (_type != STXVariableTreeNodeType_Int32)
		return 0;

	return (*((int32_t*)_ptr));
}

void CSTXMemoryVariableNode::SetThisIntegerValue(int32_t value)
{
	if (_type != STXVariableTreeNodeType_Int32)
		return;

	*((int32_t*)_ptr) = value;
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

bool CSTXMemoryVariableNode::IsValueExists()
{
	return _type != STXVariableTreeNodeType_Invalid;
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

void CSTXMemoryVariableNode::RegisterIntegerVariable(std::wstring strPathName, int32_t value)
{
	RegisterInt32Variable(strPathName, value);

}

void CSTXMemoryVariableNode::RegisterIntegerVariable(std::wstring strPathName)
{
	RegisterInt32Variable(strPathName, 0);
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
