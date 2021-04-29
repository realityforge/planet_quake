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

#include <stddef.h>

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "mathlib.h"
#include "inout.h"
#include "polylib.h"


extern int numthreads;

// counters are only bumped when running single threaded,
// because they are an awefull coherence problem
int c_active_windings;
int c_peak_windings;
int c_winding_allocs;
int c_winding_points;

#define BOGUS_RANGE WORLD_SIZE


/*
   =============
   AllocWindingAccu
   =============
 */
winding_accu_t *AllocWindingAccu( int points ){
	winding_accu_t  *w;
	int s;

	if ( points >= MAX_POINTS_ON_WINDING ) {
		Com_Error(ERR_DROP, "AllocWindingAccu failed: MAX_POINTS_ON_WINDING exceeded" );
	}

	if ( numthreads == 1 ) {
		// At the time of this writing, these statistics were not used in any way.
		c_winding_allocs++;
		c_winding_points += points;
		c_active_windings++;
		if ( c_active_windings > c_peak_windings ) {
			c_peak_windings = c_active_windings;
		}
	}
	s = sizeof( *w ) + points * sizeof( *w->p );
	w = safe_malloc( s );
	memset( w, 0, s );
	return w;
}


/*
   =============
   FreeWindingAccu
   =============
 */
void FreeWindingAccu( winding_accu_t *w ){
	if ( !w ) {
		Com_Error(ERR_DROP, "FreeWindingAccu: winding is NULL" );
	}

	if ( *( (unsigned *) w ) == 0xdeaddead ) {
		Com_Error(ERR_DROP, "FreeWindingAccu: freed a freed winding" );
	}
	*( (unsigned *) w ) = 0xdeaddead;

	if ( numthreads == 1 ) {
		c_active_windings--;
	}
	free( w );
}


/*
   =================
   BaseWindingForPlaneAccu
   =================
 */
winding_accu_t *BaseWindingForPlaneAccu( vec3_t normal, vec_t dist ){
	// The goal in this function is to replicate the behavior of the original BaseWindingForPlane()
	// function (see below) but at the same time increasing accuracy substantially.

	// The original code gave a preference for the vup vector to start out as (0, 0, 1), unless the
	// normal had a dominant Z value, in which case vup started out as (1, 0, 0).  After that, vup
	// was "bent" [along the plane defined by normal and vup] to become perpendicular to normal.
	// After that the vright vector was computed as the cross product of vup and normal.

	// I'm constructing the winding polygon points in a fashion similar to the method used in the
	// original function.  Orientation is the same.  The size of the winding polygon, however, is
	// variable in this function (depending on the angle of normal), and is larger (by about a factor
	// of 2) than the winding polygon in the original function.

	int x, i;
	vec_t max, v;
	vec3_accu_t vright, vup, org, normalAccu;
	winding_accu_t  *w;

	// One of the components of normal must have a magnitiude greater than this value,
	// otherwise normal is not a unit vector.  This is a little bit of inexpensive
	// partial error checking we can do.
	max = 0.56; // 1 / sqrt(1^2 + 1^2 + 1^2) = 0.577350269

	x = -1;
	for ( i = 0; i < 3; i++ ) {
		v = (vec_t) fabs( normal[i] );
		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		Com_Error(ERR_DROP, "BaseWindingForPlaneAccu: no dominant axis found because normal is too short" );
	}

	switch ( x ) {
	case 0:     // Fall through to next case.
	case 1:
		vright[0] = (vec_accu_t) -normal[1];
		vright[1] = (vec_accu_t) normal[0];
		vright[2] = 0;
		break;
	case 2:
		vright[0] = 0;
		vright[1] = (vec_accu_t) -normal[2];
		vright[2] = (vec_accu_t) normal[1];
		break;
	}

	// vright and normal are now perpendicular; you can prove this by taking their
	// dot product and seeing that it's always exactly 0 (with no error).

	// NOTE: vright is NOT a unit vector at this point.  vright will have length
	// not exceeding 1.0.  The minimum length that vright can achieve happens when,
	// for example, the Z and X components of the normal input vector are equal,
	// and when normal's Y component is zero.  In that case Z and X of the normal
	// vector are both approximately 0.70711.  The resulting vright vector in this
	// case will have a length of 0.70711.

	// We're relying on the fact that MAX_WORLD_COORD is a power of 2 to keep
	// our calculation precise and relatively free of floating point error.
	// [However, the code will still work fine if that's not the case.]
	VectorScaleAccu( vright, ( (vec_accu_t) MAX_WORLD_COORD ) * 4.0, vright );

	// At time time of this writing, MAX_WORLD_COORD was 65536 (2^16).  Therefore
	// the length of vright at this point is at least 185364.  In comparison, a
	// corner of the world at location (65536, 65536, 65536) is distance 113512
	// away from the origin.

	VectorCopyRegularToAccu( normal, normalAccu );
	CrossProductAccu( normalAccu, vright, vup );

	// vup now has length equal to that of vright.

	VectorScaleAccu( normalAccu, (vec_accu_t) dist, org );

	// org is now a point on the plane defined by normal and dist.  Furthermore,
	// org, vright, and vup are pairwise perpendicular.  Now, the 4 vectors
	// { (+-)vright + (+-)vup } have length that is at least sqrt(185364^2 + 185364^2),
	// which is about 262144.  That length lies outside the world, since the furthest
	// point in the world has distance 113512 from the origin as mentioned above.
	// Also, these 4 vectors are perpendicular to the org vector.  So adding them
	// to org will only increase their length.  Therefore the 4 points defined below
	// all lie outside of the world.  Furthermore, it can be easily seen that the
	// edges connecting these 4 points (in the winding_accu_t below) lie completely
	// outside the world.  sqrt(262144^2 + 262144^2)/2 = 185363, which is greater than
	// 113512.

	w = AllocWindingAccu( 4 );

	VectorSubtractAccu( org, vright, w->p[0] );
	VectorAddAccu( w->p[0], vup, w->p[0] );

	VectorAddAccu( org, vright, w->p[1] );
	VectorAddAccu( w->p[1], vup, w->p[1] );

	VectorAddAccu( org, vright, w->p[2] );
	VectorSubtractAccu( w->p[2], vup, w->p[2] );

	VectorSubtractAccu( org, vright, w->p[3] );
	VectorSubtractAccu( w->p[3], vup, w->p[3] );

	w->numpoints = 4;

	return w;
}



