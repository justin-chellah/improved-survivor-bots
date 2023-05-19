#pragma once

#include <networkvar.h>

extern CGlobalVars *gpGlobals;

//--------------------------------------------------------------------------------------------------------------
/**
 * Simple class for tracking intervals of game time.
 * Upon creation, the timer is invalidated.  To measure time intervals, start the timer via Start().
 */
class IntervalTimer
{
public:
#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();
#endif
	DECLARE_CLASS_NOBASE( IntervalTimer );
	DECLARE_EMBEDDED_NETWORKVAR();

	IntervalTimer( void )
	{
		m_timestamp = -1.0f;
	}

	void Reset( void )
	{
		m_timestamp = Now();
	}

	void Start( void )
	{
		m_timestamp = Now();
	}

	void StartFromTime( float startTime )
	{
		m_timestamp = startTime;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	/// if not started, elapsed time is very large
	float GetElapsedTime( void ) const
	{
		return (HasStarted()) ? (Now() - m_timestamp) : 99999.9f;
	}

	bool IsLessThen( float duration ) const
	{
		return (Now() - m_timestamp < duration) ? true : false;
	}

	bool IsGreaterThen( float duration ) const
	{
		return (Now() - m_timestamp > duration) ? true : false;
	}

	float GetStartTime( void ) const
	{
		return m_timestamp;
	}

protected:
	CNetworkVar( float, m_timestamp );
	float Now( void ) const				{ return gpGlobals->curtime; }
};