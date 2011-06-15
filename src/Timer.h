#pragma once
#ifndef __CTIMER_H__
#define __CTIMER_H__

class CTimer
{
public:
	inline	__int64	GetTimeInTicks( void )			const { return Time; }
	inline	float	GetTimeInSec( void )			const { return m_fTimeInSec; }
	inline	__int64	GetElapsedTimeInTicks( void )	const { return Time - StartTime; }
	inline	float	GetElapsedTimeInSec( void )		const { return m_fElapsedTimeInSec; }
	inline	__int64	GetTicksPerSecond( void )		const { return TicksPerSecond; }

	// Public member functions.
	void Reset()
	{
#ifdef WIN32
		QueryPerformanceFrequency((LARGE_INTEGER*)&TicksPerSecond);
		QueryPerformanceCounter((LARGE_INTEGER*)&Time);
		StartTime = Time;
		SecondsPerTick = 1.0 / TicksPerSecond;
#endif
	}
	void Update()
	{
#ifdef WIN32
		QueryPerformanceCounter((LARGE_INTEGER*)&Time);
		const double locTime	= (double)(Time - StartTime) * SecondsPerTick;
		m_fElapsedTimeInSec		= (float)locTime;
		m_fTimeInSec			= (float)( Time * SecondsPerTick );
#endif
	}

	CTimer(void)
	{
	}

	~CTimer(void)
	{
	}
private:
	__int64	TicksPerSecond;
	double	SecondsPerTick;
	float	m_fElapsedTimeInSec;
	float	m_fTimeInSec;

	__int64	StartTime;
	__int64	Time;
};

#endif // __CTIMER_H__