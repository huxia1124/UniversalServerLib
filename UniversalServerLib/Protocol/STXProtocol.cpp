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

#include "STXProtocol.h"
#include <string.h>
#include <string>
#include <codecvt>
#include <locale>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define STXPROTOCOL_INITIAL_BUFFER_SIZE				256
#define STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE		16
#define STXPROTOCOL_INITIAL_STRING_SIZE				256


//////////////////////////////////////////////////////////////////////////
// CSTXProtocolUtility

int CSTXProtocolUtility::ConvertString(const char* pszUTF8, int cchUTF8, char *pszBuffer, int cchBuffer)
{
	//UTF8->native(UTF8)
	if (pszBuffer == nullptr)
		return cchUTF8 + 1;

	if (cchBuffer < cchUTF8 + 1)
	{
		return -1;
	}
	if (pszBuffer == nullptr)
	{
		return cchUTF8;
	}
	memcpy(pszBuffer, pszUTF8, cchUTF8);
	pszBuffer[cchUTF8] = 0;
	return cchUTF8;
}

int CSTXProtocolUtility::ConvertString(const char* pszUTF8, int cchUTF8, char16_t *pszBuffer, int cchBuffer)
{
	//UTF8->wchar

	std::u16string wstr = CSTXProtocolUtility::UTF8ToUTF16(pszUTF8, pszUTF8 + cchUTF8);

	if (pszBuffer == nullptr)
		return wstr.size() + 1;

	if (cchBuffer < wstr.size() + 1)
	{
		return -1;
	}

	if (pszBuffer == nullptr)
	{
		return wstr.size();
	}

	memcpy(pszBuffer, wstr.c_str(), wstr.size() * sizeof(char16_t));
	pszBuffer[wstr.size()] = 0;
	return wstr.size();
}

int CSTXProtocolUtility::ConvertString( const char16_t* pszUnicode, int cchUnicode, char* pszBuffer, int cchBuffer )
{
	//wchar->UTF8
	std::string str = CSTXProtocolUtility::UTF16ToUTF8(pszUnicode, pszUnicode + cchUnicode);

	if (pszBuffer == nullptr)
		return str.size() + 1;

	if (cchBuffer < str.size() + 1)
	{
		return -1;
	}

	if (pszBuffer == nullptr)
	{
		return str.size();
	}

	memcpy(pszBuffer, str.c_str(), str.size());
	pszBuffer[str.size()] = 0;
	return str.size();
}

int CSTXProtocolUtility::ConvertString(const char16_t* pszUTF8, int cchUTF8, char16_t* pszBuffer, int cchBuffer)
{
	//wchar->wchar

	if (pszBuffer == nullptr)
		return cchUTF8 + 1;

	if (cchBuffer < cchUTF8 + 1)
	{
		return -1;
	}
	if (pszBuffer == nullptr)
	{
		return cchUTF8;
	}
	memcpy(pszBuffer, pszUTF8, cchUTF8 * sizeof(char16_t));
	pszBuffer[cchUTF8] = 0;
	return cchUTF8;
}

int CSTXProtocolUtility::ConvertStringToUTF8(const char* pszSrc, char *pszBuffer, int cchBuffer )
{
	size_t len = strlen(pszSrc);
	if (pszBuffer == nullptr)
		return len + 1;

	if (cchBuffer < len + 1)
	{
		return -1;
	}

	memcpy(pszBuffer, pszSrc, len);
	pszBuffer[len] = 0;
	return len;
}

int CSTXProtocolUtility::ConvertStringToUTF8(const char16_t* pszSrc, char *pszBuffer, int cchBuffer)
{
	//wchar->UTF8
	std::string str = CSTXProtocolUtility::UTF16ToUTF8(pszSrc);

	if (pszBuffer == nullptr)
		return str.size() + 1;

	if (cchBuffer < str.size() + 1)
	{
		return -1;
	}

	memcpy(pszBuffer, str.c_str(), str.size());
	pszBuffer[str.size()] = 0;
	return str.size();
}

int CSTXProtocolUtility::ConvertStringToUnicode(const char* pszSrc, char16_t *pszBuffer, int cchBuffer)
{
	size_t len = strlen(pszSrc);
	if (pszBuffer == nullptr)
		return len + 1;

	if (cchBuffer < len + 1)
	{
		return -1;
	}

	memcpy(pszBuffer, pszSrc, len);
	pszBuffer[len] = 0;
	return len;
}

int CSTXProtocolUtility::ConvertStringToUnicode(const char16_t* pszSrc, char16_t *pszBuffer, int cchBuffer)
{
	size_t len = std::char_traits<char16_t>::length(pszSrc);
	if (pszBuffer == nullptr)
		return len + 1;
	if (cchBuffer < len + 1)
	{
		return -1;
	}

	memcpy(pszBuffer, pszSrc, len * sizeof(char16_t));
	pszBuffer[len] = 0;
	return len;
}

std::string CSTXProtocolUtility::ConvertGUIDToUTF8(GUID *guid)
{
	char parts[5][16];
	std::stringstream stream;
	ConvertToHexString((unsigned char*)&guid->Data1, sizeof(guid->Data1), parts[0], true);
	ConvertToHexString((unsigned char*)&guid->Data2, sizeof(guid->Data2), parts[1], true);
	ConvertToHexString((unsigned char*)&guid->Data3, sizeof(guid->Data3), parts[2], true);
	ConvertToHexString((unsigned char*)guid->Data4, 2, parts[3], true);
	ConvertToHexString((unsigned char*)guid->Data4 + 2, 6, parts[4], true);

	stream << parts[0] << "-"
		<< parts[1] << "-"
		<< parts[2] << "-"
		<< parts[3] << "-"
		<< parts[4];

	return stream.str();
}

