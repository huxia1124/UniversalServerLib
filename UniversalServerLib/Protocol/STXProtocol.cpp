#include "STXProtocol.h"
#include <string.h>
#include <tchar.h>


#define STXPROTOCOL_INITIAL_BUFFER_SIZE				256
#define STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE		16
#define STXPROTOCOL_INITIAL_STRING_SIZE				256


//////////////////////////////////////////////////////////////////////////
// CSTXProtocolUtility

int CSTXProtocolUtility::ConvertString(LPCSTR pszUTF8, int cchUTF8, LPTSTR pszBuffer, int cchBuffer)
{
	LPCSTR pStrBase = pszUTF8;

#ifdef UNICODE
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, cchUTF8, NULL, 0);
	if(pszBuffer == NULL)
		return nWideBufferLen + 1;

	int nResult = MultiByteToWideChar(CP_UTF8, 0, pStrBase, cchUTF8, pszBuffer, cchBuffer);
	if(nResult < cchBuffer)
	{
		pszBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#else
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, cchUTF8, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideBufferLen];
	MultiByteToWideChar(CP_UTF8, 0, pStrBase, cchUTF8, pBufferW, nWideBufferLen);
	int nBufferLen = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	if(pszBuffer == NULL)
	{
		delete []pBufferW;
		return nBufferLen;
	}
	int nResult = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, pszBuffer, cchBuffer, NULL, NULL);
	delete []pBufferW;

	if(nResult < cchBuffer)
	{
		pszBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#endif
}

int CSTXProtocolUtility::ConvertString( LPCWSTR pszUnicode, int cchUnicode, LPTSTR pszBuffer, int cchBuffer )
{
	LPCWSTR pStrBase = pszUnicode;
	int cchStringLen = cchUnicode;

#ifdef UNICODE
	int nCopyMax = min(cchStringLen, cchBuffer);
	if(pszBuffer == NULL)
		return cchStringLen + 1;

	_tcsncpy_s(pszBuffer, cchBuffer, pStrBase, nCopyMax);

	return nCopyMax;

#else
	int nAnsiLen = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, NULL, 0, NULL, NULL);
	if(pszBuffer == NULL)
		return nAnsiLen + 1;

	int nConverted = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, pszBuffer, cchBuffer, NULL, NULL);
	if(cchBuffer > nConverted)
		pszBuffer[nConverted] = 0;

	return nConverted + 1;
#endif
}

int CSTXProtocolUtility::ConvertStringToUTF8( LPCTSTR pszSrc, LPSTR pszBuffer, int cchBuffer )
{
#ifdef UNICODE
	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pszSrc, -1, NULL, 0, NULL, NULL);
	if(pszBuffer == NULL)
		return nBufferLen;

	WideCharToMultiByte(CP_UTF8, 0, pszSrc, -1, pszBuffer, cchBuffer, NULL, NULL);

	return nBufferLen;
#else
	int nWideCharLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideCharLen];
	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pBufferW, nWideCharLen);

	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	if(pszBuffer == NULL)
	{
		delete []pBufferW;
		return nBufferLen;
	}
	WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, pszBuffer, cchBuffer, NULL, NULL);
	
	delete []pBufferW;
	return nBufferLen;
#endif
}

int CSTXProtocolUtility::ConvertStringToUnicode( LPCTSTR pszSrc, LPWSTR pszBuffer, int cchBuffer )
{
#ifdef UNICODE
	int nCopyMax = min((int)_tcslen(pszSrc), cchBuffer);
	_tcscpy_s(pszBuffer, cchBuffer, pszSrc);
	return nCopyMax;
#else
	int cchBufferLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	if(pszBuffer == NULL)
		return cchBufferLen;

	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pszBuffer, cchBuffer);
	return cchBufferLen;
#endif
}

//////////////////////////////////////////////////////////////////////////
// CSTXProtocolString

CSTXProtocolString::CSTXProtocolString()
{
	m_cchBufferSize = 0;
	m_pBuffer = NULL;
	ExpandTo(STXPROTOCOL_INITIAL_STRING_SIZE);
#ifdef UNICODE
	m_cchBufferSizeA = 0;
	m_pBufferA = NULL;
#endif
}

