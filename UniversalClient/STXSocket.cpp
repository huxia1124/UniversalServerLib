#include "StdAfx.h"
#include "STXSocket.h"

//////////////////////////////////////////////////////////////////////////
// CSTXSocketBuffer

#define STXIOCPBUFFER_FLAG_AUTOEXPAND		0x00010000		//当 Buffer 空间不够时，自动扩展空间


CSTXSocketBuffer::CSTXSocketBuffer()
{
	::InitializeCriticalSection(&m_cs);

	m_nLock = 0;
	m_dwBufferLength = 0;
	m_dwContentLength = 0;
	m_pBuffer = NULL;
	m_dwUserData = 0;

	m_dwFlags = STXIOCPBUFFER_FLAG_AUTOEXPAND;
}

CSTXSocketBuffer::~CSTXSocketBuffer()
{
	ClearBuffer();
	::DeleteCriticalSection(&m_cs);
}

BOOL CSTXSocketBuffer::ClearBuffer()
{
	if(m_pBuffer)
	{
		delete []m_pBuffer;
		m_pBuffer = NULL;
		m_dwBufferLength = 0;
		m_nLock = 0;
		m_dwContentLength = 0;

		return TRUE;
	}
	return FALSE;
}

BOOL CSTXSocketBuffer::ReallocateBuffer(DWORD dwBufferSize)
{
	char *pBuffer = new char[dwBufferSize];
	if(pBuffer == NULL)
		return FALSE;

	LockBuffer();

	if(m_pBuffer)
	{
		::CopyMemory(pBuffer, m_pBuffer, m_dwContentLength);
	}
	DWORD dwOldContentLen = m_dwContentLength;
	ClearBuffer();
	m_dwContentLength = dwOldContentLen;

	m_pBuffer = pBuffer;
	m_dwBufferLength = dwBufferSize;

	UnlockBuffer();

	return TRUE;
}

int CSTXSocketBuffer::LockBuffer()
{
	EnterCriticalSection(&m_cs);
	m_nLock++;
	return m_nLock;
}

int CSTXSocketBuffer::UnlockBuffer()
{
	m_nLock--;
	LeaveCriticalSection(&m_cs);
	return m_nLock;
}

DWORD CSTXSocketBuffer::WriteBuffer(LPVOID pData, DWORD cbDataLen)
{
	if(cbDataLen == 0)
		return 0;

	if(m_dwFlags & STXIOCPBUFFER_FLAG_AUTOEXPAND)
	{
		if(m_dwContentLength + cbDataLen > m_dwBufferLength)
		{
			ReallocateBuffer((m_dwBufferLength + cbDataLen) * 2);
		}
	}

	LockBuffer();
	DWORD dwCopyLen = min(cbDataLen, m_dwBufferLength - m_dwContentLength);
	::CopyMemory(m_pBuffer + m_dwContentLength, pData, dwCopyLen);
	m_dwContentLength += dwCopyLen;
	UnlockBuffer();

	return dwCopyLen;
}

DWORD CSTXSocketBuffer::ReadBuffer(LPVOID pOutBuffer, DWORD cbOutBufferLen)
{
	LockBuffer();
	DWORD dwCopyLen = min(cbOutBufferLen, m_dwBufferLength);
	::CopyMemory(pOutBuffer, m_pBuffer, dwCopyLen);
	UnlockBuffer();

	return dwCopyLen;
}

LPVOID CSTXSocketBuffer::GetBufferPtr()
{
	return m_pBuffer;
}

DWORD CSTXSocketBuffer::GetBufferLength()
{
	return m_dwBufferLength;
}

DWORD CSTXSocketBuffer::GetDataLength()
{
	return m_dwContentLength;
}

DWORD CSTXSocketBuffer::SetDataLength(DWORD cbBufferDataLen)
{
	m_dwContentLength = min(cbBufferDataLen, m_dwBufferLength);
	return m_dwContentLength;
}

void CSTXSocketBuffer::SetUserData( DWORD_PTR dwUserData )
{
	m_dwUserData = dwUserData;
}

DWORD_PTR CSTXSocketBuffer::GetUserData()
{
	return m_dwUserData;
}

DWORD CSTXSocketBuffer::CopyData(CSTXSocketBuffer *pSrcBuffer)
{
	ClearBuffer();
	return WriteBuffer(pSrcBuffer->GetBufferPtr(), pSrcBuffer->GetDataLength());
}

