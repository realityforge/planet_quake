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
#ifndef __SPLINES_H
#define __SPLINES_H


#include "q_shared.h"
#include "qcommon.h"

typedef int fileHandle_t;

//extern void glBox(vec3_t &color, vec3_t &point, float size);
//extern void glLabeledPoint(vec3_t &color, vec3_t &point, float size, const char *label);

static vec4_t blue = {0, 0, 1, 1};
static vec4_t red = {1, 0, 0, 1};

//class idPointListInterface {
//public:

//virtual 
void addPointXYZ(const float x, const float y, const float z) {}
//virtual 
void addPoint(const vec3_t v) {}
//virtual 
void removePoint(int index) {}
//virtual 
vec3_t *getPoint(int index) { return NULL; }

//protected:
//static int *selectedPoints;

void deselectAll( int *numPoints, int **selectedPoints ) {
	Z_Free(*selectedPoints);
	*numPoints = 0;
}

int isPointSelected(int index, int numPoints, int *selectedPoints ) {
	for (int i = 0; i < numPoints; i++) {
		if (selectedPoints[i] == index) {
			return i;
		}
	}
	return -1;
}

int selectPoint(int index, qboolean single, int *numPoints, int **selectedPoints ) {
	if (index >= 0 && index < *numPoints) {
		if (single) {
			deselectAll(numPoints, selectedPoints);
		} else {
			int i;
			if ((i = isPointSelected(index, *numPoints, *selectedPoints)) >= 0) {
				memcpy(&(*selectedPoints)[i - 1], &(*selectedPoints)[i], sizeof(int));
				(*numPoints)--;
			}
		}
		(*selectedPoints)[*numPoints] = index;
		(*numPoints)++;
		return *numPoints;
	}
	return -1;
}

int	selectPointByRay(const vec3_t origin, const vec3_t direction, qboolean single, 
	int *numPoints, int **selectedPoints )
{
	int		i, besti;
	float	d, bestd;
	vec3_t	temp, temp2, *temp3;

	// find the point closest to the ray
	besti = -1;
	bestd = 8;

	for (i=0; i < *numPoints; i++) {
		temp3 = getPoint(i);
		VectorCopy(*temp3, temp);
		VectorCopy(temp, temp2);
		VectorSubtract(temp, origin, temp);
		d = DotProduct(temp, direction);
		VectorMA (origin, d, direction, temp);
		VectorSubtract(temp2, temp, temp2);
		d = VectorNormalize(temp2);
		if (d <= bestd) {
			bestd = d;
			besti = i;
		}
	}

	if (besti >= 0) {
		selectPoint(besti, single, numPoints, selectedPoints);
	}

	return besti;
}

int	selectPointByRayXYZ(float ox, float oy, float oz, float dx, float dy, float dz, qboolean single, 
	int *numPoints, int **selectedPoints ) 
{
	vec3_t origin = {ox, oy, oz};
	vec3_t dir = {dx, dy, dz};
	return selectPointByRay(origin, dir, single, numPoints, selectedPoints);
}

void selectAll(int *numPoints, int **selectedPoints ) {
	int pointsTotal = *numPoints;
	deselectAll(numPoints, selectedPoints);
	for (int i = 0; i < pointsTotal; i++) {
		selectPoint(i, qfalse, numPoints, selectedPoints);
	}
}

void updateSelection(const vec3_t move, int numPoints, int *selectedPoints ) {
	for (int i = 0; i < numPoints; i++) {
		vec3_t *vec = getPoint(selectedPoints[i]);
		VectorAdd(*vec, move, *vec);
	}
}

void updateSelectionXYZ(float x, float y, float z, int numPoints, int *selectedPoints) {
	vec3_t move = {x, y, z};
	updateSelection(move, numPoints, selectedPoints);
}

/*
void drawSelection() {
	int count = selectedPoints.Num();
	for (int i = 0; i < count; i++) {
		glBox(red, *getPoint(selectedPoints[i]), 4);
	}
}
*/




//class idSplineList {

//protected:
typedef struct {
	char name[MAX_QPATH];
	vec3_t *controlPoints;
	vec3_t *splinePoints;
	double *splineTime;
	vec3_t *selected;
	vec3_t pathColor, segmentColor, controlColor, activeColor;
	float granularity;
	qboolean editMode;
	qboolean dirty;
	int activeSegment;
	long baseTime;
	long time;
} idSplineList;


//public:
idSplineList *initSplineList(const char *p) {
	idSplineList *result = Z_Malloc(sizeof(idSplineList));
	memcpy(result->name, p, sizeof(result->name));
	return result;
}

