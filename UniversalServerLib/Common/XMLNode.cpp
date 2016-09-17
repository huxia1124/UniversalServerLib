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


#include "XMLNode.h"

//////////////////////////////////////////////////////////////////////////
CXMLAttribute::CXMLAttribute()
{
}

CXMLAttribute::~CXMLAttribute()
{

}

LPCTSTR CXMLAttribute::GetValue()
{
	return m_strValue.c_str();
}

//////////////////////////////////////////////////////////////////////////

CXMLNode::CXMLNode()
{

}

CXMLNode::~CXMLNode()
{
	map<STLSTRING, CXMLAttribute*>::iterator itAttr = m_mapAttributes.begin();
	for(;itAttr!=m_mapAttributes.end();itAttr++)
	{
		delete itAttr->second;
	}
	m_mapAttributes.clear();
	
	vector<CXMLNode*>::iterator itChild = m_arrChildNodes.begin();
	for(;itChild!=m_arrChildNodes.end();itChild++)
	{
		delete (*itChild);
	}
	m_arrChildNodes.clear();
}

BOOL CXMLNode::Create(LPCTSTR lpszRootNodeName)
{
	if(m_spIXMLDOMDocument == NULL)
	{
		m_spIXMLDOMDocument.CoCreateInstance(__uuidof(DOMDocument30), NULL, CLSCTX_INPROC_SERVER);
		if(m_spIXMLDOMDocument == NULL)
			return FALSE;
		HRESULT hr = m_spIXMLDOMDocument->put_async(VARIANT_FALSE);
	}

	CComQIPtr <MSXML2::IXMLDOMNode> spRoot = m_spIXMLDOMDocument;

	CComPtr <MSXML2::IXMLDOMElement> pElem = NULL;
	CComQIPtr <MSXML2::IXMLDOMNode> pNode;
	CComQIPtr <MSXML2::IXMLDOMNode> pNodeOut;

	if(lpszRootNodeName)
		m_spIXMLDOMDocument->createElement(CComBSTR(lpszRootNodeName), &pElem);
	else
		m_spIXMLDOMDocument->createElement(CComBSTR("xmlRoot"), &pElem);


	spRoot->appendChild(pNode, &pNodeOut);

	m_spINode = pElem;
	m_spIElem = m_spINode;

	m_spINode->get_childNodes(&m_spIChildList);
	BSTR pStr = NULL;
	m_spINode->get_nodeName(&pStr);
	if(pStr)
	{
		COPYSTRING(m_strName, pStr)
			SysFreeString(pStr);
	}
	InitChildren();
	return TRUE;
}

BOOL CXMLNode::Load(LPCTSTR lpszXMLString)
{
	HRESULT hr = S_OK;
	if(m_spIXMLDOMDocument == NULL)
	{
		hr = m_spIXMLDOMDocument.CoCreateInstance(__uuidof(DOMDocument30), NULL, CLSCTX_INPROC_SERVER);
		if(hr == CO_E_NOTINITIALIZED)
			OutputDebugString(_T("CXMLNode::Load() Failed! Please call CoInitialize() first!\n"));
		if(m_spIXMLDOMDocument == NULL)
			return FALSE;
		hr = m_spIXMLDOMDocument->put_async(VARIANT_FALSE);
	}

	VARIANT_BOOL bSuccess = VARIANT_FALSE;

	hr = m_spIXMLDOMDocument->loadXML(CComBSTR(lpszXMLString), &bSuccess);

	if(FAILED(hr))
		return FALSE;

	m_spIXMLDOMDocument->QueryInterface(&m_spINode);
	m_spIElem = m_spINode;
	m_spINode->get_childNodes(&m_spIChildList);
	BSTR pStr = NULL;
	m_spINode->get_nodeName(&pStr);
	if(pStr)
	{
		COPYSTRING(m_strName, pStr)
		SysFreeString(pStr);
	}

	return TRUE;
}

