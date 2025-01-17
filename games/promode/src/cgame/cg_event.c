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

// cg_event.c -- handle entity events at snapshot or playerstate transitions


#include "cg_local.h"

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent )
{
  int           mod;
  int           target, attacker;
  char          *message;
  char          *message2;
  const char    *targetInfo;
  const char    *attackerInfo;
  char          targetName[ 32 ];
  char          attackerName[ 32 ];
  char          className[ 64 ];
  gender_t      gender;
  clientInfo_t  *ci;
  qboolean      teamKill = qfalse;

  target = ent->otherEntityNum;
  attacker = ent->otherEntityNum2;
  mod = ent->eventParm;

  if( target < 0 || target >= MAX_CLIENTS )
    CG_Error( "CG_Obituary: target out of range" );

  ci = &cgs.clientinfo[ target ];

  if( attacker < 0 || attacker >= MAX_CLIENTS )
  {
    attacker = ENTITYNUM_WORLD;
    attackerInfo = NULL;
  }
  else
  {
    attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
    if( ci && cgs.clientinfo[ attacker ].team == ci->team )
      teamKill = qtrue;
  }

  targetInfo = CG_ConfigString( CS_PLAYERS + target );

  if( !targetInfo )
    return;

  Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof( targetName ) - 2 );
  strcat( targetName, S_COLOR_WHITE );

  message2 = "";

  // check for single client messages

  switch( mod )
  {
    case MOD_SUICIDE:
      message = "just wanted to reincarnate via extreme pain";
      break;
    case MOD_FALLING:
      message = "bounced to heaven using the Eq: F = mg";
      break;
    case MOD_CRUSH:
      message = "can now fit in a filing cabinet";
      break;
    case MOD_WATER:
      message = "didn't know how to swim properly";
      break;
    case MOD_SLIME:
      message = "thought ^2acid^7 tasted good";
      break;
    case MOD_LAVA:
      message = "melted into hot slag";
      break;
    case MOD_TARGET_LASER: //used by lockblob now
      message = "almost dodged a vicious projectile";
      break;
    case MOD_TRIGGER_HURT:
      message = "must've died somehow";
      break;
    case MOD_HSPAWN:
      message = "was killed in some explosion.";
      break;
    case MOD_ASPAWN:
      message = "shouldn't have stayed near that organic structure's remains";
      break;
    case MOD_MGTURRET:
      message = "was shot down by a chainret";
      break;
    case MOD_TESLAGEN:
      message = "had a deadly electrical massage";
      break;
    case MOD_ATUBE:
      message = "went near the baby tubes to shower off his dead skin";
      break;
    case MOD_OVERMIND:
      message = "shouldn't have and couldn't convince the ^4Overmind^7 for an intercourse";
      break;
    case MOD_REACTOR:
      message = "didn't know what the ^4product^7 from an ^5electricity ^7generator was";
      break;
    case MOD_SLOWBLOB:
      message = "failed to make love with the ^2goo";
      break;
    case MOD_SWARM:
      message = "shouldn't have approached that hive";
      break;
    default:
      message = NULL;
      break;
  }

  if( attacker == target )
  {
    gender = ci->gender;
    switch( mod )
    {
      case MOD_FLAMER_SPLASH:
        if( gender == GENDER_FEMALE )
          message = "thought this was 1.2 tremulous.";
        else if( gender == GENDER_NEUTER )
          message = "toasted oneself";
        else
          message = "tried to make barbeque";
        break;

      case MOD_LCANNON_SPLASH:
        if( gender == GENDER_FEMALE )
          message = "should've learned not to luci spam";
        else if( gender == GENDER_NEUTER )
          message = "tried a luci jump";
        else
          message = "'s body blew up into a magnificent firework display"; //tried a luci jump
        break;

      case MOD_GRENADE:
        if( gender == GENDER_FEMALE )
          message = "had her nade betray her";
        else if( gender == GENDER_NEUTER )
          message = "blew itself up";
        else
          message = "threw the pin instead of the grenade";
        break;

      default:
        if( gender == GENDER_FEMALE )
          message = "suicided";
        else if( gender == GENDER_NEUTER )
          message = "^1died.";
        else
          message = "had to get away from the war";
        break;
    }
  }

  if( message )
  {
    CG_Printf( "%s %s.\n", targetName, message );
    return;
  }

  // check for double client messages
  if( !attackerInfo )
  {
    attacker = ENTITYNUM_WORLD;
    strcpy( attackerName, "noname" );
  }
  else
  {
    Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof( attackerName ) - 2);
    strcat( attackerName, S_COLOR_WHITE );
    // check for kill messages about the current clientNum
    if( target == cg.snap->ps.clientNum )
      Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
  }

  if( attacker != ENTITYNUM_WORLD )
  {
    switch( mod )
    {
      case MOD_TARGET_LASER:
        message = "was fked up by";
	message2 = "'s sticky glue";
        break;
      case MOD_SLOWBLOB:
        message = "shouldn't have messed with";
        message2 = "'s ^2granger goo";
        break;
      case MOD_PAINSAW:
        message = "was scratched into peices by";
        message2 = "'s ^5p^7eni^5s^7aw";
        break;
      case MOD_BLASTER:
        message = "was slowly =Powned with beauty by";
        message2 = "'s energy pistol slug";
        break;
      case MOD_MACHINEGUN:
        message = "was exterminated by";
        message2 = "'s bullet storm";
        break;
      case MOD_CHAINGUN:
        message = "was easily mowed down by";
        message2 = "'s combat gatling gun";
        break;
      case MOD_SHOTGUN:
        message = "'s body suffered the shock of the pellets from";
        message2 = "'s shotgun";
        break;
      case MOD_PRIFLE:
        message = "couldn't run from";
        message2 = "'s ^1plasma^7";
        break;
      case MOD_MDRIVER:
        message = "learnt a lesson in nuclear physics from le Prof.";
        message2 = "'s particle acceleration device";
        break;
      case MOD_LASGUN:
        message = "was spammed to gibs by";
        message2 = "'s lasgun";
        break;
      case MOD_FLAMER:
        message = "was ^1baked^7 by";
        message2 = "'s ^1p^3ortable ^1i^3ncinerater";
        break;
      case MOD_FLAMER_SPLASH:
        message = "was unintentionally caught in the flame of";
        message2 = "'s grill";
        break;
      case MOD_LCANNON:
        message = "saw";
        message2 = "'s pretty ^3lights ^7come too close";
        break;
      case MOD_LCANNON_SPLASH:
        message = "was blown up by the vicinity of";
        message2 = "'s ^3exploding sun";
        break;
      case MOD_GRENADE:
        message = "didn't see";
        message2 = " drop the bottle";
        break;

      case MOD_ABUILDER_CLAW:
        message = "was fully owned by";
        message2 = "'s ^2granger";
        break;
      case MOD_LEVEL0_BITE:
        message = "was nibbled by";
	message2 = "'s ^3dretch";
        break;
      case MOD_LEVEL1_CLAW:
        message = "was assasinated slowly and gracefully by";
//Re-enabled as spy-suit nolonger uses level1upg
        Com_sprintf( className, 64, "'s %s",
            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL1 ) );
        message2 = className;
        break;
      case MOD_LEVEL2_CLAW:
        message = "had noticed his body parts suddenly went missing as";
//        Com_sprintf( className, 64, "'s %s",
//            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL2 ) );
//        message2 = className;
        message2 = " attempted to closen their ^3distance";
        break;
      case MOD_LEVEL2_ZAP:
        message = "'s life energy was drained by";
/*
        Com_sprintf( className, 64, "'s %s",
            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL2 ) );
*/
        message2 = "'s blue light beam";
        break;
      case MOD_LEVEL3_CLAW:
        message = "had been decapitated by the awe of";
        Com_sprintf( className, 64, "'s %s",
            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL3 ) );
        message2 = className;
        break;
      case MOD_LEVEL3_POUNCE:
        message = "had his lovely face munted as";
        Com_sprintf( className, 64, "'s %s",
            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL3 ) );
        message2 = "'s dragoon flew past";
        break;
      case MOD_LEVEL3_BOUNCEBALL:
        message = "almost dodged";
        Com_sprintf( className, 64, "'s %s",
            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL3 ) );
        message2 = "'s acid barb";//className;
        break;
      case MOD_LEVEL4_CLAW:
        message = "was filleted by";
