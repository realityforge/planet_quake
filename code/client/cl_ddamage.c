// Mostly from https://github.com/xq3e/engine/blob/master/code/client/x_ddamage.c
#include "client.h"

static const char X_NUMBERS_SHADER[]   = "xmod/gfx/2d/numbers/%d_64a";
static const char X_HIT_SHADER[]       = "xmod/gfx/2d/numbers/hit";
//static const char X_MODEL_MISSLE_HIT[] = "models/weaphits/boom01.md3";
//static const char X_MODEL_RAIL_HIT[]   = "models/weaphits/ring02.md3";
//static const char X_MODEL_BULLET_HIT[] = "models/weaphits/bullet.md3";

typedef struct {
  int eNum;
	int start;
	int value;
	int dir;
	vec3_t origin;
	byte color[4];
	float params[4];
} dmgIcon_t;

qhandle_t shaderNumbers[10];
qhandle_t shaderOneHPHit;

int        dmgIconMax; // holder for max icons per client, set in Init
dmgIcon_t *dmgIcons;
qboolean   dmgIconDir = qfalse;


// ====================
//   CVars

static cvar_t* x_hck_dmg_draw = 0;
static cvar_t* x_hck_dmg_duration = 0;
static cvar_t* x_hck_dmg_combine = 0;

// ====================
//   Const vars

static char X_HELP_DMG_DRAW[] =	"Show a damage when you hit an enemy";

// ====================
//   Static routines

static void DrawDamageIcon(refdef_t* fd, dmgIcon_t *icon);
static void AddDamageNumberToScene(vec3_t origin, byte* color, int value, vec3_t axis, float radius);
static void AddDamageHitToScene(vec3_t origin, float radius);
static void ChooseDamageColor(int damage, byte* rgba);

// ====================
//   Implementation

void X_DMG_Init( void )
{
  char shader[32];

  for (int i = 0; i < ARRAY_LEN(shaderNumbers); i++)
  {
  	Com_sprintf(shader, sizeof(shader), X_NUMBERS_SHADER, i);
  	shaderNumbers[i] = re.RegisterShader(shader);
  }
  shaderOneHPHit = re.RegisterShader(X_HIT_SHADER);

  x_hck_dmg_draw = Cvar_Get("x_hck_dmg_draw", "4", CVAR_ARCHIVE|CVAR_USERINFO|CVAR_CHEAT);
  Cvar_CheckRange(x_hck_dmg_draw, "0", "4", CV_INTEGER);
  Cvar_SetDescription(x_hck_dmg_draw, X_HELP_DMG_DRAW);

  x_hck_dmg_duration = Cvar_Get("x_hck_dmg_duration", "600", CVAR_ARCHIVE|CVAR_USERINFO|CVAR_LATCH);
  Cvar_CheckRange(x_hck_dmg_duration, "100", "1000", CV_INTEGER);
  Cvar_SetDescription(x_hck_dmg_duration, X_HELP_DMG_DRAW);

  x_hck_dmg_combine = Cvar_Get("x_hck_dmg_combine", "200", CVAR_ARCHIVE|CVAR_USERINFO|CVAR_LATCH);
  Cvar_CheckRange(x_hck_dmg_combine, "100", va("%i", x_hck_dmg_duration->integer), CV_INTEGER);
  Cvar_SetDescription(x_hck_dmg_combine, X_HELP_DMG_DRAW);

  dmgIconMax = x_hck_dmg_duration->integer / x_hck_dmg_combine->integer + 1;
  dmgIcons = Z_Malloc(MAX_CLIENTS * dmgIconMax * sizeof(dmgIcon_t));
}

//void X_DMG_UpdateClientOrigin(refEntity_t *ent) {
//  if(cl.parseEntities[ent.])
//}

