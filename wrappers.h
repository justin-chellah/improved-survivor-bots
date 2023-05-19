#pragma once

#include <iplayerinfo.h>
#include <IBinTools.h>
#include "NextBot/NextBotBehavior.h"
#include "NextBot/NextBotInterface.h"
#include "NextBot/NextBotIntentionInterface.h"
#include <NextBot/Path/NextBotPathFollow.h>
#include <in_buttons.h>
#include <toolframework/itoolentity.h>
#include <IStaticPropMgr.h>
#include <IEngineTrace.h>

#define TEAM_ANY				-1	// for some team query methods
#define TEAM_SURVIVOR			2
#define TEAM_ZOMBIE				3

#define DMG_SPIT 				(1 << 10)

#define WEAPON_SLOT_RIFLE		0	// (primary slot)
#define WEAPON_SLOT_PISTOL		1	// (secondary slot)

enum ZombieClassType
{
	Zombie_Common = 0,
	Zombie_Smoker,
	Zombie_Boomer,
	Zombie_Hunter,
	Zombie_Spitter,
	Zombie_Jockey,
	Zombie_Charger,
	Zombie_Witch,
	Zombie_Tank,
	Zombie_Survivor,
};

enum BackpackItemActionType
{
	UseAction_None = 0,
	UseAction_Healing = 1,
	UseAction_Defibrillating = 4,
	UseAction_RevivedGetUp = 5,
	UseAction_UseTimedButton = 10,
	UseAction_UsePointScript = 11,
};

class CBaseCombatCharacter;
class CBaseCombatWeapon;
class CNavArea;
class CNavLadder;
class INextBotEventResponder;
class SurvivorTeamSituation;
class ShortestPathCost;
class CFuncElevator;
class PlayerBody;
class SurvivorBot;
class PlayerLocomotion;

extern IServerGameEnts *serverGameEnts;
extern IBinTools *bintools;
extern IServerTools *servertools;
extern IStaticPropMgrServer *staticpropmgr;

static CBaseEntity *FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);

// Misc useful
inline bool FStrEq(const char *sz1, const char *sz2)
{
	return ( sz1 == sz2 || V_stricmp(sz1, sz2) == 0 );
}

inline CBaseEntity *EntityFromBaseHandle(void *pAddress, int nOffset)
{
	CBaseHandle &hndl = *reinterpret_cast<CBaseHandle *>(reinterpret_cast<byte *>(pAddress) + nOffset);

	edict_t *pEdict = gamehelpers->GetHandleEntity(hndl);

	if (!pEdict)
	{
		return nullptr;
	}

	return serverGameEnts->EdictToBaseEntity(pEdict);
}

static CBaseEntity *EntityFromEntityHandle( IHandleEntity *pHandleEntity )
{
#ifdef CLIENT_DLL
	IClientUnknown *pUnk = (IClientUnknown*)pHandleEntity;
	return pUnk->GetBaseEntity();
#else
#ifndef _X360
	if ( staticpropmgr->IsStaticProp( pHandleEntity ) )
		return nullptr;
#else
	if ( !pHandleEntity || pHandleEntity->m_bIsStaticProp )
		return nullptr;
#endif

	IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
	Assert( !pUnk || pUnk->GetBaseEntity() );
	return pUnk->GetBaseEntity();
#endif
}

class RecognizedActor
{
private:
	CHandle< CBaseCombatCharacter > m_actor;

public:
	CBaseCombatCharacter *GetActor( void )
	{
		return m_actor;
	}
};

class CBaseEntity : public IServerEntity
{
public:
	static int m_nOffset_SendProp_m_flSimulationTime;
	static int m_nOffset_SendProp_m_iTeamNum;
	static int m_nOffset_m_lifeState;

	edict_t *edict( void )
	{
		return serverGameEnts->BaseEntityToEdict(this);
	}

	int entindex( void )
	{
		return gamehelpers->EntityToBCompatRef(this);
	}

	const char *GetClassname( void )
	{
		return gamehelpers->GetEntityClassname(this);
	}

	int GetTeamNumber( void )
	{
		return *reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_iTeamNum);
	}

	float GetSimulationTime( void )
	{
		return *reinterpret_cast<float *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_flSimulationTime);
	}

	const Vector &GetAbsOrigin( void )
	{
		static int nOffset = -1;

		if (nOffset == -1)
		{
			datamap_t *pMap = gamehelpers->GetDataMap(this);

			sm_datatable_info_t info;
			if (gamehelpers->FindDataMapInfo(pMap, "m_vecAbsOrigin", &info))
			{
				nOffset = info.actual_offset;
			}
			else
			{
				smutils->LogError(myself, "Unable to find DataMap info for \"m_vecAbsOrigin\"");
			}
		}

		return *reinterpret_cast<Vector *>(reinterpret_cast<byte *>(this) + nOffset);
	}

	const Vector &GetAbsVelocity( void )
	{
		static int nOffset = -1;

		if (nOffset == -1)
		{
			datamap_t *pMap = gamehelpers->GetDataMap(this);

			sm_datatable_info_t info;
			if (gamehelpers->FindDataMapInfo(pMap, "m_vecAbsVelocity", &info))
			{
				nOffset = info.actual_offset;
			}
			else
			{
				smutils->LogError(myself, "Unable to find DataMap info for \"m_vecAbsVelocity\"");
			}
		}

		return *reinterpret_cast<Vector *>(reinterpret_cast<byte *>(this) + nOffset);
	}

	bool IsAlive( void )
	{
		return *(reinterpret_cast<byte *>(this) + m_nOffset_m_lifeState) == LIFE_ALIVE;
	}
};

