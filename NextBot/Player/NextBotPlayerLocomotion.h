// NextBotPlayerLocomotion.h
// Locomotor for CBasePlayer derived bots
// Author: Michael Booth, November 2005
// Copyright (c) 2005 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_PLAYER_LOCOMOTION_H_
#define _NEXT_BOT_PLAYER_LOCOMOTION_H_

#include "../NextBotLocomotionInterface.h"
#include "../Path/NextBotPathFollow.h"

class CBasePlayer;

//--------------------------------------------------------------------------------------------------
/**
 * Basic player locomotion implementation
 */
class PlayerLocomotion : public ILocomotion
{
public:
	DECLARE_CLASS( PlayerLocomotion, ILocomotion );

	PlayerLocomotion( INextBot *bot );
	virtual ~PlayerLocomotion() { }

	virtual void Reset( void ) = 0;							// reset to initial state
	virtual void Update( void ) = 0;						// update internal state

	virtual void Approach( const Vector &pos, float goalWeight = 1.0f ) = 0;	// move directly towards the given position
	virtual void DriveTo( const Vector &pos ) = 0;			// Move the bot to the precise given position immediately,

	//
	// ILocomotion modifiers
	//
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) = 0;	// initiate a jump to an adjacent high ledge, return false if climb can't start
	virtual void JumpAcrossGap( const Vector &landingGoal, const Vector &landingForward ) = 0;	// initiate a jump across an empty volume of space to far side
	virtual void Jump( void ) = 0;								// initiate a simple undirected jump in the air
	virtual bool IsClimbingOrJumping( void ) const = 0;					// is jumping in any form
	virtual bool IsClimbingUpToLedge( void ) const = 0;			// is climbing up to a high ledge
	virtual bool IsJumpingAcrossGap( void ) const = 0;			// is jumping across a gap to the far side

	virtual void Run( void ) = 0;								// set desired movement speed to running
	virtual void Walk( void ) = 0;								// set desired movement speed to walking
	virtual void Stop( void ) = 0;								// set desired movement speed to stopped
	virtual bool IsRunning( void ) const = 0;
	virtual void SetDesiredSpeed( float speed ) = 0;			// set desired speed for locomotor movement
	virtual float GetDesiredSpeed( void ) const = 0;			// returns the current desired speed
	virtual void SetMinimumSpeedLimit( float limit ) = 0;		// speed cannot drop below this
	virtual void SetMaximumSpeedLimit( float limit ) = 0;		// speed cannot rise above this

	virtual bool IsOnGround( void ) const = 0;					// return true if standing on something
	virtual CBaseEntity *GetGround( void ) const = 0;			// return the current ground entity or NULL if not on the ground
	virtual const Vector &GetGroundNormal( void ) const = 0;	// surface normal of the ground we are in contact with

	virtual void ClimbLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;		// climb the given ladder to the top and dismount
	virtual void DescendLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;		// descend the given ladder to the bottom and dismount
	virtual bool IsUsingLadder( void ) const = 0;
	virtual bool IsAscendingOrDescendingLadder( void ) const = 0;	// we are actually on the ladder right now, either climbing up or down
	virtual bool IsAbleToAutoCenterOnLadder( void ) const = 0;

	virtual void FaceTowards( const Vector &target ) = 0;		// rotate body to face towards "target"

	virtual void SetDesiredLean( const QAngle &lean )	{ }
	virtual const QAngle &GetDesiredLean( void ) const	{ static QAngle junk; return junk; }

	//
	// ILocomotion information
	//
	virtual const Vector &GetFeet( void ) const = 0;			// return position of "feet" - point below centroid of bot at feet level

	virtual float GetStepHeight( void ) const = 0;				// if delta Z is greater than this, we have to jump to get up
	virtual float GetMaxJumpHeight( void ) const = 0;			// return maximum height of a jump
	virtual float GetDeathDropHeight( void ) const = 0;			// distance at which we will die if we fall

	virtual float GetRunSpeed( void ) const = 0;				// get maximum running speed
	virtual float GetWalkSpeed( void ) const = 0;				// get maximum walking speed

	virtual float GetMaxAcceleration( void ) const = 0;			// return maximum acceleration of locomotor
	virtual float GetMaxDeceleration( void ) const = 0;			// return maximum deceleration of locomotor

	virtual const Vector &GetVelocity( void ) const = 0;		// return current world space velocity

protected:
	virtual void AdjustPosture( const Vector &moveGoal ) = 0;

public:
	CBasePlayer *m_player;									// the player we are locomoting

	mutable bool m_isJumping;
	CountdownTimer m_jumpTimer;

	bool m_isClimbingUpToLedge;
	bool m_isJumpingAcrossGap;
	Vector m_landingGoal;
	bool m_hasLeftTheGround;

	float m_desiredSpeed;
	float m_minSpeedLimit;
	float m_maxSpeedLimit;

	bool TraverseLadder( void );			// when climbing/descending a ladder

	enum LadderState
	{
		NO_LADDER,							// not using a ladder
		APPROACHING_ASCENDING_LADDER,
		APPROACHING_DESCENDING_LADDER,
		ASCENDING_LADDER,
		DESCENDING_LADDER,
		DISMOUNTING_LADDER_TOP,
		DISMOUNTING_LADDER_BOTTOM,
	};

	LadderState m_ladderState;
	LadderState ApproachAscendingLadder( void );
	LadderState ApproachDescendingLadder( void );
	LadderState AscendLadder( void );
	LadderState DescendLadder( void );
	LadderState DismountLadderTop( void );
	LadderState DismountLadderBottom( void );

	const CNavLadder *m_ladderInfo;
	const CNavArea *m_ladderDismountGoal;
	CountdownTimer m_ladderTimer;			// a "give up" timer if things go awry

	bool IsClimbPossible( INextBot *me, const CBaseEntity *obstacle ) const;
};

#endif // _NEXT_BOT_PLAYER_LOCOMOTION_H_