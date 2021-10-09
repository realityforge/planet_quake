// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50

#ifdef USE_CLUSTER_GRENADES
gentity_t *fire_grenade2 (gentity_t *self, vec3_t start, vec3_t dir);
#endif

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			ent->s.time = level.time / 4;
			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

#ifdef USE_VULN_RPG
  // Lancer
  ent->takedamage = qfalse;
#endif

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

#ifdef USE_CLUSTER_GRENADES
  // CCH: For cluster grenades
  if (!strcmp(ent->classname, "cgrenade")) {
    vec3_t		dir;			// CCH
  	VectorSet(dir, 20, 20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, -20, 20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, 20, -20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, -20, -20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  }
#endif

	trap_LinkEntity( ent );
}


#ifdef USE_VULN_RPG
/*
================
G_MissileDie

Lancer - Destroy a missile
================
*/
void G_MissileDie( gentity_t *self, gentity_t *inflictor,
  gentity_t *attacker, int damage, int mod ) {
  if (inflictor == self)
    return;
  self->takedamage = qfalse;
  self->think = G_ExplodeMissile;
  self->nextthink = level.time + 10;
}
#endif


#ifdef MISSIONPACK
/*
================
ProximityMine_Explode
================
*/
static void ProximityMine_Explode( gentity_t *mine ) {
	G_ExplodeMissile( mine );
	// if the prox mine has a trigger free it
	if (mine->activator) {
		G_FreeEntity(mine->activator);
		mine->activator = NULL;
	}
}

/*
================
ProximityMine_Die
================
*/
static void ProximityMine_Die( gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {
	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + 1;
}

/*
================
ProximityMine_Trigger
================
*/
void ProximityMine_Trigger( gentity_t *trigger, gentity_t *other, trace_t *trace ) {
	vec3_t		v;
	gentity_t	*mine;

	if( !other->client ) {
		return;
	}

	// trigger is a cube, do a distance test now to act as if it's a sphere
	VectorSubtract( trigger->s.pos.trBase, other->s.pos.trBase, v );
	if( VectorLength( v ) > trigger->parent->splashRadius ) {
		return;
	}


	if ( g_gametype.integer >= GT_TEAM ) {
		// don't trigger same team mines
		if (trigger->parent->s.generic1 == other->client->sess.sessionTeam) {
			return;
		}
	}

	// ok, now check for ability to damage so we don't get triggered thru walls, closed doors, etc...
	if( !CanDamage( other, trigger->s.pos.trBase ) ) {
		return;
	}

	// trigger the mine!
	mine = trigger->parent;
	mine->s.loopSound = 0;
	G_AddEvent( mine, EV_PROXIMITY_MINE_TRIGGER, 0 );
	mine->nextthink = level.time + 500;

	G_FreeEntity( trigger );
}

/*
================
ProximityMine_Activate
================
*/
static void ProximityMine_Activate( gentity_t *ent ) {
	gentity_t	*trigger;
	float		r;

	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + g_proxMineTimeout.integer;

	ent->takedamage = qtrue;
	ent->health = 1;
	ent->die = ProximityMine_Die;

	ent->s.loopSound = G_SoundIndex( "sound/weapons/proxmine/wstbtick.wav" );

	// build the proximity trigger
	trigger = G_Spawn ();

	trigger->classname = "proxmine_trigger";

	r = ent->splashRadius;
	VectorSet( trigger->r.mins, -r, -r, -r );
	VectorSet( trigger->r.maxs, r, r, r );

	G_SetOrigin( trigger, ent->s.pos.trBase );

	trigger->parent = ent;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->touch = ProximityMine_Trigger;

	trap_LinkEntity (trigger);

	// set pointer to trigger so the entity can be freed when the mine explodes
	ent->activator = trigger;
}

/*
================
ProximityMine_ExplodeOnPlayer
================
*/
static void ProximityMine_ExplodeOnPlayer( gentity_t *mine ) {
	gentity_t	*player;

	player = mine->enemy;
	player->client->ps.eFlags &= ~EF_TICKING;

	if ( player->client->invulnerabilityTime > level.time ) {
		G_Damage( player, mine->parent, mine->parent, vec3_origin, mine->s.origin, 1000, DAMAGE_NO_KNOCKBACK, MOD_JUICED );
		player->client->invulnerabilityTime = 0;
		G_TempEntity( player->client->ps.origin, EV_JUICED );
	}
	else {
		G_SetOrigin( mine, player->s.pos.trBase );
		// make sure the explosion gets to the client
		mine->r.svFlags &= ~SVF_NOCLIENT;
		mine->splashMethodOfDeath = MOD_PROXIMITY_MINE;
		G_ExplodeMissile( mine );
	}
}

/*
================
ProximityMine_Player
================
*/
static void ProximityMine_Player( gentity_t *mine, gentity_t *player ) {
	if( mine->s.eFlags & EF_NODRAW ) {
		return;
	}

	G_AddEvent( mine, EV_PROXIMITY_MINE_STICK, 0 );

	if( player->s.eFlags & EF_TICKING ) {
		player->activator->splashDamage += mine->splashDamage;
		player->activator->splashRadius *= 1.50;
		mine->think = G_FreeEntity;
		mine->nextthink = level.time;
		return;
	}

	player->client->ps.eFlags |= EF_TICKING;
	player->activator = mine;

	mine->s.eFlags |= EF_NODRAW;
	mine->r.svFlags |= SVF_NOCLIENT;
	mine->s.pos.trType = TR_LINEAR;
	VectorClear( mine->s.pos.trDelta );

	mine->enemy = player;
	mine->think = ProximityMine_ExplodeOnPlayer;
	if ( player->client->invulnerabilityTime > level.time ) {
		mine->nextthink = level.time + 2 * 1000;
	}
	else {
		mine->nextthink = level.time + 10 * 1000;
	}
}
#endif

/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;
#ifdef MISSIONPACK
	vec3_t			forward, impactpoint, bouncedir;
	int				eFlags;
#endif
	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

#ifdef USE_VULN_RPG
  // Lancer
  ent->takedamage = qfalse;
#endif

#ifdef MISSIONPACK
	if ( other->takedamage ) {
		if ( ent->s.weapon != WP_PROX_LAUNCHER ) {
			if ( other->client && other->client->invulnerabilityTime > level.time ) {
				//
				VectorCopy( ent->s.pos.trDelta, forward );
				VectorNormalize( forward );
				if (G_InvulnerabilityEffect( other, forward, ent->s.pos.trBase, impactpoint, bouncedir )) {
					VectorCopy( bouncedir, trace->plane.normal );
					eFlags = ent->s.eFlags & EF_BOUNCE_HALF;
					ent->s.eFlags &= ~EF_BOUNCE_HALF;
					G_BounceMissile( ent, trace );
					ent->s.eFlags |= eFlags;
				}
				ent->target_ent = other;
				return;
			}
		}
	}
#endif
	// impact damage
	if (other->takedamage) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage, 
				0, ent->methodOfDeath);
		}
	}

