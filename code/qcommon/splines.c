/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

//#include "stdafx.h"
//#include "qe3.h"

#include "q_shared.h"
//#include "splines.h"

int FS_Write( const void *buffer, int len, fileHandle_t h );
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );
fileHandle_t FS_FOpenFileWrite( const char *filename );
void FS_FCloseFile( fileHandle_t f );


//#include "../shared/windings.h"
//#include "../qcommon/qcommon.h"
//#include "../sys/sys_public.h"
//#include "../game/game_entity.h"
typedef enum {
	EVENT_NA = 0x00,
	EVENT_WAIT,
	EVENT_TARGETWAIT,
	EVENT_SPEED,
	EVENT_TARGET,
	EVENT_SNAPTARGET,
	EVENT_FOV,
	EVENT_SCRIPT,
	EVENT_TRIGGER,
	EVENT_STOP,
	EVENT_COUNT
} eventType;

typedef enum {
	CP_FIXED = 0x00,
	CP_INTERPOLATED,
	CP_SPLINE,
	POSITION_COUNT
} positionType;

typedef struct {
	char name[MAX_QPATH];
	vec3_t *controlPoints;
	int    numControlPoints;
	vec3_t *splinePoints;
	int    numSplinePoints;
	double *splineTime;
	vec3_t *selected;
	int    numSelected;
	vec3_t pathColor, segmentColor, controlColor, activeColor;
	float granularity;
	qboolean editMode;
	qboolean dirty;
	int activeSegment;
	long baseTime;
	long time;
} idSplineList;

typedef struct {
	long	startTime;
	long	time;
	float	speed;
} idVelocity;

typedef struct {
	vec3_t *points;
	int numPoints;
	const char* positionStr[POSITION_COUNT];
	long		startTime;
	long		time;
	positionType type;
	char		name[MAX_QPATH];
	qboolean	editMode;
	idVelocity **velocities;
	int numVelocities;
	float		baseVelocity;
} idCameraPosition;

typedef struct {
	idCameraPosition *base;
	vec3_t			     pos;
} idFixedPosition;

typedef struct {
	idCameraPosition *pos;
	qboolean first;
	vec3_t startPos;
	vec3_t endPos;
	long lastTime;
	float distSoFar;
} idInterpolatedPosition;

typedef struct {
	idCameraPosition *pos;
	idSplineList *target;
} idSplinePosition;

typedef struct {
	float fov;
	float startFOV;
	float endFOV;
	int startTime;
	int time;
} idCameraFOV;

typedef struct {
	eventType type;
	char paramStr[MAX_QPATH];
	long time;
	qboolean triggered;
} idCameraEvent;

typedef struct {
	char name[MAX_QPATH];
	int currentCameraPosition;
	vec3_t lastDirection;
	qboolean cameraRunning;
	idCameraPosition *cameraPosition;
	int numCameraPositions;
	idCameraPosition *targetPositions;
	int numTargetPositions;
	idCameraEvent *events;
	int numEvents;
	idCameraFOV *fov;
	int activeTarget;
	float totalTime;
	float baseTime;
	long startTime;

	qboolean cameraEdit;
	qboolean editMode;
} idCameraDef;

idCameraDef splineList;
idCameraDef *g_splineList = &splineList;

