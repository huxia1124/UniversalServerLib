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

#define STXIOCPBUFFER_FLAG_AUTOEXPAND		0x00010000		//当 Buffer 空间不够时，自动扩展空间

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
	//重新为这个 Buffer 对象分配空间
	BOOL ReallocateBuffer(DWORD dwBufferSize);

	//锁定 Buffer 对象，返回锁定后的总锁定次数
	int LockBuffer();

	//解锁 Buffer 对象，返回解锁后的总锁定次数
	int UnlockBuffer();

	//向 Buffer 中写入数据，返回实际写入的数据大小(单位:字节)
	//此方法向对象中追加数据
	DWORD WriteBuffer(LPVOID pData, DWORD cbDataLen);

	//从 Buffer 中读取数据并保存到 pOutBuffer 所指向的存储空间中，返回实际读取的数据大小(单位:字节)
	DWORD ReadBuffer(LPVOID pOutBuffer, DWORD cbOutBufferLen);

	//获得 Buffer 空间的地址
	LPVOID GetBufferPtr();

	//获得 Buffer 空间的大小(单位:字节)
	DWORD GetBufferLength();

	//获得 Buffer 中实际数据的大小(单位:字节)
	DWORD GetDataLength();

	//设置 Buffer 中实际数据的大小(单位:字节)。
	DWORD SetDataLength(DWORD cbBufferDataLen);

	//释放这个 Buffer 空间，从此这个 Buffer 对象不再可用，直到再次成功调用 ReallocateBuffer 为止
	BOOL ClearBuffer();

	//设置用户自定义数据
	void SetUserData(DWORD_PTR dwUserData);

	//获取用户自定义数据
	DWORD_PTR GetUserData();

	//从另一个对象中复制数据
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

	//创建指定个数的 CSTXIOCPBuffer 对象，并在每个 CSTXIOCPBuffer 对象中分配指定大小的空间
	LONG CreateBuffers(LONG nBufferCount, DWORD dwSingleBufferSize, LONG nMaxBufferCount);

	//清除除所有的 CSTXIOCPBuffer 对象
	LONG ClearBuffers();

	//获得一个 CSTXIOCPBuffer 对象，如果集合中已经没有可用的 CSTXIOCPBuffer 对象，则返回 NULL
	//如果 dwWaitTime 不为 0，则等待 dwWaitTime 的时间直到时间到达或有 CSTXIOCPBuffer 对象可用，如果时间到达后仍然没有 CSTXIOCPBuffer 对象可用，则返回 NULL
	//返回的 CSTXIOCPBuffer 对象在使用完成之后，必须调用 ReleaseBuffer 释放对该对象的占有
	CSTXIOCPBuffer* GetBuffer(DWORD dwWaitTime = 0);

	//获得集合中 CSTXIOCPBuffer 对象的总个数
	LONG GetBufferTotalCount();

	//释放 CSTXIOCPBuffer 对象，以供再次使用
	BOOL ReleaseBuffer(CSTXIOCPBuffer* pBufferObj);

	//获得集合中当前可用的 CSTXIOCPBuffer 对象的个数
	LONG GetBufferAvailableCount();

	LONG ExpandBuffers(LONG nExpand = 1, BOOL bReleaseSemaphore = TRUE);


};
