// Mostly from https://github.com/xq3e/engine/blob/master/code/client/x_ddamage.c
#include "client.h"

static const char X_NUMBERS_SHADER[]   = "xmod/gfx/2d/numbers/%d_64a";
static const char X_HIT_SHADER[]       = "xmod/gfx/2d/numbers/hit";
static const char X_MODEL_MISSLE_HIT[] = "models/weaphits/boom01.md3";
static const char X_MODEL_RAIL_HIT[]   = "models/weaphits/ring02.md3";
static const char X_MODEL_BULLET_HIT[] = "models/weaphits/bullet.md3";

typedef enum {
	DmgNone,
	DmgUnknown,
	DmgDirect,
	DmgMissle,
	DmgRail,
	DmgShotgun,
  DmgMachinegun,
} DamageType;

typedef struct {
	int start;
	int value;
	int dir;
	vec3_t origin;
	byte color[4];
	float params[4];
} DamageIcon;

typedef struct {
	DamageType type;
	qboolean printed;
	int clientNum;
	int total;
	int damage;
	int target;
	int iconNum;
	int duration;
	int dir;
	int lastRedir;
	int redirDuration;
  snapshot_t *snap;

  qhandle_t shaderNumbers[10];
	qhandle_t shaderOneHPHit;
	qhandle_t modelMissle;
	qhandle_t modelRail;
	qhandle_t modelBullet;

	DamageIcon icons[256];
} Damage;

Damage xmoddmg;

// ====================
//   CVars

static cvar_t* x_hck_dmg_draw = 0;

// ====================
//   Const vars

static char X_HELP_DMG_DRAW[] =	"Show a damage when you hit an enemy";

// ====================
//   Static routines

static void X_DMG_PushSplashDamageForDirectHit(vec3_t origin);
static int GetClientNumByEntityNum(int entityNum, snapshot_t *snapshot);
static void DrawDamageIcon(refdef_t* fd, int inx);
static void AddDamageNumberToScene(vec3_t origin, byte* color, int value, vec3_t axis, float radius);
static void AddDamageHitToScene(vec3_t origin, float radius);
static void ChooseDamageColor(int damage, byte* rgba);

// ====================
//   Implementation

void X_DMG_Init( void )
{
  char shader[32];

	xmoddmg.type = DmgNone;
	xmoddmg.printed = qfalse;
	xmoddmg.duration = 600; // ms
	xmoddmg.redirDuration = 200;
  
  xmoddmg.modelMissle = re.RegisterModel(X_MODEL_MISSLE_HIT);
  xmoddmg.modelRail = re.RegisterModel(X_MODEL_RAIL_HIT);
  xmoddmg.modelBullet = re.RegisterModel(X_MODEL_BULLET_HIT);

  for (int i = 0; i < ARRAY_LEN(xmoddmg.shaderNumbers); i++)
  {
  	Com_sprintf(shader, sizeof(shader), X_NUMBERS_SHADER, i);
  	xmoddmg.shaderNumbers[i] = re.RegisterShader(shader);
  }

  xmoddmg.shaderOneHPHit = re.RegisterShader(X_HIT_SHADER);

  x_hck_dmg_draw = Cvar_Get("x_hck_dmg_draw", "4", CVAR_ARCHIVE|CVAR_USERINFO|CVAR_CHEAT);
  Cvar_CheckRange(x_hck_dmg_draw, "0", "4", CV_INTEGER);
  Cvar_SetDescription(x_hck_dmg_draw, X_HELP_DMG_DRAW);
}

void X_DMG_ParseSnapshotDamage( snapshot_t *snapshot )
{
	if (!x_hck_dmg_draw->integer)
		return;

  xmoddmg.snap = snapshot;

	int total = snapshot->ps.persistant[PERS_HITS];

	xmoddmg.type = DmgNone;
	xmoddmg.printed = qfalse;
	xmoddmg.damage = 0;

	if (xmoddmg.clientNum != snapshot->ps.clientNum)
	{
		xmoddmg.clientNum = snapshot->ps.clientNum;
		xmoddmg.total = total;
		return;
	}

	if (!total || xmoddmg.total > total)
		xmoddmg.total = total;

	if (xmoddmg.total == total)
		return;

	xmoddmg.damage = (snapshot->ps.persistant[PERS_ATTACKEE_ARMOR] & 0xFF)
    + (snapshot->ps.persistant[PERS_ATTACKEE_ARMOR] >> 8);
	xmoddmg.type = DmgUnknown;

	// Try to find a direct hit target
	for (int i = 0; i < snapshot->numEntities; i++)
	{
		int event = 0;
		if (snapshot->entities[i].eType >= ET_EVENTS)
		{
			event = snapshot->entities[i].eType - ET_EVENTS;
		}
		else
		{
			if (!snapshot->entities[i].event)
				continue;

			event = snapshot->entities[i].event & ~EV_EVENT_BITS;
		}

		if (event == EV_MISSILE_HIT || event == EV_BULLET_HIT_FLESH)
		{
			xmoddmg.type = DmgDirect;
			xmoddmg.target = GetClientNumByEntityNum(snapshot->entities[i].otherEntityNum, snapshot);
			break;
		}

		if (event == EV_MISSILE_MISS && xmoddmg.type != DmgMissle)
		{
			xmoddmg.type = DmgMissle;
		}

		if (event == EV_RAILTRAIL && xmoddmg.clientNum == snapshot->entities[i].clientNum)
		{
			xmoddmg.type = DmgRail;
			break;
		}

		if (event == EV_SHOTGUN && xmoddmg.clientNum == snapshot->entities[i].otherEntityNum)
		{
			xmoddmg.type = DmgShotgun;
			break;
		}

		if (event == EV_BULLET && xmoddmg.clientNum == snapshot->entities[i].otherEntityNum)
		{
			xmoddmg.type = DmgMachinegun;
			break;
		}
	}

	xmoddmg.total = total;
}

