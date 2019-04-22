/* ======== SimpleStats ========
* Copyright (C) 2004-2007 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */
#include <oslink.h>
#include "HLStatsX.h"
#include "Stats.h"
#include "meta_hooks.h"
#include "cvars.h"
#include "const.h"
#include <time.h>
#include "SteamIDList.h"

#include "hl2sdk/recipientfilters.h"
#include <bitbuf.h>

//This has all of the necessary hook declarations.  Read it!
#include "meta_hooks.h"

int g_IndexOfLastUserCmd;
int g_MaxClients;
int g_ModIndex;

int g_AlivePlayers;
int g_AliveBots;
int g_BotCount;
int g_PlayerCount;

int g_LastRoundShow=0;
int g_InformAboutMoreStatsCount=0;

float g_FlTime;
bool g_IsConnected[MAXPLAYERS+1];
bool g_HasMenuOpen[MAXPLAYERS+1];
bool g_FirstLoad;
bool g_FoundInterface=false;
bool g_BlockSayChat;

int g_ShowMoreStats = true;

HLStatsIngame g_MSCore;
ConstPlayerInfo UserInfo[MAXPLAYERS+1];
ConstPlayerStats UserStats[MAXPLAYERS+1];
ConstMorePlayerStats g_MoreUserStats[MAXPLAYERS+1];

EventListenPlayerWeaponFire *g_pEventPlayerFire;
EventListenPlayerHurt *g_pEventPlayerHurt;
EventListenPlayerDeath *g_pEventPlayerDeath;
ModSettingsStruct g_ModSettings;
Stats *m_Stats;
SteamIDList *m_SteamIDList;

PLUGIN_EXPOSE(HLStatsIngame, g_MSCore);

