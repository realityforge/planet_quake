// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

CREDITS

// JPL - ACE Team  - Added intro code & credits

=======================================================================
*/

/*
===========================================
SCROLLING CREDITS CODE
Author: Iain McGinniss
Company: AZ Development Team
Produced for Code 3 Arena, feel freeto use this code in your own projects
just please remember to give credit	to AZ Development team for this.
That should be easy now with thisnew credits system!
===========================================
*/


#include "ui_local.h"


#define BACKGROUND  "menu/art/dark_scene"


int SCROLLSPEED; // The scrolling speed in pixels per second.
int credits_len;
int exit_game;

typedef struct {
	menuframework_s	menu;
} creditsmenu_t;

static creditsmenu_t	s_credits;

int starttime; // game time at which credits are started
float mvolume; // records the original music volume level, as we will
               // modify it for the credits

/*
vec4_t color_headertext			= {0.80, 0.80, 0.80, 1.00};
vec4_t color_maintext			= {0.80, 0.80, 0.80, 1.00};
*/

vec4_t color_headertext			= {1.00, 1.00, 1.00, 1.00};
vec4_t color_maintext			= {0.60, 0.60, 0.60, 1.00};

qhandle_t	Background; // definition of the background shader pointer

typedef struct
{
	char *string;
	int style;
	vec4_t *colour;
} cr_line;