inline void SetEntityInBaseHandle(void *pAddress, int nOffset, CBaseEntity *pEntity)
{
	CBaseHandle &hndl = *reinterpret_cast<CBaseHandle *>(reinterpret_cast<byte *>(pAddress) + nOffset);

	if (pEntity)
	{
		gamehelpers->SetHandleEntity(hndl, pEntity->edict());
	}
	else
	{
		hndl = nullptr;
	}
}

class CInferno : public CBaseEntity
{
public:
	static void *m_pFn_IsTouching_VVV;

	static ICallWrapper *m_pCallWrap_IsTouching_VVV;

	bool IsTouching( const Vector &from, const Vector &to, Vector *where = nullptr )
	{
		struct
		{
			CInferno *pInferno;
			const Vector *from;
			const Vector *to;
			Vector *where;
		}
		stStack =
		{
			this,
			&from,
			&to,
			where
		};

		bool bResult;
		m_pCallWrap_IsTouching_VVV->Execute(&stStack, &bResult);

		return bResult;
	}
};

class CBaseCombatCharacter : public CBaseEntity
{
public:
	static int m_nOffset_m_lastKnownArea;

	static int m_nOffset_SendProp_m_iAmmo;
	static int m_nOffset_SendProp_m_hActiveWeapon;

	static int m_iVtbl_GetClass;
	static int m_iVtbl_Weapon_GetSlot;
	static int m_iVtbl_Weapon_Switch;
	static int m_iVtbl_IsIT;

	static ICallWrapper *m_pCallWrap_GetClass;
	static ICallWrapper *m_pCallWrap_Weapon_GetSlot;
	static ICallWrapper *m_pCallWrap_Weapon_Switch;

	CNavArea *GetLastKnownArea( void )
	{
		return *reinterpret_cast<CNavArea **>(reinterpret_cast<byte *>(this) + m_nOffset_m_lastKnownArea);
	}

	int GetAmmoCount( int iAmmo )
	{
		return *reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_iAmmo + (iAmmo * 4));
	}

	ZombieClassType GetClass( void )
	{
		CBaseCombatCharacter *pBCC = this;

		ZombieClassType nZombieClassType;
		m_pCallWrap_GetClass->Execute(&pBCC, &nZombieClassType);

		return nZombieClassType;
	}

	CBaseCombatWeapon *Weapon_GetSlot(int iSlot)
	{
		struct
		{
			CBaseCombatCharacter *pPlayer;
			int iSlot;
		}
		stStack =
		{
			this,
			iSlot
		};

		CBaseCombatWeapon *pWeapon;
		m_pCallWrap_Weapon_GetSlot->Execute(&stStack, &pWeapon);

		return pWeapon;
	}

	CBaseCombatWeapon *GetActiveWeapon( void )
	{
		return reinterpret_cast<CBaseCombatWeapon *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_hActiveWeapon));
	}

	bool Weapon_Switch(CBaseCombatWeapon *pWeapon, int iViewModel)
	{
		struct
		{
			CBaseCombatCharacter *pPlayer;
			CBaseCombatWeapon *pWeapon;
			int iViewModel;
		}
		stStack =
		{
			this,
			pWeapon,
			iViewModel
		};

		bool bWeaponSwitch;
		m_pCallWrap_Weapon_Switch->Execute(&stStack, &bWeaponSwitch);

		return bWeaponSwitch;
	}
};

class CCSPlayer : public CBaseCombatCharacter
{
public:
	static int m_iVtbl_GetHealthBuffer;

	static ICallWrapper *m_pCallWrap_GetHealthBuffer;

	float GetHealthBuffer( void )
	{
		CCSPlayer *pPlayer = this;

		float flHealthBuffer;
		m_pCallWrap_GetHealthBuffer->Execute(&pPlayer, &flHealthBuffer);

		return flHealthBuffer;
	}
};

class CTerrorPlayer : public CCSPlayer
{
public:
	static int m_nOffset_SendProp_m_pummelAttacker;
	static int m_nOffset_SendProp_m_pummelVictim;
	static int m_nOffset_SendProp_m_carryAttacker;
	static int m_nOffset_SendProp_m_pounceAttacker;
	static int m_nOffset_SendProp_m_jockeyAttacker;
	static int m_nOffset_SendProp_m_tongueOwner;
	static int m_nOffset_SendProp_m_reviveTarget;
	static int m_nOffset_SendProp_m_isIncapacitated;
	static int m_nOffset_SendProp_m_bAdrenalineActive;
	static int m_nOffset_SendProp_m_bIsOnThirdStrike;
	static int m_nOffset_SendProp_m_flProgressBarStartTime;
	static int m_nOffset_SendProp_m_flProgressBarDuration;
	static int m_nOffset_SendProp_m_useActionOwner;
	static int m_nOffset_SendProp_m_iCurrentUseAction;
	static int m_nOffset_SendProp_m_reachedTongueOwner;