#ifdef MISSIONPACK
	if( ent->s.weapon == WP_PROX_LAUNCHER ) {
		if( ent->s.pos.trType != TR_GRAVITY ) {
			return;
		}

		// if it's a player, stick it on to them (flag them and remove this entity)
		if( other->s.eType == ET_PLAYER && other->health > 0 ) {
			ProximityMine_Player( ent, other );
			return;
		}

		SnapVectorTowards( trace->endpos, ent->s.pos.trBase );
		G_SetOrigin( ent, trace->endpos );
		ent->s.pos.trType = TR_STATIONARY;
		VectorClear( ent->s.pos.trDelta );

		G_AddEvent( ent, EV_PROXIMITY_MINE_STICK, trace->surfaceFlags );

		ent->think = ProximityMine_Activate;
		ent->nextthink = level.time + 2000;

		vectoangles( trace->plane.normal, ent->s.angles );
		ent->s.angles[0] += 90;

		// link the prox mine to the other entity
		ent->enemy = other;
		ent->die = ProximityMine_Die;
		VectorCopy(trace->plane.normal, ent->movedir);
		VectorSet(ent->r.mins, -4, -4, -4);
		VectorSet(ent->r.maxs, 4, 4, 4);
		trap_LinkEntity(ent);

		return;
	}