void X_DMG_ParseSnapshotDamage( void )
{
	if (!x_hck_dmg_draw->integer)
		return;

  clSnapshot_t	*newSnap = &cl.snapshots[(cl.snap.messageNum - 1) & PACKET_MASK];
  clSnapshot_t	*oldSnap = &cl.snapshots[(cl.snap.messageNum - 2) & PACKET_MASK];

	if (newSnap->ps.persistant[PERS_HITS] % 3 == 0)
		dmgIconDir = !dmgIconDir;

  for ( int i = 0 ; i < newSnap->numEntities ; i++ ) {
    if(oldSnap->ps.persistant[PERS_HITS] >= newSnap->ps.persistant[PERS_HITS]) {
      continue;
    }

    entityState_t ent = cl.parseEntities[ ( newSnap->parseEntitiesNum + i ) & (MAX_PARSE_ENTITIES-1) ];
    int event;
    if (ent.eType >= ET_EVENTS) {
			event = ent.eType - ET_EVENTS;
		} else {
			if (!ent.event)
				continue;
			event = ent.event & ~EV_EVENT_BITS;
		}

    if (event == EV_MISSILE_HIT || event == EV_BULLET_HIT_FLESH
      || event == EV_MISSILE_MISS || event == EV_RAILTRAIL 
      || event == EV_SHOTGUN || event == EV_BULLET) {
      int eNum = (newSnap->parseEntitiesNum + ent.otherEntityNum) & (MAX_PARSE_ENTITIES-1);
      int clientNum = cl.parseEntities[eNum].clientNum;
      if(clientNum > MAX_CLIENTS)
        continue;
      int damage = (newSnap->ps.persistant[PERS_ATTACKEE_ARMOR] & 0xFF)
        + (newSnap->ps.persistant[PERS_ATTACKEE_ARMOR] >> 8);
      int index = (cls.realtime / x_hck_dmg_combine->integer) % dmgIconMax;
      dmgIcon_t *icon = &dmgIcons[clientNum * dmgIconMax + index];
      icon->value += damage;
      icon->eNum = cl.parseEntities[eNum].number;
      if(!icon->start) {
        icon->start = cls.realtime;
        icon->dir = (dmgIconDir ? -1 : 1);
        icon->params[0] = 10 + rand() % 10;
        icon->params[1] = ((rand() % 2) == 1 ? 1.0f : -1.0f);
        icon->params[2] = 0.1 * (1 + (rand() % 12));
        icon->origin[0] = 31.875;
        //VectorCopy(cl.parseEntities[eNum].origin, icon->origin);
      }
      ChooseDamageColor(damage, icon->color);
    }
  }
}

static void ChooseDamageColor(int damage, byte* rgba)
{
	if (damage <= 25)
	{
		MAKERGBA(rgba, 80, 255, 10, 255);
	}
	else if (damage <= 50)
	{
		MAKERGBA(rgba, 250, 250, 10, 255);
	}
	else if (damage <= 75)
	{
		MAKERGBA(rgba, 250, 170, 10, 255);
	}
	else if (damage <= 100)
	{
		MAKERGBA(rgba, 250, 25, 10, 255);
	}
	else if (damage <= 150)
	{
		MAKERGBA(rgba, 250, 15, 150, 255);
	}
	else if (damage <= 200)
	{
		MAKERGBA(rgba, 200, 15, 254, 255);
	}
	else
	{
		MAKERGBA(rgba, 128, 128, 255, 255);
	}
}

void X_DMG_DrawDamage(const refdef_t* fd)
{
  if(fd->vieworg[0] == 0 && fd->vieworg[1] == 0 && fd->vieworg[2] == 0)
    return;

  vec3_t start, end;
	trace_t trace;

	VectorCopy(fd->vieworg, start);
	VectorMA(start, 131072, fd->viewaxis[0], end);

	CM_BoxTrace(&trace, start, end, vec3_origin, vec3_origin, 0,  CONTENTS_SOLID | CONTENTS_BODY, qfalse);
  if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {
    for (int j = 0; j < dmgIconMax; j++) {
      int inx = i*dmgIconMax+j;
      if(!dmgIcons[inx].start || !dmgIcons[inx].value)
        continue; // cleared, no value == no draw
      if (dmgIcons[inx].start + x_hck_dmg_duration->integer <= cls.realtime)
      {
        dmgIcons[inx].start = 0;
        dmgIcons[inx].value = 0;
        continue;
      }
      if(!dmgIcons[inx].value)
        continue; // cleared, no value == no draw

      //dmgIcons[inx].origin[0] = 320;
      //dmgIcons[inx].origin[1] = 240;
      //dmgIcons[inx].origin[2] = 20;
      VectorCopy(trace.endpos, dmgIcons[inx].origin);
	    DrawDamageIcon((refdef_t*)fd, &dmgIcons[inx]);
    }
  }
}

