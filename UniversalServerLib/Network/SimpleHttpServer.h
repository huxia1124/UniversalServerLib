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

#include <vector>
#include <string>
#include <string.h>

#include "STXIOCPServer.h"

//////////////////////////////////////////////////////////////////////////

// Split a string using chSplit, store each element into arrResult
int SplitString( const char *pszSrc, char chSplit, std::vector<std::string> &arrResult );

//////////////////////////////////////////////////////////////////////////
// CHTTPRequestParserEventHandler
// 
//

class CHTTPRequestParserEventHandler
{
	friend class CHTTPRequestParser;
protected:
	virtual void OnHttpRequestInternal(const char *pszHeader, void* pUserData) = 0;
	virtual void OnHttpRequestError(const char *pszReason, void* pUserData) = 0;
};

//////////////////////////////////////////////////////////////////////////
// CHTTPRequestParser
// used to parse HTTP Request
//

class CHTTPRequestParser
{
public:
	CHTTPRequestParser(CHTTPRequestParserEventHandler *pEvtHandler, unsigned int nMaxHeaderSize = 4096);

protected:
	unsigned int _maxHeaderSize;
	CHTTPRequestParserEventHandler *_pEventHandler;
	int _iStage;
	std::vector<char> _arrContent;

protected:
	void OnError(const char *pszReason, void* pUserData);
	void OnMessageEnd(void* pUserData);

public:
	BOOL AppendChar(char ch, void* pUserData);
//	void AppendString(const char *pszString);
};

//////////////////////////////////////////////////////////////////////////

class CSimpleHttpClient : public CSTXIOCPServerClientContext, public CHTTPRequestParserEventHandler
{
	friend class CSimpleHttpServer;

public:
	CSimpleHttpClient() : m_reqParser(this)
	{
		InitializeCriticalSection(&m_cs);
	}
	virtual ~CSimpleHttpClient()
	{
		DeleteCriticalSection(&m_cs);
	}

protected:
	CHTTPRequestParser m_reqParser;

protected:
	virtual void OnHttpRequestInternal(const char *pszHeader, void* pUserData);
	virtual void OnHttpRequestError(const char *pszReason, void* pUserData);

protected:
	CRITICAL_SECTION m_cs;
	CSTXIOCPBuffer m_buffer;
};

class CSimpleHttpServer : public CSTXIOCPServer
{
	friend class CSimpleHttpClient;
public:
	CSimpleHttpServer();

protected:
	std::string _httpServerName;

protected:
	virtual CSTXIOCPServerClientContext *OnCreateClientContext(shared_ptr<CSTXIOCPTcpServerContext> pServerContext);
	virtual DWORD IsClientDataReadable(shared_ptr<CSTXIOCPServerClientContext> pClientContext);
	virtual BOOL OnClientReceived(CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer);

protected:
	void OnHttpRequestInternal(const char *pszHeader, void* pUserData);
	void OnHttpRequestError(const char *pszReason, void* pUserData);

protected:
	//Derived class should override this method to handle HTTP request
	virtual void OnHttpRequest(CSTXIOCPServerClientContext *pClientContext, std::map<std::string, std::string> &mapRequest);

protected:
	void Parse(const char *pszHeader, void* pUserData);
	void SendReplyMessage(CSTXIOCPServerClientContext *pClientContext, std::string strReplyLine);

public:
	void SendResponseData(CSTXIOCPServerClientContext *pClientContext, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen);


protected:
	void OnMessage(std::map<std::string, std::string> &mapRequest, void* pUserData);
};
