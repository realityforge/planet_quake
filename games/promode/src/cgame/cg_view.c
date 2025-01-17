/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering


#include "cg_local.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f( void )
{
  vec3_t    angles;

  memset( &cg.testModelEntity, 0, sizeof( cg.testModelEntity ) );
  memset( &cg.testModelBarrelEntity, 0, sizeof( cg.testModelBarrelEntity ) );

  if( trap_Argc( ) < 2 )
    return;

  Q_strncpyz( cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
  cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

  Q_strncpyz( cg.testModelBarrelName, CG_Argv( 1 ), MAX_QPATH );
  cg.testModelBarrelName[ strlen( cg.testModelBarrelName ) - 4 ] = '\0';
  Q_strcat( cg.testModelBarrelName, MAX_QPATH, "_barrel.md3" );
  cg.testModelBarrelEntity.hModel = trap_R_RegisterModel( cg.testModelBarrelName );

  if( trap_Argc( ) == 3 )
  {
    cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
    cg.testModelEntity.frame = 1;
    cg.testModelEntity.oldframe = 0;
  }

  if( !cg.testModelEntity.hModel )
  {
    CG_Printf( "Can't register model\n" );
    return;
  }

  VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[ 0 ], cg.testModelEntity.origin );

  angles[ PITCH ] = 0;
  angles[ YAW ] = 180 + cg.refdefViewAngles[ 1 ];
  angles[ ROLL ] = 0;

  AnglesToAxis( angles, cg.testModelEntity.axis );
  cg.testGun = qfalse;

  if( cg.testModelBarrelEntity.hModel )
  {
    angles[ YAW ] = 0;
    angles[ PITCH ] = 0;
    angles[ ROLL ] = 0;
    AnglesToAxis( angles, cg.testModelBarrelEntity.axis );
  }
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f( void )
{
  CG_TestModel_f( );
  cg.testGun = qtrue;
  cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f( void )
{
  cg.testModelEntity.frame++;
  CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f( void )
{
  cg.testModelEntity.frame--;

  if( cg.testModelEntity.frame < 0 )
    cg.testModelEntity.frame = 0;

  CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f( void )
{
  cg.testModelEntity.skinNum++;
  CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f( void )
{
  cg.testModelEntity.skinNum--;

  if( cg.testModelEntity.skinNum < 0 )
    cg.testModelEntity.skinNum = 0;

  CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel( void )
{
  int   i;

  // re-register the model, because the level may have changed
  cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
  cg.testModelBarrelEntity.hModel = trap_R_RegisterModel( cg.testModelBarrelName );

  if( !cg.testModelEntity.hModel )
  {
    CG_Printf( "Can't register model\n" );
    return;
  }

  // if testing a gun, set the origin reletive to the view origin
  if( cg.testGun )
  {
    VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
    VectorCopy( cg.refdef.viewaxis[ 0 ], cg.testModelEntity.axis[ 0 ] );
    VectorCopy( cg.refdef.viewaxis[ 1 ], cg.testModelEntity.axis[ 1 ] );
    VectorCopy( cg.refdef.viewaxis[ 2 ], cg.testModelEntity.axis[ 2 ] );

    // allow the position to be adjusted
    for( i = 0; i < 3; i++ )
    {
      cg.testModelEntity.origin[ i ] += cg.refdef.viewaxis[ 0 ][ i ] * cg_gun_x.value;
      cg.testModelEntity.origin[ i ] += cg.refdef.viewaxis[ 1 ][ i ] * cg_gun_y.value;
      cg.testModelEntity.origin[ i ] += cg.refdef.viewaxis[ 2 ][ i ] * cg_gun_z.value;
    }
  }

  trap_R_AddRefEntityToScene( &cg.testModelEntity );

  if( cg.testModelBarrelEntity.hModel )
  {
    CG_PositionEntityOnTag( &cg.testModelBarrelEntity, &cg.testModelEntity,
        cg.testModelEntity.hModel, "tag_barrel" );

    trap_R_AddRefEntityToScene( &cg.testModelBarrelEntity );
  }
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect( void )
{
  int   size;

  // the intermission should allways be full screen
  if( cg.snap->ps.pm_type == PM_INTERMISSION )
    size = 100;
  else
  {
    // bound normal viewsize
    if( cg_viewsize.integer < 30 )
    {
      trap_Cvar_Set( "cg_viewsize", "30" );
      size = 30;
    }
    else if( cg_viewsize.integer > 100 )
    {
      trap_Cvar_Set( "cg_viewsize","100" );
      size = 100;
    }
    else
      size = cg_viewsize.integer;
  }

  cg.refdef.width = cgs.glconfig.vidWidth * size / 100;
  cg.refdef.width &= ~1;

  cg.refdef.height = cgs.glconfig.vidHeight * size / 100;
  cg.refdef.height &= ~1;

  cg.refdef.x = ( cgs.glconfig.vidWidth - cg.refdef.width ) / 2;
  cg.refdef.y = ( cgs.glconfig.vidHeight - cg.refdef.height ) / 2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define FOCUS_DISTANCE  512
static void CG_OffsetThirdPersonView( void )
{
  vec3_t        forward, right, up;
  vec3_t        view;
  vec3_t        focusAngles;
  trace_t       trace;
  static vec3_t mins = { -8, -8, -8 };
  static vec3_t maxs = { 8, 8, 8 };
  vec3_t        focusPoint;
  float         focusDist;
  float         forwardScale, sideScale;
  vec3_t        surfNormal;

  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( surfNormal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( cg.predictedPlayerState.grapplePoint, surfNormal );
  }
  else
    VectorSet( surfNormal, 0.0f, 0.0f, 1.0f );

  VectorMA( cg.refdef.vieworg, cg.predictedPlayerState.viewheight, surfNormal, cg.refdef.vieworg );

  VectorCopy( cg.refdefViewAngles, focusAngles );

  // if dead, look at killer
  if( cg.predictedPlayerState.stats[ STAT_HEALTH ] <= 0 )
  {
    focusAngles[ YAW ] = cg.predictedPlayerState.stats[ STAT_VIEWLOCK ];
    cg.refdefViewAngles[ YAW ] = cg.predictedPlayerState.stats[ STAT_VIEWLOCK ];
  }

  //if ( focusAngles[PITCH] > 45 ) {
  //  focusAngles[PITCH] = 45;    // don't go too far overhead
  //}
  AngleVectors( focusAngles, forward, NULL, NULL );

  VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

  VectorCopy( cg.refdef.vieworg, view );

  VectorMA( view, cg_thirdpersonheight.integer, surfNormal, view ); //12

  //cg.refdefViewAngles[PITCH] *= 0.5;

  AngleVectors( cg.refdefViewAngles, forward, right, up );

  forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
  sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
  VectorMA( view, -cg_thirdPersonRange.value * forwardScale, forward, view );
  VectorMA( view, -cg_thirdPersonRange.value * sideScale, right, view );

  // trace a ray from the origin to the viewpoint to make sure the view isn't
  // in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

  if( !cg_cameraMode.integer )
  {
    CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

    if( trace.fraction != 1.0 )
    {
      VectorCopy( trace.endpos, view );
      view[ 2 ] += ( 1.0 - trace.fraction ) * 32; //*32
      // try another trace to this position, because a tunnel may have the ceiling
      // close enogh that this is poking out

      CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
      VectorCopy( trace.endpos, view );
    }
  }

  VectorCopy( view, cg.refdef.vieworg );

  // select pitch to look at focus point from vieword
  VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
  focusDist = sqrt( focusPoint[ 0 ] * focusPoint[ 0 ] + focusPoint[ 1 ] * focusPoint[ 1 ] );
  if ( focusDist < 1 ) {
    focusDist = 1;  // should never happen
  }
  cg.refdefViewAngles[ PITCH ] = -180 / M_PI * atan2( focusPoint[ 2 ], focusDist );
  cg.refdefViewAngles[ YAW ] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void )
{
  float         steptime;
  int            timeDelta;
  vec3_t        normal;
  playerState_t *ps = &cg.predictedPlayerState;

  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( ps->grapplePoint, normal );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );

  steptime = BG_FindSteptimeForClass( ps->stats[ STAT_PCLASS ] );

  // smooth out stair climbing
  timeDelta = cg.time - cg.stepTime;
  if( timeDelta < steptime )
  {
    float stepChange = cg.stepChange
      * (steptime - timeDelta) / steptime;

    if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
      VectorMA( cg.refdef.vieworg, -stepChange, normal, cg.refdef.vieworg );
    else
      cg.refdef.vieworg[ 2 ] -= stepChange;
  }
}

#define PCLOUD_ROLL_AMPLITUDE   25.0f
#define PCLOUD_ROLL_FREQUENCY   0.4f
#define PCLOUD_ZOOM_AMPLITUDE   15
#define PCLOUD_ZOOM_FREQUENCY   0.7f


/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void )
{
  float         *origin;
  float         *angles;
  float         bob;
  float         ratio;
  float         delta;
  float         speed;
  float         f;
  vec3_t        predictedVelocity;
  int           timeDelta;
  float         bob2;
  vec3_t        normal, baseOrigin;
  playerState_t *ps = &cg.predictedPlayerState;

  if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBING )
  {
    if( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING )
      VectorSet( normal, 0.0f, 0.0f, -1.0f );
    else
      VectorCopy( ps->grapplePoint, normal );
  }
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );


  if( cg.snap->ps.pm_type == PM_INTERMISSION )
    return;

  origin = cg.refdef.vieworg;
  angles = cg.refdefViewAngles;

  VectorCopy( origin, baseOrigin );

  // if dead, fix the angle and don't add any kick
  if( cg.snap->ps.stats[ STAT_HEALTH ] <= 0 )
  {
    angles[ ROLL ] = 40;
    angles[ PITCH ] = -15;
    angles[ YAW ] = cg.snap->ps.stats[ STAT_VIEWLOCK ];
    origin[ 2 ] += cg.predictedPlayerState.viewheight;
    return;
  }

  // add angles based on weapon kick
  VectorAdd( angles, cg.kick_angles, angles );

  // add angles based on damage kick
  if( cg.damageTime )
  {
    ratio = cg.time - cg.damageTime;
    if( ratio < DAMAGE_DEFLECT_TIME )
    {
      ratio /= DAMAGE_DEFLECT_TIME;
      angles[ PITCH ] += ratio * cg.v_dmg_pitch;
      angles[ ROLL ] += ratio * cg.v_dmg_roll;
    }
    else
    {
      ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
      if( ratio > 0 )
      {
        angles[ PITCH ] += ratio * cg.v_dmg_pitch;
        angles[ ROLL ] += ratio * cg.v_dmg_roll;
      }
    }
  }

  // add pitch based on fall kick
#if 0
  ratio = ( cg.time - cg.landTime) / FALL_TIME;
  if (ratio < 0)
    ratio = 0;
  angles[PITCH] += ratio * cg.fall_value;
#endif

  // add angles based on velocity
  VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

  delta = DotProduct( predictedVelocity, cg.refdef.viewaxis[ 0 ] );
  angles[ PITCH ] += delta * cg_runpitch.value;

  delta = DotProduct( predictedVelocity, cg.refdef.viewaxis[ 1 ] );
  angles[ ROLL ] -= delta * cg_runroll.value;

  // add angles based on bob
  // bob amount is class dependant

  if( cg.snap->ps.persistant[ PERS_TEAM ] == TEAM_SPECTATOR )
    bob2 = 0.0f;
  else
    bob2 = BG_FindBobForClass( cg.predictedPlayerState.stats[ STAT_PCLASS ] );


#define LEVEL4_FEEDBACK  10.0f

  //give a charging player some feedback
  if( ps->weapon == WP_ALEVEL4 )
  {
    if( ps->stats[ STAT_MISC ] > 0 )
    {
      float fraction = (float)ps->stats[ STAT_MISC ] / (float)LEVEL4_CHARGE_TIME;

      if( fraction > 1.0f )
        fraction = 1.0f;

      bob2 *= ( 1.0f + fraction * LEVEL4_FEEDBACK );
    }
  }

  if( bob2 != 0.0f )
  {
    // make sure the bob is visible even at low speeds
    speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

    delta = cg.bobfracsin * ( bob2 ) * speed;
    if( cg.predictedPlayerState.pm_flags & PMF_DUCKED )
      delta *= 3;   // crouching

    angles[ PITCH ] += delta;
    delta = cg.bobfracsin * ( bob2 ) * speed;
    if( cg.predictedPlayerState.pm_flags & PMF_DUCKED )
      delta *= 3;   // crouching accentuates roll

    if( cg.bobcycle & 1 )
      delta = -delta;

    angles[ ROLL ] += delta;
  }

#define LEVEL3_FEEDBACK  20.0f

  //provide some feedback for pouncing
  if( cg.predictedPlayerState.weapon == WP_ALEVEL3 ||
      cg.predictedPlayerState.weapon == WP_ALEVEL3_UPG )
  {
    if( cg.predictedPlayerState.stats[ STAT_MISC ] > 0 )
    {
      float   fraction1, fraction2;
      vec3_t  forward;

      AngleVectors( angles, forward, NULL, NULL );
      VectorNormalize( forward );

      fraction1 = (float)( cg.time - cg.weapon2Time ) / (float)LEVEL3_POUNCE_CHARGE_TIME;

      if( fraction1 > 1.0f )
        fraction1 = 1.0f;

      fraction2 = -sin( fraction1 * M_PI / 2 );

      VectorMA( origin, LEVEL3_FEEDBACK * fraction2, forward, origin );
    }
  }

#define STRUGGLE_DIST 5.0f
#define STRUGGLE_TIME 250

  //allow the player to struggle a little whilst grabbed
  if( cg.predictedPlayerState.pm_type == PM_GRABBED )
  {
    vec3_t    forward, right, up;
    usercmd_t cmd;
    int       cmdNum;
    float     fFraction, rFraction, uFraction;
    float     fFraction2, rFraction2, uFraction2;

    cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd( cmdNum, &cmd );

    AngleVectors( angles, forward, right, up );

    fFraction = (float)( cg.time - cg.forwardMoveTime ) / STRUGGLE_TIME;
    rFraction = (float)( cg.time - cg.rightMoveTime ) / STRUGGLE_TIME;
    uFraction = (float)( cg.time - cg.upMoveTime ) / STRUGGLE_TIME;

    if( fFraction > 1.0f )
      fFraction = 1.0f;
    if( rFraction > 1.0f )
      rFraction = 1.0f;
    if( uFraction > 1.0f )
      uFraction = 1.0f;

    fFraction2 = -sin( fFraction * M_PI / 2 );
    rFraction2 = -sin( rFraction * M_PI / 2 );
    uFraction2 = -sin( uFraction * M_PI / 2 );

    if( cmd.forwardmove > 0 )
      VectorMA( origin, STRUGGLE_DIST * fFraction, forward, origin );
    else if( cmd.forwardmove < 0 )
      VectorMA( origin, -STRUGGLE_DIST * fFraction, forward, origin );
    else
      cg.forwardMoveTime = cg.time;

    if( cmd.rightmove > 0 )
      VectorMA( origin, STRUGGLE_DIST * rFraction, right, origin );
    else if( cmd.rightmove < 0 )
      VectorMA( origin, -STRUGGLE_DIST * rFraction, right, origin );
    else
      cg.rightMoveTime = cg.time;

    if( cmd.upmove > 0 )
      VectorMA( origin, STRUGGLE_DIST * uFraction, up, origin );
    else if( cmd.upmove < 0 )
      VectorMA( origin, -STRUGGLE_DIST * uFraction, up, origin );
    else
      cg.upMoveTime = cg.time;
  }

  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_POISONCLOUDED &&
      !( cg.snap->ps.pm_flags & PMF_FOLLOW ) )
  {
    float fraction = sin( ( (float)cg.time / 1000.0f ) * M_PI * 2 * PCLOUD_ROLL_FREQUENCY );
    float pitchFraction = sin( ( (float)cg.time / 1000.0f ) * M_PI * 5 * PCLOUD_ROLL_FREQUENCY );

    fraction *= 1.0f - ( ( cg.time - cg.poisonedTime ) / (float)LEVEL1_PCLOUD_TIME );
    pitchFraction *= 1.0f - ( ( cg.time - cg.poisonedTime ) / (float)LEVEL1_PCLOUD_TIME );

    angles[ ROLL ] += fraction * PCLOUD_ROLL_AMPLITUDE;
    angles[ YAW ] += fraction * PCLOUD_ROLL_AMPLITUDE;
    angles[ PITCH ] += pitchFraction * PCLOUD_ROLL_AMPLITUDE / 2.0f;
  }

  // this *feels* more realisitic for humans
  if( cg.predictedPlayerState.stats[ STAT_PTEAM ] == PTE_HUMANS )
  {
    angles[PITCH] += cg.bobfracsin * bob2 * 0.5;

    // heavy breathing effects //FIXME: sound
    if( cg.predictedPlayerState.stats[ STAT_STAMINA ] < 0 )
    {
      float deltaBreath = (float)(
        cg.predictedPlayerState.stats[ STAT_STAMINA ] < 0 ?
        -cg.predictedPlayerState.stats[ STAT_STAMINA ] :
        cg.predictedPlayerState.stats[ STAT_STAMINA ] ) / 200.0;
      float deltaAngle = cos( (float)cg.time/150.0 ) * deltaBreath;

      deltaAngle += ( deltaAngle < 0 ? -deltaAngle : deltaAngle ) * 0.5;

      angles[ PITCH ] -= deltaAngle;
    }
    //ZdrytchX: Personal view bug fix hack [see youtube.com/zdrytchx: mdriver false bullet physics test, you'll see how annoying it is in sniping, turns out the correction value for myself is approx -1.19 degrees]
    if( cg_firstpersonanglefix_yaw.value != 0)
    {
        if( cg_firstpersonanglefix_yaw.value > 20) //TODO: make sure we don't cheat here! (doesn't work)
        trap_Cvar_Set( "cg_firstpersonanglefix_yaw.value", "20" );
        else if( cg_firstpersonanglefix_yaw.value < -20)
        trap_Cvar_Set( "cg_firstpersonanglefix_yaw.value", "-20" );
        //Set angle fix! 
        angles[ YAW ] += cg_firstpersonanglefix_yaw.value; //left is positive (backward people's tradition)
    }
    if( cg_firstpersonanglefix_pitch.value != 0)
    {
        if( cg_firstpersonanglefix_pitch.value > 20) //TODO: make sure we don't cheat here! (doesn't work)
        trap_Cvar_Set( "cg_firstpersonanglefix_pitch.value", "20" );
        else if( cg_firstpersonanglefix_pitch.value < -20)
        trap_Cvar_Set( "cg_firstpersonanglefix_pitch.value", "-20" );
        //Set angle fix! 
        angles[ PITCH ] -= cg_firstpersonanglefix_pitch.value; //up is positive
    }
    //TODO: Oh geez, let's ask mr./d/hc again.
  }

//===================================

  // add view height
  // when wall climbing the viewheight is not straight up
  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBING )
    VectorMA( origin, ps->viewheight, normal, origin );
  else
    origin[ 2 ] += cg.predictedPlayerState.viewheight;

  // smooth out duck height changes
  timeDelta = cg.time - cg.duckTime;
  if( timeDelta < DUCK_TIME)
  {
    cg.refdef.vieworg[ 2 ] -= cg.duckChange
      * ( DUCK_TIME - timeDelta ) / DUCK_TIME;
  }

  // add bob height
  bob = cg.bobfracsin * cg.xyspeed * bob2;

  if( bob > 6 )
    bob = 6;

  // likewise for bob
  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_WALLCLIMBING )
    VectorMA( origin, bob, normal, origin );
  else
    origin[ 2 ] += bob;


  // add fall height
  delta = cg.time - cg.landTime;

  if( delta < LAND_DEFLECT_TIME )
  {
    f = delta / LAND_DEFLECT_TIME;
    cg.refdef.vieworg[ 2 ] += cg.landChange * f;
  }
  else if( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME )
  {
    delta -= LAND_DEFLECT_TIME;
    f = 1.0 - ( delta / LAND_RETURN_TIME );
    cg.refdef.vieworg[ 2 ] += cg.landChange * f;
  }

  // add step offset
  CG_StepOffset( );

  // add kick offset

  VectorAdd (origin, cg.kick_origin, origin);
}

//======================================================================

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define WAVE_AMPLITUDE  1
#define WAVE_FREQUENCY  0.4

#define FOVWARPTIME     400.0

static int CG_CalcFov( void )
{
  float     x;
  float     phase;
  float     v;
  int       contents;
  float     fov_x, fov_y;
  float     zoomFov;
  float     f;
  int       inwater;
  int       attribFov;
  usercmd_t cmd;
  int       cmdNum;

  cmdNum = trap_GetCurrentCmdNumber( );
  trap_GetUserCmd( cmdNum, &cmd );

  if( cg.predictedPlayerState.pm_type == PM_INTERMISSION ||
      ( cg.snap->ps.persistant[ PERS_TEAM ] == TEAM_SPECTATOR ) )
  {
    // if in intermission, use a fixed value
    fov_x = 90;
  }
  else
  {
    // don't lock the fov globally - we need to be able to change it
    attribFov = BG_FindFovForClass( cg.predictedPlayerState.stats[ STAT_PCLASS ] );
    fov_x = attribFov;

    if ( fov_x < 1 )
      fov_x = 1;
    else if ( fov_x > 160 )
      fov_x = 160;

    if( cg.spawnTime > ( cg.time - FOVWARPTIME ) &&
        BG_ClassHasAbility( cg.predictedPlayerState.stats[ STAT_PCLASS ], SCA_FOVWARPS ) )
    {
      float temp, temp2;

      temp = (float)( cg.time - cg.spawnTime ) / FOVWARPTIME;
      temp2 = ( 170 - fov_x ) * temp;

      //Com_Printf( "%f %f\n", temp*100, temp2*100 );

      fov_x = 170 - temp2;
    }

    // account for zooms
    zoomFov = BG_FindZoomFovForWeapon( cg.predictedPlayerState.weapon );
    if ( zoomFov < 1 )
      zoomFov = 1;
    else if ( zoomFov > attribFov )
      zoomFov = attribFov;

    // only do all the zoom stuff if the client CAN zoom
    // FIXME: zoom control is currently hard coded to BUTTON_ATTACK2
    if( BG_WeaponCanZoom( cg.predictedPlayerState.weapon ) )
    {
      if ( cg.zoomed )
      {
        f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;

        if ( f > 1.0 )
          fov_x = zoomFov;
        else
          fov_x = fov_x + f * ( zoomFov - fov_x );

        // BUTTON_ATTACK2 isn't held so unzoom next time
        if( !( cmd.buttons & BUTTON_ATTACK2 ) )
        {
          cg.zoomed   = qfalse;
          cg.zoomTime = cg.time;
        }
      }
      else
      {
        f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;

        if ( f > 1.0 )
          fov_x = fov_x;
        else
          fov_x = zoomFov + f * ( fov_x - zoomFov );

        // BUTTON_ATTACK2 is held so zoom next time
        if( cmd.buttons & BUTTON_ATTACK2 )
        {
          cg.zoomed   = qtrue;
          cg.zoomTime = cg.time;
        }
      }
    }
  }

  x = cg.refdef.width / tan( fov_x / 360 * M_PI );
  fov_y = atan2( cg.refdef.height, x );
  fov_y = fov_y * 360 / M_PI;

  // warp if underwater
  contents = CG_PointContents( cg.refdef.vieworg, -1 );

  if( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) )
  {
    phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
    v = WAVE_AMPLITUDE * sin( phase );
    fov_x += v;
    fov_y -= v;
    inwater = qtrue;
  }
  else
    inwater = qfalse;

  if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_POISONCLOUDED &&
      cg.predictedPlayerState.stats[ STAT_HEALTH ] > 0 &&
      !( cg.snap->ps.pm_flags & PMF_FOLLOW ) )
  {
    phase = cg.time / 1000.0 * PCLOUD_ZOOM_FREQUENCY * M_PI * 2;
    v = PCLOUD_ZOOM_AMPLITUDE * sin( phase );
    v *= 1.0f - ( ( cg.time - cg.poisonedTime ) / (float)LEVEL1_PCLOUD_TIME );
    fov_x += v;
    fov_y += v;
  }


  // set it
  cg.refdef.fov_x = fov_x;
  cg.refdef.fov_y = fov_y;

  if( !cg.zoomed )
  //zdrytchx: for some reason sensitivity is fov-dependant?
    cg.zoomSensitivity = (7 + (90/cg.refdef.fov_y))/8;//1;
  else
    cg.zoomSensitivity = cg.refdef.fov_y / 75.0;

  return inwater;
}



