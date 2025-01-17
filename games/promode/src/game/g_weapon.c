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

// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"

static  vec3_t  forward, right, up;
static  vec3_t  muzzle;

/*
================
G_ForceWeaponChange
================
*/
void G_ForceWeaponChange( gentity_t *ent, weapon_t weapon )
{
  int i;

  if( ent )
  {
    ent->client->ps.pm_flags |= PMF_WEAPON_SWITCH;

    if( weapon == WP_NONE 
      || !BG_InventoryContainsWeapon( weapon, ent->client->ps.stats ))
    {
      //switch to the first non blaster weapon
      for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
      {
        if( i == WP_BLASTER )
          continue;

        if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) )
        {
          ent->client->ps.persistant[ PERS_NEWWEAPON ] = i;
          break;
        }
      }

      //only got the blaster to switch to
      if( i == WP_NUM_WEAPONS )
        ent->client->ps.persistant[ PERS_NEWWEAPON ] = WP_BLASTER;
    }
    else
      ent->client->ps.persistant[ PERS_NEWWEAPON ] = weapon;
   
    // Lak: The following hack has been moved to PM_BeginWeaponChange, but I'm going to
    // redundantly leave it here as well just in case there's a case I'm forgetting
    // because I don't want to face the gameplay consequences such an error would have

    // force this here to prevent flamer effect from continuing 
    ent->client->ps.generic1 = WPM_NOTFIRING;

    ent->client->ps.weapon = ent->client->ps.persistant[ PERS_NEWWEAPON ];
  }
}

/*
=================
G_GiveClientMaxAmmo
=================
*/
void G_GiveClientMaxAmmo( gentity_t *ent, qboolean buyingEnergyAmmo )
{
  int       i;
  int       maxAmmo, maxClips;
  qboolean  weaponType, restoredAmmo = qfalse;

  for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
  {
    if( buyingEnergyAmmo )
      weaponType = BG_FindUsesEnergyForWeapon( i );
    else
      weaponType = !BG_FindUsesEnergyForWeapon( i );

    if( BG_InventoryContainsWeapon( i, ent->client->ps.stats ) &&
        weaponType && !BG_FindInfinteAmmoForWeapon( i ) &&
        !BG_WeaponIsFull( i, ent->client->ps.stats,
          ent->client->ps.ammo, ent->client->ps.powerups ) )
    {
      BG_FindAmmoForWeapon( i, &maxAmmo, &maxClips );

      if( buyingEnergyAmmo )
      {
        G_AddEvent( ent, EV_RPTUSE_SOUND, 0 );

        if( BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
          maxAmmo = (int)( (float)maxAmmo * BATTPACK_MODIFIER );
      }
      else if ( BG_InventoryContainsUpgrade( UP_BATTPACK, ent->client->ps.stats ) )
	maxClips = (int)( (float)maxClips * BATTPACK_MODIFIER_NON_ENERGY );

      BG_PackAmmoArray( i, ent->client->ps.ammo, ent->client->ps.powerups,
                        maxAmmo, maxClips );

      restoredAmmo = qtrue;
    }
  }

  if( restoredAmmo )
    G_ForceWeaponChange( ent, ent->client->ps.weapon );
}

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout )
{
  vec3_t v, newv;
  float dot;

  VectorSubtract( impact, start, v );
  dot = DotProduct( v, dir );
  VectorMA( v, -2 * dot, dir, newv );

  VectorNormalize(newv);
  VectorMA(impact, 8192, newv, endout);
}

/*
================
G_WideTrace

Trace a bounding box against entities, but not the world
Also check there is a line of sight between the start and end point
================
*/
static void G_WideTrace( trace_t *tr, gentity_t *ent, float range, float width, gentity_t **target )
{
  vec3_t    mins, maxs;
  vec3_t    end;

  VectorSet( mins, -width, -width, -width );
  VectorSet( maxs, width, width, width );

  *target = NULL;

  if( !ent->client )
    return;

  // Set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );
  CalcMuzzlePoint( ent, forward, right, up, muzzle );
  VectorMA( muzzle, range, forward, end );

  G_UnlaggedOn( ent, muzzle, range );

  // Trace against entities
  trap_Trace( tr, muzzle, mins, maxs, end, ent->s.number, CONTENTS_BODY );
  if( tr->entityNum != ENTITYNUM_NONE )
  {
    *target = &g_entities[ tr->entityNum ];

    // Set range to the trace length plus the width, so that the end of the
    // LOS trace is close to the exterior of the target's bounding box
    range = Distance( muzzle, tr->endpos ) + width;
    VectorMA( muzzle, range, forward, end );

    // Trace for line of sight against the world
    trap_Trace( tr, muzzle, NULL, NULL, end, 0, CONTENTS_SOLID );
    if( tr->fraction < 1.0f )
      *target = NULL;
  }

  G_UnlaggedOff( );
}


/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to )
{
  int   i;

  for( i = 0 ; i < 3 ; i++ )
  {
    if( to[ i ] <= v[ i ] )
      v[ i ] = (int)v[ i ];
    else
      v[ i ] = (int)v[ i ] + 1;
  }
}

/*
===============
meleeAttack
===============
*/
void meleeAttack( gentity_t *ent, float range, float width, int damage, meansOfDeath_t mod )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  vec3_t    mins, maxs;

  VectorSet( mins, -width, -width, -width );
  VectorSet( maxs, width, width, width );

  // set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, range, forward, end );

  G_UnlaggedOn( ent, muzzle, range );
  trap_Trace( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // send blood impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( traceEnt->takedamage )
    G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, mod );
//     G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, mod );
}

/*
======================================================================

MACHINEGUN

======================================================================
*/

