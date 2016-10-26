#pragma once

#include <string>
#include <cstdint>
#include "STXUtility.h"
#include <memory>
#include <vector>
#include <set>


//////////////////////////////////////////////////////////////////////////
// CSTXMemoryVariableNode

enum STXVariableTreeNodeType
{
	STXVariableTreeNodeType_Invalid = -1,
	STXVariableTreeNodeType_Int32 = 1,
	STXVariableTreeNodeType_Int64 = 2,
	STXVariableTreeNodeType_WString = 3,
	STXVariableTreeNodeType_Int = 4,
	STXVariableTreeNodeType_Float = 5,
	STXVariableTreeNodeType_Double = 6,
	STXVariableTreeNodeType_Word = 7,
	STXVariableTreeNodeType_DWord = 8,
	STXVariableTreeNodeType_WStringVector = 9,
	STXVariableTreeNodeType_WStringSet = 10,
	STXVariableTreeNodeType_IntegerVector = 11,
	STXVariableTreeNodeType_IntegerSet = 12,
	STXVariableTreeNodeType_DoubleVector = 13,
	STXVariableTreeNodeType_DoubleSet = 14,
	STXVariableTreeNodeType_IntFunction = 15,
	STXVariableTreeNodeType_Custom = 30000,
};

#define STXMEMVAR_FLAG_READONLY			0x00000001

class CSTXMemoryVariableNode
{
public:
	CSTXMemoryVariableNode();
	CSTXMemoryVariableNode(std::wstring name);
	~CSTXMemoryVariableNode();

protected:
	class CWStringHash
	{
	public:
		size_t operator()(const std::wstring &val)
		{
			return std::hash<std::wstring>()(val);
		}
	};

protected:

	//Types: see STXVariableTreeNodeType enum
	// 1: int32
	// 2: int64
	// 3: wstring
	// 4: int (c++ standard int)
	// 5: float (c++ standard float)
	// 6: double (c++ standard double)
	// 7: WORD
	// 8: DWORD
	// 9: vector<wstring>
	// 10: set<wstring>
	// 11: vector<int64_t>
	// 12: set<int64_t>
	// 13: vector<double>
	// 14: set<double>
	// 30000: Custom value
	STXVariableTreeNodeType _type;

	const type_info *_varType = nullptr;			//used for custom type
	void* _varDestructor = nullptr;		//used for custom type

	void* _ptr;
	CSTXMemoryVariableNode *_parentNode = nullptr;
	std::wstring _name;
	bool _managedValue = false;		//if true, the data in _ptr should be deleted by this object
	bool _readOnly = false;
	std::recursive_mutex _value_mtx;	//used when modifying node value

	CSTXHashMap<std::wstring, std::shared_ptr<CSTXMemoryVariableNode>, 8, 1, CSTXDefaultWStringHash<8, 1>> _mapContent;


public:
	template<typename ReturnType, typename...ArgsOriginal, typename...Args>
	ReturnType Call(ReturnType(CSTXMemoryVariableNode::*pfn)(ArgsOriginal...argsOriginal), std::wstring strPathName, Args...args)
	{
		std::vector<wchar_t*> pathArray;
		wchar_t *str = (wchar_t*)strPathName.data();
		wchar_t* pch;
		wchar_t *next_token = NULL;
		pch = _tcstok_s(str, _T("\\/"), &next_token);
		while (pch != NULL)
		{
			pathArray.push_back(pch);
			pch = _tcstok_s(NULL, _T("\\/"), &next_token);
		}
		pathArray.push_back(nullptr);

		return (this->*pfn)(&pathArray[0], args...);
	}

protected:
	bool IsNewTypeAcceptable(int newType);
	std::wstring GetStringValue(void *ptr, int dataType);
	int64_t IncreaseIntegerValue(void *ptr, int dataType, int64_t delta);
	void RemoveStringValue(void *ptr, int dataType, std::wstring strValue);
	void RemoveIntegerValue(void *ptr, int dataType, int64_t value);
	void LockValue();
	void UnlockValue();
	bool IsContainStringValue(void *ptr, int dataType, std::wstring strValue);
	bool IsContainIntegerValue(void *ptr, int dataType, int64_t value);
	void ClearValue(void *ptr, int dataType);
	uint32_t GetVariableFlags(CSTXMemoryVariableNode *node);

protected:
	std::shared_ptr<CSTXMemoryVariableNode> _RegisterVariable(wchar_t **pathArray, STXVariableTreeNodeType nType, void* pAddress, bool managed);
	std::shared_ptr<CSTXMemoryVariableNode> _GetVariableNode(wchar_t **pathArray);
	void _UnregisterVariable(wchar_t **pathArray);



public:
	std::shared_ptr<CSTXMemoryVariableNode> RegisterVariable(std::wstring strPathName, STXVariableTreeNodeType nType, void* pAddress, bool managed);
	std::shared_ptr<CSTXMemoryVariableNode> GetVariableNode(std::wstring strPathName);
	void* GetVariablePtr(std::wstring strPathName);
	int GetVariableType(std::wstring strPathName);
	int GetThisVariableType();
	void UnregisterVariable(std::wstring strPathName);
	int GetChildrenNames(std::vector<std::wstring> *pArrNames);
	std::wstring GetFullPath();
	bool IsManagedValue();
	bool IsThisValueExists();
	bool IsValueExists(std::wstring strPathName);

public:
	void RegisterInt32Variable(std::wstring strPathName, int32_t *pAddress);
	void RegisterInt64Variable(std::wstring strPathName, int64_t *pAddress);
	void RegisterStringVariable(std::wstring strPathName, std::wstring *pAddress);
	void RegisterIntVariable(std::wstring strPathName, int *pAddress);
	void RegisterFloatVariable(std::wstring strPathName, float *pAddress);
	void RegisterDoubleVariable(std::wstring strPathName, double *pAddress);
	void RegisterWordVariable(std::wstring strPathName, uint16_t *pAddress);
	void RegisterDWordVariable(std::wstring strPathName, uint32_t *pAddress);

public:
	void RegisterInt32Variable(std::wstring strPathName, int32_t value);
	void RegisterInt64Variable(std::wstring strPathName, int64_t value);
	void RegisterStringVariable(std::wstring strPathName, std::wstring value);
	void RegisterStringVectorVariable(std::wstring strPathName, std::vector<std::wstring> value);
	void RegisterStringVectorVariable(std::wstring strPathName);
	void RegisterStringSetVariable(std::wstring strPathName, std::set<std::wstring> value);
	void RegisterStringSetVariable(std::wstring strPathName);
	void RegisterIntegerVariable(std::wstring strPathName, int64_t value);
	void RegisterIntegerVariable(std::wstring strPathName);
	void RegisterIntegerVariable(std::wstring strPathName, std::function<int64_t(void)> pfnValueGet, std::function<void(int64_t)> pfnValueSet = nullptr);
	void RegisterDoubleVariable(std::wstring strPathName, double value);
	void RegisterDoubleVariable(std::wstring strPathName);
	void RegisterIntegerVectorVariable(std::wstring strPathName, std::vector<int64_t> value);
	void RegisterIntegerVectorVariable(std::wstring strPathName);
	void RegisterIntegerSetVariable(std::wstring strPathName, std::set<int64_t> value);
	void RegisterIntegerSetVariable(std::wstring strPathName);
	void RegisterDoubleVectorVariable(std::wstring strPathName, std::vector<double> value);
	void RegisterDoubleVectorVariable(std::wstring strPathName);
	void RegisterDoubleSetVariable(std::wstring strPathName, std::set<double> value);
	void RegisterDoubleSetVariable(std::wstring strPathName);

