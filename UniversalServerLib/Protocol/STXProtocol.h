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
#include <memory>

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
// STXPROTOCOLVALUE
// Only used in CSTXProtocol::EnumValues

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
			size_t cchUTF8StringLen;
			const char* pszUTF8String;
		};
		struct
		{
			size_t cchUnicodeStringLen;
			const char16_t* pszUnicodeString;
		};
		struct
		{
			size_t cbDataLen;
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
	static size_t ConvertString(const char* pszUTF8, size_t cchUTF8, char* pszBuffer, size_t cchBuffer);
	static size_t ConvertString(const char* pszUTF8, size_t cchUTF8, char16_t* pszBuffer, size_t cchBuffer);
	static size_t ConvertString(const char16_t* pszUTF8, size_t cchUTF8, char* pszBuffer, size_t cchBuffer);
	static size_t ConvertString(const char16_t* pszUTF8, size_t cchUTF8, char16_t* pszBuffer, size_t cchBuffer);


	//static int ConvertString(const char16_t* pszUnicode, int cchUnicode, LPTSTR pszBuffer, int cchBuffer);
	static size_t ConvertStringToUTF8(const char* pszSrc, char *pszBuffer, size_t cchBuffer);
	static size_t ConvertStringToUTF8(const char16_t* pszSrc, char* pszBuffer, size_t cchBuffer);

	static size_t ConvertStringToUnicode(const char* pszSrc, char16_t *pszBuffer, size_t cchBuffer);
	static size_t ConvertStringToUnicode(const char16_t* pszSrc, char16_t *pszBuffer, size_t cchBuffer);

	static std::string ConvertGUIDToUTF8(GUID *guid);
	static size_t ConvertToHexString(unsigned char* lpData, size_t cbDataLen, char *lpszHexBuf, bool bUpperCased);
	static size_t ConvertToHexString(unsigned char* lpData, size_t cbDataLen, char16_t *lpszHexBuf, bool bUpperCased);

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
	size_t m_cchBufferSize;

protected:
	void ExpandTo(size_t cchMax);

public:
	operator const char16_t*();
	operator const char*();

	char16_t *GetBuffer(size_t nBufferLength = 0);		// nBufferLength = 0 : no expand
	size_t GetLength();		//in characters
};

//////////////////////////////////////////////////////////////////////////
// CSTXProtocol

class CSTXProtocol
{
public:
	CSTXProtocol(void);
	virtual ~CSTXProtocol(void);

protected:
	size_t m_nCurentWriteOffset;			//Position for writing new data.
	size_t m_nBufferLen;					//The capacity of the internal buffer
	char *m_pBuffer;						//Base address of the internal buffer
	size_t m_nCurentReadOffset;				//Position for reading data
	unsigned char m_nPrefixLen;				//Length, in bytes, of the length prefix

	//-Prefix-C++++data++++....
	//The value of the length prefix indicates the length of CRC and actual data (not including the length prefix)

protected:
	//Expand the buffer to make it capable for containing nAdd more bytes of data
	size_t Expand(size_t nAdd);

	//Get the content length in bytes of a specific field type. for variable fields, always return 0 
	size_t GetTypedDataLen(unsigned char nType);

	//Write a single byte of field type indicator. always return 1
	size_t WriteDataType(unsigned char nType);

	//Write a compact unsigned integer which represents the given value
	size_t WriteCompactInteger(size_t nValue);

	//Write the data directly to the internal buffer.
	//It will update the length prefix and CRC.
	size_t WriteRawData(void *pData, size_t cbDataLen);

	//Returns the address of the beginning of actual data. (right after length prefix and CRC)
	char* GetDataContentBasePtr();

	//Update the length prefix base on the current write position
	void UpdatePrefix();

	//Update the CRC by adding specific data
	void UpdateCRC(unsigned char *pAddedData, size_t cbDataLen);

	//Reset the read/write position, make everything fresh, as it was created without any data
	void ResetPosition();

	//Check if the field right after the current read position is of given type
	//if no more field available, return false.
	bool IsValidDataType(unsigned char nType);

	//void AssertBreak(LPCTSTR lpszError);

	//Just skip one byte if there is more data available.
	bool SkipTypeIndicator(unsigned char *pTypeSkipped = nullptr);

	//For fixed-length data field, check if there is enough data which can represent the data
	//If there is not enough data, an exception thrown
	void CheckDataAvailability(unsigned char nType);

