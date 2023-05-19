#include "extension.h"
#include "CDetour/detours.h"
#include "wrappers.h"
#include <algorithm>
#include <functional>
#include <apparent_velocity_helper.h>
#include "../imatchext/IMatchExtInterface.h"
#include "nav_ladder.h"
#include "SurvivorLegsWait.h"
#include "NextBot/NextBotComponentInterface.h"
#include "NextBot/Player/NextBotPlayerBody.h"
#include "NextBot/Player/NextBotPlayerLocomotion.h"
#include "public/tier1/utlpair.h"

CImprovedSurvivorBots g_ImprovedSurvivorBots;

SMEXT_LINK(&g_ImprovedSurvivorBots);

CGlobalVars *gpGlobals = nullptr;
IServerGameEnts *serverGameEnts = nullptr;
IBinTools *bintools = nullptr;
CBaseEntityList *g_pEntityList = nullptr;
IMatchExtInterface *g_pMatchExtInterface = nullptr;
ICvar *g_pCVar = nullptr;
IServerTools *servertools = nullptr;
IStaticPropMgrServer *staticpropmgr = nullptr;

SH_DECL_MANUALHOOK2(MHook_Action_OnStart, 0, 0, 0, ActionResult<SurvivorBot>, SurvivorBot *, Action<SurvivorBot> *);
SH_DECL_MANUALHOOK2(MHook_Action_Update, 0, 0, 0, ActionResult<SurvivorBot>, SurvivorBot *, float);
SH_DECL_MANUALHOOK2_void(MHook_Action_OnEnd, 0, 0, 0, SurvivorBot *, Action<SurvivorBot> *);
SH_DECL_MANUALHOOK2(MHook_Action_OnSuspend, 0, 0, 0, ActionResult<SurvivorBot>, SurvivorBot *, Action<SurvivorBot> *);
SH_DECL_MANUALHOOK1(MHook_Action_InitialContainedAction, 0, 0, 0, Action<SurvivorBot> *, SurvivorBot *);

SH_DECL_MANUALHOOK1_void(MHook_SurvivorUseObject_OnStartUse, 0, 0, 0, SurvivorBot *);
SH_DECL_MANUALHOOK1(MHook_SurvivorUseObject_ShouldGiveUp, 0, 0, 0, bool, SurvivorBot *);

SH_DECL_MANUALHOOK1(MHook_Action_OnIgnite, 0, 0, 0, EventDesiredResult<SurvivorBot>, SurvivorBot *);
SH_DECL_MANUALHOOK2(MHook_Action_OnInjured, 0, 0, 0, EventDesiredResult<SurvivorBot>, SurvivorBot *, const CTakeDamageInfo &);
SH_DECL_MANUALHOOK1(MHook_Action_OnEnteredSpit, 0, 0, 0, EventDesiredResult<SurvivorBot>, SurvivorBot *);
SH_DECL_MANUALHOOK3(MHook_Action_OnCommandApproach, 0, 0, 0, EventDesiredResult<SurvivorBot>, SurvivorBot *, const Vector &, float);

SH_DECL_MANUALHOOK0(MHook_CBaseCombatCharacter_IsIT, 0, 0, 0, bool);

SH_DECL_MANUALHOOK1_void(MHook_IIntention_OnInjured, 0, 0, 0, const CTakeDamageInfo &);
SH_DECL_MANUALHOOK0_void(MHook_IIntention_Reset, 0, 0, 0);

SH_DECL_MANUALHOOK0(MHook_IBody_GetSolidMask, 0, 0, 0, unsigned int);
SH_DECL_MANUALHOOK3(MHook_ILocomotion_ClimbUpToLedge, 0, 0, 0, bool, const Vector &, const Vector &, const CBaseEntity *);
SH_DECL_MANUALHOOK0(MHook_ILocomotion_IsRunning, 0, 0, 0, bool);

SH_DECL_MANUALHOOK2_void(MHook_CBasePlayer_OnNavAreaChanged, 0, 0, 0, CNavArea *, CNavArea *);

int SurvivorBot::m_nOffset_m_teamSituation = -1;
int SurvivorBot::m_nOffset_m_timeSinceLastSwitchedWeaponTimer = -1;

int CBaseEntity::m_nOffset_SendProp_m_flSimulationTime = -1;
int CBaseEntity::m_nOffset_SendProp_m_iTeamNum = -1;
int CBaseEntity::m_nOffset_m_lifeState = -1;

int CBaseCombatCharacter::m_iVtbl_GetClass = -1;
int CBaseCombatCharacter::m_iVtbl_Weapon_GetSlot = -1;
int CBaseCombatCharacter::m_iVtbl_Weapon_Switch = -1;
int CBaseCombatCharacter::m_iVtbl_IsIT = -1;

int CBaseCombatCharacter::m_nOffset_m_lastKnownArea = -1;

int CBaseCombatCharacter::m_nOffset_SendProp_m_iAmmo = -1;
int CBaseCombatCharacter::m_nOffset_SendProp_m_hActiveWeapon = -1;

int CCSPlayer::m_iVtbl_GetHealthBuffer = -1;

int CTerrorPlayer::m_nOffset_SendProp_m_pummelAttacker = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_pummelVictim = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_carryAttacker = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_pounceAttacker = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_jockeyAttacker = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_tongueOwner = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_reviveTarget = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_isIncapacitated = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_bAdrenalineActive = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_bIsOnThirdStrike = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_flProgressBarStartTime = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_flProgressBarDuration = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_useActionOwner = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_iCurrentUseAction = -1;
int CTerrorPlayer::m_nOffset_SendProp_m_reachedTongueOwner = -1;

int NextBotPlayer_CTerrorPlayer::m_nOffset_m_inputButtons = -1;
int NextBotPlayer_CTerrorPlayer::m_nOffset_m_fireButtonTimer = -1;
int NextBotPlayer_CTerrorPlayer::m_nOffset_m_meleeButtonTimer = -1;
int NextBotPlayer_CTerrorPlayer::m_nOffset_m_crouchButtonTimer = -1;
int NextBotPlayer_CTerrorPlayer::m_nOffset_m_nextBotPointer = -1;

int Witch::m_nOffset_m_hHarasser = -1;

int SurvivorTeamSituation::m_nOffset_m_me = -1;
int SurvivorTeamSituation::m_nOffset_m_friendInTrouble = -1;
int SurvivorTeamSituation::m_nOffset_m_humanFriendInTrouble = -1;
int SurvivorTeamSituation::m_nOffset_m_humanLeader = -1;
int SurvivorTeamSituation::m_nOffset_m_tonguedFriend = -1;
int SurvivorTeamSituation::m_nOffset_m_pouncedFriend = -1;
int SurvivorTeamSituation::m_nOffset_m_pummeledFriend = -1;
int SurvivorTeamSituation::m_nOffset_m_jockeyedFriend = -1;

int CDirector::m_nOffset_m_iTankCount = -1;

int CNavArea::m_nOffset_m_center = -1;
int CNavArea::m_nOffset_m_attributeFlags = -1;

void *SurvivorIntention::m_pFn_IsImmediatelyDangerousTo = nullptr;

void *SurvivorBot::m_pFn_IsReachable_Player = nullptr;

void *CTerrorPlayer::m_pFn_GetTimeSinceAttackedByEnemy = nullptr;
void *CTerrorPlayer::m_pFn_StopRevivingSomeone = nullptr;

void *CInferno::m_pFn_IsTouching_VVV = nullptr;

void *SurvivorUseObject::m_pFn_ShouldGiveUp = nullptr;

void *SurvivorAttack::m_pFn_SelectTarget = nullptr;

ICallWrapper *SurvivorIntention::m_pCallWrap_IsImmediatelyDangerousTo = nullptr;

ICallWrapper *SurvivorBot::m_pCallWrap_IsReachable_Player = nullptr;

ICallWrapper *CBaseCombatCharacter::m_pCallWrap_GetClass = nullptr;
ICallWrapper *CBaseCombatCharacter::m_pCallWrap_Weapon_GetSlot = nullptr;
ICallWrapper *CBaseCombatCharacter::m_pCallWrap_Weapon_Switch = nullptr;

ICallWrapper *CCSPlayer::m_pCallWrap_GetHealthBuffer = nullptr;

ICallWrapper *CTerrorPlayer::m_pCallWrap_GetTimeSinceAttackedByEnemy = nullptr;
ICallWrapper *CTerrorPlayer::m_pCallWrap_StopRevivingSomeone = nullptr;

ICallWrapper *CInferno::m_pCallWrap_IsTouching_VVV = nullptr;

ICallWrapper *SurvivorUseObject::m_pCallWrap_ShouldGiveUp = nullptr;

ICallWrapper *SurvivorAttack::m_pCallWrap_SelectTarget = nullptr;

bool SurvivorBot::m_bStopping[32+1] = {false};

ConVar SurvivorBotTeamSituationGiveUpRange("sb_team_situation_give_up_range", "2000.0", FCVAR_CHEAT);

ConVar SurvivorBotHealIgnoreDamageMinHealth("sb_heal_ignore_damage_min_health", "5", FCVAR_CHEAT);
ConVar SurvivorBotHealIgnoreMultiDamageMinHealth("sb_heal_ignore_multi_damage_min_health", "10", FCVAR_CHEAT);

ConVar SurvivorReviveIgnoreDamageMinHealth("sb_revive_ignore_damage_min_health", "5", FCVAR_CHEAT);
ConVar SurvivorBotReviveIgnoreMultiDamageMinHealth("sb_revive_ignore_multi_damage_min_health", "10", FCVAR_CHEAT);

ConVar SurvivorBotEscortRange("sb_escort_range", "150.0", FCVAR_CHEAT);
ConVar SurvivorBotEscortRangeVersus("sb_escort_range_versus", "180.0", FCVAR_CHEAT);
ConVar SurvivorBotEscortRangeScavenge("sb_escort_range_scavenge", "120.0", FCVAR_CHEAT);

SurvivorBotAttackOnReply g_SurvivorBotAttackOnReply;
SurvivorBotMeleeOnReply g_SurvivorBotMeleeOnReply;
#if defined DEBUG
bool g_bUseNetworkVars = false;
#endif
static bool IsGameMode(const char *pszGameMode)
{
	static ConVarRef r_mp_gamemode("mp_gamemode");

	const char *pszName = r_mp_gamemode.GetString();
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	KeyValues *pModeInfo = g_pMatchExtInterface->GetIMatchExtL4D()->GetGameModeInfo(pszName);

	if (pModeInfo)
	{
		return !V_stricmp(pModeInfo->GetString("base"), pszGameMode);
	}

	return false;
#else
	return !V_stricmp(pszName, pszGameMode);
#endif
}

static bool IsVersusMode()
{
	return IsGameMode("versus");
}

static bool IsScavengeMode()
{
	return IsGameMode("scavenge");
}

static bool IsSurvivalMode()
{
	return IsGameMode("survival");
}

FORCEINLINE void SurvivorBot_UpdateTeamSituation(SurvivorBot *pSurvivorBot, SurvivorTeamSituation *pTeamSituation)
{
	CUtlVector<CTerrorPlayer *> vecAliveSurvivors;
	CollectPlayers(&vecAliveSurvivors, TEAM_SURVIVOR, COLLECT_ONLY_LIVING_PLAYERS);

	CUtlVector<CUtlPair<CTerrorPlayer *, float>> vecSurvivors;

	FOR_EACH_VEC(vecAliveSurvivors, iter)
	{
		if (vecAliveSurvivors[iter] == pSurvivorBot)
		{
			continue;
		}

		// Fix issue where bots may consider teammates that are unreachable
		if (!pSurvivorBot->IsReachable(vecAliveSurvivors[iter]))
		{
			continue;
		}

		// Fix issues where bots wouldn't consider the closest teammate to them to save them, have them as the leader, etc
		// It adds them to a vector in a descending order so that the iterator executes the function call operator on closest players in vector
		// We want bots to chase after the closest players because it's much smarter to do so
		float flTravelDistance = g_ImprovedSurvivorBots.NavAreaTravelDistance_ShortestPathCost(vecAliveSurvivors[iter]->GetLastKnownArea(), pSurvivorBot->GetLastKnownArea());

		// Fix issue where bots would go after teammates that are too far away
		if (flTravelDistance > SurvivorBotTeamSituationGiveUpRange.GetFloat())
		{
			continue;
		}

		vecSurvivors.AddToTail(MakeUtlPair(vecAliveSurvivors[iter], flTravelDistance));
	}

	std::sort(vecSurvivors.Base(), vecSurvivors.Base() + vecSurvivors.Count(), [](auto &left, auto &right)
	{
		return left.second > right.second;
	});

	pTeamSituation->OnBeginIteration();

	bool isComplete = true;

	FOR_EACH_VEC(vecSurvivors, iter)
	{
		if (pTeamSituation->operator()(vecSurvivors[iter].first) == false)
		{
			isComplete = false;
			break;
		}
	}

	pTeamSituation->OnEndIteration(isComplete);
}