void clearControl() {
	for (int i = 0; i < controlPoints.Num(); i++) {
		delete controlPoints[i];
	}
	controlPoints.Clear();
}

void clearSpline() {
	for (int i = 0; i < splinePoints.Num(); i++) {
		delete splinePoints[i];
	}
	splinePoints.Clear();
}

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

void clear() {
	clearControl();
	clearSpline();
	splineTime.Clear();
	selected = NULL;
	dirty = true;
	activeSegment = 0;
	granularity = 0.025;
	pathColor.set(1.0, 0.5, 0.0);
	controlColor.set(0.7, 0.0, 1.0);
	segmentColor.set(0.0, 0.0, 1.0);
	activeColor.set(1.0, 0.0, 0.0);
}

void initPosition(long startTime, long totalTime);
const vec3_t *getPosition(long time);


//	void draw(qboolean editMode);
void addToRenderer();

void setSelectedPoint(vec3_t *p);
vec3_t *getSelectedPoint() {
	return selected;
}

void addPoint(const vec3_t &v) {
	controlPoints.Append(new vec3_t(v));
	dirty = qtrue;
}

void addPoint(float x, float y, float z) {
	controlPoints.Append(new vec3_t(x, y, z));
	dirty = qtrue;
}

void updateSelection(const vec3_t &move);

void startEdit() {
	editMode = qtrue;
}
	
void stopEdit() {
	editMode = qfalse;
}

void buildSpline();

void setGranularity(float f) {
	granularity = f;
}

float getGranularity() {
	return granularity;
}

int numPoints() {
	return controlPoints.Num();
}

vec3_t *getPoint(int index) {
	assert(index >= 0 && index < controlPoints.Num());
	return controlPoints[index];
}

vec3_t *getSegmentPoint(int index) {
	assert(index >= 0 && index < splinePoints.Num());
	return splinePoints[index];
}


void setSegmentTime(int index, int time) {
	assert(index >= 0 && index < splinePoints.Num());
	splineTime[index] = time;
}

double getSegmentTime(int index) {
	assert(index >= 0 && index < splinePoints.Num());
	return splineTime[index];
}
void addSegmentTime(int index, int time) {
	assert(index >= 0 && index < splinePoints.Num());
	splineTime[index] += time;
}

float totalDistance();

static vec3_t zero;

int getActiveSegment() {
	return activeSegment;
}

void setActiveSegment(int i) {
	//assert(i >= 0 && (splinePoints.Num() > 0 && i < splinePoints.Num()));
	activeSegment = i;
}

int numSegments() {
	return splinePoints.Num();
}

void setColors(vec3_t &path, vec3_t &segment, vec3_t &control, vec3_t &active) {
	pathColor = path;
	segmentColor = segment;
	controlColor = control;
	activeColor = active;
}

const char *getName() {
	return name.c_str();
}

void setName(const char *p) {
	name = p;
}

qboolean validTime() {
	if (dirty) {
		buildSpline();
	}
	// gcc doesn't allow static casting away from bools
	// why?  I've no idea...
	return (bool)(splineTime.Num() > 0 && splineTime.Num() == splinePoints.Num());
}

void setTime(long t) {
	time = t;
}

void setBaseTime(long t) {
	baseTime = t;
}

float calcSpline(int step, float tension);

// time in milliseconds 
// velocity where 1.0 equal rough walking speed
typedef struct {
idVelocity(long start, long duration, float s) {
	startTime = start;
	time = duration;
	speed = s;
}
long	startTime;
long	time;
float	speed;
} idVelocity;



// can either be a look at or origin position for a camera
// 
//class idCameraPosition : public idPointListInterface {
//public:

void clear() {
	editMode = false;
	for (int i = 0; i < velocities.Num(); i++) {
		delete velocities[i];
		velocities[i] = NULL;
	}
	velocities.Clear();
}

idCameraPosition(const char *p) {
	name = p;
}

idCameraPosition() {
	time = 0;
	name = "position";
}

idCameraPosition(long t) {
	time = t;
}

//virtual
// ~idCameraPosition() {
//	clear();
//}


// this can be done with RTTI syntax but i like the derived classes setting a type
// makes serialization a bit easier to see
//
typedef enum {
	FIXED = 0x00,
	INTERPOLATED,
	SPLINE,
	POSITION_COUNT
} positionType;


//virtual 
void start(long t) {
	startTime = t;
}

long getTime() {
	return time;
}

//virtual 
void setTime(long t) {
	time = t;
}

