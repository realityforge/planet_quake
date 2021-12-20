// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

INGAME MENU

=======================================================================
*/


#include "ui_local.h"


#define INGAME_FRAME					"menu/art/addbotframe"

#define INGAME_MENU_VERTICAL_SPACING	28

#define ID_SAVE					10
#define ID_LOAD					11

#define ID_SETUP				13

#define ID_LEAVEARENA			15

#define ID_RESUME				18



typedef struct {
	menuframework_s	menu;

	menubitmap_s	frame;

	menutext_s		resume;
	menutext_s		setup;
	menutext_s		save;
	menutext_s		load;
	menutext_s		leave;

} ingamemenu_t;

static ingamemenu_t	s_ingame;

/*
=================
InGame_RestartAction
=================
*/
/*static void InGame_RestartAction( qboolean result ) {
	if( !result ) {
		return;
	}

	UI_PopMenu();
	trap_Cmd_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
}

*/
/*
=================
InGame_QuitAction
=================
*/
/*static void InGame_QuitAction( qboolean result ) {
	if( !result ) {
		return;
	}
	UI_PopMenu();
	UI_CreditMenu();
}
*/

/*
=================
InGame_Event
=================
*/
void InGame_Event( void *ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
//	case ID_TEAM:
	//	UI_TeamMainMenu();
	//	break;

	case ID_SETUP:
		UI_SetupMenu();
		break;

	case ID_LEAVEARENA:
		trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
		break;

	case ID_LOAD:
		UI_SaveLoadMenu(0);
		break;

	case ID_SAVE:
		UI_SaveLoadMenu(1);
		break;

//	case ID_RESTART:
//		UI_ConfirmMenu( "RESTART LEVEL?", NULL, InGame_RestartAction );
//		break;

//	case ID_QUIT:
//		UI_ConfirmMenu( "LEAVE GAME?", NULL, InGame_QuitAction );
//		break;

//	case ID_SERVERINFO:
//		UI_ServerInfoMenu();
//		break;

//	case ID_ADDBOTS:
//		UI_AddBotsMenu();
//		break;

//	case ID_REMOVEBOTS:
//		UI_RemoveBotsMenu();
//		break;

//	case ID_TEAMORDERS:
//		UI_TeamOrdersMenu();
//		break;

	case ID_RESUME:
		UI_PopMenu();
		break;
	}
}


/*
=================
InGame_MenuInit
=================
*/
void InGame_MenuInit( void ) {
	int		y;
	char stmap[100];
//	uiClientState_t	cs;
//	char	info[MAX_INFO_STRING];
//	int		team;

	memset( &s_ingame, 0 ,sizeof(ingamemenu_t) );

	InGame_Cache();

	s_ingame.menu.wrapAround = qtrue;
	s_ingame.menu.fullscreen = qfalse;

	s_ingame.frame.generic.type			= MTYPE_BITMAP;
	s_ingame.frame.generic.flags		= QMF_INACTIVE;
	s_ingame.frame.generic.name			= INGAME_FRAME;
	s_ingame.frame.generic.x			= 320-233;//142;
	s_ingame.frame.generic.y			= 240-166;//118;
	s_ingame.frame.width				= 466;//359;
	s_ingame.frame.height				= 332;//256;

	//y = 96;
	y = 150;
//	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.resume.generic.type			= MTYPE_PTEXT;
	s_ingame.resume.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.resume.generic.x				= 320;
	s_ingame.resume.generic.y				= y;
	s_ingame.resume.generic.id				= ID_RESUME;
	s_ingame.resume.generic.callback		= InGame_Event; 
	s_ingame.resume.string					= "RESUME GAME";
	s_ingame.resume.color					= tdc_text_color;
	s_ingame.resume.style					= UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.setup.generic.type			= MTYPE_PTEXT;
	s_ingame.setup.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.setup.generic.x			= 320;
	s_ingame.setup.generic.y			= y;
	s_ingame.setup.generic.id			= ID_SETUP;
	s_ingame.setup.generic.callback		= InGame_Event; 
	s_ingame.setup.string				= "SETUP";
	s_ingame.setup.color				= tdc_text_color;
	s_ingame.setup.style				= UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.save.generic.type			= MTYPE_PTEXT;
	s_ingame.save.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.save.generic.x				= 320;
	s_ingame.save.generic.y				= y;
	s_ingame.save.generic.id			= ID_SAVE;
	s_ingame.save.generic.callback		= InGame_Event; 
	s_ingame.save.string				= "SAVE";
	s_ingame.save.color					= tdc_text_color;
	s_ingame.save.style					= UI_CENTER|UI_SMALLFONT;	y += INGAME_MENU_VERTICAL_SPACING;

	trap_Cvar_VariableStringBuffer( "mapname", stmap, sizeof(stmap) );
	if (!strcmp(stmap,"aircrash")) // no save in first level
		s_ingame.save.generic.flags			= QMF_CENTER_JUSTIFY|QMF_INACTIVE|QMF_GRAYED;

	
	s_ingame.load.generic.type			= MTYPE_PTEXT;
	s_ingame.load.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.load.generic.x				= 320;
	s_ingame.load.generic.y				= y;
	s_ingame.load.generic.id			= ID_LOAD;
	s_ingame.load.generic.callback		= InGame_Event; 
	s_ingame.load.string				= "LOAD";
	s_ingame.load.color					= tdc_text_color;
	s_ingame.load.style					= UI_CENTER|UI_SMALLFONT;


	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.leave.generic.type			= MTYPE_PTEXT;
	s_ingame.leave.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.leave.generic.x			= 320;
	s_ingame.leave.generic.y			= y;
	s_ingame.leave.generic.id			= ID_LEAVEARENA;
	s_ingame.leave.generic.callback		= InGame_Event; 
	s_ingame.leave.string				= "LEAVE GAME";
	s_ingame.leave.color				= tdc_text_color;
	s_ingame.leave.style				= UI_CENTER|UI_SMALLFONT;

	
	Menu_AddItem( &s_ingame.menu, &s_ingame.resume );
	Menu_AddItem( &s_ingame.menu, &s_ingame.setup );
	Menu_AddItem( &s_ingame.menu, &s_ingame.save );
	Menu_AddItem( &s_ingame.menu, &s_ingame.load );
	Menu_AddItem( &s_ingame.menu, &s_ingame.leave );

}


/*
=================
InGame_Cache
=================
*/
void InGame_Cache( void ) {
	trap_R_RegisterShaderNoMip( INGAME_FRAME );
}


/*
=================
UI_InGameMenu
=================
*/
void UI_InGameMenu( void ) {
	// force as top level menu
	uis.menusp = 0;  

	// set menu cursor to a nice location
	uis.cursorx = 319;
	uis.cursory = 80;

	InGame_MenuInit();
	UI_PushMenu( &s_ingame.menu );
}