	static void *m_pFn_StopRevivingSomeone;
	static void *m_pFn_GetTimeSinceAttackedByEnemy;

	static ICallWrapper *m_pCallWrap_StopRevivingSomeone;
	static ICallWrapper *m_pCallWrap_GetTimeSinceAttackedByEnemy;

	CTerrorPlayer *GetPummelAttacker( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_pummelAttacker));
	}

	CTerrorPlayer *GetPummelVictim( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_pummelVictim));
	}

	CTerrorPlayer *GetCarryAttacker( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_carryAttacker));
	}

	CTerrorPlayer *GetPounceAttacker( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_pounceAttacker));
	}

	CTerrorPlayer *GetJockeyAttacker( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_jockeyAttacker));
	}

	CTerrorPlayer *GetTongueOwner( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_tongueOwner));
	}

	CTerrorPlayer *GetReviveTarget( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_reviveTarget));
	}

	CTerrorPlayer *GetUseActionOwner( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_SendProp_m_useActionOwner));
	}

	CTerrorPlayer *GetDominator( void )
	{
		CTerrorPlayer *pPummelAttacker = GetPummelAttacker();

		if (pPummelAttacker)
		{
			return pPummelAttacker;
		}

		CTerrorPlayer *pCarryAttacker = GetCarryAttacker();

		if (pCarryAttacker)
		{
			return pCarryAttacker;
		}

		CTerrorPlayer *pPounceAttacker = GetPounceAttacker();

		if (pPounceAttacker)
		{
			return pPounceAttacker;
		}

		CTerrorPlayer *pJockeyAttacker = GetJockeyAttacker();

		if (pJockeyAttacker)
		{
			return pJockeyAttacker;
		}

		CTerrorPlayer *pTongueOwner = GetTongueOwner();

		if (pTongueOwner)
		{
			return pTongueOwner;
		}

		return nullptr;
	}

	float GetTimeSinceAttackedByEnemy( void )
	{
		CTerrorPlayer *pPlayer = this;

		float flTimeSinceAttackedByEnemy;
		m_pCallWrap_GetTimeSinceAttackedByEnemy->Execute(&pPlayer, &flTimeSinceAttackedByEnemy);

		return flTimeSinceAttackedByEnemy;
	}

	float GetProgressBarStartTime( void )
	{
		return *reinterpret_cast<float *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_flProgressBarStartTime);
	}

	float GetProgressBarDuration( void )
	{
		return *reinterpret_cast<float *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_flProgressBarDuration);
	}

	float GetProgressBarPercent( void )
	{
		return (GetSimulationTime() - GetProgressBarStartTime()) / GetProgressBarDuration();
	}

	float GetHealth( void )
	{
		IPlayerInfo *pPlayerInfo = playerhelpers->GetGamePlayer(entindex())->GetPlayerInfo();

		return pPlayerInfo->GetHealth();
	}

	float GetFullHealth( void )
	{
		return GetHealth() + GetHealthBuffer();
	}

	bool IsDominatedBySpecialInfected( void )
	{
		if (GetPummelAttacker())
		{
			return true;
		}

		if (GetCarryAttacker())
		{
			return true;
		}

		if (GetPounceAttacker())
		{
			return true;
		}

		if (GetJockeyAttacker())
		{
			return true;
		}

		if (GetTongueOwner())
		{
			return true;
		}

		return false;
	}

	bool IsIncapacitated( void )
	{
		return *(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_isIncapacitated);
	}

	bool IsAdrenalineActive( void )
	{
		return *(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_bAdrenalineActive);
	}

	bool IsOnThirdStrike( void )
	{
		return *(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_bIsOnThirdStrike);
	}

	bool ReachedTongueOwner( void )
	{
		return *(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_reachedTongueOwner);
	}

	void StopRevivingSomeone( int bInterrupted )
	{
		struct
		{
			CTerrorPlayer *pPlayer;
			int bInterrupted;
		}
		stStack =
		{
			this,
			bInterrupted
		};

		m_pCallWrap_StopRevivingSomeone->Execute(&stStack, nullptr);
	}

	BackpackItemActionType GetCurrentUseAction( void )
	{
		return *reinterpret_cast<BackpackItemActionType *>(reinterpret_cast<byte *>(this) + m_nOffset_SendProp_m_iCurrentUseAction);
	}
};

class NextBotPlayer_CTerrorPlayer : public CTerrorPlayer
{
public:
	static int m_nOffset_m_inputButtons;
	static int m_nOffset_m_fireButtonTimer;
	static int m_nOffset_m_meleeButtonTimer;
	static int m_nOffset_m_crouchButtonTimer;
	static int m_nOffset_m_nextBotPointer;

	void PressFireButton( float flDuration = -1.0f )
	{
		*reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_m_inputButtons) |= IN_ATTACK;

		CountdownTimer fireButtonTimer = *reinterpret_cast<CountdownTimer *>(reinterpret_cast<byte *>(this) + m_nOffset_m_fireButtonTimer);