#ifdef __linux__
DETOUR_DECL_MEMBER0(DetourFunc_SurvivorBot_UpdateTeamSituation, void)
{
	SurvivorBot *pSurvivorBot = reinterpret_cast<SurvivorBot *>(this);

	SurvivorBot_UpdateTeamSituation(pSurvivorBot, pSurvivorBot->GetTeamSituation());
}
#elif defined _WIN32
DETOUR_DECL_STATIC1(DetourFunc_SurvivorBot_UpdateTeamSituation, void, SurvivorTeamSituation *, pTeamSituation)
{
	SurvivorBot_UpdateTeamSituation(pTeamSituation->GetBot(), pTeamSituation);
}
#endif

// Fix issue where bots may prioritize teammates that are far away from them when saving them
DETOUR_DECL_MEMBER1(DetourFunc_SurvivorBot_SaveFriendsInImmediateTrouble, ActionResult<SurvivorBot>, Action<SurvivorBot> *, pPriorAction)
{
	SurvivorBot *pSurvivorBot = reinterpret_cast<SurvivorBot *>(this);

	SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

	CTerrorPlayer *pCloseFriend = g_ImprovedSurvivorBots.GetClosestDominatedFriend(pTeamSituation);

	if (pCloseFriend)
	{
		CTerrorPlayer *pTonguedFriend = pTeamSituation->GetTonguedFriend();

		if (pCloseFriend != pTonguedFriend)
		{
			pTeamSituation->SetTonguedFriend(nullptr);
		}

		CTerrorPlayer *pPouncedFriend = pTeamSituation->GetPouncedFriend();

		if (pCloseFriend != pPouncedFriend)
		{
			pTeamSituation->SetPouncedFriend(nullptr);
		}

		CTerrorPlayer *pPummeledFriend = pTeamSituation->GetPummeledFriend();

		if (pCloseFriend != pPummeledFriend)
		{
			pTeamSituation->SetPummeledFriend(nullptr);
		}

		CTerrorPlayer *pJockeyedFriend = pTeamSituation->GetJockeyedFriend();

		if (pCloseFriend != pJockeyedFriend)
		{
			pTeamSituation->SetJockeyedFriend(nullptr);
		}

		ActionResult<SurvivorBot> stOrigActionResult = DETOUR_MEMBER_CALL(DetourFunc_SurvivorBot_SaveFriendsInImmediateTrouble)(pPriorAction);

		pTeamSituation->SetTonguedFriend(pTonguedFriend);
		pTeamSituation->SetPouncedFriend(pPouncedFriend);
		pTeamSituation->SetPummeledFriend(pPummeledFriend);
		pTeamSituation->SetJockeyedFriend(pJockeyedFriend);

		return stOrigActionResult;
	}

	pCloseFriend = g_ImprovedSurvivorBots.GetClosestFriendInTrouble(pTeamSituation);

	if (pCloseFriend)
	{
		CTerrorPlayer *pFriend = pTeamSituation->GetFriendInTrouble();

		if (pCloseFriend != pFriend)
		{
			pTeamSituation->SetFriendInTrouble(nullptr);
		}

		CTerrorPlayer *pHumanFriend = pTeamSituation->GetHumanFriendInTrouble();

		if (pCloseFriend != pHumanFriend)
		{
			pTeamSituation->SetHumanFriendInTrouble(nullptr);
		}

		ActionResult<SurvivorBot> stOrigActionResult = DETOUR_MEMBER_CALL(DetourFunc_SurvivorBot_SaveFriendsInImmediateTrouble)(pPriorAction);

		pTeamSituation->SetFriendInTrouble(pFriend);
		pTeamSituation->SetHumanFriendInTrouble(pHumanFriend);

		return stOrigActionResult;
	}

	return DETOUR_MEMBER_CALL(DetourFunc_SurvivorBot_SaveFriendsInImmediateTrouble)(pPriorAction);
}

// Fix issue where Action's Update function would call this function to determine whether bot took any damage
// We've already got OnInjured event for this, that's why we're intercepting it
DETOUR_DECL_MEMBER0(DetourFunc_CTerrorPlayer_GetTimeSinceAttackedByEnemy, float)
{
	return 99999.9f;
}

DETOUR_DECL_MEMBER2(DetourFunc_SurvivorIntention_IsImmediatelyDangerousTo, bool, const CTerrorPlayer *, pPlayer, CBaseCombatCharacter *, pThreat)
{
	if (pThreat)
	{
		// Fix issue where bots would retreat from Witch which is busy with another survivor
		if (pThreat->GetClass() == Zombie_Witch)
		{
			Witch *pWitch = reinterpret_cast<Witch *>(pThreat);

			CBaseEntity *pHarasser = pWitch->GetHarasser();

			if (pHarasser && pHarasser != pPlayer)
			{
				return false;
			}
		}
		else if (pThreat->GetClass() == Zombie_Charger)		// Fix issue where bots would retreat from Charger who is busy with another survivor
		{
			CTerrorPlayer *pCharger = reinterpret_cast<CTerrorPlayer *>(pThreat);

			if (pCharger->GetPummelVictim())
			{
				return false;
			}
		}
	}

	return DETOUR_MEMBER_CALL(DetourFunc_SurvivorIntention_IsImmediatelyDangerousTo)(pPlayer, pThreat);
}

DETOUR_DECL_MEMBER1(DetourFunc_SurvivorUseObject_OnStartUse, void, SurvivorBot *, pSurvivorBot)
{
	return;
}

// Fix issue where escort ranges are hardcoded
DETOUR_DECL_MEMBER0(DetourFunc_SurvivorBot_GetEscortRange, float)
{
	if (IsVersusMode())
	{
		return SurvivorBotEscortRangeVersus.GetFloat();
	}

	if (IsScavengeMode())
	{
		return SurvivorBotEscortRangeScavenge.GetFloat();
	}

	return SurvivorBotEscortRange.GetFloat();
}

// Fix issue where bots would abandon their leader to scavenge items when regrouping
DETOUR_DECL_MEMBER1(DetourFunc_SurvivorBot_ScavengeNearbyItems, ActionResult<SurvivorBot>, Action<SurvivorBot> *, pPriorAction)
{
	if (IsSurvivalMode())
	{
		SurvivorIntention *pSurvivorIntention = reinterpret_cast<SurvivorBot *>(this)->GetIntentionInterface();

		INextBotEventResponder *pNextContainedResponder = pSurvivorIntention->m_behaviorLegsStack[0][0];

		Behavior<SurvivorBot> *pNextBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pNextContainedResponder);

		if (pNextBehavior)
		{
			Action<SurvivorBot> *pAction = dynamic_cast<Action< SurvivorBot > *>(pNextBehavior->FirstContainedResponder());

			if (pAction && pAction->IsNamed("SurvivorLegsRegroup"))
			{
				return pPriorAction->Continue();
			}
		}
	}

	return DETOUR_MEMBER_CALL(DetourFunc_SurvivorBot_ScavengeNearbyItems)(pPriorAction);
}

// Fix issue where bots wouldn't avoid area when it's marked with AVOID attribute
DETOUR_DECL_MEMBER5(DetourFunc_SurvivorBotPathCost_funcCallOp, float, CNavArea *, pArea, CNavArea *, pFromArea, const CNavLadder *, pLadder, const CFuncElevator *, pElevator, float, flLength)
{
	float flCost = DETOUR_MEMBER_CALL(DetourFunc_SurvivorBotPathCost_funcCallOp)(pArea, pFromArea, pLadder, pElevator, flLength);

	if (flCost <= 0.0f)
	{
		return flCost;
	}

	float flDist;

	if (pLadder)
	{
		// ladders are slow to use
		constexpr float flLadderPenalty = 2.0f; // 3.0f;
		flDist = flLadderPenalty * pLadder->m_length;
	}
	else if (flLength > 0.0)
	{
		flDist = flLength;
	}
	else
	{
		flDist = (pArea->GetCenter() - pFromArea->GetCenter()).Length();
	}

	// if this is an area to avoid, add penalty
	if (pArea->GetAttributes() & NAV_MESH_AVOID)
	{
		constexpr float flAvoidPenalty = 20.0f;
		flCost += flAvoidPenalty * flDist;
	}

	return flCost;
}

// Fix issue where bots would switch between secondary and primary weapon whenever targets on different ranges are acquired
DETOUR_DECL_MEMBER1(DetourFunc_SurvivorAttack_EquipBestWeapon, void, SurvivorBot *, pSurvivorBot)
{
	SurvivorAttack *pAction = reinterpret_cast<SurvivorAttack *>(this);

	if (pSurvivorBot)
	{
		// Prevent switching weapons too frequently
		constexpr float flTimeSinceLastSwitchedWeapon = 5.0f;

		if (pSurvivorBot->TimeSinceLastSwitchedWeaponTimer().GetElapsedTime() < flTimeSinceLastSwitchedWeapon)
		{
			return;
		}

		CBaseCombatWeapon *pActiveWeapon = pSurvivorBot->GetActiveWeapon();

		// Allow bots to properly defend themselves when dealing with multiple infected
		constexpr float flTimeSinceAttackedByEnemy = 0.5f;

		if (pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
		{
			CBaseCombatWeapon *pSecondaryWeapon = pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_PISTOL);

			if (pSecondaryWeapon
				&& pSecondaryWeapon != pActiveWeapon)	// always check this because bot has Weapon_Switch method overridden
														// which updates time since last switched weapon whenever method is called
			{
				if (FStrEq(pSecondaryWeapon->GetClassname(), "weapon_melee"))
				{
					CBaseCombatCharacter *pTarget = pAction->SelectTarget(pSurvivorBot);

					if (pTarget && pTarget->GetClass() == Zombie_Common)
					{
						const Vector& vecAbsStart = pSurvivorBot->GetAbsOrigin();
						const Vector& vecAbsEnd = pTarget->GetAbsOrigin();

						constexpr float flRange = 100.0f;

						if (CalcDistance(vecAbsStart, vecAbsEnd) <= flRange)
						{
							pSurvivorBot->Weapon_Switch(pSecondaryWeapon, 0);

							return;
						}
					}
				}
			}
		}

		// Better to have weapon reloaded before putting it away
		if (pActiveWeapon && pActiveWeapon->InReload())
		{
			return;
		}

		CBaseCombatWeapon *pPrimaryWeapon = pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_RIFLE);

		if (pPrimaryWeapon && pPrimaryWeapon != pActiveWeapon)
		{
			if (!pSurvivorBot->Weapon_Switch(pPrimaryWeapon, 0))
			{
				CBaseCombatWeapon *pSecondaryWeapon = pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_PISTOL);

				pSurvivorBot->Weapon_Switch(pSecondaryWeapon, 0);
			}
		}
		else if (pPrimaryWeapon == nullptr)
		{
			CBaseCombatWeapon *pSecondaryWeapon = pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_PISTOL);

			if (pSecondaryWeapon != pActiveWeapon)
			{
				pSurvivorBot->Weapon_Switch(pSecondaryWeapon, 0);
			}
		}
	}
}

DETOUR_DECL_MEMBER1(DetourFunc_PathFollower_Update, void, INextBot *, pNextBot)
{
	PathFollower *pPathFollower = reinterpret_cast<PathFollower *>(this);
	SurvivorBot *pSurvivorBot = pNextBot->MySurvivorBotPointer();

	if (pSurvivorBot)
	{
		CNavArea *pLastKnownArea = pSurvivorBot->GetLastKnownArea();

		// Fix issue where bots wouldn't stop moving when nav area is marked with STOP attribute
		int iBot = pSurvivorBot->entindex();

		if (pLastKnownArea)
		{
			if (pLastKnownArea->GetAttributes() & NAV_MESH_CROUCH)
			{
				NextBotPlayer_CTerrorPlayer *pNextBotTerrorPlayer = reinterpret_cast<NextBotPlayer_CTerrorPlayer *>(pSurvivorBot);

				pNextBotTerrorPlayer->PressCrouchButton();
			}

			if (SurvivorBot::m_bStopping[iBot])
			{
				if (pSurvivorBot->GetAbsVelocity().LengthSqr() >= 0.1f)
				{
					return;
				}

				SurvivorBot::m_bStopping[iBot] = false;
			}
		}

		// Fix issues where bots walk into acid spit or fire instead of waiting it out
		SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

		if (!pSurvivorIntention->IsTopPrimaryAction("SurvivorEscapeSpit") && !pSurvivorIntention->IsTopPrimaryAction("SurvivorEscapeFlames"))
		{
			Vector vecWhere;
			if (pSurvivorBot->IsPathAheadInAcidSpitOrFire(*pPathFollower, &vecWhere))
			{
				constexpr float flRange = 200.0f;

				if (!(vecWhere - pNextBot->GetLocomotionInterface()->GetFeet()).IsLengthGreaterThan(flRange))
				{
					return;
				}
			}
		}

		// Fix issue where bots wouldn't jump when nav area is marked with JUMP attribute
		if (pLastKnownArea && pLastKnownArea->GetAttributes() & NAV_MESH_JUMP)
		{
			pNextBot->GetLocomotionInterface()->Jump();
		}
	}

	DETOUR_MEMBER_CALL(DetourFunc_PathFollower_Update)(pNextBot);
}