void bulletFire( gentity_t *ent, float spread, int damage, int mod )
{
  trace_t   tr;
  vec3_t    end;
  float   r;
  float   u;
  gentity_t *tent;
  gentity_t *traceEnt;

  r = random( ) * M_PI * 2.0f;
  u = sin( r ) * crandom( ) * spread * 16;
  r = cos( r ) * crandom( ) * spread * 16;
  VectorMA( muzzle, 8192 * 16, forward, end );
  VectorMA( end, r, right, end );
  VectorMA( end, u, up, end );

  // don't use unlagged if this is not a client (e.g. turret)
  if( ent->client )
  {
    G_UnlaggedOn( ent, muzzle, 8192 * 16 );
    trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
    G_UnlaggedOff( );
  }
  else
    trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send bullet impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
    tent->s.eventParm = traceEnt->s.number;
  }
  else
  {
    tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
    tent->s.eventParm = DirToByte( tr.plane.normal );
  }
  tent->s.otherEntityNum = ent->s.number;

  if( traceEnt->takedamage )
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      damage, 0, mod );
  }
}

/*
======================================================================

SHOTGUN

======================================================================
*/

// this should match CG_ShotgunPattern
void ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent )
{
  int        i;
  float      r, u;
  vec3_t    end;
  vec3_t    forward, right, up;
  trace_t    tr;
  gentity_t  *traceEnt;

  // derive the right and up vectors from the forward vector, because
  // the client won't have any other information
  VectorNormalize2( origin2, forward );
  PerpendicularVector( right, forward );
  CrossProduct( forward, right, up );

  // generate the "random" spread pattern
  for( i = 0; i < SHOTGUN_PELLETS; i++ )
  {
    r = Q_crandom( &seed ) * SHOTGUN_SPREAD * 16;
    u = Q_crandom( &seed ) * SHOTGUN_SPREAD * 16;
    VectorMA( origin, 8192 * 16, forward, end );
    VectorMA( end, r, right, end );
    VectorMA( end, u, up, end );

    trap_Trace( &tr, origin, NULL, NULL, end, ent->s.number, MASK_SHOT );
    traceEnt = &g_entities[ tr.entityNum ];

    // send bullet impact
    if( !( tr.surfaceFlags & SURF_NOIMPACT ) )
    {
      if( traceEnt->takedamage )
        G_Damage( traceEnt, ent, ent, forward, tr.endpos,  SHOTGUN_DMG, 0, MOD_SHOTGUN );
    }
  }
}


void shotgunFire( gentity_t *ent )
{
  gentity_t    *tent;

  // send shotgun blast
  tent = G_TempEntity( muzzle, EV_SHOTGUN );
  VectorScale( forward, 4096, tent->s.origin2 );
  SnapVector( tent->s.origin2 );
  tent->s.eventParm = rand() & 255;    // seed for spread pattern
  tent->s.otherEntityNum = ent->s.number;
  G_UnlaggedOn( ent, muzzle, 8192 * 16 );
  ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
  G_UnlaggedOff();
}

/*
======================================================================

MASS DRIVER

======================================================================
*/
//sorry about this mess, i fucked up the code. Here's a backup from lakitu7's.
/*
void massDriverFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;

  VectorMA( muzzle, 8192 * 16, forward, end );

  G_UnlaggedOn( ent, muzzle, 8192 * 16 );
  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }
  else
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( traceEnt->takedamage )
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      MDRIVER_DMG, 0, MOD_MDRIVER );
  }
}
*/
/*
======================================================================

MASS DRIVER
BULLET PHYSICS

======================================================================
*/

void massDriverFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_mdriver( ent, muzzle, forward );

//VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta ); 
}

/*
======================================================================

LOCKBLOB

======================================================================
*/

void lockBlobLauncherFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_lockblob( ent, muzzle, forward );

  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics //although no point, there's a secret where humans can buy this weapon for 3000 funds. People may ask 'why are bots holding invisable guns, shooting blobs/barbs?' I just like to troll them. But remember, this is a secret.
}

/*
======================================================================

HIVE

======================================================================
*/

void hiveFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_hive( ent, muzzle, forward );

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

BLASTER PISTOL

======================================================================
*/

void blasterFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_blaster( ent, muzzle, forward );

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

PULSE RIFLE

======================================================================
*/

void pulseRifleFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_pulseRifle( ent, muzzle, forward );

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

FLAME THROWER

======================================================================
*/

void flamerFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_flamer( ent, muzzle, forward );
}
/*
===============
airBlastFire
===============
*/
void airBlastFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_airBlast( ent, muzzle, forward );
  //must be on so it works on lifts and movers and shit
  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );
}


/*
======================================================================

GRENADE

======================================================================
*/

void throwGrenade( gentity_t *ent )
{
  gentity_t *m;

  m = launch_grenade( ent, muzzle, forward );
}

/*
======================================================================

LAS GUN

======================================================================
*/

/*
===============
lasGunFire
===============
*N/B: Kill means it was added. I ripped it from mgun. You can 'kill'/delete these if you want.
*/
void lasGunFire( gentity_t *ent, float spread ) //kill float spread
{
  trace_t   tr;
  vec3_t    end;
  float   r; //kill
  float   u; //kill
  gentity_t *tent;
  gentity_t *traceEnt;

  r = random( ) * M_PI * 2.0f; //kill to
  u = sin( r ) * crandom( ) * spread * 16;
  r = cos( r ) * crandom( ) * spread * 16;
  VectorMA( muzzle, 8192 * 16, forward, end );
  VectorMA( end, r, right, end );
  VectorMA( end, u, up, end ); //kill

 // VectorMA( muzzle, 8192 * 16, forward, end ); //anyways included above

  G_UnlaggedOn( ent, muzzle, 8192 * 16 );
  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }
  else
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( traceEnt->takedamage )
    G_Damage( traceEnt, ent, ent, forward, tr.endpos, LASGUN_DAMAGE, 0, MOD_LASGUN );
}

/*
======================================================================

PAIN SAW

======================================================================
*/

void painSawFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;

  // set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, PAINSAW_RANGE, forward, end );

  G_UnlaggedOn( ent, muzzle, PAINSAW_RANGE );
  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  // send blood impact
  if( traceEnt->takedamage )
  {
    vec3_t  temp;

    //hack to get the particle system to line up with the weapon
    VectorCopy( tr.endpos, temp );
    temp[ 2 ] -= 10.0f;

    if( traceEnt->client )
    {
      tent = G_TempEntity( temp, EV_MISSILE_HIT );
      tent->s.otherEntityNum = traceEnt->s.number;
    }
    else
      tent = G_TempEntity( temp, EV_MISSILE_MISS );

    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( traceEnt->takedamage )
    G_Damage( traceEnt, ent, ent, forward, tr.endpos, PAINSAW_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PAINSAW );
}

/*
======================================================================

LUCIFER CANNON

======================================================================
*/

/*
===============
LCChargeFire
===============
*/
void LCChargeFire( gentity_t *ent, qboolean secondary )
{
  gentity_t *m;

  if( secondary )
  {
    m = fire_luciferCannon( ent, muzzle, forward, LCANNON_SECONDARY_DAMAGE,
      LCANNON_SECONDARY_RADIUS );
    ent->client->ps.weaponTime = LCANNON_REPEAT;
  }
  else
  {
    m = fire_luciferCannon( ent, muzzle, forward, ent->client->ps.stats[ STAT_MISC ], LCANNON_RADIUS );
    ent->client->ps.weaponTime = LCANNON_CHARGEREPEAT;
  }

  ent->client->ps.stats[ STAT_MISC ] = 0;
}

/*
======================================================================

TESLA GENERATOR

======================================================================
*/


void teslaFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *traceEnt, *tent;
//TODO: Shift muzzle up 10 or so units to the bbox max, and strike over buildables
  VectorMA( muzzle, TESLAGEN_RANGE, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

  if( tr.entityNum == ENTITYNUM_NONE )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->client )
    return;

  if( traceEnt->client && traceEnt->client->ps.stats[ STAT_PTEAM ] != PTE_ALIENS )
    return;

  //so the client side knows
  ent->s.eFlags |= EF_FIRING;

  if( traceEnt->takedamage )
  {
    G_Damage( traceEnt, ent, ent, forward, tr.endpos,
      TESLAGEN_DMG, 0, MOD_TESLAGEN );
  }

  // snap the endpos to integers to save net bandwidth, but nudged towards the line
  SnapVectorTowards( tr.endpos, muzzle );

  // send railgun beam effect
  tent = G_TempEntity( tr.endpos, EV_TESLATRAIL );

  VectorCopy( muzzle, tent->s.origin2 );

  tent->s.generic1 = ent->s.number; //src
  tent->s.clientNum = traceEnt->s.number; //dest

  // move origin a bit to come closer to the drawn gun muzzle
  VectorMA( tent->s.origin2, 28, up, tent->s.origin2 );
}


/*
======================================================================

BUILD GUN

======================================================================
*/

/*
===============
cancelBuildFire
===============
*/
void cancelBuildFire( gentity_t *ent )
{


  vec3_t      forward, end;
  trace_t     tr;
  gentity_t   *traceEnt;
  vec3_t    mins, maxs;
  int         bHealth;
//  int	      damage = 20;
//  int         hHealth;

  G_UnlaggedOn( ent, muzzle, LEVEL0_BITE_RANGE );
  trap_Trace( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( ent->client->ps.stats[ STAT_BUILDABLE ] != BA_NONE )
  {
    ent->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
    return;
  }

  //repair buildable
  if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
  {
    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
    VectorMA( ent->client->ps.origin, 1000, forward, end );

    trap_Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID );
    traceEnt = &g_entities[ tr.entityNum ];

    if( tr.fraction < 1.0 &&
        ( traceEnt->s.eType == ET_BUILDABLE ) &&
        ( traceEnt->biteam == ent->client->ps.stats[ STAT_PTEAM ] ) &&
        ( ( ent->client->ps.weapon >= WP_HBUILD2 ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) ) &&
        traceEnt->spawned && traceEnt->health > 0 )
    {
      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
      }

      bHealth = BG_FindHealthForBuildable( traceEnt->s.modelindex );

      traceEnt->health += HBUILD_HEALRATE;
      ent->client->pers.statscounters.repairspoisons++;
      level.humanStatsCounters.repairspoisons++;

      if( traceEnt->health > bHealth )
        traceEnt->health = bHealth;

      if( traceEnt->health == bHealth )
        G_AddEvent( ent, EV_BUILD_REPAIRED, 0 );
      else
        G_AddEvent( ent, EV_BUILD_REPAIR, 0 );
    }

//never mind about this part onwards: doesn't work.
//I just keep it here.

    else if ( tr.fraction < 1.0 &&
        ( traceEnt->s.eType != ET_BUILDABLE ) &&
        (traceEnt->biteam == ent->client->ps.stats[ STAT_PTEAM ] ) &&
        ( ( ent->client->ps.weapon >= WP_HBUILD2 ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) )) //same stats except not a buildable
	{

      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
      }
      traceEnt->health += HBUILD_HEALRATE;
      ent->client->pers.statscounters.repairspoisons++;
      level.humanStatsCounters.repairspoisons++;
/*
       hHealth = ( ent->health > client->ps.stats[ STAT_HEALTH ]); //client not declared etc.... just an example of what i want
 *      for max health, but it isn't needed as vampire mode's 150%maxhealth is on.
*/
/* Play sounds for 'maxhealth reached' and stuff like that
      if( traceEnt->health == maxHealth )
        G_AddEvent( ent, EV_BUILD_REPAIRED, 0 );
      else
        G_AddEvent( ent, EV_BUILD_REPAIR, 0 );
*/
	}
	else if ((traceEnt->biteam != ent->client->ps.stats[ STAT_PTEAM ] ) &&
        ( ( ent->client->ps.weapon >= WP_HBUILD2 ) &&
          ( ent->client->ps.weapon <= WP_HBUILD ) )) //same stats except not on human team)
	{
      if( ent->client->ps.stats[ STAT_MISC ] > 0 )
      {
        G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
        return;
//Following is 'unreachable' according to the compiler
//      traceEnt->health -= 20; //do sum dmg!
	//damage 
//same for this part - but i just keep it in case server crashes due to a ckit killing.
//  G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_TRIGGER_HURT );

      }
	}
  }
  else if( ent->client->ps.weapon == WP_ABUILD2 )
    meleeAttack( ent, ABUILDER_CLAW_RANGE, ABUILDER_CLAW_WIDTH,
                 ABUILDER_CLAW_DMG, MOD_ABUILDER_CLAW ); //melee attack for alien builder2

  else if( ent->client->ps.weapon == WP_ABUILD )
    meleeAttack( ent, ABUILDER_CLAW_RANGE, ABUILDER_CLAW_WIDTH,
                 ABUILDER_CLAW_DMG, MOD_ABUILDER_CLAW ); //melee attack for alien builder
}