CSTXProtocolString::CSTXProtocolString( STXPROTOCOLVALUE *pVal )
{
	m_cchBufferSize = 0;
	m_pBuffer = NULL;

	if(pVal)
	{
		if((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_UTF8)
		{
			int nExpand = CSTXProtocolUtility::ConvertString(pVal->pszUTF8String, pVal->cchUTF8StringLen, NULL, 0);
			ExpandTo(nExpand);
			CSTXProtocolUtility::ConvertString(pVal->pszUTF8String, pVal->cchUTF8StringLen, m_pBuffer, m_cchBufferSize);
			return;
		}
		else if((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_UNICODE)
		{
			int nExpand = CSTXProtocolUtility::ConvertString(pVal->pszUnicodeString, pVal->cchUnicodeStringLen, NULL, 0);
			ExpandTo(nExpand);
			CSTXProtocolUtility::ConvertString(pVal->pszUnicodeString, pVal->cchUnicodeStringLen, m_pBuffer, m_cchBufferSize);
			return;
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_BYTE)
		{
			char szValue[8] = { 0 };
			_itoa_s(pVal->byteVal, szValue, 10);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_WORD)
		{
			char szValue[12] = { 0 };
			_itoa_s(pVal->wVal, szValue, 10);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_DWORD)
		{
			char szValue[20] = {0};
			_itoa_s(pVal->dwVal, szValue, 10);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_I64)
		{
			char szValue[32] = { 0 };
			_i64toa_s(pVal->nVal64, szValue, 32, 10);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_FLOAT)
		{
			char szValue[64] = { 0 };
			_gcvt_s(szValue, 64, pVal->floatVal, 16);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_DOUBLE)
		{
			char szValue[64] = { 0 };
			_gcvt_s(szValue, 64, pVal->doubleVal, 32);
			int nLen = strlen(szValue);
			ExpandTo(nLen);
			CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_GUID)
		{
			TCHAR szValue[64] = { 0 };
			if (StringFromGUID2(pVal->guidVal, szValue, 64))
			{
				int nLen = _tcslen(szValue);
				ExpandTo(nLen);
				CSTXProtocolUtility::ConvertString(szValue, nLen, m_pBuffer, m_cchBufferSize);
			}
		}
		else if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_RAW)
		{
			TCHAR *pszHex = new TCHAR[pVal->cbDataLen * 2];
			ConvertToHexString((LPBYTE)pVal->pDataPtr, pVal->cbDataLen * 2, pszHex, TRUE);
			ExpandTo(pVal->cbDataLen * 2 * sizeof(TCHAR));
			CSTXProtocolUtility::ConvertString(pszHex, pVal->cbDataLen * 2, m_pBuffer, m_cchBufferSize);		
		}
		else		// if ((pVal->nValueType & 0x8F) == STXPROTOCOL_DATA_TYPE_INVALID)
		{
			TCHAR szValue[64] = _T("<Invalid Value>");
			int nLen = _tcslen(szValue);
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
		m_pBuffer = NULL;
	}

#ifdef UNICODE
	if(m_pBufferA)
		delete []m_pBufferA;
#endif
}

LONG CSTXProtocolString::ConvertToHexString(LPBYTE lpData, DWORD cbDataLen, LPTSTR lpszHexBuf, BOOL bUpperCased)
{
	static const TCHAR szHexDictUpper[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static const TCHAR szHexDictLower[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	DWORD i = 0;
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

void CSTXProtocolString::ExpandTo( int cchMax )
{
	while(cchMax > m_cchBufferSize)
	{
		LONG cchNewBufferLen = max(m_cchBufferSize * 2, STXPROTOCOL_INITIAL_STRING_SIZE);
		TCHAR *pNewBuffer = new TCHAR[cchNewBufferLen];
		pNewBuffer[0] = 0;
		if(m_pBuffer)
		{
			_tcscpy_s(pNewBuffer, cchNewBufferLen, m_pBuffer);
			delete []m_pBuffer;
		}
		m_pBuffer = pNewBuffer;
		m_cchBufferSize = cchNewBufferLen;
	}
}

LPTSTR CSTXProtocolString::GetBuffer( int nBufferLength /*= 0*/ )
{
	if(nBufferLength > 0)
		ExpandTo(nBufferLength);

	return m_pBuffer;
}

CSTXProtocolString::operator LPCTSTR()
{
	return m_pBuffer;
}

#ifdef UNICODE
CSTXProtocolString::operator LPCSTR()
{
	if(m_pBufferA == NULL)
	{
		int cchBufSize = WideCharToMultiByte(CP_ACP, 0, m_pBuffer, -1, NULL, 0, NULL, NULL);
		m_pBufferA = new char[cchBufSize + 2];
		WideCharToMultiByte(CP_ACP, 0, m_pBuffer, -1, m_pBufferA, cchBufSize + 2, NULL, NULL);
	}
	return m_pBufferA;
}
#endif

int CSTXProtocolString::GetLength()
{
	return _tcslen(m_pBuffer);
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
	m_pBuffer = NULL;
}

LONG CSTXProtocol::Expand( LONG nAdd )
{
	// 根据需要，动态增加缓冲区空间
	while(m_nCurentWriteOffset + nAdd > m_nBufferLen)
	{
		LONG nNewBufferLen = m_nBufferLen * 2;
		char *pNewBuffer = new char[nNewBufferLen + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE];
		memcpy(pNewBuffer, m_pBuffer, m_nCurentWriteOffset + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE);
		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
		m_nBufferLen = nNewBufferLen;
	}
	return nAdd;
}

LONG CSTXProtocol::GetDataLen()
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

LONG CSTXProtocol::GetEncryptedData(void *pBuffer, LONG cbBufferLen, DWORD dwKey)
{
	LONG cbDataLen = GetDataLen() - m_nPrefixLen - 1;
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

LONG CSTXProtocol::GetTypedDataLen(BYTE nType)
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

LONG CSTXProtocol::WriteDataType( BYTE nType )
{
	Expand(sizeof(nType));
	// 写入字段类型
	*((BYTE*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = nType;
	m_nCurentWriteOffset++;
	UpdatePrefix();
	UpdateCRC(&nType, 1);
	return 1;
}

LONG CSTXProtocol::AppendData( BYTE val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_BYTE);
	Expand(sizeof(val));
	*((BYTE*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC(&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData( WORD val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_WORD);
	Expand(sizeof(val));
	*((WORD*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((BYTE*)&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData( DWORD val ,LONG *pOffset)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_DWORD);
	Expand(sizeof(val));
	*((DWORD*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	if(pOffset)
		*pOffset = m_nCurentWriteOffset;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((BYTE*)&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData(FLOAT val)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_FLOAT);
	Expand(sizeof(val));
	*((FLOAT*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((BYTE*)&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData(DOUBLE val)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_DOUBLE);
	Expand(sizeof(val));
	*((DOUBLE*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((BYTE*)&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData( __int64 val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_I64);
	Expand(sizeof(val));
	*((__int64*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	UpdateCRC((BYTE*)&val, sizeof(val));
	return sizeof(val);
}

LONG CSTXProtocol::AppendData( LPCTSTR lpszVal )
{
#ifdef UNICODE
	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, lpszVal, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, lpszVal, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);		//CRC updated
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	//UpdatePrefix();
	return nBufferLen - 1;
#else
	int nWideCharLen = MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideCharLen];
	MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, pBufferW, nWideCharLen);

	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	delete []pBufferW;
	//UpdatePrefix();
	return nBufferLen;
#endif
}

#ifdef UNICODE
LONG CSTXProtocol::AppendData( LPCSTR lpszVal )
{
	int nWideCharLen = MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideCharLen];
	MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, pBufferW, nWideCharLen);

	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	delete []pBufferW;
	//UpdatePrefix();
	return nBufferLen;
}
#endif

LONG CSTXProtocol::AppendUnicodeString( LPCWSTR lpszVal )
{
	if(lpszVal == NULL)
		return -1;

	int nStrLen = wcslen(lpszVal);
	LONG cbLen = nStrLen * sizeof(WCHAR);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)lpszVal, cbLen);	//Skip NULL-termination
	return cbLen;
}

LONG CSTXProtocol::AppendUnicodeStringPair( LPCWSTR lpszVal1, LPCWSTR lpszVal2 )
{
	if(lpszVal1 == NULL || lpszVal2 == NULL)
		return -1;

	WriteDataType(STXPROTOCOL_DATA_TYPE_UNICODE_PAIR);

	int nStrLen = wcslen(lpszVal1);
	LONG cbLen = nStrLen * sizeof(WCHAR);
	WriteCompactInteger(cbLen);
	Expand(cbLen);
	WriteRawData((void*)lpszVal1, cbLen);	//Skip NULL-termination

	int nStrLen2 = wcslen(lpszVal2);
	LONG cbLen2 = nStrLen2 * sizeof(WCHAR);
	WriteCompactInteger(cbLen2);
	Expand(cbLen2);
	WriteRawData((void*)lpszVal2, cbLen2);	//Skip NULL-termination

	return cbLen + cbLen2;
}

LONG CSTXProtocol::AppendData( GUID &val )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_GUID);
	Expand(sizeof(val));
	*((GUID*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = val;
	m_nCurentWriteOffset += sizeof(val);
	UpdatePrefix();
	return sizeof(val);
}

LONG CSTXProtocol::AppendData( CSTXProtocol *pVal )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_OBJECT);
	LONG nDataLen = pVal->GetDataLen();
	WriteCompactInteger(nDataLen);
	WriteRawData(pVal->GetBasePtr(), nDataLen);
	//UpdatePrefix();
	return nDataLen;
}

LONG CSTXProtocol::AppendRawData(void *pData, LONG cbDataLen)
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_RAW);
	WriteCompactInteger(cbDataLen);
	WriteRawData(pData, cbDataLen);
	return cbDataLen;
}

LONG CSTXProtocol::GetCompactIntegerLen( UINT nValue )
{
	if(nValue == 0)
		return 1;

	LONG nByteLen = 0;
	while(nValue > 0)
	{
		nValue >>= 7;
		nByteLen++;
	}
	return nByteLen;
}

LONG CSTXProtocol::WriteCompactInteger( UINT nValue )
{
	Expand(GetCompactIntegerLen(nValue));
	if(nValue == 0)
	{
		*((BYTE*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = 0;
		m_nCurentWriteOffset++;
		UpdatePrefix();
		UpdateCRC((BYTE*)&nValue, 1);
		return 1;
	}

	LONG nBytesWritten = 0;
	while(nValue > 0)
	{
		BYTE nByteToWrite = (BYTE)(nValue & 0x0000007F);
		nValue >>= 7;
		if(nValue > 0)
			nByteToWrite |= 0x80;

		*((BYTE*)(GetDataContentBasePtr() + m_nCurentWriteOffset + nBytesWritten)) = nByteToWrite;
		nBytesWritten++;
		UpdateCRC((BYTE*)&nByteToWrite, 1);
	}
	m_nCurentWriteOffset += nBytesWritten;
	UpdatePrefix();
	return nBytesWritten;
}

LONG CSTXProtocol::WriteRawData( void *pData, LONG cbDataLen )
{
	Expand(cbDataLen);
	memcpy(GetDataContentBasePtr() + m_nCurentWriteOffset, pData, cbDataLen);
	m_nCurentWriteOffset += cbDataLen;
	UpdatePrefix();
	UpdateCRC((BYTE*)pData, cbDataLen);
	return cbDataLen;
}

char* CSTXProtocol::GetDataContentBasePtr()
{
	return m_pBuffer + STXPROTOCOL_INITIAL_BUFFER_PREFIX_SIZE;
}

void CSTXProtocol::UpdateCRC(BYTE *pAddedData, UINT cbDataLen)
{
	BYTE &currentCRC = *(BYTE*)GetCRCBytePtr();
	for (UINT i = 0; i < cbDataLen; i++)
	{
		currentCRC += pAddedData[i];
	}
}

void CSTXProtocol::UpdatePrefix()
{
	m_nPrefixLen = (BYTE)GetCompactIntegerLen(m_nCurentWriteOffset);
	UINT nValue = m_nCurentWriteOffset + 1;
	char* pBasePtr = (char*)GetBasePtr();
	if(nValue == 1)
	{
		*((BYTE*)pBasePtr) = 1;
		return;
	}
	while(nValue > 1)
	{
		BYTE nByteToWrite = (BYTE)(nValue & 0x0000007F);
		nValue >>= 7;
		if(nValue > 0)
			nByteToWrite |= 0x80;

		*((BYTE*)pBasePtr) = nByteToWrite;
		pBasePtr++;
	}
}

BOOL CSTXProtocol::DecodeEmbeddedObject( void *pData, LONG *pDataReadLen )
{
	BYTE nLengthBytes = 0;
	LONG nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
	if(nObjectContentLength == -1)
		return FALSE;

	BYTE nContentLengthBytes = 0;
	LONG nContentLength = DecodeCompactInteger(((char*)pData) + nLengthBytes , &nContentLengthBytes);
	if(nContentLength == -1)
		return FALSE;

	ResetPosition();
	Expand(nObjectContentLength);
	WriteRawData(((char*)pData) + nLengthBytes + nContentLengthBytes, nContentLength);
	if(pDataReadLen)
		*pDataReadLen = nLengthBytes + nObjectContentLength;

	return TRUE;
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

int CSTXProtocol::Decode(void *pData, LONG *pDataReadLen)
{
	return Decode(pData, pDataReadLen, 0);
}

int CSTXProtocol::Decode( void *pData, LONG *pDataReadLen, LONG cbInputDataLen)
{
	BYTE nLengthBytes = 0;
	LONG nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
	if(nObjectContentLength == -1 || nObjectContentLength < 1)
		return 1;	//Error parsing package length 

	if (cbInputDataLen > 0 && nObjectContentLength + nLengthBytes + 1 > cbInputDataLen)
		return 2;	//Error not enough actual data

	BYTE &crc = ((BYTE*)pData)[nLengthBytes];
	BYTE crcFrom = 0;
	for (LONG i = 1; i < nObjectContentLength; i++)
	{
		crcFrom += ((BYTE*)pData)[nLengthBytes + i];
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

int CSTXProtocol::DecodeWithDecrypt(void * pData, LONG * pDataReadLen, DWORD dwKey)
{
	BYTE nLengthBytes = 0;
	LONG nObjectContentLength = DecodeCompactInteger(pData, &nLengthBytes);
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
	pszData[0] = crc;

	if (((char)(r & 0xFF)) != crc)
	{
		AssertBreak(_T("DecodeWithDecrypt() : Error parsing package, CRC mismatch."));
		return 8;	//Error parsing package, CRC mismatch 
	}


	return Decode(pData, pDataReadLen);
}

LONG CSTXProtocol::DecodeCompactInteger(void *pData, BYTE *pLengthBytes)
{
	if(pData == NULL)
		return -1;

	BYTE *pPtr = (BYTE*)pData;
	int nDecodedBytes = 0;

	UINT nValue = 0;
	UINT nValueUnit = 0;
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
	UpdatePrefix();
}

BOOL CSTXProtocol::IsValidDataType( BYTE nType )
{
	if(m_nCurentReadOffset >= m_nCurentWriteOffset)
		return FALSE;

	BYTE nTypeInData =  *((BYTE*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	return nTypeInData == nType;
}

inline void CSTXProtocol::AssertBreak(LPCTSTR lpszError)
{
#ifdef _DEBUG
	OutputDebugString(lpszError);
	OutputDebugString(_T("\n"));
	DebugBreak();
#endif
}

BOOL CSTXProtocol::SkipTypeIndicator(BYTE *pTypeSkipped)
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		if(pTypeSkipped)
			*pTypeSkipped =  *((BYTE*)(GetDataContentBasePtr() + m_nCurentReadOffset));

		m_nCurentReadOffset++;
		return TRUE;
	}
	return FALSE;
}

void CSTXProtocol::CheckDataAvailability( BYTE nType )
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset < GetTypedDataLen(nType))
		AssertBreak(_T("CheckDataAvailability() : Not enough data available!"));
}

BYTE CSTXProtocol::GetNextByte()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_BYTE;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextByte() : Next field is not Byte type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	BYTE nValue =  *((BYTE*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

WORD CSTXProtocol::GetNextWORD()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_WORD;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextWORD() : Next field is not Word type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	WORD nValue =  *((WORD*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

DWORD CSTXProtocol::GetNextDWORD()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_DWORD;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextDWORD() : Next field is not DWord type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	DWORD nValue =  *((DWORD*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

__int64 CSTXProtocol::GetNextI64()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_I64;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextI64() : Next field is not I64 type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	__int64 nValue =  *((__int64*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

FLOAT CSTXProtocol::GetNextFloat()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_FLOAT;
	if (!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextFloat() : Next field is not Float type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	FLOAT nValue = *((FLOAT*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

DOUBLE CSTXProtocol::GetNextDouble()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_DOUBLE;
	if (!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextDouble() : Next field is not Double type."));
		return 0;
	}

	SkipTypeIndicator();
	CheckDataAvailability(nType);

	DOUBLE nValue = *((DOUBLE*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	m_nCurentReadOffset += sizeof(nValue);
	return nValue;
}

GUID CSTXProtocol::GetNextGUID()
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_GUID;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextGUID() : Next field is not GUID type."));
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
	const BYTE nType = STXPROTOCOL_DATA_TYPE_OBJECT;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextObject() : Next field is not Object type."));
		return NULL;
	}

	LONG nDataReadLen = 0;
	CSTXProtocol *pNewObject = new CSTXProtocol();
	if(pNewObject->DecodeEmbeddedObject(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nDataReadLen))
	{
		SkipTypeIndicator();
		m_nCurentReadOffset += nDataReadLen;
		return pNewObject;
	}
	delete pNewObject;
	return NULL;
}

int CSTXProtocol::GetNextString( LPTSTR lpBuffer, int cchBufferLen )
{
	if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		return GetNextUnicodeString(lpBuffer, cchBufferLen);
	}

	const BYTE nType = STXPROTOCOL_DATA_TYPE_UTF8;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextString() : Next field is not UTF8/Unicode type."));
		return -1;
	}

	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		AssertBreak(_T("GetNextString() : Not enough string data available."));
		return -1;
	}

	LPCSTR pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

#ifdef UNICODE
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);
	if(lpBuffer == NULL)
		return nWideBufferLen;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	int nResult = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, lpBuffer, cchBufferLen);
	if(nResult < cchBufferLen)
	{
		lpBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#else
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideBufferLen];
	MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, pBufferW, nWideBufferLen);
	int nBufferLen = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	if(lpBuffer == NULL)
	{
		delete []pBufferW;
		return nBufferLen;
	}
	int nResult = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, lpBuffer, cchBufferLen, NULL, NULL);
	delete []pBufferW;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	if(nResult < cchBufferLen)
	{
		lpBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#endif

}

BOOL CSTXProtocol::GetNextString( CSTXProtocolString *pString )
{
	if(pString == NULL)
		return FALSE;

	if(IsValidDataType(STXPROTOCOL_DATA_TYPE_UNICODE))
	{
		return GetNextUnicodeString(pString);
	}

	const BYTE nType = STXPROTOCOL_DATA_TYPE_UTF8;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextString() : Next field is not UTF8/Unicode type."));
		return FALSE;
	}

	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nStringLen == -1)
	{
		return FALSE;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		AssertBreak(_T("GetNextString() : Not enough string data available."));
		return FALSE;
	}

	LPCSTR pStrBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;

#ifdef UNICODE
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	LPTSTR pBuffer = pString->GetBuffer(nWideBufferLen + 1);
	int nResult = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, pBuffer, nWideBufferLen + 1);
	if(nResult < nWideBufferLen + 1)
	{
		pBuffer[nResult] = 0;
		nResult++;
	}

	return TRUE;
#else
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideBufferLen];
	MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, pBufferW, nWideBufferLen);
	int nBufferLen = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, NULL, 0, NULL, NULL);

	LPTSTR pBuffer = pString->GetBuffer(nBufferLen + 1);

	int nResult = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, pBuffer, nBufferLen + 1, NULL, NULL);
	delete []pBufferW;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	if(nResult < nBufferLen + 1)
	{
		pBuffer[nResult] = 0;
		nResult++;
	}

	return TRUE;
#endif
}

BOOL CSTXProtocol::GetNextUnicodeString(CSTXProtocolString *pString)
{
	if (pString == NULL)
		return FALSE;

	if (IsValidDataType(STXPROTOCOL_DATA_TYPE_UTF8))
	{
		return GetNextString(pString);
	}

	const BYTE nType = STXPROTOCOL_DATA_TYPE_UNICODE;
	if (!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextUnicodeString() : Next field is not UTF8/Unicode type."));
		return FALSE;
	}

	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);		//in bytes
	if (nStringLen == -1)
	{
		return FALSE;
	}

	if (m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		AssertBreak(_T("GetNextUnicodeString() : Not enough string data available."));
		return FALSE;
	}

	LPCWSTR pStrBase = (LPCWSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

#ifdef UNICODE
	LPWSTR pBuffer = pString->GetBuffer((nStringLen / sizeof(WCHAR)) + 1);
	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	memcpy(pBuffer, pStrBase, nStringLen);
	pBuffer[nStringLen / sizeof(WCHAR)] = 0;
	return TRUE;

#else
	int nMBCSBufferLen = WideCharToMultiByte(CP_ACP, 0, pStrBase, nStringLen, NULL, 0, NULL, NULL);
	CHAR *pBufferA = new CHAR[nMBCSBufferLen];
	WideCharToMultiByte(CP_ACP, 0, pStrBase, nStringLen, pBufferA, nMBCSBufferLen, NULL, NULL);

	LPTSTR pBuffer = pString->GetBuffer(nMBCSBufferLen + 1);

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	memcpy(pBuffer, pBufferA, nMBCSBufferLen);
	pBuffer[nMBCSBufferLen] = 0;
	delete[]pBufferA;

	return TRUE;
#endif
}


void CSTXProtocol::SkipNextField()
{
	BYTE nTypeSkipped = 0;
	if(!SkipTypeIndicator(&nTypeSkipped))
		return;

	if(nTypeSkipped & STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
	{
		BYTE nLengthBytes = 0;
		LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset, &nLengthBytes);
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
			BYTE nLengthBytes = 0;
			LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset, &nLengthBytes);
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

BOOL CSTXProtocol::IsDataAvailable()
{
	return (m_nCurentWriteOffset - m_nCurentReadOffset > 0);
}

int CSTXProtocol::GetNextRawData( void *pBuffer, int cbBufferSize )
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_RAW;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextRawData() : Next field is not Raw type."));
		return -1;
	}

	BYTE nLengthBytes = 0;
	LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nFieldLen == -1)
	{
		return -1;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nFieldLen + nLengthBytes + 1)
	{
		AssertBreak(_T("GetNextRawData() : Not enough raw data available."));
		return -1;
	}

	if(pBuffer == NULL)
	{
		return nFieldLen;
	}

	if(nFieldLen > cbBufferSize)
	{
		AssertBreak(_T("GetNextRawData() : Buffer size is too small to store raw data."));
		return -1;
	}

	LPCSTR pFieldDataBase = GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes;
	memcpy_s(pBuffer, cbBufferSize, pFieldDataBase, nFieldLen);

	return nFieldLen;
}

int CSTXProtocol::GetNextFieldLength()
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		BYTE nType =  *((BYTE*)(GetDataContentBasePtr() + m_nCurentReadOffset));

		if(nType & STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
		{
			BYTE nLengthBytes = 0;
			LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
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
	AssertBreak(_T("GetNextFieldLength() : Failed to get the length of next field."));
	return -1;
}

int CSTXProtocol::EnumValues( STXProtocolEnumFunc pfnEnum, void *pUserData )
{
	int nValEnumCount = 0;
	STXPROTOCOLVALUE val;
	STXPROTOCOLVALUE valExtra;
	valExtra.nValueType = STXPROTOCOL_DATA_TYPE_INVALID;
	while(IsDataAvailable())
	{
		val.nValueType = GetNextFieldType();
		switch(val.nValueType)
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
				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cchUTF8StringLen = nFieldLen;
				val.pszUTF8String = (LPCSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
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
				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cbDataLen = nFieldLen;
				val.pDataPtr = (void*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
				SkipNextField();
			}
			break;
		case STXPROTOCOL_DATA_TYPE_UNICODE:
			{
				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cchUnicodeStringLen = nFieldLen / sizeof(WCHAR);
				val.pszUnicodeString = (LPCWSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
				SkipNextField();
			}
			break;
		case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
			{
				valExtra.nValueType = val.nValueType;

				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cchUTF8StringLen = nFieldLen;
				val.pszUTF8String = (LPCSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

				BYTE nLengthBytes2 = 0;
				LONG nFieldLen2 = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen, &nLengthBytes2);
				valExtra.cchUTF8StringLen = nFieldLen2;
				valExtra.pszUTF8String = (LPCSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen + nLengthBytes2);

				SkipNextField();
			}
			break;
		case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
			{
				valExtra.nValueType = val.nValueType;

				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cchUnicodeStringLen = nFieldLen / sizeof(WCHAR);
				val.pszUnicodeString = (LPCWSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

				BYTE nLengthBytes2 = 0;
				LONG nFieldLen2 = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen, &nLengthBytes2);
				valExtra.cchUnicodeStringLen = nFieldLen2 / sizeof(WCHAR);
				valExtra.pszUnicodeString = (LPCWSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen + nLengthBytes2);

				SkipNextField();
			}
			break;
		case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
			{
				valExtra.nValueType = val.nValueType;

				BYTE nLengthBytes = 0;
				LONG nFieldLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
				val.cchUTF8StringLen = nFieldLen;
				val.pszUTF8String = (LPCSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);

				valExtra.dwVal = *((DWORD*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes + nFieldLen));

				SkipNextField();
			}
			break;
		}

		pfnEnum(&val, &valExtra, pUserData);

		if(val.nValueType == STXPROTOCOL_DATA_TYPE_OBJECT && val.pObject)
			delete val.pObject;

		nValEnumCount++;
	}
	return nValEnumCount;
}

BYTE CSTXProtocol::GetNextFieldType()
{
	if(m_nCurentWriteOffset - m_nCurentReadOffset > 0)
	{
		return *((BYTE*)(GetDataContentBasePtr() + m_nCurentReadOffset));
	}
	return STXPROTOCOL_DATA_TYPE_INVALID;
}

LPCTSTR CSTXProtocol::GetTypeString( BYTE nType )
{
	switch(nType)
	{
	case STXPROTOCOL_DATA_TYPE_BYTE:
		return _T("Byte");
	case STXPROTOCOL_DATA_TYPE_WORD:
		return _T("Word");
	case STXPROTOCOL_DATA_TYPE_DWORD:
		return _T("DWord");
	case STXPROTOCOL_DATA_TYPE_I64:
		return _T("int64");
	case STXPROTOCOL_DATA_TYPE_FLOAT:
		return _T("Float");
	case STXPROTOCOL_DATA_TYPE_DOUBLE:
		return _T("Double");
	case STXPROTOCOL_DATA_TYPE_GUID:
		return _T("GUID");
	case STXPROTOCOL_DATA_TYPE_UTF8:
		return _T("UTF-8");
	case STXPROTOCOL_DATA_TYPE_UNICODE:
		return _T("Unicode");
	case STXPROTOCOL_DATA_TYPE_OBJECT:
		return _T("Object");
	case STXPROTOCOL_DATA_TYPE_RAW:
		return _T("Raw");
	case STXPROTOCOL_DATA_TYPE_UTF8_PAIR:
		return _T("UTF-8 Pair");
	case STXPROTOCOL_DATA_TYPE_UNICODE_PAIR:
		return _T("Unicode Pair");
	case STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR:
		return _T("UTF-8 to DWORD Pair");
	}

	return _T("(Unknown Type)");
}

int CSTXProtocol::GetNextUnicodeString( LPTSTR lpBuffer, int cchBufferLen )
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_UNICODE;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextUnicodeString() : Next field is not UTF8/Unicode type."));
		return -1;
	}

	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(GetDataContentBasePtr() + m_nCurentReadOffset + 1, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(m_nCurentWriteOffset - m_nCurentReadOffset < nStringLen + nLengthBytes + 1)
	{
		AssertBreak(_T("GetNextUnicodeString() : Not enough string data available."));
		return -1;
	}
	LPCWSTR pStrBase = (LPCWSTR)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(WCHAR);

#ifdef UNICODE
	int nCopyMax = min(cchBufferLen, cchStringLen);
	if(lpBuffer == NULL)
		return cchStringLen + 1;

	_tcsncpy_s(lpBuffer, cchBufferLen, pStrBase, nCopyMax);
	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;
	return nCopyMax;

#else
	int nAnsiLen = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, NULL, 0, NULL, NULL);
	if(lpBuffer == NULL)
		return nAnsiLen + 1;

	int nConverted = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, lpBuffer, cchBufferLen, NULL, NULL);
	if(cchBufferLen > nConverted)
	lpBuffer[nConverted] = 0;

	SkipTypeIndicator();
	m_nCurentReadOffset += nLengthBytes;
	m_nCurentReadOffset += nStringLen;

	return nConverted + 1;
#endif
}

void CSTXProtocol::SeekReadToBegin()
{
	m_nCurentReadOffset = 0;
}

LONG CSTXProtocol::AppendDataPair( LPCTSTR lpszVal1, LPCTSTR lpszVal2 )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_PAIR);
	int nTotalLength = 0;

#ifdef UNICODE
	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, lpszVal1, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, lpszVal1, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	//UpdatePrefix();
	nTotalLength += nBufferLen - 1;
#else
	int nWideCharLen = MultiByteToWideChar(CP_ACP, 0, lpszVal1, -1, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideCharLen];
	MultiByteToWideChar(CP_ACP, 0, lpszVal1, -1, pBufferW, nWideCharLen);

	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	delete []pBufferW;
	//UpdatePrefix();
	nTotalLength += nBufferLen;
#endif

#ifdef UNICODE
	int nBufferLen2 = WideCharToMultiByte(CP_UTF8, 0, lpszVal2, -1, NULL, 0, NULL, NULL);
	char *pBuffer2 = new char[nBufferLen2];
	WideCharToMultiByte(CP_UTF8, 0, lpszVal2, -1, pBuffer2, nBufferLen2, NULL, NULL);
	WriteCompactInteger(nBufferLen2 - 1);
	Expand(nBufferLen2 - 1);
	WriteRawData(pBuffer2, nBufferLen2 - 1);	//Skip NULL-termination
	delete []pBuffer2;
	//UpdatePrefix();
	nTotalLength += nBufferLen2 - 1;
#else
	int nWideCharLen2 = MultiByteToWideChar(CP_ACP, 0, lpszVal2, -1, NULL, 0);
	WCHAR *pBufferW2 = new WCHAR[nWideCharLen2];
	MultiByteToWideChar(CP_ACP, 0, lpszVal2, -1, pBufferW2, nWideCharLen2);

	int nBufferLen2 = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	char *pBuffer2 = new char[nBufferLen2];
	WideCharToMultiByte(CP_UTF8, 0, pBufferW2, -1, pBuffer2, nBufferLen2, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(nBufferLen2 - 1);
	Expand(nBufferLen2 - 1);
	WriteRawData(pBuffer2, nBufferLen2 - 1);	//Skip NULL-termination
	delete []pBuffer2;
	delete []pBufferW2;
	//UpdatePrefix();
	nTotalLength += nBufferLen;
#endif

	return nTotalLength;
}

LONG CSTXProtocol::AppendDataPair( LPCTSTR lpszVal, DWORD dwVal )
{
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR);
	int nTotalLength = 0;

#ifdef UNICODE
	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, lpszVal, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, lpszVal, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	//UpdatePrefix();
	nTotalLength += nBufferLen - 1;
#else
	int nWideCharLen = MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideCharLen];
	MultiByteToWideChar(CP_ACP, 0, lpszVal, -1, pBufferW, nWideCharLen);

	int nBufferLen = WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	char *pBuffer = new char[nBufferLen];
	WideCharToMultiByte(CP_UTF8, 0, pBufferW, -1, pBuffer, nBufferLen, NULL, NULL);
	WriteDataType(STXPROTOCOL_DATA_TYPE_UTF8);
	WriteCompactInteger(nBufferLen - 1);
	Expand(nBufferLen - 1);
	WriteRawData(pBuffer, nBufferLen - 1);	//Skip NULL-termination
	delete []pBuffer;
	delete []pBufferW;
	//UpdatePrefix();
	nTotalLength += nBufferLen;
#endif

	Expand(sizeof(dwVal));
	*((DWORD*)(GetDataContentBasePtr() + m_nCurentWriteOffset)) = dwVal;
	m_nCurentWriteOffset += sizeof(dwVal);
	nTotalLength += sizeof(dwVal);

	UpdatePrefix();

	return nTotalLength;
}

DWORD CSTXProtocol::GetNextStringPair( LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2 )
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_UTF8_PAIR;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextStringPair() : Next field is not String Pair type."));
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, NULL, NULL);

	SkipNextField();

	return 0;
}

int CSTXProtocol::DecodeUTF8String( void *pDataPtr, LPTSTR lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen)
{
	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
	if(nStringLen == -1)
	{
		return -1;
	}

	if(pOriginalStringPrefixLen)
		*pOriginalStringPrefixLen = nLengthBytes;

	if(pOriginalStringLen)
		*pOriginalStringLen = nStringLen;

	LPCSTR pStrBase = ((char*)pDataPtr) + nLengthBytes;

#ifdef UNICODE
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);
	if(lpBuffer == NULL)
		return nWideBufferLen;

	int nResult = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, lpBuffer, cchBufferLen);
	if(nResult < cchBufferLen)
	{
		lpBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#else
	int nWideBufferLen = MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, NULL, 0);
	WCHAR *pBufferW = new WCHAR[nWideBufferLen];
	MultiByteToWideChar(CP_UTF8, 0, pStrBase, nStringLen, pBufferW, nWideBufferLen);
	int nBufferLen = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, NULL, 0, NULL, NULL);
	if(lpBuffer == NULL)
	{
		delete []pBufferW;
		return nBufferLen;
	}
	int nResult = WideCharToMultiByte(CP_ACP, 0, pBufferW, -1, lpBuffer, cchBufferLen, NULL, NULL);
	delete []pBufferW;

	if(nResult < cchBufferLen)
	{
		lpBuffer[nResult] = 0;
		nResult++;
	}

	return nResult;