// Fix issue where bots would climb/jump despite nav area being marked with NO_JUMP attribute
DETOUR_DECL_MEMBER5(DetourFunc_PathFollower_Climbing, bool, INextBot *, pNextBot, const Path::Segment *, pGoal, const Vector &, vecForward, const Vector &, vecRight, float, flGoalRange)
{
	SurvivorBot *pSurvivorBot = pNextBot->MySurvivorBotPointer();

	if (pSurvivorBot)
	{
		CNavArea *pLastKnownArea = pSurvivorBot->GetLastKnownArea();

		if (pLastKnownArea && pLastKnownArea->GetAttributes() & NAV_MESH_NO_JUMP)
		{
			return false;
		}
	}

	return DETOUR_MEMBER_CALL(DetourFunc_PathFollower_Climbing)(pNextBot, pGoal, vecForward, vecRight, flGoalRange);
}

DETOUR_DECL_MEMBER2(DetourFunc_NextBotTraversableTraceFilter_ShouldHitEntity, bool, IHandleEntity *, pServerEntity, int, fContentsMask)
{
	INextBot *pNextBot = reinterpret_cast<NextBotTraversableTraceFilter *>(this)->m_bot;

	if (pNextBot->MySurvivorBotPointer())
	{
		CBaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);

		if (!pEntity)
		{
			return false;
		}

		// Blockers aren't traversable
		if (fContentsMask & CONTENTS_TEAM1)
		{
			const char *pszClassname = pEntity->GetClassname();

			if (FStrEq(pszClassname, "env_physics_blocker"))
			{
				return true;
			}

			if (FStrEq(pszClassname, "env_player_blocker"))
			{
				return true;
			}
		}
	}

	return DETOUR_MEMBER_CALL(DetourFunc_NextBotTraversableTraceFilter_ShouldHitEntity)(pServerEntity, fContentsMask);
}

bool CImprovedSurvivorBots::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	static const struct
	{
		const char *m_pszClassname;
		const char *m_pszOffset;
		int &m_nOffset;
	}
	s_stSendProps[] =
	{
		{ "CBaseEntity", "m_flSimulationTime", CBaseEntity::m_nOffset_SendProp_m_flSimulationTime },
		{ "CBaseEntity", "m_iTeamNum", CBaseEntity::m_nOffset_SendProp_m_iTeamNum },
		{ "CBasePlayer", "m_lifeState", CBaseEntity::m_nOffset_m_lifeState },

		{ "CTerrorPlayer", "m_iAmmo", CBaseCombatCharacter::m_nOffset_SendProp_m_iAmmo },
		{ "CTerrorPlayer", "m_hActiveWeapon", CBaseCombatCharacter::m_nOffset_SendProp_m_hActiveWeapon },

		{ "CTerrorPlayer", "m_pummelAttacker", CTerrorPlayer::m_nOffset_SendProp_m_pummelAttacker },
		{ "CTerrorPlayer", "m_pummelVictim", CTerrorPlayer::m_nOffset_SendProp_m_pummelVictim },
		{ "CTerrorPlayer", "m_carryAttacker", CTerrorPlayer::m_nOffset_SendProp_m_carryAttacker },
		{ "CTerrorPlayer", "m_pounceAttacker", CTerrorPlayer::m_nOffset_SendProp_m_pounceAttacker },
		{ "CTerrorPlayer", "m_jockeyAttacker", CTerrorPlayer::m_nOffset_SendProp_m_jockeyAttacker },
		{ "CTerrorPlayer", "m_tongueOwner", CTerrorPlayer::m_nOffset_SendProp_m_tongueOwner },
		{ "CTerrorPlayer", "m_reviveTarget", CTerrorPlayer::m_nOffset_SendProp_m_reviveTarget },
		{ "CTerrorPlayer", "m_isIncapacitated", CTerrorPlayer::m_nOffset_SendProp_m_isIncapacitated },
		{ "CTerrorPlayer", "m_bAdrenalineActive", CTerrorPlayer::m_nOffset_SendProp_m_bAdrenalineActive },
		{ "CTerrorPlayer", "m_bIsOnThirdStrike", CTerrorPlayer::m_nOffset_SendProp_m_bIsOnThirdStrike },
		{ "CTerrorPlayer", "m_flProgressBarStartTime", CTerrorPlayer::m_nOffset_SendProp_m_flProgressBarStartTime },
		{ "CTerrorPlayer", "m_flProgressBarDuration", CTerrorPlayer::m_nOffset_SendProp_m_flProgressBarDuration },
		{ "CTerrorPlayer", "m_useActionOwner", CTerrorPlayer::m_nOffset_SendProp_m_useActionOwner },
		{ "CTerrorPlayer", "m_iCurrentUseAction", CTerrorPlayer::m_nOffset_SendProp_m_iCurrentUseAction },
		{ "CTerrorPlayer", "m_reachedTongueOwner", CTerrorPlayer::m_nOffset_SendProp_m_reachedTongueOwner },
	};

	for (auto &&elem : s_stSendProps)
	{
		sm_sendprop_info_t info;
		if (gamehelpers->FindSendPropInfo(elem.m_pszClassname, elem.m_pszOffset, &info))
		{
			elem.m_nOffset = info.actual_offset;
		}
		else
		{
			ke::SafeSprintf(error, maxlen, "Unable to find SendProp info for \"%s:%s\"", elem.m_pszClassname, elem.m_pszOffset);

			return false;
		}
	}

	IGameConfig *pGameConfig;

	if (!gameconfs->LoadGameConfigFile("sdktools.games", &pGameConfig, error, maxlen))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to load gamedata file \"sdktools.games.txt\"");

		return false;
	}

	if (!pGameConfig->GetOffset("Weapon_GetSlot", &CBaseCombatCharacter::m_iVtbl_Weapon_GetSlot))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to find gamedata offset entry for \"Weapon_GetSlot\"");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	gameconfs->CloseGameConfigFile(pGameConfig);

	if (!gameconfs->LoadGameConfigFile("sdkhooks.games", &pGameConfig, error, maxlen))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to load gamedata file \"sdkhooks.games.txt\"");

		return false;
	}

	if (!pGameConfig->GetOffset("Weapon_Switch", &CBaseCombatCharacter::m_iVtbl_Weapon_Switch))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to find gamedata offset entry for \"Weapon_Switch\"");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	gameconfs->CloseGameConfigFile(pGameConfig);

	if (!gameconfs->LoadGameConfigFile(SMEXT_CONF_GAMEDATA_FILE, &pGameConfig, error, sizeof(error)))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to load gamedata file \"" SMEXT_CONF_GAMEDATA_FILE ".txt\"");

		return false;
	}

	if (!pGameConfig->GetAddress("CDirector", &m_pObj_TheDirector))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to get address of CDirector instance");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	static const struct
	{
		const char *m_pszKey;
		void *&m_pAddress;
	}
	s_stSignatures[] =
	{
		{ "SurvivorIntention::IsImmediatelyDangerousTo", SurvivorIntention::m_pFn_IsImmediatelyDangerousTo },

		{ "NavAreaTravelDistance<ShortestPathCost>", m_pFn_NavAreaTravelDistance_ShortestPathCost },

		{ "SurvivorBot::IsReachable_Player", SurvivorBot::m_pFn_IsReachable_Player },

		{ "CTerrorPlayer::GetTimeSinceAttackedByEnemy", CTerrorPlayer::m_pFn_GetTimeSinceAttackedByEnemy },
		{ "CTerrorPlayer::StopRevivingSomeone", CTerrorPlayer::m_pFn_StopRevivingSomeone },

		{ "CInferno::IsTouching_VVV", CInferno::m_pFn_IsTouching_VVV },

		{ "SurvivorUseObject::ShouldGiveUp", SurvivorUseObject::m_pFn_ShouldGiveUp },

		{ "SurvivorAttack::SelectTarget", SurvivorAttack::m_pFn_SelectTarget },

		{ "IsBeingHeroic condition", m_pSurvivorBot_IsBeingHeroic_Condition },
		{ "FireWeapon HasPlayerControlledZombies condition", m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon] },
		{ "OnSound HasPlayerControlledZombies condition", m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound] },
	};

	for (auto &&elem : s_stSignatures)
	{
		if (!pGameConfig->GetMemSig(elem.m_pszKey, &elem.m_pAddress))
		{
			ke::SafeSprintf(error, maxlen, "Unable to find gamedata signature entry for \"%s\"", elem.m_pszKey);

			gameconfs->CloseGameConfigFile(pGameConfig);

			return false;
		}

		if (!elem.m_pAddress)
		{
			ke::SafeSprintf(error, maxlen, "Unable to find signature in binary for gamedata entry \"%s\"", elem.m_pszKey);

			gameconfs->CloseGameConfigFile(pGameConfig);

			return false;
		}
	}

	static const struct
	{
		const char *m_pszKey;
		int &m_nOffset;
	}
	s_stOffsets[] =
	{
		{ "Action<Actor>::OnStart", m_iVtbl_Action_OnStart },
		{ "Action<Actor>::Update", m_iVtbl_Action_Update },
		{ "Action<Actor>::OnEnd", m_iVtbl_Action_OnEnd },
		{ "Action<Actor>::OnSuspend", m_iVtbl_Action_OnSuspend },
		{ "Action<Actor>::InitialContainedAction", m_iVtbl_Action_InitialContainedAction },

		{ "SurvivorUseObject::OnStartUse", m_iVtbl_SurvivorUseObject_OnStartUse },
		{ "SurvivorUseObject::ShouldGiveUp", m_iVtbl_SurvivorUseObject_ShouldGiveUp },
		#if SOURCE_ENGINE == SE_LEFT4DEAD2
		{ "Action<Actor>::OnEnteredSpit", m_iVtbl_Action_OnEnteredSpit },
		#endif
		{ "Action<Actor>::OnCommandApproach", m_iVtbl_Action_OnCommandApproach },
		{ "Action<Actor>::OnIgnite", m_iVtbl_Action_OnIgnite },
		{ "Action<Actor>::OnInjured", m_iVtbl_Action_OnInjured },

		{ "IIntention::OnInjured", m_iVtbl_IIntention_OnInjured },
		{ "IIntention::Reset", m_iVtbl_IIntention_Reset },

		{ "IBody::GetSolidMask", m_iVtbl_IBody_GetSolidMask },
		{ "ILocomotion::ClimbUpToLedge", m_iVtbl_ILocomotion_ClimbUpToLedge },
		{ "ILocomotion::IsRunning", m_iVtbl_ILocomotion_IsRunning },

		{ "CBasePlayer::OnNavAreaChanged", m_iVtbl_CBasePlayer_OnNavAreaChanged },

		{ "SurvivorBot::m_teamSituation", SurvivorBot::m_nOffset_m_teamSituation },
		{ "SurvivorBot::m_timeSinceLastSwitchedWeaponTimer", SurvivorBot::m_nOffset_m_timeSinceLastSwitchedWeaponTimer },

		{ "NextBotPlayer<CTerrorPlayer>::m_inputButtons", NextBotPlayer_CTerrorPlayer::m_nOffset_m_inputButtons },
		{ "NextBotPlayer<CTerrorPlayer>::m_fireButtonTimer", NextBotPlayer_CTerrorPlayer::m_nOffset_m_fireButtonTimer },
		{ "NextBotPlayer<CTerrorPlayer>::m_meleeButtonTimer", NextBotPlayer_CTerrorPlayer::m_nOffset_m_meleeButtonTimer },
		{ "NextBotPlayer<CTerrorPlayer>::m_crouchButtonTimer", NextBotPlayer_CTerrorPlayer::m_nOffset_m_crouchButtonTimer },
		{ "NextBotPlayer<CTerrorPlayer>::m_nextBotPointer", NextBotPlayer_CTerrorPlayer::m_nOffset_m_nextBotPointer },

		{ "CBaseCombatCharacter::GetClass", CBaseCombatCharacter::m_iVtbl_GetClass },
		{ "CBaseCombatCharacter::IsIT", CBaseCombatCharacter::m_iVtbl_IsIT },
		{ "CBaseCombatCharacter::m_lastKnownArea", CBaseCombatCharacter::m_nOffset_m_lastKnownArea },

		{ "CCSPlayer::GetHealthBuffer", CCSPlayer::m_iVtbl_GetHealthBuffer },

		{ "Witch::m_hHarasser", Witch::m_nOffset_m_hHarasser },

		{ "SurvivorTeamSituation::m_me", SurvivorTeamSituation::m_nOffset_m_me },
		{ "SurvivorTeamSituation::m_friendInTrouble", SurvivorTeamSituation::m_nOffset_m_friendInTrouble },
		{ "SurvivorTeamSituation::m_humanFriendInTrouble", SurvivorTeamSituation::m_nOffset_m_humanFriendInTrouble },
		{ "SurvivorTeamSituation::m_humanLeader", SurvivorTeamSituation::m_nOffset_m_humanLeader },
		{ "SurvivorTeamSituation::m_tonguedFriend", SurvivorTeamSituation::m_nOffset_m_tonguedFriend },
		{ "SurvivorTeamSituation::m_pouncedFriend", SurvivorTeamSituation::m_nOffset_m_pouncedFriend },
		{ "SurvivorTeamSituation::m_pummeledFriend", SurvivorTeamSituation::m_nOffset_m_pummeledFriend },
		{ "SurvivorTeamSituation::m_jockeyedFriend", SurvivorTeamSituation::m_nOffset_m_jockeyedFriend },

		{ "CDirector::m_iTankCount", CDirector::m_nOffset_m_iTankCount },

		{ "CNavArea::m_center", CNavArea::m_nOffset_m_center },
		{ "CNavArea::m_attributeFlags", CNavArea::m_nOffset_m_attributeFlags },
	};

	for (auto &&elem : s_stOffsets)
	{
		if (!pGameConfig->GetOffset(elem.m_pszKey, &elem.m_nOffset))
		{
			ke::SafeSprintf(error, maxlen, "Unable to find gamedata offset entry for \"%s\"", elem.m_pszKey);

			gameconfs->CloseGameConfigFile(pGameConfig);

			return false;
		}
	}

	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnStart, m_iVtbl_Action_OnStart, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_Update, m_iVtbl_Action_Update, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnEnd, m_iVtbl_Action_OnEnd, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnSuspend, m_iVtbl_Action_OnSuspend, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_InitialContainedAction, m_iVtbl_Action_InitialContainedAction, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_SurvivorUseObject_OnStartUse, m_iVtbl_SurvivorUseObject_OnStartUse, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_SurvivorUseObject_ShouldGiveUp, m_iVtbl_SurvivorUseObject_ShouldGiveUp, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnIgnite, m_iVtbl_Action_OnIgnite, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnInjured, m_iVtbl_Action_OnInjured, 0, 0);
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnEnteredSpit, m_iVtbl_Action_OnEnteredSpit, 0, 0);
#endif
	SH_MANUALHOOK_RECONFIGURE(MHook_Action_OnCommandApproach, m_iVtbl_Action_OnCommandApproach, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_CBaseCombatCharacter_IsIT, CBaseCombatCharacter::m_iVtbl_IsIT, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_IIntention_OnInjured, m_iVtbl_IIntention_OnInjured, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_IIntention_Reset, m_iVtbl_IIntention_Reset, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_IBody_GetSolidMask, m_iVtbl_IBody_GetSolidMask, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_ILocomotion_ClimbUpToLedge, m_iVtbl_ILocomotion_ClimbUpToLedge, 0, 0);
	SH_MANUALHOOK_RECONFIGURE(MHook_ILocomotion_IsRunning, m_iVtbl_ILocomotion_IsRunning, 0, 0);

	SH_MANUALHOOK_RECONFIGURE(MHook_CBasePlayer_OnNavAreaChanged, m_iVtbl_CBasePlayer_OnNavAreaChanged, 0, 0);

	CDetourManager::Init(smutils->GetScriptingEngine(), pGameConfig);
