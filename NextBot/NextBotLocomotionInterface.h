// NextBotLocomotionInterface.h
// NextBot interface for movement through the environment
// Author: Michael Booth, April 2005
// Copyright (c) 2005 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_LOCOMOTION_INTERFACE_H_
#define _NEXT_BOT_LOCOMOTION_INTERFACE_H_

#include "NextBotComponentInterface.h"
#include "countdown_timer.h"
#include "interval_timer.h"

class Path;
class INextBot;
class CNavLadder;

//----------------------------------------------------------------------------------------------------------------
/**
 * The interface encapsulating *how* a bot moves through the world (walking? flying? etc)
 */
class ILocomotion : public INextBotComponent
{
public:
	ILocomotion( INextBot *bot );
	virtual ~ILocomotion() = 0;

	virtual void Reset( void ) = 0;								// (EXTEND) reset to initial state
	virtual void Update( void ) = 0;							// (EXTEND) update internal state

	//
	// The primary locomotive method
	// Depending on the physics of the bot's motion, it may not actually
	// reach the given position precisely.
	// The 'weight' can be used to combine multiple Approach() calls within
	// a single frame into a single goal (ie: weighted average)
	//
	virtual void Approach( const Vector &goalPos, float goalWeight = 1.0f ) = 0;	// (EXTEND) move directly towards the given position

	//
	// Move the bot to the precise given position immediately,
	// updating internal state as needed
	// Collision resolution is done to prevent interpenetration, which may prevent
	// the bot from reaching the given position. If no collisions occur, the
	// bot will be at the given position when this method returns.
	//
	virtual void DriveTo( const Vector &pos ) = 0;				// (EXTEND) Move the bot to the precise given position immediately,

	//
	// Locomotion modifiers
	//
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) { return true; }	// initiate a jump to an adjacent high ledge, return false if climb can't start
	virtual void JumpAcrossGap( const Vector &landingGoal, const Vector &landingForward ) { }	// initiate a jump across an empty volume of space to far side
	virtual void Jump( void ) { }							// initiate a simple undirected jump in the air
	virtual bool IsClimbingOrJumping( void ) const = 0;			// is jumping in any form
	virtual bool IsClimbingUpToLedge( void ) const = 0;			// is climbing up to a high ledge
	virtual bool IsJumpingAcrossGap( void ) const = 0;			// is jumping across a gap to the far side
	virtual bool IsScrambling( void ) const = 0;				// is in the middle of a complex action (climbing a ladder, climbing a ledge, jumping, etc) that shouldn't be interrupted

	virtual void Run( void ) { }							// set desired movement speed to running
	virtual void Walk( void ) { }							// set desired movement speed to walking
	virtual void Stop( void ) { }							// set desired movement speed to stopped
	virtual bool IsRunning( void ) const = 0;
	virtual void SetDesiredSpeed( float speed ) { }			// set desired speed for locomotor movement
	virtual float GetDesiredSpeed( void ) const = 0;			// returns the current desired speed

	virtual void SetSpeedLimit( float speed ) { }					// set maximum speed bot can reach, regardless of desired speed
	virtual float GetSpeedLimit( void ) const { return 1000.0f; }	// get maximum speed bot can reach, regardless of desired speed

	virtual bool IsOnGround( void ) const = 0;					// return true if standing on something
	virtual void OnLeaveGround( CBaseEntity *ground ) { }	// invoked when bot leaves ground for any reason
	virtual void OnLandOnGround( CBaseEntity *ground ) { }	// invoked when bot lands on the ground after being in the air
	virtual CBaseEntity *GetGround( void ) const = 0;			// return the current ground entity or NULL if not on the ground
	virtual const Vector &GetGroundNormal( void ) const = 0;	// surface normal of the ground we are in contact with
	virtual float GetGroundSpeed( void ) const = 0;				// return current world space speed in XY plane
	virtual const Vector &GetGroundMotionVector( void ) const = 0;	// return unit vector in XY plane describing our direction of motion - even if we are currently not moving

	virtual void ClimbLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) { }		// climb the given ladder to the top and dismount
	virtual void DescendLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) { }	// descend the given ladder to the bottom and dismount
	virtual bool IsUsingLadder( void ) const = 0;				// we are moving to get on, ascending/descending, and/or dismounting a ladder
	virtual bool IsAscendingOrDescendingLadder( void ) const = 0;	// we are actually on the ladder right now, either climbing up or down
	virtual bool IsAbleToAutoCenterOnLadder( void ) const { return false; }

	virtual void FaceTowards( const Vector &target ) { }	// rotate body to face towards "target"

	virtual void SetDesiredLean( const QAngle &lean ) { }
	virtual const QAngle &GetDesiredLean( void ) const = 0;


	//
	// Locomotion information
	//
