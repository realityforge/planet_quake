#include "../game/q_shared.h"
#ifdef CGAME
#include "../cgame/cg_local.h"
#else
#include "bg_public.h"
#endif

gitem_t bg_itemlist2[] = {
  {
		NULL,
		NULL,
		{ NULL,
			NULL,
			NULL, NULL},
		/* icon */ NULL,
		/* pickup */ NULL,
		0,
		0,
		0,
		/* precache */ "",
		/* sounds */ ""
	}, // leave index 0 alone

	//
	// WEAPONS 
	//

	/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_gauntlet",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/gauntlet/gauntlet.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_gauntlet",
		/* pickup */ "Gauntlet",
		0,
		IT_WEAPON,
		WP_GAUNTLET,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_shotgun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/shotgun/shotgun.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_shotgun",
		/* pickup */ "Shotgun",
		10,
		IT_WEAPON,
		WP_SHOTGUN,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_machinegun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/machinegun/machinegun.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_machinegun",
		/* pickup */ "Machinegun",
		40,
		IT_WEAPON,
		WP_MACHINEGUN,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_grenadelauncher",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/grenadel/grenadel.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_grenade",
		/* pickup */ "Grenade Launcher",
		10,
		IT_WEAPON,
		WP_GRENADE_LAUNCHER,
		/* precache */ "",
		/* sounds */ "baseoa/sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav"
	},

	/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_rocketlauncher",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/rocketl/rocketl.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_rocket",
		/* pickup */ "Rocket Launcher",
		10,
		IT_WEAPON,
		WP_ROCKET_LAUNCHER,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_lightning",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/lightning/lightning.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_lightning",
		/* pickup */ "Lightning Gun",
		100,
		IT_WEAPON,
		WP_LIGHTNING,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_railgun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/railgun/railgun.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_railgun",
		/* pickup */ "Railgun",
		10,
		IT_WEAPON,
		WP_RAILGUN,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_plasmagun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_plasmagun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/plasma/plasma.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_plasma",
		/* pickup */ "Plasma Gun",
		50,
		IT_WEAPON,
		WP_PLASMAGUN,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_bfg",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/bfg/bfg.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_bfg",
		/* pickup */ "BFG10K",
		20,
		IT_WEAPON,
		WP_BFG,
		/* precache */ "",
		/* sounds */ ""
	},

#ifdef USE_GRAPPLE
	/*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_grapplinghook",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons2/grapple/grapple.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_grapple",
		/* pickup */ "Grappling Hook",
		0,
		IT_WEAPON,
		WP_GRAPPLING_HOOK,
		/* precache */ "",
		/* sounds */ ""
	},
#endif

  /*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
  {
		"weapon_nailgun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons/nailgun/nailgun.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_nailgun",
		/* pickup */ "Nailgun",
		10,
		IT_WEAPON,
		WP_NAILGUN,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_prox_launcher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_prox_launcher",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons/proxmine/proxmine.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_proxlauncher",
		/* pickup */ "Prox Launcher",
		5,
		IT_WEAPON,
		WP_PROX_LAUNCHER,
		/* precache */ "",
		/* sounds */ "baseoa/sound/weapons/proxmine/wstbtick.wav "
		"baseoa/sound/weapons/proxmine/wstbactv.wav "
		"baseoa/sound/weapons/proxmine/wstbimpl.wav "
		"baseoa/sound/weapons/proxmine/wstbimpm.wav "
		"baseoa/sound/weapons/proxmine/wstbimpd.wav "
		"baseoa/sound/weapons/proxmine/wstbactv.wav"
	},

	/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_chaingun",
		"baseoa/sound/misc/w_pkup.wav",
		{ "baseoa/models/weapons/vulcan/vulcan.md3",
			NULL, NULL, NULL},
		/* icon */ "baseoa/icons/iconw_chaingun",
		/* pickup */ "Chaingun",
		80,
		IT_WEAPON,
		WP_CHAINGUN,
		/* precache */ "",
		/* sounds */ "baseoa/sound/weapons/vulcan/wvulwind.wav"
	},

	// end of list marker
	{NULL}
};


int bg_numItems2 = sizeof (bg_itemlist2) / sizeof (bg_itemlist2[0]) - 1;

#ifdef CGAME

