// NextBotPathFollow.h
// Path following
// Author: Michael Booth, April 2005
// Copyright (c) 2005 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_PATH_FOLLOWER_
#define _NEXT_BOT_PATH_FOLLOWER_

#include "NextBotPath.h"

class INextBot;
class ILocomotion;


//--------------------------------------------------------------------------------------------------------
/**
 * A PathFollower extends a Path to include mechanisms to move along (follow) it
 */
class PathFollower : public Path
{
public:
	PathFollower( void );
	virtual ~PathFollower() = 0;

	virtual void Invalidate( void ) = 0;				// (EXTEND) cause the path to become invalid
	virtual void Draw( const Path::Segment *start = NULL ) const = 0;	// (EXTEND) draw the path for debugging
	virtual void OnPathChanged( INextBot *bot, Path::ResultType result ) = 0;	// invoked when the path is (re)computed (path is valid at the time of this call)

	virtual void Update( INextBot *bot ) = 0;			// move bot along path

	virtual const Path::Segment *GetCurrentGoal( void ) const = 0;	// return current goal along the path we are trying to reach

	virtual void SetMinLookAheadDistance( float value ) = 0;		// minimum range movement goal must be along path

	virtual CBaseEntity *GetHindrance( void ) const = 0;			// returns entity that is hindering our progress along the path

	virtual bool IsDiscontinuityAhead( INextBot *bot, Path::SegmentType type, float range = -1.0f ) const = 0;	// return true if there is a the given discontinuity ahead in the path within the given range (-1 = entire remaining path)

public:
	const Path::Segment *m_goal;					// our current goal along the path
	float m_minLookAheadRange;

	//bool IsOnStairs( INextBot *bot ) const;		// return true if bot is standing on a stairway
	bool m_isOnStairs;

	CountdownTimer m_avoidTimer;					// do avoid check more often if we recently avoided

	CountdownTimer m_waitTimer;						// for waiting for a blocker to move off our path
	CHandle< CBaseEntity > m_hindrance;

	// debug display data for avoid volumes
	bool m_didAvoidCheck;
	Vector m_leftFrom;
	Vector m_leftTo;
	bool m_isLeftClear;
	Vector m_rightFrom;
	Vector m_rightTo;
	bool m_isRightClear;
	Vector m_hullMin, m_hullMax;
};

#endif // _NEXT_BOT_PATH_FOLLOWER_