#endif

	if (!strcmp(ent->classname, "hook")) {
		gentity_t *nent;
		vec3_t v;

		nent = G_Spawn();
		if ( other->takedamage && other->client ) {

			G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth
		} else {
			VectorCopy(trace->endpos, v);
			G_AddEvent( nent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
			ent->enemy = NULL;
		}

		SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType = ET_GRAPPLE;

		G_SetOrigin( ent, v );
		G_SetOrigin( nent, v );

		ent->think = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity( ent );
		trap_LinkEntity( nent );

		return;
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if ( other->takedamage && other->client ) {
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
	} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
		G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
	} else {
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

#ifdef USE_CLUSTER_GRENADES
  // CCH: For cluster grenades
  if (!strcmp(ent->classname, "cgrenade")) {
    vec3_t		dir;			// CCH
  	VectorSet(dir, 20, 20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, -20, 20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, 20, -20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  	VectorSet(dir, -20, -20, 50);
  	fire_grenade2(ent->parent, ent->r.currentOrigin, dir);
  }
#endif


	trap_LinkEntity( ent );
}


/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			passent;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
	}
#ifdef MISSIONPACK
	// prox mines that left the owner bbox will attach to anything, even the owner
	else if (ent->s.weapon == WP_PROX_LAUNCHER && ent->count) {
		passent = ENTITYNUM_NONE;
	}
#endif
	else {
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}
	// trace a line from the previous position to the current position
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

	if ( tr.startsolid || tr.allsolid ) {
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
		tr.fraction = 0;
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	trap_LinkEntity( ent );

	if ( tr.fraction != 1 ) {
		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent) {
				ent->parent->client->hook = NULL;
			}
			G_FreeEntity( ent );
			return;
		}
		G_MissileImpact( ent, &tr );
		if ( ent->s.eType != ET_MISSILE ) {
			return;		// exploded
		}
	}
#ifdef MISSIONPACK
	// if the prox mine wasn't yet outside the player body
	if (ent->s.weapon == WP_PROX_LAUNCHER && !ent->count) {
		// check if the prox mine is outside the owner bbox
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, ENTITYNUM_NONE, ent->clipmask );
		if (!tr.startsolid || tr.entityNum != ent->r.ownerNum) {
			ent->count = 1;
		}
	}
#endif
	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================

/*
=================
fire_plasma

=================
*/
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "plasma";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->splashDamage = 15;
	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	// missile owner
	bolt->s.clientNum = self->s.clientNum;
	// unlagged
	bolt->s.otherEntityNum = self->s.number;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trBase );			// save net bandwidth
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}	

//=============================================================================


#ifdef USE_FLAME_THROWER
/*
=================
fire_flame
=================
*/
gentity_t *fire_flame (gentity_t *self, vec3_t start, vec3_t dir) {
  gentity_t*bolt;

  VectorNormalize (dir);

  bolt = G_Spawn();
  bolt->classname = "flame";
  bolt->nextthink = level.time + 1500;
  bolt->think = G_ExplodeMissile;
  bolt->s.eType = ET_MISSILE;
  bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
  bolt->s.weapon = WP_FLAME_THROWER;
  bolt->r.ownerNum = self->s.number;
  bolt->parent = self;
  bolt->damage = 30;
  bolt->splashDamage = 25;
  bolt->splashRadius = 45;
  bolt->methodOfDeath = MOD_FLAME_THROWER;
  bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
  bolt->clipmask = MASK_SHOT;

  bolt->s.pos.trType = TR_LINEAR;
  bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;// move a bit on the very first frame
  VectorCopy( start, bolt->s.pos.trBase );
  VectorScale( dir, 300, bolt->s.pos.trDelta );
  SnapVector( bolt->s.pos.trDelta );// save net bandwidth

  VectorCopy (start, bolt->r.currentOrigin);

  return bolt;
}
#endif


#ifdef USE_VORTEX_GRENADES
gentity_t *findradius (gentity_t *ent, vec3_t org, float rad); 

#define GSUCK_TIMING	  50			  // the think time interval of G_Suck
#define GSUCK_VELOCITY	2000			// the amount of kick each second gets
#define GSUCK_RADIUS    500
#define GSUCK_TRIGGER	  32

