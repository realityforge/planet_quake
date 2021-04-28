/*
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
 */

// mathlib.c -- math primitives
#include "../mathlib.h"
// we use memcpy and memset
#include <memory.h>

/*
   ================
   VectorIsOnAxis
   ================
 */
qboolean VectorIsOnAxis( vec3_t v ){
	int i, zeroComponentCount;

	zeroComponentCount = 0;
	for ( i = 0; i < 3; i++ )
	{
		if ( v[i] == 0.0 ) {
			zeroComponentCount++;
		}
	}

	if ( zeroComponentCount > 1 ) {
		// The zero vector will be on axis.
		return qtrue;
	}

	return qfalse;
}

/*
   ================
   VectorIsOnAxialPlane
   ================
 */
qboolean VectorIsOnAxialPlane( vec3_t v ){
	int i;

	for ( i = 0; i < 3; i++ )
	{
		if ( v[i] == 0.0 ) {
			// The zero vector will be on axial plane.
			return qtrue;
		}
	}

	return qfalse;
}

/*
   // FIXME TTimo this implementation has to be particular to radiant
   //   through another name I'd say
   vec_t Q_rint (vec_t in)
   {
   if (g_PrefsDlg.m_bNoClamp)
    return in;
   else
    return (float)floor (in + 0.5);
   }
 */