#define DETOUR_CREATE_MEMBER_WRAPPER(Name, GameData, Enable) \
	m_pDetour_##Name = DETOUR_CREATE_MEMBER(DetourFunc_##Name, GameData); \
	\
	if (m_pDetour_##Name) \
	{ \
		if (Enable) \
			m_pDetour_##Name->EnableDetour(); \
		\
		m_vecDetours.AddToTail(m_pDetour_##Name); \
	} \
	else \
	{ \
		ke::SafeStrcpy(error, maxlen, "Unable to create member detour \"" GameData "\""); \
		\
		gameconfs->CloseGameConfigFile(pGameConfig); \
		\
		SDK_OnUnload(); \
		\
		return false; \
	} \

#define DETOUR_CREATE_STATIC_WRAPPER(Name, GameData, Enable) \
	m_pDetour_##Name = DETOUR_CREATE_STATIC(DetourFunc_##Name, GameData); \
	\
	if (m_pDetour_##Name) \
	{ \
		if (Enable) \
			m_pDetour_##Name->EnableDetour(); \
		\
		m_vecDetours.AddToTail(m_pDetour_##Name); \
	} \
	else \
	{ \
		ke::SafeStrcpy(error, maxlen, "Unable to create static detour \"" GameData "\""); \
		\
		gameconfs->CloseGameConfigFile(pGameConfig); \
		\
		SDK_OnUnload(); \
		\
		return false; \
	} \

#ifdef __linux__
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorBot_UpdateTeamSituation, "SurvivorBot::UpdateTeamSituation", true);
#elif defined _WIN32
	DETOUR_CREATE_STATIC_WRAPPER(SurvivorBot_UpdateTeamSituation, "SurvivorBot::UpdateTeamSituation", true);
#endif
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorBot_SaveFriendsInImmediateTrouble, "SurvivorBot::SaveFriendsInImmediateTrouble", true);
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorBot_GetEscortRange, "SurvivorBot::GetEscortRange", true);
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorBot_ScavengeNearbyItems, "SurvivorBot::ScavengeNearbyItems", true);

	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorBotPathCost_funcCallOp, "SurvivorBotPathCost::operator()", true);

	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorAttack_EquipBestWeapon, "SurvivorAttack::EquipBestWeapon", true);

	DETOUR_CREATE_MEMBER_WRAPPER(PathFollower_Update, "PathFollower::Update", true);
	DETOUR_CREATE_MEMBER_WRAPPER(PathFollower_Climbing, "PathFollower::Climbing", true);

	DETOUR_CREATE_MEMBER_WRAPPER(NextBotTraversableTraceFilter_ShouldHitEntity, "NextBotTraversableTraceFilter::ShouldHitEntity", true);

	DETOUR_CREATE_MEMBER_WRAPPER(CTerrorPlayer_GetTimeSinceAttackedByEnemy, "CTerrorPlayer::GetTimeSinceAttackedByEnemy", false);
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorIntention_IsImmediatelyDangerousTo, "SurvivorIntention::IsImmediatelyDangerousTo", false);
	DETOUR_CREATE_MEMBER_WRAPPER(SurvivorUseObject_OnStartUse, "SurvivorUseObject::OnStartUse", false);

	gameconfs->CloseGameConfigFile(pGameConfig);

	g_pEntityList = reinterpret_cast<CBaseEntityList *>(gamehelpers->GetGlobalEntityList());

	sharesys->AddDependency(myself, "imatchext.ext", true, true);
	sharesys->AddDependency(myself, "bintools.ext", true, true);

	Patch(true);

	ConVar_Register(0, this);

	playerhelpers->AddClientListener(this);

	if (playerhelpers->IsServerActivated())
	{
		OnServerActivated(playerhelpers->GetMaxClients());
	}

	return true;
}

void CImprovedSurvivorBots::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, bintools);
	SM_GET_LATE_IFACE(IMATCHEXT, g_pMatchExtInterface);

	if (bintools == nullptr)
	{
		return;
	}
#if SMINTERFACE_BINTOOLS_VERSION == 4
	PassInfo passInfo_Bool = { PassType_Basic, PASSFLAG_BYVAL, sizeof(bool), nullptr, 0 };
	PassInfo passInfo_Float = { PassType_Float, PASSFLAG_BYVAL, sizeof(float), nullptr, 0 };
	PassInfo passInfo_Int = { PassType_Basic, PASSFLAG_BYVAL, sizeof(int), nullptr, 0 };
	PassInfo passInfo_Pointer = { PassType_Basic, PASSFLAG_BYVAL, sizeof(void *), nullptr, 0 };
#else
	PassInfo passInfo_Bool = { PassType_Basic, PASSFLAG_BYVAL, sizeof(bool) };
	PassInfo passInfo_Float = { PassType_Float, PASSFLAG_BYVAL, sizeof(float) };
	PassInfo passInfo_Int = { PassType_Basic, PASSFLAG_BYVAL, sizeof(int) };
	PassInfo passInfo_Pointer = { PassType_Basic, PASSFLAG_BYVAL, sizeof(void *) };
#endif
	PassInfo passInfo_NavAreaTravelDistance_ShortestPathCost[] =
	{
		{ passInfo_Pointer },
		{ passInfo_Pointer },
		{ passInfo_Pointer },
		{ passInfo_Float },
	};

	PassInfo passInfo_CBaseCombatCharacter_Weapon_Switch[] =
	{
		{ passInfo_Pointer },
		{ passInfo_Int },
	};

	PassInfo passInfo_SurvivorIntention_IsImmediatelyDangerousTo[] =
	{
		{ passInfo_Pointer },
		{ passInfo_Pointer },
	};

	PassInfo passInfo_CInferno_IsTouching[] =
	{
		{ passInfo_Pointer },
		{ passInfo_Pointer },
		{ passInfo_Pointer },
	};