/*
=================
G_Suck
=================
*/
static void G_Suck( gentity_t *self ) {
	gentity_t *target;
	vec3_t start,dir,end,kvel,mins,maxs;
  int targNum[MAX_GENTITIES],num;

	target = NULL;

  //check if there are any entity's within a radius of 500 units.
  mins[0] = -GSUCK_RADIUS * 1.42;
  mins[1] = -GSUCK_RADIUS * 1.42;
  mins[2] = -GSUCK_RADIUS * 1.42;
  maxs[0] = GSUCK_RADIUS * 1.42;
  maxs[1] = GSUCK_RADIUS * 1.42;
  maxs[2] = GSUCK_RADIUS * 1.42;

  VectorAdd( self->r.currentOrigin, mins, mins );
  VectorAdd( self->r.currentOrigin, maxs, maxs );

  num = trap_EntitiesInBox(mins,maxs,targNum,MAX_GENTITIES);
  for(num--; num > 0; num--) {    // count from num-1 down to 0
  	target = &g_entities[targNum[num]];

    // target must not be vortex grenade
  	if (target == self) 
  		continue;

    // target must be a client
  	if (!target->client) 
  		continue;

    // target must not be the player who fired the vortex grenade 
  	//if (target == self->parent) 
  	//	continue;

    // target must be able to take damage
  	if (!target->takedamage) 
  		continue;
      
    // target must actually be in GSUCK_RADIUS
    if ( Distance(self->r.currentOrigin, target->r.currentOrigin) > GSUCK_RADIUS )
    	continue;

    // put target position in start
  	VectorCopy(target->r.currentOrigin, start); 
    // put grenade position in end
  	VectorCopy(self->r.currentOrigin, end); 
    // subtract start from end to get directional vector
  	VectorSubtract(end, start, dir); 
  	VectorNormalize(dir); 
    // scale directional vector by 200 and add to the targets velocity
  	VectorScale(dir, GSUCK_VELOCITY / GSUCK_TIMING, kvel);
    // make targets move direction = to directional vector.
  	//VectorCopy(dir, target->movedir); 
    // add the kick velocity to the player's velocity
    VectorAdd (target->client->ps.velocity,kvel, target->client->ps.velocity);

    // set the timer so that the other client can't cancel
    // out the movement immediately
    if ( !target->client->ps.pm_time ) {
    	target->client->ps.pm_time = GSUCK_TIMING - 1;

    	/* the next G_Suck that works here will
    	probably be the one that worked before */

    	target->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    }
       
	}

	self->nextthink = level.time + GSUCK_TIMING; 

  // check if vortext grenade is older than 20 seconds.
	if (level.time > self->wait) 
		G_ExplodeMissile( self);

  mins[0] = -GSUCK_TRIGGER * 1.42;
  mins[1] = -GSUCK_TRIGGER * 1.42;
  mins[2] = -GSUCK_TRIGGER * 1.42;
  maxs[0] = GSUCK_TRIGGER * 1.42;
  maxs[1] = GSUCK_TRIGGER * 1.42;
  maxs[2] = GSUCK_TRIGGER * 1.42;

  VectorAdd( self->r.currentOrigin, mins, mins );
  VectorAdd( self->r.currentOrigin, maxs, maxs );

  num = trap_EntitiesInBox(mins,maxs,targNum,MAX_GENTITIES);
  for(num--; num > 0; num--) {    // count from num-1 down to 0
  	target = &g_entities[targNum[num]];

  	// target must be a client
  	if (!target->client) 
  		continue;

  	// target must not be the player who fired the vortex grenade 
  	if (target == self->parent)		// makes sense here
  		continue;

  	// target must be able to take damage
  	if (!target->takedamage)
  		continue;

  	G_ExplodeMissile( self);			// EXPLODE goes the weasel!
  }
}
#endif


#ifdef USE_CLUSTER_GRENADES
/*
=================
CCH: fire_grenade2

38: 62. They will also say, `Our Lord, whosoever prepared this for us,
do thou multiply manifold his punishment in the Fire.'
--Holy Quran, translated by Maulvi Sher Ali  
=================
*/
gentity_t *fire_grenade2 (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;

	// move a bit on the very first frame
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;

	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 400, bolt->s.pos.trDelta );
	
	// save net bandwidth
	SnapVector( bolt->s.pos.trDelta );

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
#endif


#define GRENADE_DAMAGE	100		// bolt->damage for grenade
#define GRENADE_RADIUS	150		// bolt->splashRadius for grenade
/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
#ifdef USE_CLUSTER_GRENADES
  bolt->classname = "cgrenade";
#else
	bolt->classname = "grenade";