void HLStatsIngame::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	LoadPluginSettings(clientMax);
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::ClientDisconnect(edict_t *pEntity)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	
	if(g_MoreUserStats[id].ShowStats && g_MoreUserStats[id].PlayerWasInList == false)
	{
		m_SteamIDList->AddSteamID(UserInfo[id].Steamid);
	}
	else if(g_MoreUserStats[id].ShowStats == false && g_MoreUserStats[id].PlayerWasInList == true)
	{
		m_SteamIDList->RemoveSteamID(UserInfo[id].Steamid);
	}

	m_Stats->LogPlayerStats(id);
	g_IsConnected[id] = false;
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::ClientPutInServer(edict_t *pEntity, char const *playername)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	g_IsConnected[id] = true;
	UserInfo[id].PlayerEdict = pEntity;
	UserInfo[id].Userid = m_Engine->GetPlayerUserId(pEntity);

	sprintf(UserInfo[id].Steamid,m_Engine->GetPlayerNetworkIDString(pEntity));

	if(strcmp(UserInfo[id].Steamid,"BOT") == 0 || strcmp(UserInfo[id].Steamid,"HLTV") == 0 ) // || strcmp(UserInfo[id].steamid,"STEAM_ID_LAN") == 0
	{
		UserInfo[id].ShowStats = false;
		UserInfo[id].IsBot = true;
	}
	else
	{
		UserInfo[id].IsBot = false;
		UserInfo[id].ShowStats = true;

		if(m_SteamIDList->IsSteamIDInList(UserInfo[id].Steamid))
		{
			g_MoreUserStats[id].PlayerWasInList = true;
			g_MoreUserStats[id].ShowStats = true;
		} else {
			g_MoreUserStats[id].PlayerWasInList = false;
			g_MoreUserStats[id].ShowStats = false;
		}
	}		

	m_Stats->ResetStatsArray(id);
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::SetCommandClient(int index)
{
	//META_LOG(g_PLAPI, "SetCommandClient() called: index=%d", index);
	g_IndexOfLastUserCmd = index +1;
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::GameFrame(bool simulating) // We dont hook GameFrame, we leave it in here incase we ever need some timer system 
{
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::ClientCommand(edict_t *pEntity)
{
	int id = m_Engine->IndexOfEdict(pEntity);
//	const char *command = m_Engine->Cmd_Argv(0);
	RETURN_META(MRES_IGNORED);
}
void HLStatsIngame::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName())
		return;

	const char *EventName = event->GetName();
	
	if(g_ModIndex == MOD_CSTRIKE && strcmp(EventName,"round_end") == 0 || g_ModIndex == MOD_DOD && strcmp(EventName,"dod_round_start") == 0)
	{
		for(int i=0;i<=g_MaxClients;i++) if(g_IsConnected[i])
		{
			m_Stats->LogPlayerStats(i);
		}

#if UseMoreStats == 1
		if(g_ShowMoreStats)
		{
			if(g_LastRoundShow >= g_InformAboutMoreStatsCount && g_InformAboutMoreStatsCount != 0)
			{
				for (int i=1;i<=g_MaxClients;i++) if(g_IsConnected[i])
				{
					if(!g_MoreUserStats[i].ShowStats)
						MessagePlayer(i,"%s If you want to see more stats in game when you die or at round end: 'say /morestats'",StartStringTag);
				}			
				g_LastRoundShow = 0;
			}
			else
				g_LastRoundShow++;
		}
#endif
	}
	else if(strcmp(EventName,"round_freeze_end") == 0)
	{
#if UseMoreStats == 1
		if(g_ShowMoreStats == 1)
		{
			m_Stats->ShowMoreStats();
			m_Stats->ClearMoreStats();
		}
		else if(g_ShowMoreStats == 2)
			m_Stats->ClearMoreStats();
#endif
	}
	else if(strcmp(EventName,"team_info") == 0)
	{
		int TeamID = event->GetInt("teamid");
		const char *TeamName = event->GetString("teamname");
	}
	return;
}
void HLStatsIngame::ClientSay()
{
	char firstword[48];
	_snprintf(firstword,47,"%s",g_MSCore.GetEngine()->Cmd_Argv(1));
	int id = g_IndexOfLastUserCmd;

	/*
	RETURN_META(MRES_SUPERCEDE);
	else
		RETURN_META(MRES_IGNORED);
*/
	if(g_BlockSayChat && (strcmp(firstword,"/rank") == 0 || strcmp(firstword,"rank") == 0))
	{
		// What we fake
		//L 12/28/2006 - 21:17:16: "EKS<2><STEAM_ID_LAN><CT>" say "say OMG"
		char FakeSay[192];
		_snprintf(FakeSay,191,"\"%s<%d><%s><%s>\" say \"/rank\"\n",g_MSCore.GetPlayerName(id),UserInfo[id].Userid,UserInfo[id].Steamid,g_MSCore.GetTeamName(id));

		g_MSCore.GetEngine()->LogPrint(FakeSay);
		RETURN_META(MRES_SUPERCEDE);
	}
	else if(g_BlockSayChat && (strcmp(firstword,"stats") == 0 || strcmp(firstword,"/stats") == 0 || strcmp(firstword,"statsme") == 0 || strcmp(firstword,"/statsme") == 0))
	{
		char FakeSay[192];
		_snprintf(FakeSay,191,"\"%s<%d><%s><%s>\" say \"/statsme\"\n",g_MSCore.GetPlayerName(id),UserInfo[id].Userid,UserInfo[id].Steamid,g_MSCore.GetTeamName(id));

		g_MSCore.GetEngine()->LogPrint(FakeSay);
		RETURN_META(MRES_SUPERCEDE);
	}
	else if(g_BlockSayChat && (strcmp(firstword,"top10") == 0 || strcmp(firstword,"/top10") == 0))
	{
		char FakeSay[192];
		_snprintf(FakeSay,191,"\"%s<%d><%s><%s>\" say \"/top10\"\n",g_MSCore.GetPlayerName(id),UserInfo[id].Userid,UserInfo[id].Steamid,g_MSCore.GetTeamName(id));

		g_MSCore.GetEngine()->LogPrint(FakeSay);
		RETURN_META(MRES_SUPERCEDE);
	}
	else if(g_BlockSayChat && (strcmp(firstword,"session") == 0 || strcmp(firstword,"/session") == 0 || strcmp(firstword,"session_data") == 0 || strcmp(firstword,"/session_data") == 0))
	{
		char FakeSay[192];
		_snprintf(FakeSay,191,"\"%s<%d><%s><%s>\" say \"/session\"\n",g_MSCore.GetPlayerName(id),UserInfo[id].Userid,UserInfo[id].Steamid,g_MSCore.GetTeamName(id));

		g_MSCore.GetEngine()->LogPrint(FakeSay);
		RETURN_META(MRES_SUPERCEDE);
	}
#if UseMoreStats == 1
	else if(g_ShowMoreStats >= 1 && ( strcmp(firstword,"/morestats") == 0 || strcmp(firstword,"morestats") == 0))
	{
		if(g_MoreUserStats[id].ShowStats)
		{
			g_MSCore.MessagePlayer(id,"%s Showing of MoreStats is now disabled",StartStringTag);
			g_MoreUserStats[id].ShowStats = false;
		}
		else
		{
			if(g_ShowMoreStats == 1)
				g_MSCore.MessagePlayer(id,"%s Showing of MoreStats is now enabled, will show on death and round start",StartStringTag);
			else
				g_MSCore.MessagePlayer(id,"%s Showing of MoreStats is now enabled, will show on death",StartStringTag);

			g_MoreUserStats[id].ShowStats = true;
		}
	}
#endif
	else if(strcmp(firstword,"/nostats") == 0)
	{
		if(UserInfo[id].ShowStats)
		{
			g_MSCore.MessagePlayer(id,"%s Showing of ingame stats is now disabled, enable again by 'say /nostats'",StartStringTag);
			UserInfo[id].ShowStats = false;
		}
		else
		{
			g_MSCore.MessagePlayer(id,"%s Showing of ingame stats is now enabled",StartStringTag);
			UserInfo[id].ShowStats = true;
		}
	}
}