	//Decode UTF8 string (array of char) into char array or char16_t array
	//These functions assume the input data starts with length prefix followed by actual data
	//The length prefix is always in bytes
	//the output buffer must be big enough to hold the decoded characters including the termination NULL character
	//  cchBufferLen specify the length, in characters, of the output buffer
	//  pOriginalStringPrefixLen receive the length in bytes of the length prefix
	//  pOriginalStringLen receive the length in bytes of the original input data (without length prefix)
	size_t DecodeUTF8String(void *pDataPtr, char *lpBuffer, size_t cchBufferLen, size_t *pOriginalStringPrefixLen, size_t *pOriginalStringLen);
	size_t DecodeUTF8String(void *pDataPtr, char16_t *lpBuffer, size_t cchBufferLen, size_t *pOriginalStringPrefixLen, size_t *pOriginalStringLen);


	//Decode Unicode string (array if char16_t) into char array or char16_t array
	//These functions assume the input data starts with length prefix followed by actual data
	//The length prefix is always in bytes
	//the output buffer must be big enough to hold the decoded characters including the termination NULL character
	//  cchBufferLen specify the length, in characters, of the output buffer
	//  pOriginalStringPrefixLen receive the length in bytes of the length prefix
	//  pOriginalStringLen receive the length in bytes of the original input data (without length prefix)
	size_t DecodeUnicodeString(void *pDataPtr, char *lpBuffer, size_t cchBufferLen, size_t *pOriginalStringPrefixLen, size_t *pOriginalStringLen);
	size_t DecodeUnicodeString(void *pDataPtr, char16_t *lpBuffer, size_t cchBufferLen, size_t *pOriginalStringPrefixLen, size_t *pOriginalStringLen);
	
	//Initialize this object from given input data
	//This method is used for extracting an embedded CSTXProtocol object from another CSTXProtocol object
	// pData should start with a length prefix.
	bool DecodeEmbeddedObject(void *pData, size_t *pDataReadLen);

	//Encrypt and decrypt a single byte
	static char EncryptByte(char data, char key);
	static char DecryptByte(char data, char key);

public:
	//Calculate how many bytes needed to represent the given value in a compact integer
	static size_t GetCompactIntegerLen(size_t nValue);

	//Decode the compact integer from input bytes
	static size_t DecodeCompactInteger(void *pData, unsigned char *pLengthBytes);		//pLengthBytes : out, size in bytes of the length prefix

	//Increase/Decrease the value at specific offset.
	//the offset is obtained through the second parameter of AppendData(uint32_t, size_t*)
	//Sometimes, it is necessary to append the number of loop iterations before appending each item. In this case,
	//  the increment method is useful.
	// These increment/decrement methods is safe even after appending more data because they are based on the offset
	void IncreaseDWORDAtOffset(size_t nOffset, uint32_t number);
	void DecreaseDWORDAtOffset(size_t nOffset, uint32_t number);

	//Get the value of a DWORD value at specified offset.
	uint32_t GetDWORDAtOffset(size_t nOffset);

	//Get a reference of some type at specified offset.
	//The returned reference might become invalid after appending more data.
	//Remember to always call GetReferenceAtOffset again after adding some data. 
	template<typename T>
	T& GetReferenceAtOffset(size_t nOffset)
	{
		static T temp;
		if (nOffset >= m_nCurentWriteOffset)
		{
			throw std::runtime_error("GetReferenceAtOffset<T>(long) : offset is out of the range of valid data.");
			return temp;
		}
		if (nOffset + sizeof(T) > m_nCurentWriteOffset)
		{
			throw std::runtime_error("GetReferenceAtOffset<T>(long) : not enough data.");
			return temp;
		}

		return *((T*)(GetDataContentBasePtr() + nOffset));
	}

	//Append custom type of data
	//Data will be stored as raw data type
	//To retrieve the data, use the template version of GetNextData() method
	template<typename T>
	size_t AppendData(T &&val, size_t *pOffset = nullptr)
	{
		return AppendRawData(&val, sizeof(val), pOffset);
	}
	template<typename T>
	size_t AppendData(T &val, size_t *pOffset = nullptr)
	{
		return AppendRawData(&val, sizeof(val), pOffset);
	}

	//Methods to append common type of data.
	size_t AppendData(unsigned char val);
	size_t AppendData(uint16_t val);
	size_t AppendData(uint32_t val, size_t *pOffset = nullptr);
	size_t AppendData(int64_t val);
	size_t AppendData(char *lpszVal);		//Always convert and append as UTF8 string
	size_t AppendData(const char *lpszVal);		//Always convert and append as UTF8 string
	size_t AppendData(char16_t *lpszVal);		//Always convert and append as UTF8 string
	size_t AppendData(const char16_t *lpszVal);		//Always convert and append as UTF8 string
	size_t AppendData(float val);
	size_t AppendData(double val);
	size_t AppendData(GUID &val);
	size_t AppendData(CSTXProtocol *pVal);	//Object
	size_t AppendRawData(void *pData, size_t cbDataLen, size_t *pOffset = nullptr);
	size_t AppendUnicodeString(const char* lpszVal);								//Append as Unicode string
	size_t AppendUnicodeString(const char16_t* lpszVal);								//Append as Unicode string
	size_t AppendUnicodeStringPair(const char* lpszVal1, const char* lpszVal2);		//Append as Unicode string
	size_t AppendUnicodeStringPair(const char16_t* lpszVal1, const char16_t* lpszVal2);		//Append as Unicode string
	size_t AppendDataPair(const char *lpszVal1, const char *lpszVal2);				//Always convert and append as UTF8 pair
	size_t AppendDataPair(const char16_t *lpszVal1, const char16_t *lpszVal2);				//Always convert and append as UTF8 pair
	size_t AppendDataPair(const char *lpszVal, uint32_t dwVal);						//UTF8-uint32_t pair
	size_t AppendDataPair(const char16_t *lpszVal, uint32_t dwVal);						//UTF8-uint32_t pair