		fireButtonTimer.Start(flDuration);
	}

	void PressMeleeButton( float flDuration = -1.0f )
	{
		*reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_m_inputButtons) |= IN_ATTACK2;

		CountdownTimer meleeButtonTimer = *reinterpret_cast<CountdownTimer *>(reinterpret_cast<byte *>(this) + m_nOffset_m_meleeButtonTimer);

		meleeButtonTimer.Start(flDuration);
	}

	void PressCrouchButton( float flDuration = -1.0f )
	{
		*reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_m_inputButtons) |= IN_DUCK;

		CountdownTimer crouchButtonTimer = *reinterpret_cast<CountdownTimer *>(reinterpret_cast<byte *>(this) + m_nOffset_m_crouchButtonTimer);

		crouchButtonTimer.Start(flDuration);
	}

	void ReleaseCrouchButton( void )
	{
		*reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_m_inputButtons) &= ~IN_DUCK;

		CountdownTimer crouchButtonTimer = *reinterpret_cast<CountdownTimer *>(reinterpret_cast<byte *>(this) + m_nOffset_m_crouchButtonTimer);

		crouchButtonTimer.Invalidate();
	}

	INextBot *MyNextBotPointer( void )
	{
		return reinterpret_cast<INextBot *>(reinterpret_cast<byte *>(this) + m_nOffset_m_nextBotPointer);
	}
};

class SurvivorIntention : public IIntention
{
public:
	SurvivorBot *m_me;
	Behavior< SurvivorBot > *m_behavior;
	#define LEGS_STACK_SIZE	16
	Behavior< SurvivorBot > *m_behaviorLegsStack[LEGS_STACK_SIZE][1];
	int iLegsStackTop;
	CUtlVector< RecognizedActor > m_knownThreatsVector;

	static void *m_pFn_IsImmediatelyDangerousTo;

	static ICallWrapper *m_pCallWrap_IsImmediatelyDangerousTo;

	INextBotEventResponder *FirstContainedResponder( void ) const = 0;
	INextBotEventResponder *NextContainedResponder( INextBotEventResponder *current ) const = 0;

	SurvivorBot *GetSurvivorBot( void )
	{
		return m_me;
	}

	CBaseCombatCharacter *GetRecognizedActor( void )
	{
		if (m_knownThreatsVector.Count() > 0)
		{
			RecognizedActor &recognizedActor = m_knownThreatsVector.Head();

			return recognizedActor.GetActor();
		}

		return nullptr;
	}

	bool IsImmediatelyDangerousTo( const CTerrorPlayer *pPlayer, CBaseCombatCharacter *pThreat )
	{
		struct
		{
			SurvivorIntention *pSurvivorIntention;
			const CTerrorPlayer *pPlayer;
			CBaseCombatCharacter *pThreat;
		}
		stStack =
		{
			this,
			pPlayer,
			pThreat
		};

		bool bImmediatelyDangerousTo;
		m_pCallWrap_IsImmediatelyDangerousTo->Execute(&stStack, &bImmediatelyDangerousTo);

		return bImmediatelyDangerousTo;
	}

	bool IsTopPrimaryAction(const char *pszName)
	{
		Behavior<SurvivorBot> *pBehavior = dynamic_cast<Behavior< SurvivorBot > *>(FirstContainedResponder());

		if (pBehavior)
		{
			Action<SurvivorBot> *pAction = dynamic_cast<Action< SurvivorBot > *>(pBehavior->FirstContainedResponder());

			if (pAction && pAction->IsNamed(pszName))
			{
				return true;
			}
		}

		return false;
	}

	void ChangeLegs(Action< SurvivorBot > *pAction)
	{
		Behavior<SurvivorBot> *pCurrBehavior = m_behaviorLegsStack[iLegsStackTop][0];
		Behavior<SurvivorBot> *pNewBehavior = new Behavior<SurvivorBot>(pAction, "[" SMEXT_CONF_LOGTAG "]");

		m_behaviorLegsStack[iLegsStackTop][0] = pNewBehavior;

		if (pCurrBehavior)
		{
			delete pCurrBehavior;
		}
	}

	void PushLegsStack(Action< SurvivorBot > *pAction)
	{
		if (iLegsStackTop >= LEGS_STACK_SIZE-1)
		{
			DevMsg("[" SMEXT_CONF_LOGTAG "] %3.2f: %s: PushLegsStack: Overflow\n", gpGlobals->curtime, GetBot()->GetDebugIdentifier());

			if (pAction)
			{
				delete pAction;
			}
		}
		else
		{
			#if defined __linux__
			m_behaviorLegsStack[iLegsStackTop++][1] = new Behavior<SurvivorBot>(pAction, "[" SMEXT_CONF_LOGTAG "]");
			#elif defined _WIN32
			m_behaviorLegsStack[++iLegsStackTop][0] = new Behavior<SurvivorBot>(pAction, "[" SMEXT_CONF_LOGTAG "]");
			#endif
		}
	}

	void PopLegsStack( void )
	{
		if (iLegsStackTop <= 0)
		{
			DevMsg("[" SMEXT_CONF_LOGTAG "] %3.2f: %s: PopLegsStack: Underflow\n", gpGlobals->curtime, GetBot()->GetDebugIdentifier());
		}
		else
		{
			Behavior<SurvivorBot> *pCurrBehavior = m_behaviorLegsStack[iLegsStackTop][0];

			iLegsStackTop--;

			if (pCurrBehavior)
			{
				delete pCurrBehavior;
			}

			Behavior<SurvivorBot> *pPrevBehavior = m_behaviorLegsStack[iLegsStackTop][0];

			if (pPrevBehavior)
			{
				pPrevBehavior->Resume(m_me);
			}
		}
	}
};