long CSTXProtocolUtility::ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char *lpszHexBuf, bool bUpperCased)
{
	static const char szHexDictUpper[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static const char szHexDictLower[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	uint32_t i = 0;
	if (bUpperCased)
	{
		for (i = 0; i < cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictUpper[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictUpper[lpData[i] & 0x0F];
		}
	}
	else
	{
		for (i = 0; i < cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictLower[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictLower[lpData[i] & 0x0F];
		}
	}
	lpszHexBuf[i * 2] = 0;
	return cbDataLen * 2;

}

std::string CSTXProtocolUtility::UTF16ToUTF8(std::u16string utf16_string)
{
#if _MSC_VER == 1900
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = reinterpret_cast<const int16_t *>(utf16_string.data());
	return convert.to_bytes(p, p + utf16_string.size());
#else

	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(utf16_string);
#endif
}

std::string CSTXProtocolUtility::UTF16ToUTF8(const char16_t *begin, const char16_t *end)
{
#if _MSC_VER == 1900
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto pb = reinterpret_cast<const int16_t *>(begin);
	auto pe = reinterpret_cast<const int16_t *>(end);
	return convert.to_bytes(pb, pe);
#else

	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(begin, end);
#endif
}

std::u16string CSTXProtocolUtility::UTF8ToUTF16(std::string utf8_string)
{
#if _MSC_VER == 1900
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = convert.from_bytes(utf8_string.data());
	std::u16string s;
	s.resize(p.size());
	memcpy((void*)s.c_str(), p.data(), p.size() * sizeof(int16_t));
	return s;
#else

	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.from_bytes(utf8_string);
#endif

}

std::u16string CSTXProtocolUtility::UTF8ToUTF16(const char *begin, const char *end)
{
#if _MSC_VER == 1900
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = convert.from_bytes(begin, end);
	std::u16string s;
	s.resize(p.size());
	memcpy((void*)s.c_str(), p.data(), p.size() * sizeof(int16_t));
	return s;
#else

	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.from_bytes(begin, end);
#endif
}

//////////////////////////////////////////////////////////////////////////
// CSTXProtocolString

CSTXProtocolString::CSTXProtocolString()
{
	m_cchBufferSize = 0;
	m_pBuffer = nullptr;
	ExpandTo(STXPROTOCOL_INITIAL_STRING_SIZE);
}

CSTXProtocolString::CSTXProtocolString( STXPROTOCOLVALUE *pVal )
{
	m_cchBufferSize = 0;
	m_pBuffer = nullptr;

	if(pVal)
	{
		if((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_UTF8)
		{
			int nExpand = CSTXProtocolUtility::ConvertString(pVal->pszUTF8String, pVal->cchUTF8StringLen, (char*)nullptr, 0);
			ExpandTo(nExpand);
			CSTXProtocolUtility::ConvertString(pVal->pszUTF8String, pVal->cchUTF8StringLen, m_pBuffer, m_cchBufferSize);
			return;
		}
		else if((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_UNICODE)
		{
			int nExpand = CSTXProtocolUtility::ConvertString(pVal->pszUnicodeString, pVal->cchUnicodeStringLen, (char16_t*)nullptr, 0);
			ExpandTo(nExpand);
			CSTXProtocolUtility::ConvertString(pVal->pszUnicodeString, pVal->cchUnicodeStringLen, m_pBuffer, m_cchBufferSize);
			return;
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_BYTE)
		{
			std::string s = std::to_string(pVal->byteVal);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_WORD)
		{
			std::string s = std::to_string(pVal->wVal);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_DWORD)
		{
			std::string s = std::to_string(pVal->dwVal);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_I64)
		{
			std::string s = std::to_string(pVal->nVal64);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_FLOAT)
		{
			std::string s = std::to_string(pVal->floatVal);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_DOUBLE)
		{
			std::string s = std::to_string(pVal->doubleVal);
			ExpandTo(s.size());
			CSTXProtocolUtility::ConvertString(s.c_str(), s.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_GUID)
		{
			std::string guidStr = CSTXProtocolUtility::ConvertGUIDToUTF8(&pVal->guidVal);
			ExpandTo(guidStr.size());
			CSTXProtocolUtility::ConvertString(guidStr.c_str(), guidStr.size(), m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_RAW)
		{
			char *pszHex = new char[pVal->cbDataLen * 2];
			ConvertToHexString((unsigned char*)pVal->pDataPtr, pVal->cbDataLen * 2, pszHex, true);
			ExpandTo(pVal->cbDataLen * 2 * sizeof(char));
			CSTXProtocolUtility::ConvertString(pszHex, pVal->cbDataLen * 2, m_pBuffer, m_cchBufferSize);		
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_OBJECT)
		{

		}
		else		// if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_INVALID)
		{
			char szValue[64] = "<Invalid Value>";
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
	}

	ExpandTo(STXPROTOCOL_INITIAL_STRING_SIZE);
}

CSTXProtocolString::~CSTXProtocolString()
{
	if(m_pBuffer)
	{
		delete []m_pBuffer;
		m_pBuffer = nullptr;
	}
}

long CSTXProtocolString::ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char *lpszHexBuf, bool bUpperCased)
{
	static const char szHexDictUpper[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static const char szHexDictLower[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	uint32_t i = 0;
	if (bUpperCased)
	{
		for (i = 0; i<cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictUpper[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictUpper[lpData[i] & 0x0F];
		}
	}
	else
	{
		for (i = 0; i<cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictLower[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictLower[lpData[i] & 0x0F];
		}
	}
	lpszHexBuf[i * 2] = 0;
	return cbDataLen * 2;
}

long CSTXProtocolString::ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char16_t *lpszHexBuf, bool bUpperCased)
{
	static const char16_t szHexDictUpper[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static const char16_t szHexDictLower[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	uint32_t i = 0;
	if (bUpperCased)
	{
		for (i = 0; i < cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictUpper[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictUpper[lpData[i] & 0x0F];
		}
	}
	else
	{
		for (i = 0; i < cbDataLen; i++)
		{
			lpszHexBuf[i * 2] = szHexDictLower[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictLower[lpData[i] & 0x0F];
		}
	}
	lpszHexBuf[i * 2] = 0;
	return cbDataLen * 2;
}

void CSTXProtocolString::ExpandTo( int cchMax )
{
	while(cchMax > m_cchBufferSize)
	{
#ifdef max
		long cchNewBufferLen = max(m_cchBufferSize * 2, STXPROTOCOL_INITIAL_STRING_SIZE);
#else
		long cchNewBufferLen = std::max(m_cchBufferSize * 2, STXPROTOCOL_INITIAL_STRING_SIZE);
#endif
		char16_t *pNewBuffer = new char16_t[cchNewBufferLen];
		pNewBuffer[0] = 0;
		if(m_pBuffer)
		{
			std::char_traits<char16_t>::copy(pNewBuffer, m_pBuffer, std::char_traits<char16_t>::length(m_pBuffer));
			delete []m_pBuffer;
		}
		m_pBuffer = pNewBuffer;
		m_cchBufferSize = cchNewBufferLen;
	}
}

char16_t *CSTXProtocolString::GetBuffer( int nBufferLength /*= 0*/ )
{
	if(nBufferLength > 0)
		ExpandTo(nBufferLength);

	return m_pBuffer;
}

CSTXProtocolString::operator const char16_t*()
{
	return m_pBuffer;
}


CSTXProtocolString::operator const char*()
{
	if (m_convertedUTF8.size() == 0)
	{
		m_convertedUTF8 = CSTXProtocolUtility::UTF16ToUTF8(m_pBuffer);
	}
	return m_convertedUTF8.c_str();
}


int CSTXProtocolString::GetLength()
{
	return std::char_traits<char16_t>::length(m_pBuffer);
}


//////////////////////////////////////////////////////////////////////////
// CSTXProtocol

CSTXProtocol::CSTXProtocol(void)
{
	m_nBufferLen = STXPROTOCOL_INITIAL_BUFFER_SIZE;	// 初始缓冲区大小
	m_nCurentReadOffset = 0;
	m_nCurentWriteOffset = 0;
	m_nPrefixLen = 1;
	m_pBuffer = new char[m_nBufferLen + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE];
	m_pBuffer[STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE - 1] = 0;		//Set CRC = 0
	UpdatePrefix();
}

CSTXProtocol::~CSTXProtocol(void)
{
	if(m_pBuffer)
		delete []m_pBuffer;
	m_pBuffer = nullptr;
}

long CSTXProtocol::Expand( long nAdd )
{
	// 根据需要，动态增加缓冲区空间
	while(m_nCurentWriteOffset + nAdd > m_nBufferLen)
	{
		long nNewBufferLen = m_nBufferLen * 2;
		char *pNewBuffer = new char[nNewBufferLen + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE];
		memcpy(pNewBuffer, m_pBuffer, m_nCurentWriteOffset + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE);
		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
		m_nBufferLen = nNewBufferLen;
	}
	return nAdd;
}

long CSTXProtocol::GetDataLen()
{
	return m_nCurentWriteOffset + m_nPrefixLen + 1;		// +1 for CRC
}

void* CSTXProtocol::GetBasePtr()
{
	return (void*)(m_pBuffer + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE - m_nPrefixLen - 1);	// -1 for CRC
}

void* CSTXProtocol::GetCRCBytePtr()
{
	return (void*)(m_pBuffer + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE - 1);
}

long CSTXProtocol::GetEncryptedData(void *pBuffer, long cbBufferLen, uint32_t dwKey)
{
	long cbDataLen = GetDataLen() - m_nPrefixLen - 1;
	if (cbDataLen + m_nPrefixLen + 1> cbBufferLen)
	{
		return -1;
	}

	int r = 0;
	unsigned char *pszData = (unsigned char*)GetCRCBytePtr();
	char k = (char)((~(dwKey % 0x100)) & 0xFF);
	pszData++;	//Skip CRC
	for (int i = 0; i < cbDataLen; i++)
	{
		char b = *((char*)(pszData + i));
		r += b;

		char m = EncryptByte(b, k);

		((unsigned char*)pBuffer)[m_nPrefixLen + 1 + i] = m;
	}

	unsigned char crc = (unsigned char)((r % 256) & 0xFF);
	((unsigned char*)pBuffer)[m_nPrefixLen] = EncryptByte(crc, k);

	memcpy(pBuffer, GetBasePtr(), m_nPrefixLen);
	return crc;
}

long CSTXProtocol::GetTypedDataLen(unsigned char nType)
{
	if(nType & STXPROTOCOL_DATA_TYPE_FLAG_PAIR)
		return GetTypedDataLen(nType & 0x0F);

	switch(nType)
	{
	case STXPROTOCOL_DATA_TYPE_BYTE:
		return 1;
	case STXPROTOCOL_DATA_TYPE_WORD:
		return 2;
	case STXPROTOCOL_DATA_TYPE_DWORD:
		return 4;
	case STXPROTOCOL_DATA_TYPE_I64:
		return 8;
	case STXPROTOCOL_DATA_TYPE_FLOAT:
		return 4;
	case STXPROTOCOL_DATA_TYPE_DOUBLE:
		return 8;
	case STXPROTOCOL_DATA_TYPE_GUID:
		return 16;
	}
	return 0;
}

long CSTXProtocol::WriteDataType( unsigned char nType )
{
	Expand(sizeof(nType));
	// 写入字段类型
	*((unsigned char*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = nType;
	m_nCurentWriteOffset++;
	UpdatePrefix();
	UpdateCRC(&nType, 1);
	return 1;
}

long CSTXProtocol::AppendData( unsigned char val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_BYTE);
	Expand(sizeof(val));
	*((unsigned char*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC(&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData( uint16_t val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_WORD);
	Expand(sizeof(val));
	*((uint16_t*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((unsigned char*)&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData( uint32_t val ,long *pOffset)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_DWORD);
	Expand(sizeof(val));
	*((uint32_t*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	if(pOffset)
		*pOffset = m_nCurentWriteOffset;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((unsigned char*)&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData(float val)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_FLOAT);
	Expand(sizeof(val));
	*((float*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((unsigned char*)&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData(double val)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_DOUBLE);
	Expand(sizeof(val));
	*((double*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((unsigned char*)&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData( int64_t val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_I64);
	Expand(sizeof(val));
	*((int64_t*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((unsigned char*)&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData(char16_t *lpszVal)
{
	return AppendData((const char16_t*)lpszVal);
}

long CSTXProtocol::AppendData(const char16_t *lpszVal)
{
	std::string str = CSTXProtocolUtility::UTF16ToUTF8(lpszVal);

	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(str.size());
	Expand(str.size());
	WriteRawData((void*)str.c_str(), str.size());	//Skip nullptr-termination
	return str.size();
}

long CSTXProtocol::AppendData(char *lpszVal)
{
	return AppendData((const char*)lpszVal);
}

long CSTXProtocol::AppendData(const char *lpszVal)
{
	size_t len = strlen(lpszVal);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len);
	Expand(len);
	WriteRawData((void*)lpszVal, len);	//Skip nullptr-termination
	return len;
}

long CSTXProtocol::AppendUnicodeString( const char16_t* lpszVal )
{
	if(lpszVal == nullptr)
		return -1;

	int nStrLen = std::char_traits<char16_t>::length(lpszVal);
	long cbLen = nStrLen * sizeof(char16_t);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)lpszVal, cbLen);	//Skip nullptr-termination
	return cbLen;
}

long CSTXProtocol::AppendUnicodeString(const char* lpszVal)
{
	if (lpszVal == nullptr)
		return -1;

	std::u16string wstr = CSTXProtocolUtility::UTF8ToUTF16(lpszVal);

	size_t nStrLen = wstr.size();
	size_t cbLen = nStrLen * sizeof(char16_t);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)wstr.c_str(), cbLen);	//Skip nullptr-termination
	return cbLen;
}

long CSTXProtocol::AppendUnicodeStringPair( const char* lpszVal1, const char* lpszVal2 )
{
	if(lpszVal1 == nullptr || lpszVal2 == nullptr)
		return -1;

	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE_PAIR);

	std::u16string wstr1 = CSTXProtocolUtility::UTF8ToUTF16(lpszVal1);

	size_t nStrLen = wstr1.size();
	size_t cbLen = nStrLen * sizeof(char16_t);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)wstr1.c_str(), cbLen);	//Skip nullptr-termination

	std::u16string wstr2 = CSTXProtocolUtility::UTF8ToUTF16(lpszVal2);
	size_t nStrLen2 = wstr2.size();
	size_t cbLen2 = nStrLen2 * sizeof(char16_t);
	WriteCompactInteger(cbLen2);
	Expand(cbLen2);
	WriteRawData((void*)wstr2.c_str(), cbLen2);	//Skip nullptr-termination

	return cbLen + cbLen2;
}

long CSTXProtocol::AppendUnicodeStringPair(const char16_t* lpszVal1, const char16_t* lpszVal2)
{
	if (lpszVal1 == nullptr || lpszVal2 == nullptr)
		return -1;

	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE_PAIR);

	int nStrLen = std::char_traits<char16_t>::length(lpszVal1);
	long cbLen = nStrLen * sizeof(char16_t);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)lpszVal1, cbLen);	//Skip nullptr-termination

	int nStrLen2 = std::char_traits<char16_t>::length(lpszVal2);
	long cbLen2 = nStrLen2 * sizeof(char16_t);
	WriteCompactInteger(cbLen2);
	Expand(cbLen2);
	WriteRawData((void*)lpszVal2, cbLen2);	//Skip nullptr-termination

	return cbLen + cbLen2;
}

long CSTXProtocol::AppendData( GUID &val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_GUID);
	Expand(sizeof(val));
	WriteRawData(&val, sizeof(val));
	return sizeof(val);
}

long CSTXProtocol::AppendData( CSTXProtocol *pVal )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_OBJECT);
	long nDataLen = pVal->GetDataLen();
	WriteCompactInteger(nDataLen);
	WriteRawData(pVal->GetBasePtr(), nDataLen);
	return nDataLen;
}

long CSTXProtocol::AppendRawData(void *pData, long cbDataLen)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_RAW);
	WriteCompactInteger(cbDataLen);
	WriteRawData(pData, cbDataLen);
	return cbDataLen;
}

long CSTXProtocol::GetCompactIntegerLen( unsigned int nValue )
{
	if(nValue == 0)
		return 1;

	long nByteLen = 0;
	while(nValue > 0)
	{
		nValue >>= 7;
		nByteLen++;
	}
	return nByteLen;
}

long CSTXProtocol::WriteCompactInteger( unsigned int nValue )
{
	Expand(GetCompactIntegerLen(nValue));
	if(nValue == 0)
	{
		*((unsigned char*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = 0;
		m_nCurentWriteOffset++;
		UpdatePrefix();
		UpdateCRC((unsigned char*)&nValue, 1);
		return 1;
	}

	long nBytesWritten = 0;
	while(nValue > 0)
	{
		unsigned char nByteToWrite = (unsigned char)(nValue & 0x0000007F);
		nValue >>= 7;
		if(nValue > 0)
			nByteToWrite |= 0x80;

		*((unsigned char*)(GetDataContentBasePtr() + m_nCurentWriteOffset + nBytesWritten)) = nByteToWrite;
		nBytesWritten++;
		UpdateCRC((unsigned char*)&nByteToWrite, 1);
	}
	m_nCurentWriteOffset += nBytesWritten;
	UpdatePrefix();
	return nBytesWritten;
}

long CSTXProtocol::WriteRawData( void *pData, long cbDataLen )
{
	if (cbDataLen == 0)
		return 0;

	Expand(cbDataLen);
	memcpy(GetDataContentBasePtr() + m_nCurentWriteOffset, pData, cbDataLen);
	m_nCurentWriteOffset += cbDataLen;
	UpdatePrefix();
	UpdateCRC((unsigned char*)pData, cbDataLen);
	return cbDataLen;
}

char* CSTXProtocol::GetDataContentBasePtr()
{
	return m_pBuffer + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE;
}

void CSTXProtocol::UpdateCRC(unsigned char *pAddedData, unsigned int cbDataLen)
{
	unsigned char &currentCRC = *(unsigned char*)GetCRCBytePtr();
	for (unsigned int i = 0; i < cbDataLen; i++)
	{
		currentCRC += pAddedData[i];
	}
}

void CSTXProtocol::UpdatePrefix()
{
	m_nPrefixLen = (unsigned char)GetCompactIntegerLen(m_nCurentWriteOffset);
	unsigned int nValue = m_nCurentWriteOffset + 1;
	char* pBasePtr = (char*)GetBasePtr();
	if(nValue == 1)
	{
		*((unsigned char*)pBasePtr) = 1;
		return;
	}
	while(nValue > 1)
	{
		unsigned char nByteToWrite = (unsigned char)(nValue & 0x0000007F);
		nValue >>= 7;
		if(nValue > 0)
			nByteToWrite |= 0x80;

		*((unsigned char*)pBasePtr) = nByteToWrite;
		pBasePtr++;
	}
}

bool CSTXProtocol::DecodeEmbeddedObject( void *pData, long *pDataReadLen )
{
	unsigned char nLengthBytes = 0;
	long nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
	if(nObjectContentLength == -1)
		return false;

	unsigned char nContentLengthBytes = 0;
	long nContentLength = DecodeCompactInteger(((char*)pData) + nLengthBytes , &nContentLengthBytes);
	if(nContentLength == -1)
		return false;

	ResetPosition();
	Expand(nContentLength);
	WriteRawData(((char*)pData) + nLengthBytes + nContentLengthBytes + 1, nContentLength - 1);	//+1/-1 for CRC
	if(pDataReadLen)
		*pDataReadLen = nLengthBytes + nObjectContentLength;

	return true;
}

inline char CSTXProtocol::EncryptByte(char data, char key)
{
	int d = data;
	d += key;
	d %= 256;
	char dc = (d & 0xFF);
	char m = ~dc;
	m = m ^ (((key << 4) | (key >> 4)) & 0xFF);

	return m;
}

inline char CSTXProtocol::DecryptByte(char data, char key)
{
	char m = data;
	m = m ^ (((key << 4) | (key >> 4)) & 0xFF);
	char dc = ~m;

	int d = (dc + 256 - key) % 256;
	char dr = (char)(d & 0xFF);
	return dr;
}

int CSTXProtocol::Decode(void *pData, long *pDataReadLen)
{
	return Decode(pData, pDataReadLen, 0);
}

int CSTXProtocol::Decode( void *pData, long *pDataReadLen, long cbInputDataLen)
{
	unsigned char nLengthBytes = 0;
	long nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
	if(nObjectContentLength == -1 || nObjectContentLength < 1)
		return 1;	//Error parsing package length 

	if (cbInputDataLen > 0 && nObjectContentLength + nLengthBytes + 1 > cbInputDataLen)
		return 2;	//Error not enough actual data

	unsigned char &crc = ((unsigned char*)pData)[nLengthBytes];
	unsigned char crcFrom = 0;
	for (long i = 1; i < nObjectContentLength; i++)
	{
		crcFrom += ((unsigned char*)pData)[nLengthBytes + i];
	}
	if (crcFrom != crc)
	{
		//CRC check error!
		return 3;	//Error crc check failed
	}

	ResetPosition();
	Expand(nObjectContentLength);
	WriteRawData(((char*)pData) + nLengthBytes + 1, nObjectContentLength - 1);		//+1 and -1 for CRC
	if(pDataReadLen)
		*pDataReadLen = nLengthBytes;

	return 0;
}

int CSTXProtocol::DecodeWithDecrypt(void * pData, long * pDataReadLen, uint32_t dwKey)
{
	unsigned char nLengthBytes = 0;
	long nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
	if (nObjectContentLength == -1 || nObjectContentLength < 1)
		return 1;	//Error parsing package length 

	//if (cbInputDataLen > 0 && nObjectContentLength + nLengthBytes + 1 > cbInputDataLen)
	//	return 2;	//Error not enough actual data

	char k = (char)((~(dwKey % 0x100)) & 0xFF);

	int r = 0;
	char *pszData = (char*)pData;
	pszData += nLengthBytes;
	for (int i = 1; i < nObjectContentLength; i++)
	{
		char m = pszData[i];
		char dr = DecryptByte(m, k);

		pszData[i] = dr;

		r += *((char*)(pszData + i));
	}
	char crc = (char)((r % 256) & 0xFF);
	char desired_crc = DecryptByte(pszData[0], k);

	if (crc != desired_crc)
	{
		throw std::runtime_error("DecodeWithDecrypt(void*,long*,uint32_t) : Error parsing package, CRC mismatch.");
		return 8;	//Error parsing package, CRC mismatch 
	}
	pszData[0] = crc;

	return Decode(pData, pDataReadLen);
}

long CSTXProtocol::DecodeCompactInteger(void *pData, unsigned char *pLengthBytes)
{
	if(pData == nullptr)
		return -1;

	unsigned char *pPtr = (unsigned char*)pData;
	int nDecodedBytes = 0;

	unsigned int nValue = 0;
	unsigned int nValueUnit = 0;
	while((*pPtr) & 0x80)
	{
		nValueUnit = (*pPtr & 0x7F);
		nValueUnit <<= (nDecodedBytes * 7);
		nValue += nValueUnit;
		pPtr++;
		nDecodedBytes++;

		if(nDecodedBytes >= STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE)
			return -1;
	}
	nValueUnit = (*pPtr & 0x7F);
	nValueUnit <<= (nDecodedBytes * 7);
	nValue += nValueUnit;
	nDecodedBytes++;
	if(nDecodedBytes >= STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE)
		return -1;

	if(pLengthBytes)
		*pLengthBytes = nDecodedBytes;

	return nValue;
}

void CSTXProtocol::ResetPosition()
{
	m_nCurentWriteOffset = 0;
	m_nCurentReadOffset = 0;
	unsigned char &currentCRC = *(unsigned char*)GetCRCBytePtr();
	currentCRC = 0;
	UpdatePrefix();
}

bool CSTXProtocol::IsValidDataType( unsigned char nType )
{
	if(m_nCurentReadOffset >= m_nCurentWriteOffset)
		return false;

	unsigned char nTypeInData =  *((unsigned char*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	return nTypeInData == nType;
}

//inline void CSTXProtocol::AssertBreak(LPCTSTR lpszError)
//{
//#ifdef _DEBUG
//	OutputDebugString(lpszError);
//	OutputDebugString(_T("\n"));
//	DebugBreak();
//#endif
//}

bool CSTXProtocol::SkipTypeIndicator(unsigned char *pTypeSkipped)
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		if(pTypeSkipped)
			*pTypeSkipped =  *((unsigned char*)(GetDataContentBasePtr() + m_nCurentReadOffset));

		m_nCurentReadOffset++;
		return true;
	}
	return false;
}

void CSTXProtocol::CheckDataAvailability( unsigned char nType )
{
	if (m_nCurentWriteOffset - m_nCurentReadOffset < GetTypedDataLen(nType))
	{
		//AssertBreak(_T("CheckDataAvailability() : Not enough data available!"));
	}
}

unsigned char CSTXProtocol::GetNextByte()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_BYTE;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextByte() : Next field is not Byte type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	unsigned char nValue =  *((unsigned char*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

uint16_t CSTXProtocol::GetNextWORD()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_WORD;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextWORD() : Next field is not Word type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	uint16_t nValue =  *((uint16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

uint32_t CSTXProtocol::GetNextDWORD()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_DWORD;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextDWORD() : Next field is not DWord type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	uint32_t nValue =  *((uint32_t*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

int64_t CSTXProtocol::GetNextI64()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_I64;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextI64() : Next field is not I64 type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	int64_t nValue =  *((int64_t*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

float CSTXProtocol::GetNextFloat()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_FLOAT;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextFloat() : Next field is not Float type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	float nValue = *((float*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

double CSTXProtocol::GetNextDouble()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_DOUBLE;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextDouble() : Next field is not Double type.");
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	double nValue = *((double*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

GUID CSTXProtocol::GetNextGUID()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_GUID;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextGUID() : Next field is not GUID type.");
		return GUID();
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	GUID nValue =  *((GUID*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

CSTXProtocol* CSTXProtocol::GetNextObject()
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_OBJECT;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextObject() : Next field is not Object type.");
		return nullptr;
	}

	long nDataReadLen = 0;
	CSTXProtocol *pNewObject = new CSTXProtocol();
	if(pNewObject->DecodeEmbeddedObject(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nDataReadLen))
	{
		SkipTypeIndicator();
		m_nCurentReadOffset += nDataReadLen;
		return pNewObject;
	}
	delete pNewObject;
	return nullptr;
}

int CSTXProtocol::GetNextString(char *lpBuffer, int cchBufferLen )
{
	if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return -1;
		}

		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString(char*,int) : Not enough string data available.");
			return -1;
		}

		const char16_t* pStrBase = (char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

		std::string str = CSTXProtocolUtility::UTF16ToUTF8(pStrBase, pStrBase + nStringLen / sizeof(char16_t));

		if (cchBufferLen < str.size() + 1)
		{
			return -1;
		}

		memcpy(lpBuffer, str.c_str(), str.size());
		lpBuffer[str.size()] = 0;

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return str.size();
	}
	else if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return -1;
		}

		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString(char*,int) : Not enough string data available.");
			return -1;
		}

		const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

		if (cchBufferLen < nStringLen + 1)
		{
			return -1;
		}

		memcpy(lpBuffer, pStrBase, nStringLen);
		lpBuffer[nStringLen] = 0;

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return nStringLen;
	}

	throw std::runtime_error("GetNextString(char*,int) : Next field is not any of the String types.");
	return 0;
}


int CSTXProtocol::GetNextString(char16_t *lpBuffer, int cchBufferLen)
{
	if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return -1;
		}

		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString(char16_t*,int) : Not enough string data available.");
			return -1;
		}

		const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

		std::u16string str = CSTXProtocolUtility::UTF8ToUTF16(pStrBase, pStrBase + nStringLen);

		if (cchBufferLen < str.size() + 1)
		{
			return -1;
		}

		memcpy(lpBuffer, str.c_str(), str.size() * sizeof(char16_t));
		lpBuffer[str.size()] = 0;

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return str.size();
	}
	else if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return -1;
		}

		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString(char16_t*,int) : Not enough string data available.");
			return -1;
		}

		const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

		if (cchBufferLen < nStringLen + 1)
		{
			return -1;
		}

		memcpy(lpBuffer, pStrBase, nStringLen);
		lpBuffer[nStringLen] = 0;

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return nStringLen;
	}
	return 0;
}

std::string CSTXProtocol::GetNextString()
{
	if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return "";
		}
		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString() : Not enough string data available.");
			return "";
		}

		const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

		std::string s;
		s.resize(nStringLen);
		memcpy((void*)(char*)s.c_str(), pStrBase, nStringLen);

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return s;
	}
	else if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return "";
		}
		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextString() : Not enough string data available.");
			return "";
		}

		const char16_t* pStrBase = (char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

		std::string str = CSTXProtocolUtility::UTF16ToUTF8(pStrBase, pStrBase + nStringLen / sizeof(char16_t));

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return str;
	}
	throw std::runtime_error("GetNextUnicodeString() : Next field is not UTF8/Unicode type.");
	return "";
}

bool CSTXProtocol::GetNextString( CSTXProtocolString *pString )
{
	if(pString == nullptr)
		return false;

	if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		return GetNextUnicodeString(pString);
	}

	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UTF8;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextString(CSTXProtocolString*) : Next field is not UTF8/Unicode type.");
		return false;
	}

	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nStringLen == -1)
	{
		return false;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		throw std::runtime_error("GetNextString(CSTXProtocolString*) : Not enough string data available.");
		return false;
	}

	const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

	std::u16string str = CSTXProtocolUtility::UTF8ToUTF16(pStrBase, pStrBase + nStringLen);

	char16_t *pBuffer = pString->GetBuffer(str.size() + 1);
	memcpy(pBuffer, str.c_str(), str.size() * sizeof(char16_t));
	pBuffer[str.size()] = 0;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	return true;
}

bool CSTXProtocol::GetNextUnicodeString(CSTXProtocolString *pString)
{
	if (pString == nullptr)
		return false;

	if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		return GetNextString(pString);
	}

	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UNICODE;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextUnicodeString(CSTXProtocolString*) : Next field is not UTF8/Unicode type.");
		return false;
	}

	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);		//in bytes
	if (nStringLen == -1)
	{
		return false;
	}

	if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		throw std::runtime_error("GetNextUnicodeString(CSTXProtocolString*) : Not enough string data available.");
		return false;
	}

	const char16_t* pStrBase = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

	char16_t *pBuffer = pString->GetBuffer(nStringLen / sizeof(char16_t) + 1);
	memcpy(pBuffer, pStrBase, nLengthBytes);

	pBuffer[nStringLen / sizeof(char16_t)] = 0;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	return true;
}


std::u16string CSTXProtocol::GetNextUnicodeString()
{
	if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return u"";
		}
		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextUnicodeString() : Not enough string data available.");
			return u"";
		}

		const char* pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

		std::u16string str = CSTXProtocolUtility::UTF8ToUTF16(pStrBase, pStrBase + nStringLen);

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return str;
	}
	else if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		unsigned char nLengthBytes = 0;
		long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
		if (nStringLen == -1)
		{
			return u"";
		}
		if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
		{
			throw std::runtime_error("GetNextUnicodeString() : Not enough string data available.");
			return u"";
		}

		const char16_t* pStrBase = (char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

		std::u16string s;
		s.resize(nStringLen / sizeof(char16_t));
		memcpy((void*)(char*)s.c_str(), pStrBase, nStringLen);

		SkipTypeIndicator();
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nStringLen;

		return s;
	}
	throw std::runtime_error("GetNextUnicodeString() : Next field is not UTF8/Unicode type.");
	return u"";
}

void CSTXProtocol::SkipNextField()
{
	unsigned char nTypeSkipped = 0;
	if(!SkipTypeIndicator(&nTypeSkipped))
		return;

	if(nTypeSkipped & STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
	{
		unsigned char nLengthBytes = 0;
		long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset, &nLengthBytes);
		if(nFieldLen == -1)
		{
			return;
		}
		m_nCurentReadOffset += nLengthBytes;
		m_nCurentReadOffset += nFieldLen;
	}
	else
	{
		m_nCurentReadOffset += GetTypedDataLen(nTypeSkipped);
	}

	if(nTypeSkipped & STXPROTOCOL_DATA_TYPE_FLAG_PAIR)
	{
		if(nTypeSkipped & STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX)
		{
			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset, &nLengthBytes);
			if(nFieldLen == -1)
			{
				return;
			}
			m_nCurentReadOffset += nLengthBytes;
			m_nCurentReadOffset += nFieldLen;
		}
		else
		{
			m_nCurentReadOffset += GetTypedDataLen(nTypeSkipped);
		}
	}
}

bool CSTXProtocol::IsDataAvailable()
{
	return (m_nCurentWriteOffset - m_nCurentReadOffset > 0);
}

int CSTXProtocol::GetNextRawData( void *pBuffer, int cbBufferSize )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_RAW;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextRawData(void*,int) : Next field is not Raw type.");
		return -1;
	}

	unsigned char nLengthBytes = 0;
	long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nFieldLen == -1)
	{
		return -1;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nFieldLen + nLengthBytes + 1)
	{
		throw std::runtime_error("GetNextRawData(void*,int) : Not enough raw data available.");
		return -1;
	}

	if(pBuffer == nullptr)
	{
		return nFieldLen;
	}

	if(nFieldLen > cbBufferSize)
	{
		throw std::runtime_error("GetNextRawData(void*,int) : Buffer size is too small to store raw data.");
		return -1;
	}

	const char* pFieldDataBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;
	memcpy(pBuffer, pFieldDataBase, nFieldLen);

	SkipNextField();

	return nFieldLen;
}

