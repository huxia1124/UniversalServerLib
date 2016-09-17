#pragma once

#ifndef _WIN32_WINNT		// 允许使用 Windows NT 4 或更高版本的特定功能。
#define _WIN32_WINNT 0x0500		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif


//#include <windows.h>
#include <wtypes.h>
#include <wincrypt.h>
#include <tchar.h>

//////////////////////////////////////////////////////////////////////////
// CCryptographic

class CCryptographic
{
public:
	CCryptographic(void);
	virtual ~CCryptographic(void);

protected:
	HCRYPTPROV m_hCryptProv;			//Provider

public:
	BOOL Init(LPCTSTR lpszContainer = NULL);
	BOOL GetProviderName(LPTSTR lpszNameBuf, DWORD cchBufLen);

public:
	static LONG ConvertToHexString(LPBYTE lpData, DWORD cbDataLen, LPTSTR lpszHexBuf, BOOL bUpperCased = FALSE);
	static LPVOID GetDataFromHexString(LPCTSTR lpszHexValue, DWORD* pdwLen);
};

//////////////////////////////////////////////////////////////////////////
// CCryptBase

class CCryptBase : public CCryptographic
{
public:
	CCryptBase();
	virtual ~CCryptBase();

protected:
	HCRYPTKEY m_hKey;

public:
	virtual BOOL ImportKey(LPBYTE lpKeyBlob, DWORD dwDataLen);
	virtual BOOL ExportKey(LPBYTE lpKeyBlob, DWORD *pdwDataLen);

public:
	BOOL Encrypt(LPBYTE lpDataBuf, DWORD *pcbBufLen, BOOL bFinal, DWORD dwBlockLen);
	BOOL Decrypt(LPBYTE lpDataBuf, DWORD *pcbBufLen, BOOL bFinal);


};

//////////////////////////////////////////////////////////////////////////
// CRSACrypt

class CRSACrypt : public CCryptBase
{
public:
	CRSACrypt(void);
	virtual ~CRSACrypt(void);

public:
	BOOL GenerateKey();
	BOOL ExportPublicKey(LPBYTE lpBuf, DWORD *pcbBufSize);
	BOOL ExportPrivateKey(LPBYTE lpBuf, DWORD *pcbBufSize);

private:
	BOOL ExportKeyBlob(LPBYTE lpBuf, DWORD *pcbBufSize, BOOL bPublicKey);

};

//////////////////////////////////////////////////////////////////////////
// CDESCrypt

class CDESCrypt : public CCryptBase
{
public:
	CDESCrypt();
	virtual ~CDESCrypt();

public:
	BOOL ImportStringKey(LPCTSTR lpszKeyString);

};

//////////////////////////////////////////////////////////////////////////
// CCryptoHash

class CCryptoHash : public CCryptographic
{
public:
	CCryptoHash();
	virtual ~CCryptoHash();

protected:
	HCRYPTHASH m_hHash;

public:
	virtual BOOL Create() = 0;
	virtual BOOL HashData(LPBYTE lpData, DWORD dwDataLen);
	virtual BOOL SignHash(LPBYTE lpBuf, DWORD *pdwBufLen);
	virtual BOOL GetHashValue(LPBYTE lpBuf, DWORD *pdwBufLen);
	
};

//////////////////////////////////////////////////////////////////////////
// CMD5Hash

class CMD5Hash : public CCryptoHash
{
public:
	CMD5Hash();
	virtual ~CMD5Hash();

public:
	virtual BOOL Create();

};


//////////////////////////////////////////////////////////////////////////
// CSHA1Hash

class CSHA1Hash : public CCryptoHash
{
public:
	CSHA1Hash();
	virtual ~CSHA1Hash();

public:
	virtual BOOL Create();

};