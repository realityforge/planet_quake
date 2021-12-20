// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

MAIN MENU
// Ace Team modified intro screen

=======================================================================
*/


#include "ui_local.h"


#define ID_SINGLEPLAYER			10
#define ID_SETUP				12
#define ID_LOAD					13
#define ID_EXIT					17
#define MAIN_MENU_VERTICAL_SPACING		34


#define BACKGROUND	"menu/art/title"
#define SMOKE		"menu/art/smoke.jpg"
#define BEGIN_0		"menu/art/b_begin1"
#define BEGIN_1		"menu/art/b_begin2"
#define SETUP_0		"menu/art/b_setup1"
#define SETUP_1		"menu/art/b_setup2"
#define LOAD_0		"menu/art/b_load_save1"
#define LOAD_1		"menu/art/b_load_save2"
#define EXIT_0		"menu/art/b_exit1"
#define EXIT_1		"menu/art/b_exit2"

typedef struct {
	menuframework_s	menu;

	menubitmap_s    begin;
	menubitmap_s	setup;
	menubitmap_s    exit;
	menubitmap_s	load;
	menubitmap_s	smoke;
    menubitmap_s	background;

	menutext_s		version;
	menutext_s		credits;
	menutext_s		credits2;
  
} mainmenu_t;


static mainmenu_t s_main;

/*
=================
MainMenu_ExitAction
=================
*/
static void MainMenu_ExitAction( qboolean result ) {
	if( !result ) {
		return;
	}
	UI_PopMenu();
	UI_CreditMenu(0);
}


/*
=================
Main_MenuEvent
=================
*/
void Main_MenuEvent (void* ptr, int event) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_SINGLEPLAYER:
		{
		
		m_specialsound = qtrue;
		UI_SPLevelMenu();
		
		}
		break;

	case ID_SETUP:
		m_specialsound = qtrue;
		UI_SetupMenu();
		break;

	case ID_EXIT:
		m_specialsound = qtrue;
		UI_ConfirmMenu( "EXIT TDC?", NULL , MainMenu_ExitAction );
		break;
	case ID_LOAD:
		m_specialsound = qtrue;
		UI_SaveLoadMenu(0);
		break;


	}
}


/*
===============
MainMenu_Cache
===============
*/
void MainMenu_Cache( void ) {

	trap_R_RegisterShaderNoMip( BEGIN_0 );
	trap_R_RegisterShaderNoMip( BEGIN_1 );
	trap_R_RegisterShaderNoMip( EXIT_0 );
	trap_R_RegisterShaderNoMip( EXIT_1 );
	trap_R_RegisterShaderNoMip( SETUP_0 );
	trap_R_RegisterShaderNoMip( SETUP_1 );
	trap_R_RegisterShaderNoMip( LOAD_0 );
	trap_R_RegisterShaderNoMip( LOAD_1 );
	trap_R_RegisterShaderNoMip( BACKGROUND );
	trap_R_RegisterShaderNoMip( SMOKE );
}

/*
===============
Main_MenuDraw
===============
*/
static void Main_MenuDraw( void ) {
	
//	UI_DrawNamedPic(80, 0, 480 ,480, BACKGROUND);
//	Menu_Draw(&s_main.menu);

}