int CSTXProtocol::GetNextFieldLength()
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		unsigned char nType =  *((unsigned char*)(GetDataContentBasePtr() + m_nCurentReadOffset));

		if(nType & STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
		{
			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			if(nFieldLen == -1)
			{
				return -1;
			}
			return nLengthBytes + nFieldLen;
		}
		else
		{
			return GetTypedDataLen(nType);
		}
	}
	//AssertBreak(_T("GetNextFieldLength() : Failed to get the length of next field."));
	return -1;
}

int CSTXProtocol::EnumValues(std::function<void(unsigned char originalType, STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData)> pfnEnum, void *pUserData)
{
	int nValEnumCount = 0;
	STXPROTOCOLVALUE val;
	STXPROTOCOLVALUE valExtra;
	//valExtra.nValueType = STXPROTOCOL_DATA_TYPE_INVALID;
	unsigned char originalType = STXPROTOCOL_DATA_TYPE_INVALID;
	while (IsDataAvailable())
	{
		val.nValueType = GetNextFieldType();
		valExtra.nValueType = STXPROTOCOL_DATA_TYPE_INVALID;
		unsigned char originalType = val.nValueType;
		switch (val.nValueType)
		{
		case STXPROTOCOL_DATA_TYPE_BYTE:
			val.byteVal = GetNextByte();
			break;
		case STXPROTOCOL_DATA_TYPE_WORD:
			val.wVal = GetNextWORD();
			break;
		case STXPROTOCOL_DATA_TYPE_DWORD:
			val.dwVal = GetNextDWORD();
			break;
		case STXPROTOCOL_DATA_TYPE_I64:
			val.nVal64 = GetNextI64();
			break;
		case STXPROTOCOL_DATA_TYPE_FLOAT:
			val.floatVal = GetNextFloat();
			break;
		case STXPROTOCOL_DATA_TYPE_DOUBLE:
			val.doubleVal = GetNextDouble();
			break;
		case STXPROTOCOL_DATA_TYPE_GUID:
			val.guidVal = GetNextGUID();
			break;
		case STXPROTOCOL_DATA_TYPE_UTF8:
		{
			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cchUTF8StringLen = nFieldLen;
			val.pszUTF8String = (const char*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
			SkipNextField();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_OBJECT:
		{
			val.pObject = GetNextObject();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_RAW:
		{
			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cbDataLen = nFieldLen;
			val.pDataPtr = (void*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
			SkipNextField();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_UNICODE:
		{
			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cchUnicodeStringLen = nFieldLen / sizeof(char16_t);
			val.pszUnicodeString = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
			SkipNextField();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
		{
			valExtra.nValueType = val.nValueType;

			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cchUTF8StringLen = nFieldLen;
			val.pszUTF8String = (const char*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

			unsigned char nLengthBytes2 = 0;
			long nFieldLen2 = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen, &nLengthBytes2);
			valExtra.cchUTF8StringLen = nFieldLen2;
			valExtra.pszUTF8String = (const char*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen + nLengthBytes2);

			SkipNextField();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
		{
			valExtra.nValueType = val.nValueType;

			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cchUnicodeStringLen = nFieldLen / sizeof(char16_t);
			val.pszUnicodeString = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

			unsigned char nLengthBytes2 = 0;
			long nFieldLen2 = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen, &nLengthBytes2);
			valExtra.cchUnicodeStringLen = nFieldLen2 / sizeof(char16_t);
			valExtra.pszUnicodeString = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen + nLengthBytes2);

			SkipNextField();
		}
		break;
		case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		{
			val.nValueType = STXPROTOCOL_DATA_TYPE_UTF8;
			valExtra.nValueType = STXPROTOCOL_DATA_TYPE_DWORD;

			unsigned char nLengthBytes = 0;
			long nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
			val.cchUTF8StringLen = nFieldLen;
			val.pszUTF8String = (const char*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

			valExtra.dwVal = *((uint32_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen));

			SkipNextField();
		}
		break;
		}

		pfnEnum(originalType, &val, &valExtra, pUserData);

		if (val.nValueType == STXPROTOCOL_DATA_TYPE_OBJECT && val.pObject)
			delete val.pObject;

		nValEnumCount++;
	}
	return nValEnumCount;
}

unsigned char CSTXProtocol::GetNextFieldType()
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		return *((unsigned char*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	}
	return STXPROTOCOL_DATA_TYPE_INVALID;
}

const char* CSTXProtocol::GetTypeString( unsigned char nType )
{
	switch(nType)
	{
	case STXPROTOCOL_DATA_TYPE_BYTE:
		return "Byte";
	case STXPROTOCOL_DATA_TYPE_WORD:
		return "Word";
	case STXPROTOCOL_DATA_TYPE_DWORD:
		return "DWord";
	case STXPROTOCOL_DATA_TYPE_I64:
		return "int64";
	case STXPROTOCOL_DATA_TYPE_FLOAT:
		return "Float";
	case STXPROTOCOL_DATA_TYPE_DOUBLE:
		return "Double";
	case STXPROTOCOL_DATA_TYPE_GUID:
		return "GUID";
	case STXPROTOCOL_DATA_TYPE_UTF8:
		return "UTF-8";
	case STXPROTOCOL_DATA_TYPE_UNICODE:
		return "Unicode";
	case STXPROTOCOL_DATA_TYPE_OBJECT:
		return "Object";
	case STXPROTOCOL_DATA_TYPE_RAW:
		return "Raw";
	case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
		return "UTF-8 Pair";
	case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
		return "Unicode Pair";
	case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		return "UTF-8 to uint32_t Pair";
	}

	return "(Unknown Type)";
}

int CSTXProtocol::GetNextUnicodeString(char *lpBuffer, int cchBufferLen )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UNICODE;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextUnicodeString(char*,int) : Next field is not UTF8/Unicode type.");
		return -1;
	}

	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		throw std::runtime_error("GetNextUnicodeString(char*,int) : Not enough string data available.");
		return -1;
	}
	const char16_t* pStrBase = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(char16_t);

	std::string str = CSTXProtocolUtility::UTF16ToUTF8(pStrBase + cchStringLen);

	size_t convertedLen = str.size();
	if (cchBufferLen < convertedLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, str.c_str(), convertedLen);
	lpBuffer[convertedLen] = 0;

	SkipNextField();

	return convertedLen;
}

int CSTXProtocol::GetNextUnicodeString(char16_t *lpBuffer, int cchBufferLen)
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UNICODE;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextUnicodeString(char16_t*,int) : Next field is not UTF8/Unicode type.");
		return -1;
	}

	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if (nStringLen == -1)
	{
		return -1;
	}

	if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		throw std::runtime_error("GetNextUnicodeString(char16_t*,int) : Not enough string data available.");
		return -1;
	}
	const char16_t* pStrBase = (const char16_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(char16_t);

	if (cchBufferLen < cchStringLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, pStrBase, cchStringLen * sizeof(char16_t));
	lpBuffer[cchStringLen] = 0;

	SkipNextField();

	return cchStringLen;
}

void CSTXProtocol::SeekReadToBegin()
{
	m_nCurentReadOffset = 0;
}

void CSTXProtocol::Clear()
{
	ResetPosition();
}

long CSTXProtocol::AppendDataPair(const char *lpszVal1, const char *lpszVal2)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_PAIR);
	int nTotalLength = 0;
	int len1 = strlen(lpszVal1);
	int len2 = strlen(lpszVal2);

	//std::wstring_convert<std::codecvt_utf8<char16_t>> conv;
	//std::wstring str = conv.from_bytes(pStrBase, pStrBase + nLengthBytes);

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len1);
	Expand(len1);
	WriteRawData((void*)lpszVal1, len1);	//Skip nullptr-termination
	nTotalLength += len1;

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len2);
	Expand(len2);
	WriteRawData((void*)lpszVal2, len2);	//Skip nullptr-termination
	nTotalLength += len2;

	return nTotalLength;
}