/*
void glLabeledPoint(vec3_t &color, vec3_t &point, float size, const char *label) {
	qglColor3fv(color);
	qglPointSize(size);
	qglBegin(GL_POINTS);
	qglVertex3fv(point);
	qglEnd();
	vec3_t v = point;
	v.x += 1;
	v.y += 1;
	v.z += 1;
	qglRasterPos3fv (v);
	qglCallLists (strlen(label), GL_UNSIGNED_BYTE, label);
}


void glBox(vec3_t &color, vec3_t &point, float size) {
	vec3_t mins(point);
	vec3_t maxs(point);
	mins[0] -= size;
	mins[1] += size;
	mins[2] -= size;
	maxs[0] += size;
	maxs[1] -= size;
	maxs[2] += size;
	qglColor3fv(color);
	qglBegin(GL_LINE_LOOP);
	qglVertex3f(mins[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],maxs[1],mins[2]);
	qglVertex3f(mins[0],maxs[1],mins[2]);
	qglEnd();
	qglBegin(GL_LINE_LOOP);
	qglVertex3f(mins[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],maxs[2]);
	qglEnd();

	qglBegin(GL_LINES);
  	qglVertex3f(mins[0],mins[1],mins[2]);
	qglVertex3f(mins[0],mins[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],mins[2]);
	qglEnd();

}

void splineTest() {
	//g_splineList->load("p:/doom/base/maps/test_base1.camera");
}

void splineDraw() {
	//g_splineList->addToRenderer();
}


//extern void D_DebugLine( const vec3_t &color, const vec3_t &start, const vec3_t &end );

void debugLine(vec3_t &color, float x, float y, float z, float x2, float y2, float z2) {
	//vec3_t from(x, y, z);
	//vec3_t to(x2, y2, z2);
	//D_DebugLine(color, from, to);
}


void idSplineList::addToRenderer() {

	if (controlPoints.Num() == 0) {
		return;
	}

	vec3_t mins, maxs;
	vec3_t yellow(1.0, 1.0, 0);
	vec3_t white(1.0, 1.0, 1.0);
        int i;
        
	for(i = 0; i < controlPoints.Num(); i++) {
		VectorCopy(*controlPoints[i], mins);
		VectorCopy(mins, maxs);
		mins[0] -= 8;
		mins[1] += 8;
		mins[2] -= 8;
		maxs[0] += 8;
		maxs[1] -= 8;
		maxs[2] += 8;
		debugLine( yellow, mins[0], mins[1], mins[2], maxs[0], mins[1], mins[2]);
		debugLine( yellow, maxs[0], mins[1], mins[2], maxs[0], maxs[1], mins[2]);
		debugLine( yellow, maxs[0], maxs[1], mins[2], mins[0], maxs[1], mins[2]);
		debugLine( yellow, mins[0], maxs[1], mins[2], mins[0], mins[1], mins[2]);
		
		debugLine( yellow, mins[0], mins[1], maxs[2], maxs[0], mins[1], maxs[2]);
		debugLine( yellow, maxs[0], mins[1], maxs[2], maxs[0], maxs[1], maxs[2]);
		debugLine( yellow, maxs[0], maxs[1], maxs[2], mins[0], maxs[1], maxs[2]);
		debugLine( yellow, mins[0], maxs[1], maxs[2], mins[0], mins[1], maxs[2]);
	    
	}

	int step = 0;
	vec3_t step1;
	for(i = 3; i < controlPoints.Num(); i++) {
		for (float tension = 0.0f; tension < 1.001f; tension += 0.1f) {
			float x = 0;
			float y = 0;
			float z = 0;
			for (int j = 0; j < 4; j++) {
				x += controlPoints[i - (3 - j)]->x * calcSpline(j, tension);
				y += controlPoints[i - (3 - j)]->y * calcSpline(j, tension);
				z += controlPoints[i - (3 - j)]->z * calcSpline(j, tension);
			}
			if (step == 0) {
				step1[0] = x;
				step1[1] = y;
				step1[2] = z;
				step = 1;
			} else {
				debugLine( white, step1[0], step1[1], step1[2], x, y, z);
				step = 0;
			}

		}
	}
}
*/

float calcSpline(int step, float tension) {
	switch(step) {
		case 0:	return (pow(1 - tension, 3)) / 6;
		case 1:	return (3 * pow(tension, 3) - 6 * pow(tension, 2) + 4) / 6;
		case 2:	return (-3 * pow(tension, 3) + 3 * pow(tension, 2) + 3 * tension + 1) / 6;
		case 3:	return pow(tension, 3) / 6;
	}
	return 0.0;
}

void buildSpline(idSplineList *spline) {
	//int start = Sys_Milliseconds();
	spline->numSplinePoints = 0;
	for(int i = 3; i < spline->numControlPoints; i++) {
		for (float tension = 0.0f; tension < 1.001f; tension += spline->granularity) {
			float x = 0;
			float y = 0;
			float z = 0;
			for (int j = 0; j < 4; j++) {
				x += spline->controlPoints[i - (3 - j)][0] * calcSpline(j, tension);
				y += spline->controlPoints[i - (3 - j)][1] * calcSpline(j, tension);
				z += spline->controlPoints[i - (3 - j)][2] * calcSpline(j, tension);
			}
			spline->splinePoints[spline->numSplinePoints][0] = x;
			spline->splinePoints[spline->numSplinePoints][1] = y;
			spline->splinePoints[spline->numSplinePoints][2] = z;
			spline->numSplinePoints++;
		}
	}
	spline->dirty = qfalse;
	//Com_Printf("Spline build took %f seconds\n", (float)(Sys_Milliseconds() - start) / 1000);
}

