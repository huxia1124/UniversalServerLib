//////////////////////////////////////////////////////////////////////////
// Code by XiaHuan
//
// XMLNode.h: interface for the CXMLNode class.
//
// XML 文件操作类的接口定义
//
//////////////////////////////////////////////////////////////////////

#pragma once
#pragma warning(disable:4786)

#include <atlbase.h>

#import <msxml3.dll> raw_interfaces_only

using namespace MSXML2;

#include <string>
#include <vector>
#include <map>

using namespace std;

#ifdef UNICODE
#define STLSTRING wstring
#define COPYSTRING(xTarget, xSrc)\
xTarget = xSrc;
#else
#define STLSTRING string
#define COPYSTRING(xTarget, xSrc)\
{\
	int cbLenStr = WideCharToMultiByte(CP_ACP, 0, xSrc, -1, NULL, 0, NULL, NULL);\
	char *pMStr = new char[cbLenStr];\
	WideCharToMultiByte(CP_ACP, 0, xSrc, -1, pMStr, cbLenStr, NULL, NULL);\
	xTarget = pMStr;\
	delete []pMStr;\
}
#endif

//////////////////////////////////////////////////////////////////////////
class CXMLAttribute
{
	friend class CXMLNode;
public:
	CXMLAttribute();
	virtual ~CXMLAttribute();

protected:
	CComQIPtr <MSXML2::IXMLDOMNode> m_spINode;
	STLSTRING m_strValue;

public:
	LPCTSTR GetValue();			//Get attribute value text of this attribute node

};


//////////////////////////////////////////////////////////////////////////
class CXMLNode  
{
public:
	CXMLNode();
	virtual ~CXMLNode();
	CXMLNode(const CXMLNode& val);

protected:
	CComQIPtr <MSXML2::IXMLDOMNode> m_spINode;
	CComPtr <MSXML2::IXMLDOMNodeList> m_spIChildList;
	CComQIPtr <MSXML2::IXMLDOMElement> m_spIElem;
	STLSTRING m_strName;
	STLSTRING m_strText;
	vector <CXMLNode*> m_arrChildNodes;
	map <STLSTRING, CXMLAttribute*> m_mapAttributes;
	CComPtr <MSXML2::IXMLDOMDocument2> m_spIXMLDOMDocument;
	STLSTRING m_strXML;

private:
	void InitChildren();

public:
	CXMLNode& operator=(const CXMLNode& val);

	BOOL Create(LPCTSTR lpszRootNodeName = NULL);		//Create an XML document as <xmlRoot></xmlRoot>. this CXMLNode will refer to <xmlRoot>
	BOOL Load(LPCTSTR lpszXMLString);					//Load an XML document from XML string
	BOOL LoadFromFile(LPCTSTR lpszXMLFilePath);					//Load an XML document from a file
	CXMLNode* GetChildNode(int iIndex);					//Retrieve a children by index. NULL if not exists
	CXMLNode* GetChildNode(LPCTSTR lpszChildName);		//Retrieve a children by Name. NULL if not exists.If multiple nodes exist, only the first one returned.
	LONG GetChildCount();								
	LPCTSTR GetNodeName();								//Retrieve the tag name of this node
	LPCTSTR GetNodeText();								//Retrieve the content text of this node. <NodeTag>Content Text here</NodeTag>

	LPCTSTR GetAttributeValue(LPCTSTR lpszAttributeName);		//Retrieve the specified attribute value text
	CXMLAttribute* GetAttribute(LPCTSTR lpszAttributeName);		//Retrieve the specified attribute Node

	CXMLNode* AppendChild(LPCTSTR lpszName, LPCTSTR lpszText = NULL);	//Append a child to this node
	CXMLAttribute* AppendAttr(LPCTSTR lpszName, LPCTSTR lpszValue);		//Append an attribute to this node


	BOOL SetNodeName(LPCTSTR lpszName);		//Not implemented yet!
	LPCTSTR GetXML();						//Retrieve the current XML document string of this node
};

