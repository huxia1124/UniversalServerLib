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

#include "STXProtocol.h"
#include <string>
#include <atlexcept.h>
#include <atlconv.h>
#include "Cryptographic.h"
#include "STXLog.h"


class CSTXProtocolLua;


//////////////////////////////////////////////////////////////////////////
// CSTXProtocolLua

class CSTXProtocolLua
{
public:
	CSTXProtocolLua(void) {};
	CSTXProtocolLua(std::string s)
	{
		if (s.size() > 0)
		{
			DWORD dwBufferLen = 0;
			BOOL bSuccess = CryptStringToBinaryA(s.c_str(), s.size(), CRYPT_STRING_BASE64, NULL, &dwBufferLen, NULL, NULL);
			if (dwBufferLen > 0)
			{
				BYTE *pBuffer = new BYTE[dwBufferLen];
				if (pBuffer == NULL)
				{
					return;
				}
				bSuccess = CryptStringToBinaryA(s.c_str(), s.size(), CRYPT_STRING_BASE64, pBuffer, &dwBufferLen, NULL, NULL);
				if (bSuccess)
				{
					size_t nLenParsed = 0;
					int nError = 0;
					if (nError = _protocol.Decode((void*)pBuffer, &nLenParsed, (size_t)dwBufferLen))
					{
						static LPCTSTR pErrors[5] = {
							_T("Success!"),
							_T("Failed to parse package length"),
							_T("Not enough actual data"),
							_T("CRC check failed"),
							_T("Unknown Error"),
						};

						STXTRACELOGE(_T("[r][i] CSTXProtocolLua parse STXProtocol object from BASE64 error! %s"), pErrors[nError]);
					}
				}
				delete[]pBuffer;
			}
		}
	}; 
	virtual ~CSTXProtocolLua(void) {};

public:
	CSTXProtocol _protocol;

public:

	std::string GetNextString()
	{
		CSTXProtocolString s;
		if (_protocol.GetNextString(&s))
		{
			return (LPCSTR)s;
		}
		return "";
	};

	__int64 GetNextInteger()
	{
		switch (_protocol.GetNextFieldType())
		{
		case STXPROTOCOL_DATA_TYPE_BYTE:
			return _protocol.GetNextByte();
		case STXPROTOCOL_DATA_TYPE_WORD:
			return _protocol.GetNextWORD();
		case STXPROTOCOL_DATA_TYPE_DWORD:
			return _protocol.GetNextDWORD();
		case STXPROTOCOL_DATA_TYPE_I64:
			return _protocol.GetNextI64();
		}

		return 0;
	};

	double GetNextFloat()
	{
		switch (_protocol.GetNextFieldType())
		{
		case STXPROTOCOL_DATA_TYPE_FLOAT:
			return _protocol.GetNextFloat();
		case STXPROTOCOL_DATA_TYPE_DOUBLE:
			return _protocol.GetNextDouble();
		}

		return 0;
	};

	std::string GetNextGUID()
	{
		USES_CONVERSION;
		GUID guid = _protocol.GetNextGUID();
		OLECHAR szGUID[64] = { 0 };
		::StringFromGUID2(guid, szGUID, 64);
		return (LPCSTR)ATL::CW2A(szGUID);
	}

	std::string GetNextFieldTypeString()
	{
		return CSTXProtocol::GetTypeString(_protocol.GetNextFieldType());
	};

	int GetNextFieldType()
	{
		return _protocol.GetNextFieldType();
	};

	LONG GetDataSize()
	{
		return _protocol.GetDataLen();
	};

	void SeekReadToBegin()
	{
		_protocol.SeekReadToBegin();
	}

	//////////////////////////////////////

	void AppendByte(byte val)
	{
		_protocol.AppendData(val);
	}
	void AppendWord(WORD val)
	{
		_protocol.AppendData(val);
	}
	LONG AppendDWord(DWORD val)
	{
		size_t nOffset = 0;
		_protocol.AppendData(val, &nOffset);
		return nOffset;
	}
	void AppendI64(__int64 val)
	{
		_protocol.AppendData(val);
	}
	void AppendString(std::string val)
	{
		_protocol.AppendData(val.c_str());
	}



	//////////////////////////////////////
	void SkipNextField()
	{
		_protocol.SkipNextField();
	}

	void IncreaseDWORDAtOffset(LONG nOffset)
	{
		_protocol.IncreaseDWORDAtOffset(nOffset, 1);
	}
/*
	static LONG DecodeCompactInteger(void *pData, BYTE *pLengthBytes);		//pLengthBytes : out, size in bytes of the length prefix 
	void IncreaseDWORDAtOffset(LONG nOffset);

	LONG AppendData(BYTE val);
	LONG AppendData(WORD val);
	LONG AppendData(DWORD val, LONG *pOffset = NULL);
	LONG AppendData(__int64 val);
	LONG AppendData(LPCTSTR lpszVal);		//Always convert and append as UTF8 string
	LONG AppendData(FLOAT val);
	LONG AppendData(DOUBLE val);
#ifdef UNICODE
	LONG AppendData(LPCSTR lpszVal);		//Accept ANSI string, convert and append as UTF8 string
#endif
	LONG AppendData(GUID &val);
	LONG AppendData(CSTXProtocol *pVal);	//Object
	LONG AppendRawData(void *pData, LONG cbDataLen);
	LONG AppendUnicodeString(LPCWSTR lpszVal);								//Append as Unicode string
	LONG AppendUnicodeStringPair(LPCWSTR lpszVal1, LPCWSTR lpszVal2);		//Append as Unicode string
	LONG AppendDataPair(LPCTSTR lpszVal1, LPCTSTR lpszVal2);				//Always convert and append as UTF8 pair
	LONG AppendDataPair(LPCTSTR lpszVal, DWORD dwVal);						//UTF8-DWORD pair

	// the length of the prefix and all content
	LONG GetDataLen();

	// The address of the length-prefix
	void* GetBasePtr();

	//pData : address of the pure data (address of length-prefix)
	//pDataReadLen [out, opt] : data length parsed
	BOOL Decode(void *pData, LONG *pDataReadLen);

	BYTE GetNextByte();
	WORD GetNextWORD();
	DWORD GetNextDWORD();
	__int64 GetNextI64();
	FLOAT GetNextFloat();
	DOUBLE GetNextDouble();
	GUID GetNextGUID();
	CSTXProtocol* GetNextObject();
	int GetNextString(LPTSTR lpBuffer, int cchBufferLen);
	BOOL GetNextString(CSTXProtocolString *pString);
	int GetNextUnicodeString(LPTSTR lpBuffer, int cchBufferLen);
	int GetNextRawData(void *pBuffer, int cbBufferSize);
	DWORD GetNextStringPair(LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2);
	DWORD GetNextUnicodeStringPair(LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2);
	DWORD GetNextStringToDWORDPair(LPTSTR lpBuffer, int cchBufferLen);

	void SkipNextField();
	BOOL IsDataAvailable();
	int GetNextFieldLength();
	BYTE GetNextFieldType();
	static LPCTSTR GetTypeString(BYTE nType);
	void SeekReadToBegin();

	*/
};