#define CALL_WRAPPER_CREATE_STATIC(Name, GameData, RetInfo, ParamInfo, NumParams) \
	m_pCallWrap_##Name = bintools->CreateCall(m_pFn_##Name, CallConv_Cdecl, RetInfo, ParamInfo, NumParams); \
	\
	if (m_pCallWrap_##Name) \
	{ \
		m_vecCallWrappers.AddToTail(m_pCallWrap_##Name); \
	} \
	else \
	{ \
		smutils->LogError(myself, "Unable to create call for \"" GameData "\""); \
	} \

#define CALL_WRAPPER_CREATE_MEMBER(Class, Name, GameData, RetInfo, ParamInfo, NumParams) \
	Class::m_pCallWrap_##Name = bintools->CreateCall(Class::m_pFn_##Name, CallConv_ThisCall, RetInfo, ParamInfo, NumParams); \
	\
	if (Class::m_pCallWrap_##Name) \
	{ \
		m_vecCallWrappers.AddToTail(Class::m_pCallWrap_##Name); \
	} \
	else \
	{ \
		smutils->LogError(myself, "Unable to create call for \"" GameData "\""); \
	} \

#define VCALL_WRAPPER_CREATE(Class, Name, GameData, RetInfo, ParamInfo, NumParams) \
	Class::m_pCallWrap_##Name = bintools->CreateVCall(Class::m_iVtbl_##Name, 0, 0, RetInfo, ParamInfo, NumParams); \
	\
	if (Class::m_pCallWrap_##Name) \
	{ \
		m_vecCallWrappers.AddToTail(Class::m_pCallWrap_##Name); \
	} \
	else \
	{ \
		smutils->LogError(myself, "Unable to create virtual call for \"" GameData "\""); \
	} \

	CALL_WRAPPER_CREATE_STATIC(NavAreaTravelDistance_ShortestPathCost, "NavAreaTravelDistance<ShortestPathCost>", &passInfo_Float, passInfo_NavAreaTravelDistance_ShortestPathCost, 4);

	CALL_WRAPPER_CREATE_MEMBER(SurvivorBot, IsReachable_Player, "SurvivorBot::IsReachable_Player", &passInfo_Bool, &passInfo_Pointer, 1);
	CALL_WRAPPER_CREATE_MEMBER(SurvivorIntention, IsImmediatelyDangerousTo, "SurvivorIntention::IsImmediatelyDangerousTo", &passInfo_Bool, passInfo_SurvivorIntention_IsImmediatelyDangerousTo, 2);
	CALL_WRAPPER_CREATE_MEMBER(CTerrorPlayer, GetTimeSinceAttackedByEnemy, "CTerrorPlayer::GetTimeSinceAttackedByEnemy", &passInfo_Float, nullptr, 0);
	CALL_WRAPPER_CREATE_MEMBER(CTerrorPlayer, StopRevivingSomeone, "CTerrorPlayer::StopRevivingSomeone", nullptr, &passInfo_Bool, 1);
	CALL_WRAPPER_CREATE_MEMBER(CInferno, IsTouching_VVV, "CInferno::IsTouching_VVV", &passInfo_Bool, passInfo_CInferno_IsTouching, 3);
	CALL_WRAPPER_CREATE_MEMBER(SurvivorUseObject, ShouldGiveUp, "SurvivorUseObject::ShouldGiveUp", &passInfo_Bool, &passInfo_Pointer, 1);
	CALL_WRAPPER_CREATE_MEMBER(SurvivorAttack, SelectTarget, "SurvivorAttack::SelectTarget", &passInfo_Pointer, &passInfo_Pointer, 1);

	VCALL_WRAPPER_CREATE(CBaseCombatCharacter, GetClass, "CBaseCombatCharacter::GetClass", &passInfo_Int, nullptr, 0);
	VCALL_WRAPPER_CREATE(CBaseCombatCharacter, Weapon_GetSlot, "CBaseCombatCharacter::Weapon_GetSlot", &passInfo_Int, &passInfo_Pointer, 1);
	VCALL_WRAPPER_CREATE(CBaseCombatCharacter, Weapon_Switch, "CBaseCombatCharacter::Weapon_Switch", &passInfo_Bool, passInfo_CBaseCombatCharacter_Weapon_Switch, 2);
	VCALL_WRAPPER_CREATE(CCSPlayer, GetHealthBuffer, "CCSPlayer::GetHealthBuffer", &passInfo_Float, nullptr, 0);
}

void CImprovedSurvivorBots::SDK_OnUnload()
{
	Patch(false);

	ConVar_Unregister();
#define SH_REMOVE_HOOK_ID_WRAPPER(Name) \
	if (m_nSHookID_##Name == 0) \
	{ \
		SH_REMOVE_HOOK_ID(m_nSHookID_##Name); \
		\
		m_nSHookID_##Name = 0; \
	} \

	SH_REMOVE_HOOK_ID_WRAPPER(SurvivorBot_OnNavAreaChanged);

	SH_REMOVE_HOOK_ID_WRAPPER(SurvivorIntention_OnInjured);
	SH_REMOVE_HOOK_ID_WRAPPER(SurvivorIntention_Reset);

	SH_REMOVE_HOOK_ID_WRAPPER(PlayerBody_GetSolidMask);

	SH_REMOVE_HOOK_ID_WRAPPER(PlayerLocomotion_ClimbUpToLedge);
	SH_REMOVE_HOOK_ID_WRAPPER(PlayerLocomotion_IsRunning);

	FOR_EACH_VEC(m_vecDetours, iter)
	{
		if (m_vecDetours[iter])
		{
			m_vecDetours[iter]->Destroy();
			m_vecDetours[iter] = nullptr;
		}
	}

	FOR_EACH_VEC(m_vecCallWrappers, iter)
	{
		if (m_vecCallWrappers[iter])
		{
			m_vecCallWrappers[iter]->Destroy();
			m_vecCallWrappers[iter] = nullptr;
		}
	}

	for (int iHook = 0; iHook < ActionHook_MAXHOOKS; iHook++)
	{
		SH_REMOVE_HOOK_ID_WRAPPER(Action[iHook]);
	}

// Currently causes a server crash on windows
#if !defined _WIN32
	// Iterate through all survivor bots and call PopLegsStack function on Actions that were made in this extension
	// to prevent server crash
	for (int iClient = 1; iClient <= playerhelpers->GetMaxClients(); iClient++)
	{
		SurvivorBot *pSurvivorBot = dynamic_cast<SurvivorBot *>(gamehelpers->ReferenceToEntity(iClient));

		if (pSurvivorBot)
		{
			SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

			Behavior<SurvivorBot> *pBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pSurvivorIntention->FirstContainedResponder());

			if (pBehavior)
			{
				Action<SurvivorBot> *pPrimaryAction = dynamic_cast<Action< SurvivorBot > *>(pBehavior->FirstContainedResponder());

				if (pPrimaryAction && pPrimaryAction->IsNamed("SurvivorDispatchEnemy"))
				{
					pSurvivorIntention->PopLegsStack();
				}
			}
		}
	}
#endif

	playerhelpers->RemoveClientListener(this);
}

bool CImprovedSurvivorBots::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	gpGlobals = ismm->GetCGlobals();

	GET_V_IFACE_CURRENT(GetServerFactory, serverGameEnts, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, staticpropmgr, IStaticPropMgrServer, INTERFACEVERSION_STATICPROPMGR_SERVER);

	return true;
}

bool CImprovedSurvivorBots::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(BINTOOLS, bintools);
	SM_CHECK_IFACE(IMATCHEXT, g_pMatchExtInterface);

	return true;
}

bool CImprovedSurvivorBots::QueryInterfaceDrop(SMInterface *pInterface)
{
	if (pInterface == bintools)
	{
		return false;
	}

	return true;
}

void CImprovedSurvivorBots::NotifyInterfaceDrop(SMInterface *pInterface)
{
	SDK_OnUnload();
}

void CImprovedSurvivorBots::OnClientPutInServer(int iClient)
{
	SurvivorBot *pSurvivorBot = dynamic_cast<SurvivorBot *>(gamehelpers->ReferenceToEntity(iClient));

	if (pSurvivorBot)
	{
#define SH_MANUALVPHOOK_PRE_WRAPPER(Name, HookName, Instance) \
	if (m_nSHookID_##Name == 0) \
	{ \
		m_nSHookID_##Name = SH_ADD_MANUALVPHOOK(MHook_##HookName, Instance, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##Name), false); \
	} \

#define SH_MANUALVPHOOK_POST_WRAPPER(Name, HookName, Instance) \
	if (m_nSHookID_##Name == 0) \
	{ \
		m_nSHookID_##Name = SH_ADD_MANUALVPHOOK(MHook_##HookName, Instance, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##Name##_Post), true); \
	} \

		SH_MANUALVPHOOK_POST_WRAPPER(SurvivorBot_OnNavAreaChanged, CBasePlayer_OnNavAreaChanged, pSurvivorBot);

		SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

		SH_MANUALVPHOOK_PRE_WRAPPER(SurvivorIntention_OnInjured, IIntention_OnInjured, pSurvivorIntention);
		SH_MANUALVPHOOK_POST_WRAPPER(SurvivorIntention_Reset, IIntention_Reset, pSurvivorIntention);

		SH_MANUALVPHOOK_POST_WRAPPER(PlayerBody_GetSolidMask, IBody_GetSolidMask, pSurvivorBot->GetBodyInterface());

		SH_MANUALVPHOOK_PRE_WRAPPER(PlayerLocomotion_ClimbUpToLedge, ILocomotion_ClimbUpToLedge, pSurvivorBot->GetLocomotionInterface());
		SH_MANUALVPHOOK_PRE_WRAPPER(PlayerLocomotion_IsRunning, ILocomotion_IsRunning, pSurvivorBot->GetLocomotionInterface());

		Behavior<SurvivorBot> *pBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pSurvivorIntention->FirstContainedResponder());

		if (pBehavior)
		{
			Action<SurvivorBot> *pAction = dynamic_cast<Action< SurvivorBot > *>(pBehavior->FirstContainedResponder());

			if (pAction)
			{
				if (pAction->IsNamed("SurvivorBehavior"))
				{
					SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnSuspend, pAction);

					SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnIgnite, pAction);
					#if SOURCE_ENGINE == SE_LEFT4DEAD2
					SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnEnteredSpit, pAction);
					#endif
					SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnCommandApproach, pAction);

					SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorAttack, OnSuspend, pAction);
				}
				else if (!pAction->IsNamed("L4D1SurvivorBehavior"))		// In case extension is loaded late
				{
					Action<SurvivorBot> *pActionBuriedUnderMe = pAction->GetActionBuriedUnderMe();

					if (pActionBuriedUnderMe && pActionBuriedUnderMe->IsNamed("SurvivorBehavior"))
					{
						SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnSuspend, pActionBuriedUnderMe);

						SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnIgnite, pActionBuriedUnderMe);
						#if SOURCE_ENGINE == SE_LEFT4DEAD2
						SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorBehavior, OnEnteredSpit, pActionBuriedUnderMe);
						#endif

						Hook_SurvivorBehavior_OnSuspend_Post(pSurvivorBot, pAction);

						// We can hook instance here directly, so there's no need to hook OnCommandApproach as well
						if (pAction->IsNamed("SurvivorDebugApproach"))
						{
							SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorDebugApproach, InitialContainedAction, pAction);
						}

						pActionBuriedUnderMe = pAction->GetActionBuriedUnderMe();

						while (pActionBuriedUnderMe)
						{
							Hook_SurvivorBehavior_OnSuspend_Post(pSurvivorBot, pActionBuriedUnderMe);

							pActionBuriedUnderMe = pActionBuriedUnderMe->GetActionBuriedUnderMe();
						}
					}
				}
			}
		}

		INextBotEventResponder *pNextContainedResponder = pSurvivorIntention->m_behaviorLegsStack[0][0];

		Behavior<SurvivorBot> *pNextBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pNextContainedResponder);

		if (pNextBehavior)
		{
			Action<SurvivorBot> *pAction = dynamic_cast<Action< SurvivorBot > *>(pNextBehavior->FirstContainedResponder());

			if (pAction)
			{
				if (pAction->IsNamed("SurvivorLegsStayClose"))
				{
					SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorLegsStayClose, Update, pAction);

					Action<SurvivorBot> *pInitialContainedAction = pAction->InitialContainedAction(pSurvivorBot);

					// Initial contained Action will be SurvivorLegsWait if allow_all_bot_survivor_team ConVar is 0
					if (pInitialContainedAction->IsNamed("SurvivorLegsMoveOn"))
					{
						SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorLegsMoveOn, OnSuspend, pInitialContainedAction);
					}

					// In case extension is loaded late
					Action<SurvivorBot> *pActiveChildAction = pAction->GetActiveChildAction();

					if (pActiveChildAction && pActiveChildAction->IsNamed("SurvivorLegsBattleStations"))
					{
						Hook_SurvivorLegsMoveOn_OnSuspend_Post(pSurvivorBot, pActiveChildAction);
					}
				}
				else if (pAction->IsNamed("L4D1SurvivorLegsBattleStations"))	// In case extension is loaded late
				{
					SH_ACTION_VPHOOK_POST_WRAPPER(L4D1SurvivorLegsBattleStations, Update, pAction);
				}
				else
				{
					Action<SurvivorBot> *pActionBuriedUnderMe = pAction->GetActionBuriedUnderMe();

					if (pActionBuriedUnderMe && pActionBuriedUnderMe->IsNamed("SurvivorLegsStayClose"))
					{
						SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorLegsStayClose, Update, pActionBuriedUnderMe);

						Action<SurvivorBot> *pInitialContainedAction = pActionBuriedUnderMe->InitialContainedAction(pSurvivorBot);

						// Initial contained Action will be SurvivorLegsWait if allow_all_bot_survivor_team ConVar is 0
						if (pInitialContainedAction->IsNamed("SurvivorLegsMoveOn"))
						{
							SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorLegsMoveOn, OnSuspend, pInitialContainedAction);
						}

						Action<SurvivorBot> *pActiveChildAction = pActionBuriedUnderMe->GetActiveChildAction();

						if (pActiveChildAction && pActiveChildAction->IsNamed("SurvivorLegsBattleStations"))
						{
							Hook_SurvivorLegsMoveOn_OnSuspend_Post(pSurvivorBot, pActiveChildAction);
						}
					}
				}
			}
		}
	}
}

void CImprovedSurvivorBots::OnServerActivated(int nMaxClients)
{
	for (int iClient = 1; iClient <= nMaxClients; iClient++)
	{
		SurvivorBot *pSurvivorBot = dynamic_cast<SurvivorBot *>(gamehelpers->ReferenceToEntity(iClient));

		if (pSurvivorBot)
		{
			OnClientPutInServer(iClient);
		}
	}
}

bool CImprovedSurvivorBots::RegisterConCommandBase(ConCommandBase *pVar)
{
	/* Always call META_REGCVAR instead of going through the engine. */
	return META_REGCVAR(pVar);
}

void CImprovedSurvivorBots::Patch(bool bPatch)
{
#ifdef _WIN32
	#define BYTES_ARRAY_SIZE 6
	static int s_nOriginalBytes[BYTES_ARRAY_SIZE];
#endif

	if (bPatch)
	{
		// Fix issue where bots would just stand still and watch survivor get choked by smoker
		SourceHook::SetMemAccess(m_pSurvivorBot_IsBeingHeroic_Condition, sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

		#ifdef __linux__
		*reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) = 0x90;
		*(reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) + 1) = 0xE9;
		#elif defined _WIN32
		*(reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) + 7) = 0xEB;
		#endif

		SourceHook::SetMemAccess(m_pSurvivorBot_IsBeingHeroic_Condition, sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);

		// Fix issue where bots would never fire their weapon while being dragged by smoker, in versus mode
		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon], sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

		#ifdef __linux
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon]) + 7) = 0x90;
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon]) + 8) = 0x90;
		#elif defined _WIN32
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon]) + 7) = 0xEB;
		#endif

		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon], sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);

		// Fix issue where bots would ignore sound Infected make, in versus mode
		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound], sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

		#ifdef __linux__
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound]) + 7) = 0xEB;
		#elif defined _WIN32
		static int s_nPatchBytes[BYTES_ARRAY_SIZE] = { 0x90, 0x31, 0xC0, 0x0F, 0x1F, 0x00 };

		for (int iByte = 0; iByte < BYTES_ARRAY_SIZE; iByte++)
		{
			s_nOriginalBytes[iByte] = *(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound]) + iByte);

			*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound]) + iByte) = s_nPatchBytes[iByte];
		}
		#endif

		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound], sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);
	}
	else
	{
		SourceHook::SetMemAccess(m_pSurvivorBot_IsBeingHeroic_Condition, sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

		#ifdef __linux__
		*reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) = 0x0F;
		*(reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) + 1) = 0x84;
		#elif defined _WIN32
		*(reinterpret_cast<byte *>(m_pSurvivorBot_IsBeingHeroic_Condition) + 7) = 0x74;
		#endif

		SourceHook::SetMemAccess(m_pSurvivorBot_IsBeingHeroic_Condition, sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);

		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon], sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon]) + 7) = 0x74;
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon]) + 8) = 0x36;
		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorAttack_FireWeapon], sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);

		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound], sizeof(byte), SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

		#ifdef __linux__
		*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound]) + 11) = 0x74;
		#elif defined _WIN32
		for (int iByte = 0; iByte < BYTES_ARRAY_SIZE; iByte++)
		{
			*(reinterpret_cast<byte *>(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound]) + iByte) = s_nOriginalBytes[iByte];
		}
		#endif

		SourceHook::SetMemAccess(m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_SurvivorIntention_OnSound], sizeof(byte), SH_MEM_READ | SH_MEM_EXEC);
	}
}