BOOL CSTXSocketBuffer::WriteToFile( LPCTSTR lpszFile )
{
	HANDLE hFile = CreateFile(lpszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwLength = m_dwContentLength;
	while(dwLength > 0)
	{
		DWORD nBytesToWrite = min(dwLength, 65536);
		DWORD dwBytesWritten = 0;
		WriteFile(hFile, m_pBuffer + (m_dwContentLength - dwLength), nBytesToWrite, &dwBytesWritten, NULL);
		dwLength -= dwBytesWritten;
	}

	CloseHandle(hFile);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CSTXServerObjectBase

CSTXSocketObjectBase::CSTXSocketObjectBase()
{
	m_nRef = 1;
	m_sock = INVALID_SOCKET;
	m_dwTag1 = 0;
	m_dwTag2 = 0;

	m_cbBufferSize = 8192;
	m_pRecvBuffer = new char[m_cbBufferSize];
	m_cbWriteOffset = 0;
	m_cbReadOffset = 0;

	m_nPeerPort = 0;
	m_szPeerIP[0] = 0;
	m_dwEventID = 0;

	m_hWndFileUploadNotify = NULL;
	m_uMsgFileUploadNotify = 0;
	m_szFileUploading[0] = 0;
	m_hFileUploading = INVALID_HANDLE_VALUE;
	m_dwUserData = 0;
	m_dwUploadUserData = 0;
	m_dwDownloadUserData = 0;
	m_dwcbUploadTransferred = 0;
	m_bDownloading = FALSE;
	m_dwcbDownloadTransferred = 0;
	m_bPassiveDownloadBeginCalled = FALSE;
	m_bUploading = FALSE;
	m_pMainSocket = NULL;
	m_pfnIsWaitMessage = DefIsWaitMessage;
	m_hEvtUpload = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_hEvtDownload = CreateEvent(NULL, TRUE, TRUE, NULL);
	InitializeCriticalSection(&m_csWaitMap);

	m_hWndIsDataReadableNotify = NULL;
	m_uMsgIsDataReadableNotify = 0;
}

CSTXSocketObjectBase::~CSTXSocketObjectBase()
{
	if(m_hEvtUpload)
	{
		CloseHandle(m_hEvtUpload);
		m_hEvtUpload = NULL;
	}
	if(m_hEvtDownload)
	{
		CloseHandle(m_hEvtDownload);
		m_hEvtDownload = NULL;
	}
	if(m_szFileUploading[0] != 0)
	{
		m_szFileUploading[0] = 0;
		CloseHandle(m_hFileUploading);
		m_hFileUploading = INVALID_HANDLE_VALUE;
	}
	if(m_mapWaitObjects.size())
	{
		map<pair<HANDLE, DWORD_PTR>, _CMessageRecv>::iterator it = m_mapWaitObjects.begin();
		for(;it!=m_mapWaitObjects.end();it++)
		{
			if(it->second.hWndNotify)
			{
				delete []it->second.pBuffer;
			}
		}
	}
	DeleteCriticalSection(&m_csWaitMap);
	delete []m_pRecvBuffer;
}

LONG CSTXSocketObjectBase::AddRef()
{
	return InterlockedIncrement(&m_nRef);
}

LONG CSTXSocketObjectBase::Release()
{
	LONG nRef = InterlockedDecrement(&m_nRef);
	if(nRef == 0)
	{
		delete this;
	}
	return nRef;
}

void CSTXSocketObjectBase::AppendRecvData(LPVOID lpData, DWORD cbDataLen)
{
	LONG cbOldBufferSize = m_cbBufferSize;
	while((LONG)cbDataLen + m_cbWriteOffset > m_cbBufferSize)
		m_cbBufferSize *= 2;

	if(m_cbBufferSize > cbOldBufferSize)
	{
		char *pBuffer = new char[m_cbBufferSize];
		memcpy(pBuffer, m_pRecvBuffer, m_cbWriteOffset);
		delete []m_pRecvBuffer;
		m_pRecvBuffer = pBuffer;
	}

	memcpy(m_pRecvBuffer + m_cbWriteOffset, lpData, cbDataLen);
	m_cbWriteOffset += cbDataLen;
}

void CSTXSocketObjectBase::SkipRecvBuffer(LONG cbSkip)
{
	memmove(m_pRecvBuffer, m_pRecvBuffer + cbSkip, m_cbWriteOffset - cbSkip);
	m_cbWriteOffset -= cbSkip;
	m_cbReadOffset -= cbSkip;
	m_cbReadOffset = max(m_cbReadOffset, 0);
}

LPVOID CSTXSocketObjectBase::GetMessageBasePtr()
{
	return m_pRecvBuffer;
}

DWORD CSTXSocketObjectBase::GetBufferedMessageLength()
{
	return m_cbWriteOffset;
}

void CSTXSocketObjectBase::OnPreSendReady()
{
	if(m_quePendingSendData.size() > 0)
	{
		pair<char*, int> val = m_quePendingSendData.front();
		int nWsaError = 0;
		int nSent = SendHelper(val.first, val.second, FALSE, &nWsaError);
		if(nSent == SOCKET_ERROR)
		{
			if(nWsaError == WSAEWOULDBLOCK)
				return;
		}
		m_quePendingSendData.pop();
	}

}

DWORD CSTXSocketObjectBase::IsDataReadable()
{
	if(m_hWndIsDataReadableNotify)
	{
		return (DWORD)::SendMessage(m_hWndIsDataReadableNotify, m_uMsgIsDataReadableNotify, (WPARAM)GetMessageBasePtr(), (LPARAM)GetBufferedMessageLength());
	}

	if(m_pMainSocket->IsInternalMessagesEnabled())
		return 0;

	return m_cbWriteOffset;
}

DWORD CSTXSocketObjectBase::IsInternalDataReadable()
{
	if(!m_pMainSocket->IsInternalMessagesEnabled())
		return 0;

	DWORD dwMsgDataLen = GetBufferedMessageLength();

	if(dwMsgDataLen < sizeof(WORD))
		return 0;

	WORD wHeaderSize = *((WORD*)GetMessageBasePtr());

	if(wHeaderSize != sizeof(STXIOCPSERVERMESSAGEHEADER))
		return 0;

	if(dwMsgDataLen < wHeaderSize)
		return 0;

	LPSTXIOCPSERVERMESSAGEHEADER pHeader = (LPSTXIOCPSERVERMESSAGEHEADER)GetMessageBasePtr();
	if(pHeader->dwMagic != 0xFFFEFDFC)
		return 0;

	DWORD dwMsgTotalSize = pHeader->dwContentSize + sizeof(STXIOCPSERVERMESSAGEHEADER);

	if(dwMsgDataLen >= dwMsgTotalSize)
		return dwMsgTotalSize;

	return 0;
}

void CSTXSocketObjectBase::OnReceived(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	OutputDebugString(_T("CSTXSocketClientContext::OnReceived\n"));
}

void CSTXSocketObjectBase::OnInternalReceived(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	LPSTXIOCPSERVERMESSAGEHEADER pHeader = (LPSTXIOCPSERVERMESSAGEHEADER)lpDataRecv;
	switch(pHeader->wOpType)
	{
	case STXIOCP_INTERNAL_OP_UPLOAD:
		OnInternalDataTransfer(lpDataRecv, cbRecvLen);
		break;
	case STXIOCP_INTERNAL_OP_DOWNLOAD:
		OnInternalDownload(lpDataRecv, cbRecvLen);
		break;
	case STXIOCP_INTERNAL_OP_DOWNLOAD_PASSIVE:
		OnInternalPassiveDownload(lpDataRecv, cbRecvLen);
		break;
	}

}
void CSTXSocketObjectBase::OnInternalDataTransfer(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	LPSTXIOCPSERVERUPLOADFILE pMsg = (LPSTXIOCPSERVERUPLOADFILE)lpDataRecv;
	pMsg->wOpCode &= 0x7FFF;
	if(pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_INITIALIZATION
		|| pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK)		//Init //Upload Block
	{
		if(pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK)
			m_dwcbUploadTransferred += pMsg->dwTransferSize;

		char *szBuffer = new char[16400];
		LPSTXIOCPSERVERUPLOADFILE pReqMsg = (LPSTXIOCPSERVERUPLOADFILE)szBuffer;
		pReqMsg->header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
		pReqMsg->header.wOpType = STXIOCP_INTERNAL_OP_UPLOAD;
		pReqMsg->header.dwMagic = 0xFFFEFDFC;

		DWORD dwQueriedDataSize = OnQueryUploadData(pReqMsg + 1, 16400 - sizeof(STXIOCPSERVERUPLOADFILE), m_dwcbUploadTransferred, m_dwUploadUserData);
		pReqMsg->dwCookie = pMsg->dwCookie;

		if(dwQueriedDataSize == 0)
		{
			dwQueriedDataSize = OnQueryUploadFinalPaddingData(pReqMsg + 1, 16400 - sizeof(STXIOCPSERVERUPLOADFILE), m_dwUploadUserData);
			pReqMsg->wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_FINISH;
		}
		else
			pReqMsg->wOpCode = STXIOCP_INTERNAL_OPCODE_UPLOAD_BLOCK;

		pReqMsg->header.dwContentSize = sizeof(STXIOCPSERVERUPLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER) + dwQueriedDataSize;
		pReqMsg->dwTransferSize = dwQueriedDataSize;
		pReqMsg->dwContentSize = dwQueriedDataSize;
		pReqMsg->dwUserData = pMsg->dwUserData;

		Send(pReqMsg, pReqMsg->header.dwContentSize + sizeof(STXIOCPSERVERMESSAGEHEADER));

		delete[]szBuffer;
	}
// 	else if(pMsg->wOpCode == 1)		//Upload Block
// 	{
// 		LPVOID pData = (pMsg + 1);
// 		pClientContext->ProcessUpload(pMsg->dwCookie, pData, pMsg->dwContentSize);
// 
// 		STXIOCPSERVERUPLOADFILE ack;
// 		ack.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
// 		ack.header.wOpType = STXIOCP_INTERNAL_OP_UPLOAD;
// 		ack.dwContentSize = sizeof(STXIOCPSERVERUPLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER);
// 
// 		ack.dwCookie = pMsg->dwCookie;
// 		ack.wOpCode = 1 | 0x8000;
// 		ack.dwContentSize = 0;
// 
// 		pClientContext->Send(&ack, sizeof(ack));
// 	}
	else if(pMsg->wOpCode == 2)		//Finish
	{
		OnUploadComplete(m_dwUploadUserData);
		m_bUploading = FALSE;
		SetEvent(m_hEvtUpload);
		m_dwUploadUserData = 0;
	}
}

void CSTXSocketObjectBase::OnInternalPassiveDownload(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	LPSTXIOCPSERVERPASSIVEDOWNLOADFILE pMsg = (LPSTXIOCPSERVERPASSIVEDOWNLOADFILE)lpDataRecv;
	pMsg->wOpCode &= 0x7FFF;
	if(pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_INITIALIZATION)		//Init
	{
		if(!m_bPassiveDownloadBeginCalled)
		{
			OnPassiveDownloadBegin(pMsg->dwCookie, pMsg->dwUserData, pMsg->wResult);
			m_bPassiveDownloadBeginCalled = TRUE;
		}
		
// 		if(pMsg->dwTransferSize == 0)
// 		{
// 			OnDownloadComplete(m_dwDownloadUserData);
// 			SetEvent(m_hEvtDownload);
// 			m_bDownloading = FALSE;
// 			m_dwDownloadUserData = 0;
// 			return;
// 		}
	}
	else if(pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_DOWNLOAD_BLOCK)		//Data
	{
		if(!m_bPassiveDownloadBeginCalled)
		{
			OnPassiveDownloadBegin(pMsg->dwCookie, pMsg->dwUserData, 0);
			m_bPassiveDownloadBeginCalled = TRUE;
		}

		LPVOID lpData = (pMsg + 1);
		OnPassiveDownloadBlock(pMsg->dwCookie, pMsg->dwUserData, pMsg->dwOffset, lpData, pMsg->dwContentSize);
	}
	else if(pMsg->wOpCode == STXIOCP_INTERNAL_OPCODE_DOWNLOAD_FINISH)		//Finish
	{
		OnPassiveDownloadComplete(pMsg->dwCookie, pMsg->dwUserData, pMsg->wResult);
	}
}

void CSTXSocketObjectBase::OnPassiveDownloadBegin(DWORD dwCookie, DWORD dwUserData, WORD wServerResult)
{
	if(wServerResult != 0)
	{
		SetEvent(m_hEvtDownload);
		m_bDownloading = FALSE;
	}


}

void CSTXSocketObjectBase::OnPassiveDownloadBlock(DWORD dwCookie, DWORD dwUserData, DWORD dwOffset, LPVOID lpDataDownload, DWORD cbDownloadLen)
{
	m_FileDownloadBuffer.WriteBuffer(lpDataDownload, cbDownloadLen);
}

void CSTXSocketObjectBase::OnPassiveDownloadComplete(DWORD dwCookie, DWORD dwUserData, WORD wServerResult)
{
	SetEvent(m_hEvtDownload);
	m_bDownloading = FALSE;
	m_bPassiveDownloadBeginCalled = FALSE;
}

void CSTXSocketObjectBase::OnInternalDownload(LPVOID lpDataRecv, DWORD cbRecvLen)
{
	LPSTXIOCPSERVERDOWNLOADFILE pMsg = (LPSTXIOCPSERVERDOWNLOADFILE)lpDataRecv;
	pMsg->wOpCode &= 0x7FFF;
	if(pMsg->wOpCode == 0)		//Init
	{
		if(pMsg->dwTransferSize == 0)
		{
			OnDownloadComplete(m_dwDownloadUserData);
			SetEvent(m_hEvtDownload);
			m_bDownloading = FALSE;
			m_dwDownloadUserData = 0;
			return;
		}

		STXIOCPSERVERDOWNLOADFILE req;
		req.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
		req.header.wOpType = STXIOCP_INTERNAL_OP_DOWNLOAD;
		req.header.dwContentSize = sizeof(STXIOCPSERVERDOWNLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER);
		req.header.dwMagic = 0xFFFEFDFC;

		req.dwUserData = pMsg->dwUserData;
		req.dwCookie = pMsg->dwCookie;
		req.wOpCode = 1 | 0x8000;
		req.dwContentSize = 0;
		req.dwTransferSize = 0;
		req.dwOffset = 0;
		
		Send(&req, sizeof(req));
	}
	else if(pMsg->wOpCode == 1)		//Download Block
	{
		if(pMsg->dwTransferSize == 0)
		{
			OnDownloadComplete(m_dwDownloadUserData);
			SetEvent(m_hEvtDownload);
			m_bDownloading = FALSE;
			m_dwDownloadUserData = 0;
			return;
		}

		STXIOCPSERVERDOWNLOADFILE req;
		req.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
		req.header.wOpType = STXIOCP_INTERNAL_OP_DOWNLOAD;
		req.header.dwContentSize = sizeof(STXIOCPSERVERDOWNLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER);
		req.header.dwMagic = 0xFFFEFDFC;

		OnDownloadBlock(m_dwcbDownloadTransferred, pMsg + 1, pMsg->dwTransferSize);

		m_dwcbDownloadTransferred += pMsg->dwTransferSize;

		req.dwUserData = pMsg->dwUserData;
		req.dwCookie = pMsg->dwCookie;
		req.wOpCode = 1 | 0x8000;
		req.dwContentSize = 0;
		req.dwTransferSize = 0;
		req.dwOffset = m_dwcbDownloadTransferred;

		Send(&req, sizeof(req));
	}
// 	else if(pMsg->wOpCode == 2)		//Finish
// 	{
// 		OnUploadComplete(m_dwUploadUserData);
// 		m_bUploading = FALSE;
// 		m_dwUploadUserData = 0;
// 	}
}
void CSTXSocketObjectBase::OnPreReceived( LPVOID lpDataRecv, DWORD cbRecvLen )
{
	if(m_mapWaitObjects.size() > 0)
	{
		queue <pair<HANDLE, DWORD_PTR> > queMatched;
		queue <_CMessageRecv> queMatchedObject;
		EnterCriticalSection(&m_csWaitMap);
		map<pair<HANDLE, DWORD_PTR>, _CMessageRecv>::iterator it = m_mapWaitObjects.begin();
		for(;it!=m_mapWaitObjects.end();it++)
		{
			if(m_pfnIsWaitMessage(it->first.second, lpDataRecv, cbRecvLen))
			{
				queMatched.push(it->first);
				it->second.cbBufferLength = min(it->second.cbBufferLength, cbRecvLen);
				memcpy(it->second.pBuffer, lpDataRecv, it->second.cbBufferLength);

				queMatchedObject.push(it->second);
			}
		}
		LeaveCriticalSection(&m_csWaitMap);

		while(queMatched.size() > 0)
		{
			pair<HANDLE, DWORD_PTR> val = queMatched.front();
			_CMessageRecv Recv = queMatchedObject.front();
			if(val.first != NULL)
			{
				SetEvent(val.first);
			}
			if(Recv.hWndNotify != NULL)
			{
				m_pMainSocket->KillInternalTimer(Recv.nTimerID);
				SendMessage(Recv.hWndNotify, Recv.uMsgNotify, (WPARAM)Recv.pBuffer, (LPARAM)cbRecvLen);

				delete []Recv.pBuffer;
				EnterCriticalSection(&m_csWaitMap);
				m_mapWaitObjects.erase(val);
				LeaveCriticalSection(&m_csWaitMap);
			}
			queMatched.pop();
			queMatchedObject.pop();
		}
	}
}

void CSTXSocketObjectBase::OnSendReady()
{
	OutputDebugString(_T("CSTXSocketClientContext::OnSendReady\n"));
}

LONG CSTXSocketObjectBase::OnQueryUploadData(LPVOID lpDataBuf, DWORD cbBufferLen, DWORD cbTransferred, DWORD_PTR dwUserData)
{
	if(m_szFileUploading[0] != 0)
	{
		DWORD dwBytesRead = 0;
		if (ReadFile(m_hFileUploading, lpDataBuf, cbBufferLen, &dwBytesRead, NULL))
		{
			return dwBytesRead;
		}
	}
	return 0;
}

LONG CSTXSocketObjectBase::OnQueryUploadFinalPaddingData(LPVOID lpDataBuf, DWORD cbBufferLen, DWORD_PTR dwUserData)
{
	if(m_szFileUploading[0] != 0)
	{
		m_szFileUploading[0] = 0;
		CloseHandle(m_hFileUploading);
		m_hFileUploading = INVALID_HANDLE_VALUE;
		if(m_hWndFileUploadNotify)
		{
			SendMessage(m_hWndFileUploadNotify, m_uMsgFileUploadNotify, (WPARAM)dwUserData, 0);
			m_hWndFileUploadNotify = NULL;
			m_uMsgFileUploadNotify = 0;
		}
		return 0;
	}
	return 0;
}

void CSTXSocketObjectBase::OnUploadComplete(DWORD_PTR dwUserData)
{

}

void CSTXSocketObjectBase::OnDownloadBlock(DWORD dwDownloadTransferred, LPVOID lpDataDownload, DWORD cbDownloadLen)
{

}

void CSTXSocketObjectBase::OnDownloadComplete(DWORD_PTR dwUserData)
{

}

BOOL CSTXSocketObjectBase::BeginUpload(DWORD_PTR dwUserData, DWORD dwUserDataRemote, DWORD dwCookieDesire, LPVOID lpPaddingData, DWORD cbPaddingDataLen)
{
	if(m_bUploading)
		return FALSE;

	ResetEvent(m_hEvtUpload);

	char pszBuffer[8192];
	LPSTXIOCPSERVERUPLOADFILE pReq = (LPSTXIOCPSERVERUPLOADFILE)pszBuffer;

	STXIOCPSERVERUPLOADFILE &req = *pReq;
	req.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
	req.header.wOpType = STXIOCP_INTERNAL_OP_UPLOAD;
	req.header.dwMagic = 0xFFFEFDFC;

	req.dwContentSize = 0;
	if(lpPaddingData && cbPaddingDataLen)
	{
		DWORD dwPaddingLen = min(cbPaddingDataLen, sizeof(pszBuffer) - sizeof(req));
		memcpy(pReq + 1, lpPaddingData, dwPaddingLen);
		req.dwContentSize = dwPaddingLen;
	}
	req.header.dwContentSize = sizeof(STXIOCPSERVERUPLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER) + req.dwContentSize;

	req.dwUserData = dwUserDataRemote;
	req.dwCookie = dwCookieDesire;
	req.wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION;		//Init
	m_dwUploadUserData = dwUserData;
	m_dwcbUploadTransferred = 0;

	m_bUploading = (Send(pszBuffer, sizeof(req) + req.dwContentSize) == sizeof(req) + req.dwContentSize);
	return m_bUploading;
}

BOOL CSTXSocketObjectBase::BeginDownload(DWORD_PTR dwUserData, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpPaddingData, DWORD cbPaddingDataLen)
{
	if(m_bDownloading)
		return FALSE;

	ResetEvent(m_hEvtDownload);

	char pszBuffer[8192];

	LPSTXIOCPSERVERDOWNLOADFILE pReq = (LPSTXIOCPSERVERDOWNLOADFILE)pszBuffer;

	STXIOCPSERVERDOWNLOADFILE &req = *pReq;
	req.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
	req.header.wOpType = STXIOCP_INTERNAL_OP_DOWNLOAD;
	req.header.dwMagic = 0xFFFEFDFC;

	req.dwContentSize = 0;
	if(lpPaddingData && cbPaddingDataLen)
	{
		DWORD dwPaddingLen = min(cbPaddingDataLen, sizeof(pszBuffer) - sizeof(req));
		memcpy(pReq + 1, lpPaddingData, dwPaddingLen);
		req.dwContentSize = dwPaddingLen;
	}
	req.header.dwContentSize = sizeof(STXIOCPSERVERDOWNLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER) + req.dwContentSize;

	req.dwUserData = dwUserDataRemote;
	req.dwCookie = dwCookie;
	req.wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION;		//Init
	req.dwOffset = 0;

	m_dwDownloadUserData = dwUserData;
	m_dwcbDownloadTransferred = 0;

	m_bDownloading = (Send(pszBuffer, sizeof(req) + req.dwContentSize) == sizeof(req) + req.dwContentSize);
	return m_bDownloading;
}

BOOL CSTXSocketObjectBase::BeginPassiveDownload(DWORD_PTR dwUserData, DWORD dwCookie, DWORD dwUserDataRemote, LPVOID lpPaddingData, DWORD cbPaddingDataLen)
{
	if(m_bDownloading)
		return FALSE;

	m_FileDownloadBuffer.ClearBuffer();
	m_FileDownloadBuffer.ReallocateBuffer(1024 * 128);	//128 KB
	m_bPassiveDownloadBeginCalled = FALSE;
	ResetEvent(m_hEvtDownload);

	char pszBuffer[8192];

	LPSTXIOCPSERVERPASSIVEDOWNLOADFILE pReq = (LPSTXIOCPSERVERPASSIVEDOWNLOADFILE)pszBuffer;

	STXIOCPSERVERPASSIVEDOWNLOADFILE &req = *pReq;
	req.header.wHeaderSize = sizeof(STXIOCPSERVERMESSAGEHEADER);
	req.header.wOpType = STXIOCP_INTERNAL_OP_DOWNLOAD_PASSIVE;
	req.header.dwMagic = 0xFFFEFDFC;

	req.dwContentSize = 0;
	if(lpPaddingData && cbPaddingDataLen)
	{
		DWORD dwPaddingLen = min(cbPaddingDataLen, sizeof(pszBuffer) - sizeof(req));
		memcpy(pReq + 1, lpPaddingData, dwPaddingLen);
		req.dwContentSize = dwPaddingLen;
	}
	req.header.dwContentSize = sizeof(STXIOCPSERVERDOWNLOADFILE) - sizeof(STXIOCPSERVERMESSAGEHEADER) + req.dwContentSize;

	req.dwUserData = dwUserDataRemote;
	req.dwCookie = dwCookie;
	req.wOpCode = STXIOCP_INTERNAL_OPCODE_INITIALIZATION;		//Init
	req.dwOffset = 0;

	m_dwDownloadUserData = dwUserData;
	m_dwcbDownloadTransferred = 0;

	m_bDownloading = (Send(pszBuffer, sizeof(req) + req.dwContentSize) == sizeof(req) + req.dwContentSize);
	return m_bDownloading;
}

BOOL CSTXSocketObjectBase::WaitForUploadComplete(DWORD dwMilliseconds)
{
	if(WaitForSingleObject(m_hEvtUpload, dwMilliseconds) == WAIT_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL CSTXSocketObjectBase::WaitForDownloadComplete(DWORD dwMilliseconds)
{
	if(WaitForSingleObject(m_hEvtDownload, dwMilliseconds) == WAIT_TIMEOUT)
		return FALSE;

	return TRUE;
}

int CSTXSocketObjectBase::SendHelper(LPVOID lpData, int cbDataLen, BOOL bAutoCache, int *pWsaError)
{
	int nSent = send(m_sock, (char*)lpData, cbDataLen, 0);
	int nLastError = WSAGetLastError();
	if(pWsaError)
		*pWsaError = nLastError;

	if(nSent == SOCKET_ERROR && nLastError == WSAEWOULDBLOCK)
	{
		if(bAutoCache)
		{
			char *pszCacheBuffer = new char[cbDataLen];
			pair<char*, int> val;
			val.first = pszCacheBuffer;
			val.second = cbDataLen;

			m_quePendingSendData.push(val);
			return cbDataLen;
		}
	}
	return nSent;
}

int CSTXSocketObjectBase::Send(LPVOID lpData, int cbDataLen, BOOL bAutoCache)
{
	return SendHelper(lpData, cbDataLen, bAutoCache);
}

int CSTXSocketObjectBase::SendRecv(LPVOID lpSendData, int cbSendLen, LPVOID lpRecvBuf, int cbRecvBufLen, DWORD_PTR dwUserData, DWORD dwTimeOut)
{
	HANDLE hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	pair<HANDLE, DWORD_PTR> val;
	val.first = hWaitEvent;
	val.second = dwUserData;

	_CMessageRecv Recv;
	Recv.hWaitEvent = hWaitEvent;
	Recv.cbBufferLength = cbRecvBufLen;
	Recv.pBuffer = lpRecvBuf;
	Recv.hWndNotify = NULL;
	Recv.uMsgNotify = 0;

	EnterCriticalSection(&m_csWaitMap);
	m_mapWaitObjects[val] = Recv;
	LeaveCriticalSection(&m_csWaitMap);

	DWORD dwWaitResult = 0;
	if(SendHelper(lpSendData, cbSendLen, FALSE) != SOCKET_ERROR)
	{
		if(hWaitEvent)
			dwWaitResult = ::WaitForSingleObject(hWaitEvent, dwTimeOut);

		EnterCriticalSection(&m_csWaitMap);
		Recv = m_mapWaitObjects[val];
		m_mapWaitObjects.erase(val);
		LeaveCriticalSection(&m_csWaitMap);
	}

	if (hWaitEvent)
		CloseHandle(hWaitEvent);

	if( dwWaitResult == WAIT_TIMEOUT)
		return 0;

	return Recv.cbBufferLength;
}

BOOL CSTXSocketObjectBase::BeginSendRecv(LPVOID lpSendData, int cbSendLen, int cbRecvBufLenReserve, DWORD_PTR dwUserData, DWORD dwTimeOut, HWND hWndNotify, UINT uMsgNotify)
{
	pair<HANDLE, DWORD_PTR> val;
	val.first = NULL;
	val.second = dwUserData;

	_CMessageRecv Recv;
	Recv.hWaitEvent = NULL;
	Recv.cbBufferLength = cbRecvBufLenReserve;
	Recv.pBuffer = new char[cbRecvBufLenReserve];
	Recv.hWndNotify = hWndNotify;
	Recv.uMsgNotify = uMsgNotify;
	Recv.dwUserData = dwUserData;

	UINT_PTR nTimerID = m_pMainSocket->SetInternalTimer(dwTimeOut, this);
	Recv.nTimerID = nTimerID;

	m_mapTimerToWaitObject[nTimerID] = Recv;

	EnterCriticalSection(&m_csWaitMap);
	m_mapWaitObjects[val] = Recv;
	LeaveCriticalSection(&m_csWaitMap);

	if(SendHelper(lpSendData, cbSendLen, FALSE) != SOCKET_ERROR)
	{
		return TRUE;
	}
	else
	{
		delete []Recv.pBuffer;
		EnterCriticalSection(&m_csWaitMap);
		m_mapWaitObjects.erase(val);
		LeaveCriticalSection(&m_csWaitMap);
		return FALSE;
	}
}


LPCTSTR CSTXSocketObjectBase::GetPeerIP()
{
	return m_szPeerIP;
}

UINT CSTXSocketObjectBase::GetPeerPort()
{
	return m_nPeerPort;
}

void CSTXSocketObjectBase::Close()
{
	if(m_sock != INVALID_SOCKET)
	{
		m_pMainSocket->PostClose(this);
	}
}

DWORD_PTR CSTXSocketObjectBase::GetUserData()
{
	return m_dwUserData;
}

BOOL CALLBACK CSTXSocketObjectBase::DefIsWaitMessage( DWORD_PTR dwUserData, LPVOID lpRecvData, int cbRecvData )
{
	return TRUE;
}

void CSTXSocketObjectBase::ClearPendingMessages()
{
	while(m_quePendingSendData.size() > 0)
	{
		pair<char*, int> val = m_quePendingSendData.front();
		delete [](char*)(val.first);
		m_quePendingSendData.pop();
	}
}

int CSTXSocketObjectBase::Receive( LPVOID lpData, int cbDataLen )
{
	if(m_sock == INVALID_SOCKET)
		return 0;

	if(GetBufferedMessageLength() - (DWORD)m_cbReadOffset < (DWORD)cbDataLen)
		return 0;

	LPVOID lpBuffer = GetMessageBasePtr();

	if(lpData)
		memcpy(lpData, ((char*)lpBuffer) + m_cbReadOffset, cbDataLen);

	m_cbReadOffset += cbDataLen;

	return cbDataLen;
}

void CSTXSocketObjectBase::SetTag1( DWORD_PTR dwTag )
{
	m_dwTag1 = dwTag;
}

DWORD_PTR CSTXSocketObjectBase::GetTag1()
{
	return m_dwTag1;
}

void CSTXSocketObjectBase::SetTag2( DWORD_PTR dwTag )
{
	m_dwTag2 = dwTag;
}

DWORD_PTR CSTXSocketObjectBase::GetTag2()
{
	return m_dwTag2;
}

BOOL CSTXSocketObjectBase::BeginUploadFile( LPCTSTR lpszFile, DWORD_PTR dwUserData, DWORD dwUserDataRemote, DWORD dwCookieDesire, LPVOID lpPaddingData, DWORD cbPaddingDataLen, HWND hWndNotify, UINT uMsgNotify )
{
	HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	_tcscpy_s(m_szFileUploading, MAX_PATH, lpszFile);
	m_hFileUploading = hFile;
	m_hWndFileUploadNotify = hWndNotify;
	m_uMsgFileUploadNotify = uMsgNotify;

	return BeginUpload(dwUserData, dwUserDataRemote, dwCookieDesire, lpPaddingData, cbPaddingDataLen);
}

void CSTXSocketObjectBase::OnTimeout( UINT_PTR nTimerID )
{
	m_pMainSocket->KillInternalTimer(nTimerID);

	_CMessageRecv Recv;
	map<UINT_PTR, _CMessageRecv>::iterator it = m_mapTimerToWaitObject.find(nTimerID);
	if(it != m_mapTimerToWaitObject.end())
	{
		Recv = it->second;
		m_mapTimerToWaitObject.erase(nTimerID);
	}
	else
	{
		throw std::exception("Error");
	}

	SendMessage(Recv.hWndNotify, Recv.uMsgNotify, 0, 0);

	delete []Recv.pBuffer;

	pair<HANDLE, DWORD_PTR> val;
	val.first = NULL;
	val.second = Recv.dwUserData;

	EnterCriticalSection(&m_csWaitMap);
	m_mapWaitObjects.erase(val);
	LeaveCriticalSection(&m_csWaitMap);
}

void CSTXSocketObjectBase::SetIsDataReadableNotify( HWND hWndNotify, UINT uMsgNotify )
{
	m_hWndIsDataReadableNotify = hWndNotify;
	m_uMsgIsDataReadableNotify = uMsgNotify;
}

CSTXSocketBuffer * CSTXSocketObjectBase::GetFileDownloadBuffer()
{
	return &m_FileDownloadBuffer;
}

void CSTXSocketObjectBase::ClearReceiveBuffer()
{
	m_cbWriteOffset = 0;
	m_cbReadOffset = 0;
}

//////////////////////////////////////////////////////////////////////////
// CSTXSocketClientContext

CSTXSocketClientContext::CSTXSocketClientContext()
{
	m_dwEventID = WM_STXSOCKET_CLIENT_EVENT;
}

CSTXSocketClientContext::~CSTXSocketClientContext()
{
}

//////////////////////////////////////////////////////////////////////////
// CSTXSocketConnectionContext

CSTXSocketConnectionContext::CSTXSocketConnectionContext()
{
	m_hConnectSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bConnected = FALSE;
	m_dwEventID = WM_STXSOCKET_CONNECTION_EVENT;
	m_hWndConnectNotify = NULL;
	m_uMsgConnectNotify = 0;
	m_uMsgDisconnectNotify = 0;
}

CSTXSocketConnectionContext::~CSTXSocketConnectionContext()
{
	CloseHandle(m_hConnectSignal);
	//STXTRACE(_T("CSTXSocketConnectionContext destroyed."));
}

BOOL CSTXSocketConnectionContext::IsConnected()
{
	return m_bConnected;
}

//////////////////////////////////////////////////////////////////////////
// CSTXSocket

LPCTSTR CSTXSocket::s_lpszClassName = _T("_CSTXSocket_Window_Class_");

CSTXSocket::CSTXSocket(void)
{
	m_sockServer = INVALID_SOCKET;
	m_hThread = INVALID_HANDLE_VALUE;
	m_uThreadID = 0;
	m_uServerPort = 0;
	m_hCreateSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hCreateServerSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWndSocket = NULL;
	m_bCreateSucceed = FALSE;
	m_bEnableInternalMessages = FALSE;
	m_nInternalTimerIDBase = 1;

	InitializeCriticalSection(&m_csClientMap);
	InitializeCriticalSection(&m_csConnectionMap);
	InitializeCriticalSection(&m_csTimerMap);
}

CSTXSocket::~CSTXSocket(void)
{
	Close();
	DeleteCriticalSection(&m_csTimerMap);
	DeleteCriticalSection(&m_csClientMap);
	DeleteCriticalSection(&m_csConnectionMap);
	CloseHandle(m_hCreateSignal);
	CloseHandle(m_hCreateServerSignal);
}

BOOL CSTXSocket::Create(UINT uServerPort)
{
	m_uServerPort = uServerPort;
	m_bCreateSucceed = FALSE;

	ResetEvent(m_hCreateSignal);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, SocketThreadProc, this, 0, &m_uThreadID);
	WaitForSingleObject(m_hCreateSignal, INFINITE);

	return m_bCreateSucceed;
}

UINT WINAPI CSTXSocket::SocketThreadProc(LPVOID pData)
{
	CSTXSocket *pThis = (CSTXSocket*)pData;

	if(!pThis->CreateSocketWindow())
	{
		::SetEvent(pThis->m_hCreateSignal);
		return 0;
	}

	pThis->OnSocketThreadInitialize();

	pThis->m_bCreateSucceed = TRUE;
	::SetEvent(pThis->m_hCreateSignal);

	MSG msg;
	while(::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	pThis->OnSocketThreadUninitialize();

	return 0;
}

BOOL CSTXSocket::CreateServerSocket()
{
	//m_sockServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	m_sockServer = socket(AF_INET, SOCK_STREAM, 0);

	if(m_sockServer == INVALID_SOCKET)
		return FALSE;

	return TRUE;
}

void CSTXSocket::SockaddrFromHost(LPCTSTR lpszHostAddress, UINT uPort, SOCKADDR *pAddr, int nAddressFamily)
{
	if(nAddressFamily == AF_INET6)
	{
		int nLen = sizeof(SOCKADDR_IN6);
		int nResult = WSAStringToAddress((LPTSTR)lpszHostAddress, AF_INET6, NULL, (SOCKADDR*)pAddr, &nLen);
		((SOCKADDR_IN6*)pAddr)->sin6_port = htons(uPort);
		return;
	}

	SOCKADDR_IN *pAddr2 = (SOCKADDR_IN*)pAddr;

	memset(pAddr2,0,sizeof(SOCKADDR_IN));

#ifdef UNICODE
	int cbLen = (int)_tcslen(lpszHostAddress) + 2;
	LPSTR lpszAscii = (LPSTR)new char[cbLen];
	::WideCharToMultiByte(CP_ACP, 0, lpszHostAddress, -1, lpszAscii, cbLen, NULL, NULL);
#else
	LPSTR lpszAscii = (LPSTR)lpszHostAddress;
#endif

	pAddr2->sin_family = AF_INET;
	pAddr2->sin_addr.s_addr = inet_addr(lpszAscii);
	//inet_pton(AF_INET, lpszAscii, pAddr2);

	
	if (pAddr2->sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszAscii);
		if (lphost != NULL)
			pAddr2->sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
#ifdef UNICODE
			delete []lpszAscii;
#endif
			WSASetLastError(WSAEINVAL);
			return;
		}
	}
	

	pAddr2->sin_port = htons((u_short)uPort);

#ifdef UNICODE
	delete []lpszAscii;
#endif
}

BOOL CSTXSocket::BindServerSocket(UINT uDesiredListeningPort, UINT nTryRandom)
{
	if(uDesiredListeningPort == 0)
	{
		srand((unsigned int)time(NULL));
		uDesiredListeningPort = (rand() % 30000) + 1024;
	}

	SOCKADDR_IN addr;
	memset(&addr, 0 ,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(uDesiredListeningPort);

	int iResult = -1;
	while((iResult = bind(m_sockServer, (SOCKADDR*)&addr, sizeof(addr))) != 0 && nTryRandom > 0)
	{
		uDesiredListeningPort = (rand() % 30000) + 1024;
		addr.sin_port = htons(uDesiredListeningPort);
		nTryRandom--;
	}

	if(iResult != 0)
		return FALSE;

	m_uServerPort = uDesiredListeningPort;

	return TRUE;
}

BOOL CSTXSocket::RegisterServerSocketEvents()
{
	if(WSAAsyncSelect(m_sockServer, m_hWndSocket, WM_STXSOCKET_SERVER_EVENT, FD_ACCEPT|FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE) == SOCKET_ERROR)
		return FALSE;

	return TRUE;
}

BOOL CSTXSocket::CreateSocketWindow()
{
	WNDCLASS wc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hbrBackground = NULL;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hInstance = ::GetModuleHandle(NULL);
	wc.lpfnWndProc = SocketWindowProc;
	wc.lpszClassName = s_lpszClassName;
	wc.lpszMenuName = NULL;
	wc.style = 0;

	if(RegisterClass(&wc) == 0)
	{
		//Do Nothing
	}

	m_hWndSocket = ::CreateWindow(s_lpszClassName, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	if(m_hWndSocket == NULL)
		return FALSE;

	SetProp(m_hWndSocket, _T("SocketObject"), (HANDLE)this);

	return TRUE;
}

LRESULT CALLBACK CSTXSocket::SocketWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char szRecvBuffer[8192];
	if(uMsg == WM_STXSOCKET_SERVER_EVENT)		//Server Socket Event
	{
		CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
		int nError = WSAGETSELECTERROR(lParam);
		int nEvent = WSAGETSELECTEVENT(lParam);
		int nReceived = 0;
		switch(nEvent)
		{
		case FD_READ:
			{
				break;
			}
		case FD_CLOSE:
			{
				break;
			}
		case FD_WRITE:
			break;
		case FD_ACCEPT:
			{
				SOCKET sockNew = accept(pThis->m_sockServer, NULL, NULL);
				pThis->ProcessNewClient(sockNew);
				break;
			}
		case FD_CONNECT:
			break;
		}
	}
	else if(uMsg == WM_STXSOCKET_CLIENT_EVENT)
	{
		CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
		int nError = WSAGETSELECTERROR(lParam);
		int nEvent = WSAGETSELECTEVENT(lParam);
		SOCKET sock = (SOCKET)wParam;
		int nReceived = 0;
		switch(nEvent)
		{
		case FD_CLOSE:
			{
				CSTXSocketClientContext *pClientContext = NULL;


				EnterCriticalSection(&pThis->m_csClientMap);
				map<SOCKET, CSTXSocketClientContext*>::iterator it = pThis->m_mapClients.find(sock);
				if(it != pThis->m_mapClients.end())
				{
					pClientContext = it->second;
					pThis->m_mapClients.erase(it);
				}
				LeaveCriticalSection(&pThis->m_csClientMap);

				if(pClientContext)
				{
					WSAAsyncSelect(sock, pThis->m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
					closesocket(sock);
					pThis->OnClientDisconnect(pClientContext);
					pClientContext->ClearPendingMessages();
					pClientContext->m_sock = INVALID_SOCKET;
					pClientContext->Release();
				}
				break;
			}
		case FD_READ:
			{
				nReceived = recv(sock, szRecvBuffer, 8192, 0);
				if(nReceived > 0)
				{
					CSTXSocketClientContext *pClientContext = NULL;
					EnterCriticalSection(&pThis->m_csClientMap);
					map<SOCKET, CSTXSocketClientContext*>::iterator it = pThis->m_mapClients.find(sock);
					if(it != pThis->m_mapClients.end())
					{
						pClientContext = it->second;
						pClientContext->AddRef();
					}
					LeaveCriticalSection(&pThis->m_csClientMap);

					if(pClientContext)
					{
						pClientContext->AppendRecvData(szRecvBuffer, nReceived);
						DWORD dwMessageSize = 0;
						while((dwMessageSize = pClientContext->IsDataReadable()) > 0)
						{
							pClientContext->OnPreReceived(pClientContext->GetMessageBasePtr(), dwMessageSize);
							pClientContext->OnReceived(pClientContext->GetMessageBasePtr(), dwMessageSize);
							pClientContext->SkipRecvBuffer(dwMessageSize);
						}
						pClientContext->Release();
					}
				}
				break;
			}
		case FD_WRITE:
			{
				CSTXSocketClientContext *pClientContext = NULL;
				EnterCriticalSection(&pThis->m_csClientMap);
				map<SOCKET, CSTXSocketClientContext*>::iterator it = pThis->m_mapClients.find(sock);
				if(it != pThis->m_mapClients.end())
				{
					pClientContext = it->second;
					pClientContext->AddRef();
				}
				LeaveCriticalSection(&pThis->m_csClientMap);
				if(pClientContext)
				{
					pClientContext->OnPreSendReady();
					pClientContext->OnSendReady();
					pClientContext->Release();
				}
				break;
			}
		}
	}
	else if(uMsg == WM_STXSOCKET_CONNECTION_EVENT)
	{
		CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
		int nError = WSAGETSELECTERROR(lParam);
		int nEvent = WSAGETSELECTEVENT(lParam);
		SOCKET sock = (SOCKET)wParam;
		int nReceived = 0;
		switch(nEvent)
		{
		case FD_CLOSE:
			{
				CSTXSocketConnectionContext *pConnectionContext = NULL;
				EnterCriticalSection(&pThis->m_csConnectionMap);
				map<SOCKET, CSTXSocketConnectionContext*>::iterator it = pThis->m_mapConnections.find(sock);
				if(it != pThis->m_mapConnections.end())
				{
					pConnectionContext = it->second;
					pThis->m_mapConnections.erase(it);
				}
				LeaveCriticalSection(&pThis->m_csConnectionMap);

				if(pConnectionContext)
				{
					if (pConnectionContext->m_hWndConnectNotify)
						SendMessage(pConnectionContext->m_hWndConnectNotify, pConnectionContext->m_uMsgDisconnectNotify, (WPARAM)pConnectionContext, (LPARAM)nError);

					pConnectionContext->m_bConnected = FALSE;
					WSAAsyncSelect(sock, pThis->m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
					closesocket(sock);

					pThis->OnConnectionClose(pConnectionContext);
					SetEvent(pConnectionContext->m_hEvtUpload);
					SetEvent(pConnectionContext->m_hEvtDownload);
					pConnectionContext->ClearPendingMessages();
					pConnectionContext->Release();
				}

				break;
			}
		case FD_READ:
			{
				CSTXSocketConnectionContext *pConnectionContext = NULL;
				map<SOCKET, CSTXSocketConnectionContext*>::iterator it;
				EnterCriticalSection(&pThis->m_csConnectionMap);
				it = pThis->m_mapConnections.find(sock);
				if(it != pThis->m_mapConnections.end())
				{
					pConnectionContext = it->second;
					pConnectionContext->AddRef();
				}
				LeaveCriticalSection(&pThis->m_csConnectionMap);

				if(pConnectionContext)
				{
					nReceived = recv(sock, szRecvBuffer, 8192, 0);
					if(nReceived > 0)
					{
						pConnectionContext->AppendRecvData(szRecvBuffer, nReceived);
						DWORD dwMessageSize = 0;

						while((dwMessageSize = pConnectionContext->IsInternalDataReadable()) > 0)
						{
							pConnectionContext->OnInternalReceived(pConnectionContext->GetMessageBasePtr(), dwMessageSize);
							pConnectionContext->SkipRecvBuffer(dwMessageSize);
						}

						while((dwMessageSize = pConnectionContext->IsDataReadable()) > 0)
						{
							pConnectionContext->OnPreReceived(pConnectionContext->GetMessageBasePtr(), dwMessageSize);
							pConnectionContext->OnReceived(pConnectionContext->GetMessageBasePtr(), dwMessageSize);
							pConnectionContext->SkipRecvBuffer(dwMessageSize);
						}
					}
					pConnectionContext->Release();
				}

				break;
			}
		case FD_WRITE:
			{
				CSTXSocketConnectionContext *pConnectionContext = NULL;
				map<SOCKET, CSTXSocketConnectionContext*>::iterator it;
				EnterCriticalSection(&pThis->m_csConnectionMap);
				it = pThis->m_mapConnections.find(sock);
				if(it != pThis->m_mapConnections.end())
				{
					pConnectionContext = it->second;
					pConnectionContext->AddRef();
				}
				LeaveCriticalSection(&pThis->m_csConnectionMap);
				if(pConnectionContext)
				{
					pConnectionContext->OnSendReady();
					pConnectionContext->Release();
				}
				break;
			}
		case FD_CONNECT:
			pThis->ProcessNewConnection(sock, nError);
			break;
		}
	}
	else if(uMsg == WM_STXSOCKET_CREATESERVER_EVENT)
	{
		CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
		pThis->CreateServer((UINT)wParam, (UINT)lParam);
		SetEvent(pThis->m_hCreateServerSignal);
	}
	else if(uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
	}
	else if(uMsg == WM_TIMER)
	{
		CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
		UINT_PTR nTimerID = (UINT_PTR)wParam;
		EnterCriticalSection(&pThis->m_csTimerMap);
		map<UINT_PTR, CSTXSocketObjectBase*>::iterator it = pThis->m_mapTimers.find(nTimerID);
		if(it == pThis->m_mapTimers.end())
			KillTimer(hWnd, nTimerID);

		CSTXSocketObjectBase *pObject = it->second;
		LeaveCriticalSection(&pThis->m_csTimerMap);
		pObject->OnTimeout(nTimerID);
	}
	else
	{
		if(uMsg >= WM_USER)
		{
			CSTXSocket *pThis = (CSTXSocket*)GetProp(hWnd, _T("SocketObject"));
			return pThis->OnUserMessage(uMsg, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

CSTXSocketClientContext* CSTXSocket::OnCreateClientContext(SOCKET sock)
{
	CSTXSocketClientContext *pClientContext = new CSTXSocketClientContext();
	pClientContext->m_sock = sock;
	return pClientContext;
}

CSTXSocketConnectionContext* CSTXSocket::OnCreateConnectionContext(DWORD_PTR dwUserData)
{
	CSTXSocketConnectionContext *pConnectionContext = new CSTXSocketConnectionContext();
	return pConnectionContext;
}

UINT CSTXSocket::GetServerListeningPort()
{
	return m_uServerPort;
}

void CSTXSocket::OnClientDisconnect(CSTXSocketClientContext *pClientContext)
{
	OutputDebugString(_T("CSTXSocket::OnClientDisconnect\n"));
}

void CSTXSocket::OnConnectionClose(CSTXSocketConnectionContext *pConnectionContext)
{
	OutputDebugString(_T("CSTXSocket::OnConnectionClose\n"));
}

void CSTXSocket::OnClientInit(CSTXSocketClientContext *pClientContext)
{
	OutputDebugString(_T("CSTXSocket::OnClientInit ["));
	OutputDebugString(pClientContext->m_szPeerIP);
	OutputDebugString(_T("]\n"));
}

void CSTXSocket::OnConnectionInit(CSTXSocketConnectionContext *pConnectionContext)
{
	OutputDebugString(_T("CSTXSocket::OnConnectionInit ["));
	OutputDebugString(pConnectionContext->m_szPeerIP);
	OutputDebugString(_T("]\n"));
}

void CSTXSocket::OnSocketThreadInitialize()
{

}

void CSTXSocket::OnSocketThreadUninitialize()
{

}

LRESULT CSTXSocket::OnUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CSTXSocket::Close()
{
	if(m_sockServer != INVALID_SOCKET)
	{
		WSAAsyncSelect(m_sockServer, m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
		closesocket(m_sockServer);
		m_sockServer = INVALID_SOCKET;
	}

	CloseAllConnections();
	CloseAllClients();

	if(m_hThread != INVALID_HANDLE_VALUE)
	{
		DestroyWindow(m_hWndSocket);
		PostThreadMessage(m_uThreadID, WM_QUIT, 0, 0);
		WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = INVALID_HANDLE_VALUE;
	}
}

void CSTXSocket::CloseAllClients()
{
	EnterCriticalSection(&m_csClientMap);
	map<SOCKET, CSTXSocketClientContext*>::iterator it = m_mapClients.begin();
	for(;it!=m_mapClients.end();it++)
	{
		closesocket(it->second->m_sock);
		it->second->Release();
	}
	m_mapClients.clear();
	LeaveCriticalSection(&m_csClientMap);
}

void CSTXSocket::CloseAllConnections()
{
	EnterCriticalSection(&m_csConnectionMap);
	map<SOCKET, CSTXSocketConnectionContext*>::iterator it = m_mapConnections.begin();
	for(;it!=m_mapConnections.end();it++)
	{
		closesocket(it->second->m_sock);
		it->second->Release();
	}
	m_mapConnections.clear();
	LeaveCriticalSection(&m_csConnectionMap);
}

CSTXSocketConnectionContext* CSTXSocket::ConnectEx(LPCTSTR lpszAddress, DWORD dwTimeout, DWORD_PTR dwUserData)
{
	int nLen =(int) _tcslen(lpszAddress);
	TCHAR *pszAddrFull = new TCHAR[nLen + 2];
	_tcscpy_s(pszAddrFull, nLen + 2, lpszAddress);

	TCHAR *pszSep = _tcschr(pszAddrFull, _T(':'));
	if(pszSep == NULL)
	{
		delete []pszAddrFull;
		return NULL;
	}

	*pszSep = 0;
	TCHAR *pszHost = pszAddrFull;
	TCHAR *pszPort = pszSep + 1;

	CSTXSocketConnectionContext *pConnCtx = Connect(pszHost, _ttol(pszPort), dwTimeout, dwUserData);
	delete []pszAddrFull;
	return pConnCtx;
}

CSTXSocketConnectionContext* CSTXSocket::Connect(LPCTSTR lpszAddress, UINT uPort, DWORD dwWaitTime, DWORD_PTR dwUserData)
{
	SOCKADDR_IN addr;
	SockaddrFromHost(lpszAddress, uPort, (SOCKADDR*)&addr, AF_INET);

	//SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET)
		return NULL;

	CSTXSocketConnectionContext *pConnectionContext = OnCreateConnectionContext(dwUserData);
	pConnectionContext->AddRef();
	pConnectionContext->m_pMainSocket = this;
	pConnectionContext->m_dwUserData = dwUserData;
	pConnectionContext->m_sock = sock;

	EnterCriticalSection(&m_csConnectionMap);
	m_mapConnections[sock] = pConnectionContext;
	LeaveCriticalSection(&m_csConnectionMap);

	WSAAsyncSelect(sock, m_hWndSocket, WM_STXSOCKET_CONNECTION_EVENT, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);

	if(connect(sock, (SOCKADDR*)&addr, sizeof(addr)) == 0)
	{
		InitBaseObjectInformation(pConnectionContext);
		pConnectionContext->Release();
		return pConnectionContext;
	}

	if(WSAGetLastError() == WSAEWOULDBLOCK)
	{
		DWORD dwWaitResult = WaitForSingleObject(pConnectionContext->m_hConnectSignal, dwWaitTime);
		if(dwWaitTime == WAIT_TIMEOUT)
		{
		}
		else
		{
			if(pConnectionContext->m_bConnected)
			{
				InitBaseObjectInformation(pConnectionContext);
				pConnectionContext->Release();
				return pConnectionContext;
			}
		}
	}

	EnterCriticalSection(&m_csConnectionMap);
	m_mapConnections.erase(sock);
	LeaveCriticalSection(&m_csConnectionMap);

	pConnectionContext->Release();

	WSAAsyncSelect(sock, m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
	closesocket(sock);

	return NULL;
}

BOOL CSTXSocket::InitBaseObjectInformation(CSTXSocketObjectBase *pBaseObject)
{
	SOCKADDR_IN addr;
	int nAddrLen = sizeof(addr);
	getpeername(pBaseObject->m_sock, (SOCKADDR*)&addr, &nAddrLen);
	pBaseObject->m_addrPeer = addr;

	LPSTR pszAddr = inet_ntoa(addr.sin_addr);
	if(pszAddr)
	{
#ifdef UNICODE
		::MultiByteToWideChar(CP_ACP, 0, pszAddr, -1, pBaseObject->m_szPeerIP, 64);
#else
		_tcscpy_s(pClientContext->m_szPeerIP, pszAddr);
#endif
	}
	else
	{
		OutputDebugString(_T("Failed to convert client addr!"));
	}
	pBaseObject->m_nPeerPort = ntohs(addr.sin_port);

	return TRUE;
}

BOOL CSTXSocket::ProcessNewConnection(SOCKET sock, int nError)
{
	if(sock != INVALID_SOCKET)
	{
		CSTXSocketConnectionContext *pConnectionContext = NULL;
		EnterCriticalSection(&m_csConnectionMap);
		map<SOCKET, CSTXSocketConnectionContext*>::iterator it = m_mapConnections.find(sock);
		if(it == m_mapConnections.end())
		{
			LeaveCriticalSection(&m_csConnectionMap);
			return FALSE;
		}
		pConnectionContext = it->second;
		LeaveCriticalSection(&m_csConnectionMap);

		if(nError)
		{
			pConnectionContext->m_bConnected = FALSE;
		}
		else
		{
			pConnectionContext->m_bConnected = TRUE;
		}

		SetEvent(pConnectionContext->m_hConnectSignal);

		if(nError == 0)
		{
			InitBaseObjectInformation(pConnectionContext);
			OnConnectionInit(pConnectionContext);
			if(pConnectionContext->m_hWndConnectNotify)
				SendMessage(pConnectionContext->m_hWndConnectNotify, pConnectionContext->m_uMsgConnectNotify, (WPARAM)pConnectionContext, (LPARAM)nError);
		}
		else
		{
			EnterCriticalSection(&m_csConnectionMap);
			m_mapConnections.erase(sock);
			LeaveCriticalSection(&m_csConnectionMap);

			if(pConnectionContext->m_hWndConnectNotify)
				SendMessage(pConnectionContext->m_hWndConnectNotify, pConnectionContext->m_uMsgConnectNotify, (WPARAM)pConnectionContext, (LPARAM)nError);
			pConnectionContext->Release();

			WSAAsyncSelect(sock, m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
			closesocket(sock);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CSTXSocket::ProcessNewClient(SOCKET sock)
{
	if(sock != INVALID_SOCKET)
	{
		CSTXSocketClientContext *pClientContext = OnCreateClientContext(sock);
		pClientContext->m_pMainSocket = this;
		pClientContext->m_sock = sock;
		InitBaseObjectInformation(pClientContext);

		EnterCriticalSection(&m_csClientMap);
		m_mapClients[sock] = pClientContext;
		LeaveCriticalSection(&m_csClientMap);

		WSAAsyncSelect(sock, m_hWndSocket, WM_STXSOCKET_CLIENT_EVENT, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);

		OnClientInit(pClientContext);
		return TRUE;
	}

	return FALSE;
}

void CSTXSocket::PostOnSendReady(CSTXSocketObjectBase *pBaseObject)
{
	PostMessage(m_hWndSocket, pBaseObject->m_dwEventID, (WPARAM)pBaseObject->m_sock, MAKELPARAM(FD_WRITE, 0));
}

void CSTXSocket::PostClose(CSTXSocketObjectBase *pBaseObject)
{
	PostMessage(m_hWndSocket, pBaseObject->m_dwEventID, (WPARAM)pBaseObject->m_sock, MAKELPARAM(FD_CLOSE, 0));
}

BOOL CSTXSocket::CreateServer(UINT uDesiredListeningPort, UINT nTryRandom)
{
	if(GetCurrentThreadId() == m_uThreadID)
	{
		if(!CreateServerSocket())
		{
			return FALSE;
		}

		if(!BindServerSocket(uDesiredListeningPort, nTryRandom))
		{
			return FALSE;
		}

		listen(m_sockServer, 5);

		if(!RegisterServerSocketEvents())
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		ResetEvent(m_hCreateServerSignal);
		PostMessage(m_hWndSocket, WM_STXSOCKET_CREATESERVER_EVENT, uDesiredListeningPort, nTryRandom);
		WaitForSingleObject(m_hCreateServerSignal, INFINITE);
		return (GetServerListeningPort() != 0);
	}
	return FALSE;
}

void CSTXSocket::EnableInternalMessages( BOOL bEnable /*= TRUE*/ )
{
	m_bEnableInternalMessages = bEnable;
}

BOOL CSTXSocket::IsInternalMessagesEnabled()
{
	return m_bEnableInternalMessages;
}

BOOL CSTXSocket::PostUserMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return ::PostMessage(m_hWndSocket, uMsg, wParam, lParam);
}

LRESULT CSTXSocket::SendUserMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return ::SendMessage(m_hWndSocket, uMsg, wParam, lParam);
}

HWND CSTXSocket::GetInternalHWND()
{
	return m_hWndSocket;
}

void CSTXSocket::BeginConnect( LPCTSTR lpszAddress, UINT uPort, HWND hWndNotify, UINT uMsgNotify, UINT uMsgDisconnectNotify, DWORD_PTR dwUserData )
{
	SOCKADDR_IN addr;
	SockaddrFromHost(lpszAddress, uPort, (SOCKADDR*)&addr, AF_INET);

	int nError = 0;
	//SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	nError = WSAGetLastError();
	if(sock == INVALID_SOCKET)
	{
		SendMessage(hWndNotify, uMsgNotify, (WPARAM)0, nError);
		return;
	}

	CSTXSocketConnectionContext *pConnectionContext = OnCreateConnectionContext(dwUserData);
	pConnectionContext->m_pMainSocket = this;
	pConnectionContext->m_dwUserData = dwUserData;
	pConnectionContext->m_sock = sock;
	pConnectionContext->m_hWndConnectNotify = hWndNotify;
	pConnectionContext->m_uMsgConnectNotify = uMsgNotify;
	pConnectionContext->m_uMsgDisconnectNotify = uMsgDisconnectNotify;

	EnterCriticalSection(&m_csConnectionMap);
	m_mapConnections[sock] = pConnectionContext;
	LeaveCriticalSection(&m_csConnectionMap);

	WSAAsyncSelect(sock, m_hWndSocket, WM_STXSOCKET_CONNECTION_EVENT, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);

	if(connect(sock, (SOCKADDR*)&addr, sizeof(addr)) == 0)
	{
		InitBaseObjectInformation(pConnectionContext);
		SendMessage(hWndNotify, uMsgNotify, (WPARAM)pConnectionContext, 0);
		return;
	}

	nError = WSAGetLastError();
	if(nError == WSAEWOULDBLOCK)
	{
		return;
	}
	else
	{
		SendMessage(hWndNotify, uMsgNotify, (WPARAM)pConnectionContext, (LPARAM)nError);
	}

	EnterCriticalSection(&m_csConnectionMap);
	m_mapConnections.erase(sock);
	LeaveCriticalSection(&m_csConnectionMap);

	pConnectionContext->Release();

	WSAAsyncSelect(sock, m_hWndSocket, 0, FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE);
	closesocket(sock);

	return;
}

void CSTXSocket::BeginConnectEx( LPCTSTR lpszAddress, HWND hWndNotify, UINT uMsgNotify, UINT uMsgDisconnectNotify, DWORD_PTR dwUserData /*= 0*/ )
{
	int nLen =(int) _tcslen(lpszAddress);
	TCHAR *pszAddrFull = new TCHAR[nLen + 2];
	_tcscpy_s(pszAddrFull, nLen + 2, lpszAddress);

	TCHAR *pszSep = _tcschr(pszAddrFull, _T(':'));
	if(pszSep == NULL)
	{
		delete []pszAddrFull;
		SendMessage(hWndNotify, uMsgNotify, (WPARAM)0, ERROR_INVALID_PARAMETER);
		return;
	}

	*pszSep = 0;
	TCHAR *pszHost = pszAddrFull;
	TCHAR *pszPort = pszSep + 1;

	BeginConnect(pszHost, _ttol(pszPort), hWndNotify, uMsgNotify, uMsgDisconnectNotify);
	delete []pszAddrFull;
}

UINT_PTR CSTXSocket::SetInternalTimer( DWORD dwInterval, CSTXSocketObjectBase* pNotifyObject )
{
	UINT_PTR nTimerID = m_nInternalTimerIDBase++;
	pNotifyObject->AddRef();
	SetTimer(m_hWndSocket, nTimerID, dwInterval, NULL);
	EnterCriticalSection(&m_csTimerMap);
	m_mapTimers[nTimerID] = pNotifyObject;
	LeaveCriticalSection(&m_csTimerMap);
	return nTimerID;
}

void CSTXSocket::KillInternalTimer( UINT_PTR nTimerID )
{
	EnterCriticalSection(&m_csTimerMap);
	if(m_mapTimers.find(nTimerID) == m_mapTimers.end())
	{
		LeaveCriticalSection(&m_csTimerMap);
		return;
	}

	KillTimer(m_hWndSocket, nTimerID);
	m_mapTimers[nTimerID]->Release();
	m_mapTimers.erase(nTimerID);
	LeaveCriticalSection(&m_csTimerMap);
}
