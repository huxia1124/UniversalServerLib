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

#include <cstdint>
#include <string>
#include <functional>

#ifdef WIN32
#include <windows.h>
#else
typedef struct _GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;
#endif

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
	unsigned char nValueType;
	union
	{
		unsigned char byteVal;
		uint16_t wVal;
		uint32_t dwVal;
		int64_t nVal64;
		GUID guidVal;
		float floatVal;
		double doubleVal;
		struct
		{
			int cchUTF8StringLen;
			const char* pszUTF8String;
		};
		struct
		{
			int cchUnicodeStringLen;
			const char16_t* pszUnicodeString;
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


typedef void (*STXProtocolEnumFunc)(STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData);

//////////////////////////////////////////////////////////////////////////
// CSTXProtocolUtility

class CSTXProtocolUtility
{
public:
	//UTF8 to char/wchar
	static int ConvertString(const char* pszUTF8, int cchUTF8, char* pszBuffer, int cchBuffer);
	static int ConvertString(const char* pszUTF8, int cchUTF8, char16_t* pszBuffer, int cchBuffer);
	static int ConvertString(const char16_t* pszUTF8, int cchUTF8, char* pszBuffer, int cchBuffer);
	static int ConvertString(const char16_t* pszUTF8, int cchUTF8, char16_t* pszBuffer, int cchBuffer);


	//static int ConvertString(const char16_t* pszUnicode, int cchUnicode, LPTSTR pszBuffer, int cchBuffer);
	static int ConvertStringToUTF8(const char* pszSrc, char *pszBuffer, int cchBuffer);
	static int ConvertStringToUTF8(const char16_t* pszSrc, char* pszBuffer, int cchBuffer);

	static int ConvertStringToUnicode(const char* pszSrc, char16_t *pszBuffer, int cchBuffer);
	static int ConvertStringToUnicode(const char16_t* pszSrc, char16_t *pszBuffer, int cchBuffer);

	static std::string ConvertGUIDToUTF8(GUID *guid);
	static long ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char *lpszHexBuf, bool bUpperCased);

	static std::string UTF16ToUTF8(std::u16string utf16_string);
	static std::string UTF16ToUTF8(const char16_t *begin, const char16_t *end);
	static std::u16string UTF8ToUTF16(std::string utf8_string);
	static std::u16string UTF8ToUTF16(const char *begin, const char *end);

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
	char16_t *m_pBuffer;
	std::string m_convertedUTF8;
	int m_cchBufferSize;

protected:
	void ExpandTo(int cchMax);
	long ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char *lpszHexBuf, bool bUpperCased);
	long ConvertToHexString(unsigned char* lpData, uint32_t cbDataLen, char16_t *lpszHexBuf, bool bUpperCased);

public:
	operator const char16_t*();
	operator const char*();

	char16_t *GetBuffer(int nBufferLength = 0);		// nBufferLength = 0 : no expand
	int GetLength();		//in characters
};

//////////////////////////////////////////////////////////////////////////
// CSTXProtocol

class CSTXProtocol
{
public:
	CSTXProtocol(void);
	virtual ~CSTXProtocol(void);

protected:
	long m_nCurentWriteOffset;								//当前写入位置偏移量(字节)。此偏移量是相对于数据首地址。新追加的数据将从该位置开始写入
	long m_nBufferLen;										//当前数据缓冲区的总大小
	char *m_pBuffer;										//数据缓冲区首地址
	long m_nCurentReadOffset;								//当前读取位置偏移量(字节)。此偏移量是相对于数据首地址。新读取数据将从这里开始获取
	unsigned char m_nPrefixLen;										//长度前缀占用多少字节

	//-Prefix-C++++data++++....
	//长度前缀的值是包含CRC在内的消息数据长度，不包括长度前缀本身的长度。

protected:
	long Expand(long nAdd);
	long GetTypedDataLen(unsigned char nType);
	long WriteDataType(unsigned char nType);
	long WriteCompactInteger(unsigned int nValue);
	long WriteRawData(void *pData, long cbDataLen);
	char* GetDataContentBasePtr();
	void UpdatePrefix();
	void UpdateCRC(unsigned char *pAddedData, unsigned int cbDataLen);
	void ResetPosition();