/*
===============
buildFire
===============
*/
void buildFire( gentity_t *ent, dynMenu_t menu )
{
  if( ( ent->client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT ) > BA_NONE )
  {
    if( ent->client->ps.stats[ STAT_MISC ] > 0 )
    {
      G_AddEvent( ent, EV_BUILD_DELAY, ent->client->ps.clientNum );
      return;
    }

    if( G_BuildIfValid( ent, ent->client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT ) )
    {
      if( g_cheats.integer )
      {
        ent->client->ps.stats[ STAT_MISC ] = 0;
      }
      
      else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS && !G_IsOvermindBuilt( ) )
      {
        ent->client->ps.stats[ STAT_MISC ] += OVERMIND_BT;
         //BG_FindBuildDelayForWeapon( ent->s.weapon ) * 2;
      }
      else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS && !G_IsPowered( muzzle ) &&
          ( ent->client->ps.stats[ STAT_BUILDABLE ] & ~SB_VALID_TOGGLEBIT ) != BA_H_REPEATER ) //hack
      {
        ent->client->ps.stats[ STAT_MISC ] += REACTOR_BT;
          //BG_FindBuildDelayForWeapon( ent->s.weapon ) * 2;
      }

      else
        ent->client->ps.stats[ STAT_MISC ] +=
          BG_FindBuildDelayForWeapon( ent->s.weapon );
//Reset stat
      ent->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
      
      /*
      //TODO: ZdrytchX: Something wrong here... BG_FindBuildTimeForBuildable(ent->client->ps.stats [stat_buildable]) returns same value for every buildable?
      else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
      {
        //float modifier = ABUILDER_ADV_DELAY / BG_FindBuildDelayForWeapon( ent->s.weapon );
        ent->client->ps.stats[ STAT_MISC ] += 
          BG_FindBuildTimeForBuildable( ent->client->ps.stats[ STAT_BUILDABLE ] ); // * modifier + ABUILDER_BUILD_EXTRA;
      }
      else if( ent->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
      {
        //float modifier = HBUILD2_DELAY / BG_FindBuildDelayForWeapon( ent->s.weapon );
        ent->client->ps.stats[ STAT_MISC ] += 
          BG_FindBuildTimeForBuildable( ent->client->ps.stats[ STAT_BUILDABLE ] ); // * modifier + HBUILD_EXTRA;
      }
      */


      // don't want it bigger than 32k
      if( ent->client->ps.stats[ STAT_MISC ] > 30000 )
        ent->client->ps.stats[ STAT_MISC ] = 30000;
    }
    return;
  }

  G_TriggerMenu( ent->client->ps.clientNum, menu );
}

void slowBlobFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_slowBlob( ent, muzzle, forward );

  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}


/*
======================================================================

LEVEL0

======================================================================
*/

/*
===============
CheckVenomAttack
===============
*/
qboolean CheckVenomAttack( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *tent;
  gentity_t *traceEnt;
  vec3_t    mins, maxs;
  int       damage = LEVEL0_BITE_DMG;

  VectorSet( mins, -LEVEL0_BITE_WIDTH, -LEVEL0_BITE_WIDTH, -LEVEL0_BITE_WIDTH );
  VectorSet( maxs, LEVEL0_BITE_WIDTH, LEVEL0_BITE_WIDTH, LEVEL0_BITE_WIDTH );

  // set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, LEVEL0_BITE_RANGE, forward, end );

  G_UnlaggedOn( ent, muzzle, LEVEL0_BITE_RANGE );
  trap_Trace( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return qfalse;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->takedamage )
    return qfalse;

  if( !traceEnt->client && !traceEnt->s.eType == ET_BUILDABLE )
    return qfalse;

  //allow level0 bites to work against these buildables only
  if( traceEnt->s.eType == ET_BUILDABLE )
  {
    if( traceEnt->s.modelindex != BA_H_DCC &&
        traceEnt->s.modelindex != BA_H_MGTURRET &&
        traceEnt->s.modelindex != BA_H_REACTOR &&
        traceEnt->s.modelindex != BA_H_SPAWN &&
        traceEnt->s.modelindex != BA_H_REPEATER &&
        traceEnt->s.modelindex != BA_H_TESLAGEN )

      return qfalse;

    //hackery
    damage *= 0.5f;
  }

  if( traceEnt->client )
  {
    if( traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS && g_mode_teamkill.integer != 1)
      return qfalse;
    if( traceEnt->client->ps.stats[ STAT_HEALTH ] <= 0 )
      return qfalse;
  }

  // send blood impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_LEVEL0_BITE );

  return qtrue;
}

/*
======================================================================

LEVEL1

======================================================================
*/

