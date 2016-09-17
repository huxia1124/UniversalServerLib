#pragma once


#include "STXSocket.h"

//////////////////////////////////////////////////////////////////////////

class CSTXProtocolDialog;

///////////////////////////////////////////////////////////////////////////////////

class CUniversalClientSocketConnectionContext : public CSTXSocketConnectionContext
{
	friend class CUniversalClientSocket;

protected:
	CSTXProtocolDialog *_dialog = nullptr;

protected:
	virtual DWORD IsDataReadable();
	virtual void OnReceived(LPVOID lpDataRecv, DWORD cbRecvLen);

};

///////////////////////////////////////////////////////////////////////////////////

class CUniversalClientSocket : public CSTXSocket
{
protected:
	CSTXProtocolDialog *_dialog = nullptr;

protected:
	virtual CSTXSocketConnectionContext* OnCreateConnectionContext(DWORD_PTR dwUserData);

public:
	void SetDialogPtr(CSTXProtocolDialog *pDlg);
};