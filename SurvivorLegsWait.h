#ifndef _SURVIVOR_LEGS_WAIT_H_
#define _SURVIVOR_LEGS_WAIT_H_

#include <NextBot/NextBotBehavior.h>
#include <countdown_timer.h>

class SurvivorBot;

class SurvivorLegsWait : public Action< SurvivorBot >
{
public:
	SurvivorLegsWait( float duration );
	virtual ~SurvivorLegsWait() { }

	virtual ActionResult< SurvivorBot > OnStart( SurvivorBot *me, Action< SurvivorBot > *priorAction );
	virtual ActionResult< SurvivorBot > OnSuspend( SurvivorBot *me, Action< SurvivorBot > *interruptingAction );
	virtual ActionResult< SurvivorBot > Update( SurvivorBot *me, float interval );

	virtual EventDesiredResult< SurvivorBot > OnCommandPause( SurvivorBot *me, float duration );
	virtual EventDesiredResult< SurvivorBot > OnCommandResume( SurvivorBot *me );

	virtual const char *GetName() const		{ return "[" SMEXT_CONF_LOGTAG "] SurvivorLegsWait"; }

private:
	float m_flDuration;
	CountdownTimer m_waitTimer;
};

inline SurvivorLegsWait::SurvivorLegsWait( float duration = 0.0f )
{
	m_flDuration = duration;
}

inline ActionResult< SurvivorBot > SurvivorLegsWait::OnStart( SurvivorBot *me, Action< SurvivorBot > *priorAction )
{
	if (m_flDuration <= 0.0f)
	{
		m_flDuration = 10.0f;
	}

	m_waitTimer.Start(m_flDuration);

	return Continue();
}

inline ActionResult< SurvivorBot > SurvivorLegsWait::OnSuspend( SurvivorBot *me, Action< SurvivorBot > *interruptingAction )
{
	return Done();
}

inline ActionResult< SurvivorBot > SurvivorLegsWait::Update( SurvivorBot *me, float interval )
{
	if (m_waitTimer.HasStarted() && m_waitTimer.IsElapsed())
	{
		return Done("[" SMEXT_CONF_LOGTAG "] Wait duration elapsed");
	}

	return Continue();
}

inline EventDesiredResult< SurvivorBot > SurvivorLegsWait::OnCommandPause( SurvivorBot *me, float duration )
{
	if (duration > m_waitTimer.GetRemainingTime())
	{
		m_waitTimer.Start(duration);
	}

	return TryToSustain();
}

inline EventDesiredResult< SurvivorBot > SurvivorLegsWait::OnCommandResume( SurvivorBot *me )
{
	return TryDone(RESULT_CRITICAL);
}

#endif // _SURVIVOR_LEGS_WAIT_H_