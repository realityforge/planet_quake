#include "../game/q_shared.h"
#include "bg_public.h"

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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/gauntlet/gauntlet.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_gauntlet",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/shotgun/shotgun.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_shotgun",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/machinegun/machinegun.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_machinegun",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/grenadel/grenadel.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_grenade",
		/* pickup */ "Grenade Launcher",
		10,
		IT_WEAPON,
		WP_GRENADE_LAUNCHER,
		/* precache */ "",
		/* sounds */ "sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav"
	},

	/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_rocketlauncher",
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/rocketl/rocketl.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_rocket",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/lightning/lightning.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_lightning",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/railgun/railgun.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_railgun",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/plasma/plasma.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_plasma",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/bfg/bfg.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_bfg",
		/* pickup */ "BFG10K",
		20,
		IT_WEAPON,
		WP_BFG,
		/* precache */ "",
		/* sounds */ ""
	},

	/*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_grapplinghook",
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/grapple/grapple.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_grapple",
		/* pickup */ "Grappling Hook",
		0,
		IT_WEAPON,
		WP_GRAPPLING_HOOK,
		/* precache */ "",
		/* sounds */ ""
	},

  /*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
  {
		"weapon_nailgun",
		"sound/misc/w_pkup.wav",
		{ "models/weapons/nailgun/nailgun.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_nailgun",
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
		"sound/misc/w_pkup.wav",
		{ "models/weapons/proxmine/proxmine.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_proxlauncher",
		/* pickup */ "Prox Launcher",
		5,
		IT_WEAPON,
		WP_PROX_LAUNCHER,
		/* precache */ "",
		/* sounds */ "sound/weapons/proxmine/wstbtick.wav "
		"sound/weapons/proxmine/wstbactv.wav "
		"sound/weapons/proxmine/wstbimpl.wav "
		"sound/weapons/proxmine/wstbimpm.wav "
		"sound/weapons/proxmine/wstbimpd.wav "
		"sound/weapons/proxmine/wstbactv.wav"
	},

	/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	 */
	{
		"weapon_chaingun",
		"sound/misc/w_pkup.wav",
		{ "models/weapons/vulcan/vulcan.md3",
			NULL, NULL, NULL},
		/* icon */ "icons/iconw_chaingun",
		/* pickup */ "Chaingun",
		80,
		IT_WEAPON,
		WP_CHAINGUN,
		/* precache */ "",
		/* sounds */ "sound/weapons/vulcan/wvulwind.wav"
	},

	// end of list marker
	{NULL}
};


int bg_numItems2 = sizeof (bg_itemlist2) / sizeof (bg_itemlist2[0]) - 1;
