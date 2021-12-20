// g_savestate.c
//
// JPL (jpl@ing.puc.cl) - The Dark Conjunction - ACE Team
//
// Saving State Code, only saves ammo, difficulty and basic player info

#include "g_local.h"
#include "q_shared.h"


char stbuf[1000];
char filename[100];
char first_level[]="aircrash";
char autosave[]="autosave";
char ver_stamp[100];

int num_params_version[]={
	9,			// gun (1) & ammo(2), trishotgun (1) & ammo (2), health (2), current weapon (1)
	14			// difficulty(1), bone (1), mg42 (1) & ammo (2)
};	// agreg

#define LAST_VERSION 1

void CalculateStamp(int len)	// kind of encrypting to avoid cheating
{
	int i;

	ver_stamp[0]=0;
	for (i=0;i<len;i++)
		ver_stamp[len-i-1]=(stbuf[i]*i+ver_stamp[(len-i)%len]*(len-i))%26+'A';
	ver_stamp[i]=0;
}

void LoadPlayerState()
{
	int NUM_PARAMS=0;
	int version;
	fileHandle_t f;
	int len;
	gclient_t *client=g_entities[0].client;
	gentity_t *ent=&g_entities[0];
	char load_game[100];
	char stmap[100];
	int health;

	trap_Cvar_VariableStringBuffer( "load_game", load_game, sizeof(load_game) );
	if (load_game[0])
		Com_sprintf(filename,100,"%s.sav",load_game);
	else
		Com_sprintf(filename,100,"%s.sav",autosave);

	trap_Cvar_VariableStringBuffer( "mapname", stmap, sizeof(stmap) );
	if (!strcmp(stmap,first_level) && !load_game[0]) return;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 )
	{
state_corrupt:
		G_Printf("Valid (without cheats) saved state not found.\n");
		ent->health=client->ps.stats[STAT_HEALTH]=10;
		trap_Cvar_Set("load_game","");
		return;
	}
	if ( len >= sizeof( stbuf ) - 1 )
	{
		G_Printf( "File %s too long\n", filename );
		trap_Cvar_Set("load_game","");
		return;
	}

	trap_FS_Read( stbuf, len, f );
	stbuf[len] = 0;
	trap_FS_FCloseFile( f );

//	G_Printf("Load state: %s\n",stbuf);

//	G_Printf("map name: %s\n",stmap);

	version=0;
	if (stbuf[0]=='?')
	{
		int i;
		version=stbuf[1]-'0';
		for (i=2;i<len;i++)
			stbuf[i-2]=stbuf[i];
		len-=2;
	}

//	G_Printf("version %d\n",version);

	NUM_PARAMS=num_params_version[version];

	len/=2;

	if (Q_strncmp(stmap,stbuf,len-NUM_PARAMS) || Q_strncmp(stmap,stbuf,strlen(stmap)))
	{
		char buf[100];

		stbuf[len-NUM_PARAMS]=0;
		Com_sprintf(buf,sizeof(buf),"map %s\n",stbuf);
		trap_SendConsoleCommand( EXEC_APPEND, buf );
		return;
	}

	CalculateStamp(len);

