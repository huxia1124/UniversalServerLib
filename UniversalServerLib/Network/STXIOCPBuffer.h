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
#include <tchar.h>

#include "STXLog.h"

#pragma warning(disable:4786)
#include <map>
#include <queue>
#include <concurrentqueue.h>
#include <FastSpinlock.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////

#define STXIOCPBUFFERCOLLECTION_MAX_BUFFER_OBJECT_COUNT		100000


//////////////////////////////////////////////////////////////////////////
// CSTXIOCPBuffer

#define STXIOCPBUFFER_FLAG_AUTOEXPAND		0x00010000		//�� Buffer �ռ䲻��ʱ���Զ���չ�ռ�

class CSTXIOCPBuffer
{
public:
	CSTXIOCPBuffer();
	virtual ~CSTXIOCPBuffer();

protected:
	FastSpinlock m_lock;
	//CRITICAL_SECTION m_cs;
	char* m_pBuffer;
	DWORD m_dwBufferLength;		//Full length of buffer
	DWORD m_dwContentLength;	//Length of data
	int m_nLock;

	DWORD m_dwFlags;
	DWORD_PTR m_dwUserData;

public:
	//����Ϊ��� Buffer �������ռ�
	BOOL ReallocateBuffer(DWORD dwBufferSize);

	//���� Buffer ���󣬷��������������������
	int LockBuffer();

	//���� Buffer ���󣬷��ؽ����������������
	int UnlockBuffer();

	//�� Buffer ��д�����ݣ�����ʵ��д������ݴ�С(��λ:�ֽ�)
	//�˷����������׷������
	DWORD WriteBuffer(LPVOID pData, DWORD cbDataLen);

	//�� Buffer �ж�ȡ���ݲ����浽 pOutBuffer ��ָ��Ĵ洢�ռ��У�����ʵ�ʶ�ȡ�����ݴ�С(��λ:�ֽ�)
	DWORD ReadBuffer(LPVOID pOutBuffer, DWORD cbOutBufferLen);

	//��� Buffer �ռ�ĵ�ַ
	LPVOID GetBufferPtr();

	//��� Buffer �ռ�Ĵ�С(��λ:�ֽ�)
	DWORD GetBufferLength();

	//��� Buffer ��ʵ�����ݵĴ�С(��λ:�ֽ�)
	DWORD GetDataLength();

	//���� Buffer ��ʵ�����ݵĴ�С(��λ:�ֽ�)��
	DWORD SetDataLength(DWORD cbBufferDataLen);

	//�ͷ���� Buffer �ռ䣬�Ӵ���� Buffer �����ٿ��ã�ֱ���ٴγɹ����� ReallocateBuffer Ϊֹ
	BOOL ClearBuffer();

	//�����û��Զ�������
	void SetUserData(DWORD_PTR dwUserData);

	//��ȡ�û��Զ�������
	DWORD_PTR GetUserData();

	//����һ�������и�������
	DWORD CopyData(CSTXIOCPBuffer *pSrcBuffer);

	BOOL WriteToFile( LPCTSTR lpszFile );
	BOOL ResetWritePos();
};

//////////////////////////////////////////////////////////////////////////
// CSTXIOCPBufferCollection

class CSTXIOCPBufferCollection
{
public:
	CSTXIOCPBufferCollection();
	virtual ~CSTXIOCPBufferCollection();

protected:
	moodycamel::ConcurrentQueue<CSTXIOCPBuffer*> m_queueBuffers;

	volatile LONG m_nBufferAvailable;
	volatile LONG m_nMaxBufferCount;
	volatile LONG m_nTotalBufferCount;
	DWORD m_dwSingleBufferSize;

private:
	static int s_iBufferCollectionID;

public:

	//����ָ�������� CSTXIOCPBuffer ���󣬲���ÿ�� CSTXIOCPBuffer �����з���ָ����С�Ŀռ�
	LONG CreateBuffers(LONG nBufferCount, DWORD dwSingleBufferSize, LONG nMaxBufferCount);

	//��������е� CSTXIOCPBuffer ����
	LONG ClearBuffers();

	//���һ�� CSTXIOCPBuffer ��������������Ѿ�û�п��õ� CSTXIOCPBuffer �����򷵻� NULL
	//��� dwWaitTime ��Ϊ 0����ȴ� dwWaitTime ��ʱ��ֱ��ʱ�䵽����� CSTXIOCPBuffer ������ã����ʱ�䵽�����Ȼû�� CSTXIOCPBuffer ������ã��򷵻� NULL
	//���ص� CSTXIOCPBuffer ������ʹ�����֮�󣬱������ ReleaseBuffer �ͷŶԸö����ռ��
	CSTXIOCPBuffer* GetBuffer(DWORD dwWaitTime = 0);

	//��ü����� CSTXIOCPBuffer ������ܸ���
	LONG GetBufferTotalCount();

	//�ͷ� CSTXIOCPBuffer �����Թ��ٴ�ʹ��
	BOOL ReleaseBuffer(CSTXIOCPBuffer* pBufferObj);

	//��ü����е�ǰ���õ� CSTXIOCPBuffer ����ĸ���
	LONG GetBufferAvailableCount();

	LONG ExpandBuffers(LONG nExpand = 1, BOOL bReleaseSemaphore = TRUE);


};
