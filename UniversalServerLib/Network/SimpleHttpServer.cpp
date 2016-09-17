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


#include "SimpleHttpServer.h"
#include <Shlwapi.h>

//////////////////////////////////////////////////////////////////////////

int SplitString( const char *pszSrc, char chSplit, std::vector<std::string> &arrResult )
{
	const char *pValue = pszSrc;
	size_t nLen = strlen(pszSrc);
	if(nLen > 0)
	{
		char *pValueCopy = new char[nLen + 2];
		strcpy_s(pValueCopy, nLen + 2, pValue);

		char seps[2] = {0};
		seps[0] = chSplit;
		seps[1] = 0;

		char *next_token = NULL;
		char *token;
		int iTokenPos = 0;
		token = strtok_s(pValueCopy, seps, &next_token);
		while( token != NULL )
		{
			arrResult.push_back(token);
			// Get next token:
			token = strtok_s(NULL, seps, &next_token);
		}
		delete []pValueCopy;
	}
	return (int)arrResult.size();
}

//////////////////////////////////////////////////////////////////////////

CHTTPRequestParser::CHTTPRequestParser(CHTTPRequestParserEventHandler *pEvtHandler, unsigned int nMaxHeaderSize)
{
	_maxHeaderSize = nMaxHeaderSize;
	_pEventHandler = pEvtHandler;
	_iStage = 0;
	_arrContent.reserve(256);	//reserve an initial space for performance optimization
}

void CHTTPRequestParser::OnError(const char *pszReason, void* pUserData)
{
	if(_pEventHandler)
		_pEventHandler->OnHttpRequestError(pszReason, pUserData);

}

BOOL CHTTPRequestParser::AppendChar(char ch, void* pUserData)
{
	//Parse each character until a double empty line encountered. (double empty line means HTTP header end)
	//HTTP header is stored in _arrContent

	switch(_iStage)
	{
	case 0:		//Initial Stage
		{
			if(ch >= 0x20 && ch <= 0x7E)
			{
				_iStage = 'C';
			}
			else if(ch < 0x20)
			{
				OnError("HTTP Request Header begins with an invalid character.", pUserData);
				return FALSE;
			}
		}
		break;
	case 'C':
		{
			if(ch >= 0x20 && ch <= 0x7E)
			{
				//Do Nothing
			}
			else if(ch == '\r')
			{
				_iStage = 'E';
			}
		}
		break;
	case 'E':
		{
			if(ch >= 0x20 && ch <= 0x7E)
			{
				_iStage = 'C';
			}
			else if(ch == '\r')
			{
				_iStage = 'F';
			}
		}
		break;
	case 'F':
		{
			if(ch >= 0x20 && ch <= 0x7E)
			{
				_arrContent.push_back(0);
				OnMessageEnd(pUserData);
				_arrContent.clear();
				_iStage = 'C';
			}
			else if(ch == '\n')
			{
				_arrContent.push_back(0);
				OnMessageEnd(pUserData);
				_arrContent.clear();
				_iStage = 'C';
				return TRUE;
			}
			else if(ch == '\r')
			{
				_iStage = 'F';
			}
		}
		break;
	}
	_arrContent.push_back(ch);

	if(_maxHeaderSize > 0 && _arrContent.size() > _maxHeaderSize)
	{
		OnError("HTTP Request Header is too long.", pUserData);
		return FALSE;
	}

	return TRUE;
}

void CHTTPRequestParser::OnMessageEnd(void* pUserData)
{
	const char *psz = (const char*)&_arrContent[0];
	if(psz == NULL || psz[0] == 0)
		return;

	if(_pEventHandler)
		_pEventHandler->OnHttpRequestInternal(psz, pUserData);

}

//////////////////////////////////////////////////////////////////////////

CSimpleHttpServer::CSimpleHttpServer()
{
	_httpServerName = "SimpleHttpServer/1.0";
}

CSTXIOCPServerClientContext *CSimpleHttpServer::OnCreateClientContext( tr1::shared_ptr<CSTXIOCPTcpServerContext> pServerContext )
{
	return new CSimpleHttpClient();
}

DWORD CSimpleHttpServer::IsClientDataReadable( tr1::shared_ptr<CSTXIOCPServerClientContext> pClientContext )
{
	return pClientContext->GetBufferedMessageLength();		//for text stream protocol, Always readable. OnClientReceived would handle each incoming byte.
}

BOOL CSimpleHttpServer::OnClientReceived( CSTXIOCPServerClientContext *pClientContext, CSTXIOCPBuffer *pBuffer )
{
	CSimpleHttpClient *pClient = dynamic_cast<CSimpleHttpClient*>(pClientContext);
	DWORD dwLen = pBuffer->GetDataLength();
	const char *psz = (const char*)pBuffer->GetBufferPtr();
	for(DWORD i=0;i<dwLen;i++)
	{
		if(!pClient->m_reqParser.AppendChar(psz[i], pClientContext))
		{
			break;
		}
	}

	return TRUE;
}

void CSimpleHttpServer::OnMessage(std::map<std::string, std::string> &mapRequest, void* pUserData)
{
	OnHttpRequest((CSTXIOCPServerClientContext*)pUserData, mapRequest);
}

