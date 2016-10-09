#pragma once
#include <chrono>

template<class T = __int64, int seconds = 10>
class CStatisticsBuffer
{
public:
	CStatisticsBuffer();
	~CStatisticsBuffer();

public:
	T _data[seconds];
	long long _timePoint[seconds];
	long long _lastIndex;
	long long _startIndex;

public:
	void AddValue(T val);
	T GetTotal();
	T GetAverage();
};

template<class T /*= __int64*/, int seconds /*= 10*/>
T CStatisticsBuffer<T, seconds>::GetAverage()
{
	T total = GetTotal();
	return total / seconds;
}

template<class T /*= __int64*/, int seconds /*= 10*/>
T CStatisticsBuffer<T, seconds>::GetTotal()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto currentSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

	T total = 0;
	for (int k = 0; k < seconds - 1; k++) {
		if (_timePoint[(k + _lastIndex + 1) % seconds] == currentSec - seconds + k + 1)
			total += _data[(k + _lastIndex + 1) % seconds];
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

