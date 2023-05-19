// NextBotBodyInterface.h
// Control and information about the bot's body state (posture, animation state, etc)
// Author: Michael Booth, April 2006
// Copyright (c) 2006 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_BODY_INTERFACE_H_
#define _NEXT_BOT_BODY_INTERFACE_H_

#include "animation.h"
#include "NextBotComponentInterface.h"
#include <ai_activity.h>

class INextBot;
struct animevent_t;


//----------------------------------------------------------------------------------------------------------------
/**
 * The interface for control and information about the bot's body state (posture, animation state, etc)
 */
class IBody : public INextBotComponent
{
public:
	IBody( INextBot *bot ) : INextBotComponent( bot ) { }
	virtual ~IBody() { }

	virtual void Reset( void ) { INextBotComponent::Reset(); }			// reset to initial state
	virtual void Update( void ) { }										// update internal state

	/**
	 * Move the bot to a new position.
	 * If the body is not currently movable or if it
	 * is in a motion-controlled animation activity
	 * the position will not be changed and false will be returned.
	 */
	virtual bool SetPosition( const Vector &pos ) = 0;

	virtual const Vector &GetEyePosition( void ) const = 0;					// return the eye position of the bot in world coordinates
	virtual const Vector &GetViewVector( void ) const = 0;					// return the view unit direction vector in world coordinates

	enum LookAtPriorityType
	{
		BORING,
		INTERESTING,				// last known enemy location, dangerous sound location
		IMPORTANT,					// a danger
		CRITICAL,					// an active threat to our safety
		MANDATORY					// nothing can interrupt this look at - two simultaneous look ats with this priority is an error
	};
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
	virtual float GetHeadSteadyDuration( void ) const = 0;				// return the duration that the bot's head has not been rotating
	virtual float GetHeadAimSubjectLeadTime( void ) const = 0;			// return how far into the future we should predict our moving subject's position to aim at when tracking subject look-ats

	virtual float GetMaxHeadAngularVelocity( void ) const = 0;			// return max turn rate of head in degrees/second

	enum ActivityType
	{
		MOTION_CONTROLLED_XY	= 0x0001,	// XY position and orientation of the bot is driven by the animation.
		MOTION_CONTROLLED_Z		= 0x0002,	// Z position of the bot is driven by the animation.
		ACTIVITY_UNINTERRUPTIBLE= 0x0004,	// activity can't be changed until animation finishes
		ACTIVITY_TRANSITORY		= 0x0008,	// a short animation that takes over from the underlying animation momentarily, resuming it upon completion
		ENTINDEX_PLAYBACK_RATE	= 0x0010,	// played back at different rates based on entindex
	};

	/**
	 * Begin an animation activity, return false if we cant do that right now.
	 */
	virtual bool StartActivity( Activity act, unsigned int flags = 0 ) = 0;
	virtual int SelectAnimationSequence( Activity act ) const = 0;			// given an Activity, select and return a specific animation sequence within it

	virtual Activity GetActivity( void ) const = 0;							// return currently animating activity
	virtual bool IsActivity( Activity act ) const = 0;						// return true if currently animating activity matches the given one
	virtual bool HasActivityType( unsigned int flags ) const = 0;			// return true if currently animating activity has any of the given flags

	enum PostureType
	{
		STAND,
		CROUCH,
		SIT,
		CRAWL,
		LIE
	};
	virtual void SetDesiredPosture( PostureType posture ) { }			// request a posture change
	virtual PostureType GetDesiredPosture( void ) const = 0;				// get posture body is trying to assume
	virtual bool IsDesiredPosture( PostureType posture ) const = 0;			// return true if body is trying to assume this posture
	virtual bool IsInDesiredPosture( void ) const = 0;						// return true if body's actual posture matches its desired posture

	virtual PostureType GetActualPosture( void ) const = 0;					// return body's current actual posture
	virtual bool IsActualPosture( PostureType posture ) const = 0;			// return true if body is actually in the given posture

	virtual bool IsPostureMobile( void ) const = 0;							// return true if body's current posture allows it to move around the world
	virtual bool IsPostureChanging( void ) const = 0;						// return true if body's posture is in the process of changing to new posture


	/**
	 * "Arousal" is the level of excitedness/arousal/anxiety of the body.
	 * Is changes instantaneously to avoid complex interactions with posture transitions.
	 */
	enum ArousalType
	{
		NEUTRAL,
		ALERT,
		INTENSE
	};
	virtual void SetArousal( ArousalType arousal ) { }					// arousal level change
	virtual ArousalType GetArousal( void ) const = 0;						// get arousal level
	virtual bool IsArousal( ArousalType arousal ) const = 0;				// return true if body is at this arousal level


	virtual float GetHullWidth( void ) const = 0;							// width of bot's collision hull in XY plane
	virtual float GetHullHeight( void ) const = 0;							// height of bot's current collision hull based on posture
	virtual float GetStandHullHeight( void ) const = 0;						// height of bot's collision hull when standing
	virtual float GetCrouchHullHeight( void ) const = 0;					// height of bot's collision hull when crouched
	virtual const Vector &GetHullMins( void ) const = 0;					// return current collision hull minimums based on actual body posture
	virtual const Vector &GetHullMaxs( void ) const = 0;					// return current collision hull maximums based on actual body posture

	virtual unsigned int GetSolidMask( void ) const = 0;					// return the bot's collision mask (hack until we get a general hull trace abstraction here or in the locomotion interface)
};

#endif // _NEXT_BOT_BODY_INTERFACE_H_