#include "stdafx.h"
#include "FastSpinlock.h"
#include <windows.h>
#include <MMSystem.h>

#pragma comment(lib, "Winmm.lib")
FastSpinlock::FastSpinlock() : mLockFlag(0), mThreadId(0), mSpinCount(0)
{
}


FastSpinlock::~FastSpinlock()
{
}


void FastSpinlock::EnterLock()
{
	for (int nloops = 0;; nloops++)
	{
		if (InterlockedExchange(&mLockFlag, 1) == 0)
		{
			InterlockedIncrement(&mSpinCount);
			mThreadId = GetCurrentThreadId();
			return;
		}
		else
		{
			if (mThreadId != 0 && GetCurrentThreadId() == mThreadId)
			{
				InterlockedIncrement(&mSpinCount);
				return;
			}
		}
	
		UINT uTimerRes = 1;
		timeBeginPeriod(uTimerRes); 
		Sleep((DWORD)min(10, nloops));
		timeEndPeriod(uTimerRes);
	}

}

void FastSpinlock::LeaveLock()
{
	if (GetCurrentThreadId() == mThreadId)
	{
		if(InterlockedDecrement(&mSpinCount) == 0)
		{
			mThreadId = 0;
			InterlockedExchange(&mLockFlag, 0);
		}
	}
}