class CBaseCombatWeapon : public CBaseEntity
{
public:
	int GetPrimaryAmmoType( void )
	{
		static int nOffset = -1;

		if (nOffset == -1)
		{
			datamap_t *pMap = gamehelpers->GetDataMap(this);

			sm_datatable_info_t info;
			if (gamehelpers->FindDataMapInfo(pMap, "m_iPrimaryAmmoType", &info))
			{
				nOffset = info.actual_offset;
			}
			else
			{
				smutils->LogError(myself, "Unable to find DataMap info for \"m_iPrimaryAmmoType\"");
			}
		}

		return *reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + nOffset);
	}

	bool InReload( void )
	{
		static int nOffset = -1;

		if (nOffset == -1)
		{
			datamap_t *pMap = gamehelpers->GetDataMap(this);

			sm_datatable_info_t info;
			if (gamehelpers->FindDataMapInfo(pMap, "m_bInReload", &info))
			{
				nOffset = info.actual_offset;
			}
			else
			{
				smutils->LogError(myself, "Unable to find DataMap info for \"m_bInReload\"");
			}
		}

		return *(reinterpret_cast<byte *>(this) + nOffset);
	}
};

class CNavArea
{
public:
	static int m_nOffset_m_center;
	static int m_nOffset_m_attributeFlags;

	const Vector &GetCenter( void )
	{
		return *reinterpret_cast<Vector *>(reinterpret_cast<byte *>(this) + m_nOffset_m_center);
	}

	int GetAttributes( void ) const
	{
		return *reinterpret_cast<NavAttributeType *>(reinterpret_cast<byte *>(const_cast<CNavArea *>(this)) + m_nOffset_m_attributeFlags);
	}
};

class SurvivorBot : public CTerrorPlayer
{
public:
	static int m_nOffset_m_teamSituation;
	static int m_nOffset_m_timeSinceLastSwitchedWeaponTimer;

	static void *m_pFn_IsReachable_Player;

	static ICallWrapper *m_pCallWrap_IsReachable_Player;

	static bool m_bStopping[32+1];

	PlayerBody *GetBodyInterface( void )
	{
		return reinterpret_cast<PlayerBody *>(NextBotPointer()->GetBodyInterface());
	}

	PlayerLocomotion *GetLocomotionInterface( void )
	{
		return reinterpret_cast<PlayerLocomotion *>(NextBotPointer()->GetLocomotionInterface());
	}

	SurvivorIntention *GetIntentionInterface( void )
	{
		return reinterpret_cast<SurvivorIntention *>(NextBotPointer()->GetIntentionInterface());
	}

	SurvivorTeamSituation *GetTeamSituation( void )
	{
		return reinterpret_cast<SurvivorTeamSituation *>(reinterpret_cast<byte *>(this) + m_nOffset_m_teamSituation);
	}

	IntervalTimer TimeSinceLastSwitchedWeaponTimer( void )
	{
		return *reinterpret_cast<IntervalTimer *>(reinterpret_cast<byte *>(this) + m_nOffset_m_timeSinceLastSwitchedWeaponTimer);
	}

	bool IsReachable( CTerrorPlayer *pPlayer )
	{
		struct
		{
			SurvivorBot *pSurvivorBot;
			CTerrorPlayer *pPlayer;
		}
		stStack =
		{
			this,
			pPlayer
		};

		bool bReachable;
		m_pCallWrap_IsReachable_Player->Execute(&stStack, &bReachable);

		return bReachable;
	}