#endif
#ifdef USE_VORTEX_GRENADES
  if(g_vortexGrenades.integer) {
    bolt->nextthink = level.time + 1000; // call G_Suck in 1 second
    bolt->think = G_Suck;
    bolt->wait = level.time + 20000; // vortext grenade lifetime.
  } else
#endif
  {
  	bolt->nextthink = level.time + 2500;
  	bolt->think = G_ExplodeMissile;
  }
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
  bolt->damage = GRENADE_DAMAGE;
  bolt->splashDamage = GRENADE_DAMAGE;
  bolt->splashRadius = GRENADE_RADIUS;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	if ( self->items[ITEM_PW_MIN + PW_QUAD] )
		bolt->items[ITEM_PW_MIN + PW_QUAD] = self->items[ITEM_PW_MIN + PW_QUAD];

	// missile owner
	bolt->s.clientNum = self->s.clientNum;
	// unlagged
	bolt->s.otherEntityNum = self->s.number;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_bfg
=================
*/
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

  if ( self->items[ITEM_PW_MIN + PW_QUAD] )
		bolt->items[ITEM_PW_MIN + PW_QUAD] = self->items[ITEM_PW_MIN + PW_QUAD];

	// missile owner
	bolt->s.clientNum = self->s.clientNum;
	// unlagged
	bolt->s.otherEntityNum = self->s.number;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trBase );			// save net bandwidth
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


#ifdef USE_HOMING_MISSILE
/*
================
CCH: rocket_think

Fly like an eagle...
--"Fly Like an Eagle", Steve Miller Band
================
*/
#define ROCKET_SPEED	   600
#define ROCKET_VIS_CONE  0.95
#define ROCKET_TURNING   0.08
// 

void rocket_think( gentity_t *ent ) {
	gentity_t	*target, *tent;
	float		targetlength, tentlength;
	int		i;
	vec3_t		tentdir, targetdir, forward, midbody;
	trace_t		tr;

	target = NULL;
	targetlength = LIGHTNING_RANGE;
	// Best way to get forward vector for this rocket?
	VectorCopy(ent->s.pos.trDelta, forward);
	VectorNormalize(forward);
	for (i = 0; i < level.maxclients; i++) {
		// Here we use tent to point to potential targets
		tent = &g_entities[i];

		if (!tent->inuse) continue;
		if (tent == ent->parent) continue;
		if ( OnSameTeam( tent, ent->parent ) ) continue;

		// Aim for the body, not the feet
		midbody[0] = tent->r.currentOrigin[0] + 
			(tent->r.mins[0] + tent->r.maxs[0]) * 0.5;
		midbody[1] = tent->r.currentOrigin[1] + 
			(tent->r.mins[1] + tent->r.maxs[1]) * 0.5;
		midbody[2] = tent->r.currentOrigin[2] + 
			(tent->r.mins[2] + tent->r.maxs[2]) * 0.5;

		VectorSubtract(midbody, ent->r.currentOrigin, tentdir);
		tentlength = VectorLength(tentdir);
		if ( tentlength > targetlength ) continue;

		// Quick normalization of tentdir since 
		// we already have the length
		tentdir[0] /= tentlength;
		tentdir[1] /= tentlength;
		tentdir[2] /= tentlength;
    // this value determines how wide from it's direction it can search for 
    //   players to target
		if ( DotProduct(forward, tentdir) < ROCKET_VIS_CONE ) continue;

		trap_Trace( &tr, ent->r.currentOrigin, NULL, NULL, 
			tent->r.currentOrigin, ENTITYNUM_NONE, MASK_SHOT );

		if ( tent != &g_entities[tr.entityNum] ) continue;

		target = tent;
		targetlength = tentlength;
		VectorCopy(tentdir, targetdir);
	}

	ent->nextthink += 20;

	if (!target) return;

  // this variable determines how quickly it can change direction
	VectorMA(forward, ROCKET_TURNING, targetdir, targetdir);
	VectorNormalize(targetdir);
	VectorScale(targetdir, ROCKET_SPEED, ent->s.pos.trDelta);
}
#endif


/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
#ifdef USE_BOUNCE_RPG
  if (self->flags & FL_ROCKETBOUNCE || g_bounceRockets.integer)
    bolt->nextthink = level.time + 2500;
  else