/*
   ==================
   CopyWindingAccuIncreaseSizeAndFreeOld
   ==================
 */
winding_accu_t *CopyWindingAccuIncreaseSizeAndFreeOld( winding_accu_t *w ){
	int i;
	winding_accu_t  *c;

	if ( !w ) {
		Com_Error(ERR_DROP, "CopyWindingAccuIncreaseSizeAndFreeOld: winding is NULL" );
	}

	c = AllocWindingAccu( w->numpoints + 1 );
	c->numpoints = w->numpoints;
	for ( i = 0; i < c->numpoints; i++ )
	{
		VectorCopyAccu( w->p[i], c->p[i] );
	}
	FreeWindingAccu( w );
	return c;
}

/*
   ==================
   CopyWindingAccuToRegular
   ==================
 */
winding_t   *CopyWindingAccuToRegular( winding_accu_t *w ){
	int i;
	winding_t   *c;

	if ( !w ) {
		Com_Error(ERR_DROP, "CopyWindingAccuToRegular: winding is NULL" );
	}

	c = AllocWinding( w->numpoints );
	c->numpoints = w->numpoints;
	for ( i = 0; i < c->numpoints; i++ )
	{
		VectorCopyAccuToRegular( w->p[i], c->p[i] );
	}
	return c;
}



/*
   =============
   ChopWindingInPlaceAccu
   =============
 */