void HLStatsIngame::LoadPluginSettings(int clientMax)
{	
	if(g_FirstLoad == false)
	{
		SetupModSpesficInfo();
		m_Stats = new Stats;
		m_SteamIDList = new SteamIDList;
		m_SteamIDList->ReadSteamIDFile();


		g_FirstLoad = true;
		g_MaxClients = clientMax;

		g_pEventPlayerDeath = new EventListenPlayerDeath;
		g_pEventPlayerHurt = new EventListenPlayerHurt;
		g_pEventPlayerFire = new EventListenPlayerWeaponFire;

		m_GameEventManager->AddListener(g_pEventPlayerDeath,"player_death",true);
		m_GameEventManager->AddListener(g_pEventPlayerHurt,"player_hurt",true);
		m_GameEventManager->AddListener(g_pEventPlayerDeath,"weapon_fire",true);
		
		if(g_ModIndex == MOD_CSTRIKE)
		{
			m_GameEventManager->AddListener(this,"round_end",true);
			m_GameEventManager->AddListener(this,"team_info",true);

#if UseMoreStats == 1
			m_GameEventManager->AddListener(this,"round_freeze_end",true);
#endif
		}
		else if(g_ModIndex == MOD_DOD)
		{
			m_GameEventManager->AddListener(this,"dod_round_start",true);			
		}
		HLSVars::SetupCallBacks();

		pSayCmd = HookConsoleCmd("say");
		if(!pSayCmd) pSayCmd = HookConsoleCmd("say2");
		pSayTeamCmd = HookConsoleCmd("say_team");
	}
	for(int i=1;i<=g_MaxClients;i++)
		g_IsConnected[i] = false;
	
	g_BlockSayChat = g_HLSVars.BlockSayRank();

	//m_SteamIDList->AddSteamID("STEAM_0:0:78039");
}
//bool HLStatsIngame::FindValveInterface(char *PreferedInterfaceVersion,)
bool HLStatsIngame::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(serverFactory, m_ServerDll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(serverFactory, m_InfoMngr,IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_ANY(serverFactory, m_ServerClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(engineFactory, m_Engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);	
	GET_V_IFACE_CURRENT(engineFactory, m_GameEventManager, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(engineFactory, m_ICvar,ICvar, VENGINE_CVAR_INTERFACE_VERSION);
	//GET_V_IFACE_CURRENT(engineFactory, m_Helpers,IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	//GET_V_IFACE_CURRENT(engineFactory, m_Sound,IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);

	
	//Init our cvars/concmds
	ConCommandBaseMgr::OneTimeInit(&g_HLSVars);

	//We're hooking the following things as POST, in order to seem like Server Plugins.
	//However, I don't actually know if Valve has done server plugins as POST or not.
	//Change the last parameter to 'false' in order to change this to PRE.
	//SH_ADD_HOOK_MEMFUNC means "SourceHook, Add Hook, Member Function".

	//Hook LevelInit to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, m_ServerDll, &g_MSCore, &HLStatsIngame::ServerActivate, true);		//Hook GameFrame to our function
	//SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, m_ServerDll, &g_ForgiveTKCore, &HLStatsIngame::GameFrame, true);				//Hook LevelShutdown to our function -- this makes more sense as pre I guess
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientDisconnect, true);		//Hook ClientPutInServer to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientPutInServer, true);	//Hook SetCommandClient to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, m_ServerClients, &g_MSCore, &HLStatsIngame::SetCommandClient, true);		//Hook ClientSettingsChanged to our function
	//SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientCommand, false);	//This hook is a static hook, no member function


	//The following functions are pre handled, because that's how they are in IServerPluginCallbacks
		
	m_Engine_CC = SH_GET_CALLCLASS(m_Engine); // invoking their hooks (when needed).
	
	
	SH_CALL(m_Engine_CC, &IVEngineServer::LogPrint)("All hooks started!\n");
	if(late)
	{
		LoadPluginSettings(ismm->pGlobals()->maxClients);
		ServerCommand("echo [%s] Late load is not really supported, plugin will function properly after 1 mapchange",StartStringTag);
	}
	return true;
}
bool HLStatsIngame::Unload(char *error, size_t maxlen)
{
	m_SteamIDList->WriteSteamIDFile();

	//IT IS CRUCIAL THAT YOU REMOVE CVARS.
	//As of Metamod:Source 1.00-RC2, it will automatically remove them for you.
	//But this is only if you've registered them correctly!

	//Make sure we remove any hooks we did... this may not be necessary since
	//SourceHook is capable of unloading plugins' hooks itself, but just to be safe.

	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, m_ServerDll, &g_MSCore, &HLStatsIngame::ServerActivate, true);
	//SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, m_ServerDll, &g_ForgiveTKCore, &HLStatsIngame::GameFrame, true);
	//SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, m_ServerClients, &g_ForgiveTKCore, &HLStatsIngame::ClientActive, true);
	
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, m_ServerClients, &g_MSCore, &HLStatsIngame::SetCommandClient, true);
	//SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, m_ServerClients, &g_MSCore, &HLStatsIngame::ClientCommand, false);

	if(pSayCmd) RemoveHookConsoleCmd(pSayCmd);
	if(pSayTeamCmd) RemoveHookConsoleCmd(pSayTeamCmd);

	m_GameEventManager->RemoveListener(g_pEventPlayerHurt);
	m_GameEventManager->RemoveListener(g_pEventPlayerHurt);
	m_GameEventManager->RemoveListener(g_pEventPlayerFire);
	
	m_GameEventManager->RemoveListener(this);  // This will be your basic events that dont fire often, like round_end

	//this, sourcehook does not keep track of.  we must do this.
	SH_RELEASE_CALLCLASS(m_Engine_CC);


	return true;
}
void HLStatsIngame::AllPluginsLoaded()
{

}