long CSTXProtocol::AppendDataPair(const char16_t *lpszVal1, const char16_t *lpszVal2)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_PAIR);
	int nTotalLength = 0;

	std::string str1 = CSTXProtocolUtility::UTF16ToUTF8(lpszVal1);
	std::string str2 = CSTXProtocolUtility::UTF16ToUTF8(lpszVal2);

	size_t len1 = str1.size();
	size_t len2 = str2.size();

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len1);
	Expand(len1);
	WriteRawData((void*)str1.c_str(), len1);	//Skip nullptr-termination
	nTotalLength += len1;

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len2);
	Expand(len2);
	WriteRawData((void*)str2.c_str(), len2);	//Skip nullptr-termination
	nTotalLength += len2;

	return nTotalLength;
}

long CSTXProtocol::AppendDataPair( const char *lpszVal, uint32_t dwVal )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR);
	int nTotalLength = 0;

	int len1 = strlen(lpszVal);

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len1);
	Expand(len1);
	WriteRawData((void*)lpszVal, len1);	//Skip nullptr-termination
	nTotalLength += len1;

	Expand(sizeof(dwVal));
	WriteRawData(&dwVal, sizeof(dwVal));
	nTotalLength += sizeof(dwVal);

	return nTotalLength;
}

long CSTXProtocol::AppendDataPair(const char16_t *lpszVal, uint32_t dwVal)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR);
	int nTotalLength = 0;

	std::string str1 = CSTXProtocolUtility::UTF16ToUTF8(lpszVal);
	size_t len1 = str1.size();

	//WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(len1);
	Expand(len1);
	WriteRawData((void*)str1.c_str(), len1);	//Skip nullptr-termination
	nTotalLength += len1;

	Expand(sizeof(dwVal));
	WriteRawData(&dwVal, sizeof(dwVal));
	nTotalLength += sizeof(dwVal);

	return nTotalLength;
}