/*
===============
CheckGrabAttack
===============
*/
void CheckGrabAttack( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end, dir;
  gentity_t *traceEnt;

  // set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, LEVEL1_GRAB_RANGE, forward, end );

  trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  if( !traceEnt->takedamage )
    return;

  if( traceEnt->client )
  {
    if( traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS )
      return;

    if( traceEnt->client->ps.stats[ STAT_HEALTH ] <= 0 )
      return;

    if( !( traceEnt->client->ps.stats[ STAT_STATE ] & SS_GRABBED ) )
    {
      AngleVectors( traceEnt->client->ps.viewangles, dir, NULL, NULL );
      traceEnt->client->ps.stats[ STAT_VIEWLOCK ] = DirToByte( dir );

      //event for client side grab effect
      G_AddPredictableEvent( ent, EV_LEV1_GRAB, 0 );
    }

    traceEnt->client->ps.stats[ STAT_STATE ] |= SS_GRABBED;

    if( ent->client->ps.weapon == WP_ALEVEL1 )
      traceEnt->client->grabExpiryTime = level.time + LEVEL1_GRAB_TIME;
    else if( ent->client->ps.weapon == WP_ALEVEL1_UPG )
      traceEnt->client->grabExpiryTime = level.time + LEVEL1_GRAB_U_TIME;
  }
  else if( traceEnt->s.eType == ET_BUILDABLE &&
      traceEnt->s.modelindex == BA_H_MGTURRET )
  {
    if( !traceEnt->lev1Grabbed )
      G_AddPredictableEvent( ent, EV_LEV1_GRAB, 0 );

    traceEnt->lev1Grabbed = qtrue;
    traceEnt->lev1GrabTime = level.time;
  }
}

/*
===============
poisonCloud
===============
*/
void poisonCloud( gentity_t *ent )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { LEVEL1_PCLOUD_RANGE, LEVEL1_PCLOUD_RANGE, LEVEL1_PCLOUD_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *humanPlayer;
  trace_t   tr;

  VectorAdd( ent->client->ps.origin, range, maxs );
  VectorSubtract( ent->client->ps.origin, range, mins );

  G_UnlaggedOn( ent, ent->client->ps.origin, LEVEL1_PCLOUD_RANGE );
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    humanPlayer = &g_entities[ entityList[ i ] ];

    if( humanPlayer->client && humanPlayer->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS )
    {
//      if( BG_InventoryContainsUpgrade( UP_HELMET, humanPlayer->client->ps.stats ) )
//        continue;

      if( BG_InventoryContainsUpgrade( UP_BATTLESUIT, humanPlayer->client->ps.stats ) )
        continue;

      trap_Trace( &tr, muzzle, NULL, NULL, humanPlayer->s.origin, humanPlayer->s.number, MASK_SHOT );

      //can't see target from here
      if( tr.entityNum == ENTITYNUM_WORLD )
        continue;

 //     if( !( humanPlayer->client->ps.stats[ STAT_STATE ] & SS_POISONCLOUDED ) )
      {
      if( !BG_InventoryContainsUpgrade( UP_BATTLESUIT, humanPlayer->client->ps.stats ) || !BG_InventoryContainsUpgrade( UP_HELMET, humanPlayer->client->ps.stats )  )
	{
        humanPlayer->client->ps.stats[ STAT_STATE ] |= SS_POISONCLOUDED;
	}
        else //Only accepts one arguement, adding SS_SLOWLOCKED invalidates the state
        humanPlayer->client->ps.stats[ STAT_STATE ] |= SS_POISONED;
//        humanPlayer->client->ps.stats[ STAT_STATE ] |= SS_SLOWLOCKED;

        humanPlayer->client->lastPoisonCloudedTime = level.time;
        humanPlayer->client->lastPoisonCloudedClient = ent;
        trap_SendServerCommand( humanPlayer->client->ps.clientNum, "poisoncloud" );
      }
    }
  }
  G_UnlaggedOff( );
}


/*
======================================================================

LEVEL2

======================================================================
*/

#define MAX_ZAPS  64

static zap_t  zaps[ MAX_CLIENTS ];

/*
===============
G_FindNewZapTarget
===============
*/
static gentity_t *G_FindNewZapTarget( gentity_t *ent )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { LEVEL2_AREAZAP_RANGE, LEVEL2_AREAZAP_RANGE, LEVEL2_AREAZAP_RANGE };
  vec3_t    mins, maxs;
  int       i, j, k, num;
  gentity_t *enemy;
  trace_t   tr;

  VectorScale( range, 1.0f / M_ROOT3, range );
  VectorAdd( ent->s.origin, range, maxs );
  VectorSubtract( ent->s.origin, range, mins );

  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];

    if( ( ( enemy->client && enemy->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) ||
        ( enemy->s.eType == ET_BUILDABLE &&
          BG_FindTeamForBuildable( enemy->s.modelindex ) == BIT_HUMANS ) ) && enemy->health > 0 )
    {
      qboolean foundOldTarget = qfalse;

      trap_Trace( &tr, muzzle, NULL, NULL, enemy->s.origin, ent->s.number, MASK_SHOT );

      //can't see target from here
      if( tr.entityNum == ENTITYNUM_WORLD )
        continue;

      for( j = 0; j < MAX_ZAPS; j++ )
      {
        zap_t *zap = &zaps[ j ];

        for( k = 0; k < zap->numTargets; k++ )
        {
          if( zap->targets[ k ] == enemy )
          {
            foundOldTarget = qtrue;
            break;
          }
        }

        if( foundOldTarget )
          break;
      }

      // enemy is already targetted
      if( foundOldTarget )
        continue;

      return enemy;
    }
  }

  return NULL;
}

