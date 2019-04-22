/* ======== SimpleStats ========
* Copyright (C) 2004-2007 Erling K. S�terdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. S�terdal ( EKS )
* Credits:
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#include "SteamIDList.h"
#include <sourcehook/sourcehook.h>
#include <sh_vector.h>
#include <time.h>

SourceHook::CVector<ConstSteamIDInfo *>g_SteamIDList;
uint g_SteamIDListCount =0;
/**********************	Public functions **********************/
void SteamIDList::AddSteamID(char *SteamID)
{
	if(g_SteamIDListCount >= g_SteamIDList.size()-1 || g_SteamIDList.size() == 0)
		GrowSteamIDList();

	//STEAM_0:0:78039
	ConstSteamIDInfo tSteamid = GetConstSteamIDInfo(SteamID);
	if(tSteamid.SteamIDNum ==0) // Something went wrong parsing that steamid
	{
		g_MSCore.ServerCommand("echo ERROR parsing steamid: %s",SteamID);
		return;
	}
	_snprintf(g_SteamIDList[g_SteamIDListCount]->SteamID,MAX_NETWORKID_LENGTH,"%s",SteamID);
	g_SteamIDList[g_SteamIDListCount]->FirstNum = tSteamid.FirstNum;
	g_SteamIDList[g_SteamIDListCount]->SecNum = tSteamid.SecNum;
	g_SteamIDList[g_SteamIDListCount]->ThirdNum = tSteamid.ThirdNum;
	g_SteamIDList[g_SteamIDListCount]->SteamIDNum = tSteamid.SteamIDNum;
	g_SteamIDList[g_SteamIDListCount]->DaysSizeLastUse = GetDaysSince1970();
	g_SteamIDListCount++;
}
void SteamIDList::RemoveSteamID(char *SteamID)
{
	ConstSteamIDInfo tSteamid = GetConstSteamIDInfo(SteamID);
	if(tSteamid.SteamIDNum ==0) // Something went wrong parsing that steamid
	{
		g_MSCore.ServerCommand("echo ERROR parsing steamid: %s",SteamID);
		return;
	}
	
	SourceHook::CVector<ConstSteamIDInfo *>::iterator i;
	for(i=g_SteamIDList.begin();i != g_SteamIDList.end(); i++)
	{
		ConstSteamIDInfo *tSteamid2 = (*i);
		if(tSteamid.SteamIDNum == tSteamid2->SteamIDNum && tSteamid.FirstNum == tSteamid2->FirstNum  && tSteamid.SecNum == tSteamid2->SecNum && tSteamid.ThirdNum == tSteamid2->ThirdNum)
		{
			g_SteamIDList.erase(i);
			break;
		}
	}
}
bool SteamIDList::IsSteamIDInList(char *SteamID)
{
	ConstSteamIDInfo tSteamid = GetConstSteamIDInfo(SteamID);
	if(tSteamid.SteamIDNum ==0) // Something went wrong parsing that steamid
	{
		g_MSCore.ServerCommand("echo ERROR parsing steamid: %s",SteamID);
		return false;
	}
	if(g_SteamIDListCount == 0)
		return false;
	
	for(uint i=0;i<g_SteamIDListCount;i++)
	{
		if(tSteamid.SteamIDNum == g_SteamIDList[i]->SteamIDNum && tSteamid.FirstNum == g_SteamIDList[i]->FirstNum  && tSteamid.SecNum == g_SteamIDList[i]->SecNum && tSteamid.ThirdNum == g_SteamIDList[i]->ThirdNum)
		{
			g_SteamIDList[i]->DaysSizeLastUse = GetDaysSince1970();
			return true;
		}
	}
	return false;
}
void SteamIDList::ReadSteamIDFile() 
{
	//Begin loading admins
	char temp[786], tdir[512],Line[512];	//This gets the actual path to the users.ini file.
	char *pLine;

	g_MSCore.GetEngine()->GetGameDir(tdir,512);
	_snprintf(temp,785,STEAMID_FILE,tdir);
	FILE *is = NULL;
	is = fopen(temp,"rt");

	if (!is)
	{
		_snprintf(temp,785,"ERROR: Unable to find Old steamid file, should be in ( %s )\n",STEAMID_FILE);
		g_MSCore.GetEngine()->LogPrint(temp);
		return;
	}

	int TextIndex=0;
	int LineNum=0;
	int iLineLen;

	int IOT;
	int TempDays;

	int CurDaysSize1970 = GetDaysSince1970();
	int OldSteamIDRemoved = 0;
	g_SteamIDList.clear(); // Remove any old entries

	while (!feof(is)) 
	{
		if(g_SteamIDListCount >= g_SteamIDList.size()-1 || g_SteamIDList.size() == 0)
			GrowSteamIDList();
		

		fgets(Line,512,is);
		iLineLen = strlen(Line);
		LineNum++;
		
		if(iLineLen <= 5 || Line[0] == ';' ||  Line[0] == '/')
			continue;
		
		IOT = GetFirstIndexOfChar(Line,MAX_NETWORKID_LENGTH,' ');
		if(IOT == -1)
			return;

		_snprintf(temp,IOT,"%s",Line);
		temp[IOT] = '\0';
		
		pLine = (char *)Line;
		pLine = pLine + IOT;
		TempDays = atoi(pLine);

		if((CurDaysSize1970-TempDays) <= HLX_REMOVEOLDSTEAMIDS)
			AddSteamID(temp,TempDays);
		else
		{
#if HLX_DEBUG == 1
			_snprintf(tdir,511,"HLX Removed %s was %d days old\r\n",temp,(CurDaysSize1970-TempDays));
			g_MSCore.GetEngine()->LogPrint(tdir);
#endif
			OldSteamIDRemoved++;
		}
	}
	fclose(is);

	if(OldSteamIDRemoved > 0)
	{
		_snprintf(tdir,511,"HLX: Removed %d old steamids from list\r\n",OldSteamIDRemoved,(CurDaysSize1970-TempDays));
		g_MSCore.GetEngine()->LogPrint(tdir);
	}
}
void SteamIDList::WriteSteamIDFile()
{
	char Temp[786], tdir[512];

	g_MSCore.GetEngine()->GetGameDir(tdir,512);
	_snprintf(Temp,785,STEAMID_FILE,tdir);

	FILE *FileStream = fopen(Temp, "wt"); // "wt" clears the file each time
	
	if (!FileStream)
	{
		_snprintf(Temp,511,"ERROR: Unable to write steamid file, should be in ( %s )",STEAMID_FILE);
		g_MSCore.GetEngine()->LogPrint(Temp);
		return;
	}
	
	SourceHook::CVector<ConstSteamIDInfo *>::iterator i;
	for(i=g_SteamIDList.begin();i != g_SteamIDList.end(); i++)
	{
		ConstSteamIDInfo *tSteamid2 = (*i);
		
		if(tSteamid2->DaysSizeLastUse == -1)
			continue;

		_snprintf(Temp,511,"%s %d\r\n",tSteamid2->SteamID,tSteamid2->DaysSizeLastUse);
		fputs(Temp,FileStream);
	}
	fclose(FileStream);
}
/**********************	Private functions **********************/
ConstSteamIDInfo SteamIDList::GetConstSteamIDInfo(char *SteamID)
{
	if(g_SteamIDListCount >= g_SteamIDList.size()-1 || g_SteamIDList.size() == 0)
		GrowSteamIDList();

	//STEAM_0:0:78039
	ConstSteamIDInfo tSteamid;
	tSteamid.SteamIDNum = 0; // If we return this ConstSteamIDInfo like this, the other functions should check for the 0, and if it has it. Then something went wrong
	char tText[MAX_NETWORKID_LENGTH];
	_snprintf(tSteamid.SteamID,MAX_NETWORKID_LENGTH,"%s",SteamID);

	int SkipInt = 6; // this removes the STEAM_

	int IOT = GetFirstIndexOfChar(SteamID+SkipInt,MAX_NETWORKID_LENGTH-(SkipInt+1),':');	
	if(IOT <= 0)
		return tSteamid;

	_snprintf(tText,IOT,"%s",SteamID+SkipInt);
	tText[IOT]  = '\0';
	tSteamid.FirstNum = atoi(tText);

	SkipInt += IOT + 1;
	IOT = GetFirstIndexOfChar(SteamID+SkipInt,MAX_NETWORKID_LENGTH-(SkipInt+1),':');
	if(IOT <= 0)
		return tSteamid;

	_snprintf(tText,IOT,"%s",SteamID+SkipInt);
	tText[IOT] = '\0';
	tSteamid.SecNum = atoi(tText);

	SkipInt += IOT + 1;
	_snprintf(tText,MAX_NETWORKID_LENGTH,"%s",SteamID+SkipInt);
	tSteamid.ThirdNum = atoi(tText);

	// The steamid is now nice and split up.
	tSteamid.SteamIDNum = tSteamid.FirstNum + tSteamid.SecNum + tSteamid.ThirdNum;

	return tSteamid;
}
void SteamIDList::GrowSteamIDList()
{
	ConstSteamIDInfo *tSteamid = new ConstSteamIDInfo;
	tSteamid->DaysSizeLastUse = -1;
	g_SteamIDList.push_back(tSteamid);
}
int SteamIDList::GetDaysSince1970()
{
	time_t seconds;
	seconds = time (NULL);

	return  seconds/86400;
}
void SteamIDList::AddSteamID(char *SteamID,uint DaysSinceOnline)
{
	if(g_SteamIDListCount >= g_SteamIDList.size()-1 || g_SteamIDList.size() == 0)
		g_SteamIDList.push_back(new ConstSteamIDInfo);

	//STEAM_0:0:78039
	ConstSteamIDInfo tSteamid = GetConstSteamIDInfo(SteamID);
	if(tSteamid.SteamIDNum ==0) // Something went wrong parsing that steamid
	{
		g_MSCore.ServerCommand("echo ERROR parsing steamid: %s",SteamID);
		return;
	}
	_snprintf(g_SteamIDList[g_SteamIDListCount]->SteamID,MAX_NETWORKID_LENGTH,"%s",SteamID);
	g_SteamIDList[g_SteamIDListCount]->FirstNum = tSteamid.FirstNum;
	g_SteamIDList[g_SteamIDListCount]->SecNum = tSteamid.SecNum;
	g_SteamIDList[g_SteamIDListCount]->ThirdNum = tSteamid.ThirdNum;
	g_SteamIDList[g_SteamIDListCount]->SteamIDNum = tSteamid.SteamIDNum;
	g_SteamIDList[g_SteamIDListCount]->DaysSizeLastUse = DaysSinceOnline;
	g_SteamIDListCount++;
}