float getVelocity(long t) {
	long check = t - startTime;
	for (int i = 0; i < velocities.Num(); i++) {
		if (check >= velocities[i]->startTime && check <= velocities[i]->startTime + velocities[i]->time) {
			return velocities[i]->speed;
		}
	}
	return baseVelocity;
}

void addVelocity(long start, long duration, float speed) {
	velocities.Append(new idVelocity(start, duration, speed));
}

//virtual 
const vec3_t *getPosition(long t) { 
//	assert(true);
	return NULL;
}

//	virtual void draw(qboolean editMode) {};

//virtual 
void parse(const char *(*text)) {};
//virtual 
void write(fileHandle_t file, const char *name);
//virtual 
qboolean parseToken(const char *key, const char *(*text));

const char *getName() {
	return name.c_str();
}

void setName(const char *p) {
	name = p;
}

//virtual 
void startEdit() {
	editMode = true;
}

//virtual 
void stopEdit() {
	editMode = false;
}

//	virtual void draw() {};

const char *typeStr() {
	return positionStr[static_cast<int>(type)];
}

void calcVelocity(float distance) {
	float secs = (float)time / 1000;
	baseVelocity = distance / secs;
}

//protected:
typedef struct {
	const char* positionStr[POSITION_COUNT];
	long		startTime;
	long		time;
	positionType type;
	char		name[MAX_QPATH];
	qboolean	editMode;
	idVelocity *velocities;
	float		baseVelocity;
} idCameraPosition;
//};




//class idFixedPosition : public idCameraPosition {
//public:

void init() {
	pos.Zero();
	type = idCameraPosition::FIXED;
}

idFixedPosition() : idCameraPosition() {
	init();
}

idFixedPosition(vec3_t p) : idCameraPosition() {
	init();
	pos = p;
}

//virtual 
void addPoint(const vec3_t &v) {
	pos = v;
}

//virtual 
void addPoint(const float x, const float y, const float z) {
	pos.set(x, y, z);
}


~idFixedPosition() {
}

//virtual 
const vec3_t *getPosition(long t) { 
	return &pos;
}

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

//virtual 
int numPoints() {
	return 1;
}

//virtual 
vec3_t *getPoint(int index) {
	if (index != 0) {
		assert(true);
	};
	return &pos;
}
/*
virtual void draw(qboolean editMode) {
	glLabeledPoint(blue, pos, (editMode) ? 5 : 3, "Fixed point");
}
*/
//protected:
vec3_t pos;
//};




//class idInterpolatedPosition : public idCameraPosition {
//public:

void init() {
	type = idCameraPosition::INTERPOLATED;
	first = true;
	startPos.Zero();
	endPos.Zero();
}

idInterpolatedPosition() : idCameraPosition() {
	init();
}

idInterpolatedPosition(vec3_t start, vec3_t end, long time) : idCameraPosition(time) {
	init();
	startPos = start;
	endPos = end;
}

~idInterpolatedPosition() {
}

//virtual 
const vec3_t *getPosition(long t);

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

//virtual 
int numPoints() {
	return 2;
}

//virtual 
vec3_t *getPoint(int index) {
	assert(index >= 0 && index < 2);
	if (index == 0) {
		return &startPos;
	}
	return &endPos;
}

//virtual 
void addPoint(const float x, const float y, const float z) {
	if (first) {
		startPos.set(x, y, z);
		first = false;
	} else {
		endPos.set(x, y, z);
		first = true;
	}
}

//virtual 
void addPoint(const vec3_t &v) {
	if (first) {
		startPos = v;
		first = false;
	} else {
		endPos = v;
		first = true;
	}
}
/*
virtual void draw(qboolean editMode) {
	glLabeledPoint(blue, startPos, (editMode) ? 5 : 3, "Start interpolated");
	glLabeledPoint(blue, endPos, (editMode) ? 5 : 3, "End interpolated");
	qglBegin(GL_LINES);
	qglVertex3fv(startPos);
	qglVertex3fv(endPos);
	qglEnd();
}
*/
//virtual 
void start(long t) {
	idCameraPosition::start(t);
	lastTime = startTime;
	distSoFar = 0.0;
	vec3_t temp = startPos;
	temp -= endPos;
	calcVelocity(temp.Length());
}

//protected:
qboolean first;
vec3_t startPos;
vec3_t endPos;
long lastTime;
float distSoFar;
//};

//class idSplinePosition : public idCameraPosition {
//public:

void init() {
	type = idCameraPosition::SPLINE;
}

idSplinePosition() : idCameraPosition() {
	init();
}

idSplinePosition(long time) : idCameraPosition(time) {
	init();
}

~idSplinePosition() {
}