#endif
#ifdef USE_HOMING_MISSILE
  bolt->nextthink = level.time + 20;	// CCH
  bolt->think = rocket_think;		// CCH
#else
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
#endif
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
#ifdef USE_VULN_RPG
  // Lancer
  if(g_vulnRockets.integer) {
    bolt->health = 5;
    bolt->takedamage = qtrue;
    bolt->die = G_MissileDie;
    bolt->r.contents = CONTENTS_BODY;
    VectorSet(bolt->r.mins, -10, -3, 0);
    VectorCopy(bolt->r.mins, bolt->r.absmin);
    VectorSet(bolt->r.maxs, 10, 3, 6);
    VectorCopy(bolt->r.maxs, bolt->r.absmax);
  }
  if (self->flags & FL_ROCKETBOUNCE || g_bounceRockets.integer)
  	bolt->s.eFlags = EF_BOUNCE;
#endif
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

  if ( self->items[ITEM_PW_MIN + PW_QUAD] )
		bolt->items[ITEM_PW_MIN + PW_QUAD] = self->items[ITEM_PW_MIN + PW_QUAD];

	// missile owner
	bolt->s.clientNum = self->s.clientNum;
	// unlagged
	bolt->s.otherEntityNum = self->s.number;

#ifdef USE_ACCEL_RPG
  bolt->s.pos.trType = TR_ACCEL;
	bolt->s.pos.trDuration = 500;
#else
	bolt->s.pos.trType = TR_LINEAR;
#endif
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trBase );			// save net bandwidth
#ifdef USE_HOMING_MISSILE
  VectorScale( dir, ROCKET_SPEED, bolt->s.pos.trDelta );	// CCH
#else
	VectorScale( dir, 900, bolt->s.pos.trDelta );
#endif
#ifdef USE_ACCEL_RPG
  VectorScale( dir, 50, bolt->s.pos.trDelta );
#endif
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}


/*
=================
fire_grapple
=================
*/
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*hook;
	// unlagged
	int			hooktime;

	VectorNormalize (dir);

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_MISSILE;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon = WP_GRAPPLING_HOOK;
	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	hook->clipmask = MASK_SHOT;
	hook->parent = self;
	hook->target_ent = NULL;

	// missile owner
	hook->s.clientNum = self->s.clientNum;
	// unlagged
	hook->s.otherEntityNum = self->s.number;

	if ( self->client ) {
		hooktime = self->client->pers.cmd.serverTime + MISSILE_PRESTEP_TIME;
	} else {
		hooktime = level.time - MISSILE_PRESTEP_TIME; // // move a bit on the very first frame
	}

	hook->s.pos.trType = TR_LINEAR;
	hook->s.pos.trTime = hooktime;
	VectorCopy( start, hook->s.pos.trBase );
	SnapVector( hook->s.pos.trBase );			// save net bandwidth
	VectorScale( dir, 800, hook->s.pos.trDelta );
	SnapVector( hook->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, hook->r.currentOrigin);

	self->client->hook = hook;

	return hook;
}


#ifdef MISSIONPACK
/*
=================
fire_nail
=================
*/
#define NAILGUN_SPREAD	500

gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up ) {
	gentity_t	*bolt;
	vec3_t		dir;
	vec3_t		end;
	float		r, u, scale;

	bolt = G_Spawn();
	bolt->classname = "nail";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAILGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->methodOfDeath = MOD_NAIL;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( start, bolt->s.pos.trBase );

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * NAILGUN_SPREAD * 16;
	r = cos(r) * crandom() * NAILGUN_SPREAD * 16;
	VectorMA( start, 8192 * 16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );

	scale = 555 + random() * 1800;
	VectorScale( dir, scale, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );

	VectorCopy( start, bolt->r.currentOrigin );

	return bolt;
}	


/*
=================
fire_prox
=================
*/
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t dir ) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "prox mine";
	bolt->nextthink = level.time + 3000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PROX_LAUNCHER;
	bolt->s.eFlags = 0;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_PROXIMITY_MINE;
	bolt->splashMethodOfDeath = MOD_PROXIMITY_MINE;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	// count is used to check if the prox mine left the player bbox
	// if count == 1 then the prox mine left the player bbox and can attack to it
	bolt->count = 0;

	//FIXME: we prolly wanna abuse another field
	bolt->s.generic1 = self->client->sess.sessionTeam;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
#endif