/*
===============
G_UpdateZapEffect
===============
*/
static void G_UpdateZapEffect( zap_t *zap )
{
  int       j;
  gentity_t *effect = zap->effectChannel;

  effect->s.eType = ET_LEV2_ZAP_CHAIN;
  effect->classname = "lev2zapchain";
  G_SetOrigin( effect, zap->creator->s.origin );
  effect->s.powerups = zap->creator->s.number;

  effect->s.time = effect->s.time2 = effect->s.constantLight = -1;

  for( j = 0; j < zap->numTargets; j++ )
  {
    int number = zap->targets[ j ]->s.number;

    switch( j )
    {
      case 0: effect->s.time = number;          break;
      case 1: effect->s.time2 = number;         break;
      case 2: effect->s.constantLight = number; break;
      default:                                  break;
    }
  }

  trap_LinkEntity( effect );
}

/*
===============
G_CreateNewZap
===============
*/
static void G_CreateNewZap( gentity_t *creator, gentity_t *target )
{
  int       i, j;
  zap_t     *zap;

  for( i = 0; i < MAX_ZAPS; i++ )
  {
    zap = &zaps[ i ];

    if( !zap->used )
    {
      zap->used = qtrue;

      zap->timeToLive = LEVEL2_AREAZAP_TIME;
      zap->damageUsed = 0;

      zap->creator = creator;

      zap->targets[ 0 ] = target;
      zap->numTargets = 1;

      for( j = 1; j < MAX_ZAP_TARGETS && zap->targets[ j - 1 ]; j++ )
      {
        zap->targets[ j ] = G_FindNewZapTarget( zap->targets[ j - 1 ] );

        if( zap->targets[ j ] )
	        {
          zap->numTargets++;
	//testing something  to encourage multiple targets
          zap->damageUsed -= LEVEL2_AREAZAP_DMG / (zap->numTargets - 1); //This should do it
	        }
      }

      zap->effectChannel = G_Spawn( );
      G_UpdateZapEffect( zap );

      return;
    }
  }
}


/*
===============
G_UpdateZaps
===============
*/
void G_UpdateZaps( int msec )
{
  int   i, j;
  zap_t *zap;
  int   damage;

  for( i = 0; i < MAX_ZAPS; i++ )
  {
    zap = &zaps[ i ];

    if( zap->used )
    {
      //check each target is valid
      for( j = 0; j < zap->numTargets; j++ )
      {
        gentity_t *source;
        gentity_t *target = zap->targets[ j ];

        if( j == 0 )
          source = zap->creator;
        else
          source = zap->targets[ j - 1 ];
//If target dies, find new victim else continue zapping the dead body :P
        if( target->health <= 0 || !target->inuse// || //early out
            /*Distance( source->s.origin, target->s.origin ) > LEVEL2_AREAZAP_RANGE_SUSTAIN*/ )
        {
          target = zap->targets[ j ] = G_FindNewZapTarget( source );

          //couldn't find a target, so forget about the rest of the chain //remove this and the zap won't change targets once target dies; leaves a cool zap effect floating about dead body though
          /*
          if( !target )
		{
            zap->numTargets = j;

//		 zap->used = qfalse;		//we don't have to aim anymore:
//        G_FreeEntity( zap->effectChannel );	//ripped from below (when target dies and zap transfers to something else
		}
          */
        }
//If outta range, find new source, else kill off
          if ( Distance( source->s.origin, target->s.origin ) > LEVEL2_AREAZAP_RANGE_SUSTAIN )
          {        
//          if( !target )
//		{
            zap->numTargets = j;
            G_FreeEntity( zap->effectChannel );
//		}
          }
      }

      if( zap->numTargets )
      {
        for( j = 0; j < zap->numTargets; j++ )
        {
          gentity_t *source;
          gentity_t *target = zap->targets[ j ];
          float     r = 1.0f / zap->numTargets;
          float     damageFraction = 2 * r - 2 * j * r * r - r * r; //2 * r - 2 * j * r * r - r * r

          vec3_t    forward;

          if( j == 0 )
            source = zap->creator;
          else
            source = zap->targets[ j - 1 ];
          if(LEVEL2_AREAZAP_TIME > 0) { //can't divide by 0 check
            damage = ceil( ( (float)msec / LEVEL2_AREAZAP_TIME ) *
              LEVEL2_AREAZAP_DMG * damageFraction); }
          else {
            damage = ceil( LEVEL2_AREAZAP_DMG * damageFraction ); }
//idea:
//want to drain energy weapons like KoRx
          // don't let a high msec value inflate the total damage
          if( damage + zap->damageUsed > LEVEL2_AREAZAP_DMG )
            damage = LEVEL2_AREAZAP_DMG - zap->damageUsed + 0.5;

          VectorSubtract( target->s.origin, source->s.origin, forward );
          VectorNormalize( forward );

          //do the damage
          if( damage )
          {
            G_Damage( target, source, zap->creator, forward, target->s.origin,
                    damage, /*DAMAGE_NO_KNOCKBACK |*/ DAMAGE_NO_LOCDAMAGE, MOD_LEVEL2_ZAP );
            zap->damageUsed += damage;
          }
        }
      }

      G_UpdateZapEffect( zap );

      zap->timeToLive -= msec;

      if( zap->timeToLive <= 0 || zap->numTargets == 0 /*|| zap->creator->health <= 0*/ ) //Continue to live while the player dies
      {
        zap->used = qfalse;
        G_FreeEntity( zap->effectChannel );
      }
    }
  }
}