	template<typename CustomType>
	void RegisterCustomVariable(std::wstring strPathName, CustomType value)
	{
		auto nodeExists = GetVariableNode(strPathName);
		if (nodeExists)
		{
			if (!nodeExists->IsNewTypeAcceptable(STXVariableTreeNodeType_Custom))
			{
				throw std::runtime_error("The node already has a value of different type.");
			}
			return;
		}
		CustomType *v = new CustomType(value);
		auto var = RegisterVariable(strPathName, STXVariableTreeNodeType_Custom, v, true);
		var->_managedValue = true;
		var->_varType = &typeid(value);
		var->_varDestructor = (void*)&deleter<CustomType>;
	}

	template<typename T>
	static void deleter(void *ptr)
	{
		delete static_cast<T*>(ptr);
	}

public:
	std::wstring GetName();
	void SetStringValue(std::wstring strPathName, std::wstring strValue);
	std::wstring GetStringValue(std::wstring strPathName);
	std::wstring GetThisStringValue();
	void SetThisStringValue(std::wstring strValue);

	size_t GetThisValues(std::vector<std::wstring> *values);
	size_t GetThisValues(std::vector<int64_t> *values);
	size_t GetThisValues(std::vector<double> *values);

	void AddStringValue(std::wstring strPathName, std::wstring strValue);
	void AddIntegerValue(std::wstring strPathName, int64_t value);
	void AddDoubleValue(std::wstring strPathName, double value);

	void RemoveStringValue(std::wstring strPathName, std::wstring strValue);
	void RemoveThisStringValue(std::wstring strValue);
	void RemoveIntegerValue(std::wstring strPathName, int64_t value);
	void RemoveThisIntegerValue(int64_t value);

	bool IsContainStringValue(std::wstring strPathName, std::wstring strValue);
	bool IsContainIntegerValue(std::wstring strPathName, int64_t value);
	bool IsContainThisStringValue(std::wstring strValue);
	bool IsContainThisIntegerValue(int64_t value);
	void ClearValue(std::wstring strPathName);

	void GetChildren(std::vector<std::shared_ptr<CSTXMemoryVariableNode>>* children);

	int64_t GetIntegerValue(std::wstring strPathName);
	void SetIntegerValue(std::wstring strPathName, int64_t value);
	int64_t GetThisIntegerValue();
	void SetThisIntegerValue(int64_t value);

	double GetDoubleValue(std::wstring strPathName);
	void SetDoubleValue(std::wstring strPathName, double value);
	void SetThisDoubleValue(double value);

	int64_t IncreaseIntegerValue(std::wstring strPathName, int64_t delta);
	int64_t IncreaseThisIntegerValue(int64_t delta);

	template<typename CustomType>
	CustomType GetCustomValue(std::wstring strPathName, CustomType defaultValue)
	{
		auto pNode = GetVariableNode(strPathName);
		if (pNode == NULL || pNode->_ptr == NULL || pNode->_type < 0)
			return defaultValue;

		if (pNode->_type != STXVariableTreeNodeType_Custom)
			return defaultValue;

		if(typeid(CustomType) != *pNode->_varType)
			return defaultValue;

		return (*((CustomType*)pNode->_ptr));
	}

	size_t GetThisChildrenCount();
	size_t GetChildrenCount(std::wstring strPathName);
	bool IsReadOnlyVariable(std::wstring strPathName);
	bool IsThisReadOnlyVariable();
	void SetVariableReadOnly(std::wstring strPathName, bool readOnly = true);
	void SetThisVariableReadOnly(bool readOnly = true);

	uint32_t GetVariableFlags(std::wstring strPathName);
	uint32_t GetThisVariableFlags();

};