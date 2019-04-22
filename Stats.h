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

#ifndef _INCLUDE_STATS_CLASS
#define _INCLUDE_STATS_CLASS

#include "const.h"

//extern menuId g_AdminMenu;
class Stats
{
public:
	void AddPlayerAttack(int id,int dmg,int hitgroup,int WeaponIndex);
	void AddPlayerAttackKill(int id,bool Headshot,int WeaponIndex,bool TeamKill);
	void AddPlayerShot(int id,int WeaponIndex);		// We call this everytime a weapon is fired.
	void ResetStatsArray(int id);
	void LogPlayerStats(int id);
#if UseMoreStats == 1
	void ShowMoreStats();  // Shows to all enabled players, and clears old stats when their shown
	void ClearMoreStats();  // Shows to all enabled players, and clears old stats when their shown
	void ShowMoreStats(int id); // Shows only the stats to a spesfic player
#endif
private:
	//char *GetDifficulty();
	void ResetMoreStats(int id);
	void GenMoreStats(int id);
	void ClearMoreStats(int id);
	const char* GetHitGroupName(int HitGroup);
	ConstWeaponStats *FastResetWeaponStats(ConstWeaponStats *WeaponStats);
	
	WeaponStatsReturnStruct GetWeaponStatsIndex(int id,int WeaponIndex);
};
#endif