//virtual 
void start(long t) {
	idCameraPosition::start(t);
	target.initPosition(t, time);
	calcVelocity(target.totalDistance());
}

//virtual 
const vec3_t *getPosition(long t) { 
	return target.getPosition(t);
}

//virtual const vec3_t *getPosition(long t) const { 

void addControlPoint(vec3_t &v) {
	target.addPoint(v);
}

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

//virtual 
int numPoints() {
	return target.numPoints();
}

//virtual 
vec3_t *getPoint(int index) {
	return target.getPoint(index);
}

//virtual 
void addPoint(const vec3_t &v) {
	target.addPoint(v);
}

//virtual 
void addPoint(const float x, const float y, const float z) {
	target.addPoint(x, y, z);
}

/*	virtual void draw(qboolean editMode) {
	target.draw(editMode);
}
*/
//virtual 
void updateSelection(const vec3_t &move) {
	idCameraPosition::updateSelection(move);
	target.buildSpline();
}

//protected:
idSplineList target;
//};




//class idCameraFOV {
//public:

idCameraFOV() {
	time = 0;
	fov = 90;
}

idCameraFOV(int v) {
	time = 0;
	fov = v;
}

idCameraFOV(int s, int e, long t) {
	startFOV = s;
	endFOV = e;
	time = t;
}


//~idCameraFOV(){}

void setFOV(float f) {
	fov = f;
}

float getFOV(long t) {
	if (time) {
		assert(startTime);
		float percent = t / startTime;
		float temp = startFOV - endFOV;
		temp *= percent;
		fov = startFOV + temp;
	}
	return fov;
}

void start(long t) {
	startTime = t;
}

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

//protected:
typedef struct {
	float fov;
	float startFOV;
	float endFOV;
	int startTime;
	int time;
} idCameraFOV;
//};




//class idCameraEvent {
//public:
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

static const char* eventStr[EVENT_COUNT];

//protected:
typedef struct {
	eventType type;
	char paramStr[MAX_QPATH];
	long time;
	qboolean triggered;
} idCameraEvent;
//};

idCameraEvent *initCameraEvent(eventType t, const char *param, long n) {
	//paramStr = "";
	//type = EVENT_NA;
	//time = 0;
	idCameraEvent *result = (void *)Z_Malloc(sizeof(idCameraEvent));
	result->type = t;
	result->paramStr = param;
	result->time = n;
	return result;
}

//~idCameraEvent() {};

eventType getType() {
	return type;
}

const char *typeStr() {
	return eventStr[static_cast<int>(type)];
}

const char *getParam() {
	return paramStr.c_str();
}

long getTime() {
	return time;
}

void setTime(long n) {
	time = n;
}

void parse(const char *(*text));
void write(fileHandle_t file, const char *name);

void setTriggered(qboolean b) {
	triggered = b;
}

qboolean getTriggered() {
	return triggered;
}



//class idCameraDef {
//public:

void cameraClear() {
	currentCameraPosition = 0;
	cameraRunning = qfalse;
	lastDirection.Zero();
	baseTime = 30;
	activeTarget = 0;
	name = "camera01";
	fov.setFOV(90);
	int i;
	for (i = 0; i < targetPositions.Num(); i++) {
		delete targetPositions[i];
	}
	for (i = 0; i < events.Num(); i++) {
		delete events[i];
	}
	delete cameraPosition;
	cameraPosition = NULL;
	events.Clear();
	targetPositions.Clear();
}


