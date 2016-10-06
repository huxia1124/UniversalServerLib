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

#include "StdAfx.h"
#include "UniversalClientSocket.h"
#include "STXProtocol.h"
#include "STXProtocolDialog.h"

///////////////////////////////////////////////////////////////////////////////////

DWORD CUniversalClientSocketConnectionContext::IsDataReadable()
{
	LPVOID pBuffer = GetMessageBasePtr();
	DWORD dwMsgDataLen = GetBufferedMessageLength();

	if (dwMsgDataLen == 0)
		return 0;

	BYTE nLengthBytes = 0;
	size_t nLength = CSTXProtocol::DecodeCompactInteger(pBuffer, &nLengthBytes);

	if (nLength < 0)
		return 0;

	//if ((UINT)nLength > g_maxTcpPackageLength)
	//{
	//	DisconnectClient(pClientContext);
	//	return 0;
	//}

	if ((DWORD)(nLength + nLengthBytes) <= dwMsgDataLen)
		return nLengthBytes + nLength;

	return 0;
}

void CUniversalClientSocketConnectionContext::OnReceived(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	CString responseMsg;
	size_t nLenParsed = 0;
	CSTXProtocol p;
	if (_dialog->_encryptionEnabled)
	{
		p.DecodeWithDecrypt(lpDataRecv, &nLenParsed, 1332);
	}
	else
	{
		p.Decode(lpDataRecv, &nLenParsed);
	}
	p.EnumValues([&responseMsg](unsigned char, STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData)
	{
		CSTXProtocolString val(pVal);
		CSTXProtocolString valExtra(pValExtra);
		TCHAR szText[4096];
		if (pValExtra->nValueType != STXPROTOCOL_DATA_TYPE_INVALID)
		{
			_stprintf_s(szText, _T("  %16S :\t %s / %s"), CSTXProtocol::GetTypeString(pVal->nValueType), (const char16_t*)val, (const char16_t*)valExtra);
		}
		else
		{
			_stprintf_s(szText, _T("  %16S :\t %s"), CSTXProtocol::GetTypeString(pVal->nValueType), (const char16_t*)val);
		}

		OutputDebugString(szText);
		OutputDebugString(_T("\n"));

		responseMsg += szText;
		responseMsg += _T("\n");

	}, 0);

	_dialog->AppendResponseMsg(responseMsg);


	p.SeekReadToBegin();
	//CSTXProtocolString s;
	//p.GetNextString(&s);

}

///////////////////////////////////////////////////////////////////////////////////


CSTXSocketConnectionContext* CUniversalClientSocket::OnCreateConnectionContext(DWORD_PTR dwUserData)
{
	CUniversalClientSocketConnectionContext *pContext = new CUniversalClientSocketConnectionContext();

	pContext->_dialog = _dialog;
	return pContext;
}

void CUniversalClientSocket::SetDialogPtr(CSTXProtocolDialog *pDlg)
{
	_dialog = pDlg;
}