// Fix issue where bots would stand still while consuming pain pills/adrenaline
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorTakePills_OnStart(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction)
{
	SurvivorTakePills *pAction = META_IFACEPTR(SurvivorTakePills);

	pAction->Start(4.0f);

	RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Continue());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorDispatchEnemy_OnStart_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction)
{
	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	pSurvivorBot->EquipMeleeWeapon();

// Currently causes a server crash on windows
#if !defined _WIN32
	// Fix issue where bots would still move around while saving another survivor from Charger
	pSurvivorBot->GetIntentionInterface()->PushLegsStack(new SurvivorLegsWait());
#endif

	RETURN_META_VALUE(MRES_IGNORED, pAction->Continue());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorDislodgeVictim_OnStart(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction)
{
	SurvivorDislodgeVictim *pAction = META_IFACEPTR(SurvivorDislodgeVictim);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	// It's better to equip melee weapon when we have physical contact with Smoker instead because survivors
	// are held in front of them. Hunters and Jockeys are always on top
	if (pTarget && pTarget->GetTongueOwner())
	{
		RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
	}

	if (pSurvivorBot->EquipMeleeWeapon())
	{
		// Always kill when using melee weapon
		pAction->m_bBash = false;
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorLegsStayClose_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

	if (m_pDetour_SurvivorIntention_IsImmediatelyDangerousTo)
	{
		m_pDetour_SurvivorIntention_IsImmediatelyDangerousTo->EnableDetour();
	}

	ActionResult<SurvivorBot> stOrigActionResult = SH_MCALL(pAction, MHook_Action_Update)(pSurvivorBot, flInterval);

	if (m_pDetour_SurvivorIntention_IsImmediatelyDangerousTo)
	{
		m_pDetour_SurvivorIntention_IsImmediatelyDangerousTo->DisableDetour();
	}

	Action<SurvivorBot> *pChildAction = pAction->GetActiveChildAction();

	if (stOrigActionResult.IsRequestingChange())
	{
		bool bIsChildActionSurvivorLegsBattleStations = (pChildAction && pChildAction->IsNamed("SurvivorLegsBattleStations"));

		if (stOrigActionResult.m_action->IsNamed("SurvivorLegsCoverFriendInCombat"))
		{
			// Fix issue where bots would give up battle stations to cover another survivor in combat
			if (bIsChildActionSurvivorLegsBattleStations)
			{
				delete stOrigActionResult.m_action;

				RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Continue());
			}
		}

		if (stOrigActionResult.m_action->IsNamed("SurvivorLegsRegroup"))
		{
			// #define TEST_RETREATING_WHILE_REGROUPING
			#if defined TEST_RETREATING_WHILE_REGROUPING
			SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorLegsRegroup, Update, stOrigActionResult.m_action);
			#endif

			// Fix issue where bots would give up battle stations to regroup with their leader regardless of give up range
			if (bIsChildActionSurvivorLegsBattleStations)
			{
				SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();
				CTerrorPlayer *pHumanLeader = pTeamSituation->GetHumanLeader();

				if (pHumanLeader)
				{
					const Vector& vecAbsStart = pSurvivorBot->GetAbsOrigin();
					const Vector& vecAbsEnd = pHumanLeader->GetAbsOrigin();

					static ConVarRef r_sb_battlestation_give_up_range_from_human("sb_battlestation_give_up_range_from_human");

					if (CalcDistance(vecAbsStart, vecAbsEnd) < r_sb_battlestation_give_up_range_from_human.GetFloat())
					{
						delete stOrigActionResult.m_action;

						RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Continue());
					}
				}
			}
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorHealFriend_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorHealFriend *pAction = META_IFACEPTR(SurvivorHealFriend);

	if (m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy)
	{
		m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy->EnableDetour();
	}

	CTerrorPlayer *pPatient = pAction->GetPatient();

	if (pPatient)
	{
		SH_ADD_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pPatient, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);
	}

	ActionResult<SurvivorBot> stOrigActionResult = SH_MCALL(pAction, MHook_Action_Update)(pSurvivorBot, flInterval);

	if (pPatient)
	{
		SH_REMOVE_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pPatient, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);
	}

	if (m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy)
	{
		m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy->DisableDetour();
	}

	if (!stOrigActionResult.IsDone() && pPatient)
	{
		const Vector& vecAbsStart = pSurvivorBot->GetAbsOrigin();
		const Vector& vecAbsEnd = pPatient->GetAbsOrigin();

		// Fix issue where bots would chase after survivor with first aid kit despite initial range having been exceeded
		// Note that given range is hardcoded in SurvivorBot::UseHealingItems(Action<SurvivorBot> *) function
		constexpr float flRange = 1000.0f;

		if (CalcDistance(vecAbsStart, vecAbsEnd) >= flRange)
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Patient is too far away"));
		}

		// Fix issue where bots would continously side-step when their patient became unreachable
		if (!pSurvivorBot->IsReachable(pPatient))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Patient became unreachable"));
		}

		if (pPatient->IsOnThirdStrike())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		if (pSurvivorBot->IsAdrenalineActive())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		constexpr float flProgressBarPercent = 0.5f;

		if (pSurvivorBot->GetProgressBarPercent() >= flProgressBarPercent)
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		// Fix issue where bots would continue healing their patient despite recognizing imminent threat to them
		SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

		CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

		if (pRecognizedActor && pRecognizedActor->GetClass() != Zombie_Common && pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Spotted imminent threat to me"));
		}

		// Fix issue where bots would continue healing their patient despite another survivor being incapacitated or dominated by Special Infected
		SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

		if (pTeamSituation->HasAnyFriendInTrouble())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is in trouble - I'll try to help him"));
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorHealSelf_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	if (m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy)
	{
		m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy->EnableDetour();
	}

	SH_ADD_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pSurvivorBot, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);

	ActionResult<SurvivorBot> stOrigActionResult = SH_MCALL(pAction, MHook_Action_Update)(pSurvivorBot, flInterval);

	SH_REMOVE_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pSurvivorBot, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);

	if (m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy)
	{
		m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy->DisableDetour();
	}

	if (!stOrigActionResult.IsDone())
	{
		// Fix issue where bots would continue healing themselves despite recognizing imminent threat to them
		SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

		CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

		if (pRecognizedActor && pRecognizedActor->GetClass() != Zombie_Common && pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Spotted imminent threat to me"));
		}

		if (pSurvivorBot->IsOnThirdStrike())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		if (pSurvivorBot->IsAdrenalineActive())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		constexpr float flProgressBarPercent = 0.5f;

		if (pSurvivorBot->GetProgressBarPercent() >= flProgressBarPercent)
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
		}

		// Fix issue where bots would continue healing themselves despite another survivor being incapacitated or dominated by Special Infected
		SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

		if (pTeamSituation->HasAnyFriendInTrouble())
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is in trouble - I'll try to help him"));
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorTakePills_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorTakePills *pAction = META_IFACEPTR(SurvivorTakePills);

	// Fix issue where bots would continue consuming pain pills/adrenaline despite being healed by another player
	CTerrorPlayer *pUseActionOwner = pSurvivorBot->GetUseActionOwner();

	if (pUseActionOwner && pUseActionOwner->GetCurrentUseAction() == UseAction_Healing)
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is healing me"));
	}

	// Fix issue where bots would continue consuming pain pills/adrenaline despite recognizing imminent threat to them
	SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

	CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

	if (pRecognizedActor && pRecognizedActor->GetClass() != Zombie_Common && pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Spotted imminent threat to me"));
	}

	// Fix issue where bots would continue healing themselves despite another survivor being incapacitated or dominated by Special Infected
	SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

	if (pTeamSituation->HasAnyFriendInTrouble())
	{
		if (pSurvivorBot->IsOnThirdStrike())
		{
			RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
		}

		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is in trouble - I'll try to help him"));
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorGivePillsToFriend_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	ActionResult<SurvivorBot> stOrigActionResult = META_RESULT_ORIG_RET(ActionResult<SurvivorBot>);

	if (stOrigActionResult.IsRequestingChange())
	{
		RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
	}

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	if (pTarget)
	{
		const Vector& vecAbsStart = pSurvivorBot->GetAbsOrigin();
		const Vector& vecAbsEnd = pTarget->GetAbsOrigin();

		// Fix issue where bots would chase after survivor with pain pills/adrenaline despite initial range having been exceeded
		// Note that given range is hardcoded in SurvivorBot::UseHealingItems(Action<SurvivorBot> *) function
		constexpr float flRange = 1000.0f;

		if (CalcDistance(vecAbsStart, vecAbsEnd) >= flRange)
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Patient is too far away"));
		}

		// Fix issue where bots would continously side-step when their patient became unreachable
		if (!pSurvivorBot->IsReachable(pTarget))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Patient became unreachable"));
		}

		// Fix issue where bots would continue trying to give pain pills/adrenaline to another player despite recognizing imminent threat to them
		// Common Infected are considered a threat to them as well since they can't properly defend themselves while having pain pills/adrenaline equipped
		SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

		CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

		if (pRecognizedActor && pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Spotted imminent threat to me"));
		}

		// Fix issue where bots would continue trying to give pain pills/adrenaline despite another survivor being incapacitated or dominated by Special Infected
		SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

		if (pTeamSituation->HasAnyFriendInTrouble())
		{
			if (pTarget->IsOnThirdStrike())
			{
				RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
			}

			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is in trouble - I'll try to help him"));
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

// Fix issue where bots would continue this Action despite friend being no longer in trouble
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorLiberateBesiegedFriend_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorLiberateBesiegedFriend *pAction = META_IFACEPTR(SurvivorLiberateBesiegedFriend);

	CTerrorPlayer *pBesiegedFriend = pAction->GetBesiegedFriend();

	if (pBesiegedFriend && !pBesiegedFriend->IsIncapacitated() && !pBesiegedFriend->IsDominatedBySpecialInfected())
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Friend is no longer in trouble"));
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorDispatchEnemy_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorDispatchEnemy *pAction = META_IFACEPTR(SurvivorDispatchEnemy);

	// Fix issue where bots would tap-fire chargers instead of spraying them, when using assault rifles
	CBaseCombatWeapon *pWeapon = pSurvivorBot->GetActiveWeapon();

	if (pWeapon)
	{
		bool bUsingPistol = (FStrEq(pWeapon->GetClassname(), "weapon_pistol") || FStrEq(pWeapon->GetClassname(), "weapon_pistol_magnum"));

		if (!bUsingPistol)
		{
			pAction->m_bFiringWeapon = false;
		}
	}

	SH_ADD_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pSurvivorBot, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);

	ActionResult<SurvivorBot> stOrigActionResult = SH_MCALL(pAction, MHook_Action_Update)(pSurvivorBot, flInterval);

	SH_REMOVE_MANUALHOOK(MHook_CBaseCombatCharacter_IsIT, pSurvivorBot, SH_MEMBER(this, &CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT), false);

	RETURN_META_VALUE(MRES_SUPERCEDE, stOrigActionResult);
}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
// Since bots will wait until spit has disappeared from their path, allow them to move around after escaping it safely
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorEscapeSpit_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorEscapeSpit *pAction = META_IFACEPTR(SurvivorEscapeSpit);

	// Timer gets updated every time bot takes spit damage
	constexpr float flDuration = 1.0f;

	if (!pAction->m_waitTimer.IsLessThen(flDuration))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Escaped from spit"));
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}
#endif

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorEscapeFlames_Update(SurvivorBot *pSurvivorBot, float flInterval)
{
	SurvivorEscapeFlames *pAction = META_IFACEPTR(SurvivorEscapeFlames);

	// Timer gets updated every time bot fire damage
	constexpr float flDuration = 0.75f;

	if (!pAction->m_waitTimer.IsLessThen(flDuration))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, pAction->Done("[" SMEXT_CONF_LOGTAG "] Escaped from fire"));
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