//        Com_sprintf( className, 64, "'s %s",
//            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL4 ) );
        message2 = "'s Tyrant";
        break;
      case MOD_LEVEL4_CHARGE:
        message = " was mowed down when he heard";
//        Com_sprintf( className, 64, "'s %s",
//            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL4 ) );
        message2 = "'s RaWr";
        break;

      case MOD_POISON:
        message = "had became unfortunate from";
        message2 = "'s AIDS";
        break;
      case MOD_LEVEL1_PCLOUD:
        message = "was drugged by";
//        Com_sprintf( className, 64, "'s %s",
//            BG_FindHumanNameForClassNum( PCL_ALIEN_LEVEL1 ) );
        message2 = "'s basi ^1love ^2gas";
        break;


      case MOD_TELEFRAG:
        message = "should've moved from the teleporter when";
        message2 = " arrived";
        break;
      default:
        message = "was murdered by";
        break;
    }

    if( message )
    {
      CG_Printf( "%s %s %s%s%s\n",
        targetName, message,
        ( teamKill ) ? S_COLOR_RED "TEAMMATE " S_COLOR_WHITE : "",
        attackerName, message2 );
      if( teamKill && attacker == cg.clientNum )
      {
        CG_CenterPrint( va ( "M8,  You broke the law by " S_COLOR_RED "RAPING TEAMMATE "
          S_COLOR_WHITE "%s", targetName ),
          SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
      }
      return;
    }
  }

  // we don't know what it was
  CG_Printf( "%s pressed a red button on his ipad's screen.\n", targetName );
}

