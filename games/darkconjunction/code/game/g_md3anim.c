// g_md3anim.c
//
// JPL (jpl@ing.puc.cl) - The Dark Conjunction - ACE Team
//

#include "g_local.h"

// JPL - Animated MD3s code

void FinishSpawningMD3Anim( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	ent->s.modelindex = G_ModelIndex( ent->model );
	ent->s.eType = ET_MODELANIM;


/*	if ( ent->spawnflags & 4 )	// not suspended -> default size
	{
	}
*/

	if ( ! (ent->spawnflags & 4 )) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor

		ent->r.contents = CONTENTS_BODY;
		VectorSet( ent->r.mins, -24, -24, -24 );
		VectorSet( ent->r.maxs, 24, 24, 24 );
		{
			char modelName[MAX_QPATH];
			char		fname[MAX_QPATH];
			int			len;
			char		text[10000],*text_p;
			fileHandle_t	f;
			static char err_parse[]="Error parsing ";

			trap_GetConfigstring( CS_MODELS+ent->s.modelindex ,modelName,sizeof(modelName));
			
			strcpy(fname,modelName);
			text_p=fname+strlen(fname);

			while (*text_p!='.' && *text_p!='\\' && text_p!=fname)
				text_p--;

			if (*text_p=='.')
				*text_p=0;

			Q_strcat(fname,sizeof(fname),".cfg");

			len = trap_FS_FOpenFile( fname, &f, FS_READ );
			if ( len <= 0 )
			{
				G_Printf("No config file found for %s , defaults will be used.\n",modelName);
				goto read_times;
			}
			if ( len >= sizeof( text ) - 1 )
			{
				G_Printf( "File %s too long\n", fname );
				goto read_times;
			}
			trap_FS_Read( text, len, f );
			text[len] = 0;
			trap_FS_FCloseFile( f );
			
			text_p=text;

			Parse1DMatrix (&text_p, 3, ent->r.mins);
			Parse1DMatrix (&text_p, 3, ent->r.maxs);
		read_times:
			;
		}


		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			G_Printf ("FinishSpawningMD3Anim: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// load config file

//	G_Printf("%s %s\n",vtos(ent->r.mins),vtos(ent->r.maxs));


	trap_LinkEntity (ent);
}

void ActivateAnimModel(gentity_t *ent,gentity_t *other,gentity_t *activator) {
	ent->s.modelindex2++;
}

/*QUAKED misc_model_anim (1 0 0) (-16 -16 -16) (16 16 16) HIDDEN_START HIDDEN_END NOTSUSPENDED
Animated MD3, select HIDDEN_START for the model to be hidden until de animation is triggered, and HIDDEN_END for disappearing after it's finished.
"model"		arbitrary .md3 file to display/animate
"speed"		speed scale (not less than 1.0) - used to scale ".anim" fps data (default = 1)
"wait"		1=wait for trigger activation  0=animate w/o waiting (default = 0)
*/

void SP_misc_model_anim( gentity_t *ent ) {

	G_SpawnFloat( "speed", "1.0", &ent->speed);
	if (ent->speed<1) ent->speed=1;

	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningMD3Anim;

	G_SpawnFloat( "wait", "0", &ent->wait);
	ent->s.modelindex2=0;
	ent->s.generic1=ent->spawnflags + ((int)(ent->speed * 16) & 0xF0);	// solo 3 flags
	if (ent->wait==0)
		ActivateAnimModel(ent,ent,ent);
	else
		ent->use = ActivateAnimModel;

}

void G_RunMD3Anim( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
		ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		G_FreeEntity( ent );
		return;
	}
}

// JPL - end



