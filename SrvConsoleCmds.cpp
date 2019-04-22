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
#include "SrvConsoleCmds.h"
#include "hl2sdk\recipientfilters.h"
#include <string.h>
#include <bitbuf.h>
#include <ctype.h>

extern HLStatsIngame g_MSCore;

ConCommand HLSSrvCmd1( "ma_csay", SrvConsoleCmds::CmdCenterSay, "BAT Registered Server cmd" );
ConCommand HLSSrvCmd2( "ma_psay", SrvConsoleCmds::CmdPSay, "BAT Registered Server cmd" );
ConCommand HLSSrvCmd3( "ma_msay", SrvConsoleCmds::CmdMenuSay, "BAT Registered Server cmd" );
ConCommand HLSSrvCmd4( "ma_cexec", SrvConsoleCmds::CmdShowMOTD, "BAT Registered Server cmd" );

ConCommand HLSSrvCmd5( "ms_psay", SrvConsoleCmds::CmdPSayViaChat, "ms_psay <name> - Allows for private chat to a player via chat text" );
ConCommand HLSSrvCmd6( "ms_hint", SrvConsoleCmds::CmdPSay, "ms_hint <name> - Allows for private chat to a player via hint message" );
ConCommand HLSSrvCmd7( "ms_csay", SrvConsoleCmds::CmdCenterSay, "ms_csay <name> - Sends a message to everyone center of life");
ConCommand HLSSrvCmd8( "ms_browse", SrvConsoleCmds::CmdShowMOTD, "ms_browse <userid> - Shows a MOTD box" );
ConCommand HLSSrvCmd9( "ms_msay", SrvConsoleCmds::CmdMenuSay, "ms_msay <userid> - Menu say");
ConCommand HLSSrvCmd10( "ms_swap", SrvConsoleCmds::CmdSwapUserTeam, "ms_swap <userid> - Swaps the users team");

// msay, csay, psay, swap, browse, hint
#define MaxMenuSize 1024 // The maximum size of the menu arrays
#define MaxPrSend 240	// This is the maximum we can send at once, the total max is 255, but the source engines adds a few bytes after us. So we need to be safe

char g_OrgSayText[MaxMenuSize+1];
extern int g_MaxClients;
extern bool g_IsConnected[MAXPLAYERS+1];
extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern ModSettingsStruct g_ModSettings;