BOOL CXMLNode::LoadFromFile(LPCTSTR lpszXMLFilePath)
{
	HRESULT hr = S_OK;
	if(m_spIXMLDOMDocument == NULL)
	{
		hr = m_spIXMLDOMDocument.CoCreateInstance(__uuidof(DOMDocument30), NULL, CLSCTX_INPROC_SERVER);
		if(hr == CO_E_NOTINITIALIZED)
			OutputDebugString(_T("CXMLNode::LoadFromFile() Failed! Please call CoInitialize() first!\n"));
		if(m_spIXMLDOMDocument == NULL)
			return FALSE;
		hr = m_spIXMLDOMDocument->put_async(VARIANT_FALSE);
	}
	
	VARIANT_BOOL bSuccess = VARIANT_FALSE;
	
	hr = m_spIXMLDOMDocument->load(CComVariant(lpszXMLFilePath), &bSuccess);
		
	if(FAILED(hr))
		return FALSE;
	
	m_spIXMLDOMDocument->QueryInterface(&m_spINode);
	m_spIElem = m_spINode;
	m_spINode->get_childNodes(&m_spIChildList);
	BSTR pStr = NULL;
	m_spINode->get_nodeName(&pStr);
	if(pStr)
	{
		COPYSTRING(m_strName, pStr)
			SysFreeString(pStr);
	}
	
	return TRUE;
}

void CXMLNode::InitChildren()
{
	LONG nChildCount = GetChildCount();
	if(nChildCount == 0 || m_spIChildList == NULL)
		return;

	m_arrChildNodes.resize(nChildCount);
	
	for(LONG i=0;i<nChildCount;i++)
	{
		m_arrChildNodes[i] = new CXMLNode();
		m_spIChildList->get_item(i, &m_arrChildNodes[i]->m_spINode);
		m_arrChildNodes[i]->m_spIElem = m_arrChildNodes[i]->m_spINode;
		m_arrChildNodes[i]->m_spINode->get_childNodes(&m_arrChildNodes[i]->m_spIChildList);
		m_arrChildNodes[i]->m_spIXMLDOMDocument = m_spIXMLDOMDocument;
		BSTR pStr = NULL;
		m_arrChildNodes[i]->m_spINode->get_nodeName(&pStr);
		if(pStr)
		{
			COPYSTRING(m_arrChildNodes[i]->m_strName, pStr)
			SysFreeString(pStr);
			pStr = NULL;
		}
		m_arrChildNodes[i]->m_spINode->get_text(&pStr);
		if(pStr)
		{
			COPYSTRING(m_arrChildNodes[i]->m_strText, pStr)
			SysFreeString(pStr);
		}
	}
}

CXMLNode* CXMLNode::GetChildNode(int iIndex)
{
	if(m_arrChildNodes.size() == 0)
		InitChildren();
	if(m_arrChildNodes.size() < (UINT)iIndex + 1)
		return NULL;

	return m_arrChildNodes[iIndex];
}

CXMLNode* CXMLNode::GetChildNode(LPCTSTR lpszChildName)
{
	if(m_arrChildNodes.size() == 0)
		InitChildren();

	vector<CXMLNode*>::iterator it = m_arrChildNodes.begin();
	for(;it!=m_arrChildNodes.end();it++)
	{
		if((*it)->m_strName == lpszChildName)
			return (*it);
	}
	return NULL;
}

LONG CXMLNode::GetChildCount()
{
	long nLength = 0;
	m_spIChildList->get_length(&nLength);
	return nLength;
}

LPCTSTR CXMLNode::GetNodeName()
{
	return m_strName.c_str();
}

LPCTSTR CXMLNode::GetNodeText()
{
	return m_strText.c_str();
}

CXMLAttribute* CXMLNode::GetAttribute(LPCTSTR lpszAttributeName)
{
	if(m_spIElem == NULL)
		return NULL;
	
	map<STLSTRING, CXMLAttribute*>::iterator it = m_mapAttributes.find(lpszAttributeName);
	if(it != m_mapAttributes.end())
		return it->second;
	
	CComPtr <MSXML2::IXMLDOMAttribute> pAttribute;
	m_spIElem->getAttributeNode(CComBSTR(lpszAttributeName), &pAttribute);
	if(pAttribute == NULL)
		return NULL;

	m_mapAttributes[lpszAttributeName] = new CXMLAttribute();
	BSTR pStr = NULL;
	pAttribute->get_text(&pStr);
	if(pStr)
	{
		COPYSTRING(m_mapAttributes[lpszAttributeName]->m_strValue , pStr);
		SysFreeString(pStr);
	}
	CXMLAttribute* pAttr = m_mapAttributes[lpszAttributeName];
	pAttr->m_spINode = pAttribute;
	return pAttr;
}

