// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_misc.c

#include "g_local.h"


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}



/*
=================================================================================

TELEPORTERS

=================================================================================
*/
#define PORTAL_EXTRA_SPEED 42.0f // two times the size of hit box?

static void TeleportPlayer_real( gentity_t *player, 
  vec3_t origin, vec3_t angles, 
  gentity_t *destination, gentity_t *self ) {
	gentity_t	*tent;
  vec3_t angles2;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( !destination && !self && player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity( player );

	VectorCopy( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1.0f;

	// spit the player out
  if(!destination && !self) {
    AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
    VectorScale( player->client->ps.velocity, (g_speed.value * 1.25f), player->client->ps.velocity );
    SetClientViewAngle( player, angles );
  } else {
    float normal;
    vec3_t angleView, angleVelocity, vec;
    VectorCopy(player->client->ps.velocity, vec);
    normal = VectorNormalize(vec);
    VectorClear(angleView);
    VectorClear(angleVelocity);
    if(normal < PORTAL_EXTRA_SPEED) {
      Com_Printf("Player speed too low: %f\n", normal);
      normal = PORTAL_EXTRA_SPEED;
    }
    if(destination->s.eventParm) {
      ByteToDir( destination->s.eventParm, angles );
      vectoangles( angles, angles );
			// the destination is a wall view
			if(self->s.eventParm) {
        Com_Printf("wall -> wall\n");
				// the source portal is also a wall view
				// this only spits out at the angle of the destination portal
				//   we want the velocity relative to the source portal
				//   when the player comes out they appear to maintain the same jump angle
        // change the direction of the velocity by the same angle so if you enter
        //   while jumping at an angle, it doesn't reset the view straight forward,
        //   A disappointing missing feature from Q3DM0
        ByteToDir( self->s.eventParm, angles2 );
        vectoangles( angles2, angles2 );

        vectoangles(vec, angleVelocity);
        VectorSubtract(player->client->ps.viewangles, angleVelocity, angleView);
        VectorSubtract(angleVelocity, angles2, angleVelocity);
        angleVelocity[1] -= 180; // for the other side of the portal?
        VectorAdd(angles, angleVelocity, angleVelocity);
        AngleVectors( angleVelocity, player->client->ps.velocity, NULL, NULL );
				VectorScale( player->client->ps.velocity, normal * 1.25f, player->client->ps.velocity );
        // change client view angle
        angleView[0] = player->client->ps.viewangles[0];
        //angleView[0] = angleView[0] + angleVelocity[0];
        angleView[1] = angleView[1] + angleVelocity[1];
        angleView[2] = player->client->ps.viewangles[2];
        //VectorAdd(angleView, angleVelocity, angleView);
        SetClientViewAngle( player, angleView );
			} else {
        Com_Printf("free -> wall\n");
				// source is a free standing, destination is a wall
				// free standing portals don't need angle calculation because 
        //   viewangles is assumed, since the portal rotates to face the player
        // get the angle relative to the velocity
        vectoangles(vec, angleVelocity);
        VectorSubtract(player->client->ps.viewangles, angleVelocity, angleView);
        // change the velocity direction but maintain Z
        angleVelocity[1] = angles[1];
        AngleVectors( angleVelocity, player->client->ps.velocity, NULL, NULL );
				VectorScale( player->client->ps.velocity, normal * 1.25f, player->client->ps.velocity );
        // add the angle to the new velocity
        VectorAdd(angleView, angleVelocity, angleView);
        SetClientViewAngle( player, angleView );
			}
    } else {
			if(self->s.eventParm) {
        Com_Printf("wall -> free\n");
				// source is a wall view and destination is a free standing,
        //   only because that's what the camera client side does now?
				// TODO: spit out at the original angle the portal was set using trDelta?
				VectorScale( player->client->ps.velocity, 1.25f, player->client->ps.velocity );
        SetClientViewAngle( player, player->client->ps.viewangles );
			} else {
        Com_Printf("free -> free\n");
        // stand-alone portal to stand-alone portal the player can move around
        //   to see the the world from a different angle
				// both are free standing portals, maintain same velocity and direction
				VectorScale( player->client->ps.velocity, 1.25f, player->client->ps.velocity );
        SetClientViewAngle( player, player->client->ps.viewangles );
			}
		}
  }
	player->client->ps.pm_time = 160; // hold time
	player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// unlagged
	G_ResetHistory( player );

	// kill anything at the destination
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		G_KillBox( player );
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		trap_LinkEntity( player );
	}
}
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
  TeleportPlayer_real(player, origin, angles, NULL, NULL);
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
	char *s;
	int arena;
	G_SpawnString( "targetname", "", &s );
	if(!ent->targetname) {
		G_SpawnInt("arena", "0", &arena);
		if(arena) {
			ent->targetname = va("arena_%i", arena);
		}
	}
#ifdef USE_MULTIWORLD
	G_SpawnInt( "world", "0", &ent->world );
#endif
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) {
	vec3_t		dir;
	gentity_t	*target;
	gentity_t	*owner;

	owner = G_PickTarget( ent->target );
	if ( !owner ) {
    ent->nextthink = level.time + 500;
		G_Printf( "Couldn't find target for misc_partal_surface\n" );
		//G_FreeEntity( ent );
		return;
	}
	ent->r.ownerNum = owner->s.number;

	// frame holds the rotate speed
	if ( owner->spawnflags & 1 ) {
		ent->s.frame = 25;
	} else if ( owner->spawnflags & 2 ) {
		ent->s.frame = 75;
	}

	// swing camera ?
	if ( owner->spawnflags & 4 ) {
		// set to 0 for no rotation at all
		ent->s.powerups = 0;
	}
	else {
		ent->s.powerups = 1;
	}
  ent->s.powerups |= (owner->world << 8);

	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy( owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	target = G_PickTarget( owner->target );
	if ( target ) {
		VectorSubtract( target->s.origin, owner->s.origin, dir );
		VectorNormalize( dir );
	} else {
		G_SetMovedir( owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) {
	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if ( !ent->target ) {
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} else {
		ent->think = locateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);
#ifdef USE_MULTIWORLD
  // TODO: default "world" is the current world, store in a variable somewhere like `g_mvWorld_0..9`
	G_SpawnInt( "world", "0", &ent->world );
#endif
	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t		dir;
	float		deg;
	vec3_t		up, right;

	// see if we have a target
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
	} else {
		VectorCopy( ent->movedir, dir );
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) {
	case WP_GRENADE_LAUNCHER:
		fire_grenade( ent, ent->s.origin, dir );
		break;
	case WP_ROCKET_LAUNCHER:
		fire_rocket( ent, ent->s.origin, dir );
		break;
	case WP_PLASMAGUN:
		fire_plasma( ent, ent->s.origin, dir );
		break;
	}

	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
	ent->enemy = G_PickTarget( ent->target );
	ent->think = 0;
	ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
	ent->use = Use_Shooter;
	ent->s.weapon = weapon;

	RegisterItem( BG_FindItemForWeapon( weapon ) );

	G_SetMovedir( ent->s.angles, ent->movedir );

	if ( !ent->random ) {
		ent->random = 1.0;
	}
	ent->random = sin( M_PI * ent->random / 180 );
	// target might be a moving object, so we can't set movedir for it
	if ( ent->target ) {
		ent->think = InitShooter_Finish;
		ent->nextthink = level.time + 500;
	}
	trap_LinkEntity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) {
	InitShooter( ent, WP_ROCKET_LAUNCHER );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) {
	InitShooter( ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) {
	InitShooter( ent, WP_GRENADE_LAUNCHER);
}


#ifdef USE_PORTALS
void PortalDestroy( gentity_t *self ) {
  gclient_t *client;
  client = &level.clients[self->r.ownerNum];
  client->portalID = 0;
  // free both ends
  if(self != client->portalDestination
    && self != client->portalSource) {
    G_FreeEntity(&g_entities[self->s.otherEntityNum]);
    G_FreeEntity(self);
  }
  if(client->portalDestination) {
    G_FreeEntity(client->portalDestination);
    client->portalDestination = NULL;
  }
  if(client->portalSource) {
    G_FreeEntity(client->portalSource);
    client->portalSource = NULL;
  }
}


static void PortalDie (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod) {
	PortalDestroy( self );
}



static void PortalTouch( gentity_t *self, gentity_t *other, trace_t *trace) {
	vec3_t		vel;
	float     len;
	gentity_t	*destination;
	gclient_t *client = &level.clients[self->r.ownerNum];

	// see if we will even let other try to use it
	if( other->health <= 0 ) {
		return;
	}
	if( !other->client ) {
		return;
	}
  
  //VectorSubtract(self->r.currentOrigin, other->r.currentOrigin, vec);
  //len = VectorNormalize(vec);
  //if(len > 64) {

  if(level.time - other->client->lastPortal < 1 * 100
    // keep track of the exit point so we don't switch back and 
    //   forth a ton before there is time to move out of the way, 
    || (level.time - other->client->lastPortal < 1 * 1000
      && self == other->client->lastPortalEnt)
  ) {
    //Com_Printf("portal too soon\n");
    return;
  }
  other->client->lastPortal = level.time;

//	if( other->client->ps.persistant[PERS_TEAM] != self->spawnflags ) {
//		return;
//	}

	if ( other->items[ITEM_PW_MIN + PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_NEUTRALFLAG ), 0 );
		other->items[ITEM_PW_MIN + PW_NEUTRALFLAG] = 0;
	}
	else if ( other->items[ITEM_PW_MIN + PW_REDFLAG] ) {		// only happens in standard CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_REDFLAG ), 0 );
		other->items[ITEM_PW_MIN + PW_REDFLAG] = 0;
	}
	else if ( other->items[ITEM_PW_MIN + PW_BLUEFLAG] ) {	// only happens in standard CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_BLUEFLAG ), 0 );
		other->items[ITEM_PW_MIN + PW_BLUEFLAG] = 0;
	}

	// find the destination
  if(self == client->portalSource) {
    destination = client->portalDestination;
  } else {
    destination = client->portalSource;
  }
  other->client->lastPortalEnt = destination;

  if( self->pos1[0] || self->pos1[1] || self->pos1[2] ) {
    // if velocity is less than PORTAL_EXTRA_SPEED add it towards the portal
    //   velocity is required in the next step TeleportPlayer_real, so
    //   it appears the player is always stepping through a portal
    //   instead of partly in between
    VectorCopy(other->client->ps.velocity, vel);
    len = VectorNormalize(vel);
    if(len < PORTAL_EXTRA_SPEED) {
      VectorSubtract(other->r.currentOrigin, self->r.currentOrigin, vel);
      VectorNormalize(vel);
      //VectorScale( vel, -PORTAL_EXTRA_SPEED, vel );
      //VectorAdd(other->client->ps.velocity, vel, other->client->ps.velocity);
    }

    TeleportPlayer_real( other, self->pos1, vec3_origin, destination, self );
    return;
  }
  // if there is not one, die!
	if( !destination ) {
    if(self->damage == GIB_HEALTH) {
      Com_Printf("kill player\n");
      G_Damage( other, other, other, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
    }
		return;
	}

	TeleportPlayer_real( other, destination->s.pos.trBase, other->client->ps.viewangles, destination, self );
}

#define AWAY_FROM_WALL 16.0f
//

static void PortalEnable( gentity_t *self ) {
  gentity_t	*target = NULL;
  gclient_t *client = &level.clients[self->r.ownerNum];
  
  self->s.pos.trType = TR_STATIONARY;
	self->touch = PortalTouch;
	self->think = PortalDestroy;
	self->nextthink = level.time + 2 * 60 * 1000;

  // see if the portal_camera has a target
  if(self == client->portalSource)
  	target = client->portalDestination;
  else if (self == client->portalDestination)
    target = client->portalSource;

  if(target) {
    target->touch = PortalTouch;
  	target->think = PortalDestroy;
  	target->nextthink = level.time + 2 * 60 * 1000;

    if(self->s.eventParm) {
      vec3_t velocity, angles;
      ByteToDir( self->s.eventParm, angles );
      vectoangles( angles, angles );
      AngleVectors ( angles, velocity, NULL, NULL );
      VectorNormalize( velocity );
      VectorScale( velocity, AWAY_FROM_WALL, velocity );
      VectorAdd( self->r.currentOrigin, velocity, target->pos1 );
      VectorCopy( self->r.currentOrigin, target->s.origin2 );
    } else {
      VectorCopy( self->r.currentOrigin, target->pos1 );
      VectorCopy( self->r.currentOrigin, target->s.origin2 );
    }
  
    if(target->s.eventParm) {
      vec3_t velocity, angles;
      ByteToDir( target->s.eventParm, angles );
      vectoangles( angles, angles );
      AngleVectors ( angles, velocity, NULL, NULL );
      VectorNormalize( velocity );
      VectorScale( velocity, AWAY_FROM_WALL, velocity );
      VectorAdd( target->r.currentOrigin, velocity, self->pos1 );
      VectorCopy( target->r.currentOrigin, self->s.origin2 );
    } else {
      VectorCopy( target->r.currentOrigin, self->pos1 );
      VectorCopy( target->r.currentOrigin, self->s.origin2 );
    }

    self->s.otherEntityNum = target->s.number;
    target->s.otherEntityNum = self->s.number;
  }
}

static vec3_t	PORTAL_SIZE = { 32, 32, 32 }; // only affects trigger, not physics
//static vec3_t	PORTAL_SIZE = { 160, 160, 208 };
#define PORTAL_TIMEOUT 100
void DropPortalDestination( gentity_t *player, qboolean isWall ) {
	gentity_t	*ent;
	vec3_t		snapped, vel;
  int       len;

  if(player->client->portalDestination) {
    G_FreeEntity(player->client->portalDestination);
    player->client->portalDestination = NULL;
  }

	// create the portal destination
	ent = G_Spawn();
	ent->s.modelindex = G_ModelIndex( "models/portal/portal_blue.md3" );
	VectorCopy( player->r.currentOrigin, snapped );
  if(!isWall) {
    VectorCopy(player->client->ps.velocity, vel);
    len = VectorNormalize(vel);
    // TODO: make this and velocity change optional
    if(len < PORTAL_EXTRA_SPEED) {
      AngleVectors(player->client->ps.viewangles, vel, NULL, NULL);
      // was thinking to use this initial trajectory to "pull" the portal downwards, angle-wise
      //VectorCopy( player->client->ps.viewangles, ent->s.pos.trDelta );
      VectorNormalize(vel);
      VectorScale( vel, PORTAL_EXTRA_SPEED, vel );
      VectorAdd(player->client->ps.velocity, vel, player->client->ps.velocity);
    } else {
      VectorScale( vel, PORTAL_EXTRA_SPEED, vel );
    }
    VectorAdd(snapped, vel, snapped);
    snapped[2] += 32; // TODO: mipoint?
  } else {
    // use the angle it was shot at, player is the projectile
    // was thinking to use this initial trajectory to "pull" the portal downwards, angle-wise
    //VectorCopy( player->s.pos.trDelta, ent->s.pos.trDelta );
    //vectoangles(ent->s.pos.trDelta, ent->s.angles);
  }
	SnapVector( snapped );
	G_SetOrigin( ent, snapped );
  VectorCopy( ent->r.currentOrigin, ent->s.origin2 );
  VectorSubtract( vec3_origin, PORTAL_SIZE, ent->r.mins );
	VectorAdd( vec3_origin, PORTAL_SIZE, ent->r.maxs );
	//VectorCopy( player->r.mins, ent->r.mins );
	//VectorCopy( player->r.maxs, ent->r.maxs );

	ent->classname = "hi_portal destination";
  // give it time to settle
  //if(isWall) {
  ent->s.pos.trType = TR_STATIONARY;
  //} else {
  /*
    ent->s.pos.trTime = level.time - 50; // MISSILE_PRESTEP_TIME;
    ent->s.pos.trType = TR_GRAVITY;
    ent->s.eFlags = EF_BOUNCE;
    VectorCopy( player->r.currentOrigin, ent->s.pos.trBase );
    VectorScale( player->client->ps.velocity, PORTAL_EXTRA_SPEED, ent->s.pos.trDelta );
    SnapVector( ent->s.pos.trDelta );
  */
  //}
  ent->s.eType = ET_TELEPORT_TRIGGER;
  ent->r.svFlags = SVF_PORTAL | SVF_BROADCAST;
  ent->clipmask = CONTENTS_TRIGGER;
	ent->r.contents = CONTENTS_TRIGGER;
	ent->takedamage = qtrue;
  if(isWall) {
    ent->damage = GIB_HEALTH;
    ent->s.eventParm = DirToByte( player->movedir );
  }
	ent->health = 200;
	ent->die = PortalDie;
  ent->s.powerups = 1 << 5;
  // TODO: player world from server somehow? ent->s.powerups |= 

  // copied from misc_portal
  ent->r.ownerNum = player->client->ps.clientNum;
	ent->s.clientNum = player->client->ps.clientNum;

  // TODO: angles not used because model rotates?
	//VectorCopy( player->s.apos.trBase, ent->s.angles );

  player->client->portalDestination = ent;
  ent->nextthink = level.time + PORTAL_TIMEOUT; // give the player time to get away from it
	ent->think = PortalEnable;

	trap_LinkEntity( ent );

	player->client->portalID = ++level.portalSequence;
	ent->count = player->client->portalID;

	// give the item back so they can drop the source now
	player->client->ps.stats[STAT_HOLDABLE_ITEM] = BG_FindItem( "Portal" ) - bg_itemlist;
}

void DropPortalSource( gentity_t *player, qboolean isWall ) {
	gentity_t	*ent;
	vec3_t		snapped, vel;
  int       len;

  if(player->client->portalSource) {
    G_FreeEntity(player->client->portalSource);
    player->client->portalSource = NULL;
  }

	// create the portal source
	ent = G_Spawn();
	ent->s.modelindex = G_ModelIndex( "models/portal/portal_red.md3" );

	VectorCopy( player->r.currentOrigin, snapped );
  if(!isWall) {
    // always place portal a little bit in front of out current velocity so
    //   angle can be detected when spitting player out, never zero
    VectorCopy(player->client->ps.velocity, vel);
    len = VectorNormalize(vel);
    // TODO: make this and velocity change optional
    if(len < PORTAL_EXTRA_SPEED) {
      // use view angles if they are standing still
      AngleVectors(player->client->ps.viewangles, vel, NULL, NULL);
      VectorNormalize(vel);
      VectorScale( vel, PORTAL_EXTRA_SPEED, vel );
      VectorAdd(player->client->ps.velocity, vel, player->client->ps.velocity);
    } else {
      VectorScale( vel, PORTAL_EXTRA_SPEED, vel );
    }
    VectorAdd(snapped, vel, snapped);
    // TODO: portal should be gravity based for 
    //   1/2 second to fall to ground otherwise mid air
    snapped[2] += 32; // TODO: mipoint?
  }

	SnapVector( snapped );
	G_SetOrigin( ent, snapped );
  VectorCopy( ent->r.currentOrigin, ent->s.origin2 );
  VectorSubtract( vec3_origin, PORTAL_SIZE, ent->r.mins );
	VectorAdd( vec3_origin, PORTAL_SIZE, ent->r.maxs );
	//VectorCopy( player->r.mins, ent->r.mins );
	//VectorCopy( player->r.maxs, ent->r.maxs );

	ent->classname = "hi_portal source";
  //if(isWall) {
  ent->s.pos.trType = TR_STATIONARY;
  VectorCopy( player->s.pos.trDelta, ent->s.pos.trDelta );
  //} else {
  /*
    ent->s.pos.trTime = level.time - 50; // MISSILE_PRESTEP_TIME;
    ent->s.pos.trType = TR_GRAVITY;
    ent->s.eFlags = EF_BOUNCE;
    VectorCopy( player->r.currentOrigin, ent->s.pos.trBase );
    VectorScale( player->client->ps.velocity, PORTAL_EXTRA_SPEED, ent->s.pos.trDelta );
    SnapVector( ent->s.pos.trDelta );
  */
  //}
  ent->s.eType = ET_TELEPORT_TRIGGER;
  ent->r.svFlags = SVF_PORTAL | SVF_BROADCAST;
  ent->clipmask = CONTENTS_TRIGGER;
	ent->r.contents = CONTENTS_TRIGGER;
	ent->takedamage = qtrue;
  if(isWall) {
    ent->damage = GIB_HEALTH;
    //player->movedir[2] = 90;
    ent->s.eventParm = DirToByte( player->movedir );  // trace->plane.normal in g_missile.c
  } else {
    // TODO: use s.powerups = (1 << 3) to indicate wall or free standing portal
    //ent->damage = GIB_HEALTH;
    //ent->s.eventParm = DirToByte( player->r.currentAngles );  // trace->plane.normal in g_missile.c
  }
	ent->health = 200;
	ent->die = PortalDie;
  ent->s.powerups = 1 << 4;

  // copied from misc_portal
  ent->r.ownerNum = player->client->ps.clientNum;
	ent->s.clientNum = player->client->ps.clientNum;
  // end misc_portal

  player->client->portalSource = ent;
	ent->nextthink = level.time + PORTAL_TIMEOUT; // give the player time to get away from it
	ent->think = PortalEnable;

	trap_LinkEntity( ent );

	ent->count = player->client->portalID;
	player->client->portalID = 0;
//	ent->spawnflags = player->client->ps.persistant[PERS_TEAM];
}
#endif
