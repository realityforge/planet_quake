// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=============================================================================

SAVE/LOAD Menu

JPL (jpl@ing.puc.cl) - ACE Team

=============================================================================
*/

#include "ui_local.h"


#define BACKGROUND	"menu/art/dark_scene"
#define ART_ARROWS			"menu/art/arrows_horz_0"
#define ART_ARROWLEFT		"menu/art/arrows_horz_left"
#define ART_ARROWRIGHT		"menu/art/arrows_horz_right"
#define ID_NAME						10
#define ID_LIST					11
#define ID_BACK						12
#define ID_DOIT						13
#define ID_LEFT						14
#define ID_RIGHT					15


#define MAX_NAMELENGTH	20

#define MAX_SAVES			128
#define NAMEBUFSIZE			( MAX_SAVES * 16 )

#define ARROWS_WIDTH		128
#define ARROWS_HEIGHT		48


typedef struct {
	menuframework_s	menu;

	menubitmap_s		background;
	menubitmap_s		arrows;
	menubitmap_s		left;
	menubitmap_s		right;
	menutext_s		art_banner;
	menufield_s		item_name;
	menulist_s		item_list;
	menutext_s		item_back;
	menutext_s	    item_doit;

	char			names[NAMEBUFSIZE];
	char*			saveslist[MAX_SAVES];

} saveloadMenuInfo_t;

static saveloadMenuInfo_t	saveloadMenuInfo;
int saveload;
int nosaved;

/*
=================
SaveLoad_DrawName
=================
*/
static void SaveLoad_DrawName( void *self ) {
	menufield_s		*f;
	qboolean		focus;
	int				style;
	char			*txt;
	char			c;
	float			*color;
	int				n;
	int				basex, x, y;

	f = (menufield_s*)self;
	basex = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( basex, y, "Select File Name", style, color );

	// draw the actual name
	basex += 64;
	y += PROP_HEIGHT;
	txt = f->field.buffer;
	color = g_color_table[ColorIndex(COLOR_WHITE)];
	x = basex;
	while ( (c = *txt) != 0 ) {
		if ( !focus && Q_IsColorString( txt ) ) {
			n = ColorIndex( *(txt+1) );
			if( n == 0 ) {
				n = 7;
			}
			color = g_color_table[n];
			txt += 2;
			continue;
		}
		UI_DrawChar( x, y, c, style, color );
		txt++;
		x += SMALLCHAR_WIDTH;
	}

	// draw cursor if we have focus
	if( focus ) {
		if ( trap_Key_GetOverstrikeMode() ) {
			c = 11;
		} else {
			c = 10;
		}

		style &= ~UI_PULSE;
		style |= UI_BLINK;

		UI_DrawChar( basex + f->field.cursor * SMALLCHAR_WIDTH, y, c, style, color_white );
	}

}


/*
=================
UI_SaveLoadMenu_ListEvent
=================
*/
static void SaveLoad_ListEvent( void *ptr, int notification ) {
	if (notification != QM_CLICK)
		return;

	if (!nosaved)
	{
		strcpy(saveloadMenuInfo.item_name.field.buffer,saveloadMenuInfo.item_list.itemnames[saveloadMenuInfo.item_list.curvalue]);
		saveloadMenuInfo.item_name.field.cursor=0;
	}
}

/*
=================
UI_SaveLoadMenu_RightEvent
=================
*/
static void SaveLoad_RightEvent( void *ptr, int notification ) {
	if (notification != QM_ACTIVATED)
		return;

	ScrollList_Key( &saveloadMenuInfo.item_list, K_RIGHTARROW );
}

/*
=================
UI_SaveLoadMenu_LeftEvent
=================
*/
static void SaveLoad_LeftEvent( void *ptr, int notification ) {
	if (notification != QM_ACTIVATED)
		return;

	ScrollList_Key( &saveloadMenuInfo.item_list, K_LEFTARROW );
}





/*
=================
UI_SaveLoadMenu_DoitEvent
=================
*/
static void UI_SaveLoadMenu_DoItEvent( void *ptr, int notification ) {
	if (notification != QM_ACTIVATED)
		return;

	UI_PopMenu();
	UI_PopMenu();
	if (saveload)	// save
	{
		trap_Cvar_Set( "save_game", saveloadMenuInfo.item_name.field.buffer );	// JPL - for allowing save/load
	}
	else		// load
	{
		trap_Cvar_Set( "load_game", saveloadMenuInfo.item_name.field.buffer );	// JPL - for allowing save/load
		UI_SPArena_Start(NULL);
	}
}