/*
void idSplineList::draw(bool editMode) {
	int i;
	vec4_t yellow(1, 1, 0, 1);
        
	if (controlPoints.Num() == 0) {
		return;
	}

	if (dirty) {
		buildSpline();
	}


	qglColor3fv(controlColor);
	qglPointSize(5);
	
	qglBegin(GL_POINTS);
	for (i = 0; i < controlPoints.Num(); i++) {
		qglVertex3fv(*controlPoints[i]);
	}
	qglEnd();
	
	if (editMode) {
		for(i = 0; i < controlPoints.Num(); i++) {
			glBox(activeColor, *controlPoints[i], 4);
		}
	}

	//Draw the curve
	qglColor3fv(pathColor);
	qglBegin(GL_LINE_STRIP);
	int count = splinePoints.Num();
	for (i = 0; i < count; i++) {
		qglVertex3fv(*splinePoints[i]);
	}
	qglEnd();

	if (editMode) {
		qglColor3fv(segmentColor);
		qglPointSize(3);
		qglBegin(GL_POINTS);
		for (i = 0; i < count; i++) {
			qglVertex3fv(*splinePoints[i]);
		}
		qglEnd();
	}
	if (count > 0) {
		//assert(activeSegment >=0 && activeSegment < count);
		if (activeSegment >=0 && activeSegment < count) {
			glBox(activeColor, *splinePoints[activeSegment], 6);
			glBox(yellow, *splinePoints[activeSegment], 8);
		}
	}

}
*/

float totalDistance(idSplineList *spline) {

	if (spline->numControlPoints == 0) {
		return 0.0;
	}

	if (spline->dirty) {
		buildSpline(spline);
	}

	float dist = 0.0;
	vec3_t temp;
	int count = spline->numSplinePoints;
	for(int i = 1; i < count; i++) {
		VectorCopy(spline->splinePoints[i-1], temp);
		VectorSubtract(temp, spline->splinePoints[i], temp);
		dist += VectorLength(temp);
	}
	return dist;
}

void initPosition(long bt, long totalTime, idSplineList *spline) {

	if (spline->dirty) {
		buildSpline(spline);
	}

	if (spline->numSplinePoints == 0) {
		return;
	}

	spline->baseTime = bt;
	spline->time = totalTime;

	// calc distance to travel ( this will soon be broken into time segments )
	spline->splineTime[0] = bt;
	double dist = totalDistance(spline);
	double distSoFar = 0.0;
	vec3_t temp;
	int count = spline->numSplinePoints;
	//for(int i = 2; i < count - 1; i++) {
	for(int i = 1; i < count; i++) {
		VectorCopy(spline->splinePoints[i-1], temp);
		VectorSubtract(temp, spline->splinePoints[i], temp);
		distSoFar += VectorLength(temp);
		double percent = distSoFar / dist;
		percent *= totalTime;
		spline->splineTime[i] = percent + bt;
	}
	//assert(splineTime.Num() == splinePoints.Num());
	spline->activeSegment = 0;
}


void updateSelection(const vec3_t move, idSplineList *spline) {
	if (spline->selected) {
		spline->dirty = qtrue;
		VectorAdd(*spline->selected, move, *spline->selected);
	}
}


void setSelectedPoint(vec3_t *p, idSplineList *spline) {
	if (p) {
		SnapVector(*p);
		for(int i = 0; i < spline->numControlPoints; i++) {
			if (!VectorCompare(*p, spline->controlPoints[i])) {
				spline->selected = &spline->controlPoints[i];
			}
		}
	} else {
		spline->selected = NULL;
	}
}

