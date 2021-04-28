/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define BSPFILE_ABSTRACT_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   this file was copied out of the common directory in order to not break
   compatibility with the q3map 1.x tree. it was moved out in order to support
   the raven bsp format (RBSP) used in soldier of fortune 2 and jedi knight 2.

   since each game has its own set of particular features, the data structures
   below no longer directly correspond to the binary format of a particular game.

   the translation will be done at bsp load/save time to keep any sort of
   special-case code messiness out of the rest of the program.

   ------------------------------------------------------------------------------- */



/* FIXME: remove the functions below that handle memory management of bsp file chunks */


void BSPFilesCleanup(){
	if ( bspDrawVerts != 0 ) {
		free( bspDrawVerts );
	}
	if ( bspDrawSurfaces != 0 ) {
		free( bspDrawSurfaces );
	}
	if ( bspLightBytes != 0 ) {
		free( bspLightBytes );
	}
	if ( bspGridPoints != 0 ) {
		free( bspGridPoints );
	}
}








/*
   GetLump()
   returns a pointer to the specified lump
 */

void *GetLump( bspHeader_t *header, int lump ){
	return (void*)( (byte*) header + header->lumps[ lump ].fileofs );
}






/* -------------------------------------------------------------------------------

   entity data handling

   ------------------------------------------------------------------------------- */




/*
   ParseEpair()
   parses a single quoted "key" "value" pair into an epair struct
 */

epair_t *ParseEPair( void ){
	epair_t     *e;


	/* allocate and clear new epair */
	e = safe_malloc( sizeof( epair_t ) );
	memset( e, 0, sizeof( epair_t ) );

	/* handle key */
	if ( strlen( token ) >= ( MAX_KEY - 1 ) ) {
		Com_Error(ERR_DROP, "ParseEPair: token too long" );
	}

	e->key = CopyString( token );
	GetToken( qfalse );

	/* handle value */
	if ( strlen( token ) >= MAX_VALUE - 1 ) {
		Com_Error(ERR_DROP, "ParseEpar: token too long" );
	}
	e->value = CopyString( token );

	/* strip trailing spaces that sometimes get accidentally added in the editor */
	StripTrailing( e->key );
	StripTrailing( e->value );

	/* return it */
	return e;
}


/*
   IntForKey()
   gets the integer point value for an entity key
 */

int IntForKey( const bspEntity_t *ent, const char *key ){
	const char  *k;


	k = ValueForKey( ent, key );
	return atoi( k );
}




/*
   FindTargetEntity()
   finds an entity target
 */

bspEntity_t *FindTargetEntity( const char *target ){
	int i;
	const char  *n;


	/* walk entity list */
	for ( i = 0; i < numEntities; i++ )
	{
		n = ValueForKey( &entities[ i ], "targetname" );
		if ( !strcmp( n, target ) ) {
			return &entities[ i ];
		}
	}

	/* nada */
	return NULL;
}



/*
   GetEntityShadowFlags() - ydnar
   gets an entity's shadow flags
   note: does not set them to defaults if the keys are not found!
 */

void GetEntityShadowFlags( const bspEntity_t *ent, const bspEntity_t *ent2, int *castShadows, int *recvShadows ){
	const char  *value;

	/* get cast shadows */
	if ( castShadows != NULL ) {
		value = ValueForKey( ent, "_castShadows" );
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent, "_cs" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent2, "_castShadows" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent2, "_cs" );
		}
		if ( value[ 0 ] != '\0' ) {
			*castShadows = atoi( value );
		}
	}

	/* receive */
	if ( recvShadows != NULL ) {
		value = ValueForKey( ent, "_receiveShadows" );
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent, "_rs" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent2, "_receiveShadows" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( ent2, "_rs" );
		}
		if ( value[ 0 ] != '\0' ) {
			*recvShadows = atoi( value );
		}
	}
}