	bool IsPathAheadInAcidSpitOrFire( const Path &path, Vector *pWhere = nullptr )
	{
		const Path::Segment *pGoalSeg = path.GetCurrentGoal();

		if (pGoalSeg)
		{
			CBaseEntity *pEntity = nullptr;

			static const char *s_pszAvoidEntityList[] =
			{
				#if SOURCE_ENGINE == SE_LEFT4DEAD2
				"insect_swarm",
				#endif
				"inferno"
			};

			for (const char *pszClassname : s_pszAvoidEntityList)
			{
				while ((pEntity = FindEntityByClassname(pEntity, pszClassname)) != nullptr)
				{
					for (const Path::Segment *pTargetSeg = pGoalSeg; pTargetSeg; pTargetSeg = path.NextSegment(pTargetSeg))
					{
						const Path::Segment *pPriorSeg = path.PriorSegment(pTargetSeg);

						if (!pPriorSeg)
						{
							pPriorSeg = pTargetSeg;
						}

						if (reinterpret_cast<CInferno *>(pEntity)->IsTouching(pPriorSeg->pos, pTargetSeg->pos, pWhere))
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool EquipMeleeWeapon( void )
	{
		CBaseCombatWeapon *pSecondaryWeapon = Weapon_GetSlot(WEAPON_SLOT_PISTOL);

		if (pSecondaryWeapon
			&& pSecondaryWeapon != GetActiveWeapon())	// always check this because bot has Weapon_Switch method overridden
														// which updates time since last switched weapon whenever method is called
		{
			if (FStrEq(pSecondaryWeapon->GetClassname(), "weapon_melee"))
			{
				return Weapon_Switch(pSecondaryWeapon, 0);
			}
		}

		return false;
	}

	void EquipWeapon( void )
	{
		CBaseCombatWeapon *pPrimaryWeapon = Weapon_GetSlot(WEAPON_SLOT_RIFLE);

		if (pPrimaryWeapon)
		{
			if (!Weapon_Switch(pPrimaryWeapon, 0))
			{
				pPrimaryWeapon = Weapon_GetSlot(WEAPON_SLOT_PISTOL);

				Weapon_Switch(pPrimaryWeapon, 0);
			}
		}
		else
		{
			CBaseCombatWeapon *pSecondaryWeapon = Weapon_GetSlot(WEAPON_SLOT_PISTOL);

			Weapon_Switch(pSecondaryWeapon, 0);
		}
	}

	INextBot *NextBotPointer( void )
	{
		return reinterpret_cast<INextBot *>(reinterpret_cast<byte *>(this) + NextBotPlayer_CTerrorPlayer::m_nOffset_m_nextBotPointer);
	}
};

class Witch : public CBaseCombatCharacter
{
public:
	static int m_nOffset_m_hHarasser;

	CBaseEntity *GetHarasser( void )
	{
		return EntityFromBaseHandle(this, m_nOffset_m_hHarasser);
	}
};

class SurvivorTeamSituation
{
public:
	static int m_nOffset_m_me;
	static int m_nOffset_m_friendInTrouble;
	static int m_nOffset_m_humanFriendInTrouble;
	static int m_nOffset_m_humanLeader;
	static int m_nOffset_m_tonguedFriend;
	static int m_nOffset_m_pouncedFriend;
	static int m_nOffset_m_pummeledFriend;
	static int m_nOffset_m_jockeyedFriend;

	SurvivorBot *GetBot( void )
	{
		return *reinterpret_cast<SurvivorBot **>(reinterpret_cast<byte *>(this) + m_nOffset_m_me);
	}

	CTerrorPlayer *GetHumanLeader( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_humanLeader));
	}

	CTerrorPlayer *GetFriendInTrouble( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_friendInTrouble));
	}

	CTerrorPlayer *GetHumanFriendInTrouble( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_humanFriendInTrouble));
	}

	CTerrorPlayer *GetTonguedFriend( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_tonguedFriend));
	}

	CTerrorPlayer *GetPouncedFriend( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_pouncedFriend));
	}

	CTerrorPlayer *GetPummeledFriend( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_pummeledFriend));
	}

	CTerrorPlayer *GetJockeyedFriend( void )
	{
		return reinterpret_cast<CTerrorPlayer *>(EntityFromBaseHandle(this, m_nOffset_m_jockeyedFriend));
	}

	bool HasAnyFriendInTrouble( void )
	{
		if ( GetFriendInTrouble() )
		{
			return true;
		}

		if ( GetHumanFriendInTrouble() )
		{
			return true;
		}

		if ( GetTonguedFriend() )
		{
			return true;
		}

		if ( GetPouncedFriend() )
		{
			return true;
		}

		if ( GetPummeledFriend() )
		{
			return true;
		}

		if ( GetJockeyedFriend() )
		{
			return true;
		}

		return false;
	}

	void SetFriendInTrouble( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_friendInTrouble, pPlayer);
	}

	void SetHumanFriendInTrouble( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_humanFriendInTrouble, pPlayer);
	}

	void SetTonguedFriend( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_tonguedFriend, pPlayer);
	}

	void SetPouncedFriend( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_pouncedFriend, pPlayer);
	}

	void SetPummeledFriend( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_pummeledFriend, pPlayer);
	}

	void SetJockeyedFriend( CTerrorPlayer *pPlayer )
	{
		SetEntityInBaseHandle(this, m_nOffset_m_jockeyedFriend, pPlayer);
	}

	virtual void OnBeginIteration( void ) = 0;							// invoked once before iteration begins

	virtual bool operator() ( CTerrorPlayer *player ) = 0;

	virtual void OnEndIteration( bool allElementsIterated )	= 0;		// invoked once after iteration is complete whether successful or not
};

class CDirector
{
public:
	static int m_nOffset_m_iTankCount;

	int GetTankCount( void )
	{
		return *reinterpret_cast<int *>(reinterpret_cast<byte *>(this) + m_nOffset_m_iTankCount);
	}
};

/**
 * Functor used with NavAreaBuildPath()
 */
class DummyPathCost
{
public:
	float operator() ( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder, const CFuncElevator *elevator, float length )
	{
		return 0.0f;
	}
};

//
// Collect all valid, connected players into given vector.
// Returns number of players collected.
//
#define COLLECT_ONLY_LIVING_PLAYERS true
#define APPEND_PLAYERS true
template < typename T >
int CollectPlayers( CUtlVector< T * > *playerVector, int team = TEAM_ANY, bool isAlive = false, bool shouldAppend = false )
{
	if (!shouldAppend)
	{
		playerVector->RemoveAll();
	}

	for (int iClient = 1; iClient <= playerhelpers->GetMaxClients(); iClient++)
	{
		IGamePlayer *pGamePlayer = playerhelpers->GetGamePlayer(iClient);

		if (!pGamePlayer)
		{
			continue;
		}

		if (!pGamePlayer->IsConnected())
		{
			continue;
		}

		IPlayerInfo *pPlayerInfo = pGamePlayer->GetPlayerInfo();

		if (team != TEAM_ANY)
		{
			if (!pPlayerInfo)
			{
				continue;
			}

			if (pPlayerInfo->GetTeamIndex() != team)
			{
				continue;
			}
		}

		T *pPlayer = reinterpret_cast<T *>(gamehelpers->ReferenceToEntity(iClient));

		if (isAlive)
		{
			if (!pPlayerInfo)
			{
				continue;
			}

			if (!pPlayer->IsAlive())
			{
				continue;
			}
		}

		playerVector->AddToTail(pPlayer);
	}

	return playerVector->Count();
}