void ChopWindingInPlaceAccu( winding_accu_t **inout, vec3_t normal, vec_t dist, vec_t crudeEpsilon ){
	vec_accu_t fineEpsilon;
	winding_accu_t  *in;
	int counts[3];
	int i, j;
	vec_accu_t dists[MAX_POINTS_ON_WINDING + 1];
	int sides[MAX_POINTS_ON_WINDING + 1];
	int maxpts;
	winding_accu_t  *f;
	vec_accu_t  *p1, *p2;
	vec_accu_t w;
	vec3_accu_t mid, normalAccu;

	// We require at least a very small epsilon.  It's a good idea for several reasons.
	// First, we will be dividing by a potentially very small distance below.  We don't
	// want that distance to be too small; otherwise, things "blow up" with little accuracy
	// due to the division.  (After a second look, the value w below is in range (0,1), but
	// graininess problem remains.)  Second, Having minimum epsilon also prevents the following
	// situation.  Say for example we have a perfect octagon defined by the input winding.
	// Say our chopping plane (defined by normal and dist) is essentially the same plane
	// that the octagon is sitting on.  Well, due to rounding errors, it may be that point
	// 1 of the octagon might be in front, point 2 might be in back, point 3 might be in
	// front, point 4 might be in back, and so on.  So we could end up with a very ugly-
	// looking chopped winding, and this might be undesirable, and would at least lead to
	// a possible exhaustion of MAX_POINTS_ON_WINDING.  It's better to assume that points
	// very very close to the plane are on the plane, using an infinitesimal epsilon amount.

	// Now, the original ChopWindingInPlace() function used a vec_t-based winding_t.
	// So this minimum epsilon is quite similar to casting the higher resolution numbers to
	// the lower resolution and comparing them in the lower resolution mode.  We explicitly
	// choose the minimum epsilon as something around the vec_t epsilon of one because we
	// want the resolution of vec_accu_t to have a large resolution around the epsilon.
	// Some of that leftover resolution even goes away after we scale to points far away.

	// Here is a further discussion regarding the choice of smallestEpsilonAllowed.
	// In the 32 float world (we can assume vec_t is that), the "epsilon around 1.0" is
	// 0.00000011921.  In the 64 bit float world (we can assume vec_accu_t is that), the
	// "epsilon around 1.0" is 0.00000000000000022204.  (By the way these two epsilons
	// are defined as VEC_SMALLEST_EPSILON_AROUND_ONE VEC_ACCU_SMALLEST_EPSILON_AROUND_ONE
	// respectively.)  If you divide the first by the second, you get approximately
	// 536,885,246.  Dividing that number by 200,000 (a typical base winding coordinate)
	// gives 2684.  So in other words, if our smallestEpsilonAllowed was chosen as exactly
	// VEC_SMALLEST_EPSILON_AROUND_ONE, you would be guaranteed at least 2000 "ticks" in
	// 64-bit land inside of the epsilon for all numbers we're dealing with.

	static const vec_accu_t smallestEpsilonAllowed = ( (vec_accu_t) VEC_SMALLEST_EPSILON_AROUND_ONE ) * 0.5;
	if ( crudeEpsilon < smallestEpsilonAllowed ) {
		fineEpsilon = smallestEpsilonAllowed;
	}
	else{fineEpsilon = (vec_accu_t) crudeEpsilon; }

	in = *inout;
	counts[0] = counts[1] = counts[2] = 0;
	VectorCopyRegularToAccu( normal, normalAccu );

	for ( i = 0; i < in->numpoints; i++ )
	{
		dists[i] = DotProductAccu( in->p[i], normalAccu ) - dist;
		if ( dists[i] > fineEpsilon ) {
			sides[i] = SIDE_FRONT;
		}
		else if ( dists[i] < -fineEpsilon ) {
			sides[i] = SIDE_BACK;
		}
		else{sides[i] = SIDE_ON; }
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	// I'm wondering if whatever code that handles duplicate planes is robust enough
	// that we never get a case where two nearly equal planes result in 2 NULL windings
	// due to the 'if' statement below.  TODO: Investigate this.
	if ( !counts[SIDE_FRONT] ) {
		FreeWindingAccu( in );
		*inout = NULL;
		return;
	}
	if ( !counts[SIDE_BACK] ) {
		return; // Winding is unmodified.
	}

	// NOTE: The least number of points that a winding can have at this point is 2.
	// In that case, one point is SIDE_FRONT and the other is SIDE_BACK.

	maxpts = counts[SIDE_FRONT] + 2; // We dynamically expand if this is too small.
	f = AllocWindingAccu( maxpts );

	for ( i = 0; i < in->numpoints; i++ )
	{
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON || sides[i] == SIDE_FRONT ) {
			if ( f->numpoints >= MAX_POINTS_ON_WINDING ) {
				Com_Error(ERR_DROP, "ChopWindingInPlaceAccu: MAX_POINTS_ON_WINDING" );
			}
			if ( f->numpoints >= maxpts ) { // This will probably never happen.
				Com_DPrintf( "WARNING: estimate on chopped winding size incorrect (no problem)\n" );
				f = CopyWindingAccuIncreaseSizeAndFreeOld( f );
				maxpts++;
			}
			VectorCopyAccu( p1, f->p[f->numpoints] );
			f->numpoints++;
			if ( sides[i] == SIDE_ON ) {
				continue;
			}
		}
		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// Generate a split point.
		p2 = in->p[( ( i + 1 ) == in->numpoints ) ? 0 : ( i + 1 )];

		// The divisor's absolute value is greater than the dividend's absolute value.
		// w is in the range (0,1).
		w = dists[i] / ( dists[i] - dists[i + 1] );

		for ( j = 0; j < 3; j++ )
		{
			// Avoid round-off error when possible.  Check axis-aligned normal.
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			}
			else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			}
			else{mid[j] = p1[j] + ( w * ( p2[j] - p1[j] ) ); }
		}
		if ( f->numpoints >= MAX_POINTS_ON_WINDING ) {
			Com_Error(ERR_DROP, "ChopWindingInPlaceAccu: MAX_POINTS_ON_WINDING" );
		}
		if ( f->numpoints >= maxpts ) { // This will probably never happen.
			Com_DPrintf( "WARNING: estimate on chopped winding size incorrect (no problem)\n" );
			f = CopyWindingAccuIncreaseSizeAndFreeOld( f );
			maxpts++;
		}
		VectorCopyAccu( mid, f->p[f->numpoints] );
		f->numpoints++;
	}

	FreeWindingAccu( in );
	*inout = f;
}
