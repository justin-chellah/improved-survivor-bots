#pragma once

#include "smsdk_ext.h"
#include <IBinTools.h>
#include <iplayerinfo.h>
#include "NextBot/NextBotBehavior.h"
#include <sh_memory.h>

#define SH_ACTION_VPHOOK_PRE_WRAPPER(ClassName, Name, Action) \
	if (m_nSHookID_Action[ActionHook_##ClassName##_##Name] == 0) \
	{ \
		m_nSHookID_Action[ActionHook_##ClassName##_##Name] = SH_ADD_MANUALVPHOOK(MHook_Action_##Name, Action, \
			SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##ClassName##_##Name), false); \
	} \

#define SH_ACTION_VPHOOK_POST_WRAPPER(ClassName, Name, Action) \
	if (m_nSHookID_Action[ActionHook_##ClassName##_##Name] == 0) \
	{ \
		m_nSHookID_Action[ActionHook_##ClassName##_##Name] = SH_ADD_MANUALVPHOOK(MHook_Action_##Name, Action, \
			SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##ClassName##_##Name##_Post), true); \
	} \

// For Actions derived from SurvivorUseObject
#define SH_SURVIVOR_USE_OBJECT_VPHOOK_PRE_WRAPPER(ClassName, Name, Action) \
	if (m_nSHookID_Action[ActionHook_##ClassName##_##Name] == 0) \
	{ \
		m_nSHookID_Action[ActionHook_##ClassName##_##Name] = SH_ADD_MANUALVPHOOK(MHook_SurvivorUseObject_##Name, Action, \
			SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##ClassName##_##Name), false); \
	} \

#define SH_SURVIVOR_USE_OBJECT_VPHOOK_POST_WRAPPER(ClassName, Name, Action) \
	if (m_nSHookID_Action[ActionHook_##ClassName##_##Name] == 0) \
	{ \
		m_nSHookID_Action[ActionHook_##ClassName##_##Name] = SH_ADD_MANUALVPHOOK(MHook_SurvivorUseObject_##Name, Action, \
			SH_MEMBER(this, &CImprovedSurvivorBots::Hook_##ClassName##_##Name##_Post), true); \
	} \

class CDetour;
class SurvivorBot;
class SurvivorTeamSituation;
class CNavArea;
class CTerrorPlayer;

class CImprovedSurvivorBots : public SDKExtension, public IClientListener, public IConCommandBaseAccessor
{
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late) override;

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
    virtual void SDK_OnAllLoaded() override;

	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload() override;
#ifdef SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late) override;
#endif
	/**
	 * @brief Return false to tell Core that your extension should be considered unusable.
	 *
	 * @param error				Error buffer.
	 * @param maxlength			Size of error buffer.
	 * @return					True on success, false otherwise.
	 */
    	virtual bool QueryRunning(char *error, size_t maxlen) override;

    	/**
	 * @brief Asks the extension whether it's safe to remove an external
	 * interface it's using.  If it's not safe, return false, and the
	 * extension will be unloaded afterwards.
	 *
	 * NOTE: It is important to also hook NotifyInterfaceDrop() in order to clean
	 * up resources.
	 *
	 * @param pInterface		Pointer to interface being dropped.  This
	 * 							pointer may be opaque, and it should not
	 *							be queried using SMInterface functions unless
	 *							it can be verified to match an existing
	 *							pointer of known type.
	 * @return					True to continue, false to unload this
	 * 							extension afterwards.
	 */
	virtual bool QueryInterfaceDrop(SMInterface *pInterface) override;

    	/**
	 * @brief Notifies the extension that an external interface it uses is being removed.
	 *
	 * @param pInterface		Pointer to interface being dropped.  This
	 * 							pointer may be opaque, and it should not
	 *							be queried using SMInterface functions unless
	 *							it can be verified to match an existing
	 */
	virtual void NotifyInterfaceDrop(SMInterface *pInterface) override;

	/**
	 * @brief Called when a client is put in server.
	 *
	 * @param iClient		Index of the client.
	 */
	virtual void OnClientPutInServer(int iClient) override;

	/**
	 * @brief Called when the server is activated.
	 */
	virtual void OnServerActivated(int nMaxClients) override;

	// Flags is a combination of FCVAR flags in cvar.h.
	// hOut is filled in with a handle to the variable.
	virtual bool RegisterConCommandBase(ConCommandBase *pVar) override;

	void Patch(bool bPatch);

	/**
	 * Try to start the Action. Result is immediately processed,
	 * which can cause an immediate transition, another OnStart(), etc.
	 * An Action can count on each OnStart() being followed (eventually) with an OnEnd().
	 */
	ActionResult< SurvivorBot > Hook_SurvivorTakePills_OnStart(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction);
	ActionResult< SurvivorBot > Hook_SurvivorDispatchEnemy_OnStart_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction);
	ActionResult< SurvivorBot > Hook_SurvivorDislodgeVictim_OnStart(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pPriorAction);

	/**
	 * Do the work of the Action. It is possible for Update to not be
	 * called between a given OnStart/OnEnd pair due to immediate transitions.
	 */
	ActionResult< SurvivorBot > Hook_SurvivorLegsStayClose_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorHealFriend_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorHealSelf_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorTakePills_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorGivePillsToFriend_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorLiberateBesiegedFriend_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorDispatchEnemy_Update(SurvivorBot *pSurvivorBot, float flInterval);
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	ActionResult< SurvivorBot > Hook_SurvivorEscapeSpit_Update(SurvivorBot *pSurvivorBot, float flInterval);
#endif
	ActionResult< SurvivorBot > Hook_SurvivorEscapeFlames_Update(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorLegsBattleStations_Update_Post(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_L4D1SurvivorLegsBattleStations_Update_Post(SurvivorBot *pSurvivorBot, float flInterval);
	ActionResult< SurvivorBot > Hook_SurvivorLegsRegroup_Update_Post(SurvivorBot *pSurvivorBot, float flInterval);

	// Invoked when an Action is ended for any reason
	void Hook_SurvivorLiberateBesiegedFriend_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);
	void Hook_SurvivorUseObject_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);
	void Hook_SurvivorHealSelf_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);
	void Hook_SurvivorTakePills_OnEnd(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);
	void Hook_SurvivorDispatchEnemy_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);
	void Hook_SurvivorDislodgeVictim_OnEnd_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pNextAction);

	/*
	 * When an Action is suspended by a new action.
	 * Note that only CONTINUE and DONE are valid results.  All other results will
	 * be considered as a CONTINUE.
	 */
	ActionResult< SurvivorBot >	Hook_SurvivorBehavior_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction);
	ActionResult< SurvivorBot > Hook_SurvivorAttack_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction);
	ActionResult< SurvivorBot > Hook_SurvivorLegsMoveOn_OnSuspend_Post(SurvivorBot *pSurvivorBot, Action< SurvivorBot > *pInterruptingAction);

	// create and return an Action to start as sub-action within this Action when it starts
	Action< SurvivorBot > *Hook_SurvivorDebugApproach_InitialContainedAction(SurvivorBot *pSurvivorBot);

	/**
	 * Override the event handler methods below to respond to events that occur during this Action
	 * NOTE: These are identical to the events in INextBotEventResponder with the addition
	 * of an actor argument and a return result. Their translators are located in the private area
	 * below.
	 */
	EventDesiredResult< SurvivorBot > Hook_SurvivorBehavior_OnIgnite_Post(SurvivorBot *pSurvivorBot);

	EventDesiredResult< SurvivorBot > Hook_SurvivorReviveFriend_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info);
	EventDesiredResult< SurvivorBot > Hook_SurvivorHealFriend_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info);
	EventDesiredResult< SurvivorBot > Hook_SurvivorHealSelf_OnInjured(SurvivorBot *pSurvivorBot, const CTakeDamageInfo &info);
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	EventDesiredResult< SurvivorBot > Hook_SurvivorBehavior_OnEnteredSpit_Post(SurvivorBot *pSurvivorBot);
#endif
	EventDesiredResult< SurvivorBot > Hook_SurvivorBehavior_OnCommandApproach_Post(SurvivorBot *pSurvivorBot, const Vector &vecPos, float flRange);

	void Hook_SurvivorDislodgeVictim_OnStartUse(SurvivorBot *pSurvivorBot);

	bool Hook_SurvivorReviveFriend_ShouldGiveUp(SurvivorBot *pSurvivorBot);
	bool Hook_SurvivorDislodgeVictim_ShouldGiveUp(SurvivorBot *pSurvivorBot);
	bool Hook_SurvivorCollectObject_ShouldGiveUp(SurvivorBot *pSurvivorBot);
	bool Hook_SurvivorAmbushBoomer_ShouldGiveUp(SurvivorBot *pSurvivorBot);

	bool Hook_CBaseCombatCharacter_IsIT(void);

	void Hook_SurvivorIntention_OnInjured(const CTakeDamageInfo &info);
	void Hook_SurvivorIntention_Reset_Post(void);

	unsigned int Hook_PlayerBody_GetSolidMask_Post(void);
	bool Hook_PlayerLocomotion_ClimbUpToLedge(const Vector &vecLandingGoal, const Vector &vecLandingForward, const CBaseEntity *pObstacle);
	bool Hook_PlayerLocomotion_IsRunning(void);
	void Hook_SurvivorBot_OnNavAreaChanged_Post(CNavArea *pEnteredArea, CNavArea *pLeftArea);

	float NavAreaTravelDistance_ShortestPathCost(CNavArea *startArea, CNavArea *endArea, float maxPathLength = 0.0f);

	CTerrorPlayer *GetClosestDominatedFriend(SurvivorTeamSituation *pTeamSituation);
	CTerrorPlayer *GetClosestFriendInTrouble(SurvivorTeamSituation *pTeamSituation);

private:
	CDetour *m_pDetour_SurvivorBot_UpdateTeamSituation = nullptr;
	CDetour *m_pDetour_SurvivorBot_SaveFriendsInImmediateTrouble = nullptr;
	CDetour *m_pDetour_SurvivorBot_GetEscortRange = nullptr;
	CDetour *m_pDetour_SurvivorBot_ScavengeNearbyItems = nullptr;

	CDetour *m_pDetour_SurvivorBotPathCost_funcCallOp = nullptr;

	CDetour *m_pDetour_CTerrorPlayer_GetTimeSinceAttackedByEnemy = nullptr;
	CDetour *m_pDetour_SurvivorIntention_IsImmediatelyDangerousTo = nullptr;
	CDetour *m_pDetour_SurvivorUseObject_OnStartUse = nullptr;
	CDetour *m_pDetour_SurvivorAttack_EquipBestWeapon = nullptr;

	CDetour *m_pDetour_PathFollower_Update = nullptr;
	CDetour *m_pDetour_PathFollower_Climbing = nullptr;

	CDetour *m_pDetour_NextBotTraversableTraceFilter_ShouldHitEntity = nullptr;

	CUtlVector<CDetour *> m_vecDetours;
	CUtlVector<ICallWrapper *> m_vecCallWrappers;

	int m_iVtbl_Action_OnStart = -1;
	int m_iVtbl_Action_Update = -1;
	int m_iVtbl_Action_OnEnd = -1;
	int m_iVtbl_Action_OnSuspend = -1;
	int m_iVtbl_Action_InitialContainedAction = -1;

	int m_iVtbl_SurvivorUseObject_OnStartUse = -1;
	int m_iVtbl_SurvivorUseObject_ShouldGiveUp = -1;

	int m_iVtbl_Action_OnIgnite = -1;
	int m_iVtbl_Action_OnInjured = -1;
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	int m_iVtbl_Action_OnEnteredSpit = -1;
#endif
	int m_iVtbl_Action_OnCommandApproach = -1;

	int m_iVtbl_IIntention_OnInjured = -1;
	int m_iVtbl_IIntention_Reset = -1;

	int m_iVtbl_IBody_GetSolidMask = -1;
	int m_iVtbl_ILocomotion_ClimbUpToLedge = -1;
	int m_iVtbl_ILocomotion_IsRunning = -1;

	int m_iVtbl_CBasePlayer_OnNavAreaChanged = -1;

	enum ActionHookType
	{
		ActionHook_SurvivorTakePills_OnStart = 0,
		ActionHook_SurvivorDispatchEnemy_OnStart,
		ActionHook_SurvivorDislodgeVictim_OnStart,

		ActionHook_SurvivorLegsStayClose_Update,
		ActionHook_SurvivorHealFriend_Update,
		ActionHook_SurvivorHealSelf_Update,
		ActionHook_SurvivorTakePills_Update,
		ActionHook_SurvivorGivePillsToFriend_Update,
		ActionHook_SurvivorLiberateBesiegedFriend_Update,
		ActionHook_SurvivorDispatchEnemy_Update,
		#if SOURCE_ENGINE == SE_LEFT4DEAD2
		ActionHook_SurvivorEscapeSpit_Update,
		#endif
		ActionHook_SurvivorEscapeFlames_Update,
		ActionHook_SurvivorLegsBattleStations_Update,
		ActionHook_L4D1SurvivorLegsBattleStations_Update,
		ActionHook_SurvivorLegsRegroup_Update,

		ActionHook_SurvivorLiberateBesiegedFriend_OnEnd,
		ActionHook_SurvivorUseObject_OnEnd,
		ActionHook_SurvivorHealSelf_OnEnd,
		ActionHook_SurvivorTakePills_OnEnd,
		ActionHook_SurvivorDispatchEnemy_OnEnd,
		ActionHook_SurvivorDislodgeVictim_OnEnd,

		ActionHook_SurvivorBehavior_OnSuspend,
		ActionHook_SurvivorAttack_OnSuspend,
		ActionHook_SurvivorLegsMoveOn_OnSuspend,

		ActionHook_SurvivorDebugApproach_InitialContainedAction,

		ActionHook_SurvivorDislodgeVictim_OnStartUse,

		ActionHook_SurvivorReviveFriend_ShouldGiveUp,
		ActionHook_SurvivorDislodgeVictim_ShouldGiveUp,
		ActionHook_SurvivorCollectObject_ShouldGiveUp,
		ActionHook_SurvivorAmbushBoomer_ShouldGiveUp,

		ActionHook_SurvivorBehavior_OnIgnite,

		ActionHook_SurvivorReviveFriend_OnInjured,
		ActionHook_SurvivorHealFriend_OnInjured,
		ActionHook_SurvivorHealSelf_OnInjured,
		#if SOURCE_ENGINE == SE_LEFT4DEAD2
		ActionHook_SurvivorBehavior_OnEnteredSpit,
		#endif
		ActionHook_SurvivorBehavior_OnCommandApproach,

		ActionHook_MAXHOOKS,
	};

	enum PatchFnType
	{
		PatchFn_SurvivorAttack_FireWeapon = 0,
		PatchFn_SurvivorIntention_OnSound,
		PatchFn_MAX,
	};

	int m_nSHookID_Action[ActionHook_MAXHOOKS] = {0};

	int m_nSHookID_SurvivorIntention_OnInjured = 0;
	int m_nSHookID_SurvivorIntention_Reset = 0;

	int m_nSHookID_PlayerBody_GetSolidMask = 0;
	int m_nSHookID_PlayerLocomotion_ClimbUpToLedge = 0;
	int m_nSHookID_PlayerLocomotion_IsRunning = 0;
	int m_nSHookID_SurvivorBot_OnNavAreaChanged = 0;

	void *m_pSurvivorBot_IsBeingHeroic_Condition = nullptr;
	void *m_pCTerrorGameRules_HasPlayerControlledZombies_Condition[PatchFn_MAX] = {nullptr};

	void *m_pFn_NavAreaTravelDistance_ShortestPathCost = nullptr;

	void *m_pObj_TheDirector = nullptr;

	ICallWrapper *m_pCallWrap_NavAreaTravelDistance_ShortestPathCost = nullptr;
};