	// The length of the prefix and all content (prefix + CRC + content)
	// This method returns the length, in bytes, of all valid data, which is exactly the amount of actual data
	//  ready for transferring or storing.
	size_t GetDataLen();

	// The address of the length-prefix (This is the address of all valid data: prefix + CRC + content)
	// When the data size increases, the base pointer might change.
	void* GetBasePtr();

	// The address of the CRC byte.
	void* GetCRCBytePtr();

	//Get the encrypted data. Data is encrypted using the given key through the dwKey parameter
	//the size of encrypted data keeps the same as the original data.
	size_t GetEncryptedData(void *pBuffer, size_t cbBufferLen, uint32_t dwKey);

	//pData : address of the pure data (address of length-prefix)
	//pDataReadLen [out, opt] : data length parsed
	//return: 0 if success, non-zero otherwise
	int Decode(void *pData, size_t *pDataReadLen);
	int Decode(void *pData, size_t *pDataReadLen, size_t cbInputDataLen);
	int DecodeWithDecrypt(void *pData, size_t *pDataReadLen, uint32_t dwKey, size_t cbInputDataLen = 0);

	//Retrieve custom data of given type.
	//This method can only perform on RAW data field. It will not check the actual type of the data.
	template<typename T>
	T GetNextData()
	{
		T tmp;
		GetNextRawData(&tmp, sizeof(tmp));
		return tmp;
	}

	//These methods are used to retrieve data from this object.
	//After each call, the read position will be updated to point to the next field.
	unsigned char GetNextByte();
	uint16_t GetNextWORD();
	uint32_t GetNextDWORD();
	int64_t GetNextI64();
	float GetNextFloat();
	double GetNextDouble();
	GUID GetNextGUID();
	std::shared_ptr<CSTXProtocol> GetNextObject();
	size_t GetNextString(char *lpBuffer, size_t cchBufferLen);
	size_t GetNextString(char16_t *lpBuffer, size_t cchBufferLen);
	std::string GetNextString();
	bool GetNextString(CSTXProtocolString *pString);
	size_t GetNextUnicodeString(char *lpBuffer, size_t cchBufferLen);
	size_t GetNextUnicodeString(char16_t *lpBuffer, size_t cchBufferLen);
	bool GetNextUnicodeString(CSTXProtocolString *pString);
	std::u16string GetNextUnicodeString();
	size_t GetNextRawData(void *pBuffer, size_t cbBufferSize);
	uint32_t GetNextStringPair(char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2);
	uint32_t GetNextStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2);

	uint32_t GetNextUnicodeStringPair(char *lpBuffer1, int cchBufferLen1, char *lpBuffer2, int cchBufferLen2);
	uint32_t GetNextUnicodeStringPair(char16_t *lpBuffer1, int cchBufferLen1, char16_t *lpBuffer2, int cchBufferLen2);

	uint32_t GetNextStringToDWORDPair(char *lpBuffer, int cchBufferLen);
	uint32_t GetNextStringToDWORDPair(char16_t *lpBuffer, int cchBufferLen);

	//Iterate through all the fields starting at the current read position
	//After calling this method, the read position will point to the end of all fields
	int EnumValues(std::function<void (unsigned char originalType, STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData)> pfnEnum, void *pUserData);

	//Move read position to next data field
	void SkipNextField();

	//Is there more data can be read
	bool IsDataAvailable();

	//Get the length of next field, including everything in that field except the type indicator
	size_t GetNextFieldLength();

	//Returns the type of the next data field.
	// refer to STXPROTOCOL_DATA_TYPE_*** macros
	unsigned char GetNextFieldType();

	//Return the textual representation of a field type
	static const char *GetTypeString(unsigned char nType);

	//Reset the read position, make this object ready to be read from the beginning.
	void SeekReadToBegin();

	//Reset all the content and read/write pointers in this object. Will become empty.
	void Clear();
};
