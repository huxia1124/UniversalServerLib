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
};

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
	// 9: set<wstring>
	// 10: vector<wstring>
	STXVariableTreeNodeType _type;
	void* _ptr;
	CSTXMemoryVariableNode *_parentNode = nullptr;
	std::wstring _name;
	bool _managedValue = false;		//if true, the data in _ptr should be deleted by this object

	CSTXHashMap<std::wstring, std::shared_ptr<CSTXMemoryVariableNode>, 8, 1, CSTXDefaultWStringHash<8, 1>> _mapContent;

protected:
	bool IsNewTypeAcceptable(int newType);

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
	bool IsValueExists();

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
	void RegisterStringVariable(std::wstring strPathName, std::wstring value);
	void RegisterStringVectorVariable(std::wstring strPathName, std::vector<std::wstring> value);
	void RegisterStringVectorVariable(std::wstring strPathName);
	void RegisterStringSetVariable(std::wstring strPathName, std::set<std::wstring> value);
	void RegisterStringSetVariable(std::wstring strPathName);
	void RegisterIntegerVariable(std::wstring strPathName, int32_t value);
	void RegisterIntegerVariable(std::wstring strPathName);
	void RegisterDoubleVariable(std::wstring strPathName, double value);
	void RegisterDoubleVariable(std::wstring strPathName);
	void RegisterIntegerVectorVariable(std::wstring strPathName, std::vector<int64_t> value);
	void RegisterIntegerVectorVariable(std::wstring strPathName);
	void RegisterIntegerSetVariable(std::wstring strPathName, std::set<int64_t> value);
	void RegisterIntegerSetVariable(std::wstring strPathName);


public:
	std::wstring GetName();
	void SetStringValue(std::wstring strPathName, std::wstring strValue);
	std::wstring GetStringValue(std::wstring strPathName);
	std::wstring GetThisStringValue();
	void SetThisStringValue(std::wstring strValue);

	size_t GetThisValues(std::vector<std::wstring> *values);
	size_t GetThisValues(std::vector<int64_t> *values);

	void AddStringValue(std::wstring strPathName, std::wstring strValue);
	void AddIntegerValue(std::wstring strPathName, int64_t value);

	void GetChildren(std::vector<std::shared_ptr<CSTXMemoryVariableNode>>* children);

	int32_t GetIntegerValue(std::wstring strPathName);
	void SetIntegerValue(std::wstring strPathName, int32_t value);
	int32_t GetThisIntegerValue();
	void SetThisIntegerValue(int32_t value);

	double GetDoubleValue(std::wstring strPathName);
	void SetDoubleValue(std::wstring strPathName, double value);
};