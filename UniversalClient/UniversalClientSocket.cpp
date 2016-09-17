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
	LONG nLength = CSTXProtocol::DecodeCompactInteger(pBuffer, &nLengthBytes);

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
	LONG nLenParsed = 0;
	CSTXProtocol p;
	if (_dialog->_encryptionEnabled)
	{
		p.DecodeWithDecrypt(lpDataRecv, &nLenParsed, 1332);
	}
	else
	{
		p.Decode(lpDataRecv, &nLenParsed);
	}
	p.EnumValues([&responseMsg](STXPROTOCOLVALUE *pVal, STXPROTOCOLVALUE *pValExtra, void *pUserData)
	{
		CSTXProtocolString val(pVal);
		CSTXProtocolString valExtra(pValExtra);
		TCHAR szText[512];
		if (pValExtra->nValueType != STXPROTOCOL_DATA_TYPE_INVALID)
		{
			_stprintf_s(szText, _T("  %16s :\t %s / %s"), CSTXProtocol::GetTypeString(pVal->nValueType), (LPCTSTR)val, (LPCTSTR)valExtra);
		}
		else
		{
			_stprintf_s(szText, _T("  %16s :\t %s"), CSTXProtocol::GetTypeString(pVal->nValueType), (LPCTSTR)val);
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

