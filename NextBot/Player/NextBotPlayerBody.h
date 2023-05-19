// NextBotPlayerBody.h
// Control and information about the bot's body state (posture, animation state, etc)
// Author: Michael Booth, October 2006
// Copyright (c) 2006 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_PLAYER_BODY_H_
#define _NEXT_BOT_PLAYER_BODY_H_

#include "NextBot/NextBotBodyInterface.h"
#include "countdown_timer.h"
#include "interval_timer.h"

class CBasePlayer;

//----------------------------------------------------------------------------------------------------------------
/**
 * A useful reply for IBody::AimHeadTowards.  When the
 * head is aiming on target, press the fire button.
 */
class PressFireButtonReply : public INextBotReply
{
public:
	virtual void OnSuccess( INextBot *bot );	// invoked when process completed successfully
};


//----------------------------------------------------------------------------------------------------------------
/**
 * A useful reply for IBody::AimHeadTowards.  When the
 * head is aiming on target, press the alt-fire button.
 */
class PressAltFireButtonReply : public INextBotReply
{
public:
	virtual void OnSuccess( INextBot *bot );	// invoked when process completed successfully
};


//----------------------------------------------------------------------------------------------------------------
/**
 * A useful reply for IBody::AimHeadTowards.  When the
 * head is aiming on target, press the jump button.
 */
class PressJumpButtonReply : public INextBotReply
{
public:
	virtual void OnSuccess( INextBot *bot );	// invoked when process completed successfully
};


//----------------------------------------------------------------------------------------------------------------
/**
 * The interface for control and information about the bot's body state (posture, animation state, etc)
 */
class PlayerBody : public IBody
{
public:
	PlayerBody( INextBot *bot );
	virtual ~PlayerBody() = 0;

	virtual void Reset( void ) = 0;										// reset to initial state
	virtual void Upkeep( void ) = 0;									// lightweight update guaranteed to occur every server tick

	virtual bool SetPosition( const Vector &pos ) = 0;

	virtual const Vector &GetEyePosition( void ) const = 0;				// return the eye position of the bot in world coordinates
	virtual const Vector &GetViewVector( void ) const = 0;				// return the view unit direction vector in world coordinates
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual void AimHeadTowards( const Vector &lookAtPos,
								 LookAtPriorityType priority = BORING,
								 float duration = 0.0f,
								 INextBotReply *replyWhenAimed = NULL,
								 const char *reason = NULL,
								 bool mustMaintainHeadSteadiness = true,
								 float easeInTime = 0.25f ) = 0;		// aim the bot's head towards the given goal
#else
	virtual void AimHeadTowards( const Vector &lookAtPos,
								 LookAtPriorityType priority = BORING,
								 float duration = 0.0f,
								 INextBotReply *replyWhenAimed = NULL,
								 const char *reason = NULL ) = 0;		// aim the bot's head towards the given goal
#endif
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual void AimHeadTowards( CBaseEntity *subject,
								 LookAtPriorityType priority = BORING,
								 float duration = 0.0f,
								 INextBotReply *replyWhenAimed = NULL,
								 const char *reason = NULL,
								 bool mustMaintainHeadSteadiness = true,
								 float easeInTime = 0.25f ) = 0;		// continually aim the bot's head towards the given subject
#else
	virtual void AimHeadTowards( CBaseEntity *subject,
								 LookAtPriorityType priority = BORING,
								 float duration = 0.0f,
								 INextBotReply *replyWhenAimed = NULL,
								 const char *reason = NULL ) = 0;		// continually aim the bot's head towards the given subject
#endif
	virtual bool IsHeadAimingOnTarget( void ) const = 0;				// return true if the bot's head has achieved its most recent lookat target
	virtual bool IsHeadSteady( void ) const = 0;						// return true if head is not rapidly turning to look somewhere else
	virtual float GetHeadSteadyDuration( void ) const = 0;				// return the duration that the bot's head has been on-target