#endif

}

int CSTXProtocol::DecodeUnicodeString( void *pDataPtr, LPTSTR lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen )
{
	BYTE nLengthBytes = 0;
	LONG nStringLen = DecodeCompactInteger(pDataPtr, &nLengthBytes);
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
		AssertBreak(_T("DecodeUnicodeString() : Not enough string data available."));
		return -1;
	}
	LPCWSTR pStrBase = (LPCWSTR)((char*)pDataPtr + nLengthBytes);
	int cchStringLen = nStringLen / sizeof(WCHAR);

#ifdef UNICODE
	int nCopyMax = min(cchBufferLen, cchStringLen);
	if(lpBuffer == NULL)
		return cchStringLen + 1;

	_tcsncpy_s(lpBuffer, cchBufferLen, pStrBase, nCopyMax);
	return nCopyMax;

#else
	int nAnsiLen = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, NULL, 0, NULL, NULL);
	if(lpBuffer == NULL)
		return nAnsiLen + 1;

	int nConverted = WideCharToMultiByte(CP_ACP, 0, pStrBase, cchStringLen, lpBuffer, cchBufferLen, NULL, NULL);
	if(cchBufferLen > nConverted)
		lpBuffer[nConverted] = 0;

	return nConverted + 1;
#endif
}

DWORD CSTXProtocol::GetNextUnicodeStringPair( LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2 )
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_UNICODE_PAIR;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextStringPair() : Next field is not String Pair type."));
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer1, cchBufferLen1, &nPrefixLen, &nStringLen);
	DecodeUnicodeString(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen, lpBuffer2, cchBufferLen2, NULL, NULL);

	SkipNextField();

	return 0;
}

DWORD CSTXProtocol::GetNextStringToDWORDPair( LPTSTR lpBuffer, int cchBufferLen )
{
	const BYTE nType = STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR;
	if(!IsValidDataType(nType))
	{
		AssertBreak(_T("GetNextStringToDWORDPair() : Next field is not StringToDWord Pair type."));
		return -1;
	}

	int nPrefixLen = 0, nStringLen = 0;
	DecodeUTF8String(GetDataContentBasePtr() + m_nCurentReadOffset + 1, lpBuffer, cchBufferLen, &nPrefixLen, &nStringLen);
	DWORD dwValue = *((DWORD*)(GetDataContentBasePtr() + m_nCurentReadOffset + 1 + nPrefixLen + nStringLen));

	SkipNextField();

	return dwValue;
}

void CSTXProtocol::IncreaseDWORDAtOffset( LONG nOffset )
{
	(*((DWORD*)(GetDataContentBasePtr() + nOffset)))++;
}