uint32_t CSTXProtocol::GetNextStringPair(char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2 )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UTF8_PAIR;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextStringPair(char*,int,char*,int) : Next field is not String Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, nullptr, nullptr);

	SkipNextField();

	return 0;
}

uint32_t CSTXProtocol::GetNextStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2 )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UTF8_PAIR;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextStringPair(char16_t*,int,char16_t*,int) : Next field is not String Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, nullptr, nullptr);

	SkipNextField();

	return 0;
}

int CSTXProtocol::DecodeUTF8String( void *pDataPtr, char *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen)
{
	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(pOriginalStringPrefixLen)
		*pOriginalStringPrefixLen = nLengthBytes;

	if(pOriginalStringLen)
		*pOriginalStringLen = nStringLen;

	const char* pStrBase = ((char*)pDataPtr) + nLengthBytes;

	if (cchBufferLen < nStringLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, pStrBase, nStringLen);
	lpBuffer[nStringLen] = 0;

	return nStringLen;

}

int CSTXProtocol::DecodeUTF8String(void *pDataPtr, char16_t *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen)
{
	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
	if (nStringLen == -1)
	{
		return -1;
	}

	if (pOriginalStringPrefixLen)
		*pOriginalStringPrefixLen = nLengthBytes;

	if (pOriginalStringLen)
		*pOriginalStringLen = nStringLen;

	const char* pStrBase = ((char*)pDataPtr) + nLengthBytes;

	std::u16string str1 = CSTXProtocolUtility::UTF8ToUTF16(pStrBase, pStrBase + nStringLen);

	size_t convertedLen = str1.size();

	if (cchBufferLen < convertedLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, str1.c_str(), convertedLen * sizeof(char16_t));
	lpBuffer[convertedLen] = 0;

	return convertedLen;

}

