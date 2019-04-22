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

#include "HLStatsX.h"
#include "Stats.h"

extern bool g_IsConnected[MAXPLAYERS+1];
extern HLStatsIngame g_MSCore;
extern int g_MaxClients;
extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern ConstPlayerStats UserStats[MAXPLAYERS+1];
extern ConstMorePlayerStats g_MoreUserStats[MAXPLAYERS+1];

extern HLStatsIngame g_MSCore;
extern IPlayerInfoManager *m_InfoMngr;
extern int g_ModIndex;

#if UseMoreStats == 1
void Stats::ShowMoreStats()
{
	for(int id=1;id<=g_MaxClients;id++) if(g_IsConnected[id] && !UserInfo[id].IsBot)
	{
		if(g_MoreUserStats[id].ShowStats)
			ShowMoreStats(id);
		else
			ClearMoreStats(id);
	}
}
void Stats::ClearMoreStats()
{
	for(int id=1;id<=g_MaxClients;id++) if(g_IsConnected[id] && !UserInfo[id].IsBot)
	{
		ClearMoreStats(id);
	}
}
void Stats::ShowMoreStats(int id)
{
	if(g_MoreUserStats[id].StatsStringMade == false)
		GenMoreStats(id);
	
	if(g_MoreUserStats[id].StatsToShow == false)		
		return;

	g_MSCore.MessagePlayer(id,g_MoreUserStats[id].StatsString);
}
void Stats::GenMoreStats(int id)
{
	char VictimBuffer[120];
	char AttacerBuffer[120];

	int VictimBufferLen = 0;
	int AttacerBufferLen = 0;

	int DmgGivenOut=0;
	int DmgTaken=0;

	for(int i=1;i<=g_MaxClients;i++)
	{
		if(g_MoreUserStats[id].Players[i].DmgGiven > 0)
		{
			DmgGivenOut += g_MoreUserStats[id].Players[i].DmgGiven;

			if(g_MoreUserStats[id].Players[i].Killed)
				VictimBufferLen += _snprintf(&(VictimBuffer[VictimBufferLen]),120-VictimBufferLen,"%s (Killed - %d)\n",g_MSCore.GetPlayerName(i),g_MoreUserStats[id].Players[i].DmgGiven);
			else
				VictimBufferLen += _snprintf(&(VictimBuffer[VictimBufferLen]),120-VictimBufferLen,"%s (%d)\n",g_MSCore.GetPlayerName(i),g_MoreUserStats[id].Players[i].DmgGiven);
		}
		if(g_MoreUserStats[id].Players[i].DmgTaken > 0)
		{
			DmgTaken += g_MoreUserStats[id].Players[i].DmgTaken;

			if(g_MoreUserStats[id].Players[i].Killer)
				AttacerBufferLen += _snprintf(&(AttacerBuffer[AttacerBufferLen]),120-AttacerBufferLen,"%s (Killer - %d)\n",g_MSCore.GetPlayerName(i),g_MoreUserStats[id].Players[i].DmgTaken);
			else
				AttacerBufferLen += _snprintf(&(AttacerBuffer[AttacerBufferLen]),120-AttacerBufferLen,"%s (%d)\n",g_MSCore.GetPlayerName(i),g_MoreUserStats[id].Players[i].DmgTaken);
		}
/*
// 		g_MoreUserStats[id].Players[i].DmgGiven = 0;
// 		g_MoreUserStats[id].Players[i].DmgTaken = 0;
// 		g_MoreUserStats[id].Players[i].Killed = false;
// 		g_MoreUserStats[id].Players[i].Killer = false;
*/
	}
	if(VictimBufferLen == 0 && AttacerBufferLen == 0 ) // No stats to show, we halt on this user
	{
		g_MoreUserStats[id].StatsToShow = false;
		g_MoreUserStats[id].StatsStringMade = false;
		return;
	}

	VictimBuffer[VictimBufferLen] = '\0';
	AttacerBuffer[AttacerBufferLen] = '\0';

	if(DmgGivenOut > 0 && DmgTaken > 0)
		_snprintf(g_MoreUserStats[id].StatsString,239,"\x04 Victims \x01: ( %d Dmg given out )\n%s\n\x04 Attackers \x01: ( %d Dmg taken )\n%s",DmgGivenOut,VictimBuffer,DmgTaken,AttacerBuffer);
	else if(DmgGivenOut > 0)
		_snprintf(g_MoreUserStats[id].StatsString,239,"\x04 Victims \x01: ( %d Dmg given out )\n%s",DmgGivenOut,VictimBuffer);
	else
		_snprintf(g_MoreUserStats[id].StatsString,239,"\x04 Attackers \x01: ( %d Dmg taken )\n%s",DmgTaken,AttacerBuffer);

	g_MoreUserStats[id].StatsStringMade = true;
	g_MoreUserStats[id].StatsToShow = true;
}
void Stats::ClearMoreStats(int id)
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		g_MoreUserStats[id].Players[i].DmgGiven = 0;
		g_MoreUserStats[id].Players[i].DmgTaken = 0;
		g_MoreUserStats[id].Players[i].Killed = false;
		g_MoreUserStats[id].Players[i].Killer = false;
	}

	g_MoreUserStats[id].StatsStringMade = false;
	g_MoreUserStats[id].StatsToShow = false;
}
#endif
void Stats::AddPlayerShot(int id,int WeaponIndex)
{
	WeaponStatsReturnStruct WStatsR = GetWeaponStatsIndex(id,WeaponIndex);
	
	if(WStatsR.ArrayIndex == -1)
		return;

	WStatsR.WeaponStats->Miss++;
	WStatsR.WeaponStats->Shots++;
	UserStats[id].WeaponStats[WStatsR.ArrayIndex] = WStatsR.WeaponStats;
}
void Stats::AddPlayerAttackKill(int id,bool Headshot,int WeaponIndex,bool TeamKill)
{
	UserStats[id].HasDoneSomething = true;
	WeaponStatsReturnStruct WStatsR = GetWeaponStatsIndex(id,WeaponIndex);
	
	if(WStatsR.ArrayIndex == -1)
		return;

	WStatsR.WeaponStats->Kills++;
	if(Headshot)
		WStatsR.WeaponStats->HKills++;
	if(TeamKill)
		WStatsR.WeaponStats->TKills++;

	UserStats[id].WeaponStats[WStatsR.ArrayIndex] = WStatsR.WeaponStats;
}
void Stats::AddPlayerAttack(int id, int Dmg, int HitGroup,int WeaponIndex)
{
	UserStats[id].HasDoneSomething = true;
	int ArrayIndex;		// This is the index we are gonna use in the WeaponStats array.
	ConstWeaponStats *WeaponStats;

	WeaponStatsReturnStruct WStatsR = GetWeaponStatsIndex(id,WeaponIndex);

	WeaponStats = WStatsR.WeaponStats;
	ArrayIndex = WStatsR.ArrayIndex;

	WeaponStats->Damage += Dmg;
	WeaponStats->Hits++;
	WeaponStats->Miss--; // Since this shot clearly hit someone, we need to remove 1 miss. As we first record every shot a miss, then remove the miss when its did damage

	switch(HitGroup)
	{
	case HITGROUP_HEAD:
		WeaponStats->HitBoxHead++;
		break;
	case HITGROUP_CHEST:
		WeaponStats->HitBoxChest++;
		break;
	case HITGROUP_LEFTARM:
		WeaponStats->HitBoxLeftArm++;
		break;
	case HITGROUP_RIGHTARM:
		WeaponStats->HitBoxRightArm++;
		break;
	case HITGROUP_LEFTLEG:
		WeaponStats->HitBoxLeftLeg++;
		break;
	case HITGROUP_RIGHTLEG:
		WeaponStats->HitBoxLeftLeg++;
		break;
	}
	UserStats[id].WeaponStats[ArrayIndex] = WeaponStats;
}
// Headshot on first line? Is that headshot or kills via headshot?
void Stats::LogPlayerStats(int id)
{
	if(!UserStats[id].HasDoneSomething)
		return;

	char StartOfLog[200];
	_snprintf(StartOfLog,199,"\"%s<%d><%s><%s>\" triggered \"weaponstats",g_MSCore.GetPlayerName(id),UserInfo[id].Userid,UserInfo[id].Steamid,g_MSCore.GetTeamName(id));
	
	for(int i=1;i<MAX_WEAPONS;i++)
	{
		ConstWeaponStats *WeaponStats = UserStats[id].WeaponStats[i];
		
		if(!WeaponStats->WeaponUsed)
			break;
		

		g_MSCore.SrvLog("%s\" (weapon \"%s\") (shots \"%d\") (hits \"%d\") (kills \"%d\") (headshots \"%d\") (tks \"%d\") (damage \"%d\") (deaths \"%d\")",StartOfLog,g_MSCore.GetWeaponName(WeaponStats->WeaponIndex),WeaponStats->Shots,WeaponStats->Hits,WeaponStats->Kills,WeaponStats->HKills,WeaponStats->TKills,WeaponStats->Damage,WeaponStats->Death,WeaponStats->Death);
		g_MSCore.SrvLog("%s2\" (weapon \"%s\") (head \"%d\") (chest \"%d\") (stomach \"%d\") (leftarm \"%d\") (rightarm \"%d\") (leftleg \"%d\") (rightleg \"%d\")",StartOfLog,g_MSCore.GetWeaponName(WeaponStats->WeaponIndex),WeaponStats->HitBoxHead,WeaponStats->HitBoxChest,WeaponStats->HitBoxStomach,WeaponStats->HitBoxLeftArm,WeaponStats->HitBoxRightArm,WeaponStats->HitBoxLeftLeg,WeaponStats->HitBoxRightLeg);
		
		UserStats[id].WeaponStats[i] = FastResetWeaponStats(WeaponStats);
	}	
	UserStats[id].HasDoneSomething = false;
	g_MoreUserStats[id].StatsStringMade = false; // This in effect resets the morestats
}
/*
L 06/18/2006 - 17:54:52: "Perry<3><BOT><TERRORIST>" triggered "weaponstats" (weapon "galil") (shots "4") (hits "4") (kills "2") (headshots "1") (tks "0") (damage "199") (death "0")
L 06/18/2006 - 17:54:52: "Perry<3><BOT><TERRORIST>" triggered "weaponstats2" (weapon "galil") (head "1") (chest "0") (stomach "0") (leftarm "1") (rightarm "0")(leftleg "0") (rightleg "0")
*/
/*
From Mani
L 06/18/2006 - 15:26:27: "Pinkey<226><STEAM_0:0:7295768><CT>" triggered "weaponstats" (weapon "ak47") (shots "0") (hits "0") (kills "0") (headshots "0") (tks "0") (damage "0") (deaths "1")
L 06/18/2006 - 15:26:27: "Pinkey<226><STEAM_0:0:7295768><CT>" triggered "weaponstats2" (weapon "ak47") (head "0") (chest "0") (stomach "0") (leftarm "0") (rightarm "0") (leftleg "0") (rightleg "0")

// From statsmeminimum
L 06/11/2006 - 19:49:40: "Eric<22><BOT><TERRORIST>" triggered "weaponstats" (weapon "galil") (shots "4") (hits "2") (kills "1") (headshots "1") (tks "0") (damage "108") (deaths "0")
L 06/11/2006 - 19:49:40: "Eric<22><BOT><TERRORIST>" triggered "weaponstats2" (weapon "galil") (head "1") (chest "1") (stomach "0") (leftarm "0") (rightarm "0")(leftleg "0") (rightleg "0")
*/
const char* Stats::GetHitGroupName(int HitGroup)
{
	switch(HitGroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_CHEST:
		return "stomach";
	case HITGROUP_LEFTARM:
		return "leftarm";
	case HITGROUP_RIGHTARM:
		return "rightarm";
	case HITGROUP_LEFTLEG:
		return "leftleg";
	case HITGROUP_RIGHTLEG:
		return "rightleg";
	}
	return "ERROR";
}
WeaponStatsReturnStruct Stats::GetWeaponStatsIndex(int id,int WeaponIndex)
{
	WeaponStatsReturnStruct WStatsR; //= new WeaponStatsReturnStruct;
	WStatsR.ArrayIndex  = -1;

	if(UserStats[id].HasDoneSomething == false)
	{
		UserStats[id].HasDoneSomething = true;
		WStatsR.WeaponStats = UserStats[id].WeaponStats[1];
		WStatsR.WeaponStats->WeaponUsed = true;
		WStatsR.WeaponStats->WeaponIndex = WeaponIndex;

		WStatsR.ArrayIndex = 0;
		return WStatsR;
	}
	else
	{
		for(int i=1;i<MAX_WEAPONS;i++)
		{
			if(!UserStats[id].WeaponStats[i]->WeaponUsed) // This index is not in use, so we use this one.
			{
				WStatsR.WeaponStats = UserStats[id].WeaponStats[i];
				WStatsR.WeaponStats->WeaponUsed = true;
				WStatsR.WeaponStats->WeaponIndex = WeaponIndex;
				WStatsR.ArrayIndex = i;	// We found the old use of the weapon
				return WStatsR;	
			}
			if(WeaponIndex == UserStats[id].WeaponStats[i]->WeaponIndex)
			{
				WStatsR.WeaponStats = UserStats[id].WeaponStats[i];
				WStatsR.ArrayIndex = i;	// We found the old use of the weapon
				return WStatsR;
			}
		}
	}
	return WStatsR;
}
void Stats::ResetMoreStats(int id)
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		g_MoreUserStats[id].Players[i].DmgGiven = 0;
		g_MoreUserStats[id].Players[i].DmgTaken = 0;
	}
}
void Stats::ResetStatsArray(int id)
{
	UserStats[id].HasDoneSomething = false;

	for(int i=1;i<=g_MaxClients;i++)
	{
//		if(!g_MoreUserStats[id].Players[i])
//			g_MoreUserStats[id].Players[i] = new ConstMoreVictimStats;

		g_MoreUserStats[id].Players[i].DmgGiven = 0;
		g_MoreUserStats[id].Players[i].DmgTaken = 0;
	}

	for(int i=1;i<MAX_WEAPONS;i++)
	{
		ConstWeaponStats *WeaponStats = UserStats[id].WeaponStats[i];
		if(!WeaponStats)
			WeaponStats = new ConstWeaponStats;

		WeaponStats->Damage = 0;
		WeaponStats->HitBoxChest = 0;
		WeaponStats->HitBoxHead = 0;
		WeaponStats->HitBoxLeftArm = 0;
		WeaponStats->HitBoxLeftLeg = 0;
		WeaponStats->HitBoxRightArm = 0;
		WeaponStats->HitBoxRightLeg = 0;
		WeaponStats->HitBoxStomach = 0;
		WeaponStats->Hits = 0;
		WeaponStats->Kills= 0;
		WeaponStats->Miss = 0 ;
		WeaponStats->WeaponIndex = 0;
		WeaponStats->Shots = 0;
		WeaponStats->HKills = 0;
		WeaponStats->TKills = 0;
		WeaponStats->Death = 0;
		WeaponStats->WeaponUsed = false;

		UserStats[id].WeaponStats[i] = WeaponStats;
	}
}
ConstWeaponStats *Stats::FastResetWeaponStats(ConstWeaponStats *WeaponStats)
{
	WeaponStats->Damage = 0;
	WeaponStats->HitBoxChest = 0;
	WeaponStats->HitBoxHead = 0;
	WeaponStats->HitBoxLeftArm = 0;
	WeaponStats->HitBoxLeftLeg = 0;
	WeaponStats->HitBoxRightArm = 0;
	WeaponStats->HitBoxRightLeg = 0;
	WeaponStats->HitBoxStomach = 0;
	WeaponStats->Hits = 0;
	WeaponStats->Kills= 0;
	WeaponStats->Miss = 0 ;
	WeaponStats->WeaponIndex = 0;
	WeaponStats->Shots = 0;
	WeaponStats->HKills = 0;
	WeaponStats->TKills = 0;
	WeaponStats->Death = 0;
	WeaponStats->WeaponUsed = false;
	return WeaponStats;
}