idCameraPosition *startNewCamera(idCameraPosition::positionType type) {
	clear();
	if (type == idCameraPosition::SPLINE) {
		cameraPosition = new idSplinePosition();
	} else if (type == idCameraPosition::INTERPOLATED) {
		cameraPosition = new idInterpolatedPosition();
	} else {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}


void addEvent(idCameraEvent::eventType t, const char *param, long time);
void addEvent(idCameraEvent *event);
static int sortEvents(const void *p1, const void *p2);

int numEvents() {
	return events.Num();
}

idCameraEvent *getEvent(int index) {
	assert(index >= 0 && index < events.Num());
	return events[index];
}

void parse(const char *(*text));
qboolean load(const char *filename);
void save(const char *filename);

void buildCamera();

//idSplineList *getcameraPosition() {
//	return &cameraPosition;
//}

static idCameraPosition *newFromType(idCameraPosition::positionType t) {
	switch (t) {
		case idCameraPosition::FIXED : return new idFixedPosition();
		case idCameraPosition::INTERPOLATED : return new idInterpolatedPosition();
	case idCameraPosition::SPLINE : return new idSplinePosition();
	default:
			break;
	};
	return NULL;
}

void addTarget(const char *name, idCameraPosition::positionType type);

idCameraPosition *getActiveTarget() {
	if (targetPositions.Num() == 0) {
		addTarget(NULL, idCameraPosition::FIXED);
	}
	return targetPositions[activeTarget];
}

idCameraPosition *getActiveTarget(int index) {
	if (targetPositions.Num() == 0) {
		addTarget(NULL, idCameraPosition::FIXED);
		return targetPositions[0];
	}
	return targetPositions[index];
}

int numTargets() {
	return targetPositions.Num();
}


void setActiveTargetByName(const char *name) {
	for (int i = 0; i < targetPositions.Num(); i++) {
		if (Q_stricmp(name, targetPositions[i]->getName()) == 0) {
			setActiveTarget(i);
			return;
		}
	}
}

void setActiveTarget(int index) {
	assert(index >= 0 && index < targetPositions.Num());
	activeTarget = index;
}

void setRunning(qboolean b) {
	cameraRunning = b;
}

void setBaseTime(float f) {
	baseTime = f;
}

float getBaseTime() {
	return baseTime;
}

float getTotalTime() {
	return totalTime;
}

void startCamera(long t);
void stopCamera() {
	cameraRunning = true;
}
void getActiveSegmentInfo(int segment, vec3_t &origin, vec3_t &direction, float *fv);

qboolean getCameraInfo(long time, vec3_t &origin, vec3_t &direction, float *fv);
qboolean getCameraInfo(long time, float *origin, float *direction, float *fv) {
	vec3_t org, dir;
	org[0] = origin[0];
	org[1] = origin[1];
	org[2] = origin[2];
	dir[0] = direction[0];
	dir[1] = direction[1];
	dir[2] = direction[2];
	qboolean b = getCameraInfo(time, org, dir, fv);
	origin[0] = org[0];
	origin[1] = org[1];
	origin[2] = org[2];
	direction[0] = dir[0];
	direction[1] = dir[1];
	direction[2] = dir[2];
	return b;
}
/*
	void draw(qboolean editMode) {
                // gcc doesn't allow casting away from bools
                // why?  I've no idea...
		if (cameraPosition) {
			cameraPosition->draw((bool)((editMode || cameraRunning) && cameraEdit));
			int count = targetPositions.Num();
			for (int i = 0; i < count; i++) {
				targetPositions[i]->draw((bool)((editMode || cameraRunning) && i == activeTarget && !cameraEdit));
			}
		}
	}
*/
/*
	int numSegments() {
		if (cameraEdit) {
			return cameraPosition.numSegments();
		}
		return getTargetSpline()->numSegments();
	}

	int getActiveSegment() {
		if (cameraEdit) {
			return cameraPosition.getActiveSegment();
		}
		return getTargetSpline()->getActiveSegment();
	}

	void setActiveSegment(int i) {
		if (cameraEdit) {
			cameraPosition.setActiveSegment(i);
		} else {
			getTargetSpline()->setActiveSegment(i);
		}
	}
*/
int numPoints() {
	if (cameraEdit) {
		return cameraPosition->numPoints();
	}
	return getActiveTarget()->numPoints();
}

const vec3_t *getPoint(int index) {
	if (cameraEdit) {
		return cameraPosition->getPoint(index);
	}
	return getActiveTarget()->getPoint(index);
}

void stopEdit() {
	editMode = qfalse;
	if (cameraEdit) {
		cameraPosition->stopEdit();
	} else {
		getActiveTarget()->stopEdit();
	}
}

void startEdit(qboolean camera) {
	cameraEdit = camera;
	if (camera) {
		cameraPosition->startEdit();
		for (int i = 0; i < targetPositions.Num(); i++) {
			targetPositions[i]->stopEdit();
		}
	} else {
		getActiveTarget()->startEdit();
		cameraPosition->stopEdit();
	}
	editMode = qtrue;
}

qboolean waitEvent(int index);

const char *getName() {
	return name.c_str();
}

void setName(const char *p) {
	name = p;
}

idCameraPosition *getPositionObj() {
	if (cameraPosition == NULL) {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}

//protected:
typedef struct {
	char name[MAX_QPATH];
	int currentCameraPosition;
	vec3_t lastDirection;
	qboolean cameraRunning;
	idCameraPosition *cameraPosition;
	idCameraPosition *targetPositions;
	idCameraEvent *events;
	idCameraFOV fov;
	int activeTarget;
	float totalTime;
	float baseTime;
	long startTime;

	qboolean cameraEdit;
	qboolean editMode;
} idCameraDef;

//};

extern qboolean g_splineMode;
extern idCameraDef *g_splineList;


#endif