int CSTXProtocol::DecodeUnicodeString( void *pDataPtr, char *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen )
{
	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(pOriginalStringPrefixLen)
		*pOriginalStringPrefixLen = nLengthBytes;

	if(pOriginalStringLen)
		*pOriginalStringLen = nStringLen;

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		//AssertBreak(_T("DecodeUnicodeString() : Not enough string data available."));
		return -1;
	}
	const char16_t* pStrBase = (const char16_t*)((char*)pDataPtr + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(char16_t);

	std::string str = CSTXProtocolUtility::UTF16ToUTF8(pStrBase, pStrBase + cchStringLen);

	size_t convertedLen = str.size();
	if (cchBufferLen < convertedLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, str.c_str(), convertedLen);
	lpBuffer[convertedLen] = 0;

	return convertedLen;
}

int CSTXProtocol::DecodeUnicodeString(void *pDataPtr, char16_t *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen)
{
	unsigned char nLengthBytes = 0;
	long nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
	if (nStringLen == -1)
	{
		return -1;
	}

	if (pOriginalStringPrefixLen)
		*pOriginalStringPrefixLen = nLengthBytes;

	if (pOriginalStringLen)
		*pOriginalStringLen = nStringLen;

	if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		//AssertBreak(_T("DecodeUnicodeString() : Not enough string data available."));
		return -1;
	}
	const char16_t* pStrBase = (const char16_t*)((char*)pDataPtr + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(char16_t);

	if (cchBufferLen < cchStringLen + 1)
	{
		return -1;
	}

	memcpy(lpBuffer, pStrBase, nStringLen);
	lpBuffer[cchStringLen] = 0;

	return cchStringLen;
}

