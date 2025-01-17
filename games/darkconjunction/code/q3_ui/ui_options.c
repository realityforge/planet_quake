/*
=======================================================================

SYSTEM CONFIGURATION MENU

=======================================================================
*/

#include "ui_local.h"
							
#define BACKGROUND			"menu/art/dark_scene"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_SOUND			12

#define ID_BACK				14

#define VERTICAL_SPACING	34

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menutext_s		graphics;
	menutext_s		display;
	menutext_s		sound;
	menutext_s		network;
	menutext_s		back;
	
	menubitmap_s background;

} optionsmenu_t;

static optionsmenu_t	s_options;



/*
=================
Options_Event
=================
*/
static void Options_Event( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		UI_SoundOptionsMenu();
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
SystemConfig_Cache
===============
*/
void SystemConfig_Cache( void ) {

	trap_R_RegisterShaderNoMip(BACKGROUND);


}

/*
===============
Options_MenuInit
===============
*/
void Options_MenuInit( void ) {
	int				y;
	uiClientState_t	cstate;

	memset( &s_options, 0, sizeof(optionsmenu_t) );

	SystemConfig_Cache();
	s_options.menu.wrapAround = qtrue;

	trap_GetClientState( &cstate );
	if ( cstate.connState >= CA_CONNECTED ) {
		s_options.menu.fullscreen = qfalse;
	}
	else {
		s_options.menu.fullscreen = qtrue;
	}

	
	s_options.banner.generic.type	= MTYPE_BTEXT;
	s_options.banner.generic.flags	= QMF_CENTER_JUSTIFY;
	s_options.banner.generic.x		= 320;
	s_options.banner.generic.y		= 16;
	s_options.banner.string		    = "SYSTEM SETUP";
	s_options.banner.color			= color_white;
	s_options.banner.style			= UI_SMALLFONT|UI_CENTER;

	s_options.background.generic.type  = MTYPE_BITMAP;
	s_options.background.generic.name  = BACKGROUND;
	s_options.background.generic.flags = QMF_INACTIVE;
	s_options.background.generic.x	   = 0;  
	s_options.background.generic.y	   = 0;
	s_options.background.width  	   = 640;
	s_options.background.height  	   = 480;

	y = 168;
	s_options.graphics.generic.type		= MTYPE_PTEXT;
	s_options.graphics.generic.flags	= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.graphics.generic.callback	= Options_Event;
	s_options.graphics.generic.id		= ID_GRAPHICS;
	s_options.graphics.generic.x		= 320;
	s_options.graphics.generic.y		= y;
	s_options.graphics.string			= "GRAPHICS";
	s_options.graphics.color			= tdc_text_color;
	s_options.graphics.style			= UI_SMALLFONT|UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.display.generic.type		= MTYPE_PTEXT;
	s_options.display.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.display.generic.callback	= Options_Event;
	s_options.display.generic.id		= ID_DISPLAY;
	s_options.display.generic.x			= 320;
	s_options.display.generic.y			= y;
	s_options.display.string			= "DISPLAY";
	s_options.display.color				= tdc_text_color;
	s_options.display.style				= UI_SMALLFONT|UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.sound.generic.type		= MTYPE_PTEXT;
	s_options.sound.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.sound.generic.callback	= Options_Event;
	s_options.sound.generic.id			= ID_SOUND;
	s_options.sound.generic.x			= 320;
	s_options.sound.generic.y			= y;
	s_options.sound.string				= "SOUND";
	s_options.sound.color				= tdc_text_color;
	s_options.sound.style				= UI_SMALLFONT|UI_CENTER;

/*	y += VERTICAL_SPACING;
	s_options.network.generic.type		= MTYPE_PTEXT;
	s_options.network.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.network.generic.callback	= Options_Event;
	s_options.network.generic.id		= ID_NETWORK;
	s_options.network.generic.x			= 320;
	s_options.network.generic.y			= y;
	s_options.network.string			= "NETWORK";
	s_options.network.color				= color_red;
	s_options.network.style				= UI_CENTER;
*/
	s_options.back.generic.type	    = MTYPE_PTEXT;
	s_options.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.back.generic.callback = Options_Event;
	s_options.back.generic.id	    = ID_BACK;
	s_options.back.generic.x		= 32;
	s_options.back.generic.y		= 480-36;
	s_options.back.string  		    = "back";
	s_options.back.color  		    = tdc_text_color;
	s_options.back.style	        = UI_LEFT;


	Menu_AddItem( &s_options.menu, ( void * ) &s_options.background );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.banner );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.graphics );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.display );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.sound );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.back );
}


/*
===============
UI_SystemConfigMenu
===============
*/
void UI_SystemConfigMenu( void ) {
	Options_MenuInit();
	UI_PushMenu ( &s_options.menu );
}