	bool IsValidDataType(unsigned char nType);
	//void AssertBreak(LPCTSTR lpszError);
	bool SkipTypeIndicator(unsigned char *pTypeSkipped = nullptr);
	void CheckDataAvailability(unsigned char nType);
	int DecodeUTF8String(void *pDataPtr, char *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	int DecodeUTF8String(void *pDataPtr, char16_t *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	int DecodeUnicodeString(void *pDataPtr, char *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	int DecodeUnicodeString(void *pDataPtr, char16_t *lpBuffer, int cchBufferLen, int *pOriginalStringPrefixLen, int *pOriginalStringLen);
	bool DecodeEmbeddedObject(void *pData, long *pDataReadLen);

	static char EncryptByte(char data, char key);
	static char DecryptByte(char data, char key);

public:
	static long GetCompactIntegerLen(unsigned int nValue);
	static long DecodeCompactInteger(void *pData, unsigned char *pLengthBytes);		//pLengthBytes : out, size in bytes of the length prefix 
	void IncreaseDWORDAtOffset(long nOffset);

	long AppendData(unsigned char val);
	long AppendData(uint16_t val);
	long AppendData(uint32_t val, long *pOffset = nullptr);
	long AppendData(int64_t val);
	long AppendData(char *lpszVal);		//Always convert and append as UTF8 string
	long AppendData(const char *lpszVal);		//Always convert and append as UTF8 string
	long AppendData(char16_t *lpszVal);		//Always convert and append as UTF8 string
	long AppendData(const char16_t *lpszVal);		//Always convert and append as UTF8 string
	long AppendData(float val);
	long AppendData(double val);
	long AppendData(GUID &val);
	long AppendData(CSTXProtocol *pVal);	//Object
	long AppendRawData(void *pData, long cbDataLen);
	long AppendUnicodeString(const char* lpszVal);								//Append as Unicode string
	long AppendUnicodeString(const char16_t* lpszVal);								//Append as Unicode string
	long AppendUnicodeStringPair(const char* lpszVal1, const char* lpszVal2);		//Append as Unicode string
	long AppendUnicodeStringPair(const char16_t* lpszVal1, const char16_t* lpszVal2);		//Append as Unicode string
	long AppendDataPair(const char *lpszVal1, const char *lpszVal2);				//Always convert and append as UTF8 pair
	long AppendDataPair(const char16_t *lpszVal1, const char16_t *lpszVal2);				//Always convert and append as UTF8 pair
	long AppendDataPair(const char *lpszVal, uint32_t dwVal);						//UTF8-uint32_t pair
	long AppendDataPair(const char16_t *lpszVal, uint32_t dwVal);						//UTF8-uint32_t pair

	// the length of the prefix and all content (prefix + CRC + content)
	long GetDataLen();

	// The address of the length-prefix (This is the address of all valid data: prefix + CRC + content)
	// When the data size increases, the base pointer might change.
	void* GetBasePtr();

	// The address of the CRC byte.
	void* GetCRCBytePtr();

	long GetEncryptedData(void *pBuffer, long cbBufferLen, uint32_t dwKey);

	//pData : address of the pure data (address of length-prefix)
	//pDataReadLen [out, opt] : data length parsed
	//return: 0 if success, non-zero otherwise
	int Decode(void *pData, long *pDataReadLen);
	int Decode(void *pData, long *pDataReadLen, long cbInputDataLen);
	int DecodeWithDecrypt(void *pData, long *pDataReadLen, uint32_t dwKey);

	unsigned char GetNextByte();
	uint16_t GetNextWORD();
	uint32_t GetNextDWORD();
	int64_t GetNextI64();
	float GetNextFloat();
	double GetNextDouble();
	GUID GetNextGUID();
	CSTXProtocol* GetNextObject();
	int GetNextString(char *lpBuffer, int cchBufferLen);
	int GetNextString(char16_t *lpBuffer, int cchBufferLen);
	std::string GetNextString();
	bool GetNextString(CSTXProtocolString *pString);
	int GetNextUnicodeString(char *lpBuffer, int cchBufferLen);
	int GetNextUnicodeString(char16_t *lpBuffer, int cchBufferLen);
	bool GetNextUnicodeString(CSTXProtocolString *pString);
	std::u16string GetNextUnicodeString();
	int GetNextRawData(void *pBuffer, int cbBufferSize);
	uint32_t GetNextStringPair(char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2);
	uint32_t GetNextStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2);

	uint32_t GetNextUnicodeStringPair(char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2);
	uint32_t GetNextUnicodeStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2);

	uint32_t GetNextStringToDWORDPair(char *lpBuffer, int cchBufferLen);
	uint32_t GetNextStringToDWORDPair(char16_t *lpBuffer, int cchBufferLen);

	int EnumValues(std::function<void (unsigned char originalType, STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData)> pfnEnum, void *pUserData);

	void SkipNextField();
	bool IsDataAvailable();
	int GetNextFieldLength();
	unsigned char GetNextFieldType();
	static const char *GetTypeString(unsigned char nType);
	void SeekReadToBegin();
	void Clear();
};