uint32_t CSTXProtocol::GetNextUnicodeStringPair( char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2 )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UNICODE_PAIR;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextUnicodeStringPair(char*,int,char*,int) : Next field is not String Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, nullptr, nullptr);

	SkipNextField();

	return 0;
}

uint32_t CSTXProtocol::GetNextUnicodeStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2)
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UNICODE_PAIR;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextObject() : Next field is not String Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, nullptr, nullptr);

	SkipNextField();

	return 0;
}

uint32_t CSTXProtocol::GetNextStringToDWORDPair( char *lpBuffer, int cchBufferLen )
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR;
	if(!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextStringToDWORDPair(char*,int) : Next field is not StringToDWord Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer, cchBufferLen, &nPrefixLen, &nStringLen);
	uint32_t dwValue = *((uint32_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen));

	SkipNextField();

	return dwValue;
}

uint32_t CSTXProtocol::GetNextStringToDWORDPair(char16_t *lpBuffer, int cchBufferLen)
{
	const unsigned char nType = STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR;
	if (!IsValidDataType(nType))
	{
		throw std::runtime_error("GetNextStringToDWORDPair(char16_t*,int) : Next field is not StringToDWord Pair type.");
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer, cchBufferLen, &nPrefixLen, &nStringLen);
	uint32_t dwValue = *((uint32_t*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen));

	SkipNextField();

	return dwValue;
}

void CSTXProtocol::IncreaseDWORDAtOffset( long nOffset )
{
	(*((uint32_t*)(GetDataContentBasePtr() + nOffset)))++;
}