static void SetDamageIconPosition(dmgIcon_t* icon, vec3_t origin, vec3_t viewaxis, int delataMs)
{
	float step = 2.0f / x_hck_dmg_duration->integer;
	float x = (delataMs * step);
	float y = -(x * x) + (2 * x);

	switch (x_hck_dmg_draw->integer)
	{
	case 1:
		x /= (step * 15) * icon->dir;
		y /= (step * 8);
		break;
	case 2:
		x /= (step * 15) * icon->dir;
		y /= (step * icon->params[0]);
		break;
	case 3:
		step = (2.0f - icon->params[2]) / x_hck_dmg_duration->integer;
		x = (delataMs * step);
		y = -(x * x) + (2 * x);
		x /= (step * 15) * icon->params[1];
		y /= (step * icon->params[0]);
		break;
	case 4:
		step = (2.0f - icon->params[2]) / x_hck_dmg_duration->integer;
		x = (delataMs * step);
		y = -(x * x) + (2 * x);
		x /= (step * 40) * icon->params[1];
		y /= (step * icon->params[0]);
		break;
	default:
		x /= (step * 15) * icon->dir;
		y /= (step * 20);
		break;
	}

	VectorMA(origin, x, viewaxis, origin);
	origin[2] += 50 + y;

	int end = x_hck_dmg_duration->integer - 0xFF;
	if (delataMs > end)
		icon->color[3] = 0xFF - (delataMs - end);
}

static void DrawDamageIcon(refdef_t* fd, dmgIcon_t* icon)
{
	if (!icon->value)
		return;

	float radius = ((Distance(fd->vieworg, icon->origin) / 100.f) / 2) + 5;

	vec3_t origin;
	VectorCopy(icon->origin, origin);

	SetDamageIconPosition(icon, origin, fd->viewaxis[1], (cls.realtime - icon->start));

	if (icon->value == 1)
		AddDamageHitToScene(origin, radius);
	else
		AddDamageNumberToScene(origin, icon->color, icon->value, fd->viewaxis[1], radius);
}

static void AddDamageNumberToScene(vec3_t origin, byte* color, int value, vec3_t axis, float radius)
{
	for (int i = 0; i < 10; i++)
	{
		if (!value)
			break;

		int num = value % 10;
		value /= 10;

		refEntity_t	ent;
		memset(&ent, 0, sizeof(ent));
		ent.reType = RT_SPRITE;
		ent.customShader = shaderNumbers[num];
		ent.radius = radius;
		ent.renderfx = RF_DEPTHHACK;
		ent.shader.rgba[0] = color[0];
		ent.shader.rgba[1] = color[1];
		ent.shader.rgba[2] = color[2];
		ent.shader.rgba[3] = color[3];

		float mult = radius * 1.5;
		
		VectorCopy(origin, ent.origin);
		VectorMA(ent.origin, i * mult, axis, ent.origin);
		
		re.AddRefEntityToScene(&ent, qfalse);
	}
}

static void AddDamageHitToScene(vec3_t origin, float radius)
{
	refEntity_t	ent;
	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_SPRITE;
	ent.customShader = shaderOneHPHit;
	ent.radius = radius;
	ent.renderfx = RF_DEPTHHACK;
	ent.shader.rgba[0] = 255;
	ent.shader.rgba[1] = 255;
	ent.shader.rgba[2] = 255;
	ent.shader.rgba[3] = 255;

	VectorCopy(origin, ent.origin);

	re.AddRefEntityToScene(&ent, qfalse);
}