const vec3_t *getPosition(long t, idSplineList *spline) {
	static vec3_t interpolatedPos;
	//static long lastTime = -1;

	int count = spline->numSplinePoints;
	if (count == 0) {
		return &vec3_origin;
	}

	Com_Printf("Time: %ld\n", t);
	//assert(splineTime.Num() == splinePoints.Num());

	while (spline->activeSegment < count) {
		if (spline->splineTime[spline->activeSegment] >= t) {
			if (spline->activeSegment > 0 && spline->activeSegment < count - 1) {
				double timeHi = spline->splineTime[spline->activeSegment + 1];
				double timeLo = spline->splineTime[spline->activeSegment - 1];
				double percent = (timeHi - t) / (timeHi - timeLo); 
				// pick two bounding points
				vec3_t v1, v2;
				VectorCopy(spline->splinePoints[spline->activeSegment - 1], v1);
				VectorCopy(spline->splinePoints[spline->activeSegment + 1], v2);
				VectorScale(v2, 1.0 - percent, v2);
				VectorScale(v1, percent, v1);
				VectorAdd(v2, v1, v2);
				VectorCopy(v2, interpolatedPos);
				return &interpolatedPos;
			}
			return &spline->splinePoints[spline->activeSegment];
		} else {
			spline->activeSegment++;
		}
	}
	return &spline->splinePoints[count-1];
}

void parseSplines(const char *(*text), idSplineList *spline ) {
	const char *token;
	//Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !Q_stricmp (token, "(") || !Q_stricmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "granularity") == 0) {
				granularity = atof(token);
			} else if (Q_stricmp(key.c_str(), "name") == 0) {
				name = token;
			}
			token = Com_Parse(text);

		} while (1);

		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		Com_UngetToken();
		// read the control point
		vec3_t point;
		Com_Parse1DMatrix( text, 3, point );
		addPoint(point[0], point[1], point[2]);
	} while (1);
 
	//Com_UngetToken();
	//Com_MatchToken( text, "}" );
	dirty = qtrue;
}

