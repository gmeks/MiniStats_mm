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

#include <ISmmPlugin.h>
#include <sourcehook/sourcehook.h>
#include <igameevents.h>
#include "ienginesound.h"
#include <iplayerinfo.h>
#include "convar.h"
#include "cvars.h"
#include "Stats.h"
#include "const.h"
#include "HLStatsX.h"

extern Stats *m_Stats;
extern bool g_IsConnected[MAXPLAYERS+1];
extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern ConstMorePlayerStats g_MoreUserStats[MAXPLAYERS+1];
extern int g_ModIndex;
extern int g_ShowMoreStats;

void EventListenPlayerHurt::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName())
		return;

	const char *EventName = event->GetName();

	int victim = g_MSCore.FindPlayer(event->GetInt("userid"));
	int attacker = g_MSCore.FindPlayer(event->GetInt("attacker"));
	if(attacker <= 0) // Hurt by some entity perhaps its the world 
		return;

	int Dmg = event->GetInt("dmg_health");
	int HitGroup = event->GetInt("hitgroup");
	int WeaponIndex = g_MSCore.GetWeaponIndex(event->GetString("weapon"));

	if(WeaponIndex <= 0) // We dident find the correct weapon index. Screw this im going home
		return;
	m_Stats->AddPlayerAttack(attacker,Dmg,HitGroup,WeaponIndex);

	g_MoreUserStats[attacker].Players[victim].DmgGiven += Dmg;
	g_MoreUserStats[victim].Players[attacker].DmgTaken += Dmg;
}
void EventListenPlayerWeaponFire::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName())
		return;

	const char *EventName = event->GetName();

	int id = g_MSCore.FindPlayer(event->GetInt("userid"));
	int WeaponIndex = g_MSCore.GetWeaponIndex(event->GetString("weapon"));
	if(WeaponIndex <= 0)
		return;

	m_Stats->AddPlayerShot(id,WeaponIndex);
}
void EventListenPlayerDeath::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName())
		return;

	const char *EventName = event->GetName();

	int attacker = g_MSCore.FindPlayer(event->GetInt("attacker"));
	if(attacker <= 0) // Hurt by some entity perhaps its the world 
		return;

	int victim = g_MSCore.FindPlayer(event->GetInt("userid"));
	int WeaponIndex = g_MSCore.GetWeaponIndex(event->GetString("weapon"));
	bool Headshot = event->GetBool("headshot");
	if(WeaponIndex <= 0) // We dident find the correct weapon index. Screw this im going home
		return;
	bool TeamKill = false;

	if(g_MSCore.GetUserTeam(victim) == g_MSCore.GetUserTeam(attacker))
		TeamKill = true;
	m_Stats->AddPlayerAttackKill(attacker,Headshot,WeaponIndex,TeamKill);

	g_MoreUserStats[attacker].Players[victim].Killed = true;
	g_MoreUserStats[victim].Players[attacker].Killer = true;

	if(g_HLSVars.ShowHPLeft() && !UserInfo[victim].IsBot && victim != attacker)
	{
		IPlayerInfo *pInfo = g_MSCore.PlayerInfo()->GetPlayerInfo(UserInfo[attacker].PlayerEdict);
		if(!pInfo)
			return;

		//char ShortWeaponName[32];
		//_snprintf(ShortWeaponName,31,"%s",pInfo->GetWeaponName());
		//g_MSCore.StrReplace(ShortWeaponName,"weapon_","",31);
		const char *pWeapon = pInfo->GetWeaponName();
		if(!pWeapon)
			return;
		
		if(strlen(pWeapon) > 7)
			pWeapon = &(pWeapon[7]);
		
		g_MSCore.MessagePlayer(victim,"%s %s has %d HP left after killing you with a %s",StartStringTag,g_MSCore.GetPlayerName(attacker),pInfo->GetHealth(),pWeapon);

#if UseMoreStats == 1
		if(g_ShowMoreStats != 0 && g_MoreUserStats[victim].ShowStats)
		{
			m_Stats->ShowMoreStats(victim);
		}
#endif
	}

	if(g_ModIndex != MOD_CSTRIKE) // We need to add extra logging info to the log, in CS this is done on round end
	{
		m_Stats->LogPlayerStats(victim);
	}
}