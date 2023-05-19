// NextBotIntentionInterface.h
// Interface for intentional thinking
// Author: Michael Booth, April 2005
// Copyright (c) 2005 Turtle Rock Studios, Inc. - All Rights Reserved

#ifndef _NEXT_BOT_INTENTION_INTERFACE_H_
#define _NEXT_BOT_INTENTION_INTERFACE_H_

#include "NextBotComponentInterface.h"
#include "NextBotContextualQueryInterface.h"

class INextBot;


//----------------------------------------------------------------------------------------------------------------
/**
 * The interface for intentional thinking.
 * The assumption is that this is a container for one or more concurrent Behaviors.
 * The "primary" Behavior is the FirstContainedResponder, and so on.
 * IContextualQuery requests are prioritized in contained responder order, such that the first responder
 * that returns a definitive answer is accepted.  WITHIN a given responder (ie: a Behavior), the deepest child
 * Behavior in the active stack is asked first, then its parent, and so on, allowing the most specific active
 * Behavior to override the query responses of its more general parent Behaviors.
 */
class IIntention : public INextBotComponent, public IContextualQuery
{
public:
	IIntention( INextBot *bot ) : INextBotComponent( bot ) { }
	virtual ~IIntention() { }

	virtual void Reset( void )  { INextBotComponent::Reset(); }	// reset to initial state
	virtual void Update( void ) { }								// update internal state
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	virtual const char *GetDebugString( void ) const = 0;
#endif
	// IContextualQuery propagation --------------------------------
	virtual QueryResultType			ShouldPickUp( const INextBot *me, CBaseEntity *item ) const = 0;		// if the desired item was available right now, should we pick it up?
	virtual QueryResultType			ShouldHurry( const INextBot *me ) const = 0;							// are we in a hurry?
	virtual QueryResultType			IsHindrance( const INextBot *me, CBaseEntity *blocker ) const = 0;		// return true if we should wait for 'blocker' that is across our path somewhere up ahead.
	virtual Vector					SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const = 0;		// given a subject, return the world space position we should aim at
	virtual QueryResultType			IsPositionAllowed( const INextBot *me, const Vector &pos ) const = 0;	// is the a place we can be?
	virtual const Path *			QueryCurrentPath( const INextBot *me ) const = 0;
	virtual const CBaseCombatCharacter *	SelectMoreDangerousThreat( const INextBot *me,
															   const CBaseCombatCharacter *subject,
															   CBaseCombatCharacter *threat1,
															   CBaseCombatCharacter *threat2 ) const = 0;	// return the more dangerous of the two threats to 'subject', or NULL if we have no opinion
	// NOTE: As further queries are added, update the Behavior class to propagate them
};

#endif // _NEXT_BOT_INTENTION_INTERFACE_H_