void _CrossProduct( vec3_t v1, vec3_t v2, vec3_t cross ){
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

vec_t ColorNormalize( const vec3_t in, vec3_t out ) {
	float max, scale;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( max == 0 ) {
		out[0] = out[1] = out[2] = 1.0;
		return 0;
	}

	scale = 1.0f / max;

	VectorScale( in, scale, out );

	return max;
}

/*
   void VectorScale (vec3_t v, vec_t scale, vec3_t out)
   {
    out[0] = v[0] * scale;
    out[1] = v[1] * scale;
    out[2] = v[2] * scale;
   }
 */

void VectorRotateOrigin( vec3_t vIn, vec3_t vRotation, vec3_t vOrigin, vec3_t out ){
	vec3_t vTemp, vTemp2;
	vec3_t in[3] = {{*vRotation, 0, 0}};

	VectorSubtract( vIn, vOrigin, vTemp );
	VectorRotate( vTemp, in, vTemp2 );
	VectorAdd( vTemp2, vOrigin, out );
}

void VectorPolar( vec3_t v, float radius, float theta, float phi ){
	v[0] = (float)( radius * cos( theta ) * cos( phi ) );
	v[1] = (float)( radius * sin( theta ) * cos( phi ) );
	v[2] = (float)( radius * sin( phi ) );
}

void VectorSnap( vec3_t v ){
	int i;
	for ( i = 0; i < 3; i++ )
	{
		v[i] = (vec_t)floor( v[i] + 0.5 );
	}
}

void VectorISnap( vec3_t point, int snap ){
	int i;
	for ( i = 0 ; i < 3 ; i++ )
	{
		point[i] = (vec_t)floor( point[i] / snap + 0.5 ) * snap;
	}
}

void VectorFSnap( vec3_t point, float snap ){
	int i;
	for ( i = 0 ; i < 3 ; i++ )
	{
		point[i] = (vec_t)floor( point[i] / snap + 0.5 ) * snap;
	}
}

void _Vector5Add( vec5_t va, vec5_t vb, vec5_t out ){
	out[0] = va[0] + vb[0];
	out[1] = va[1] + vb[1];
	out[2] = va[2] + vb[2];
	out[3] = va[3] + vb[3];
	out[4] = va[4] + vb[4];
}

void _Vector5Scale( vec5_t v, vec_t scale, vec5_t out ){
	out[0] = v[0] * scale;
	out[1] = v[1] * scale;
	out[2] = v[2] * scale;
	out[3] = v[3] * scale;
	out[4] = v[4] * scale;
}

void _Vector53Copy( vec5_t in, vec3_t out ){
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}


#define PITCH               0       // up / down
#define YAW                 1       // left / right
#define ROLL                2       // fall over
#ifndef M_PI
#define M_PI        3.14159265358979323846f // matches value in gcc v2 math.h
#endif

void VectorToAngles( vec3_t vec, vec3_t angles ){
	float forward;
	float yaw, pitch;

	if ( ( vec[ 0 ] == 0 ) && ( vec[ 1 ] == 0 ) ) {
		yaw = 0;
		if ( vec[ 2 ] > 0 ) {
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		yaw = (vec_t)atan2( vec[ 1 ], vec[ 0 ] ) * 180 / M_PI;
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = ( float )sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] );
		pitch = (vec_t)atan2( vec[ 2 ], forward ) * 180 / M_PI;
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles[ 0 ] = pitch;
	angles[ 1 ] = yaw;
	angles[ 2 ] = 0;
}


/*
** NormalToLatLong
**
** We use two byte encoded normals in some space critical applications.
** Lat = 0 at (1,0,0) to 360 (-1,0,0), encoded in 8-bit sine table format
** Lng = 0 at (0,0,1) to 180 (0,0,-1), encoded in 8-bit sine table format
**
*/
void NormalToLatLong( const vec3_t normal, byte bytes[2] ) {
	// check for singularities
	if ( normal[0] == 0 && normal[1] == 0 ) {
		if ( normal[2] > 0 ) {
			bytes[0] = 0;
			bytes[1] = 0;       // lat = 0, long = 0
		}
		else {
			bytes[0] = 128;
			bytes[1] = 0;       // lat = 0, long = 128
		}
	}
	else {
		int a, b;

		a = (int)( RAD2DEG( atan2( normal[1], normal[0] ) ) * ( 255.0f / 360.0f ) );
		a &= 0xff;

		b = (int)( RAD2DEG( acos( normal[2] ) ) * ( 255.0f / 360.0f ) );
		b &= 0xff;

		bytes[0] = b;   // longitude
		bytes[1] = a;   // lattitude
	}
}


////////////////////////////////////////////////////////////////////////////////
// Below is double-precision math stuff.  This was initially needed by the new
// "base winding" code in q3map2 brush processing in order to fix the famous
// "disappearing triangles" issue.  These definitions can be used wherever extra
// precision is needed.
////////////////////////////////////////////////////////////////////////////////

/*
   =================
   VectorLengthAccu
   =================
 */
vec_accu_t VectorLengthAccu( const vec3_accu_t v ){
	return (vec_accu_t) sqrt( ( v[0] * v[0] ) + ( v[1] * v[1] ) + ( v[2] * v[2] ) );
}

/*
   =================
   DotProductAccu
   =================
 */
vec_accu_t DotProductAccu( const vec3_accu_t a, const vec3_accu_t b ){
	return ( a[0] * b[0] ) + ( a[1] * b[1] ) + ( a[2] * b[2] );
}

/*
   =================
   VectorSubtractAccu
   =================
 */
void VectorSubtractAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out ){
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

/*
   =================
   VectorAddAccu
   =================
 */
void VectorAddAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out ){
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

/*
   =================
   VectorCopyAccu
   =================
 */
void VectorCopyAccu( const vec3_accu_t in, vec3_accu_t out ){
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

/*
   =================
   VectorScaleAccu
   =================
 */
void VectorScaleAccu( const vec3_accu_t in, vec_accu_t scaleFactor, vec3_accu_t out ){
	out[0] = in[0] * scaleFactor;
	out[1] = in[1] * scaleFactor;
	out[2] = in[2] * scaleFactor;
}

/*
   =================
   CrossProductAccu
   =================
 */
void CrossProductAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out ){
	out[0] = ( a[1] * b[2] ) - ( a[2] * b[1] );
	out[1] = ( a[2] * b[0] ) - ( a[0] * b[2] );
	out[2] = ( a[0] * b[1] ) - ( a[1] * b[0] );
}

/*
   =================
   Q_rintAccu
   =================
 */
vec_accu_t Q_rintAccu( vec_accu_t val ){
	return (vec_accu_t) floor( val + 0.5 );
}

/*
   =================
   VectorCopyAccuToRegular
   =================
 */
void VectorCopyAccuToRegular( const vec3_accu_t in, vec3_t out ){
	out[0] = (vec_t) in[0];
	out[1] = (vec_t) in[1];
	out[2] = (vec_t) in[2];
}

/*
   =================
   VectorCopyRegularToAccu
   =================
 */
void VectorCopyRegularToAccu( const vec3_t in, vec3_accu_t out ){
	out[0] = (vec_accu_t) in[0];
	out[1] = (vec_accu_t) in[1];
	out[2] = (vec_accu_t) in[2];
}

/*
   =================
   VectorNormalizeAccu
   =================
 */
vec_accu_t VectorNormalizeAccu( const vec3_accu_t in, vec3_accu_t out ){
	// The sqrt() function takes double as an input and returns double as an
	// output according the the man pages on Debian and on FreeBSD.  Therefore,
	// I don't see a reason why using a double outright (instead of using the
	// vec_accu_t alias for example) could possibly be frowned upon.

	vec_accu_t length;

	length = (vec_accu_t) sqrt( ( in[0] * in[0] ) + ( in[1] * in[1] ) + ( in[2] * in[2] ) );
	if ( length == 0 ) {
		VectorClear( out );
		return 0;
	}

	out[0] = in[0] / length;
	out[1] = in[1] / length;
	out[2] = in[2] / length;

	return length;
}