#define NORMAL_HEIGHT 64.0f
#define NORMAL_WIDTH  6.0f

/*
===============
CG_DrawSurfNormal

Draws a vector against
the surface player is looking at
===============
*/
static void CG_DrawSurfNormal( void )
{
  trace_t     tr;
  vec3_t      end, temp;
  polyVert_t  normal[ 4 ];
  vec4_t      color = { 0.0f, 255.0f, 0.0f, 128.0f };

  VectorMA( cg.refdef.vieworg, 8192, cg.refdef.viewaxis[ 0 ], end );

  CG_Trace( &tr, cg.refdef.vieworg, NULL, NULL, end, cg.predictedPlayerState.clientNum, MASK_SOLID );

  VectorCopy( tr.endpos, normal[ 0 ].xyz );
  normal[ 0 ].st[ 0 ] = 0;
  normal[ 0 ].st[ 1 ] = 0;
  Vector4Copy( color, normal[ 0 ].modulate );

  VectorMA( tr.endpos, NORMAL_WIDTH, cg.refdef.viewaxis[ 1 ], temp );
  VectorCopy( temp, normal[ 1 ].xyz);
  normal[ 1 ].st[ 0 ] = 0;
  normal[ 1 ].st[ 1 ] = 1;
  Vector4Copy( color, normal[ 1 ].modulate );

  VectorMA( tr.endpos, NORMAL_HEIGHT, tr.plane.normal, temp );
  VectorMA( temp, NORMAL_WIDTH, cg.refdef.viewaxis[ 1 ], temp );
  VectorCopy( temp, normal[ 2 ].xyz );
  normal[ 2 ].st[ 0 ] = 1;
  normal[ 2 ].st[ 1 ] = 1;
  Vector4Copy( color, normal[ 2 ].modulate );

  VectorMA( tr.endpos, NORMAL_HEIGHT, tr.plane.normal, temp );
  VectorCopy( temp, normal[ 3 ].xyz );
  normal[ 3 ].st[ 0 ] = 1;
  normal[ 3 ].st[ 1 ] = 0;
  Vector4Copy( color, normal[ 3 ].modulate );

  trap_R_AddPolyToScene( cgs.media.outlineShader, 4, normal );
}