	virtual float GetMaxHeadAngularVelocity( void ) const = 0;			// return max turn rate of head in degrees/second

	virtual bool StartActivity( Activity act, unsigned int flags ) = 0;
	virtual Activity GetActivity( void ) const = 0;						// return currently animating activity
	virtual bool IsActivity( Activity act ) const = 0;					// return true if currently animating activity matches the given one
	virtual bool HasActivityType( unsigned int flags ) const = 0;		// return true if currently animating activity has any of the given flags

	virtual void SetDesiredPosture( PostureType posture ) = 0;			// request a posture change
	virtual PostureType GetDesiredPosture( void ) const = 0;			// get posture body is trying to assume
	virtual bool IsDesiredPosture( PostureType posture ) const = 0;		// return true if body is trying to assume this posture
	virtual bool IsInDesiredPosture( void ) const = 0;					// return true if body's actual posture matches its desired posture

	virtual PostureType GetActualPosture( void ) const = 0;				// return body's current actual posture
	virtual bool IsActualPosture( PostureType posture ) const = 0;		// return true if body is actually in the given posture

	virtual bool IsPostureMobile( void ) const = 0;						// return true if body's current posture allows it to move around the world
	virtual bool IsPostureChanging( void ) const = 0;					// return true if body's posture is in the process of changing to new posture

	virtual void SetArousal( ArousalType arousal ) = 0;					// arousal level change
	virtual ArousalType GetArousal( void ) const = 0;					// get arousal level
	virtual bool IsArousal( ArousalType arousal ) const = 0;			// return true if body is at this arousal level

	virtual float GetHullWidth( void ) const = 0;						// width of bot's collision hull in XY plane
	virtual float GetHullHeight( void ) const = 0;						// height of bot's current collision hull based on posture
	virtual float GetStandHullHeight( void ) const = 0;					// height of bot's collision hull when standing
	virtual float GetCrouchHullHeight( void ) const = 0;				// height of bot's collision hull when crouched
	virtual const Vector &GetHullMins( void ) const = 0;				// return current collision hull minimums based on actual body posture
	virtual const Vector &GetHullMaxs( void ) const = 0;				// return current collision hull maximums based on actual body posture

	virtual unsigned int GetSolidMask( void ) const = 0;				// return the bot's collision mask (hack until we get a general hull trace abstraction here or in the locomotion interface)

	virtual CBaseEntity *GetEntity( void ) = 0;					// get the entity
private:
	CBasePlayer *m_player;

	PostureType m_posture;
	ArousalType m_arousal;

	mutable Vector m_eyePos;			// for use with GetEyePosition() ONLY
	mutable Vector m_viewVector;		// for use with GetViewVector() ONLY
	mutable Vector m_hullMins;			// for use with GetHullMins() ONLY
	mutable Vector m_hullMaxs;			// for use with GetHullMaxs() ONLY

	Vector m_lookAtPos;					// if m_lookAtSubject is non-NULL, it continually overwrites this position with its own
	EHANDLE m_lookAtSubject;

	LookAtPriorityType m_lookAtPriority;
	CountdownTimer m_lookAtExpireTimer;		// how long until this lookat expired
	IntervalTimer m_lookAtDurationTimer;	// how long have we been looking at this target
	INextBotReply *m_lookAtReplyWhenAimed;
	bool m_isSightedIn;					// true if we are looking at our last lookat target
	bool m_hasBeenSightedIn;			// true if we have hit the current lookat target
	float m_yawRate;
	float m_pitchRate;
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	float m_easeInTime;
#endif
	IntervalTimer m_headSteadyTimer;
	QAngle m_priorAngles;				// last update's head angles
	QAngle m_desiredAngles;
};

inline bool PlayerBody::IsHeadAimingOnTarget( void ) const
{
	return m_isSightedIn;
}


#endif // _NEXT_BOT_PLAYER_BODY_H_