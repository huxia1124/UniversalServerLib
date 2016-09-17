
#include "STXIOCPBuffer.h"
//#include "Global.h"

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPBuffer
//////////////////////////////////////////////////////////////////////////

CSTXIOCPBuffer::CSTXIOCPBuffer()
{
	m_nLock = 0;
	m_dwBufferLength = 0;
	m_dwContentLength = 0;
	m_pBuffer = NULL;
	m_dwUserData = 0;

	m_dwFlags = STXIOCPBUFFER_FLAG_AUTOEXPAND;
}

CSTXIOCPBuffer::~CSTXIOCPBuffer()
{
	ClearBuffer();
}

BOOL CSTXIOCPBuffer::ClearBuffer()
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

BOOL CSTXIOCPBuffer::ReallocateBuffer(DWORD dwBufferSize)
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

int CSTXIOCPBuffer::LockBuffer()
{
	m_lock.EnterLock();
	m_nLock++;
	return m_nLock;
}

int CSTXIOCPBuffer::UnlockBuffer()
{
	m_nLock--;
	m_lock.LeaveLock();
	return m_nLock;
}

DWORD CSTXIOCPBuffer::WriteBuffer(LPVOID pData, DWORD cbDataLen)
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

DWORD CSTXIOCPBuffer::ReadBuffer(LPVOID pOutBuffer, DWORD cbOutBufferLen)
{
	LockBuffer();
	DWORD dwCopyLen = min(cbOutBufferLen, m_dwBufferLength);
	::CopyMemory(pOutBuffer, m_pBuffer, dwCopyLen);
	UnlockBuffer();

	return dwCopyLen;
}

LPVOID CSTXIOCPBuffer::GetBufferPtr()
{
	return m_pBuffer;
}

DWORD CSTXIOCPBuffer::GetBufferLength()
{
	return m_dwBufferLength;
}

DWORD CSTXIOCPBuffer::GetDataLength()
{
	return m_dwContentLength;
}

DWORD CSTXIOCPBuffer::SetDataLength(DWORD cbBufferDataLen)
{
	m_dwContentLength = min(cbBufferDataLen, m_dwBufferLength);
	return m_dwContentLength;
}

void CSTXIOCPBuffer::SetUserData( DWORD_PTR dwUserData )
{
	m_dwUserData = dwUserData;
}

DWORD_PTR CSTXIOCPBuffer::GetUserData()
{
	return m_dwUserData;
}

DWORD CSTXIOCPBuffer::CopyData(CSTXIOCPBuffer *pSrcBuffer)
{
	ClearBuffer();
	return WriteBuffer(pSrcBuffer->GetBufferPtr(), pSrcBuffer->GetDataLength());
}

BOOL CSTXIOCPBuffer::WriteToFile( LPCTSTR lpszFile )
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

BOOL CSTXIOCPBuffer::ResetWritePos()
{
	m_dwContentLength = 0;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPBufferCollection
//////////////////////////////////////////////////////////////////////////

int CSTXIOCPBufferCollection::s_iBufferCollectionID = 0;

CSTXIOCPBufferCollection::CSTXIOCPBufferCollection()
{
	m_nBufferAvailable = 0;
	m_nMaxBufferCount = 0;
	m_nTotalBufferCount = 0;
	m_dwSingleBufferSize = 256;
}

CSTXIOCPBufferCollection::~CSTXIOCPBufferCollection()
{
	ClearBuffers();
}

LONG CSTXIOCPBufferCollection::ClearBuffers()
{
	LONG nCount = (LONG)m_queueBuffers.size_approx();

	if(nCount == 0)
		return 0;


	LONG nResult = nCount;
	CSTXIOCPBuffer *pBuffer = NULL;
	while(m_queueBuffers.try_dequeue(pBuffer))
	{
		 delete pBuffer;
	}

	return nResult;
}

LONG CSTXIOCPBufferCollection::CreateBuffers(LONG nBufferCount, DWORD dwSingleBufferSize, LONG nMaxBufferCount)
{
	for(int i=0;i<nBufferCount;i++)
	{
		CSTXIOCPBuffer *pBufferObj = new CSTXIOCPBuffer();
		if(pBufferObj->ReallocateBuffer(dwSingleBufferSize))
		{
			m_queueBuffers.enqueue(pBufferObj);
		}
		pBufferObj = NULL;
	}

	m_nBufferAvailable = (LONG)m_queueBuffers.size_approx();
	m_nTotalBufferCount = (LONG)m_queueBuffers.size_approx();
	m_nMaxBufferCount = nMaxBufferCount;
	m_dwSingleBufferSize = dwSingleBufferSize;

	return (LONG)m_nBufferAvailable;
}

CSTXIOCPBuffer* CSTXIOCPBufferCollection::GetBuffer(DWORD dwWaitTime)
{
	CSTXIOCPBuffer *pBufferObj = NULL;
	BOOL bExpand = FALSE;

	if (m_queueBuffers.try_dequeue(pBufferObj))
	{
		pBufferObj->ResetWritePos();
		InterlockedDecrement(&m_nBufferAvailable);
		return pBufferObj;
	}
	else
	{
		ExpandBuffers(1, FALSE);
	}

	
	if (m_queueBuffers.try_dequeue(pBufferObj))
	{
		pBufferObj->ResetWritePos();
		InterlockedDecrement(&m_nBufferAvailable);
		return pBufferObj;
	}
	else
	{
		ExpandBuffers(1, FALSE);
	}

	return NULL;
}

BOOL CSTXIOCPBufferCollection::ReleaseBuffer(CSTXIOCPBuffer* pBufferObj)
{
	BOOL bResult = FALSE;
	LONG nPrevValue = 0;
	pBufferObj->SetDataLength(0);
	pBufferObj->SetUserData(0);
	m_queueBuffers.enqueue(pBufferObj);
	InterlockedIncrement(&m_nBufferAvailable);

	return bResult;
}

LONG CSTXIOCPBufferCollection::GetBufferTotalCount()
{
	return m_nTotalBufferCount;
}

LONG CSTXIOCPBufferCollection::GetBufferAvailableCount()
{
	return m_nBufferAvailable;
}

LONG CSTXIOCPBufferCollection::ExpandBuffers(LONG nExpand, BOOL bReleaseSemaphore)
{
	for(int i=0;i<nExpand;i++)
	{
		CSTXIOCPBuffer *pBufferObj = new CSTXIOCPBuffer();
		if(pBufferObj->ReallocateBuffer(m_dwSingleBufferSize))
		{
			m_queueBuffers.enqueue(pBufferObj);
			InterlockedIncrement(&m_nTotalBufferCount);
			InterlockedIncrement(&m_nBufferAvailable);
		}
		pBufferObj = NULL;
	}

	return m_nTotalBufferCount;
}