#if SOURCE_ENGINE != SE_LEFT4DEAD && SOURCE_ENGINE != SE_LEFT4DEAD2
	virtual bool IsAbleToJumpAcrossGaps( void ) const = 0;		// return true if this bot can jump across gaps in its path
	virtual bool IsAbleToClimb( void ) const = 0;				// return true if this bot can climb arbitrary geometry it encounters
#endif
	virtual const Vector &GetFeet( void ) const = 0;			// return position of "feet" - the driving point where the bot contacts the ground

	virtual float GetStepHeight( void ) const = 0;				// if delta Z is greater than this, we have to jump to get up
	virtual float GetMaxJumpHeight( void ) const = 0;			// return maximum height of a jump
	virtual float GetDeathDropHeight( void ) const = 0;			// distance at which we will die if we fall

	virtual float GetRunSpeed( void ) const = 0;				// get maximum running speed
	virtual float GetWalkSpeed( void ) const = 0;				// get maximum walking speed
#if SOURCE_ENGINE != SE_LEFT4DEAD2
	virtual float GetMaxAcceleration( void ) const = 0;			// return maximum acceleration of locomotor
	virtual float GetMaxDeceleration( void ) const = 0;			// return maximum deceleration of locomotor
#endif
	virtual const Vector &GetVelocity( void ) const = 0;		// return current world space velocity
	virtual float GetSpeed( void ) const = 0;					// return current world space speed (magnitude of velocity)
	virtual const Vector &GetMotionVector( void ) const = 0;	// return unit vector describing our direction of motion - even if we are currently not moving

	virtual bool IsAreaTraversable( const CNavArea *baseArea ) const = 0;	// return true if given area can be used for navigation

	virtual float GetTraversableSlopeLimit( void ) const = 0;	// return Z component of unit normal of steepest traversable slope

	// return true if the given entity can be ignored during locomotion
	enum TraverseWhenType
	{
		IMMEDIATELY,		// the entity will not block our motion - we'll carry right through
		EVENTUALLY			// the entity will block us until we spend effort to open/destroy it
	};

	/**
	 * Return true if this locomotor could potentially move along the line given.
	 * If false is returned, fraction of walkable ray is returned in 'fraction'
	 */
	virtual bool IsPotentiallyTraversable( const Vector &from, const Vector &to, TraverseWhenType when = EVENTUALLY, float *fraction = NULL ) const = 0;

	/**
	 * Return true if there is a possible "gap" that will need to be jumped over
	 * If true is returned, fraction of ray before gap is returned in 'fraction'
	 */
	virtual bool HasPotentialGap( const Vector &from, const Vector &to, float *fraction = NULL ) const = 0;

	// return true if there is a "gap" here when moving in the given direction
	virtual bool IsGap( const Vector &pos, const Vector &forward ) const = 0;

	virtual bool IsEntityTraversable( CBaseEntity *obstacle, TraverseWhenType when = EVENTUALLY ) const = 0;

	//
	// Stuck state.  If the locomotor cannot make progress, it becomes "stuck" and can only leave
	// this stuck state by successfully moving and becoming un-stuck.
	//
	virtual bool IsStuck( void ) const = 0;				// return true if bot is stuck
	virtual float GetStuckDuration( void ) const = 0;	// return how long we've been stuck
	virtual void ClearStuckStatus( const char *reason = "" ) = 0;	// reset stuck status to un-stuck

	virtual bool IsAttemptingToMove( void ) const = 0;	// return true if we have tried to Approach() or DriveTo() very recently
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual const Vector &GetLastApproachPosition( void ) const = 0;
#endif
protected:
	virtual void AdjustPosture( const Vector &moveGoal ) = 0;
	virtual void StuckMonitor( void ) = 0;

private:
	Vector m_motionVector;
	Vector m_groundMotionVector;
	float m_speed;
	float m_groundSpeed;

	// stuck monitoring
	bool m_isStuck;									// if true, we are stuck
	IntervalTimer m_stuckTimer;						// how long we've been stuck
	CountdownTimer m_stillStuckTimer;				// for resending stuck events
	Vector m_stuckPos;								// where we got stuck
	IntervalTimer m_moveRequestTimer;
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	Vector m_goalPos;
#endif
};

#endif // _NEXT_BOT_LOCOMOTION_INTERFACE_H_