void CG_GrappleTrail( centity_t *ent, const struct weaponInfo_s *wi );
void CG_PlasmaTrail( centity_t *ent, const struct weaponInfo_s *wi );
void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_MachineGunEjectBrass( centity_t *cent );
void CG_ShotgunEjectBrass( centity_t *cent );
void CG_NailgunEjectBrass( centity_t *cent );


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon2( int weaponNum )
{
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
  char			strippedPath[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist2 + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == (weaponNum & 0xF) ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist2 );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist2 + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == (weaponNum & 0xF) ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->flashModel ) {
		weaponInfo->flashModel = trap_R_RegisterModel( "baseoa/models/weapons2/shotgun/shotgun_flash.md3" );	// default flash
	}

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash1.md3" );
	weaponInfo->flashModel_type1 = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash2.md3" );
	weaponInfo->flashModel_type2 = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash2a.md3" );
	weaponInfo->flashModel_type2a = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash3.md3" );
	weaponInfo->flashModel_type3 = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash4.md3" );
	weaponInfo->flashModel_type4 = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash5.md3" );
	weaponInfo->flashModel_type5 = trap_R_RegisterModel( path );


	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_flash5a.md3" );
	weaponInfo->flashModel_type5a = trap_R_RegisterModel( path );


	if (!weaponInfo->flashModel_type1) weaponInfo->flashModel_type1 = trap_R_RegisterModel( "baseoa/models/muzzle/flash1.md3" );
	if (!weaponInfo->flashModel_type2) weaponInfo->flashModel_type2 = trap_R_RegisterModel( "baseoa/models/muzzle/flash2.md3" ); 
	if (!weaponInfo->flashModel_type2a) weaponInfo->flashModel_type2a = trap_R_RegisterModel( "baseoa/models/muzzle/flash2a.md3" ); 
	if (!weaponInfo->flashModel_type3) weaponInfo->flashModel_type3 = trap_R_RegisterModel( "baseoa/models/muzzle/flash3.md3" ); 
	if (!weaponInfo->flashModel_type4) weaponInfo->flashModel_type4 = trap_R_RegisterModel( "baseoa/models/muzzle/flash4.md3" ); 
	if (!weaponInfo->flashModel_type5) weaponInfo->flashModel_type5 = trap_R_RegisterModel( "baseoa/models/muzzle/flash5.md3" ); 
	if (!weaponInfo->flashModel_type5a) weaponInfo->flashModel_type5a = trap_R_RegisterModel( "baseoa/models/muzzle/flash5a.md3" ); 

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, strippedPath, sizeof(path));
	Q_strcat( path, sizeof(path), "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "baseoa/models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	switch ( (weaponNum & 0xF) ) {
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "baseoa/sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/melee/fstatck.wav", qfalse );
		weaponInfo->lfx = 0; //  no effect
		break;

	case WP_LIGHTNING:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->readySound = trap_S_RegisterSound( "baseoa/sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "baseoa/sound/weapons/lightning/lg_hum.wav", qfalse );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/lightning/lg_fire.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "baseoa/lightningBoltNew");
		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "baseoa/models/weaphits/crackle.md3" );
		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "baseoa/sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "baseoa/sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "baseoa/sound/weapons/lightning/lg_hit3.wav", qfalse );
		weaponInfo->lfx = 0; //  no effect
		break;

#ifdef USE_GRAPPLE
	case WP_GRAPPLING_HOOK:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/ammo/hook/hook.md3" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 0;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		cgs.media.grappleShader = trap_R_RegisterShader( "baseoa/grappleRope");
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/grapple/grapfire.wav", qfalse );
		weaponInfo->missileSound = trap_S_RegisterSound( "baseoa/sound/weapons/grapple/grappull.wav", qfalse );
		//cgs.media.lightningShader = trap_R_RegisterShader( "baseoa/lightningBoltNew");
		weaponInfo->lfx = 0; //  no effect
		break;
#endif

//#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound( "baseoa/sound/weapons/vulcan/wvulfire.wav", qfalse );
		weaponInfo->loopFireSound = qtrue;
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "baseoa/sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "baseoa/sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "baseoa/sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "baseoa/bulletExplosion" );
		weaponInfo->lfx = 71;
		break;
//#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "baseoa/sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "baseoa/sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "baseoa/sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "baseoa/bulletExplosion" );
		weaponInfo->lfx = 62;
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		weaponInfo->lfx = 63;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "baseoa/sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;

		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "baseoa/rocketExplosion" );
		weaponInfo->lfx = 65;
		break;

//#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "baseoa/grenadeExplosion" );
		weaponInfo->lfx = 70;
		break;
//#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "baseoa/grenadeExplosion" );
		weaponInfo->lfx = 64;
		break;

//#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound( "baseoa/sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/nailgun/wnalfire.wav", qfalse );
		weaponInfo->lfx = 69;
		break;
//#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound( "baseoa/sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "baseoa/plasmaExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "baseoa/railDisc" );
		weaponInfo->lfx = 66;
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "baseoa/sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.5f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "baseoa/railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "baseoa/railDisc" );
		cgs.media.railCoreShader = trap_R_RegisterShader( "baseoa/railCore" );
		weaponInfo->lfx = 67;
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound( "baseoa/sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = trap_R_RegisterShader( "baseoa/bfgExplosion" );
		weaponInfo->missileModel = trap_R_RegisterModel( "baseoa/models/weaphits/bfg.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "baseoa/sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->lfx = 68;
		break;

	default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "baseoa/sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals2( int itemNum )
{
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum >= bg_numItems2 ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems2-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist2[ itemNum ];

	memset( itemInfo, 0, sizeof( itemInfo_t ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH ||
	        item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}

#endif