/*
===============
CG_addSmoothOp
===============
*/
void CG_addSmoothOp( vec3_t rotAxis, float rotAngle, float timeMod )
{
  int i;

  //iterate through smooth array
  for( i = 0; i < MAXSMOOTHS; i++ )
  {
    //found an unused index in the smooth array
    if( cg.sList[ i ].time + cg_wwSmoothTime.integer < cg.time )
    {
      //copy to array and stop
      VectorCopy( rotAxis, cg.sList[ i ].rotAxis );
      cg.sList[ i ].rotAngle = rotAngle;
      cg.sList[ i ].time = cg.time;
      cg.sList[ i ].timeMod = timeMod;
      return;
    }
  }

  //no free indices in the smooth array
}

/*
===============
CG_smoothWWTransitions
===============
*/
static void CG_smoothWWTransitions( playerState_t *ps, const vec3_t in, vec3_t out )
{
  vec3_t    surfNormal, rotAxis, temp;
  vec3_t    refNormal     = { 0.0f, 0.0f,  1.0f };
  vec3_t    ceilingNormal = { 0.0f, 0.0f, -1.0f };
  int       i;
  float     stLocal, sFraction, rotAngle;
  float     smoothTime, timeMod;
  qboolean  performed = qfalse;
  vec3_t    inAxis[ 3 ], lastAxis[ 3 ], outAxis[ 3 ];

  if( cg.snap->ps.pm_flags & PMF_FOLLOW )
  {
    VectorCopy( in, out );
    return;
  }

  //set surfNormal
  if( !( ps->stats[ STAT_STATE ] & SS_WALLCLIMBINGCEILING ) )
    VectorCopy( ps->grapplePoint, surfNormal );
  else
    VectorCopy( ceilingNormal, surfNormal );

  AnglesToAxis( in, inAxis );

  //if we are moving from one surface to another smooth the transition
  if( !VectorCompare( surfNormal, cg.lastNormal ) )
  {
    //if we moving from the ceiling to the floor special case
    //( x product of colinear vectors is undefined)
    if( VectorCompare( ceilingNormal, cg.lastNormal ) &&
        VectorCompare( refNormal,     surfNormal ) )
    {
      AngleVectors( in, temp, NULL, NULL );
      ProjectPointOnPlane( rotAxis, temp, refNormal );
      VectorNormalize( rotAxis );
      rotAngle = 180.0f;
      timeMod = 1.5f;
    }
    else
    {
      AnglesToAxis( cg.lastVangles, lastAxis );
      rotAngle = DotProduct( inAxis[ 0 ], lastAxis[ 0 ] ) +
                 DotProduct( inAxis[ 1 ], lastAxis[ 1 ] ) +
                 DotProduct( inAxis[ 2 ], lastAxis[ 2 ] );

      rotAngle = RAD2DEG( acos( ( rotAngle - 1.0f ) / 2.0f ) );

      CrossProduct( lastAxis[ 0 ], inAxis[ 0 ], temp );
      VectorCopy( temp, rotAxis );
      CrossProduct( lastAxis[ 1 ], inAxis[ 1 ], temp );
      VectorAdd( rotAxis, temp, rotAxis );
      CrossProduct( lastAxis[ 2 ], inAxis[ 2 ], temp );
      VectorAdd( rotAxis, temp, rotAxis );

      VectorNormalize( rotAxis );

      timeMod = 1.0f;
    }

    //add the op
    CG_addSmoothOp( rotAxis, rotAngle, timeMod );
  }

  //iterate through ops
  for( i = MAXSMOOTHS - 1; i >= 0; i-- )
  {
    smoothTime = (int)( cg_wwSmoothTime.integer * cg.sList[ i ].timeMod );

    //if this op has time remaining, perform it
    if( cg.time < cg.sList[ i ].time + smoothTime )
    {
      stLocal = 1.0f - ( ( ( cg.sList[ i ].time + smoothTime ) - cg.time ) / smoothTime );
      sFraction = -( cos( stLocal * M_PI ) + 1.0f ) / 2.0f;

      RotatePointAroundVector( outAxis[ 0 ], cg.sList[ i ].rotAxis,
        inAxis[ 0 ], sFraction * cg.sList[ i ].rotAngle );
      RotatePointAroundVector( outAxis[ 1 ], cg.sList[ i ].rotAxis,
        inAxis[ 1 ], sFraction * cg.sList[ i ].rotAngle );
      RotatePointAroundVector( outAxis[ 2 ], cg.sList[ i ].rotAxis,
        inAxis[ 2 ], sFraction * cg.sList[ i ].rotAngle );

      AxisCopy( outAxis, inAxis );
      performed = qtrue;
    }
  }

  //if we performed any ops then return the smoothed angles
  //otherwise simply return the in angles
  if( performed )
    AxisToAngles( outAxis, out );
  else
    VectorCopy( in, out );

  //copy the current normal to the lastNormal
  VectorCopy( in, cg.lastVangles );
  VectorCopy( surfNormal, cg.lastNormal );
}