/*
=================
UI_SaveLoadMenu_BackEvent
=================
*/
static void UI_SaveLoadMenu_BackEvent( void* ptr, int notification ) {
	if (notification != QM_ACTIVATED) {
		return;
	}

	UI_PopMenu();
}


/*
=================
UI_SaveLoadMenu_Key
=================
*/
static sfxHandle_t UI_SaveLoadMenu_Key( int key ) {
	return Menu_DefaultKey( &saveloadMenuInfo.menu, key );
}


/*
=================
UI_SaveLoadMenu_Cache
=================
*/
void UI_SaveLoadMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( BACKGROUND );
}


/*
=================
UI_SaveLoadMenu_Init
=================
*/
static void UI_SaveLoadMenu_Init( void ) {

	char	*savename;
	int y,i,len;
	
	nosaved=0;

	memset( &saveloadMenuInfo, 0, sizeof(saveloadMenuInfo) );
	saveloadMenuInfo.menu.fullscreen = qtrue;
	saveloadMenuInfo.menu.key = UI_SaveLoadMenu_Key;

	UI_SaveLoadMenu_Cache();

	saveloadMenuInfo.art_banner.generic.type		= MTYPE_BTEXT;
	saveloadMenuInfo.art_banner.generic.flags		= QMF_CENTER_JUSTIFY;
	saveloadMenuInfo.art_banner.generic.x			= 320;
	saveloadMenuInfo.art_banner.generic.y			= 16;
	saveloadMenuInfo.art_banner.string				= (saveload)?"SAVE GAME":"LOAD GAME";
	saveloadMenuInfo.art_banner.color				= color_white;
	saveloadMenuInfo.art_banner.style				= UI_CENTER;

	
	saveloadMenuInfo.background.generic.type		= MTYPE_BITMAP;
	saveloadMenuInfo.background.generic.name		= BACKGROUND;
	saveloadMenuInfo.background.generic.flags		= QMF_INACTIVE ;
	saveloadMenuInfo.background.generic.x			= 0;
	saveloadMenuInfo.background.generic.y			= 0;
	saveloadMenuInfo.background.width				= 640;
	saveloadMenuInfo.background.height				= 480;

	saveloadMenuInfo.item_back.generic.type			= MTYPE_PTEXT;
	saveloadMenuInfo.item_back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	saveloadMenuInfo.item_back.generic.callback		= UI_SaveLoadMenu_BackEvent;
	saveloadMenuInfo.item_back.generic.id			= ID_BACK;
	saveloadMenuInfo.item_back.generic.x			= 32;
	saveloadMenuInfo.item_back.generic.y			= 480-36;
	saveloadMenuInfo.item_back.string				= "Back";
	saveloadMenuInfo.item_back.color 				= tdc_text_color;
	saveloadMenuInfo.item_back.style				= UI_LEFT;

	saveloadMenuInfo.item_doit.generic.type		= MTYPE_PTEXT;
	saveloadMenuInfo.item_doit.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	saveloadMenuInfo.item_doit.generic.callback	= UI_SaveLoadMenu_DoItEvent;
	saveloadMenuInfo.item_doit.generic.id			= ID_DOIT;
	saveloadMenuInfo.item_doit.generic.x			= 640-32;
	saveloadMenuInfo.item_doit.generic.y			= 480-36;
	saveloadMenuInfo.item_doit.string				= (saveload)?"Save":"Load";
	saveloadMenuInfo.item_doit.color				= tdc_text_color;
	saveloadMenuInfo.item_doit.style			= UI_RIGHT;

	saveloadMenuInfo.arrows.generic.type	= MTYPE_BITMAP;
	saveloadMenuInfo.arrows.generic.name	= ART_ARROWS;
	saveloadMenuInfo.arrows.generic.flags	= QMF_INACTIVE;
	saveloadMenuInfo.arrows.generic.x		= 320-ARROWS_WIDTH/2;
	saveloadMenuInfo.arrows.generic.y		= 400;
	saveloadMenuInfo.arrows.width			= ARROWS_WIDTH;
	saveloadMenuInfo.arrows.height			= ARROWS_HEIGHT;

	saveloadMenuInfo.left.generic.type		= MTYPE_BITMAP;
	saveloadMenuInfo.left.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	saveloadMenuInfo.left.generic.x		= 320-ARROWS_WIDTH/2;
	saveloadMenuInfo.left.generic.y		= 400;
	saveloadMenuInfo.left.generic.id		= ID_LEFT;
	saveloadMenuInfo.left.generic.callback	= SaveLoad_LeftEvent;
	saveloadMenuInfo.left.width			= ARROWS_WIDTH/2;
	saveloadMenuInfo.left.height			= ARROWS_HEIGHT;
	saveloadMenuInfo.left.focuspic			= ART_ARROWLEFT;

	saveloadMenuInfo.right.generic.type	= MTYPE_BITMAP;
	saveloadMenuInfo.right.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	saveloadMenuInfo.right.generic.x		= 320;
	saveloadMenuInfo.right.generic.y		= 400;
	saveloadMenuInfo.right.generic.id		= ID_RIGHT;
	saveloadMenuInfo.right.generic.callback = SaveLoad_RightEvent;
	saveloadMenuInfo.right.width			= ARROWS_WIDTH/2;
	saveloadMenuInfo.right.height			= ARROWS_HEIGHT;
	saveloadMenuInfo.right.focuspic		= ART_ARROWRIGHT;

	y=100;
	saveloadMenuInfo.item_name.generic.type			= MTYPE_FIELD;
	saveloadMenuInfo.item_name.generic.flags			= QMF_NODEFAULTINIT | QMF_UPPERCASE;
	saveloadMenuInfo.item_name.generic.ownerdraw		= SaveLoad_DrawName;
	saveloadMenuInfo.item_name.field.widthInChars	= MAX_NAMELENGTH;
	saveloadMenuInfo.item_name.field.maxchars		= MAX_NAMELENGTH;
	saveloadMenuInfo.item_name.generic.x				= 192;
	saveloadMenuInfo.item_name.generic.y				= y;
	saveloadMenuInfo.item_name.generic.left			= 192 - 8;
	saveloadMenuInfo.item_name.generic.top			= y - 8;
	saveloadMenuInfo.item_name.generic.right			= 192 + 200;
	saveloadMenuInfo.item_name.generic.bottom		= y + 2 * PROP_HEIGHT;

	if (!saveload)
		saveloadMenuInfo.item_name.generic.flags |= QMF_INACTIVE;

	saveloadMenuInfo.item_list.generic.type		= MTYPE_SCROLLLIST;
	saveloadMenuInfo.item_list.generic.flags	= QMF_PULSEIFFOCUS;
	saveloadMenuInfo.item_list.generic.callback	= SaveLoad_ListEvent;
	saveloadMenuInfo.item_list.generic.id		= ID_LIST;
	saveloadMenuInfo.item_list.generic.x		= 108;
	saveloadMenuInfo.item_list.generic.y		= 160;
	saveloadMenuInfo.item_list.width			= 20;
	saveloadMenuInfo.item_list.height			= 14;
	saveloadMenuInfo.item_list.numitems			= trap_FS_GetFileList( "", "sav", saveloadMenuInfo.names, NAMEBUFSIZE );
	saveloadMenuInfo.item_list.itemnames		= (const char **)saveloadMenuInfo.saveslist;
	saveloadMenuInfo.item_list.columns			= 2;

	savename = saveloadMenuInfo.names;
	if (!saveloadMenuInfo.item_list.numitems) {
		if (!saveload)
		{
			strcpy(saveloadMenuInfo.names,"No Saved Games Found");
			//degenerate case, not selectable	
			saveloadMenuInfo.item_doit.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
			nosaved=1;
		}

		saveloadMenuInfo.item_list.numitems = 1;

	}
	else if (saveloadMenuInfo.item_list.numitems > MAX_SAVES)
	{
		saveloadMenuInfo.item_list.numitems = MAX_SAVES;
		saveloadMenuInfo.item_name.field.widthInChars=strlen(savename);
	}
	
	for ( i = 0; i < saveloadMenuInfo.item_list.numitems; i++ ) {
		saveloadMenuInfo.item_list.itemnames[i] = savename;
		
		// strip extension
		len = strlen( savename );
		if (!Q_stricmp(savename +  len - 4,".sav"))
			savename[len-4] = '\0';

		Q_strupr(savename);
		if (i==0 && !nosaved)
		{
			strcpy(saveloadMenuInfo.item_name.field.buffer,savename);
		}

		savename += len + 1;
	}

	Menu_AddItem( &saveloadMenuInfo.menu,( void * )&saveloadMenuInfo.background );
	
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.art_banner );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.arrows );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.left );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.right );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.item_name );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.item_list );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.item_back );
	Menu_AddItem( &saveloadMenuInfo.menu, ( void * )&saveloadMenuInfo.item_doit );

}


/*
=====================================
UI_SaveLoadMenu

=====================================
*/
void UI_SaveLoadMenu( int save ) { // 0 = load, 1 = save
	saveload=save;
	UI_SaveLoadMenu_Init();
	UI_PushMenu( &saveloadMenuInfo.menu );
	Menu_SetCursorToItem( &saveloadMenuInfo.menu, &saveloadMenuInfo.item_doit );
}