// Allow bots to take cover when occupying battlestations, when desired
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorLegsBattleStations_Update_Post(SurvivorBot *pSurvivorBot, float flInterval)
{
	CNavArea *pLastKnownArea = pSurvivorBot->GetLastKnownArea();

	if (pLastKnownArea && pLastKnownArea->GetAttributes() & NAV_MESH_CROUCH)
	{
		NextBotPlayer_CTerrorPlayer *pNextBotTerrorPlayer = reinterpret_cast<NextBotPlayer_CTerrorPlayer *>(pSurvivorBot);

		pNextBotTerrorPlayer->PressCrouchButton();
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

// Allow bots to take cover when occupying battlestations, when desired
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_L4D1SurvivorLegsBattleStations_Update_Post(SurvivorBot *pSurvivorBot, float flInterval)
{
	CNavArea *pLastKnownArea = pSurvivorBot->GetLastKnownArea();

	if (pLastKnownArea && pLastKnownArea->GetAttributes() & NAV_MESH_CROUCH)
	{
		NextBotPlayer_CTerrorPlayer *pNextBotTerrorPlayer = reinterpret_cast<NextBotPlayer_CTerrorPlayer *>(pSurvivorBot);
		INextBot *pNextBot = pNextBotTerrorPlayer->MyNextBotPointer();

		// Prevent crouching when leaving battlestation
		if (pNextBot->GetLocomotionInterface()->IsAttemptingToMove())
		{
			pNextBotTerrorPlayer->ReleaseCrouchButton();

			RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
		}

		pNextBotTerrorPlayer->PressCrouchButton();
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

// Work-in-progress code
ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorLegsRegroup_Update_Post(SurvivorBot *pSurvivorBot, float flInterval)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	ActionResult<SurvivorBot> stOrigActionResult = SH_MCALL(pAction, MHook_Action_Update)(pSurvivorBot, flInterval);

	if (stOrigActionResult.m_type == DONE)
	{
		return stOrigActionResult;
	}

	SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

	CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

	if (pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
	{
		Action<SurvivorBot> *pActionBuriedUnderMe = pAction->GetActionBuriedUnderMe();

		EventDesiredResult<SurvivorBot> stCommandRetreatResult = pActionBuriedUnderMe->OnCommandRetreat(pSurvivorBot, pRecognizedActor, 100.0f);

		return pAction->SuspendFor(stCommandRetreatResult.m_action, "(TEST) Retreating from a dangerous threat");
	}

	return stOrigActionResult;
}

void CImprovedSurvivorBots::Hook_SurvivorLiberateBesiegedFriend_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	if (pNextAction->IsNamed("SurvivorReviveFriend"))
	{
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorUseObject, OnEnd, pNextAction);

		SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(SurvivorReviveFriend, ShouldGiveUp, pNextAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorReviveFriend, OnInjured, pNextAction);
	}
	else if (pNextAction->IsNamed("SurvivorDislodgeVictim"))
	{
		SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(SurvivorDislodgeVictim, ShouldGiveUp, pNextAction);
		SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(SurvivorDislodgeVictim, OnStartUse, pNextAction);
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorDislodgeVictim, OnEnd, pNextAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorDislodgeVictim, OnStart, pNextAction);
	}
	if (pNextAction->IsNamed("SurvivorDispatchEnemy"))
	{
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorDispatchEnemy, OnStart, pNextAction);
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorDispatchEnemy, OnEnd, pNextAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorDispatchEnemy, Update, pNextAction);
	}

	RETURN_META(MRES_IGNORED);
}

// Fix issue where bots would continue reviving someone despite Action having ended already
void CImprovedSurvivorBots::Hook_SurvivorUseObject_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	if (pSurvivorBot->GetReviveTarget())
	{
		pSurvivorBot->StopRevivingSomeone(true);
	}

	RETURN_META(MRES_IGNORED);
}

// Fix issue where bots would never switch to a weapon after this Action has ended
void CImprovedSurvivorBots::Hook_SurvivorHealSelf_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	pSurvivorBot->EquipWeapon();

	RETURN_META(MRES_IGNORED);
}

// Fix issue where bots would never switch to a weapon after this Action has ended
void CImprovedSurvivorBots::Hook_SurvivorTakePills_OnEnd(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	pSurvivorBot->EquipWeapon();

	// Original function tries to resume a previous Behavior (SurvivorLegsStayClose usually) after Action ended but since
	// OnStart method has been intercepted, which would start a new Behavior to tell bot to wait during consumption, it is no longer necessary
	RETURN_META(MRES_SUPERCEDE);
}

// Try to have bot switch to a primary weapon because it's much safer
void CImprovedSurvivorBots::Hook_SurvivorDispatchEnemy_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	if (pSurvivorBot->GetActiveWeapon() == pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_PISTOL))
	{
		pSurvivorBot->EquipWeapon();
	}

// Currently causes a server crash on windows
#if !defined _WIN32
	pSurvivorBot->GetIntentionInterface()->PopLegsStack();
#endif

	RETURN_META(MRES_IGNORED);
}

// Try to have bot switch to a primary weapon because it's much safer
void CImprovedSurvivorBots::Hook_SurvivorDislodgeVictim_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction)
{
	if (pSurvivorBot->GetActiveWeapon() == pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_PISTOL))
	{
		pSurvivorBot->EquipWeapon();
	}

	RETURN_META(MRES_IGNORED);
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorBehavior_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction)
{
	if (pInterruptingAction->IsNamed("SurvivorLiberateBesiegedFriend"))
	{
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorLiberateBesiegedFriend, OnEnd, pInterruptingAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorLiberateBesiegedFriend, Update, pInterruptingAction);
	}
	else if (pInterruptingAction->IsNamed("SurvivorHealFriend"))
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorHealFriend, Update, pInterruptingAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorHealFriend, OnInjured, pInterruptingAction);
	}
	else if (pInterruptingAction->IsNamed("SurvivorHealSelf"))
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorHealSelf, Update, pInterruptingAction);
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorHealSelf, OnEnd, pInterruptingAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorHealSelf, OnInjured, pInterruptingAction);
	}
	else if (pInterruptingAction->IsNamed("SurvivorTakePills"))
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorTakePills, OnStart, pInterruptingAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorTakePills, Update, pInterruptingAction);
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorTakePills, OnEnd, pInterruptingAction);
	}
	else if (pInterruptingAction->IsNamed("SurvivorGivePillsToFriend"))
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorGivePillsToFriend, Update, pInterruptingAction);
	}
	else if (pInterruptingAction->IsNamed("SurvivorCollectObject"))
	{
		SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(SurvivorCollectObject, ShouldGiveUp, pInterruptingAction);
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorAttack_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction)
{
	if (pInterruptingAction->IsNamed("SurvivorAmbushBoomer"))
	{
		SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(SurvivorAmbushBoomer, ShouldGiveUp, pInterruptingAction);
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

ActionResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorLegsMoveOn_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction)
{
	if (pInterruptingAction->IsNamed("SurvivorLegsBattleStations"))
	{
		SH_ACTION_VPHOOK_POST_WRAPPER(SurvivorLegsBattleStations, Update, pInterruptingAction);
	}

	RETURN_META_VALUE(MRES_IGNORED, ActionResult<SurvivorBot>());
}

// Solely for Archangel
Action< SurvivorBot > *CImprovedSurvivorBots::Hook_SurvivorDebugApproach_InitialContainedAction(SurvivorBot *pSurvivorBot)
{
	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	RETURN_META_VALUE(MRES_SUPERCEDE, pAction->GetActionBuriedUnderMe()->InitialContainedAction(pSurvivorBot));
}

EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorBehavior_OnIgnite_Post(SurvivorBot *pSurvivorBot)
{
	EventDesiredResult<SurvivorBot> stOrigEventResult = META_RESULT_ORIG_RET(EventDesiredResult<SurvivorBot>);

	if (stOrigEventResult.IsRequestingChange())
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorEscapeFlames, Update, stOrigEventResult.m_action);
	}

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}

