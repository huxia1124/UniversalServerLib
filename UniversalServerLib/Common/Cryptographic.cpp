#include "StdAfx.h"
#include ".\cryptographic.h"

//////////////////////////////////////////////////////////////////////////
// CCryptographic

CCryptographic::CCryptographic(void)
{
	m_hCryptProv = NULL;
}

CCryptographic::~CCryptographic(void)
{
	if(m_hCryptProv)
		CryptReleaseContext(m_hCryptProv, 0);
}

BOOL CCryptographic::Init(LPCTSTR lpszContainer)
{
	if(!CryptAcquireContext(
		&m_hCryptProv, 
		lpszContainer,
		MS_ENHANCED_PROV,
		PROV_RSA_FULL, 
		0))
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if(!CryptAcquireContext(
				&m_hCryptProv, 
				lpszContainer,
				MS_ENHANCED_PROV,
				PROV_RSA_FULL, 
				CRYPT_NEWKEYSET)) 
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL CCryptographic::GetProviderName(LPTSTR lpszNameBuf, DWORD cchBufLen)
{
	DWORD cbDataLen = cchBufLen * sizeof(TCHAR);
	if(CryptGetProvParam(
		m_hCryptProv,
		PP_NAME,
		(BYTE*)lpszNameBuf,
		&cbDataLen,
		0))
	{
		return TRUE;
	}

	return FALSE;
}

LONG CCryptographic::ConvertToHexString(LPBYTE lpData, DWORD cbDataLen, LPTSTR lpszHexBuf, BOOL bUpperCased)
{
	static const TCHAR szHexDictUpper[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	static const TCHAR szHexDictLower[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	DWORD i = 0;
	if(bUpperCased)
	{
		for(i=0;i<cbDataLen;i++)
		{
			lpszHexBuf[i * 2] = szHexDictUpper[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictUpper[lpData[i] & 0x0F];
		}
	}
	else
	{
		for(i=0;i<cbDataLen;i++)
		{
			lpszHexBuf[i * 2] = szHexDictLower[(lpData[i] >> 4) & 0x0F];
			lpszHexBuf[i * 2 + 1] = szHexDictLower[lpData[i] & 0x0F];
		}
	}
	lpszHexBuf[i * 2] = 0;
	return cbDataLen * 2;
}

LPVOID CCryptographic::GetDataFromHexString( LPCTSTR lpszHexValue, DWORD* pdwLen )
{
	if(_tcsnicmp(lpszHexValue, _T("0x"), 2) != 0)
		return NULL;

	int Len = (int)(_tcslen(lpszHexValue) - 2) / 2;
	char* pBuf = new char[Len];

	int nPos = 0;
	lpszHexValue += 2;
	while(*lpszHexValue && *(lpszHexValue + 1))
	{
		BYTE bVal = 0;
		BYTE nHI = 0, nLO;
		if(*lpszHexValue >= _T('0') && *lpszHexValue <= _T('9'))
			nHI = *lpszHexValue - _T('0');
		else if((*lpszHexValue >= _T('a') && *lpszHexValue <= _T('f')))
			nHI = *lpszHexValue - _T('a') + 10;
		else if((*lpszHexValue >= _T('A') && *lpszHexValue <= _T('F')))
			nHI = *lpszHexValue - _T('A') + 10;

		lpszHexValue++;
		if(*lpszHexValue >= _T('0') && *lpszHexValue <= _T('9'))
			nLO = *lpszHexValue - _T('0');
		else if((*lpszHexValue >= _T('a') && *lpszHexValue <= _T('f')))
			nLO = *lpszHexValue - _T('a') + 10;
		else if((*lpszHexValue >= _T('A') && *lpszHexValue <= _T('F')))
			nLO = *lpszHexValue - _T('A') + 10;

		bVal = nHI * 16 + nLO;

		lpszHexValue++;

		pBuf[nPos++] = bVal;
	}

	*pdwLen = Len;
	return pBuf;
}

//////////////////////////////////////////////////////////////////////////
// CCryptBase

CCryptBase::CCryptBase()
{
	m_hKey = NULL;
}

CCryptBase::~CCryptBase()
{
	if(m_hKey)
		CryptDestroyKey(m_hKey);
}

BOOL CCryptBase::ImportKey(LPBYTE lpKeyBlob, DWORD dwDataLen)
{
	return CryptImportKey(m_hCryptProv, lpKeyBlob, dwDataLen, NULL, CRYPT_EXPORTABLE, &m_hKey);
}

BOOL CCryptBase::ExportKey(LPBYTE lpKeyBlob, DWORD *pdwDataLen)
{
	return CryptExportKey(m_hKey, NULL, PLAINTEXTKEYBLOB, 0, lpKeyBlob, pdwDataLen);
}


BOOL CCryptBase::Encrypt(LPBYTE lpDataBuf, DWORD *pcbBufLen, BOOL bFinal, DWORD dwBlockLen)
{
	if(!CryptEncrypt(
		m_hKey,
		0,           //no hashing
		bFinal,
		0,
		lpDataBuf,
		pcbBufLen,
		dwBlockLen))
	{ 
		return FALSE;
	} 
	return TRUE;
}

BOOL CCryptBase::Decrypt(LPBYTE lpDataBuf, DWORD *pcbBufLen, BOOL bFinal)
{
	if(!CryptDecrypt(
		m_hKey,
		0,           //no hashing
		bFinal,
		0,
		lpDataBuf,
		pcbBufLen))
	{ 
		return FALSE;

	} 
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CRSACrypt

CRSACrypt::CRSACrypt()
{

}

CRSACrypt::~CRSACrypt()
{

}

BOOL CRSACrypt::GenerateKey()
{
	if(CryptGenKey(
		m_hCryptProv,
		AT_KEYEXCHANGE,
		CRYPT_EXPORTABLE,
		&m_hKey)) 
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CRSACrypt::ExportPublicKey(LPBYTE lpBuf, DWORD *pcbBufSize)
{
	return ExportKeyBlob(lpBuf, pcbBufSize, TRUE);
}

BOOL CRSACrypt::ExportPrivateKey(LPBYTE lpBuf, DWORD *pcbBufSize)
{
	return ExportKeyBlob(lpBuf, pcbBufSize, FALSE);
}

BOOL CRSACrypt::ExportKeyBlob(LPBYTE lpBuf, DWORD *pcbBufSize, BOOL bPublicKey)
{
	DWORD dwBlobType = (bPublicKey?PUBLICKEYBLOB:PRIVATEKEYBLOB);

	if(CryptExportKey(   
		m_hKey, 
		NULL,    
		dwBlobType,
		0,    
		lpBuf,    
		pcbBufSize))
	{
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// CDESCrypt

CDESCrypt::CDESCrypt()
{

}

CDESCrypt::~CDESCrypt()
{

}

BOOL CDESCrypt::ImportStringKey(LPCTSTR lpszKeyString)
{
	HCRYPTHASH hHash = NULL;
	if(!CryptCreateHash(
		m_hCryptProv, 
		CALG_MD5, 
		0, 
		0, 
		&hHash))
	{
		return FALSE;
	}


	if(!CryptHashData(
		hHash, 
		(LPBYTE)lpszKeyString, 
		lstrlen(lpszKeyString), 
		0))
	{
		return FALSE;
	}

	if(m_hKey)
	{
		CryptDestroyKey(m_hKey);
		m_hKey = NULL;
	}
	if(!CryptDeriveKey(
		m_hCryptProv, 
		CALG_RC4, 
		hHash, 
		CRYPT_EXPORTABLE | 0x00800000, 
		&m_hKey))
	{
		return FALSE;
	}

	CryptDestroyHash(hHash);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CCryptoHash

CCryptoHash::CCryptoHash()
{
	m_hHash = NULL;
}

CCryptoHash::~CCryptoHash()
{
	if(m_hHash)
		CryptDestroyHash(m_hHash);
}

BOOL CCryptoHash::HashData(LPBYTE lpData, DWORD dwDataLen)
{
	return CryptHashData(m_hHash, lpData, dwDataLen, 0);
}

BOOL CCryptoHash::SignHash(LPBYTE lpBuf, DWORD *pdwBufLen)
{
	return CryptSignHash(m_hHash, AT_KEYEXCHANGE, NULL, 0, lpBuf, pdwBufLen);
}

BOOL CCryptoHash::GetHashValue(LPBYTE lpBuf, DWORD *pdwBufLen)
{
	return CryptGetHashParam(m_hHash, HP_HASHVAL, lpBuf, pdwBufLen, 0);
}


//////////////////////////////////////////////////////////////////////////
// CMD5Hash

CMD5Hash::CMD5Hash()
{

}

CMD5Hash::~CMD5Hash()
{

}

BOOL CMD5Hash::Create()
{
	if(m_hHash)
	{
		CryptDestroyHash(m_hHash);
		m_hHash = NULL;
	}

	return CryptCreateHash(
		m_hCryptProv, 
		CALG_MD5, 
		0, 
		0, 
		&m_hHash);
}

//////////////////////////////////////////////////////////////////////////
// CSHA1Hash

CSHA1Hash::CSHA1Hash()
{

}

CSHA1Hash::~CSHA1Hash()
{

}

BOOL CSHA1Hash::Create()
{
	if(m_hHash)
	{
		CryptDestroyHash(m_hHash);
		m_hHash = NULL;
	}

	return CryptCreateHash(
		m_hCryptProv, 
		CALG_SHA1, 
		0, 
		0, 
		&m_hHash);
}