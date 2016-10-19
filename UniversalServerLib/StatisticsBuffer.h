#pragma once
#include <chrono>

template<class T = __int64, int seconds = 10>
class CStatisticsBuffer
{
public:
	CStatisticsBuffer();
	~CStatisticsBuffer();

protected:
	int _validBufferSize = seconds;

public:
	T _data[seconds];
	long long _timePoint[seconds];
	long long _lastIndex;
	long long _startIndex;

public:
	void AddValue(T val);
	T GetTotal();
	T GetAverage();
	int GetMaxBufferSize();
	int GetValidBufferSize();
	void SetValidBufferSize(int size);
};

template<class T /*= __int64*/, int seconds /*= 10*/>
void CStatisticsBuffer<T, seconds>::SetValidBufferSize(int size)
{
	_validBufferSize = max(1, min(seconds, size));
}

template<class T /*= __int64*/, int seconds /*= 10*/>
int CStatisticsBuffer<T, seconds>::GetValidBufferSize()
{
	return _validBufferSize;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
int CStatisticsBuffer<T, seconds>::GetMaxBufferSize()
{
	return seconds;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
T CStatisticsBuffer<T, seconds>::GetAverage()
{
	T total = GetTotal();
	return total / _validBufferSize;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
T CStatisticsBuffer<T, seconds>::GetTotal()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto currentSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

	T total = 0;
	for (int k = 0; k < _validBufferSize; k++) {
		int index = (-k + _lastIndex + seconds) % seconds;
		auto timepoint = _timePoint[index];
		if (timepoint >= currentSec - _validBufferSize && timepoint < currentSec)
			total += _data[index];
	}
	return total;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
CStatisticsBuffer<T, seconds>::~CStatisticsBuffer()
{

}

template<class T, int seconds>
inline void CStatisticsBuffer<T, seconds>::AddValue(T val)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto currentSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	auto index = currentSec % seconds;
	if (_timePoint[index] != currentSec) {
		_timePoint[index] = currentSec;

		_data[index] = 0;
		_lastIndex = index;
	}
	_data[index] += val;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
CStatisticsBuffer<T, seconds>::CStatisticsBuffer()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto currentSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	auto startIndex = currentSec % seconds;
	_startIndex = startIndex;
	_lastIndex = startIndex;
	memset(_data, 0, sizeof(_data));
	memset(_timePoint, 0, sizeof(_timePoint));
}