void idSplineList::write(fileHandle_t file, const char *p) {
	idStr s = va("\t\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	//s = va("\t\tname %s\n", name.c_str());
	//FS_Write(s.c_str(), s.length(), file);
	s = va("\t\t\tgranularity %f\n", granularity);
	FS_Write(s.c_str(), s.length(), file);
	int count = controlPoints.Num();
	for (int i = 0; i < count; i++) {
		s = va("\t\t\t( %f %f %f )\n", controlPoints[i]->x, controlPoints[i]->y, controlPoints[i]->z);
		FS_Write(s.c_str(), s.length(), file);
	}
	s = "\t\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


void idCameraDef::getActiveSegmentInfo(int segment, vec3_t &origin, vec3_t &direction, float *fov) {
#if 0
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	double d = (double)segment / numSegments();
	getCameraInfo(d * totalTime * 1000, origin, direction, fov);
#endif
/*
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	origin = *cameraSpline.getSegmentPoint(segment);
	

	vec3_t temp;

	int numTargets = getTargetSpline()->controlPoints.Num();
	int count = cameraSpline.splineTime.Num();
	if (numTargets == 0) {
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
		}
	} else if (numTargets == 1) {
		temp = *getTargetSpline()->controlPoints[0];
	} else {
		temp = *getTargetSpline()->getSegmentPoint(segment);
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;
*/
}

bool idCameraDef::getCameraInfo(long time, vec3_t &origin, vec3_t &direction, float *fv) {


	if ((time - startTime) / 1000 > totalTime) {
		return false;
	}


	for (int i = 0; i < events.Num(); i++) {
		if (time >= startTime + events[i]->getTime() && !events[i]->getTriggered()) {
			events[i]->setTriggered(true);
			if (events[i]->getType() == idCameraEvent::EVENT_TARGET) {
				setActiveTargetByName(events[i]->getParam());
				getActiveTarget()->start(startTime + events[i]->getTime());
				//Com_Printf("Triggered event switch to target: %s\n",events[i]->getParam());
			} else if (events[i]->getType() == idCameraEvent::EVENT_TRIGGER) {
				//idEntity *ent = NULL;
				//ent = level.FindTarget( ent, events[i]->getParam());
				//if (ent) {
				//	ent->signal( SIG_TRIGGER );
				//	ent->ProcessEvent( &EV_Activate, world );
				//}
			} else if (events[i]->getType() == idCameraEvent::EVENT_FOV) {
				//*fv = fov = atof(events[i]->getParam());
			} else if (events[i]->getType() == idCameraEvent::EVENT_STOP) {
				return false;
			}
		}
	}

	origin = *cameraPosition->getPosition(time);
	
	*fv = fov.getFOV(time);

	vec3_t temp = origin;

	int numTargets = targetPositions.Num();
	if (numTargets == 0) {
/*
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
			if (temp == origin) {
				int index = cameraSpline.getActiveSegment() + 2;
				while (temp == origin && index < count - 1) {
					temp = *cameraSpline.splinePoints[index++];
				}
			}
		}
*/
	} else {
		temp = *getActiveTarget()->getPosition(time);
	}
	
	temp -= origin;
	temp.Normalize();
	direction = temp;

	return true;
}

bool idCameraDef::waitEvent(int index) {
	//for (int i = 0; i < events.Num(); i++) {
	//	if (events[i]->getSegment() == index && events[i]->getType() == idCameraEvent::EVENT_WAIT) {
	//		return true;
	//	}
    //}
	return false;
}


#define NUM_CCELERATION_SEGS 10
#define CELL_AMT 5

void buildCamera(idCameraDef *cam) {
	int i;
	//int lastSwitch = 0;
	idList<float> waits;
	idList<int> targets;

	totalTime = baseTime;
	cameraPosition->setTime(totalTime * 1000);
	// we have a base time layout for the path and the target path
	// now we need to layer on any wait or speed changes
	for (i = 0; i < events.Num(); i++) {
		//idCameraEvent *ev = events[i];
		events[i]->setTriggered(false);
		switch (events[i]->getType()) {
			case idCameraEvent::EVENT_TARGET : {
				targets.Append(i);
				break;
			}
			case idCameraEvent::EVENT_WAIT : {
				waits.Append(atof(events[i]->getParam()));
				cameraPosition->addVelocity(events[i]->getTime(), atof(events[i]->getParam()) * 1000, 0);
				break;
			}
			case idCameraEvent::EVENT_TARGETWAIT : {
				//targetWaits.Append(i);
				break;
			}
			case idCameraEvent::EVENT_SPEED : {
/*
				// take the average delay between up to the next five segments
				float adjust = atof(events[i]->getParam());
				int index = events[i]->getSegment();
				total = 0;
				count = 0;

				// get total amount of time over the remainder of the segment
				for (j = index; j < cameraSpline.numSegments() - 1; j++) {
					total += cameraSpline.getSegmentTime(j + 1) - cameraSpline.getSegmentTime(j);
					count++;
				}

				// multiply that by the adjustment
				double newTotal = total * adjust;
				// what is the difference.. 
				newTotal -= total;
				totalTime += newTotal / 1000;

				// per segment difference
				newTotal /= count;
				int additive = newTotal;

				// now propogate that difference out to each segment
				for (j = index; j < cameraSpline.numSegments(); j++) {
					cameraSpline.addSegmentTime(j, additive);
					additive += newTotal;
				}
				break;
*/
			}
    default: break; // FIXME: what about other idCameraEvent?
		}
	}


	for (i = 0; i < waits.Num(); i++) {
		totalTime += waits[i];
	}

	// on a new target switch, we need to take time to this point ( since last target switch ) 
	// and allocate it across the active target, then reset time to this point
	long timeSoFar = 0;
	long total = (int)(totalTime * 1000);
	for (i = 0; i < targets.Num(); i++) {
		long t;
		if (i < targets.Num() - 1) {
			t = events[targets[i+1]]->getTime();
		} else {
			t = total - timeSoFar;
		}
		// t is how much time to use for this target
		setActiveTargetByName(events[targets[i]]->getParam());
		getActiveTarget()->setTime(t);
		timeSoFar += t;
	}

	
}

void idCameraDef::startCamera(long t) {
	buildCamera();
	cameraPosition->start(t);
	//for (int i = 0; i < targetPositions.Num(); i++) {
	//	targetPositions[i]->
	//}
	startTime = t;
	cameraRunning = true;
}


void idCameraDef::parse(const char *(*text)  ) {

	const char	*token;
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		if (Q_stricmp(token, "time") == 0) {
			baseTime = Com_ParseFloat(text);
		}

		if (Q_stricmp(token, "camera_fixed") == 0) {
			cameraPosition = new idFixedPosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "camera_interpolated") == 0) {
			cameraPosition = new idInterpolatedPosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "camera_spline") == 0) {
			cameraPosition = new idSplinePosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "target_fixed") == 0) {
			idFixedPosition *pos = new idFixedPosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}
		
		if (Q_stricmp(token, "target_interpolated") == 0) {
			idInterpolatedPosition *pos = new idInterpolatedPosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}

		if (Q_stricmp(token, "target_spline") == 0) {
			idSplinePosition *pos = new idSplinePosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}

		if (Q_stricmp(token, "fov") == 0) {
			fov.parse(text);
		}

		if (Q_stricmp(token, "event") == 0) {
			idCameraEvent *event = new idCameraEvent();
			event->parse(text);
			addEvent(event);
		}


	} while (1);

	Com_UngetToken();
	Com_MatchToken( text, "}" );

}