/*
===============
areaZapFire
===============
*/
void areaZapFire( gentity_t *ent )
{
  trace_t   tr;
  vec3_t    end;
  gentity_t *traceEnt;
  vec3_t    mins, maxs;

  VectorSet( mins, -LEVEL2_AREAZAP_WIDTH, -LEVEL2_AREAZAP_WIDTH, -LEVEL2_AREAZAP_WIDTH );
  VectorSet( maxs, LEVEL2_AREAZAP_WIDTH, LEVEL2_AREAZAP_WIDTH, LEVEL2_AREAZAP_WIDTH );

  // set aiming directions
  AngleVectors( ent->client->ps.viewangles, forward, right, up );

  CalcMuzzlePoint( ent, forward, right, up, muzzle );

  VectorMA( muzzle, LEVEL2_AREAZAP_RANGE, forward, end );

  G_UnlaggedOn( ent, muzzle, LEVEL2_AREAZAP_RANGE );
  trap_Trace( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );
  G_UnlaggedOff( );

  if( tr.surfaceFlags & SURF_NOIMPACT )
    return;

  traceEnt = &g_entities[ tr.entityNum ];

  if( ( ( traceEnt->client && traceEnt->client->ps.stats[ STAT_PTEAM ] == PTE_HUMANS ) ||
      ( traceEnt->s.eType == ET_BUILDABLE &&
        BG_FindTeamForBuildable( traceEnt->s.modelindex ) == BIT_HUMANS ) ) && traceEnt->health > 0 )
  {
    G_CreateNewZap( ent, traceEnt );
  }
}


/*
======================================================================

LEVEL3

======================================================================
*/

/*
===============
CheckPounceAttack
===============
*/
qboolean CheckPounceAttack( gentity_t *ent )
{
  trace_t   tr;
  gentity_t *tent;
  gentity_t *traceEnt;
  int       damage;

  if( ent->client->ps.groundEntityNum != ENTITYNUM_NONE )
  {
    ent->client->allowedToPounce = qfalse;
    ent->client->pmext.pouncePayload = 0;
  }

  if( !ent->client->allowedToPounce )
    return qfalse;

  if( ent->client->ps.weaponTime )
    return qfalse;

  G_WideTrace( &tr, ent, LEVEL3_POUNCE_RANGE, LEVEL3_POUNCE_WIDTH, &traceEnt );

  if( traceEnt == NULL )
    return qfalse;

  // send blood impact
  if( traceEnt->takedamage && traceEnt->client )
  {
    tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
    tent->s.otherEntityNum = traceEnt->s.number;
    tent->s.eventParm = DirToByte( tr.plane.normal );
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( !traceEnt->takedamage )
    return qfalse;

  damage = (int)( ( (float)ent->client->pmext.pouncePayload
    / (float)LEVEL3_POUNCE_SPEED ) * LEVEL3_POUNCE_DMG ); 

  ent->client->pmext.pouncePayload = 0;

  G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage,
      0|DAMAGE_NO_LOCDAMAGE, MOD_LEVEL3_POUNCE ); //now like gpp: has knockback

  ent->client->allowedToPounce = qfalse;

  return qtrue;
}
/*
===============
bounceBallFire
===============
*/
void bounceBallFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_bounceBall( ent, muzzle, forward );
//test for max ammo
/*
  if(ent->client)
  {
      int ammo, maxAmmo;

      BG_FindAmmoForWeapon( WP_ALEVEL3_UPG, &maxAmmo, NULL );
      BG_UnpackAmmoArray( WP_ALEVEL3_UPG, ent->client->ps.ammo, client->ps.powerups, &ammo, NULL );
            else if ( ammo == maxAmmo )
      {
        ent->client->time10000 = 0; //Set timer
      }
  }
*/
//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics by adding ur inertia - gets annoying
}


/*
======================================================================

LEVEL4

======================================================================
*/

/*
===============
ChargeAttack
===============
*/
void ChargeAttack( gentity_t *ent, gentity_t *victim )
{
  gentity_t *tent;
  int       damage;
//  gentity_t *traceEnt;
  vec3_t    forward, normal;

  if( level.time < victim->chargeRepeat )
    return;

  victim->chargeRepeat = level.time + LEVEL4_CHARGE_REPEAT;

  VectorSubtract( victim->s.origin, ent->s.origin, forward );
  VectorNormalize( forward );
  VectorNegate( forward, normal );

  if( victim->client )
  {
    tent = G_TempEntity( victim->s.origin, EV_MISSILE_HIT );
    tent->s.otherEntityNum = victim->s.number;
    tent->s.eventParm = DirToByte( normal ); //normal
    tent->s.weapon = ent->s.weapon;
    tent->s.generic1 = ent->s.generic1; //weaponMode
  }

  if( !victim->takedamage )
    return;

  damage = LEVEL4_CHARGE_DMG * (float)ent->client->ps.stats[ STAT_MISC ] /
           LEVEL4_CHARGE_TIME + LEVEL4_CHARGE_EXTRA;
  //allow level4 charge to do only little damage to buildables so it isn't overpowered like hell
  if( victim->s.eType == ET_BUILDABLE ) 
  {
    damage *= (float)(g_level4_trample_buildable_percent.integer * 0.01);
  }

  G_Damage( victim, ent, ent, forward, victim->s.origin, damage, 0|DAMAGE_NO_LOCDAMAGE, MOD_LEVEL4_CHARGE );
}
/*
===============
aBlobFire
===============
*/

void aBlobFire( gentity_t *ent )
{
  gentity_t *m;

  m = fire_aBlob( ent, muzzle, forward );
  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  
}

//======================================================================

/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint )
{
  VectorCopy( ent->s.pos.trBase, muzzlePoint );
  muzzlePoint[ 2 ] += ent->client->ps.viewheight;
  VectorMA( muzzlePoint, 1, forward, muzzlePoint );
  VectorMA( muzzlePoint, 1, right, muzzlePoint );
  // snap to integer coordinates for more efficient network bandwidth usage
  SnapVector( muzzlePoint );
}

/*
===============
FireWeapon3
===============
*/
void FireWeapon3( gentity_t *ent )
{
  if( ent->client )
  {
    // set aiming directions
    AngleVectors( ent->client->ps.viewangles, forward, right, up );
    CalcMuzzlePoint( ent, forward, right, up, muzzle );
  }
  else
  {
    AngleVectors( ent->s.angles2, forward, right, up );
    VectorCopy( ent->s.pos.trBase, muzzle );
  }

  // fire the specific weapon
  switch( ent->s.weapon )
  {
    case WP_ALEVEL3_UPG:
      bounceBallFire( ent );
      break;

    case WP_ABUILD2:
      slowBlobFire( ent );
      break;

    case WP_ALEVEL4:
      aBlobFire( ent );
    break;

    default:
      break;
  }
}