//==========================================================================

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health )
{
  char  *snd;

  // don't do more than two pain sounds a second
  if( cg.time - cent->pe.painTime < 500 )
    return;

  if( health < 25 )
    snd = "*pain25_1.wav";
  else if( health < 50 )
    snd = "*pain50_1.wav";
  else if( health < 75 )
    snd = "*pain75_1.wav";
  else
    snd = "*pain100_1.wav";

  trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE,
    CG_CustomSound( cent->currentState.number, snd ) );

  // save pain time for programitic twitch animation
  cent->pe.painTime = cg.time;
  cent->pe.painDirection ^= 1;
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position )
{
  entityState_t *es;
  int           event;
  vec3_t        dir;
  const char    *s;
  int           clientNum;
  clientInfo_t  *ci;
  int           steptime;

  if( cg.snap->ps.persistant[ PERS_TEAM ] == TEAM_SPECTATOR )
    steptime = 200;
  else
    steptime = BG_FindSteptimeForClass( cg.snap->ps.stats[ STAT_PCLASS ] );

  es = &cent->currentState;
  event = es->event & ~EV_EVENT_BITS;

  if( cg_debugEvents.integer )
    CG_Printf( "ent:%3i  event:%3i ", es->number, event );

  if( !event )
  {
    DEBUGNAME("ZEROEVENT");
    return;
  }

  clientNum = es->clientNum;
  if( clientNum < 0 || clientNum >= MAX_CLIENTS )
    clientNum = 0;

  ci = &cgs.clientinfo[ clientNum ];

  switch( event )
  {
    //
    // movement generated events
    //
    case EV_FOOTSTEP:
      DEBUGNAME( "EV_FOOTSTEP" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        if( ci->footsteps == FOOTSTEP_CUSTOM )
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            ci->customFootsteps[ rand( ) & 3 ] );
        else
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            cgs.media.footsteps[ ci->footsteps ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSTEP_METAL:
      DEBUGNAME( "EV_FOOTSTEP_METAL" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        if( ci->footsteps == FOOTSTEP_CUSTOM )
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            ci->customMetalFootsteps[ rand( ) & 3 ] );
        else
          trap_S_StartSound( NULL, es->number, CHAN_BODY,
            cgs.media.footsteps[ FOOTSTEP_METAL ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSTEP_SQUELCH:
      DEBUGNAME( "EV_FOOTSTEP_SQUELCH" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_FLESH ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTSPLASH:
      DEBUGNAME( "EV_FOOTSPLASH" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;

    case EV_FOOTWADE:
      DEBUGNAME( "EV_FOOTWADE" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;

    case EV_SWIM:
      DEBUGNAME( "EV_SWIM" );
      if( cg_footsteps.integer && ci->footsteps != FOOTSTEP_NONE )
      {
        trap_S_StartSound( NULL, es->number, CHAN_BODY,
          cgs.media.footsteps[ FOOTSTEP_SPLASH ][ rand( ) & 3 ] );
      }
      break;


    case EV_FALL_SHORT:
      DEBUGNAME( "EV_FALL_SHORT" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.landSound );

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -8;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALL_MEDIUM:
      DEBUGNAME( "EV_FALL_MEDIUM" );
      // use normal pain sound
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -16;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALL_FAR:
      DEBUGNAME( "EV_FALL_FAR" );
      trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
      cent->pe.painTime = cg.time;  // don't play a pain sound right after this

      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        // smooth landing z changes
        cg.landChange = -24;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALLING:
      DEBUGNAME( "EV_FALLING" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*falling1.wav" ) );
      break;

    case EV_STEP_4:
    case EV_STEP_8:
    case EV_STEP_12:
    case EV_STEP_16:    // smooth out step up transitions
    case EV_STEPDN_4:
    case EV_STEPDN_8:
    case EV_STEPDN_12:
    case EV_STEPDN_16:    // smooth out step down transitions
      DEBUGNAME( "EV_STEP" );
      {
        float  oldStep;
        int    delta;
        int    step;

        if( clientNum != cg.predictedPlayerState.clientNum )
          break;

        // if we are interpolating, we don't need to smooth steps
        if( cg.demoPlayback || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) ||
            cg_nopredict.integer || cg_synchronousClients.integer )
          break;

        // check for stepping up before a previous step is completed
        delta = cg.time - cg.stepTime;

        if( delta < steptime )
          oldStep = cg.stepChange * ( steptime - delta ) / steptime;
        else
          oldStep = 0;

        // add this amount
        if( event >= EV_STEPDN_4 )
        {
          step = 4 * ( event - EV_STEPDN_4 + 1 );
          cg.stepChange = oldStep - step;
        }
        else
        {
          step = 4 * ( event - EV_STEP_4 + 1 );
          cg.stepChange = oldStep + step;
        }

        if( cg.stepChange > MAX_STEP_CHANGE )
          cg.stepChange = MAX_STEP_CHANGE;
        else if( cg.stepChange < -MAX_STEP_CHANGE )
          cg.stepChange = -MAX_STEP_CHANGE;

        cg.stepTime = cg.time;
        break;
      }

    case EV_JUMP:
      DEBUGNAME( "EV_JUMP" );
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );

      if( BG_ClassHasAbility( cg.predictedPlayerState.stats[ STAT_PCLASS ], SCA_WALLJUMPER ) )
      {
        vec3_t  surfNormal, refNormal = { 0.0f, 0.0f, 1.0f };
        vec3_t  rotAxis;

        if( clientNum != cg.predictedPlayerState.clientNum )
          break;

        //set surfNormal
        VectorCopy( cg.predictedPlayerState.grapplePoint, surfNormal );

        //if we are moving from one surface to another smooth the transition
        if( !VectorCompare( surfNormal, cg.lastNormal ) && surfNormal[ 2 ] != 1.0f )
        {
          CrossProduct( refNormal, surfNormal, rotAxis );
          VectorNormalize( rotAxis );

          //add the op
          CG_addSmoothOp( rotAxis, 15.0f, 1.0f );
        }

        //copy the current normal to the lastNormal
        VectorCopy( surfNormal, cg.lastNormal );
      }

      break;

    case EV_LEV1_GRAB:
      DEBUGNAME( "EV_LEV1_GRAB" );
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL1Grab );
      break;

    case EV_LEV4_CHARGE_PREPARE:
      DEBUGNAME( "EV_LEV4_CHARGE_PREPARE" );
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL4ChargePrepare );
      break;

    case EV_LEV4_CHARGE_START:
      DEBUGNAME( "EV_LEV4_CHARGE_START" );
      //FIXME: stop cgs.media.alienL4ChargePrepare playing here
      trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.alienL4ChargeStart );
      break;

    case EV_TAUNT:
      DEBUGNAME( "EV_TAUNT" );
      if( !cg_noTaunt.integer )
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
      break;

    case EV_WATER_TOUCH:
      DEBUGNAME( "EV_WATER_TOUCH" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
      break;

    case EV_WATER_LEAVE:
      DEBUGNAME( "EV_WATER_LEAVE" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
      break;

    case EV_WATER_UNDER:
      DEBUGNAME( "EV_WATER_UNDER" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
      break;

    case EV_WATER_CLEAR:
      DEBUGNAME( "EV_WATER_CLEAR" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
      break;

    //
    // weapon events
    //
    case EV_NOAMMO:
      DEBUGNAME( "EV_NOAMMO" );
      {
      }
      break;

    case EV_CHANGE_WEAPON:
      DEBUGNAME( "EV_CHANGE_WEAPON" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
      break;

    case EV_FIRE_WEAPON:
      DEBUGNAME( "EV_FIRE_WEAPON" );
      CG_FireWeapon( cent, WPM_PRIMARY );
      break;

    case EV_FIRE_WEAPON2:
      DEBUGNAME( "EV_FIRE_WEAPON2" );
      CG_FireWeapon( cent, WPM_SECONDARY );
      break;

    case EV_FIRE_WEAPON3:
      DEBUGNAME( "EV_FIRE_WEAPON3" );
      CG_FireWeapon( cent, WPM_TERTIARY );
      break;

    //=================================================================

    //
    // other events
    //
    case EV_PLAYER_TELEPORT_IN:
      DEBUGNAME( "EV_PLAYER_TELEPORT_IN" );
      //deprecated
      break;

    case EV_PLAYER_TELEPORT_OUT:
      DEBUGNAME( "EV_PLAYER_TELEPORT_OUT" );
      CG_PlayerDisconnect( position );
      break;

    case EV_BUILD_CONSTRUCT:
      DEBUGNAME( "EV_BUILD_CONSTRUCT" );
      //do something useful here
      break;

    case EV_BUILD_DESTROY:
      DEBUGNAME( "EV_BUILD_DESTROY" );
      //do something useful here
      break;

    case EV_RPTUSE_SOUND:
      DEBUGNAME( "EV_RPTUSE_SOUND" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.repeaterUseSound );
      break;

    case EV_GRENADE_BOUNCE:
      DEBUGNAME( "EV_GRENADE_BOUNCE" );
      if( rand( ) & 1 )
        trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.hardBounceSound1 );
      else
        trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.hardBounceSound2 );
      break;

    //
    // missile impacts
    //
    case EV_MISSILE_HIT:
      DEBUGNAME( "EV_MISSILE_HIT" );
      ByteToDir( es->eventParm, dir );
      CG_MissileHitPlayer( es->weapon, es->generic1, position, dir, es->otherEntityNum );
      break;

    case EV_MISSILE_MISS:
      DEBUGNAME( "EV_MISSILE_MISS" );
      ByteToDir( es->eventParm, dir );
      CG_MissileHitWall( es->weapon, es->generic1, 0, position, dir, IMPACTSOUND_DEFAULT );
      break;

    case EV_MISSILE_MISS_METAL:
      DEBUGNAME( "EV_MISSILE_MISS_METAL" );
      ByteToDir( es->eventParm, dir );
      CG_MissileHitWall( es->weapon, es->generic1, 0, position, dir, IMPACTSOUND_METAL );
      break;

    case EV_HUMAN_BUILDABLE_EXPLOSION:
      DEBUGNAME( "EV_HUMAN_BUILDABLE_EXPLOSION" );
      ByteToDir( es->eventParm, dir );
      CG_HumanBuildableExplosion( position, dir );
      break;

    case EV_ALIEN_BUILDABLE_EXPLOSION:
      DEBUGNAME( "EV_ALIEN_BUILDABLE_EXPLOSION" );
      ByteToDir( es->eventParm, dir );
      CG_AlienBuildableExplosion( position, dir );
      break;

    case EV_TESLATRAIL:
      DEBUGNAME( "EV_TESLATRAIL" );
      cent->currentState.weapon = WP_TESLAGEN;
      {
        centity_t *source = &cg_entities[ es->generic1 ];
        centity_t *target = &cg_entities[ es->clientNum ];
        vec3_t    sourceOffset = { 0.0f, 0.0f, 28.0f };

        if( !CG_IsTrailSystemValid( &source->muzzleTS ) )
        {
          source->muzzleTS = CG_SpawnNewTrailSystem( cgs.media.teslaZapTS );

          if( CG_IsTrailSystemValid( &source->muzzleTS ) )
          {
            CG_SetAttachmentCent( &source->muzzleTS->frontAttachment, source );
            CG_SetAttachmentCent( &source->muzzleTS->backAttachment, target );
            CG_AttachToCent( &source->muzzleTS->frontAttachment );
            CG_AttachToCent( &source->muzzleTS->backAttachment );
            CG_SetAttachmentOffset( &source->muzzleTS->frontAttachment, sourceOffset );

            source->muzzleTSDeathTime = cg.time + cg_teslaTrailTime.integer;
          }
        }
      }
      break;

    case EV_BULLET_HIT_WALL:
      DEBUGNAME( "EV_BULLET_HIT_WALL" );
      ByteToDir( es->eventParm, dir );
      CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
      break;

    case EV_BULLET_HIT_FLESH:
      DEBUGNAME( "EV_BULLET_HIT_FLESH" );
      CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
      break;

    case EV_SHOTGUN:
      DEBUGNAME( "EV_SHOTGUN" );
      CG_ShotgunFire( es );
      break;

    case EV_GENERAL_SOUND:
      DEBUGNAME( "EV_GENERAL_SOUND" );
      if( cgs.gameSounds[ es->eventParm ] )
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
      else
      {
        s = CG_ConfigString( CS_SOUNDS + es->eventParm );
        trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
      }
      break;

    case EV_GLOBAL_SOUND: // play from the player's head so it never diminishes
      DEBUGNAME( "EV_GLOBAL_SOUND" );
      if( cgs.gameSounds[ es->eventParm ] )
        trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
      else
      {
        s = CG_ConfigString( CS_SOUNDS + es->eventParm );
        trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
      }
      break;

    case EV_PAIN:
      // local player sounds are triggered in CG_CheckLocalSounds,
      // so ignore events on the player
      DEBUGNAME( "EV_PAIN" );
      if( cent->currentState.number != cg.snap->ps.clientNum )
        CG_PainEvent( cent, es->eventParm );
      break;

    case EV_DEATH1:
    case EV_DEATH2:
    case EV_DEATH3:
      DEBUGNAME( "EV_DEATHx" );
      trap_S_StartSound( NULL, es->number, CHAN_VOICE,
          CG_CustomSound( es->number, va( "*death%i.wav", event - EV_DEATH1 + 1 ) ) );
      break;

    case EV_OBITUARY:
      DEBUGNAME( "EV_OBITUARY" );
      CG_Obituary( es );
      break;

    case EV_GIB_PLAYER:
      DEBUGNAME( "EV_GIB_PLAYER" );
      // no gibbing
      break;

    case EV_STOPLOOPINGSOUND:
      DEBUGNAME( "EV_STOPLOOPINGSOUND" );
      trap_S_StopLoopingSound( es->number );
      es->loopSound = 0;
      break;

    case EV_DEBUG_LINE:
      DEBUGNAME( "EV_DEBUG_LINE" );
      CG_Beam( cent );
      break;

    case EV_BUILD_DELAY:
      DEBUGNAME( "EV_BUILD_DELAY" );
      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        trap_S_StartLocalSound( cgs.media.buildableRepairedSound, CHAN_LOCAL_SOUND );
        cg.lastBuildAttempt = cg.time;
      }
      break;

    case EV_BUILD_REPAIR:
      DEBUGNAME( "EV_BUILD_REPAIR" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.buildableRepairSound );
      break;

    case EV_BUILD_REPAIRED:
      DEBUGNAME( "EV_BUILD_REPAIRED" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.buildableRepairedSound );
      break;

    case EV_OVERMIND_ATTACK:
      DEBUGNAME( "EV_OVERMIND_ATTACK" );
      if( cg.predictedPlayerState.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindAttack, CHAN_ANNOUNCER );
        CG_CenterPrint( "Our Overmind is under attack!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_OVERMIND_DYING:
      DEBUGNAME( "EV_OVERMIND_DYING" );
      if( cg.predictedPlayerState.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindDying, CHAN_ANNOUNCER );
        CG_CenterPrint( "The Overmind is dying!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_DCC_ATTACK:
      DEBUGNAME( "EV_DCC_ATTACK" );
      if( cg.predictedPlayerState.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        //trap_S_StartLocalSound( cgs.media.humanDCCAttack, CHAN_ANNOUNCER );
        CG_CenterPrint( "Our base has been attacked!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_OVERMIND_SPAWNS:
      DEBUGNAME( "EV_OVERMIND_SPAWNS" );
      if( cg.predictedPlayerState.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        trap_S_StartLocalSound( cgs.media.alienOvermindSpawns, CHAN_ANNOUNCER );
        CG_CenterPrint( "The Overmind needs eggs!", 200, GIANTCHAR_WIDTH * 4 );
      }
      break;

    case EV_ALIEN_EVOLVE:
      DEBUGNAME( "EV_ALIEN_EVOLVE" );
      trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.alienEvolveSound );
      {
        particleSystem_t *ps = CG_SpawnNewParticleSystem( cgs.media.alienEvolvePS );

        if( CG_IsParticleSystemValid( &ps ) )
        {
          CG_SetAttachmentCent( &ps->attachment, cent );
          CG_AttachToCent( &ps->attachment );
        }
      }

      if( es->number == cg.clientNum )
      {
        CG_ResetPainBlend( );
        cg.spawnTime = cg.time;
      }
      break;

    case EV_ALIEN_EVOLVE_FAILED:
      DEBUGNAME( "EV_ALIEN_EVOLVE_FAILED" );
      if( clientNum == cg.predictedPlayerState.clientNum )
      {
        //FIXME: change to "negative" sound
        trap_S_StartLocalSound( cgs.media.buildableRepairedSound, CHAN_LOCAL_SOUND );
        cg.lastEvolveAttempt = cg.time;
      }
      break;

    case EV_ALIEN_ACIDTUBE:
      DEBUGNAME( "EV_ALIEN_ACIDTUBE" );
      {
        particleSystem_t *ps = CG_SpawnNewParticleSystem( cgs.media.alienAcidTubePS );

        if( CG_IsParticleSystemValid( &ps ) )
        {
          CG_SetAttachmentCent( &ps->attachment, cent );
          ByteToDir( es->eventParm, dir );
          CG_SetParticleSystemNormal( ps, dir );
          CG_AttachToCent( &ps->attachment );
        }
      }
      break;

    case EV_MEDKIT_USED:
      DEBUGNAME( "EV_MEDKIT_USED" );
      trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.medkitUseSound );
      break;

    case EV_PLAYER_RESPAWN:
      DEBUGNAME( "EV_PLAYER_RESPAWN" );
      if( es->number == cg.clientNum )
        cg.spawnTime = cg.time;
      break;

    default:
      DEBUGNAME( "UNKNOWN" );
      CG_Error( "Unknown event: %i", event );
      break;
  }
}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent )
{
  entity_event_t event;
  entity_event_t oldEvent = EV_NONE;

  // check for event-only entities
  if( cent->currentState.eType > ET_EVENTS )
  {
    event = cent->currentState.eType - ET_EVENTS;

    if( cent->previousEvent )
      return; // already fired

    cent->previousEvent = 1;

    cent->currentState.event = cent->currentState.eType - ET_EVENTS;
    
    // Move the pointer to the entity that the
    // event was originally attached to
    if( cent->currentState.eFlags & EF_PLAYER_EVENT )
    {
      cent = &cg_entities[ cent->currentState.otherEntityNum ];
      oldEvent = cent->currentState.event;
      cent->currentState.event = event;
    }
  }
  else
  {
    // check for events riding with another entity
    if( cent->currentState.event == cent->previousEvent )
      return;

    cent->previousEvent = cent->currentState.event;
    if( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 )
      return;
  }

  // calculate the position at exactly the frame time
  BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
  CG_SetEntitySoundPosition( cent );

  CG_EntityEvent( cent, cent->lerpOrigin );
  
  // If this was a reattached spilled event, restore the original event
  if( oldEvent != EV_NONE )
    cent->currentState.event = oldEvent;
}

