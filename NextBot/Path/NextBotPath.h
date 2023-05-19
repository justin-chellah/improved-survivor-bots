// NextBotPath.h
// Encapsulate and manipulate a path through the world
// Author: Michael Booth, February 2006
// Copyright (c) 2006 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_PATH_H_
#define _NEXT_BOT_PATH_H_

#include "../NextBotInterface.h"
#include "nav.h"
#include "interval_timer.h"

class INextBot;
class CNavArea;
class CNavLadder;
class CFuncElevator;

//---------------------------------------------------------------------------------------------------------------
/**
 * The interface for pathfinding costs.
 * TODO: Replace all template cost functors with this interface, so we can virtualize and derive from them.
 */
class IPathCost
{
public:
	virtual float operator()( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder, const CFuncElevator *elevator, float length ) const = 0;
};


//---------------------------------------------------------------------------------------------------------------
/**
 * The interface for selecting a goal area during "open goal" pathfinding
 */
class IPathOpenGoalSelector
{
public:
	// compare "newArea" to "currentGoal" and return the area that is the better goal area
	virtual CNavArea *operator() ( CNavArea *currentGoal, CNavArea *newArea ) const = 0;
};


//---------------------------------------------------------------------------------------------------------------
/**
 * A Path through the world.
 * Not only does this encapsulate a path to get from point A to point B,
 * but also the selecting the decision algorithm for how to build that path.
 */
class Path
{
public:
	Path( void );
	virtual ~Path() { }

	enum SegmentType
	{
		ON_GROUND,
		DROP_DOWN,
		CLIMB_UP,
		JUMP_OVER_GAP,
		LADDER_UP,
		LADDER_DOWN,

		NUM_SEGMENT_TYPES
	};

	// @todo Allow custom Segment classes for different kinds of paths
	struct Segment
	{
		CNavArea *area;									// the area along the path
		NavTraverseType how;							// how to enter this area from the previous one
		Vector pos;										// our movement goal position at this point in the path
		const CNavLadder *ladder;						// if "how" refers to a ladder, this is it

		SegmentType type;								// how to traverse this segment of the path
		Vector forward;									// unit vector along segment
		float length;									// length of this segment
		float distanceFromStart;						// distance of this node from the start of the path
		float curvature;								// how much the path 'curves' at this point in the XY plane (0 = none, 1 = 180 degree doubleback)

		Vector m_portalCenter;							// position of center of 'portal' between previous area and this area
		float m_portalHalfWidth;						// half width of 'portal'
	};

	virtual float GetLength( void ) const = 0;						// return length of path from start to finish
	virtual const Vector &GetPosition( float distanceFromStart, const Segment *start = NULL ) const = 0;	// return a position on the path at the given distance from the path start
	virtual const Vector &GetClosestPosition( const Vector &pos, const Segment *start = NULL, float alongLimit = 0.0f ) const = 0;		// return the closest point on the path to the given position

	virtual const Vector &GetStartPosition( void ) const = 0;	// return the position where this path starts
	virtual const Vector &GetEndPosition( void ) const = 0;		// return the position where this path ends
	virtual CBaseCombatCharacter *GetSubject( void ) const = 0;	// return the actor this path leads to, or NULL if there is no subject
#if SOURCE_ENGINE == SE_LEFT4DEAD2
    virtual void SetSubject( CBaseEntity *subject ) = 0;
#endif
	virtual const Path::Segment *GetCurrentGoal( void ) const = 0;	// return current goal along the path we are trying to reach

	virtual float GetAge( void ) const = 0;					// return "age" of this path (time since it was built)

	enum SeekType
	{
		SEEK_ENTIRE_PATH,			// search the entire path length
		SEEK_AHEAD,					// search from current cursor position forward toward end of path
		SEEK_BEHIND					// search from current cursor position backward toward path start
	};
	virtual void MoveCursorToClosestPosition( const Vector &pos, SeekType type = SEEK_ENTIRE_PATH, float alongLimit = 0.0f ) const = 0;		// Set cursor position to closest point on path to given position