void HLStatsIngame::Client_Authorized(int id)
{
#if HLX_DEBUG == 1
	ServerCommand("echo [ForgiveTK Debug]Client_Authorized: %d",id);
#endif
}
ConCommand *HLStatsIngame::HookConsoleCmd(const char *CmdName)
{
	ConCommandBase *pCmd = m_ICvar->GetCommands();
	ConCommand* pTemp;

	while (pCmd)
	{
		if (pCmd->IsCommand() && stricmp(pCmd->GetName(),CmdName) == 0) 
		{
			pTemp = (ConCommand *)pCmd;
			SH_ADD_HOOK_MEMFUNC(ConCommand, Dispatch, pTemp, &g_MSCore, &HLStatsIngame::ClientSay, false);

			// How sslice does it
			//SH_ADD_HOOK_STATICFUNC(ConCommand, Dispatch, pTemp, BATCore::ClientSay, false);
			return pTemp;
		}

		pCmd = const_cast<ConCommandBase *>(pCmd->GetNext());
	}
	return NULL;
}
void HLStatsIngame::RemoveHookConsoleCmd(ConCommand *pTheCmd)
{
	
	SH_REMOVE_HOOK_MEMFUNC(ConCommand, Dispatch, pTheCmd ,&g_MSCore, &HLStatsIngame::ClientSay, false);	

	//SH_REMOVE_HOOK_STATICFUNC(ConCommand, Dispatch, pTheCmd, BATCore::ClientSay, false);
}