void SrvConsoleCmds::CmdShowMOTD()
{
	int g_ArgCount = g_MSCore.GetEngine()->Cmd_Argc();
	int len=0;
	int id = g_MSCore.FindPlayer(atoi(g_MSCore.GetEngine()->Cmd_Argv(1)));
	if(id == -1)
		return;

	// g_OrgSayText = 0x0ed95030 "@ma_browse http://www.esports.no/stats/ingame.php?game=css&mode=statsme&player=1"
	_snprintf(g_OrgSayText,191,"%s",g_MSCore.GetEngine()->Cmd_Argv(3));
	if(strlen(g_OrgSayText) == 0)
		return;
//g_OrgSayText = 0x0ed96030 "http://81.166.41.6/web/ingame.php?game=css&mode=statsme&player=1"
	g_MSCore.StrRemoveQuotes(g_OrgSayText);
	g_MSCore.StrReplace(g_OrgSayText,"@ma_browse ","",191);

	ShowMOTD(id,g_OrgSayText);
}
void SrvConsoleCmds::CmdCenterSay()
{
	_snprintf(g_OrgSayText,191,"%s",g_MSCore.GetEngine()->Cmd_Args());

	g_MSCore.SendCSay(0,g_OrgSayText);
}
void SrvConsoleCmds::CmdPSay()
{
	int g_ArgCount = g_MSCore.GetEngine()->Cmd_Argc();
	int len=0;
	int id = g_MSCore.FindPlayer(g_MSCore.GetEngine()->Cmd_Argv(2));

	if(id == -1)
		id = 0;

	if(g_ArgCount >= 3)
	{
		len += _snprintf(&(g_OrgSayText[len]),191-len,"%s",g_MSCore.GetEngine()->Cmd_Argv(2));

		for(int i=3;i<g_ArgCount;i++)
		{
			len += _snprintf(&(g_OrgSayText[len]),191-len," %s",g_MSCore.GetEngine()->Cmd_Argv(i));
		}
	}
	else
		return;

	g_MSCore.SendHintMsg(id,g_OrgSayText);
}
void SrvConsoleCmds::CmdPSayViaChat()
{
	int g_ArgCount = g_MSCore.GetEngine()->Cmd_Argc();
	int len=0;
	int id = g_MSCore.FindPlayer(g_MSCore.GetEngine()->Cmd_Argv(2));

	if(id == -1)
		id = 0;

	if(g_ArgCount >= 3)
	{
		len += _snprintf(&(g_OrgSayText[len]),191-len,"%s",g_MSCore.GetEngine()->Cmd_Argv(2));

		for(int i=3;i<g_ArgCount;i++)
		{
			len += _snprintf(&(g_OrgSayText[len]),191-len," %s",g_MSCore.GetEngine()->Cmd_Argv(i));
		}
	}
	else
		return;

	g_MSCore.MessagePlayer(id,g_OrgSayText);
}
void SrvConsoleCmds::CmdMenuSay()
{
	unsigned int g_ArgCount = g_MSCore.GetEngine()->Cmd_Argc();
	int len=0;
	int id = g_MSCore.FindPlayer(atoi(g_MSCore.GetEngine()->Cmd_Argv(2)));

	if(g_ArgCount >= 3)
	{
		len += _snprintf(&(g_OrgSayText[len]),MaxMenuSize-len,"%s",g_MSCore.GetEngine()->Cmd_Argv(3));
		
		for(unsigned int i=4;i<g_ArgCount;i++)
		{			
			len += _snprintf(&(g_OrgSayText[len]),MaxMenuSize-len," %s",g_MSCore.GetEngine()->Cmd_Argv(i));
		}
	}
	else
		_snprintf(g_OrgSayText,MaxMenuSize,"%s",g_MSCore.GetEngine()->Cmd_Args());

	if(id == -1)
	{
		g_MSCore.ServerCommand("echo CmdMenuSay - Error processing command: '%s'",g_OrgSayText);
		return;
	}
	bool LastWasSign=false;
	int FoundCount = 0;

	for(int i=0;i<len;i++)
	{
		if(g_OrgSayText[i] == 92) // this is "\"
			LastWasSign = true; // 110 is n
		else if(LastWasSign && g_OrgSayText[i] == 110)
		{
			FoundCount++;
			LastWasSign=false;

			g_OrgSayText[i-1]  = '#';
			g_OrgSayText[i]  = '#';
		}
	}
	int test = g_MSCore.StrReplace(g_OrgSayText,"##","#",MaxMenuSize);
	for(int i=0;i<len;i++)
	{
		if(g_OrgSayText[i] == '#')
			g_OrgSayText[i] = '\n';
	}
	SendMenuString(id,g_OrgSayText);
}
void SrvConsoleCmds::CmdSwapUserTeam()
{
	int g_ArgCount = g_MSCore.GetEngine()->Cmd_Argc();
	int len=0;
	int id = g_MSCore.FindPlayer(atoi(g_MSCore.GetEngine()->Cmd_Argv(1)));
	if(id == -1)
		return;

	IPlayerInfo *PlayerInfo = g_MSCore.GetPlayerInfo()->GetPlayerInfo(UserInfo[id].PlayerEdict);

	if (PlayerInfo && UserInfo[id].PlayerEdict && !UserInfo[id].PlayerEdict->IsFree()) 
	{ 
		int Team = PlayerInfo->GetTeamIndex();
		if(Team == 2)
			Team = 3;
		else if(Team == 3)
			Team = 2;
		else
			return;

		PlayerInfo->ChangeTeam( Team ); 
	}	
}
void SrvConsoleCmds::ShowMOTD(int target_id,const char *url)
{
	RecipientFilter rf; // the corresponding recipient filter 

	rf.AddPlayer (target_id); 
	rf.MakeReliable();

	bf_write *netmsg = g_MSCore.GetEngine()->UserMessageBegin (static_cast<IRecipientFilter *>(&rf), g_ModSettings.VGUIMenu); 
	netmsg->WriteString("info"); // the HUD message itself 
	netmsg->WriteByte(1);
	netmsg->WriteByte(3); // I don't know yet the purpose of this byte, it can be 1 or 0 
	netmsg->WriteString("type"); // the HUD message itself 
	netmsg->WriteString("2"); // the HUD message itself 
	netmsg->WriteString("title"); // the HUD message itself 
	netmsg->WriteString("Server Rules"); // the HUD message itself 

	netmsg->WriteString("msg"); // the HUD message itself 
	netmsg->WriteString(url); // the HUD message itself 

	g_MSCore.GetEngine()->MessageEnd();
}
#define MENU_OPEN_TIME 30
void SrvConsoleCmds::SendMenuString(int id,char *MenuString)
{
	RecipientFilter rf;

	if (id == 0) 
	{
		for(int i=1;i<=g_MaxClients;i++) if(g_IsConnected[i] && !UserInfo[i].IsBot)
		{
			rf.AddPlayer(i);
		}
	} else {
		rf.AddPlayer(id);
	}
	char *ptr = MenuString;
	size_t len = strlen(MenuString);
	char save = 0;

	while (true)
	{
		if (len > MaxPrSend)
		{
			save = ptr[MaxPrSend];
			ptr[MaxPrSend] = '\0';
		}
		bf_write *buffer = g_MSCore.GetEngine()->UserMessageBegin(static_cast<IRecipientFilter *>(&rf), g_ModSettings.MenuMsg);
		buffer->WriteWord((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7)| (1<<8) | (1<<9));
		buffer->WriteChar(MENU_OPEN_TIME);
		buffer->WriteByte( (len > MaxPrSend) ? 1 : 0 );
		buffer->WriteString(ptr);
		g_MSCore.GetEngine()->MessageEnd();
		if (len > MaxPrSend)
		{
			ptr[MaxPrSend] = save;
			ptr = &(ptr[MaxPrSend]);
			len -= MaxPrSend;
		} else {
			break;
		}
	}
}