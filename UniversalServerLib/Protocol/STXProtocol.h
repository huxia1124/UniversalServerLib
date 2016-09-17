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

#include <WTypes.h>

//////////////////////////////////////////////////////////////////////////

#define STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX	0x80
#define STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX	0x40
#define STXPROTOCOL_DATA_TYPE_FLAG_PAIR				0x20
#define STXPROTOCOL_DATA_TYPE_INVALID				0
#define STXPROTOCOL_DATA_TYPE_BYTE					1
#define STXPROTOCOL_DATA_TYPE_WORD					2
#define STXPROTOCOL_DATA_TYPE_DWORD					3
#define STXPROTOCOL_DATA_TYPE_I64					4
#define STXPROTOCOL_DATA_TYPE_UTF8					(5 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
#define STXPROTOCOL_DATA_TYPE_UNICODE				(6 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
#define STXPROTOCOL_DATA_TYPE_GUID					7
#define STXPROTOCOL_DATA_TYPE_OBJECT				(8 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)		//Embedded CSTXProtocol object
#define STXPROTOCOL_DATA_TYPE_RAW					(9 | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)
#define STXPROTOCOL_DATA_TYPE_FLOAT					10
#define STXPROTOCOL_DATA_TYPE_DOUBLE				11
#define STXPROTOCOL_DATA_TYPE_UTF8_PAIR				(STXPROTOCOL_DATA_TYPE_UTF8 | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX)
#define STXPROTOCOL_DATA_TYPE_UNICODE_PAIR			(STXPROTOCOL_DATA_TYPE_UNICODE | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_SECOND_PREFIX)
#define STXPROTOCOL_DATA_TYPE_UTF8_DWORD_PAIR		(STXPROTOCOL_DATA_TYPE_DWORD | STXPROTOCOL_DATA_TYPE_FLAG_PAIR | STXPROTOCOL_DATA_TYPE_FLAG_LENGTH_PREFIX)

//////////////////////////////////////////////////////////////////////////

class CSTXProtocolString;
class CSTXProtocol;
class CSTXProtocolUtility;

//////////////////////////////////////////////////////////////////////////

struct STXPROTOCOLVALUE
{
	BYTE nValueType;
	union
	{
		BYTE byteVal;
		WORD wVal;
		DWORD dwVal;
		__int64 nVal64;
		GUID guidVal;
		FLOAT floatVal;
		DOUBLE doubleVal;
		struct
		{
			int cchUTF8StringLen;
			LPCSTR pszUTF8String;
		};
		struct
		{
			int cchUnicodeStringLen;
			LPCWSTR pszUnicodeString;
		};
		struct
		{
			int cbDataLen;
			void *pDataPtr;
		};
		CSTXProtocolString *pString;
		CSTXProtocol *pObject;
	};
};


typedef void (CALLBACK*STXProtocolEnumFunc)(STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData);

//////////////////////////////////////////////////////////////////////////
// CSTXProtocolUtility

class CSTXProtocolUtility
{
public:
	static int ConvertString(LPCSTR pszUTF8, int cchUTF8, LPTSTR pszBuffer, int cchBuffer);
	static int ConvertString(LPCWSTR pszUnicode, int cchUnicode, LPTSTR pszBuffer, int cchBuffer);
	static int ConvertStringToUTF8(LPCTSTR pszSrc, LPSTR pszBuffer, int cchBuffer);
	static int ConvertStringToUnicode(LPCTSTR pszSrc, LPWSTR pszBuffer, int cchBuffer);
};

//////////////////////////////////////////////////////////////////////////
// CSTXProtocolString

class CSTXProtocolString
{
public:
	CSTXProtocolString();
	CSTXProtocolString(STXPROTOCOLVALUE *pVal);
	~CSTXProtocolString();

protected:
	LPTSTR m_pBuffer;
	int m_cchBufferSize;

#ifdef UNICODE
	LPSTR m_pBufferA;
	int m_cchBufferSizeA;
#endif

protected:
	void ExpandTo(int cchMax);
	LONG ConvertToHexString(LPBYTE lpData, DWORD cbDataLen, LPTSTR lpszHexBuf, BOOL bUpperCased);

public:
	operator LPCTSTR();
#ifdef UNICODE
	operator LPCSTR();
#endif
	LPTSTR GetBuffer(int nBufferLength = 0);		// nBufferLength = 0 : no expand
	int GetLength();
};