void CSimpleHttpServer::OnHttpRequest(CSTXIOCPServerClientContext *pClientContext, std::map<std::string, std::string> &mapRequest)
{
	char szDefaultResponse[256];
	int nLen = sprintf_s(szDefaultResponse, "This is %s. Please override CSimpleHttpServer::OnHttpRequest.", _httpServerName.c_str());
	if(nLen > 0)
	{
		SendResponseData(pClientContext, 200, "text", (LPVOID)szDefaultResponse, nLen);
	}

	//ShowCallStack();

	//BeginDownloadURL(_T("ximserver.oicp.net"), _T("/Report_W530.htm"), 1);
	//BeginDownloadURL(_T("10.10.26.129"), _T("/pic1.png"), 1);

	//AddTimerObject(this, 20000);

	//this->KillAllConnections();
	//Test:Error!!!!
	int *p = (int*)0x1777;
	*p = 1;
}

void CSimpleHttpServer::OnHttpRequestInternal( const char *pszHeader, void* pUserData)
{
	Parse(pszHeader, pUserData);
}

void CSimpleHttpServer::OnHttpRequestError(const char *pszReason, void* pUserData)
{
	if(pszReason && pUserData)
	{
		SendResponseData((CSTXIOCPServerClientContext*)pUserData, 500, "text", (LPVOID)pszReason, strlen(pszReason));
	}
}

void CSimpleHttpServer::Parse( const char *pszHeader, void* pUserData )
{
	std::map<std::string, std::string> mapRequestContext;
	int nLen = strlen(pszHeader);

	const char *pszStart = strstr(pszHeader, "/?");
	if(pszStart == NULL)
	{
		OnMessage(mapRequestContext, pUserData);
		return;
	}

	pszStart += 2;

	const char *pszEnd = strchr(pszStart, ' ');
	if(pszEnd == NULL)
	{
		OnMessage(mapRequestContext, pUserData);
		return;
	}

	char *pszCopy = new char[nLen + 2];
	memcpy(pszCopy, pszHeader, nLen + 1);

	pszCopy[pszEnd - pszHeader] = 0;
	const char *pszStartCopy = pszCopy + (pszStart - pszHeader);

	std::vector<std::string> arrParts;
	SplitString(pszStartCopy, '&', arrParts);

	std::vector<std::string>::iterator it = arrParts.begin();
	char szUnescape[1024];
	DWORD dwUnescapeLen = 1024;
	for(;it!=arrParts.end();it++)
	{
		dwUnescapeLen = 1024;
		std::vector<std::string> arrKeyValue;
		SplitString((*it).c_str(), '=', arrKeyValue);
		if(arrKeyValue.size() < 2)
			continue;

		UrlUnescapeA((char*)arrKeyValue[1].c_str(), szUnescape, &dwUnescapeLen, 0);

		mapRequestContext[arrKeyValue[0]] = szUnescape;
	}

	delete []pszCopy;

	OnMessage(mapRequestContext, pUserData);
}

void CSimpleHttpServer::SendReplyMessage( CSTXIOCPServerClientContext *pClientContext, std::string strReplyLine )
{
	//std::string strHeader = "HTTP/1.1 200 OK\r\nServer: SimpleHttpServer/1.0\r\nContent-Type: text/html\r\nContent-Length: ";
	//int nLen = strReplyLine.size();
	//char szLen[32];
	//_itoa_s(nLen, szLen, 10);
	//strHeader += szLen;
	//strHeader += "\r\n\r\n";

	//strHeader += strReplyLine;
	//SendClientData(pClientContext, (void*)strHeader.c_str(), strHeader.size());

	SendResponseData(pClientContext, 200, "text/html", (LPVOID)strReplyLine.c_str(), strReplyLine.size());
}

void CSimpleHttpServer::SendResponseData(CSTXIOCPServerClientContext *pClientContext, unsigned int nResponseCode, const char* pszContentType, LPVOID pData, unsigned int nDataLen)
{
	char szResponseHeader[256];
	int nLen = sprintf_s(szResponseHeader, "HTTP/1.1 %d OK\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", nResponseCode, _httpServerName.c_str(), pszContentType, nDataLen);

	if(nLen > 0)
	{
		SendClientData(pClientContext, (void*)szResponseHeader, nLen);

		if(nDataLen > 0)
		{
			SendClientData(pClientContext, pData, nDataLen);
		}
	}
}


//////////////////////////////////////////////////////////////////////////

void CSimpleHttpClient::OnHttpRequestInternal( const char *pszHeader, void* pUserData )
{
	CSimpleHttpServer *pServer = dynamic_cast<CSimpleHttpServer*>(m_pServer);
	if(pServer)
	{
		pServer->OnHttpRequestInternal(pszHeader, pUserData);
	}
}

void CSimpleHttpClient::OnHttpRequestError(const char *pszReason, void* pUserData)
{
	CSimpleHttpServer *pServer = dynamic_cast<CSimpleHttpServer*>(m_pServer);
	if(pServer)
	{
		pServer->OnHttpRequestError(pszReason, pUserData);
	}
}