cr_line final_credits[] = { 
{ "ACE TEAM IS:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "GAME DESIGN AND CONCEPT:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "ANDRES BORDEU, CARLOS BORDEU", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "3D AND 2D ART:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "ANDRES BORDEU, CARLOS BORDEU, EDMUNDO BORDEU", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "LEAD PROGRAMMING:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "JUAN PABLO LASTRA", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "PROGRAMMING:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "ESTEBAN HURTADO, ANA MARIA ALBORNOZ", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "MUSIC: ", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "KEVIN RIEPL",UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "WWW.XPLORESTUDIOS.COM" ,UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "SOUND: ", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "KLAUS REITER" ,UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "LEVEL DESIGN:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "ANDRES BORDEU, CARLOS BORDEU, EDMUNDO BORDEU", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "SPECIAL THANKS:", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "ID SOFTWARE, PLANETQUAKE", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "THE DARK CONJUNCTION", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "WWW.PLANETQUAKE.COM/TDC", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
  {NULL}
};

cr_line intro_story[] = { 
{ "The Dark Conjunction", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "Thousands of years ago, when the world was", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "nothing like the one we know now, the", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "universe was in equilibrium. Two opposite", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "forces controlled the destiny of all mortal", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "life. They were the keepers of all creation.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "For some unknown reason, when this world was", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "still young, both forces entered in a", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "terrible conflict. The greatest battle", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "of all times began. Because of the mighty", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "gods' confrontation, death struck every", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "living being and life itself was almost", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "annihilated from the universe.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "During that great war, the eldest of the", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "dark ones was Ilak. Ilak called upon all of", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "his allies to battle on his side. However,", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "the war was so fierce that not even the ", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "mightiest of the rulers of the universe", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "could survive. The two forces that once ", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "controlled everything vanished.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "But Ajivatkar, one of the great gods of", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "darkness, had not perished in the battle.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "He had foreseen the fate of his kind.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "Ajivatkar imprisoned himself in the depths", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "of a mortal world in order to survive. That", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "world is known by us as planet earth.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "Among the survivors of the war were the", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "ilaken, who had been always loyal to the", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "great Ilak. They quickly realized what", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "Ajivatkar had done and built an  underground", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "temple to seal off the dark god so that no", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "one could ever awaken him.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "Centuries have passed and life has returned.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "The ilaken became the keepers of the temple", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "and were never seen again. Their story is", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "only a myth.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "", UI_CENTER|UI_SMALLFONT, &color_headertext },
{ "", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "But today, history takes a new course.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "Flying aboard a B-29, you are unaware of the", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "evil presence that awakens from its prison.", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "The seal that imprisoned Ajivatkar has been", UI_CENTER|UI_SMALLFONT, &color_maintext },
{ "broken. The Dark Conjunction is at hand.", UI_CENTER|UI_SMALLFONT, &color_maintext },
  {NULL}
};

cr_line *credits;

/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key( int key ) {
	if( key & K_CHAR_FLAG ) {
		return 0;
	}
	if (exit_game)
		trap_Cmd_ExecuteText( EXEC_APPEND, va("s_musicvolume %f; quit\n", mvolume));
	else
		UI_SPArena_Start( NULL );

	return 0;
}


/*
=================
ScrollingCredits_Draw
This is the main drawing function for the credits.Most of the code is self-explanatory.
=================
*/
static void ScrollingCredits_Draw(void)
{
  int x = 320, y, n, ysize = 0, fadetime = 0;
  vec4_t fadecolour = { 0.00, 0.00, 0.00, 0.00 };

    if(!ysize) 
  {
    // loop through entire credits array
    for(n = 0; n <= credits_len - 1; n++) 
    {
      // it is a small character
      if(credits[n].style & UI_SMALLFONT) 
      {
        // add small character height
        ysize += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
        
      // it is a big character
      }else if(credits[n].style & UI_BIGFONT) 
      {
        // add big character size
        ysize += PROP_HEIGHT;
        
      // it is a huge character
      }else if(credits[n].style & UI_GIANTFONT) 
      {
        // add giant character size.
        ysize += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE); 
      }
    }
  }

 //fondo 
  UI_DrawHandlePic(0, 0, 640,  480, Background);
  
  // let's draw the stuff
  // set initial y location
  y = 480 - SCROLLSPEED * (float)(uis.realtime - starttime) / 100;
  
  // loop through the entire credits sequence
  for(n = 0; n <= credits_len - 1; n++)
  {
    // this NULL string marks the end of the credits struct
    if(credits[n].string == NULL) 
    {
      if(y < -16) // credits sequence is completely off screen
      {
		if (exit_game)
			trap_Cmd_ExecuteText( EXEC_APPEND, va("s_musicvolume %f; quit\n", mvolume));
		else
			UI_SPArena_Start( NULL );
        break; // end of credits
      }
      break;
    }
		
    if( strlen(credits[n].string) == 1) // spacer string, no need to draw
      continue;

    if( y > -(PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE))) 
      // the line is within the visible range of the screen
      if (exit_game)
		  UI_DrawProportionalString(x, y, credits[n].string, 
                                credits[n].style, *credits[n].colour );
	  else
		  UI_DrawString(x,y,credits[n].string,credits[n].style,*credits[n].colour);
		
    // re-adjust y for next line
    if(credits[n].style & UI_SMALLFONT)
    {
		if (exit_game)
			y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
		else
			y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE/1.8;
    }else if(credits[n].style & UI_BIGFONT)
    {
      y += PROP_HEIGHT;
    }else if(credits[n].style & UI_GIANTFONT)
    {
      y += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE);
    }

	// if y is off the screen, break out of loop
    if (y > 480)
    break;
  }
}

/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu( int text_id ) {		// 0 = final credits, 1 = intro story
	memset( &s_credits, 0 ,sizeof(s_credits) );

	// JPL - for intro story & credits
	if (!text_id)
	{
		credits=final_credits;
		credits_len=sizeof(final_credits);
		SCROLLSPEED=5;
		exit_game=1;
	}
	else
	{
		credits=intro_story;
		credits_len=sizeof(intro_story);
		SCROLLSPEED=1.5;
		exit_game=0;
	}

	s_credits.menu.draw = ScrollingCredits_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;
	UI_PushMenu ( &s_credits.menu );

	starttime = uis.realtime; // record start time for credits to scroll properly
	mvolume = trap_Cvar_VariableValue( "s_musicvolume" );
//	if(mvolume < 0.5)
	//	trap_Cmd_ExecuteText( EXEC_APPEND, "s_musicvolume 0.5\n" );
	//trap_Cmd_ExecuteText( EXEC_APPEND, "music music/fla22k_02\n" );


	Background = trap_R_RegisterShaderNoMip(BACKGROUND);


}
