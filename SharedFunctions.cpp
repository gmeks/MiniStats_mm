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
#include "time.h"
#include "cvars.h"
#include "convar.h"
#include "const.h"
#include <string.h>
#include <bitbuf.h>
#include <ctype.h>
#include "hl2sdk/recipientfilters.h"

#define GAME_DLL 1
#include "cbase.h"
//#pragma warning(disable:4996)

extern int g_MaxClients;
extern int g_ModIndex;
extern bool g_IsConnected[MAXPLAYERS+1];
extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern ModSettingsStruct g_ModSettings;
extern IVEngineServer *m_Engine;
extern bool g_IsConnected[MAXPLAYERS+1];

char g_sWeaponList[MAX_WEAPONS+1][MAX_WEAPONNAMELEN+1];
int g_WeaponListCount=0;

void HLStatsIngame::SendCSay(int id,const char *msg, ...) 
{
	char vafmt[192];
	va_list ap;
	va_start(ap, msg);
	_vsnprintf(vafmt,191,msg,ap);
	va_end(ap);

	for(int i=1;i<=g_MaxClients;i++)	
	{
		if(g_IsConnected[i] && UserInfo[i].IsBot == false)
			m_Engine->ClientPrintf(UserInfo[i].PlayerEdict,vafmt);
	}

	if(g_ModIndex == MOD_CSTRIKE || g_ModIndex == MOD_DOD)
	{
		RecipientFilter mrf;
		
		if(id == 0)
			mrf.AddAllPlayers(g_MaxClients);
		else
			mrf.AddPlayer(id);

		bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&mrf), g_ModSettings.TextMsg);
		buffer->WriteByte(HUD_PRINTCENTER);
		buffer->WriteString(msg);
		m_Engine->MessageEnd();
	}
	else
		MessagePlayer(id,vafmt);
}
const char* HLStatsIngame::GetTeamName(int id)
{
	if(g_ModIndex == MOD_CSTRIKE)
	{
		switch (g_MSCore.GetUserTeam(id))
		{
		case 1: 
			return "Spectator";
		case 2: 
			return "TERRORIST";
		case 3: 
			return "Counter-Terrorist";
		}
	}
	else if(g_ModIndex == MOD_DOD)
	{
		switch (g_MSCore.GetUserTeam(id))
		{
		case 1: 
			return "Spectator";
		case 2: 
			return "Allied";
		case 3: 
			return "Axis";
		}
	}
	else if(g_ModIndex == MOD_HL2MP)
	{
		switch (g_MSCore.GetUserTeam(id))
		{
		case 1: 
			return "Spectator";
		case 2: 
			return "Combines";
		case 3: 
			return "Rebels";
		}
	}
	else
	{
		switch (g_MSCore.GetUserTeam(id))
		{
		case 1: 
			return "Spectator";
		case 2: 
			return g_MSCore.GetHLSVar().GetTeam1Name();
		case 3: 
			return g_MSCore.GetHLSVar().GetTeam2Name();
		}
	}
	return "Error";
}
int HLStatsIngame::GetUserTeam(int id)
{
	IPlayerInfo *pInfo = m_InfoMngr->GetPlayerInfo(UserInfo[id].PlayerEdict);
	if(!pInfo || g_IsConnected[id] == false)
		return 0;
	return pInfo->GetTeamIndex();
//	CPlayerState *pPlayerState = m_ServerClients->GetPlayerState(g_UserInfo[id].PlayerEdict);
}
void HLStatsIngame::GetModName()		// This function gets the actual mod name, and not the full pathname
{
	char path[128];
	char ModName[48];
	m_Engine->GetGameDir(path,127);

	const char *modname = path;
	for (const char *iter = path; *iter; ++iter)
		{
		if (*iter == '/' || *iter == '\\') // or something
			modname = iter + 1;
		}
	strncpy(ModName,modname,47);

	if(strcmp(ModName,"cstrike") == 0)
		g_ModIndex = MOD_CSTRIKE;
	else if(strcmp(ModName,"dod") == 0 || stricmp(ModName,"DODC") == 0)
		g_ModIndex = MOD_DOD;
	else if(strcmp(ModName,"hl2mp") == 0)
		g_ModIndex = MOD_HL2MP;
	else 
		g_ModIndex = MOD_NONE;	
}
int HLStatsIngame::GetMsgNum(char *Msg)
{
	char temp[40];
	int dontdoit=0;
	int MsgCount = atoi(m_Engine->Cmd_Argv(1));
	int MaxScan= 20;
	if(g_ModIndex == MOD_CSTRIKE)
		MaxScan = 25;

	for (int i=1;i<=MaxScan;i++) 
	{
		m_ServerDll->GetUserMessageInfo(i,temp,39,dontdoit);
		if(strcmp(Msg,temp) == 0)
			return i;
	}
	return -1;
}
void HLStatsIngame::SetupModSpesficInfo()	// Gets the menu message id, for the running mod
{
	GetModName();

	g_ModSettings.HintText = GetMsgNum("HintText");
	g_ModSettings.HudMsg = GetMsgNum("HudMsg");
	g_ModSettings.MenuMsg = GetMsgNum("ShowMenu");
	g_ModSettings.TextMsg = GetMsgNum("TextMsg");
	g_ModSettings.SayText = GetMsgNum("SayText");
	g_ModSettings.VGUIMenu = GetMsgNum("VGUIMenu");		
}
void HLStatsIngame::ServerCommand(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer)-2, fmt, ap);
	va_end(ap);
	strcat(buffer,"\n");
	m_Engine->ServerCommand(buffer);
}
void HLStatsIngame::SrvLog(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer)-2, fmt, ap);
	va_end(ap);
	strcat(buffer,"\n");
	m_Engine->LogPrint(buffer);
}
int HLStatsIngame::FindPlayer(int UserID)	// Finds the player index based on userid.
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		if(g_IsConnected[i] == true)
		{
			if(UserID == UserInfo[i].Userid )	// Name or steamid matches TargetInfo so we return the index of the player
			{
				return i;
			}
		}
	}
	return -1;
}
int HLStatsIngame::FindPlayer(char *Name)	// Finds the player index based on name.
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		if(g_IsConnected[i] == true)
		{
			if(strcmp(GetPlayerName(i),Name) == 0)	// Name or steamid matches TargetInfo so we return the index of the player
			{
				return i;
			}
		}
	}
	return -1;
}
void HLStatsIngame::MessagePlayer(int index, const char *msg, ...)
{
	RecipientFilter rf;
	if (index > g_MaxClients || index < 0)
		return;

	if (index == 0)
	{
		rf.AddAllPlayers(g_MaxClients);
	} else {
		rf.AddPlayer(index);
	}
	char vafmt[240];
	va_list ap;
	va_start(ap, msg);
	int len = _vsnprintf(vafmt, 239, msg, ap);
	va_end(ap);
	len += _snprintf(&(vafmt[len]),239-len," \n");

	bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&rf), g_ModSettings.SayText);
	buffer->WriteByte(0);
	buffer->WriteString(vafmt);
	buffer->WriteByte(0);		// 0 = seems to mean it dont parse the text for colors , it does not parse for text
	m_Engine->MessageEnd();
}
const char *HLStatsIngame::GetPlayerName(int id)
{
	return m_Engine->GetClientConVarValue(id,"name");
}
void HLStatsIngame::SendHintMsg(int id,char *text)
{
	RecipientFilter mrf;
	char FormatedText[192];
	if(id == 0)
		mrf.AddAllPlayers(g_MaxClients);
	else
		mrf.AddPlayer(id);

	_snprintf(FormatedText,191,"%s",text);
	int len = strlen(FormatedText);
	if(len > 30) // Since the client dont automatically add \n when needed, we have to do it.
	{
		int LastAdded=0;
		for(int i=0;i<len;i++)
		{

			int t = GetNextSpaceCount(text,i);
			char b = FormatedText[i];

			if(FormatedText[i] == ' ' && LastAdded > 30 && (len-i) > 10 || (GetNextSpaceCount(text,i+1) + LastAdded)  > 34)
			{
				FormatedText[i] = '\n';
				LastAdded = 0;
			}
			else
				LastAdded++;
		}
	}

	bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&mrf), g_ModSettings.HintText);
	buffer->WriteByte(-1);
	buffer->WriteString(FormatedText);
	m_Engine->MessageEnd();	
}
void HLStatsIngame::SendHintMsgNoFormat(int id,char *text)
{
	RecipientFilter mrf;
	if(id == 0)
		mrf.AddAllPlayers(g_MaxClients);
	else
		mrf.AddPlayer(id);

	bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&mrf), g_ModSettings.HintText);
	buffer->WriteByte(-1);
	buffer->WriteString(text);
	m_Engine->MessageEnd();	
}
int HLStatsIngame::GetNextSpaceCount(char *Text,int CurIndex)
{
	int Count=0;
	int len = strlen(Text);
	for(int i=CurIndex;i<len;i++)
	{
		if(Text[i] == ' ')
			return Count;
		else
			Count++;
	}

	return Count;
}

