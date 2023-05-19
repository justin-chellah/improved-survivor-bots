// NextBotVisionInterface.h
// Visual information query interface for bots
// Author: Michael Booth, April 2005
// Copyright (c) 2005 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_VISION_INTERFACE_H_
#define _NEXT_BOT_VISION_INTERFACE_H_

#include "NextBotComponentInterface.h"
#include <shareddefs.h>
#include "countdown_timer.h"
#include "interval_timer.h"

class IBody;
class INextBotEntityFilter;
class CKnownEntity;
class RecognizedActor;

//----------------------------------------------------------------------------------------------------------------
/**
 * The interface for HOW the bot sees (near sighted? night vision? etc)
 */
class IVision : public INextBotComponent
{
public:
	IVision( INextBot *bot );
	virtual ~IVision() { }

	virtual void Reset( void ) = 0;									// reset to initial state
	virtual void Update( void ) = 0;								// update internal state

	//-- attention/short term memory interface follows ------------------------------------------
#if SOURCE_ENGINE != SE_LEFT4DEAD && SOURCE_ENGINE != SE_LEFT4DEAD2
	/**
	 * Iterate each interesting entity we are aware of.
	 * If functor returns false, stop iterating and return false.
	 * NOTE: known.GetEntity() is guaranteed to be non-NULL
	 */
	class IForEachKnownEntity
	{
	public:
		virtual bool Inspect( const CKnownEntity &known ) = 0;
	};
	virtual bool ForEachKnownEntity( IForEachKnownEntity &func );
#endif
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual const RecognizedActor *GetPrimaryRecognizedThreat( void ) const = 0;	// return the biggest threat to ourselves that we are aware of
#else
	virtual const CKnownEntity *GetPrimaryKnownThreat( bool onlyVisibleThreats = false ) const;	// return the biggest threat to ourselves that we are aware of
#endif
	virtual float GetTimeSinceVisible( int team ) const = 0;				// return time since we saw any member of the given team
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual const RecognizedActor *GetClosestRecognized( int team = TEAM_ANY ) const = 0;	// return the closest known entity
	virtual int GetRecognizedCount( int team, float rangeLimit = -1.0f ) const = 0;		// return the number of entities on the given team known to us closer than rangeLimit

	virtual const RecognizedActor *GetClosestRecognized( const INextBotEntityFilter &filter ) const = 0;	// return the closest known entity that passes the given filter
#else
	virtual const CKnownEntity *GetClosestKnown( int team = TEAM_ANY ) const = 0;	// return the closest known entity
	virtual int GetKnownCount( int team, bool onlyVisible = false, float rangeLimit = -1.0f ) const = 0;		// return the number of entities on the given team known to us closer than rangeLimit

	virtual const CKnownEntity *GetClosestKnown( const INextBotEntityFilter &filter ) const = 0;	// return the closest known entity that passes the given filter

	virtual const CKnownEntity *GetKnown( const CBaseEntity *entity ) const = 0;		// given an entity, return our known version of it (or NULL if we don't know of it)

	// Introduce a known entity into the system. Its position is assumed to be known
	// and will be updated, and it is assumed to not yet have been seen by us, allowing for learning
	// of known entities by being told about them, hearing them, etc.
	virtual void AddKnownEntity( CBaseEntity *entity ) = 0;


	//-- physical vision interface follows ------------------------------------------------------

	/**
	 * Populate "potentiallyVisible" with the set of all entities we could potentially see.
	 * Entities in this set will be tested for visibility/recognition in IVision::Update()
	 */
	virtual void CollectPotentiallyVisibleEntities( CUtlVector< CBaseEntity * > *potentiallyVisible ) = 0;
#endif
	virtual float GetMaxVisionRange( void ) const = 0;				// return maximum distance vision can reach
	virtual float GetMinRecognizeTime( void ) const = 0;			// return VISUAL reaction time

	/**
	 * IsAbleToSee() returns true if the viewer can ACTUALLY SEE the subject or position,
	 * taking into account blindness, smoke effects, invisibility, etc.
	 * If 'visibleSpot' is non-NULL, the highest priority spot on the subject that is visible is returned.
	 */
	enum FieldOfViewCheckType { USE_FOV, DISREGARD_FOV };
	virtual bool IsAbleToSee( CBaseEntity *subject, FieldOfViewCheckType checkFOV, Vector *visibleSpot = NULL ) const = 0;
	virtual bool IsAbleToSee( const Vector &pos, FieldOfViewCheckType checkFOV ) const = 0;

	virtual bool IsNoticed( CBaseEntity *subject ) const = 0;		// return true if we 'notice' the subject, even if we have clear LOS
	virtual bool IsIgnored( CBaseEntity *subject ) const = 0;		// return true to completely ignore this entity

	/**
	 * Check if 'subject' is within the viewer's field of view
	 */
	virtual bool IsInFieldOfView( const Vector &pos ) const = 0;
	virtual bool IsInFieldOfView( CBaseEntity *subject ) const = 0;
	virtual float GetDefaultFieldOfView( void ) const = 0;			// return default FOV in degrees
	virtual float GetFieldOfView( void ) const = 0;					// return FOV in degrees
	virtual void SetFieldOfView( float horizAngle ) = 0;			// angle given in degrees

	virtual bool IsLineOfSightClear( const Vector &pos ) const = 0;	// return true if the ray to the given point is unobstructed

	/**
	 * Returns true if the ray between the position and the subject is unobstructed.
	 * A visible spot on the subject is returned in 'visibleSpot'.
	 */
	virtual bool IsLineOfSightClearToEntity( const CBaseEntity *subject, Vector *visibleSpot = NULL ) const = 0;

	/// @todo: Implement LookAt system
	virtual bool IsLookingAt( const Vector &pos, float cosTolerance = 0.95f ) const = 0;					// are we looking at the given position
	virtual bool IsLookingAt( const CBaseCombatCharacter *actor, float cosTolerance = 0.95f ) const = 0;	// are we looking at the given actor

private:
	CountdownTimer m_scanTimer;			// for throttling update rate

	float m_FOV;						// current FOV in degrees
	float m_cosHalfFOV;					// the cosine of FOV/2
#if SOURCE_ENGINE == SE_LEFT4DEAD || SOURCE_ENGINE == SE_LEFT4DEAD2
	CUtlVector< RecognizedActor > m_knownEntityVector;		// the set of enemies/friends we are aware of
#else
	CUtlVector< CKnownEntity > m_knownEntityVector;		// the set of enemies/friends we are aware of
#endif
	float m_lastVisionUpdateTimestamp;
	IntervalTimer m_notVisibleTimer[ MAX_TEAMS ];		// for tracking interval since last saw a member of the given team
};

#endif // _NEXT_BOT_VISION_INTERFACE_H_