//	G_Printf("stamp: %s\n",ver_stamp);

	if (Q_strncmp(ver_stamp,stbuf+len,len))
		goto state_corrupt;

	trap_Cvar_Set("load_game","");
	len-=NUM_PARAMS;		// here goes the number of state parameters

	health=(stbuf[len++]-'A')*26;
	health+=(stbuf[len++]-'A');
	if ( health<0) health=1;
	ent->health=client->ps.stats[STAT_HEALTH]=health;

	client->ps.stats[STAT_WEAPONS]=0;
	client->ps.stats[STAT_WEAPONS] |= (stbuf[len++]=='G')?( 1 << WP_GUN):0;
	client->ps.stats[STAT_WEAPONS] |= (stbuf[len++]=='T')?( 1 << WP_TRISHOTGUN):0;
	client->ps.ammo[WP_GUN]=(stbuf[len++]-'A')*26;
	client->ps.ammo[WP_GUN]+=(stbuf[len++]-'A');
	client->ps.ammo[WP_TRISHOTGUN]=(stbuf[len++]-'A')*26;
	client->ps.ammo[WP_TRISHOTGUN]+=(stbuf[len++]-'A');
	client->ps.weapon=(stbuf[len++]-'A');

	if (version>=1)
	{
		client->ps.stats[STAT_WEAPONS] |= (stbuf[len++]=='B')?( 1 << WP_BONE):0;
		client->ps.stats[STAT_WEAPONS] |= (stbuf[len++]=='M')?( 1 << WP_MG42):0;
		client->ps.ammo[WP_MG42]=(stbuf[len++]-'A')*26;
		client->ps.ammo[WP_MG42]+=(stbuf[len++]-'A');
		npc_skill=((stbuf[len++]-'A')-1);	// convert 0..2 to -1..1
	}

	if (!client->ps.ammo[client->ps.weapon])	// no ammo = weapon down
		client->ps.weapon=WP_NONE;

	SavePlayerState(stmap);		// generate autosave file for restarting
}

void SavePlayerState(char *nextmap)
{
	fileHandle_t f;
	int len;
	gclient_t *client=g_entities[0].client;
	char save_game[100];
	char stmap[100];

	trap_Cvar_VariableStringBuffer( "save_game", save_game, sizeof(save_game) );
	if (save_game[0])
	{
		trap_Cvar_VariableStringBuffer( "mapname", stmap, sizeof(stmap) );
		if (!strcmp(stmap,first_level)) return;	// no save in first level

		Com_sprintf(filename,100,"%s.sav",autosave);
		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len <= 0  ||  len >= sizeof( stbuf ) - 1 )
		{
			G_Printf("Could not save.\n");
			trap_Cvar_Set("save_game","");
			return;
		}

		trap_FS_Read( stbuf, len, f );
		stbuf[len] = 0;
		trap_FS_FCloseFile( f );

		Com_sprintf(filename,100,"%s.sav",save_game);
	}
	else
	{
		Com_sprintf(filename,100,"%s.sav",autosave);
		Com_sprintf(stbuf,sizeof(stbuf),"%s",nextmap);
		
		len=strlen(stbuf);

	// version 0

		stbuf[len++]=(client->ps.stats[STAT_HEALTH])/26+'A';
		stbuf[len++]=(client->ps.stats[STAT_HEALTH])%26+'A';
		stbuf[len++]=(client->ps.stats[STAT_WEAPONS] & ( 1 << WP_GUN))?'G':' ';
		stbuf[len++]=(client->ps.stats[STAT_WEAPONS] & ( 1 << WP_TRISHOTGUN))?'T':' ';
		stbuf[len++]=(client->ps.ammo[WP_GUN])/26+'A';
		stbuf[len++]=(client->ps.ammo[WP_GUN])%26+'A';
		stbuf[len++]=(client->ps.ammo[WP_TRISHOTGUN])/26+'A';
		stbuf[len++]=(client->ps.ammo[WP_TRISHOTGUN])%26+'A';
		stbuf[len++]=(client->ps.weapon)%26+'A';

	// version 1

		stbuf[len++]=(client->ps.stats[STAT_WEAPONS] & ( 1 << WP_BONE))?'B':' ';
		stbuf[len++]=(client->ps.stats[STAT_WEAPONS] & ( 1 << WP_MG42))?'M':' ';
		stbuf[len++]=(client->ps.ammo[WP_MG42])/26+'A';
		stbuf[len++]=(client->ps.ammo[WP_MG42])%26+'A';
		stbuf[len++]=(npc_skill+'A')+1;	// from -1..1 to 0..2

		stbuf[len]=0;

		CalculateStamp(len);

		Com_sprintf(stbuf,sizeof(stbuf),"?%c%s%s",LAST_VERSION+'0',stbuf,ver_stamp);
	}
	

	trap_FS_FOpenFile( filename, &f, FS_WRITE );
	trap_FS_Write( stbuf, strlen(stbuf), f );
	trap_FS_FCloseFile( f );

	trap_Cvar_Set("save_game","");
	if (save_game[0])
		G_Printf("Saved game: %s\n",filename);
}