qboolean idCameraDef::load(const char *filename) {
	char *buf;
	const char *buf_p;
	//int length = 
  FS_ReadFile( filename, (void **)&buf );
	if ( !buf ) {
		return qfalse;
	}

	clear();
	Com_BeginParseSession( filename );
	buf_p = buf;
	parse(&buf_p);
	Com_EndParseSession();
	FS_FreeFile( buf );

	return qtrue;
}

void idCameraDef::save(const char *filename) {
	fileHandle_t file = FS_FOpenFileWrite(filename);
	if (file) {
		int i;
		idStr s = "cameraPathDef { \n"; 
		FS_Write(s.c_str(), s.length(), file);
		s = va("\ttime %f\n", baseTime);
		FS_Write(s.c_str(), s.length(), file);

		cameraPosition->write(file, va("camera_%s",cameraPosition->typeStr()));

		for (i = 0; i < numTargets(); i++) {
			targetPositions[i]->write(file, va("target_%s", targetPositions[i]->typeStr()));
		}

		for (i = 0; i < events.Num(); i++) {
			events[i]->write(file, "event");
		}

		fov.write(file, "fov");

		s = "}\n";
		FS_Write(s.c_str(), s.length(), file);
	}
	FS_FCloseFile(file);
}

int idCameraDef::sortEvents(const void *p1, const void *p2) {
	idCameraEvent *ev1 = (idCameraEvent*)(p1);
	idCameraEvent *ev2 = (idCameraEvent*)(p2);

	if (ev1->getTime() > ev2->getTime()) {
		return -1;
	}
	if (ev1->getTime() < ev2->getTime()) {
		return 1;
	}
	return 0; 
}

void idCameraDef::addEvent(idCameraEvent *event) {
	events.Append(event);
	//events.Sort(&sortEvents);

}
void idCameraDef::addEvent(idCameraEvent::eventType t, const char *param, long time) {
	addEvent(new idCameraEvent(t, param, time));
	buildCamera();
}


const char *idCameraEvent::eventStr[] = {
	"NA",
	"WAIT",
	"TARGETWAIT",
	"SPEED",
	"TARGET",
	"SNAPTARGET",
	"FOV",
	"SCRIPT",
	"TRIGGER",
	"STOP"
};

