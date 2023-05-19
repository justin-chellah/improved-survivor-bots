#pragma once

#include <networkvar.h>

extern CGlobalVars *gpGlobals;

//--------------------------------------------------------------------------------------------------------------
/**
 * Simple class for counting down a short interval of time.
 * Upon creation, the timer is invalidated.  Invalidated countdown timers are considered to have elapsed.
 */
class CountdownTimer
{
public:
#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();
#endif
	DECLARE_CLASS_NOBASE( CountdownTimer );
	DECLARE_EMBEDDED_NETWORKVAR();

	CountdownTimer( void )
	{
		m_duration = 0.0f;
		m_timestamp = -1.0f;
	}

	void Reset( void )
	{
		m_timestamp = Now() + m_duration;
	}

	void Start( float duration )
	{
		m_timestamp = Now() + duration;
		m_duration = duration;
	}

	void StartFromTime( float startTime, float duration )
	{
		m_timestamp = startTime + duration;
		m_duration = duration;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	bool IsElapsed( void ) const
	{
		return (Now() > m_timestamp);
	}

	float GetElapsedTime( void ) const
	{
		return Now() - m_timestamp + m_duration;
	}

	float GetRemainingTime( void ) const
	{
		return (m_timestamp - Now());
	}

	/// return original countdown time
	float GetCountdownDuration( void ) const
	{
		return (m_timestamp > 0.0f) ? m_duration : 0.0f;
	}

	/// 1.0 for newly started, 0.0 for elapsed
	float GetRemainingRatio( void ) const
	{
		if ( HasStarted() )
		{
			float left = GetRemainingTime() / m_duration;
			if ( left < 0.0f )
				return 0.0f;
			if ( left > 1.0f )
				return 1.0f;
			return left;
		}

		return 0.0f;
	}

private:
	CNetworkVar( float, m_duration );
	CNetworkVar( float, m_timestamp );
	float Now( void ) const				{ return gpGlobals->curtime; }
};