/*
===============
FireWeapon2
===============
*/
void FireWeapon2( gentity_t *ent )
{
  if( ent->client )
  {
    // set aiming directions
    AngleVectors( ent->client->ps.viewangles, forward, right, up );
    CalcMuzzlePoint( ent, forward, right, up, muzzle );
  }
  else
  {
    AngleVectors( ent->s.angles2, forward, right, up );
    VectorCopy( ent->s.pos.trBase, muzzle );
  }

  // fire the specific weapon
  switch( ent->s.weapon )
  {

    case WP_ALEVEL0:
      meleeAttack( ent, LEVEL0_SCRATCH_RANGE, LEVEL0_SCRATCH_WIDTH, LEVEL0_SCRATCH_DMG, MOD_LEVEL0_BITE );
      break;
    case WP_ALEVEL1_UPG:
      poisonCloud( ent );
      break;
    case WP_ALEVEL2_UPG:
      areaZapFire( ent );
      break;

    case WP_LUCIFER_CANNON:
      LCChargeFire( ent, qtrue );
      break;

//meleeAttack( ent, [range], [width], [damage], MOD_[means of death] );
//note: keep width 20 for humans otherwise it will not hit the target in vents.

    case WP_CHAINGUN:
      bulletFire( ent, CHAINGUN_SPREAD2, CHAINGUN_DMG/3, MOD_CHAINGUN ); 
      bulletFire( ent, CHAINGUN_SPREAD2, CHAINGUN_DMG/3, MOD_CHAINGUN ); 
      bulletFire( ent, CHAINGUN_SPREAD2, CHAINGUN_DMG/3, MOD_CHAINGUN );
      break;

    case WP_FLAMER:
      airBlastFire( ent );
    break;

    case WP_ABUILD:
    case WP_ABUILD2:
      cancelBuildFire( ent );
      break;

    case WP_HBUILD:
    case WP_HBUILD2:
      cancelBuildFire( ent );
      break;
    default:
      break;
  }
}

/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent )
{
  if( ent->client )
  {
    // set aiming directions
    AngleVectors( ent->client->ps.viewangles, forward, right, up );
    CalcMuzzlePoint( ent, forward, right, up, muzzle );
  }
  else
  {
    AngleVectors( ent->turretAim, forward, right, up );
    VectorCopy( ent->s.pos.trBase, muzzle );
  }

  // fire the specific weapon
  switch( ent->s.weapon )
  {
    case WP_ALEVEL1:
    case WP_ALEVEL1_UPG:
      meleeAttack( ent, LEVEL1_CLAW_RANGE, LEVEL1_CLAW_WIDTH, LEVEL1_CLAW_DMG, MOD_LEVEL1_CLAW );
      break;
    case WP_ALEVEL3:
    case WP_ALEVEL3_UPG:
      meleeAttack( ent, LEVEL3_CLAW_RANGE, LEVEL3_CLAW_WIDTH, LEVEL3_CLAW_DMG, MOD_LEVEL3_CLAW );
      break;
    case WP_ALEVEL2:
      meleeAttack( ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW );
      break;
    case WP_ALEVEL2_UPG:
      meleeAttack( ent, LEVEL2_CLAW_RANGE, LEVEL2_CLAW_U_WIDTH, LEVEL2_CLAW_DMG, MOD_LEVEL2_CLAW );
      break;
    case WP_ALEVEL4:
      meleeAttack( ent, LEVEL4_CLAW_RANGE, LEVEL4_CLAW_WIDTH, LEVEL4_CLAW_DMG, MOD_LEVEL4_CLAW );
      break;

    case WP_BLASTER:
      blasterFire( ent );
      break;
    case WP_MACHINEGUN:
      bulletFire( ent, RIFLE_SPREAD, RIFLE_DMG, MOD_MACHINEGUN );
      break;
    case WP_SHOTGUN:
      shotgunFire( ent );
      break;
    case WP_CHAINGUN:
      bulletFire( ent, CHAINGUN_SPREAD, CHAINGUN_DMG, MOD_CHAINGUN );
      break;
    case WP_FLAMER:
      flamerFire( ent );
      break;
    case WP_PULSE_RIFLE:
      pulseRifleFire( ent );
      break;
    case WP_MASS_DRIVER:
      massDriverFire( ent );
//      massDriverFire( ent, 0 );
      break;
    case WP_LUCIFER_CANNON:
      LCChargeFire( ent, qfalse );
      break;
    case WP_LAS_GUN:
      lasGunFire( ent, LASGUN_SPREAD );
      break;
    case WP_PAIN_SAW:
      painSawFire( ent );
      break;
    case WP_GRENADE:
      throwGrenade( ent );
      break;

    case WP_LOCKBLOB_LAUNCHER:
      lockBlobLauncherFire( ent );
      break;
    case WP_HIVE:
      hiveFire( ent );
      break;
    case WP_TESLAGEN:
      teslaFire( ent );
      break;
    case WP_MGTURRET:
      bulletFire( ent, MGTURRET_SPREAD, MGTURRET_DMG/2, MOD_MGTURRET );
      bulletFire( ent, MGTURRET_SPREAD, MGTURRET_DMG/2, MOD_MGTURRET );
      break;

    case WP_ABUILD:
    case WP_ABUILD2:
      buildFire( ent, MN_A_BUILD );
      break;
    case WP_HBUILD:
    case WP_HBUILD2:
      buildFire( ent, MN_H_BUILD );
      break;
    default:
      break;
  }
}

