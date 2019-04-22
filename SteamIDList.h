/* ======== SimpleStats ========
* Copyright (C) 2004-2007 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Adminloading function from Xaphan ( http://www.sourcemod.net/forums/viewtopic.php?p=25807 ) Created by Devicenull
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#ifndef _INCLUDE_STEAMIDLIST
#define _INCLUDE_STEAMIDLIST

#include "HLStatsX.h"

class SteamIDList : StrUtil
{
public:
	//static void CmdPSay();
	void AddSteamID(char *SteamID);
	void RemoveSteamID(char *SteamID);
	bool IsSteamIDInList(char *SteamID);

	void ReadSteamIDFile();
	void WriteSteamIDFile();

private:
	ConstSteamIDInfo GetConstSteamIDInfo(char *SteamID);
	void AddSteamID(char *SteamID,uint DaysSinceOnline);
	int GetDaysSince1970();
	void GrowSteamIDList();

};
#endif