//////////////////////////////////////////////////////////////////////////
// CSTXProtocol

class CSTXProtocol
{
public:
	CSTXProtocol(void);
	virtual ~CSTXProtocol(void);

protected:
	LONG m_nCurentWriteOffset;								//当前写入位置偏移量(字节)。此偏移量是相对于数据首地址。新追加的数据将从该位置开始写入
	LONG m_nBufferLen;										//当前数据缓冲区的总大小
	char *m_pBuffer;										//数据缓冲区首地址
	LONG m_nCurentReadOffset;								//当前读取位置偏移量(字节)。此偏移量是相对于数据首地址。新读取数据将从这里开始获取
	BYTE m_nPrefixLen;										//长度前缀占用多少字节

	//-Prefix-C++++data++++....
	//长度前缀的值是包含CRC在内的消息数据长度，不包括长度前缀本身的长度。

protected:
	LONG Expand(LONG nAdd);
	LONG GetTypedDataLen(BYTE nType);
	LONG WriteDataType(BYTE nType);
	LONG WriteCompactInteger(UINT nValue);
	LONG WriteRawData(void *pData, LONG cbDataLen);
	LONG GetCompactIntegerLen(UINT nValue);
	char* GetDataContentBasePtr();
	void UpdatePrefix();
	void UpdateCRC(BYTE *pAddedData, UINT cbDataLen);
	void ResetPosition();

	BOOL IsValidDataType(BYTE nType);
	void AssertBreak(LPCTSTR lpszError);
	BOOL SkipTypeIndicator(BYTE *pTypeSkipped = NULL);
	void CheckDataAvailability(BYTE nType);
	int DecodeUTF8String(void *pDataPtr, LPTSTR lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	int DecodeUnicodeString(void *pDataPtr, LPTSTR lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	BOOL DecodeEmbeddedObject(void *pData, LONG *pDataReadLen);

	static char EncryptByte(char data, char key);
	static char DecryptByte(char data, char key);

public:
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

	// the length of the prefix and all content (prefix + CRC + content)
	LONG GetDataLen();

	// The address of the length-prefix (This is the address of all valid data: prefix + CRC + content)
	void* GetBasePtr();

	// The address of the CRC byte.
	void* GetCRCBytePtr();

	LONG GetEncryptedData(void *pBuffer, LONG cbBufferLen, DWORD dwKey);

	//pData : address of the pure data (address of length-prefix)
	//pDataReadLen [out, opt] : data length parsed
	int Decode(void *pData, LONG *pDataReadLen);
	int Decode(void *pData, LONG *pDataReadLen, LONG cbInputDataLen);
	int DecodeWithDecrypt(void *pData, LONG *pDataReadLen, DWORD dwKey);

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
	BOOL GetNextUnicodeString(CSTXProtocolString *pString);
	int GetNextRawData(void *pBuffer, int cbBufferSize);
	DWORD GetNextStringPair(LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2);
	DWORD GetNextUnicodeStringPair(LPTSTR lpBuffer1, int cchBufferLen1, LPTSTR lpBuffer2, int cchBufferLen2);
	DWORD GetNextStringToDWORDPair(LPTSTR lpBuffer, int cchBufferLen);

	int EnumValues(STXProtocolEnumFunc pfnEnum, void *pUserData);

	template<typename T>
	int EnumValues(T pfnEnum, void *pUserData)
	{
		int nValEnumCount = 0;
		STXPROTOCOLVALUE val;
		STXPROTOCOLVALUE valExtra;
		valExtra.nValueType = STXPROTOCOL_DATA_TYPE_INVALID;
		while (IsDataAvailable())
		{
			val.nValueType = GetNextFieldType();
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
			default:
				//Unknown field type
				AssertBreak(_T("EnumValues() : Unknown field type"));
			}

			pfnEnum(&val, &valExtra, pUserData);

			if (val.nValueType == STXPROTOCOL_DATA_TYPE_OBJECT && val.pObject)
				delete val.pObject;

			nValEnumCount++;
		}
		return nValEnumCount;
	}

	void SkipNextField();
	BOOL IsDataAvailable();
	int GetNextFieldLength();
	BYTE GetNextFieldType();
	static LPCTSTR GetTypeString(BYTE nType);
	void SeekReadToBegin();
};