	enum MoveCursorType
	{
		PATH_ABSOLUTE_DISTANCE,
		PATH_RELATIVE_DISTANCE
	};
	virtual void MoveCursorToStart( void ) = 0;				// set seek cursor to start of path
	virtual void MoveCursorToEnd( void ) = 0;				// set seek cursor to end of path
	virtual void MoveCursor( float value, MoveCursorType type = PATH_ABSOLUTE_DISTANCE ) = 0;	// change seek cursor position
	virtual float GetCursorPosition( void ) const = 0;		// return position of seek cursor (distance along path)

	struct Data
	{
		Vector pos;										// the position along the path
		Vector forward;									// unit vector along path direction
		float curvature;								// how much the path 'curves' at this point in the XY plane (0 = none, 1 = 180 degree doubleback)
		const Segment *segmentPrior;					// the segment just before this position
	};
	virtual const Data &GetCursorData( void ) const = 0;	// return path state at the current cursor position

	virtual bool IsValid( void ) const = 0;
	virtual void Invalidate( void ) = 0;					// make path invalid (clear it)

	virtual void Draw( const Path::Segment *start = NULL ) const = 0;	// draw the path for debugging
	virtual void DrawInterpolated( float from, float to ) = 0;	// draw the path for debugging - MODIFIES cursor position

	virtual const Segment *FirstSegment( void ) const = 0;	// return first segment of path
	virtual const Segment *NextSegment( const Segment *currentSegment ) const = 0;	// return next segment of path, given current one
	virtual const Segment *PriorSegment( const Segment *currentSegment ) const = 0;	// return previous segment of path, given current one
	virtual const Segment *LastSegment( void ) const = 0;	// return last segment of path

	enum ResultType
	{
		COMPLETE_PATH,
		PARTIAL_PATH,
		NO_PATH
	};
	virtual void OnPathChanged( INextBot *bot, ResultType result ) { }		// invoked when the path is (re)computed (path is valid at the time of this call)

	virtual void Copy( INextBot *bot, const Path &path ) = 0;	// Replace this path with the given path's data
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	//-----------------------------------------------------------------------------------------------------------------
	/**
	 * Build a path from bot's current location to an undetermined goal area
	 * that minimizes the given cost along the final path and meets the
	 * goal criteria.
	 */
	virtual bool ComputeWithOpenGoal( INextBot *bot, const IPathCost &costFunc, const IPathOpenGoalSelector &goalSelector, float maxSearchRadius = 0.0f ) = 0;
#endif
	/**
	 * Determine exactly where the path goes between the given two areas
	 * on the path. Return this point in 'crossPos'.
	 */
	virtual void ComputeAreaCrossing( INextBot *bot, const CNavArea *from, const Vector &fromPos, const CNavArea *to, NavDirType dir, Vector *crossPos ) const = 0;

private:
	enum { MAX_PATH_SEGMENTS = 256 };
	Segment m_path[ MAX_PATH_SEGMENTS ];
	int m_segmentCount;

	mutable Vector m_pathPos;								// used by GetPosition()
	mutable Vector m_closePos;								// used by GetClosestPosition()

	mutable float m_cursorPos;					// current cursor position (distance along path)
	mutable Data m_cursorData;					// used by GetCursorData()
	mutable bool m_isCursorDataDirty;

	IntervalTimer m_ageTimer;					// how old is this path?
	CHandle< CBaseCombatCharacter > m_subject;	// the subject this path leads to

	enum { MAX_ADJ_AREAS = 64 };

	struct AdjInfo
	{
		CNavArea *area;
		CNavLadder *ladder;
		NavTraverseType how;
	};

	AdjInfo m_adjAreaVector[ MAX_ADJ_AREAS ];
	int m_adjAreaIndex;

};

#endif	// _NEXT_BOT_PATH_H_