/*
===============
CG_smoothWJTransitions
===============
*/
static void CG_smoothWJTransitions( playerState_t *ps, const vec3_t in, vec3_t out )
{
  int       i;
  float     stLocal, sFraction;
  qboolean  performed = qfalse;
  vec3_t    inAxis[ 3 ], outAxis[ 3 ];

  if( cg.snap->ps.pm_flags & PMF_FOLLOW )
  {
    VectorCopy( in, out );
    return;
  }

  AnglesToAxis( in, inAxis );

  //iterate through ops
  for( i = MAXSMOOTHS - 1; i >= 0; i-- )
  {
    //if this op has time remaining, perform it
    if( cg.time < cg.sList[ i ].time + cg_wwSmoothTime.integer )
    {
      stLocal = ( ( cg.sList[ i ].time + cg_wwSmoothTime.integer ) - cg.time ) / cg_wwSmoothTime.integer;
      sFraction = 1.0f - ( ( cos( stLocal * M_PI * 2.0f ) + 1.0f ) / 2.0f );

      RotatePointAroundVector( outAxis[ 0 ], cg.sList[ i ].rotAxis,
        inAxis[ 0 ], sFraction * cg.sList[ i ].rotAngle );
      RotatePointAroundVector( outAxis[ 1 ], cg.sList[ i ].rotAxis,
        inAxis[ 1 ], sFraction * cg.sList[ i ].rotAngle );
      RotatePointAroundVector( outAxis[ 2 ], cg.sList[ i ].rotAxis,
        inAxis[ 2 ], sFraction * cg.sList[ i ].rotAngle );

      AxisCopy( outAxis, inAxis );
      performed = qtrue;
    }
  }

  //if we performed any ops then return the smoothed angles
  //otherwise simply return the in angles
  if( performed )
    AxisToAngles( outAxis, out );
  else
    VectorCopy( in, out );
}


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void )
{
  playerState_t *ps;

  memset( &cg.refdef, 0, sizeof( cg.refdef ) );

  // calculate size of 3D view
  CG_CalcVrect( );

  ps = &cg.predictedPlayerState;

  // intermission view
  if( ps->pm_type == PM_INTERMISSION )
  {
    VectorCopy( ps->origin, cg.refdef.vieworg );
    VectorCopy( ps->viewangles, cg.refdefViewAngles );
    AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

    return CG_CalcFov( );
  }

  cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
  cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
  cg.xyspeed = sqrt( ps->velocity[ 0 ] * ps->velocity[ 0 ] +
    ps->velocity[ 1 ] * ps->velocity[ 1 ] );//use for speedometer?

  VectorCopy( ps->origin, cg.refdef.vieworg );

  if( BG_ClassHasAbility( ps->stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) )
    CG_smoothWWTransitions( ps, ps->viewangles, cg.refdefViewAngles );
  else if( BG_ClassHasAbility( ps->stats[ STAT_PCLASS ], SCA_WALLJUMPER ) )
    CG_smoothWJTransitions( ps, ps->viewangles, cg.refdefViewAngles );
  else
    VectorCopy( ps->viewangles, cg.refdefViewAngles );

  //clumsy logic, but it needs to be this way round because the CS propogation
  //delay screws things up otherwise
  if( !BG_ClassHasAbility( ps->stats[ STAT_PCLASS ], SCA_WALLJUMPER ) )
  {
    if( !( ps->stats[ STAT_STATE ] & SS_WALLCLIMBING ) )
      VectorSet( cg.lastNormal, 0.0f, 0.0f, 1.0f );
  }

  // add error decay
  if( cg_errorDecay.value > 0 )
  {
    int   t;
    float f;

    t = cg.time - cg.predictedErrorTime;
    f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;

    if( f > 0 && f < 1 )
      VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
    else
      cg.predictedErrorTime = 0;
  }

  //shut off the poison cloud effect if it's still on the go
  if( cg.snap->ps.stats[ STAT_HEALTH ] <= 0 )
  {
    if( CG_IsParticleSystemValid( &cg.poisonCloudPS ) )
      CG_DestroyParticleSystem( &cg.poisonCloudPS );
  }

  if( cg.renderingThirdPerson )
  {
    // back away from character
    CG_OffsetThirdPersonView( );
  }
  else
  {
    // offset for local bobbing and kicks
    CG_OffsetFirstPersonView( );
  }

  // position eye reletive to origin
  AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

  if( cg.hyperspace )
    cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;

  //draw the surface normal looking at
  if( cg_drawSurfNormal.integer )
    CG_DrawSurfNormal( );

  // field of view
  return CG_CalcFov( );
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx )
{
  if( !sfx )
    return;

  cg.soundBuffer[ cg.soundBufferIn ] = sfx;
  cg.soundBufferIn = ( cg.soundBufferIn + 1 ) % MAX_SOUNDBUFFER;

  if( cg.soundBufferIn == cg.soundBufferOut )
    cg.soundBufferOut++;
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void )
{
  if( cg.soundTime < cg.time )
  {
    if( cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[ cg.soundBufferOut ] )
    {
      trap_S_StartLocalSound( cg.soundBuffer[ cg.soundBufferOut ], CHAN_ANNOUNCER );
      cg.soundBuffer[ cg.soundBufferOut ] = 0;
      cg.soundBufferOut = ( cg.soundBufferOut + 1 ) % MAX_SOUNDBUFFER;
      cg.soundTime = cg.time + 750;
    }
  }
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback )
{
  int   inwater;

  cg.time = serverTime;
  cg.demoPlayback = demoPlayback;

  // update cvars
  CG_UpdateCvars( );

  // if we are only updating the screen as a loading
  // pacifier, don't even try to read snapshots
  if( cg.infoScreenText[ 0 ] != 0 )
  {
    CG_DrawLoadingScreen( );
    return;
  }

  // any looped sounds will be respecified as entities
  // are added to the render list
  trap_S_ClearLoopingSounds( qfalse );

  // clear all the render lists
  trap_R_ClearScene( );

  // set up cg.snap and possibly cg.nextSnap
  CG_ProcessSnapshots( );

  // if we haven't received any snapshots yet, all
  // we can draw is the information screen
  if( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) )
  {
    CG_DrawLoadingScreen( );
    return;
  }

  // let the client system know what our weapon and zoom settings are
  trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

  // this counter will be bumped for every valid scene we generate
  cg.clientFrame++;

  // update cg.predictedPlayerState
  CG_PredictPlayerState( );

  // decide on third person view
  cg.renderingThirdPerson = cg_thirdPerson.integer || ( cg.snap->ps.stats[ STAT_HEALTH ] <= 0 );

  // build cg.refdef
  inwater = CG_CalcViewValues( );

  // build the render lists
  if( !cg.hyperspace )
  {
    CG_AddPacketEntities( );     // after calcViewValues, so predicted player state is correct
    CG_AddMarks( );
  }

  CG_AddViewWeapon( &cg.predictedPlayerState );

  //after CG_AddViewWeapon
  if( !cg.hyperspace )
  {
    CG_AddParticles( );
    CG_AddTrails( );
  }

  // add buffered sounds
  CG_PlayBufferedSounds( );

  // finish up the rest of the refdef
  if( cg.testModelEntity.hModel )
    CG_AddTestModel( );

  cg.refdef.time = cg.time;
  memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

  //remove expired console lines
  if( cg.consoleLines[ 0 ].time + cg_consoleLatency.integer < cg.time && cg_consoleLatency.integer > 0 )
    CG_RemoveNotifyLine( );

  // update audio positions
  trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

  // make sure the lagometerSample and frame timing isn't done twice when in stereo
  if( stereoView != STEREO_RIGHT )
  {
    cg.frametime = cg.time - cg.oldTime;

    if( cg.frametime < 0 )
      cg.frametime = 0;

    cg.oldTime = cg.time;
    CG_AddLagometerFrameInfo( );
  }

  if( cg_timescale.value != cg_timescaleFadeEnd.value )
  {
    if( cg_timescale.value < cg_timescaleFadeEnd.value )
    {
      cg_timescale.value += cg_timescaleFadeSpeed.value * ( (float)cg.frametime ) / 1000;
      if( cg_timescale.value > cg_timescaleFadeEnd.value )
        cg_timescale.value = cg_timescaleFadeEnd.value;
    }
    else
    {
      cg_timescale.value -= cg_timescaleFadeSpeed.value * ( (float)cg.frametime ) / 1000;
      if( cg_timescale.value < cg_timescaleFadeEnd.value )
        cg_timescale.value = cg_timescaleFadeEnd.value;
    }

    if( cg_timescaleFadeSpeed.value )
      trap_Cvar_Set( "timescale", va( "%f", cg_timescale.value ) );
  }

  // actually issue the rendering calls
  CG_DrawActive( stereoView );

  if( cg_stats.integer )
    CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
}