void idCameraEvent::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "type") == 0) {
				type = static_cast<idCameraEvent::eventType>(atoi(token));
			} else if (Q_stricmp(key.c_str(), "param") == 0) {
				paramStr = token;
			} else if (Q_stricmp(key.c_str(), "time") == 0) {
				time = atoi(token);
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idCameraEvent::write(fileHandle_t file, const char *name) {
	idStr s = va("\t%s {\n", name);
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\ttype %d\n", static_cast<int>(type));
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\tparam %s\n", paramStr.c_str());
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\ttime %d\n", time);
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


const char *idCameraPosition::positionStr[] = {
	"Fixed",
	"Interpolated",
	"Spline",
};



const vec3_t *idInterpolatedPosition::getPosition(long t) { 
	static vec3_t interpolatedPos;

	float velocity = getVelocity(t);
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds	
	timePassed /= 1000;

	float distToTravel = timePassed *= velocity;

	vec3_t temp = startPos;
	temp -= endPos;
	float distance = temp.Length();

	distSoFar += distToTravel;
	float percent = (float)(distSoFar) / distance;

	if (percent > 1.0) {
		percent = 1.0;
	} else if (percent < 0.0) {
		percent = 0.0;
	}

	// the following line does a straigt calc on percentage of time
	// float percent = (float)(startTime + time - t) / time;

	vec3_t v1 = startPos;
	vec3_t v2 = endPos;
	v1 *= (1.0 - percent);
	v2 *= percent;
	v1 += v2;
	interpolatedPos = v1;
	return &interpolatedPos;
}


void idCameraFOV::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "fov") == 0) {
				fov = atof(token);
			} else if (Q_stricmp(key.c_str(), "startFOV") == 0) {
				startFOV = atof(token);
			} else if (Q_stricmp(key.c_str(), "endFOV") == 0) {
				endFOV = atof(token);
			} else if (Q_stricmp(key.c_str(), "time") == 0) {
				time = atoi(token);
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

bool idCameraPosition::parseToken(const char *key, const char *(*text)) {
	const char *token = Com_Parse(text);
	if (Q_stricmp(key, "time") == 0) {
		time = atol(token);
		return true;
	} else if (Q_stricmp(key, "type") == 0) {
		type = static_cast<idCameraPosition::positionType>(atoi(token));
		return true;
	} else if (Q_stricmp(key, "velocity") == 0) {
		long t = atol(token);
		token = Com_Parse(text);
		long d = atol(token);
		token = Com_Parse(text);
		float s = atof(token);
		addVelocity(t, d, s);
		return true;
	} else if (Q_stricmp(key, "baseVelocity") == 0) {
		baseVelocity = atof(token);
		return true;
	} else if (Q_stricmp(key, "name") == 0) {
		name = token;
		return true;
	} else if (Q_stricmp(key, "time") == 0) {
		time = atoi(token);
		return true;
	}
	Com_UngetToken();
	return false;
}



void idFixedPosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "pos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, pos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idInterpolatedPosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "startPos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, startPos );
			} else if (Q_stricmp(key.c_str(), "endPos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, endPos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}


void idSplinePosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "target") == 0) {
				target.parse(text);
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}



void idCameraFOV::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	
	s = va("\t\tfov %f\n", fov);
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tstartFOV %f\n", startFOV);
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tendFOV %f\n", endFOV);
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\ttime %i\n", time);
	FS_Write(s.c_str(), s.length(), file);

	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


void idCameraPosition::write(fileHandle_t file, const char *p) {
	
	idStr s = va("\t\ttime %i\n", time);
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\ttype %i\n", static_cast<int>(type));
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tname %s\n", name.c_str());
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tbaseVelocity %f\n", baseVelocity);
	FS_Write(s.c_str(), s.length(), file);

	for (int i = 0; i < velocities.Num(); i++) {
		s = va("\t\tvelocity %i %i %f\n", velocities[i]->startTime, velocities[i]->time, velocities[i]->speed);
		FS_Write(s.c_str(), s.length(), file);
	}

}

void idFixedPosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	s = va("\t\tpos ( %f %f %f )\n", pos.x, pos.y, pos.z);
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idInterpolatedPosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	s = va("\t\tstartPos ( %f %f %f )\n", startPos.x, startPos.y, startPos.z);
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\tendPos ( %f %f %f )\n", endPos.x, endPos.y, endPos.z);
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idSplinePosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	target.write(file, "target");
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idCameraDef::addTarget(const char *name, idCameraPosition::positionType type) {
	//const char *text = (name == NULL) ? va("target0%d", numTargets()+1) : name; // TTimo: unused
	idCameraPosition *pos = newFromType(type);
	if (pos) {
		pos->setName(name);
		targetPositions.Append(pos);
		activeTarget = numTargets()-1;
		if (activeTarget == 0) {
			// first one
			addEvent(idCameraEvent::EVENT_TARGET, name, 0);
		}
	}
}


idCameraDef camera;

qboolean loadCamera(const char *name) {
  camera.clear();
  return static_cast<qboolean>(camera.load(name));
}

qboolean getCameraInfo(int time, float *origin, float*angles) {
	vec3_t dir, org;
	org[0] = origin[0];
	org[1] = origin[1];
	org[2] = origin[2];
	float fov = 90;
	if (camera.getCameraInfo(time, org, dir, &fov)) {
		origin[0] = org[0];
		origin[1] = org[1];
		origin[2] = org[2];
		angles[1] = atan2 (dir[1], dir[0])*180/3.14159;
		angles[0] = asin (dir[2])*180/3.14159;
		return qtrue;
	}
	return qfalse;
}

void startCamera(int time) {
	camera.startCamera(time);
}


