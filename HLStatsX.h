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

#ifndef _INCLUDE_SAMPLEPLUGIN_H
#define _INCLUDE_SAMPLEPLUGIN_H

#include <ISmmPlugin.h>
#include <sourcehook/sourcehook.h>
#include <igameevents.h>
#include "ienginesound.h"
#include <iplayerinfo.h>
#include "convar.h"
#include "cvars.h"
#include "const.h"


//SH_DECL_HOOK0_void(ConCommand, Dispatch, SH_NOATTRIB, 0);
class StrUtil
{
public:
	int StrReplace(char *str, const char *from, const char *to, int maxlen); // Replaces part of a string with something else
	char* StrRemoveQuotes(char *text);			// Removes Quotes from the string
	int GetFirstIndexOfChar(char *Text,int MaxLen,unsigned char t);	// Gets index of the instance of t in a char array
	void StrTrimLeft(char *buffer);	// Trim from the left ( Removes white spaces )
	void StrTrimRight(char *buffer); // Trim from the right ( Removes white spaces )
	void StrTrim(char *buffer);		// Trims the string ( Removes white spaces )
	bool StrIsSpace(unsigned char b);	// If the char is a space or whitespace
	unsigned long StrHash(register char *str, register int len); // Hashes the string  ( Uses dbm hash ( Duff's device version )
};

class HLStatsIngame : public ISmmPlugin, public IMetamodListener, public IGameEventListener2, public StrUtil
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	void AllPluginsLoaded();
	bool Pause(char *error, size_t maxlen)
	{
		return true;
	}
	bool Unpause(char *error, size_t maxlen)
	{
		return true;
	}
public:
	int GetApiVersion() { return PLAPI_VERSION; }
public:
	const char *GetAuthor()
	{
		return "EKS";
	}
	const char *GetName()
	{
		return "MiniStats"; // HLX Ingame
	}
	const char *GetDescription()
	{
		return "Generates the extra stats needed for hlstatsx to do stats and helps showing them ingame, and can show killer hp/weapon and end of roundstats";
	}
	const char *GetURL()
	{
		return "http://www.sourcemm.net/";
	}
	const char *GetLicense()
	{
		return "zlib/libpng";
	}
	const char *GetVersion()
	{
		return HLX_VERSION;
	}
	const char *GetDate()
	{
		return __DATE__;
	}
	const char *GetLogTag()
	{
		return "MS";
	}
public:
	//These functions are from IServerPluginCallbacks
	//Note, the parameters might be a little different to match the actual calls!


	//Called on ServerActivate.  Same definition as server plugins
	void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);
	//Called when a client uses a command.  Unlike server plugins, it's void.
	// You can still supercede the gamedll through RETURN_META(MRES_SUPERCEDE).
	void ClientCommand(edict_t *pEntity);
	//Called on a game tick.  Same definition as server plugins
	void GameFrame(bool simulating);
	//Client disconnects - same as server plugins
	void ClientDisconnect(edict_t *pEntity);
	//Client is put in server - same as server plugins
	void ClientPutInServer(edict_t *pEntity, char const *playername);
	//Sets the client index - same as server plugins
	void SetCommandClient(int index);
	// Where the plugin catches events.
	void FireGameEvent(IGameEvent *event);
	void ClientSay();

	IVEngineServer *GetEngine() { return m_Engine; }
	IPlayerInfoManager *PlayerInfo() { return m_InfoMngr; }
	IServerPluginHelpers *GetHelpers() {return m_Helpers;}
	IPlayerInfoManager *GetPlayerInfo() { return m_InfoMngr; }

	HLSVars GetHLSVar() { return g_HLSVars; }
	void MessagePlayer(int index, const char *msg, ...);
	void SendCSay(int id,const char *msg, ...);
	void ServerCommand(const char *fmt, ...);
	void SrvLog(const char *fmt, ...);
	void SendHintMsg(int id,char *text);
	void SendHintMsgNoFormat(int id,char *text);

	int GetUserTeam(int id);
	int FindPlayer(int UserID);
	int FindPlayer(char *Name);

	bool IsUserAlive(int id);
	bool HasAdminImmunity(int id);

	const char *GetPlayerName(int id);
	int GetWeaponIndex(const char *WeaponName);
	char* GetWeaponName(int WeaponIndex);
	const char *GetTeamName(int id);
	
	void Client_Authorized(int id);

	CBaseEntity *GetCBaseEntity(int id);

private:
	IGameEventManager2 *m_GameEventManager;	
	IVEngineServer *m_Engine;
	IServerGameDLL *m_ServerDll;
	IServerGameClients *m_ServerClients;
	IPlayerInfoManager *m_InfoMngr;
	IServerPluginHelpers *m_Helpers;
	IEngineSound *m_Sound;
	ICvar *m_ICvar;
	SourceHook::CallClass<IVEngineServer> *m_Engine_CC;
	ConCommand *pSayCmd;
	ConCommand *pSayTeamCmd;

	void LoadPluginSettings(int clientMax);
	void GetModName();
	int GetMsgNum(char *Msg);
	void SetupModSpesficInfo();
	int GetNextSpaceCount(char *Text,int CurIndex);


	ConCommand *HookConsoleCmd(const char *CmdName);
	void RemoveHookConsoleCmd(ConCommand *pTheCmd);
};

extern HLStatsIngame g_MSCore;
extern HLSVars g_HLSVars;
PLUGIN_GLOBALVARS();



// This retarded? nooo lies
class EventListenPlayerDeath :  public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent *event); 
};
class EventListenPlayerHurt :  public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent *event); 
};
class EventListenPlayerWeaponFire :  public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent *event); 
};
#endif //_INCLUDE_SAMPLEPLUGIN_H
