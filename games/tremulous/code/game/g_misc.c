/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

#include "g_local.h"

#define SV_LinkEntity trap_LinkEntity
#define SV_UnlinkEntity trap_UnlinkEntity
#define SV_Trace(a, b, c, w, x, y, z, _1, _2) trap_Trace(a, b, c, w, x, y, z)

extern void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins,
                 const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
extern void trap_UnlinkEntity( gentity_t *ent );
extern void trap_LinkEntity( gentity_t *ent );


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self )
{
  G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self )
{
  G_SetOrigin( self, self->r.currentOrigin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( gentity_t *self )
{
  G_FreeEntity( self );
}

/*
============
G_NoTarget

Determines if an entity can be targeted
============
*/
qboolean G_NoTarget( gentity_t *ent )
{
  if( g_targetProtection.integer &&
      ent->targetProtectionTime > level.time )
    return qtrue;

  return (ent->flags & FL_NOTARGET);
}

/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles, float speed )
{
  // unlink to make sure it can't possibly interfere with G_KillBox
  SV_UnlinkEntity( player );

  VectorCopy( origin, player->client->ps.origin );
  player->client->ps.groundEntityNum = ENTITYNUM_NONE;
  player->client->ps.stats[ STAT_STATE ] &= ~SS_GRABBED;

  AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
  VectorScale( player->client->ps.velocity, speed, player->client->ps.velocity );
  player->client->ps.pm_time = 0.4f * fabs( speed ); // duration of loss of control
  if( player->client->ps.pm_time > 160 )
    player->client->ps.pm_time = 160;
  if( player->client->ps.pm_time != 0 )
    player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

  // toggle the teleport bit so the client knows to not lerp
  player->client->ps.eFlags ^= EF_TELEPORT_BIT;
  G_UnlaggedClear( player );

  // cut all relevant zap beams
  G_ClearPlayerZapEffects( player );

  // set angles
  G_SetClientViewAngle( player, angles );

  // save results of pmove
  BG_PlayerStateToEntityState(
    &player->client->ps, &player->s, &player->client->pmext );

  // use the precise origin for linking
  VectorCopy( player->client->ps.origin, player->r.currentOrigin );
  VectorCopy( player->client->ps.viewangles, player->r.currentAngles );

  // Protect against targeting by buildables
  player->targetProtectionTime = level.time + TELEPORT_PROTECTION_TIME;

  if( player->client->sess.spectatorState == SPECTATOR_NOT )
  {
    // kill anything at the destination
    G_KillBox( player );

    SV_LinkEntity (player);
  }
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent )
{
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"   arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent )
{
#if 0
  ent->s.modelindex = G_ModelIndex( ent->model );
  VectorSet (ent->mins, -16, -16, -16);
  VectorSet (ent->maxs, 16, 16, 16);
  SV_LinkEntity (ent);

  G_SetOrigin( ent, ent->r.currentOrigin );
#else
  G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent )
{
  vec3_t    dir;
  gentity_t *target;
  gentity_t *owner;

  owner = G_PickTarget( ent->target );
  if( !owner )
  {
    Com_Printf( "Couldn't find target for misc_portal_surface\n" );
    G_FreeEntity( ent );
    return;
  }
  ent->r.ownerNum = owner->s.number;

  // frame holds the rotate speed
  if( owner->spawnflags & 1 )
    ent->s.frame = 25;
  else if( owner->spawnflags & 2 )
    ent->s.frame = 75;

  // swing camera ?
  if( owner->spawnflags & 4 )
  {
    // set to 0 for no rotation at all
    ent->s.misc = 0;
  }
  else
    ent->s.misc = 1;

  // clientNum holds the rotate offset
  ent->s.clientNum = owner->s.clientNum;

  VectorCopy( owner->r.currentOrigin, ent->s.origin2 );

  // see if the portal_camera has a target
  target = G_PickTarget( owner->target );
  if( target )
  {
    VectorSubtract( target->r.currentOrigin, owner->r.currentOrigin, dir );
    VectorNormalize( dir );
  }
  else
    G_SetMovedir( owner->r.currentAngles, dir );

  ent->s.eventParm = DirToByte( dir );

  ByteToDir( ent->s.eventParm, dir );
  vectoangles( dir, ent->r.currentAngles );
  ent->r.currentAngles[ 2 ] = ent->s.clientNum * 360.0f / 256;
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface( gentity_t *ent )
{
  VectorClear( ent->r.mins );
  VectorClear( ent->r.maxs );
  SV_LinkEntity( ent );

  ent->r.svFlags = SVF_PORTAL;
  ent->s.eType = ET_PORTAL;

  if( !ent->target )
  {
    VectorCopy( ent->r.currentOrigin, ent->s.origin2 );
  }
  else
  {
    ent->think = locateCamera;
    ent->nextthink = level.time + 100;
  }
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing

The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera( gentity_t *ent )
{
  float roll;

  VectorClear( ent->r.mins );
  VectorClear( ent->r.maxs );
  SV_LinkEntity( ent );

  G_SpawnFloat( "roll", "0", &roll );

  ent->s.clientNum = roll / 360.0f * 256;
}

/*
======================================================================

  NEAT EFFECTS AND STUFF FOR TREMULOUS

======================================================================
*/

void SP_toggle_particle_system( gentity_t *self )
{
  //toggle EF_NODRAW
  self->s.eFlags ^= EF_NODRAW;

  self->nextthink = 0;
}

/*
===============
SP_use_particle_system

Use function for particle_system
===============
*/
void SP_use_particle_system( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  SP_toggle_particle_system( self );

  if( self->wait > 0.0f )
  {
    self->think = SP_toggle_particle_system;
    self->nextthink = level.time + (int)( self->wait * 1000 );
  }
}

/*
===============
SP_spawn_particle_system

Spawn function for particle system
===============
*/
void SP_misc_particle_system( gentity_t *self )
{
  char  *s;

  G_SetOrigin( self, self->r.currentOrigin );

  G_SpawnString( "psName", "", &s );
  G_SpawnFloat( "wait", "0", &self->wait );

  //add the particle system to the client precache list
  self->s.modelindex = G_ParticleSystemIndex( s );

  if( self->spawnflags & 1 )
    self->s.eFlags |= EF_NODRAW;

  self->use = SP_use_particle_system;
  self->s.eType = ET_PARTICLE_SYSTEM;
  SV_LinkEntity( self );
}

/*
===============
SP_use_anim_model

Use function for anim model
===============
*/
void SP_use_anim_model( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( self->spawnflags & 1 )
  {
    //if spawnflag 1 is set
    //toggle EF_NODRAW
    if( self->s.eFlags & EF_NODRAW )
      self->s.eFlags &= ~EF_NODRAW;
    else
      self->s.eFlags |= EF_NODRAW;
  }
  else
  {
    //if the animation loops then toggle the animation
    //toggle EF_MOVER_STOP
    if( self->s.eFlags & EF_MOVER_STOP )
      self->s.eFlags &= ~EF_MOVER_STOP;
    else
      self->s.eFlags |= EF_MOVER_STOP;
  }
}

/*
===============
SP_misc_anim_model

Spawn function for anim model
===============
*/
void SP_misc_anim_model( gentity_t *self )
{
  self->s.misc      = (int)self->animation[ 0 ];
  self->s.weapon    = (int)self->animation[ 1 ];
  self->s.torsoAnim = (int)self->animation[ 2 ];
  self->s.legsAnim  = (int)self->animation[ 3 ];

  self->s.angles2[ 0 ] = self->pos2[ 0 ];

  //add the model to the client precache list
  self->s.modelindex = G_ModelIndex( self->model );

  self->use = SP_use_anim_model;

  self->s.eType = ET_ANIMMAPOBJ;

  // spawn with animation stopped
  if( self->spawnflags & 2 )
    self->s.eFlags |= EF_MOVER_STOP;

  SV_LinkEntity( self );
}

/*
===============
SP_use_light_flare

Use function for light flare
===============
*/
void SP_use_light_flare( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  self->s.eFlags ^= EF_NODRAW;
}

/*
===============
findEmptySpot

Finds an empty spot radius units from origin
==============
*/
static void findEmptySpot( vec3_t origin, float radius, vec3_t spot )
{
  int     i, j, k;
  vec3_t  delta, test, total;
  trace_t tr;

  VectorClear( total );

  //54(!) traces to test for empty spots
  for( i = -1; i <= 1; i++ )
  {
    for( j = -1; j <= 1; j++ )
    {
      for( k = -1; k <= 1; k++ )
      {
        VectorSet( delta, ( i * radius ),
                          ( j * radius ),
                          ( k * radius ) );

        VectorAdd( origin, delta, test );

        SV_Trace(
          &tr, test, NULL, NULL, test, -1, qfalse,
          *Temp_Clip_Mask(MASK_SOLID, 0), TT_AABB);

        if( !tr.allsolid )
        {
          SV_Trace(
            &tr, test, NULL, NULL, origin, -1, qfalse,
            *Temp_Clip_Mask(MASK_SOLID, 0), TT_AABB);
          VectorScale( delta, tr.fraction, delta );
          VectorAdd( total, delta, total );
        }
      }
    }
  }

  VectorNormalize( total );
  VectorScale( total, radius, total );
  VectorAdd( origin, total, spot );
}

/*
===============
SP_misc_light_flare

Spawn function for light flare
===============
*/
void SP_misc_light_flare( gentity_t *self )
{
  self->s.eType = ET_LIGHTFLARE;
  self->s.modelindex = G_ShaderIndex( self->targetShaderName );
  VectorCopy( self->pos2, self->s.origin2 );

  //try to find a spot near to the flare which is empty. This
  //is used to facilitate visibility testing
  findEmptySpot( self->r.currentOrigin, 8.0f, self->s.angles2 );

  self->use = SP_use_light_flare;

  G_SpawnFloat( "speed", "200", &self->speed );
  self->s.time = self->speed;

  G_SpawnInt( "mindist", "0", &self->s.generic1 );

  if( self->spawnflags & 1 )
    self->s.eFlags |= EF_NODRAW;

  SV_LinkEntity( self );
}

/*
===============
G_PackEntityNumbers

Pack entity numbers into an entityState_t.
Used for sending zap effects to clients.
===============
*/
void G_PackEntityNumbers( entityState_t *es, int creatorNum,
                           bglist_t *targetQueue )
{
  bglink_t *targetZapLink = BG_List_Peek_Head_Link( targetQueue );
  int      i;
  int      count = BG_List_Get_Length( targetQueue ) + 1;

  if( count > MAX_NUM_PACKED_ENTITY_NUMS )
  {
    count = MAX_NUM_PACKED_ENTITY_NUMS;
    Com_Printf( S_COLOR_YELLOW "WARNING: A maximum of %d entity numbers can be "
      "packed, but BG_PackEntityNumbers was passed %d entities",
      MAX_NUM_PACKED_ENTITY_NUMS, count );
  }

  es->misc = es->time = es->time2 = es->constantLight = 0;

  if( creatorNum & ~GENTITYNUM_MASK )
  {
    Com_Error( ERR_FATAL, "BG_PackEntityNumbers passed a creator entity number (%d) "
      "which exceeds %d bits", creatorNum, GENTITYNUM_BITS );
  }
  es->misc |= creatorNum;
  for( i = 1; i < MAX_NUM_PACKED_ENTITY_NUMS; i++ )
  {
    int entityNum;

    if( i < count )
    {
      entityNum = ((zapTarget_t *)targetZapLink->data)->targetEnt->s.number;
      targetZapLink = targetZapLink->next;
    }
    else
      entityNum = ENTITYNUM_NONE;

    if( entityNum & ~GENTITYNUM_MASK )
    {
      Com_Error( ERR_FATAL, "BG_PackEntityNumbers passed an entity number (%d) which "
        "exceeds %d bits", entityNum, GENTITYNUM_BITS );
    }

    switch( i )
    {
      case 1: es->time |= entityNum;                                       break;
      case 2: es->time |= entityNum << GENTITYNUM_BITS;                    break;
      case 3: es->time |= entityNum << (GENTITYNUM_BITS * 2);              break;
      case 4: es->time2 |= entityNum;                                      break;
      case 5: es->time2 |= entityNum << GENTITYNUM_BITS;                   break;
      case 6: es->time2 |= entityNum << (GENTITYNUM_BITS * 2);             break;
      case 7: es->constantLight |= entityNum;                              break;
      case 8: es->constantLight |= entityNum << GENTITYNUM_BITS;           break;
      case 9: es->constantLight |= entityNum << (GENTITYNUM_BITS * 2);     break;
      default: Com_Error( ERR_FATAL, "Entity index %d not handled", i );   break;
    }
  }
}