/*
===============
UI_MainMenu

The main menu only comes up when not in a game,
so make sure that the attract loop server is down
and that local cinematics are killed
===============
*/
void UI_MainMenu( void ) {
	qboolean teamArena = qfalse;
	int		style = UI_CENTER | UI_DROPSHADOW;

	trap_Cvar_Set( "sv_killserver", "1" );
	
	trap_Cvar_Set( "sv_pure", "0" );	// JPL - for allowing save/load

	if( !uis.demoversion && !ui_cdkeychecked.integer ) {
		char	key[17];

		trap_GetCDKey( key, sizeof(key) );
		if( trap_VerifyCDKey( key, NULL ) == qfalse ) {
			UI_CDKeyMenu();
			return;
		}
	}

	memset( &s_main, 0 ,sizeof(mainmenu_t) );


	MainMenu_Cache();

	s_main.menu.fullscreen = qtrue;
	s_main.menu.wrapAround = qtrue;
	s_main.menu.showlogo = qfalse;/*qtrue;*/

	s_main.smoke.generic.type		= MTYPE_BITMAP;
	s_main.smoke.generic.name		= SMOKE;
	s_main.smoke.generic.flags		= QMF_INACTIVE ;
	s_main.smoke.generic.x			= 81;
	s_main.smoke.generic.y			= 1;
	s_main.smoke.width				= 478;
	s_main.smoke.height				= 478;
	
	s_main.background.generic.type		= MTYPE_BITMAP;
	s_main.background.generic.name		= BACKGROUND;
	s_main.background.generic.flags		= QMF_INACTIVE ;
	s_main.background.generic.x			= 80;
	s_main.background.generic.y			= 0;
	s_main.background.width				= 480;
	s_main.background.height			= 480;

	s_main.begin.generic.type		= MTYPE_BITMAP;
	s_main.begin.generic.name		= BEGIN_0;
	s_main.begin.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.begin.generic.x			= 228+80;
	s_main.begin.generic.y			= 65-48;
	s_main.begin.generic.id			= ID_SINGLEPLAYER;
	s_main.begin.generic.callback	= Main_MenuEvent; 
	s_main.begin.width				= 96;
	s_main.begin.height				= 96;
	s_main.begin.focuspic			= BEGIN_1;

	s_main.setup.generic.type				= MTYPE_BITMAP;
	s_main.setup.generic.name				= SETUP_0;
	s_main.setup.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;//QMF_PULSEIFFOCUS;
	s_main.setup.generic.x					= 427+80;
	s_main.setup.generic.y					= 218-48;
	s_main.setup.generic.id					= ID_SETUP;
	s_main.setup.generic.callback			= Main_MenuEvent; 
	s_main.setup.width						= 96;
	s_main.setup.height						= 96;
	s_main.setup.focuspic					= SETUP_1;

	s_main.exit.generic.type				= MTYPE_BITMAP;
	s_main.exit.generic.name				= EXIT_0;
	s_main.exit.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.exit.generic.x					= 241+80;
	s_main.exit.generic.y					= 420-48;
	s_main.exit.generic.id					= ID_EXIT;
	s_main.exit.generic.callback			= Main_MenuEvent; 
	s_main.exit.width						= 96;
	s_main.exit.height						= 96;
	s_main.exit.focuspic					= EXIT_1;

	s_main.load.generic.type				= MTYPE_BITMAP;
	s_main.load.generic.name				= LOAD_0;
	s_main.load.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.load.generic.x					= 50+80;
	s_main.load.generic.y					= 218-48;
	s_main.load.generic.id					= ID_LOAD;
	s_main.load.generic.callback			= Main_MenuEvent; 
	s_main.load.width						= 96;
	s_main.load.height						= 96;
	s_main.load.focuspic					= LOAD_1;

	s_main.credits.generic.type=MTYPE_TEXT;
	s_main.credits.generic.flags=UI_CENTER | UI_SMALLFONT;
	s_main.credits.generic.x=320;
	s_main.credits.generic.y=450;
	s_main.credits.string="The Dark Conjunction, version demo 1.0, ACE Team, 2001-2002";
	s_main.credits.style=UI_CENTER | UI_SMALLFONT;
	s_main.credits.color=tdc_text_color;

	s_main.credits2.generic.type=MTYPE_TEXT;
	s_main.credits2.generic.flags=UI_CENTER | UI_SMALLFONT;
	s_main.credits2.generic.x=320;
	s_main.credits2.generic.y=465;
	s_main.credits2.string="QUAKE III ARENA (c), id Software, Inc, All Rights Reserved";
	s_main.credits2.style=UI_CENTER | UI_SMALLFONT;
	s_main.credits2.color=tdc_text_color;

	Menu_AddItem( &s_main.menu,	&s_main.smoke );
	Menu_AddItem( &s_main.menu,	&s_main.background );
	Menu_AddItem( &s_main.menu,	&s_main.begin );
	Menu_AddItem( &s_main.menu,	&s_main.setup );
	Menu_AddItem( &s_main.menu,	&s_main.exit );             
	Menu_AddItem( &s_main.menu,	&s_main.load );
	Menu_AddItem( &s_main.menu,	&s_main.credits );
	Menu_AddItem( &s_main.menu,	&s_main.credits2 );
	
// add end or stop condition
	trap_S_RegisterSound( "music music/TDC_menu[1].wav", qfalse );
	trap_Cmd_ExecuteText( EXEC_APPEND, "music music/TDC_menu[1]\n" );
	
	trap_Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
	UI_PushMenu ( &s_main.menu );
}