// Fix issue where bots would give up too easy reviving another player when taking damage from Infected
EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorReviveFriend_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info)
{
	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	if (pTarget)
	{
		CBaseEntity *pAttacker = info.GetAttacker();

		if (pAttacker->GetTeamNumber() == TEAM_ZOMBIE)
		{
			bool bSpitDamage = (info.GetDamageType() & DMG_SPIT);

			if (!bSpitDamage)
			{
				constexpr float flTimeSinceAttackedByEnemy = 0.5f;

				if (pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
				{
					if (pSurvivorBot->GetFullHealth() >= SurvivorBotReviveIgnoreMultiDamageMinHealth.GetInt())
					{
						RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
					}

					RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking too much damage by multiple Infected"));
				}

				if (pSurvivorBot->GetFullHealth() >= SurvivorReviveIgnoreDamageMinHealth.GetInt())
				{
					RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
				}

				RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking damage by Infected"));
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}

// Fix issue where bots would give up too easy healing another player when taking damage from Infected
EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorHealFriend_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info)
{
	SurvivorHealFriend *pAction = META_IFACEPTR(SurvivorHealFriend);

	CTerrorPlayer *pPatient = pAction->GetPatient();

	if (pPatient)
	{
		CBaseEntity *pAttacker = info.GetAttacker();

		if (pAttacker->GetTeamNumber() == TEAM_ZOMBIE)
		{
			bool bSpitDamage = (info.GetDamageType() & DMG_SPIT);

			if (!bSpitDamage)
			{
				constexpr float flTimeSinceAttackedByEnemy = 0.5f;

				if (pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
				{
					if (pSurvivorBot->GetFullHealth() >= SurvivorBotHealIgnoreMultiDamageMinHealth.GetInt())
					{
						RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
					}

					RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking too much damage by multiple Infected"));
				}

				if (pSurvivorBot->GetFullHealth() >= SurvivorBotHealIgnoreDamageMinHealth.GetInt())
				{
					RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
				}

				RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking damage by Infected"));
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}

// Fix issue where bots would give up too easy healing themselves when taking damage from Infected
EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorHealSelf_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info)
{
	Action<SurvivorBot> *pAction = META_IFACEPTR(Action<SurvivorBot>);

	CBaseEntity *pAttacker = info.GetAttacker();

	if (pAttacker->GetTeamNumber() == TEAM_ZOMBIE)
	{
		bool bSpitDamage = (info.GetDamageType() & DMG_SPIT);

		if (!bSpitDamage)
		{
			constexpr float flTimeSinceAttackedByEnemy = 0.5f;

			if (pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
			{
				if (pSurvivorBot->GetFullHealth() >= SurvivorBotHealIgnoreMultiDamageMinHealth.GetInt())
				{
					RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
				}

				RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking too much damage by multiple Infected"));
			}

			if (pSurvivorBot->GetFullHealth() >= SurvivorBotHealIgnoreDamageMinHealth.GetInt())
			{
				RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryToSustain(RESULT_CRITICAL));
			}

			RETURN_META_VALUE(MRES_SUPERCEDE, pAction->TryDone(RESULT_IMPORTANT, "[" SMEXT_CONF_LOGTAG "] Taking damage by Infected"));
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}

#if SOURCE_ENGINE == SE_LEFT4DEAD2
EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorBehavior_OnEnteredSpit_Post(SurvivorBot *pSurvivorBot)
{
	EventDesiredResult<SurvivorBot> stOrigEventResult = META_RESULT_ORIG_RET(EventDesiredResult<SurvivorBot>);

	if (stOrigEventResult.IsRequestingChange())
	{
		SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorEscapeSpit, Update, stOrigEventResult.m_action);
	}

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}
#endif

// Solely for Archangel
EventDesiredResult< SurvivorBot > CImprovedSurvivorBots::Hook_SurvivorBehavior_OnCommandApproach_Post(SurvivorBot *pSurvivorBot, const Vector &vecPos, float flRange)
{
	EventDesiredResult<SurvivorBot> stOrigEventResult = META_RESULT_ORIG_RET(EventDesiredResult<SurvivorBot>);

	SH_ACTION_VPHOOK_PRE_WRAPPER(SurvivorDebugApproach, InitialContainedAction, stOrigEventResult.m_action);

	RETURN_META_VALUE(MRES_IGNORED, EventDesiredResult<SurvivorBot>());
}

void CImprovedSurvivorBots::Hook_SurvivorDislodgeVictim_OnStartUse(SurvivorBot *pSurvivorBot)
{
	SurvivorDislodgeVictim *pAction = META_IFACEPTR(SurvivorDislodgeVictim);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	// Fix issue #1 where bots would stand still for one second whenever they try to kill/shove jockey which is ineffective because jockeys usually keep moving with survivor
	// Fix issue #2 where bots have trouble freeing survivors that are hanging from tongue or are still being dragged
	bool bShouldKeepMoving = pTarget && (pTarget->GetJockeyAttacker() || !pTarget->ReachedTongueOwner());

	if (bShouldKeepMoving && m_pDetour_SurvivorUseObject_OnStartUse)
	{
		m_pDetour_SurvivorUseObject_OnStartUse->EnableDetour();
	}

	SH_MCALL(pAction, MHook_SurvivorUseObject_OnStartUse)(pSurvivorBot);

	if (m_pDetour_SurvivorUseObject_OnStartUse)
	{
		m_pDetour_SurvivorUseObject_OnStartUse->DisableDetour();
	}

	RETURN_META(MRES_SUPERCEDE);
}

bool CImprovedSurvivorBots::Hook_SurvivorReviveFriend_ShouldGiveUp(SurvivorBot *pSurvivorBot)
{
	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	// Fix issue where bots would continue this Action despite patient being no longer incapacitated
	if (!pTarget->IsIncapacitated())
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	// Fix issue where bots would continously side-step when their patient became unreachable
	if (!pSurvivorBot->IsReachable(pTarget))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}

bool CImprovedSurvivorBots::Hook_SurvivorDislodgeVictim_ShouldGiveUp(SurvivorBot *pSurvivorBot)
{
	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	if (!pTarget)
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	// Fix issue where bots would continously side-step when their target became unreachable
	if (!pSurvivorBot->IsReachable(pTarget))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	// Fix issue where bots would always give this Action up when their target is dominated by Special Infected
	if (!pTarget->GetTongueOwner() && !pTarget->GetPounceAttacker() && !pTarget->GetJockeyAttacker())
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, false);
}

// Fix issue where bots would only check ammo count of weapon that's currently being held, when trying to collect ammo
bool CImprovedSurvivorBots::Hook_SurvivorCollectObject_ShouldGiveUp(SurvivorBot *pSurvivorBot)
{
	SurvivorTeamSituation *pTeamSituation = pSurvivorBot->GetTeamSituation();

	if (pTeamSituation->GetTonguedFriend())
	{
		return true;
	}

	if (pTeamSituation->GetPouncedFriend())
	{
		return true;
	}

	if (pTeamSituation->GetPummeledFriend())
	{
		return true;
	}

	if (pTeamSituation->GetFriendInTrouble())
	{
		return true;
	}

	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	constexpr float flTimeSinceAttackedByEnemy = 2.0f;

	if (reinterpret_cast<CDirector *>(m_pObj_TheDirector)->GetTankCount() > 0 || pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
	{
		CBaseEntity *pEntity = pAction->GetTarget();

		if (pEntity)
		{
			if (FStrEq(pEntity->GetClassname(), "weapon_ammo_spawn"))
			{
				CBaseCombatWeapon *pWeapon = pSurvivorBot->Weapon_GetSlot(WEAPON_SLOT_RIFLE);

				if (pWeapon)
				{
					const char *pszWeaponName = pWeapon->GetClassname();

					constexpr float flPercent = 0.4f;

					if (pSurvivorBot->GetAmmoCount(pWeapon->GetPrimaryAmmoType()) >= (GetMaxCarryOfWeapon(pszWeaponName) * flPercent))
					{
						return true;
					}

					return false;
				}
			}
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, pAction->ShouldGiveUp(pSurvivorBot));
}

// Fix issue where bots would try to ambush dead boomers
bool CImprovedSurvivorBots::Hook_SurvivorAmbushBoomer_ShouldGiveUp(SurvivorBot *pSurvivorBot)
{
	SurvivorUseObject *pAction = META_IFACEPTR(SurvivorUseObject);

	CTerrorPlayer *pTarget = reinterpret_cast<CTerrorPlayer *>(pAction->GetTarget());

	if (pTarget && !pTarget->IsAlive())
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, true);
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, false);
}

// Fix issue where bots would stop specific Actions when they or target, they're interacting with, got vomited upon by a Boomer
bool CImprovedSurvivorBots::Hook_CBaseCombatCharacter_IsIT()
{
	RETURN_META_VALUE(MRES_SUPERCEDE, false);
}

// Fix issue where bots would never properly defend themselves during this Action, when being attacked by Infected
void CImprovedSurvivorBots::Hook_SurvivorIntention_OnInjured(const CTakeDamageInfo &info)
{
	SET_META_RESULT(MRES_IGNORED);

	SurvivorIntention *pSurvivorIntention = META_IFACEPTR(SurvivorIntention);
	SurvivorBot *pSurvivorBot = pSurvivorIntention->GetSurvivorBot();

	CBaseEntity *pAttacker = info.GetAttacker();

	bool bSpitDamage = (info.GetDamageType() & DMG_SPIT);

	if (pAttacker->GetTeamNumber() == TEAM_ZOMBIE && !bSpitDamage)
	{
		CBaseCombatCharacter *pBCC = dynamic_cast<CBaseCombatCharacter *>(pAttacker);

		if (pBCC)
		{
			ZombieClassType nZombieClass = pBCC->GetClass();

			if (nZombieClass > Zombie_Charger)
			{
				return;
			}

			// Make an exception when saving another player from a Charger
			SurvivorIntention *pSurvivorIntention = pSurvivorBot->GetIntentionInterface();

			Behavior<SurvivorBot> *pBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pSurvivorIntention->FirstContainedResponder());

			if (pBehavior)
			{
				Action<SurvivorBot> *pPrimaryAction = dynamic_cast<Action< SurvivorBot > *>(pBehavior->FirstContainedResponder());

				if (pPrimaryAction && pPrimaryAction->IsNamed("SurvivorDispatchEnemy"))
				{
					return;
				}
			}

			if (nZombieClass == Zombie_Charger)
			{
				static ConVarRef r_z_charger_allow_shove("z_charger_allow_shove");

				if (r_z_charger_allow_shove.GetBool())
				{
					pSurvivorBot->GetBodyInterface()->AimHeadTowards(pAttacker, IBody::MANDATORY, 1.0f, &g_SurvivorBotMeleeOnReply,
						"[" SMEXT_CONF_LOGTAG "] Trying to defend myself against Charger by shoving", false);
				}
				else
				{
					pSurvivorBot->GetBodyInterface()->AimHeadTowards(pAttacker, IBody::MANDATORY, 1.0f, &g_SurvivorBotAttackOnReply,
						"[" SMEXT_CONF_LOGTAG "] Trying to defend myself against Charger by attacking", false);
				}
			}
			else
			{
				constexpr float flTimeSinceAttackedByEnemy = 0.5f;

				if (pSurvivorBot->GetTimeSinceAttackedByEnemy() < flTimeSinceAttackedByEnemy)
				{
					pSurvivorBot->GetBodyInterface()->AimHeadTowards(pAttacker, IBody::MANDATORY, 1.0f, &g_SurvivorBotAttackOnReply,
						"[" SMEXT_CONF_LOGTAG "] Trying to defend myself against Infected by attacking", false);
				}
				else
				{
					CBaseCombatCharacter *pRecognizedActor = pSurvivorIntention->GetRecognizedActor();

					if (pRecognizedActor && pRecognizedActor != pAttacker && pSurvivorIntention->IsImmediatelyDangerousTo(pSurvivorBot, pRecognizedActor))
					{
						ZombieClassType nZombieClass = pRecognizedActor->GetClass();

						// If we have threats that are more dangerous, prioritize them instead as opposed to Common Infected since they do more damage
						// Spitter isn't as dangerous to us as the other Special Infected, that's why we're ignoring her
						if (nZombieClass >= Zombie_Smoker && nZombieClass <= Zombie_Tank && nZombieClass != Zombie_Spitter)
						{
							return;
						}
					}

					pSurvivorBot->GetBodyInterface()->AimHeadTowards(pAttacker, IBody::MANDATORY, 1.0f, &g_SurvivorBotMeleeOnReply,
						"[" SMEXT_CONF_LOGTAG "] Trying to defend myself against Infected by shoving", false);
				}
			}
		}
	}
}

void CImprovedSurvivorBots::Hook_SurvivorIntention_Reset_Post()
{
	SurvivorIntention *pSurvivorIntention = META_IFACEPTR(SurvivorIntention);

	INextBotEventResponder *pNextContainedResponder = pSurvivorIntention->m_behaviorLegsStack[0][0];

	Behavior<SurvivorBot> *pNextBehavior = dynamic_cast<Behavior< SurvivorBot > *>(pNextContainedResponder);

	if (pNextBehavior)
	{
		Action<SurvivorBot> *pAction = dynamic_cast<Action< SurvivorBot > *>(pNextBehavior->FirstContainedResponder());

		if (pAction && pAction->IsNamed("L4D1SurvivorLegsBattleStations"))
		{
			SH_ACTION_VPHOOK_POST_WRAPPER(L4D1SurvivorLegsBattleStations, Update, pAction);
		}
	}

	RETURN_META(MRES_IGNORED);
}

unsigned int CImprovedSurvivorBots::Hook_PlayerBody_GetSolidMask_Post()
{
	RETURN_META_VALUE(MRES_OVERRIDE, META_RESULT_ORIG_RET(unsigned int) | CONTENTS_TEAM1);
}

bool CImprovedSurvivorBots::Hook_PlayerLocomotion_ClimbUpToLedge(const Vector &vecLandingGoal, const Vector &vecLandingForward, const CBaseEntity *pObstacle)
{
	// We're fixing this issue by rewriting this function body without calling IsClimbPossible.
	// The game does a ray cast to check for climbable ledges, so it's unnecessary to check for a CLIMB_UP discontinuity
	// on the path segment (which IsClimbPossible does) when raycast already determines that.
	// if ( !IsClimbPossible( GetBot(), obstacle ) )
	// {
	// 	return false;
	// }

	PlayerLocomotion *pPlayerLocomotion = META_IFACEPTR(PlayerLocomotion);

	pPlayerLocomotion->Jump();
	pPlayerLocomotion->m_isClimbingUpToLedge = true;
	pPlayerLocomotion->m_landingGoal = vecLandingGoal;
	pPlayerLocomotion->m_hasLeftTheGround = false;

	RETURN_META_VALUE(MRES_SUPERCEDE, true);
}

// Fix issue where bots wouldn't walk when nav area is marked with WALK attribute
bool CImprovedSurvivorBots::Hook_PlayerLocomotion_IsRunning()
{
	PlayerLocomotion *pPlayerLocomotion = META_IFACEPTR(PlayerLocomotion);

	CTerrorPlayer *pPlayer = reinterpret_cast<CTerrorPlayer *>(pPlayerLocomotion->m_player);

	CNavArea *pNavArea = pPlayer->GetLastKnownArea();

	if (pNavArea && pNavArea->GetAttributes() & NAV_MESH_WALK)
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, false);
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}

void CImprovedSurvivorBots::Hook_SurvivorBot_OnNavAreaChanged_Post(CNavArea *pEnteredArea, CNavArea *pLeftArea)
{
	SET_META_RESULT(MRES_IGNORED);

	if (!pEnteredArea)
	{
		return;
	}

	SurvivorBot *pSurvivorBot = META_IFACEPTR(SurvivorBot);

	// if we just entered a 'stop' area, set the flag
	if (pEnteredArea->GetAttributes() & NAV_MESH_STOP)
	{
		SurvivorBot::m_bStopping[pSurvivorBot->entindex()] = true;
	}
}

float CImprovedSurvivorBots::NavAreaTravelDistance_ShortestPathCost(CNavArea *startArea, CNavArea *endArea, float maxPathLength)
{
	DummyPathCost costFunc;		// cost functor is inlined, that's why we're using a dummy

	struct
	{
		CNavArea *startArea;
		CNavArea *endArea;
		DummyPathCost *costFunc;
		float maxPathLength;
	}
	stStack =
	{
		startArea,
		endArea,
		&costFunc,
		maxPathLength
	};

	float flTravelDistance;
	m_pCallWrap_NavAreaTravelDistance_ShortestPathCost->Execute(&stStack, &flTravelDistance);

	return flTravelDistance;
}

CTerrorPlayer *CImprovedSurvivorBots::GetClosestDominatedFriend(SurvivorTeamSituation *pTeamSituation)
{
	CTerrorPlayer *pDominatedSurvivor[] =
	{
		pTeamSituation->GetJockeyedFriend(),
		pTeamSituation->GetPummeledFriend(),
		pTeamSituation->GetTonguedFriend(),
		pTeamSituation->GetPouncedFriend()
	};

	CTerrorPlayer *pCloseFriend = nullptr;
	float flCloseTravelDistance = FLT_MAX;

	for (const auto pFriend : pDominatedSurvivor)
	{
		if (!pFriend)
		{
			continue;
		}

		float flTravelDistance = NavAreaTravelDistance_ShortestPathCost(pTeamSituation->GetBot()->GetLastKnownArea(), pFriend->GetLastKnownArea());

		if (flTravelDistance < 0.0f)
		{
			continue;
		}

		if (flTravelDistance < flCloseTravelDistance)
		{
			pCloseFriend = pFriend;
			flCloseTravelDistance = flTravelDistance;
		}
	}

	return pCloseFriend;
}

CTerrorPlayer *CImprovedSurvivorBots::GetClosestFriendInTrouble(SurvivorTeamSituation *pTeamSituation)
{
	CTerrorPlayer *pSurvivorInTrouble[] =
	{
		pTeamSituation->GetFriendInTrouble(),
		pTeamSituation->GetHumanFriendInTrouble()
	};

	CTerrorPlayer *pCloseFriend = nullptr;
	float flCloseTravelDistance = FLT_MAX;

	for (const auto pFriend : pSurvivorInTrouble)
	{
		if (!pFriend)
		{
			continue;
		}

		float flTravelDistance = NavAreaTravelDistance_ShortestPathCost(pTeamSituation->GetBot()->GetLastKnownArea(), pFriend->GetLastKnownArea());

		if (flTravelDistance < 0.0f)
		{
			continue;
		}

		if (flTravelDistance < flCloseTravelDistance)
		{
			pCloseFriend = pFriend;
			flCloseTravelDistance = flTravelDistance;
		}
	}

	return pCloseFriend;
}