int HLStatsIngame::GetWeaponIndex(const char *WeaponName)
{
	for(int i=1;i<MAX_WEAPONS;i++)	
	{
		if(i >= g_WeaponListCount) // This is a new weapon we add it
		{
			_snprintf(g_sWeaponList[i],MAX_WEAPONNAMELEN,"%s",WeaponName);
			g_WeaponListCount++;
			return i;
		}
		if(strcmp(WeaponName,g_sWeaponList[i]) == 0)
			return i;
	}
	return -1;
}
char* HLStatsIngame::GetWeaponName(int WeaponIndex)
{
	if(WeaponIndex >= g_WeaponListCount)
		return "Error";
	return g_sWeaponList[WeaponIndex];
}
/*
const char* Stats::GetWeaponName(int WeaponIndex)
{
	switch(WeaponIndex)
	{
	case 1:
		return "knife";
	case 2:
		return "usp";
	case 3:
		return "glock";
	case 4:
		return "deagle";
	case 5:
		return "p228";
	case 6:
		return "m3";
	case 7:
		return "xm1014";
	case 8:
		return "mp5navy";
	case 9:
		return "tmp";
	case 10:
		return "p90";
	case 11:
		return "m4a1";
	case 12:
		return "ak47";
	case 13:
		return "sg552";
	case 14:
		return "scout";
	case 15:
		return "awp";
	case 16:
		return "g3sg1";
	case 17:
		return "m249";
	case 18:
		return "hegrenade";
	case 19:
		return "flashbang";
	case 20:
		return "elite";
	case 21:
		return "aug";
	case 22:
		return "mac10";
	case 23:
		return "fiveseven";
	case 24:
		return "ump45";
	case 25:
		return "sg550";
	case 26:
		return "famas";
	case 27:
		return "galil";
	case 28:
		return "smoke_grenade";
	}
	return "ERROR";
}

Hardcoded version
int HLStatsIngame::GetWeaponIndex(const char *WeaponName)
{
	if(g_ModIndex == MOD_CSTRIKE)
	{
		if(strcmp(WeaponName,"knife") == 0)
			return 1;
		else if(strcmp(WeaponName,"usp") == 0)
			return 2;
		else if(strcmp(WeaponName,"glock") == 0)
			return 3;
		else if(strcmp(WeaponName,"deagle") == 0)
			return 4;
		else if(strcmp(WeaponName,"p228") == 0)
			return 5;
		else if(strcmp(WeaponName,"m3") == 0)
			return 6;
		else if(strcmp(WeaponName,"xm1014") == 0)
			return 7;
		else if(strcmp(WeaponName,"mp5navy") == 0)
			return 8;
		else if(strcmp(WeaponName,"tmp") == 0)
			return 9;
		else if(strcmp(WeaponName,"p90") == 0)
			return 10;
		else if(strcmp(WeaponName,"m4a1") == 0)
			return 11;
		else if(strcmp(WeaponName,"ak47") == 0)
			return 12;
		else if(strcmp(WeaponName,"sg552") == 0)
			return 13;
		else if(strcmp(WeaponName,"scout") == 0)
			return 14;
		else if(strcmp(WeaponName,"awp") == 0)
			return 15;
		else if(strcmp(WeaponName,"g3sg1") == 0)
			return 16;
		else if(strcmp(WeaponName,"m249") == 0)
			return 17;
		else if(strcmp(WeaponName,"hegrenade") == 0)
			return 18;
		else if(strcmp(WeaponName,"flashbang") == 0)
			return 19;
		else if(strcmp(WeaponName,"elite") == 0)
			return 20;
		else if(strcmp(WeaponName,"aug") == 0)
			return 21;
		else if(strcmp(WeaponName,"mac10") == 0)
			return 22;
		else if(strcmp(WeaponName,"fiveseven") == 0)
			return 23;
		else if(strcmp(WeaponName,"ump45") == 0)
			return 24;
		else if(strcmp(WeaponName,"sg550") == 0)
			return 25;
		else if(strcmp(WeaponName,"famas") == 0)
			return 26;
		else if(strcmp(WeaponName,"galil") == 0)
			return 27;
		else if(strcmp(WeaponName,"smoke_grenade") == 0)
			return 28;
	}
	return -1;
}
*/
/***************************************** Bellow here comes the String util functions ***************************************************/
char* StrUtil::StrRemoveQuotes(char *text)
{
	int len = strlen(text);

	for(int i=0;i<len;i++)
	{
		if(text[i] == '\"')
			text[i] = NULL;
	}
	return text;
}
int StrUtil::GetFirstIndexOfChar(char *Text,int MaxLen,unsigned char t)
{
	int TextIndex = 0;
	bool WSS = false; // WhiteSpaceSearch
	if(StrIsSpace(t))
		WSS = true;

	while(TextIndex <= MaxLen)
	{
		if(WSS)		// If the caller is searching for a whitespace, we make sure to search for the other whitespaces to.
		{
			if(StrIsSpace(Text[TextIndex]))
				return TextIndex;
			else
				TextIndex++;
		}
		else if(Text[TextIndex] == t)
		{
			return TextIndex;
		}
		else
			TextIndex++;
	}
	return -1;
}
int StrUtil::StrReplace(char *str, const char *from, const char *to, int maxlen) // Function from sslice 
{
	char  *pstr   = str;
	int   fromlen = Q_strlen(from);
	int   tolen   = Q_strlen(to);
	int	  RC=0;		// Removed count

	while (*pstr != '\0' && pstr - str < maxlen) {
		if (Q_strncmp(pstr, from, fromlen) != 0) {
			*pstr++;
			continue;
		}
		Q_memmove(pstr + tolen, pstr + fromlen, maxlen - ((pstr + tolen) - str) - 1);
		Q_memcpy(pstr, to, tolen);
		pstr += tolen;
		RC++;
	}
	return RC;
}
void StrUtil::StrTrim(char *buffer)
{
	StrTrimLeft(buffer);
	StrTrimRight(buffer);	
}
void StrUtil::StrTrimRight(char *buffer)
{
	// Make sure buffer isn't null
	if (buffer)
	{
		// Loop through buffer backwards while replacing whitespace chars with null chars
		for (int i = (int)strlen(buffer) - 1; i >= 0; i--)
		{
			if (StrIsSpace(buffer[i]))
				buffer[i] = '\0';
			else
				break;
		}
	}
}
bool StrUtil::StrIsSpace(unsigned char b)
{
	switch (b)
	{
	case ' ': 
		return true;
	case '\n':
		return true;
	case '\r':
		return true;
	case '\0': 
		return true;
	}
	return false;
}
/*
More info can be found here: Hashes
http://en.wikipedia.org/wiki/Duff's_device
*/
unsigned long StrUtil::StrHash(register char *str, register int len) 
{
	register unsigned long n = 0;

#define HASHC	n = *str++ + 65587 * n  // The other number is better somehow, magic i guess ( Old num: 65599 )

	if (len > 0) {
		register int loop = (len + 8 - 1) >> 3;

		switch(len & (8 - 1)) {
		case 0:	do {
			HASHC;	case 7:	HASHC;
		case 6:	HASHC;	case 5:	HASHC;
		case 4:	HASHC;	case 3:	HASHC;
		case 2:	HASHC;	case 1:	HASHC;
				} while (--loop);
		}
	}
	return n;
}
void StrUtil::StrTrimLeft(char *buffer)
{
	// Let's think of this as our iterator
	char *i = buffer;

	// Make sure the buffer isn't null
	if (i && *i)
	{
		// Add up number of whitespace characters
		while(StrIsSpace(*i))
			i++;

		// If whitespace chars in buffer then adjust string so first non-whitespace char is at start of buffer
		// :TODO: change this to not use memcpy()!
		if (i != buffer)
			memcpy(buffer, i, (strlen(i) + 1) * sizeof(char));
	}
}