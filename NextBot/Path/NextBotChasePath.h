// NextBotChasePath.h
// Maintain and follow a "chase path" to a selected Actor
// Author: Michael Booth, September 2006
// Copyright (c) 2006 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_CHASE_PATH_
#define _NEXT_BOT_CHASE_PATH_

#include "nav.h"
#include "../NextBotInterface.h"
#include "NextBotPathFollow.h"
#include "NextBotPath.h"
#include "countdown_timer.h"
#include <ehandle.h>

//----------------------------------------------------------------------------------------------
/**
 * A ChasePath extends a PathFollower to periodically recompute a path to a chase
 * subject, and to move along the path towards that subject.
 */
class ChasePath : public PathFollower
{
public:
	enum SubjectChaseType
	{
		LEAD_SUBJECT,
		DONT_LEAD_SUBJECT
	};
	ChasePath( SubjectChaseType chaseHow = DONT_LEAD_SUBJECT );

	virtual ~ChasePath() { }

	virtual void Update( INextBot *bot, CBaseEntity *subject, const IPathCost &cost, Vector *pPredictedSubjectPos = NULL ) = 0;	// update path to chase target and move bot along path

	virtual float GetLeadRadius( void ) const = 0;			// range where movement leading begins - beyond this just head right for the subject
	virtual float GetMaxPathLength( void ) const = 0;		// return maximum path length
	virtual Vector PredictSubjectPosition( INextBot *bot, CBaseEntity *subject ) const = 0;	// try to cutoff our chase subject, knowing our relative positions and velocities
	virtual bool IsRepathNeeded( INextBot *bot, CBaseEntity *subject ) const = 0;			// return true if situation has changed enough to warrant recomputing the current path

	virtual float GetLifetime( void ) const = 0;			// Return duration this path is valid. Path will become invalid at its earliest opportunity once this duration elapses. Zero = infinite lifetime

	virtual void Invalidate( void ) = 0;					// (EXTEND) cause the path to become invalid

private:
	CountdownTimer m_failTimer;							// throttle re-pathing if last path attempt failed
	CountdownTimer m_throttleTimer;						// require a minimum time between re-paths
	CountdownTimer m_lifetimeTimer;
	EHANDLE m_lastPathSubject;							// the subject used to compute the current/last path
	SubjectChaseType m_chaseHow;
};

#endif // _NEXT_BOT_CHASE_PATH_