// This code is from the Workspace Whiz! source distribution.
// Copyright 2000 Joshua C. Jensen
#pragma once

#include <windows.h>

class Timer
{
public:
	Timer(void);

	void Reset(void);
	void Start(void);
	void Stop(void);
	double GetDuration(void) const;
	double GetMillisecs(void) const;

protected:
	LARGE_INTEGER m_startTime;
	LONGLONG m_totalTime;

	LONGLONG m_frequency;
};

inline Timer::Timer(void) :
	m_totalTime(0)
{
	LARGE_INTEGER liFrequency;

	QueryPerformanceFrequency(&liFrequency);
	m_frequency = liFrequency.QuadPart;

	m_totalTime = 0;
}

inline void Timer::Reset()
{
	m_totalTime = 0;
}
	
inline void Timer::Start(void)
{
	QueryPerformanceCounter(&m_startTime);
}

inline void Timer::Stop(void)
{
	LARGE_INTEGER stopTime;
	QueryPerformanceCounter(&stopTime);
	m_totalTime += (stopTime.QuadPart - m_startTime.QuadPart);
}

inline double Timer::GetDuration(void) const
{
	return (double)m_totalTime * 1000000.0 / m_frequency;
}

inline double Timer::GetMillisecs(void) const
{
	return (double)m_totalTime * 1000000.0 / m_frequency / 1000.0;
}