static int GetMaxCarryOfWeapon(const char *pszWeaponName)
{
	static ConVarRef r_ammo_assaultrifle_max("ammo_assaultrifle_max"),
			r_ammo_smg_max("ammo_smg_max"),
			r_ammo_autoshotgun_max("ammo_autoshotgun_max"),
			r_ammo_grenadelauncher_max("ammo_grenadelauncher_max"),
			r_ammo_huntingrifle_max("ammo_huntingrifle_max"),
			r_ammo_m60_max("ammo_m60_max"),
			r_ammo_shotgun_max("ammo_shotgun_max"),
			r_ammo_sniperrifle_max("ammo_sniperrifle_max"),
			r_ammo_pistol_max("ammo_pistol_max");

	if (!V_strcmp(pszWeaponName, "weapon_smg") || !V_strcmp(pszWeaponName, "weapon_smg_silenced") || !V_strcmp(pszWeaponName, "weapon_smg_mp5"))
	{
		return r_ammo_smg_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_pumpshotgun") || !V_strcmp(pszWeaponName, "weapon_shotgun_chrome"))
	{
		return r_ammo_shotgun_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_autoshotgun") || !V_strcmp(pszWeaponName, "weapon_shotgun_spas"))
	{
		return r_ammo_autoshotgun_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_rifle") || !V_strcmp(pszWeaponName, "weapon_rifle_ak47")
		|| !V_strcmp(pszWeaponName, "weapon_rifle_desert") || !V_strcmp(pszWeaponName, "weapon_rifle_sg552"))
	{
		return r_ammo_assaultrifle_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_rifle_m60"))
	{
		return r_ammo_m60_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_hunting_rifle"))
	{
		return r_ammo_huntingrifle_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_sniper_military") || !V_strcmp(pszWeaponName, "weapon_sniper_awp")
		|| !V_strcmp(pszWeaponName, "weapon_sniper_scout"))
	{
		return r_ammo_sniperrifle_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_grenade_launcher"))
	{
		return r_ammo_grenadelauncher_max.GetInt();
	}

	if (!V_strcmp(pszWeaponName, "weapon_pistol") || !V_strcmp(pszWeaponName, "weapon_pistol_magnum"))
	{
		return r_ammo_pistol_max.GetInt();
	}

	return -1;
}

static CBaseEntity *FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName)
{
	if (pStartEntity == nullptr)
	{
		pStartEntity = (CBaseEntity *)servertools->FirstEntity();
	}
	else
	{
		pStartEntity = (CBaseEntity *)servertools->NextEntity(pStartEntity);
	}

	// it's tough to find a good ent these days
	if (!pStartEntity)
	{
		return nullptr;
	}

	const char *classname;
	int lastletterpos;

	static int offset = -1;
	if (offset == -1)
	{
		sm_datatable_info_t info;
		if (!gamehelpers->FindDataMapInfo(gamehelpers->GetDataMap(pStartEntity), "m_iClassname", &info))
		{
			return nullptr;
		}

		offset = info.actual_offset;
	}

	string_t s;

	while (pStartEntity)
	{
		if ((s = *(string_t *)((uint8_t *)pStartEntity + offset)) == NULL_STRING)
		{
			pStartEntity = (CBaseEntity *)servertools->NextEntity(pStartEntity);
			continue;
		}

		classname = STRING(s);

		lastletterpos = strlen(szName) - 1;
		if (szName[lastletterpos] == '*')
		{
			if (strncasecmp(szName, classname, lastletterpos) == 0)
			{
				return pStartEntity;
			}
		}
		else if (strcasecmp(szName, classname) == 0)
		{
			return pStartEntity;
		}

		pStartEntity = (CBaseEntity *)servertools->NextEntity(pStartEntity);
	}

	return nullptr;
}

class SurvivorUseObject : public Action< SurvivorBot >
{
private:
	CHandle< CBaseEntity > m_target;
	CountdownTimer m_useObjectTimer;
	CountdownTimer m_unknownUnusedTimer;
	CountdownTimer m_shouldGiveUpTimer;
	Vector m_objectPosition;

public:
	static void *m_pFn_ShouldGiveUp;

	static ICallWrapper *m_pCallWrap_ShouldGiveUp;

	CBaseEntity *GetTarget( void )
	{
		return m_target;
	}

	bool ShouldGiveUp( SurvivorBot *pSurvivorBot ) const
	{
		struct
		{
			const SurvivorUseObject *pAction;
			SurvivorBot *pSurvivorBot;
		}
		stStack =
		{
			this,
			pSurvivorBot
		};

		bool bShouldGiveUp;
		m_pCallWrap_ShouldGiveUp->Execute(&stStack, &bShouldGiveUp);

		return bShouldGiveUp;
	}
};

class SurvivorHealFriend : public Action< SurvivorBot >
{
private:
	CHandle< CTerrorPlayer > m_patient;

public:
	CTerrorPlayer *GetPatient( void )
	{
		return m_patient;
	}
};

class SurvivorTakePills : public Action< SurvivorBot >
{
private:
	CountdownTimer m_takePillsTimer;

public:
	void Start( float flDuration )
	{
		m_takePillsTimer.Start(flDuration);
	}
};

class SurvivorLiberateBesiegedFriend : public Action< SurvivorBot >
{
private:
	CHandle< CTerrorPlayer > m_besiegedFriend;

public:
	CTerrorPlayer *GetBesiegedFriend( void )
	{
		return m_besiegedFriend;
	}
};

class SurvivorDislodgeVictim : public SurvivorUseObject
{
public:
	bool m_bBash;
};

class SurvivorAttack : public Action< SurvivorBot >
{
public:
	static void *m_pFn_SelectTarget;

	static ICallWrapper *m_pCallWrap_SelectTarget;

	CBaseCombatCharacter *SelectTarget( SurvivorBot *pSurvivorBot )
	{
		struct
		{
			SurvivorAttack *pAction;
			SurvivorBot *pSurvivorBot;
		}
		stStack =
		{
			this,
			pSurvivorBot
		};

		CBaseCombatCharacter *pTarget;
		m_pCallWrap_SelectTarget->Execute(&stStack, &pTarget);

		return pTarget;
	}
};

class SurvivorDispatchEnemy : public Action< SurvivorBot >
{
public:
	CHandle< CTerrorPlayer > m_hCharger;
	CountdownTimer m_giveUpTimer;
	bool m_bFiringWeapon;
	bool m_bWeaponReady;
};

class SurvivorEscapeSpit : public Action< SurvivorBot >
{
public:
	char padding[4];
	IntervalTimer m_waitTimer;
};

class SurvivorEscapeFlames : public Action< SurvivorBot >
{
public:
	char padding[4];
	IntervalTimer m_waitTimer;
};

class SurvivorLegsApproach : public Action< SurvivorBot >
{
public:
	CHandle< CBaseEntity > m_entity;
	Vector m_moveToPos;

	SurvivorLegsApproach( CBaseEntity *pEntity )
	{
		return;
	}

	virtual const char *GetName() const		{ return "SurvivorLegsApproach"; }
};

class SurvivorBotAttackOnReply : public INextBotReply
{
public:
	virtual void OnSuccess( INextBot *pNextBot )
	{
		// NextBotPlayer<CTerrorPlayer> *pNextBotTerrorPlayer = static_cast<NextBotPlayer<CTerrorPlayer> *>(pNextBot);

		// if (pNextBotTerrorPlayer)
		// {
		// 	pNextBotTerrorPlayer->PressFireButton(-1.0f);
		// }

		NextBotPlayer_CTerrorPlayer *pNextBotTerrorPlayer = reinterpret_cast<NextBotPlayer_CTerrorPlayer *>(reinterpret_cast<byte *>(pNextBot) \
			- NextBotPlayer_CTerrorPlayer::m_nOffset_m_nextBotPointer);

		pNextBotTerrorPlayer->PressFireButton();
	}
};

class SurvivorBotMeleeOnReply : public INextBotReply
{
public:
	virtual void OnSuccess( INextBot *pNextBot )
	{
		// NextBotPlayer<CTerrorPlayer> *pNextBotTerrorPlayer = static_cast<NextBotPlayer<CTerrorPlayer> *>(pNextBot);

		// if (pNextBotTerrorPlayer)
		// {
		// 	pNextBotTerrorPlayer->PressMeleeButton(-1.0f);
		// }

		NextBotPlayer_CTerrorPlayer *pNextBotTerrorPlayer = reinterpret_cast<NextBotPlayer_CTerrorPlayer *>(reinterpret_cast<byte *>(pNextBot) \
			- NextBotPlayer_CTerrorPlayer::m_nOffset_m_nextBotPointer);

		pNextBotTerrorPlayer->PressMeleeButton();
	}
};

typedef bool (*ShouldHitFunc_t)( IHandleEntity *pHandleEntity, int contentsMask );

class CTraceFilterSimple : public CTraceFilter
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterSimple );

	CTraceFilterSimple( const IHandleEntity *passentity, int collisionGroup, ShouldHitFunc_t pExtraShouldHitCheckFn = nullptr );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask ) = 0;
	virtual void SetPassEntity( const IHandleEntity *pPassEntity ) { m_pPassEnt = pPassEntity; }
	virtual void SetCollisionGroup( int iCollisionGroup ) { m_collisionGroup = iCollisionGroup; }

	const IHandleEntity *GetPassEntity( void ){ return m_pPassEnt;}

private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
	ShouldHitFunc_t m_pExtraShouldHitCheckFunction;

};

/**
 * Trace filter that skips "traversable" entities.  The "when" argument creates
 * a temporal context for asking if an entity is IMMEDIATELY traversable (like thin
 * glass that just breaks as we walk through it) or EVENTUALLY traversable (like a
 * breakable object that will take some time to break through)
 */
class NextBotTraversableTraceFilter : public CTraceFilterSimple
{
public:
	INextBot *m_bot;
	ILocomotion::TraverseWhenType m_when;
};