LPCTSTR CXMLNode::GetAttributeValue(LPCTSTR lpszAttributeName)
{
	CXMLAttribute* pAttr = GetAttribute(lpszAttributeName);
	if(pAttr == NULL)
		return _T("");

	return pAttr->m_strValue.c_str();
}

CXMLNode* CXMLNode::AppendChild(LPCTSTR lpszName, LPCTSTR lpszText)
{
	CComPtr <MSXML2::IXMLDOMElement> pElem = NULL;
	CComQIPtr <MSXML2::IXMLDOMNode> pNodeOut;
	m_spIXMLDOMDocument->createNode(CComVariant(MSXML2::NODE_ELEMENT), CComBSTR(lpszName), CComBSTR(""), &pNodeOut);

	if(pNodeOut == NULL)
		return NULL;

	CComQIPtr <MSXML2::IXMLDOMNode> pNodeOut2;
	HRESULT hr = m_spINode->insertBefore(pNodeOut, CComVariant((IUnknown*)NULL), &pNodeOut2);
	if(pNodeOut == NULL)
		return NULL;

	m_arrChildNodes.push_back(new CXMLNode());

	CXMLNode* pNodeNew = m_arrChildNodes.back();
	int nCurrentCount = (int)m_arrChildNodes.size() - 1;

	pNodeNew->m_spINode = pNodeOut;

	pNodeNew->m_spIElem = pNodeOut;
	pNodeNew->m_spINode->get_childNodes(&m_arrChildNodes[nCurrentCount]->m_spIChildList);
	pNodeNew->m_spIXMLDOMDocument = m_spIXMLDOMDocument;
	pNodeNew->m_strName = lpszName;
	if(lpszText)
	{
		pNodeNew->m_strText = lpszText;
		pNodeOut->put_text(CComBSTR(lpszText));
	}

	return pNodeNew;
}

CXMLNode& CXMLNode::operator=(const CXMLNode& val)
{
	return *this;
}

CXMLNode::CXMLNode(const CXMLNode& val)
{
	
}

LPCTSTR CXMLNode::GetXML()
{
	BSTR pStr = NULL;
	m_spINode->get_xml(&pStr);
	if(pStr)
	{
		m_strXML = pStr;
		SysFreeString(pStr);
	}
	return m_strXML.c_str();
}

CXMLAttribute* CXMLNode::AppendAttr(LPCTSTR lpszName, LPCTSTR lpszValue)
{
	map<STLSTRING, CXMLAttribute*>::iterator it = m_mapAttributes.find(lpszName);
	if(it != m_mapAttributes.end())
		return it->second;


	CComPtr <MSXML2::IXMLDOMAttribute> spAttr;
	m_spIXMLDOMDocument->createAttribute(CComBSTR(lpszName), &spAttr);
	if(spAttr == NULL)
		return NULL;

	spAttr->put_value(CComVariant(lpszValue));

	CComPtr <MSXML2::IXMLDOMAttribute> spAttrOut;
	m_spIElem->setAttributeNode(spAttr, &spAttrOut);
	m_mapAttributes[lpszName] = new CXMLAttribute();
	m_mapAttributes[lpszName]->m_spINode = spAttrOut;

	it = m_mapAttributes.find(lpszName);

	it->second->m_strValue = lpszValue;
	return it->second;

}

BOOL CXMLNode::SetNodeName(LPCTSTR lpszName)
{
	//Not implemented yet
	//Always return FALSE
	OutputDebugString(_T("CXMLNode::SetNodeName() Failed! This method is not implemented!\n"));
	::SetLastError(ERROR_NOT_SUPPORTED);

	return FALSE;
}