void X_DMG_PushDamageForEntity(refEntity_t* ref)
{
	if (ref->reType != RT_MODEL)
		return;

	if (xmoddmg.type == DmgMissle 
	  && ref->hModel && xmoddmg.modelMissle
	  && ref->hModel == xmoddmg.modelMissle)
		X_DMG_PushSplashDamageForDirectHit(ref->origin);
	else if (xmoddmg.type == DmgRail 
	  && ref->hModel && xmoddmg.modelRail
	  && ref->hModel == xmoddmg.modelRail)
		X_DMG_PushSplashDamageForDirectHit(ref->origin);
	else if (xmoddmg.type == DmgShotgun
	  && ref->hModel && xmoddmg.modelBullet
	  && ref->hModel == xmoddmg.modelBullet)
		X_DMG_PushSplashDamageForDirectHit(ref->origin);
  else if (xmoddmg.type == DmgMachinegun
	  && ref->hModel && xmoddmg.modelBullet
	  && ref->hModel == xmoddmg.modelBullet)
		X_DMG_PushSplashDamageForDirectHit(ref->origin);
}

void X_DMG_PushDamageForDirectHit(int entityNum, vec3_t origin)
{
	if (xmoddmg.type != DmgDirect) // || xmoddmg.printed)
		return;

  int clientNum = GetClientNumByEntityNum(entityNum, xmoddmg.snap);
	if (xmoddmg.target != clientNum)
		return;

	X_DMG_PushSplashDamageForDirectHit(origin);
}

static void X_DMG_PushSplashDamageForDirectHit(vec3_t origin)
{
	if (xmoddmg.type == DmgNone)
		return;

	//if (xmoddmg.printed)
	//	return;

	if (xmoddmg.lastRedir + xmoddmg.redirDuration < cls.realtime)
		xmoddmg.dir = (xmoddmg.dir == 1 ? -1 : 1);
	
	xmoddmg.lastRedir = cls.realtime;

	DamageIcon* icon = xmoddmg.icons + (xmoddmg.iconNum++ % ARRAY_LEN(xmoddmg.icons));
	icon->value = xmoddmg.damage;
	icon->start = cls.realtime;
	icon->dir = xmoddmg.dir;
	icon->params[0] = 10 + rand() % 10;
	icon->params[1] = ((rand() % 2) == 1 ? 1.0f : -1.0f);
	icon->params[2] = 0.1 * (1 + (rand() % 12));
	VectorCopy(origin, icon->origin);

	ChooseDamageColor(xmoddmg.damage, icon->color);

	xmoddmg.printed = qtrue;
}

static void PushUnclassifiedDamage(const refdef_t* fd)
{
	//if (xmoddmg.printed)
	//	return;

	vec3_t start, end;
	trace_t trace;

	VectorCopy(fd->vieworg, start);
	VectorMA(start, 131072, fd->viewaxis[0], end);

	CM_BoxTrace(&trace, start, end, vec3_origin, vec3_origin, 0,  CONTENTS_SOLID | CONTENTS_BODY, qfalse);

	X_DMG_PushSplashDamageForDirectHit(trace.endpos);
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

static int GetClientNumByEntityNum(int entityNum, snapshot_t *snapshot)
{
	for (int i = 0; i < snapshot->numEntities; i++)
	{
		if (snapshot->entities[i].eType == ET_PLAYER && snapshot->entities[i].number == entityNum)
			return snapshot->entities[i].clientNum;
	}

	return -1;
}

void X_DMG_DrawDamage(const refdef_t* fd)
{
	PushUnclassifiedDamage(fd);

	int peak = (xmoddmg.iconNum % ARRAY_LEN(xmoddmg.icons));
	for (int i = peak; i < ARRAY_LEN(xmoddmg.icons); i++)
		DrawDamageIcon((refdef_t*)fd, i);

	for (int i = 0; i < peak; i++)
		DrawDamageIcon((refdef_t*)fd, i);
}

static void SetDamageIconPosition(DamageIcon* icon, vec3_t origin, vec3_t viewaxis, int delataMs)
{
	float step = 2.0f / xmoddmg.duration;
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
		step = (2.0f - icon->params[2]) / xmoddmg.duration;
		x = (delataMs * step);
		y = -(x * x) + (2 * x);
		x /= (step * 15) * icon->params[1];
		y /= (step * icon->params[0]);
		break;
	case 4:
		step = (2.0f - icon->params[2]) / xmoddmg.duration;
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

	int end = xmoddmg.duration - 0xFF;
	if (delataMs > end)
		icon->color[3] = 0xFF - (delataMs - end);

	if (icon->value >= 100)
		icon->value = icon->value;
}

static void DrawDamageIcon(refdef_t* fd, int inx)
{
	DamageIcon* icon = xmoddmg.icons + inx;

	if (!icon->value)
		return;

	if (icon->start + xmoddmg.duration <= cls.realtime)
	{
		icon->value = 0;
		return;
	}

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
		ent.customShader = xmoddmg.shaderNumbers[num];
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
	ent.customShader = xmoddmg.shaderOneHPHit;
	ent.radius = radius;
	ent.renderfx = RF_DEPTHHACK;
	ent.shader.rgba[0] = 255;
	ent.shader.rgba[1] = 255;
	ent.shader.rgba[2] = 255;
	ent.shader.rgba[3] = 255;

	VectorCopy(origin, ent.origin);

	re.AddRefEntityToScene(&ent, qfalse);
}
