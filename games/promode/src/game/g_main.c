/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"
#include "bg_promode.h"

#define QVM_NAME       "GPP-1.1 Arena"
#define QVM_VERSIONNUM      "1.8"

level_locals_t  level;

typedef struct
{
  vmCvar_t  *vmCvar;
  char    *cvarName;
  char    *defaultString;
  int     cvarFlags;
  int     modificationCount;  // for tracking changes
  qboolean  trackChange;  // track this variable, and announce if changed
  qboolean teamShader;        // track and if changed, update shader state
} cvarTable_t;

gentity_t   g_entities[ MAX_GENTITIES ];
gclient_t   g_clients[ MAX_CLIENTS ];

vmCvar_t  g_fraglimit;
vmCvar_t  g_timelimit;
vmCvar_t  g_suddenDeathTime;
vmCvar_t  g_suddenDeath;
vmCvar_t  g_suddenDeathMode;
vmCvar_t  g_capturelimit;
vmCvar_t  g_friendlyFire;
vmCvar_t  g_friendlyFireAliens;
vmCvar_t  g_friendlyFireHumans;
vmCvar_t  g_friendlyFireMovementAttacks;
vmCvar_t  g_retribution;
vmCvar_t  g_friendlyBuildableFire;
vmCvar_t  g_password;
vmCvar_t  g_needpass;
vmCvar_t  g_maxclients;
vmCvar_t  g_maxGameClients;
vmCvar_t  g_dedicated;
vmCvar_t  g_speed;
vmCvar_t  g_gravity;
vmCvar_t  g_cheats;
vmCvar_t  g_knockback;
vmCvar_t  g_quadfactor;
vmCvar_t  g_inactivity;
vmCvar_t  g_debugMove;
vmCvar_t  g_debugDamage;
vmCvar_t  g_debugAlloc;
vmCvar_t  g_weaponRespawn;
vmCvar_t  g_weaponTeamRespawn;
vmCvar_t  g_motd;
vmCvar_t  g_synchronousClients;
vmCvar_t  g_warmup;
vmCvar_t  g_warmupMode;
vmCvar_t  g_doWarmup;
vmCvar_t  g_restarted;
vmCvar_t  g_lockTeamsAtStart;
vmCvar_t  g_logFile;
vmCvar_t  g_logFileSync;
vmCvar_t  g_blood;
vmCvar_t  g_podiumDist;
vmCvar_t  g_podiumDrop;
vmCvar_t  g_allowVote;
vmCvar_t  g_requireVoteReasons;
vmCvar_t  g_voteLimit;
vmCvar_t  g_suddenDeathVotePercent;
vmCvar_t  g_suddenDeathVoteDelay;
vmCvar_t  g_mapVotesPercent;
vmCvar_t  g_designateVotes;
vmCvar_t  g_teamAutoJoin;
vmCvar_t  g_teamForceBalance;
vmCvar_t  g_banIPs;
vmCvar_t  g_filterBan;
vmCvar_t  g_smoothClients;
vmCvar_t  g_clientUpgradeNotice;
vmCvar_t  pmove_fixed;
vmCvar_t  pmove_msec;
vmCvar_t  g_rankings;
vmCvar_t  g_listEntity;
vmCvar_t  g_minCommandPeriod;
vmCvar_t  g_minNameChangePeriod;
vmCvar_t  g_maxNameChanges;
vmCvar_t  g_newbieNumbering;
vmCvar_t  g_newbieNamePrefix;
vmCvar_t  g_newbieDenyBuild;

vmCvar_t  g_humanBuildPoints;
vmCvar_t  g_alienBuildPoints;
vmCvar_t  g_humanStage;
vmCvar_t  g_humanKills;
vmCvar_t  g_humanMaxStage;
vmCvar_t  g_humanStage2Threshold;
vmCvar_t  g_humanStage3Threshold;
vmCvar_t  g_alienStage;
vmCvar_t  g_alienKills;
vmCvar_t  g_alienMaxStage;
vmCvar_t  g_alienStage2Threshold;
vmCvar_t  g_alienStage3Threshold;
vmCvar_t  g_teamImbalanceWarnings;

vmCvar_t  g_unlagged;

vmCvar_t  g_disabledEquipment;
vmCvar_t  g_disabledClasses;
vmCvar_t  g_disabledBuildables;

vmCvar_t  g_markDeconstruct;
vmCvar_t  g_deconDead;

vmCvar_t  g_debugMapRotation;
vmCvar_t  g_currentMapRotation;
vmCvar_t  g_currentMap;
vmCvar_t  g_nextMap;
vmCvar_t  g_initialMapRotation;

vmCvar_t  g_shove;

vmCvar_t  g_mapConfigs;
vmCvar_t  g_chatTeamPrefix;
vmCvar_t  g_actionPrefix;
vmCvar_t  g_floodMaxDemerits;
vmCvar_t  g_floodMinTime;

vmCvar_t  g_layouts;
vmCvar_t  g_layoutAuto;

vmCvar_t  g_admin;
vmCvar_t  g_adminLog;
vmCvar_t  g_adminParseSay;
vmCvar_t  g_adminSayFilter;
vmCvar_t  g_adminNameProtect;
vmCvar_t  g_adminTempBan;
vmCvar_t  g_adminMaxBan;
vmCvar_t  g_adminMapLog;
vmCvar_t  g_minLevelToJoinTeam;
vmCvar_t  g_forceAutoSelect;

vmCvar_t  g_privateMessages;
vmCvar_t  g_decolourLogfiles;
vmCvar_t  g_minLevelToSpecMM1;
vmCvar_t  g_publicSayadmins;
vmCvar_t  g_myStats;
vmCvar_t  g_myStatstime;
vmCvar_t  g_antiSpawnBlock;
vmCvar_t  g_banNotice;
vmCvar_t  g_adminLeaveMsg;

vmCvar_t  g_devmapKillerHP;

vmCvar_t  g_buildLogMaxLength;

vmCvar_t  g_tag;

vmCvar_t  g_hiddenClients;

vmCvar_t  g_dretchPunt;

vmCvar_t  g_allowShare;
vmCvar_t  g_shareFee;
vmCvar_t  g_allowDonate;

vmCvar_t  g_devmapNoGod;
vmCvar_t  g_devmapNoStructDmg;

vmCvar_t  g_voteMinTime;
vmCvar_t  g_mapvoteMaxTime;

vmCvar_t  g_msg;
vmCvar_t  g_msgTime;

// <bot stuff>

// bot buy cvars
vmCvar_t g_bot_buy;
vmCvar_t g_bot_rifle;
vmCvar_t g_bot_psaw;
vmCvar_t g_bot_shotgun;
vmCvar_t g_bot_lgun;
vmCvar_t g_bot_mdriver;
vmCvar_t g_bot_chaingun;
vmCvar_t g_bot_prifle;
vmCvar_t g_bot_flamer;
vmCvar_t g_bot_lcannon;

// bot evolution cvars
vmCvar_t g_bot_evolve;
vmCvar_t g_bot_level1;
vmCvar_t g_bot_level1upg;
vmCvar_t g_bot_level2;
vmCvar_t g_bot_level2upg;
vmCvar_t g_bot_level3;
vmCvar_t g_bot_level3upg;
vmCvar_t g_bot_level4;

// misc bot cvars
vmCvar_t g_bot_attackStruct;
vmCvar_t g_bot_roam;
vmCvar_t g_bot_infinite_funds;
vmCvar_t g_bot_survival;
vmCvar_t g_bot_wave_interval;
vmCvar_t g_mode_teamkill;

//</bot stuff>
// for editing the bot's paths
vmCvar_t g_pathediting;

// cicho-sza add-on:
vmCvar_t  g_DebugMsg;
vmCvar_t  g_lesson_NoVotes;
vmCvar_t  g_ReservedNameMatch;
vmCvar_t  g_ReservedNameMinLev;
vmCvar_t  g_AutoStrip;
vmCvar_t  g_AutoStrip_MinKillToFeed;
vmCvar_t  g_AutoStrip_MinKills;
vmCvar_t  g_AutoStrip_KillsPerStage;
vmCvar_t  g_AutoStrip_BetterThanTeam;
vmCvar_t  g_AutoStrip_BetterThanEnemy;
vmCvar_t  g_AutoStrip_DebugMsg;
vmCvar_t  g_AutoStrip_KillingSpreeLvl;
vmCvar_t  g_AutoStrip_KillingSpreePerStage;
vmCvar_t  g_AutoStrip_KillingSpreeStageDif;
vmCvar_t  g_AutoStrip_kpmAllowed;
vmCvar_t  g_AutoStrip_kpmTreshold;
vmCvar_t  g_AutoStrip_NotInLesson;

vmCvar_t  g_StripEqChange;
vmCvar_t  g_StripEqChange_def;
vmCvar_t  g_StructNoDmg;
vmCvar_t  g_ShowKillerHealth;
vmCvar_t  g_lesson;
vmCvar_t  g_lesson_BlockEqStr;
vmCvar_t  g_strip_PlayerDmgPrcnt;
vmCvar_t  g_strip_PlayerDmgPrcnt_def;
vmCvar_t  g_strip_StructDmgPrcnt;
vmCvar_t  g_strip_StructDmgPrcnt_def;
vmCvar_t  g_connectedStripPrcnt;
//ZdrytchX
vmCvar_t  g_mode_vampire;
vmCvar_t  g_mode_vampireVotePercent;
vmCvar_t  g_mode_vampirebuildables;
vmCvar_t  g_mode_vampirebuildables_take;
vmCvar_t  g_bunnyhop;//TODO
vmCvar_t  g_weapswitchtime;//CANNOT EXIST! (bg/g/cg inteference)
vmCvar_t  g_blaster_ammoregen;
vmCvar_t  g_bot_name_showskill;
vmCvar_t  g_bot_granger;
vmCvar_t  g_bot_dodge_jump;
vmCvar_t  g_bot_dodge_crouch;
vmCvar_t  g_bot_ping;
vmCvar_t  g_bot_ping_compensate;
vmCvar_t  g_bot_ping_unlagged;
vmCvar_t  g_bot_gren;
vmCvar_t  g_bot_bsuit;
vmCvar_t  g_bot_gren_buypercent;
vmCvar_t  g_bot_gren_buildablesonlypercent;
vmCvar_t  g_bot_reactiontime;
vmCvar_t  g_teamStatus;
vmCvar_t  g_teamStatusTime;
vmCvar_t  g_demoState;
vmCvar_t  g_allowBlackInNames;
vmCvar_t  g_killingSpree;
vmCvar_t  g_feedingSpree;
vmCvar_t  g_minDeconLevel;
vmCvar_t  g_bot_alien_secondaryonly;
vmCvar_t  g_mode_cpm;
vmCvar_t  g_noclip_speed;
vmCvar_t  g_mode_teamkillVotePercent;
vmCvar_t  g_mode_teamkill_human_knockback;
vmCvar_t  g_autoGhost;
vmCvar_t  g_extendVotesPercent;
vmCvar_t  g_extendVotesTime;
vmCvar_t  g_extendVotesCount;
vmCvar_t  g_adminTempSpec;
vmCvar_t  g_hitsounds;
vmCvar_t  g_hitsounds_type;
vmCvar_t  g_allStats;
vmCvar_t  g_allStatsTime;

vmCvar_t  g_level4_trample_buildable_percent;

static cvarTable_t   gameCvarTable[ ] =
{
  { &g_cheats, "sv_cheats", "0", CVAR_ARCHIVE, 0, qfalse },

  // noset vars
  { NULL, "gamename", GAME_VERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
  { NULL, "gamedate", __DATE__ , CVAR_ROM, 0, qfalse  },
  { &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },
  { &g_lockTeamsAtStart, "g_lockTeamsAtStart", "0", CVAR_ROM, 0, qfalse  },
  { NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
  { NULL, "P", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
  { NULL, "ff", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
  { NULL, "qvm_version", QVM_NAME " " QVM_VERSIONNUM " (" __DATE__ ", " __TIME__ ")", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },

  // latched vars

  { &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
  { &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue  },

  // change anytime vars
  { &g_timelimit, "timelimit", "45", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_suddenDeathTime, "g_suddenDeathTime", "30", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_suddenDeathMode, "g_suddenDeathMode", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_suddenDeath, "g_suddenDeath", "0", CVAR_SERVERINFO | CVAR_NORESTART, 0, qtrue },

  { &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

  { &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue  },
  { &g_friendlyFireAliens, "g_friendlyFireAliens", "0", CVAR_ARCHIVE, 0, qtrue  },
  { &g_friendlyFireHumans, "g_friendlyFireHumans", "0", CVAR_ARCHIVE, 0, qtrue  },
  { &g_retribution, "g_retribution", "0", CVAR_ARCHIVE, 0, qtrue  },
  { &g_friendlyBuildableFire, "g_friendlyBuildableFire", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue  },
  { &g_friendlyFireMovementAttacks, "g_friendlyFireMovementAttacks", "1", CVAR_ARCHIVE, 0, qtrue  },
  { &g_devmapNoGod, "g_devmapNoGod", "0", CVAR_ARCHIVE, 0, qtrue  },
  { &g_devmapNoStructDmg, "g_devmapNoStructDmg", "0", CVAR_ARCHIVE, 0, qtrue  },

  { &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
  { &g_teamForceBalance, "g_teamForceBalance", "1", CVAR_ARCHIVE  },

  { &g_warmup, "g_warmup", "10", CVAR_ARCHIVE, 0, qtrue  },
  { &g_warmupMode, "g_warmupMode", "0", CVAR_ARCHIVE, 0, qtrue  }, //0 = allow joining, don't spawn; 1 = don't allow anything
  { &g_doWarmup, "g_doWarmup", "1", CVAR_ARCHIVE, 0, qtrue  },
  { &g_logFile, "g_logFile", "games.log", CVAR_ARCHIVE, 0, qfalse  },
  { &g_logFileSync, "g_logFileSync", "0", CVAR_ARCHIVE, 0, qfalse  },

  { &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

  { &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
  { &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

  { &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

  { &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

  { &g_speed, "g_speed", "320", CVAR_SERVERINFO, 0, qtrue  },
  { &g_gravity, "g_gravity", "800", CVAR_SERVERINFO, 0, qtrue  }, //I would like the default to be 700, as gravity accel is 9.8 m/s and a human is about 60 units high.
  { &g_knockback, "g_knockback", "1000", CVAR_SERVERINFO, 0, qtrue  },
  { &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
  { &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
  { &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue },
  { &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
  { &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
  { &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
  { &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
  { &g_motd, "g_motd", "", 0, 0, qfalse },
  { &g_blood, "com_blood", "1", 0, 0, qfalse },

  { &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
  { &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

  { &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_requireVoteReasons, "g_requireVoteReasons", "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_voteLimit, "g_voteLimit", "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_voteMinTime, "g_voteMinTime", "120", CVAR_ARCHIVE, 0, qfalse },
  { &g_mapvoteMaxTime, "g_mapvoteMaxTime", "240", CVAR_ARCHIVE, 0, qfalse },
  { &g_suddenDeathVotePercent, "g_suddenDeathVotePercent", "74", CVAR_ARCHIVE, 0, qfalse },
  { &g_suddenDeathVoteDelay, "g_suddenDeathVoteDelay", "180", CVAR_ARCHIVE, 0, qfalse },
  { &g_mapVotesPercent, "g_mapVotesPercent", "50", CVAR_ARCHIVE, 0, qfalse },
  { &g_designateVotes, "g_designateVotes", "0", CVAR_ARCHIVE, 0, qfalse },
  
  { &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },
  { &g_minCommandPeriod, "g_minCommandPeriod", "500", 0, 0, qfalse},
  { &g_minNameChangePeriod, "g_minNameChangePeriod", "5", 0, 0, qfalse},
  { &g_maxNameChanges, "g_maxNameChanges", "5", 0, 0, qfalse},
  { &g_newbieNumbering, "g_newbieNumbering", "0", CVAR_ARCHIVE, 0, qfalse},
  { &g_newbieNamePrefix, "g_newbieNamePrefix", "Newbie#", CVAR_ARCHIVE, 0, qfalse},
  { &g_newbieDenyBuild, "g_newbieDenyBuild", "0", CVAR_ARCHIVE, 0, qfalse },

  { &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
  { &g_clientUpgradeNotice, "g_clientUpgradeNotice", "1", 0, 0, qfalse},
  { &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
  { &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},

  { &g_humanBuildPoints, "g_humanBuildPoints", DEFAULT_HUMAN_BUILDPOINTS, CVAR_SERVERINFO, 0, qfalse  },
  { &g_alienBuildPoints, "g_alienBuildPoints", DEFAULT_ALIEN_BUILDPOINTS, CVAR_SERVERINFO, 0, qfalse  },
  { &g_humanStage, "g_humanStage", "0", 0, 0, qfalse  },
  { &g_humanKills, "g_humanKills", "0", 0, 0, qfalse  },
  { &g_humanMaxStage, "g_humanMaxStage", DEFAULT_HUMAN_MAX_STAGE, 0, 0, qfalse  },
  { &g_humanStage2Threshold, "g_humanStage2Threshold", DEFAULT_HUMAN_STAGE2_THRESH, 0, 0, qfalse  },
  { &g_humanStage3Threshold, "g_humanStage3Threshold", DEFAULT_HUMAN_STAGE3_THRESH, 0, 0, qfalse  },
  { &g_alienStage, "g_alienStage", "0", 0, 0, qfalse  },
  { &g_alienKills, "g_alienKills", "0", 0, 0, qfalse  },
  { &g_alienMaxStage, "g_alienMaxStage", DEFAULT_ALIEN_MAX_STAGE, 0, 0, qfalse  },
  { &g_alienStage2Threshold, "g_alienStage2Threshold", DEFAULT_ALIEN_STAGE2_THRESH, 0, 0, qfalse  },
  { &g_alienStage3Threshold, "g_alienStage3Threshold", DEFAULT_ALIEN_STAGE3_THRESH, 0, 0, qfalse  },
  
  { &g_teamImbalanceWarnings, "g_teamImbalanceWarnings", "30", CVAR_ARCHIVE, 0, qfalse  },
  
  { &g_unlagged, "g_unlagged", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse  },

  { &g_disabledEquipment, "g_disabledEquipment", "", CVAR_ROM, 0, qfalse  },
  { &g_disabledClasses, "g_disabledClasses", "", CVAR_ROM, 0, qfalse  },
  { &g_disabledBuildables, "g_disabledBuildables", "", CVAR_ROM, 0, qfalse  },

  { &g_chatTeamPrefix, "g_chatTeamPrefix", "1", CVAR_ARCHIVE  },
  { &g_actionPrefix, "g_actionPrefix", "* ", CVAR_ARCHIVE, 0, qfalse },
  { &g_floodMaxDemerits, "g_floodMaxDemerits", "5000", CVAR_ARCHIVE, 0, qfalse  },
  { &g_floodMinTime, "g_floodMinTime", "2000", CVAR_ARCHIVE, 0, qfalse  },

  { &g_markDeconstruct, "g_markDeconstruct", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse  },
  { &g_deconDead, "g_deconDead", "0", CVAR_ARCHIVE, 0, qtrue  },

  { &g_debugMapRotation, "g_debugMapRotation", "0", 0, 0, qfalse  },
  { &g_currentMapRotation, "g_currentMapRotation", "-1", 0, 0, qfalse  }, // -1 = NOT_ROTATING
  { &g_currentMap, "g_currentMap", "0", 0, 0, qfalse  },
  { &g_nextMap, "g_nextMap", "", 0 , 0, qtrue  },
  { &g_initialMapRotation, "g_initialMapRotation", "", CVAR_ARCHIVE, 0, qfalse  },
  { &g_shove, "g_shove", "15", CVAR_ARCHIVE, 0, qfalse  },
  { &g_mapConfigs, "g_mapConfigs", "", CVAR_ARCHIVE, 0, qfalse  },
  { NULL, "g_mapConfigsLoaded", "0", CVAR_ROM, 0, qfalse  },
  { NULL, "g_LayoutConfigsLoaded", "0", CVAR_ROM, 0, qfalse  },

  { &g_layouts, "g_layouts", "", CVAR_LATCH, 0, qfalse  },
  { &g_layoutAuto, "g_layoutAuto", "1", CVAR_ARCHIVE, 0, qfalse  },

  { &g_admin, "g_admin", "admin.dat", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminLog, "g_adminLog", "admin.log", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminParseSay, "g_adminParseSay", "1", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminSayFilter, "g_adminSayFilter", "0", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminNameProtect, "g_adminNameProtect", "1", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminTempBan, "g_adminTempBan", "2m", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminMaxBan, "g_adminMaxBan", "2w", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminMapLog, "g_adminMapLog", "", CVAR_ROM, 0, qfalse  },
  { &g_minLevelToJoinTeam, "g_minLevelToJoinTeam", "0", CVAR_ARCHIVE, 0, qfalse  },
  { &g_forceAutoSelect, "g_forceAutoSelect", "0", CVAR_ARCHIVE, 0, qtrue }, 
  
  { &g_privateMessages, "g_privateMessages", "1", CVAR_ARCHIVE, 0, qfalse  },
  { &g_decolourLogfiles, "g_decolourLogfiles", "0", CVAR_ARCHIVE, 0, qfalse  },
  { &g_buildLogMaxLength, "g_buildLogMaxLength", "50", CVAR_ARCHIVE, 0, qfalse  },
  { &g_myStats, "g_myStats", "1", CVAR_ARCHIVE, 0, qfalse  },
  { &g_publicSayadmins, "g_publicSayadmins", "1", CVAR_ARCHIVE, 0, qfalse  },    
  { &g_minLevelToSpecMM1, "g_minLevelToSpecMM1", "0", CVAR_ARCHIVE, 0, qfalse  },
  { &g_antiSpawnBlock, "g_antiSpawnBlock", "150", CVAR_ARCHIVE, 0, qfalse  },
  
  { &g_devmapKillerHP, "g_devmapKillerHP", "0", CVAR_ARCHIVE, 0, qtrue  },
  
  { &g_tag, "g_tag", "main", CVAR_INIT, 0, qfalse },
  
  { &g_hiddenClients, "g_hiddenClients", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },

  { &g_dretchPunt, "g_dretchPunt", "1", CVAR_ARCHIVE, 0, qfalse  },
  
  { &g_msg, "g_msg", "", CVAR_ARCHIVE, 0, qfalse  },
  { &g_msgTime, "g_msgTime", "0", CVAR_ARCHIVE, 0, qfalse  },
  
  { &g_rankings, "g_rankings", "0", 0, 0, qfalse },
  { &g_allowShare, "g_allowShare", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse},
  { &g_banNotice, "g_banNotice", "", CVAR_ARCHIVE, 0, qfalse  },
  { &g_adminLeaveMsg, "g_adminLeaveMsg", "", CVAR_ARCHIVE, 0, qfalse  },
  
  // <bot stuff>
  
  // bot buy cvars
  { &g_bot_buy, "g_bot_buy", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_rifle, "g_bot_rifle", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_psaw, "g_bot_psaw", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_shotgun, "g_bot_shotgun", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_lgun, "g_bot_lgun", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_mdriver, "g_bot_mdriver", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_chaingun, "g_bot_chaingun", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_prifle, "g_bot_prifle", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_flamer, "g_bot_flamer", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_lcannon, "g_bot_lcannon", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  
  // bot evolution cvars
  { &g_bot_evolve, "g_bot_evolve", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level1, "g_bot_level1", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level1upg, "g_bot_level1upg", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level2, "g_bot_level2", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level2upg, "g_bot_level2upg", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level3, "g_bot_level3", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level3upg, "g_bot_level3upg", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_level4, "g_bot_level4", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  
  // misc bot cvars
  { &g_bot_attackStruct, "g_bot_attackStruct", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_roam, "g_bot_roam", "1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_infinite_funds, "g_bot_infinite_funds", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_survival, "g_bot_survival", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  { &g_bot_wave_interval, "g_bot_wave_interval", "120", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  
  // </bot stuff>
  
  // for editing the bot's paths
  { &g_pathediting, "g_pathediting", "0", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
  
  // cicho-sza add-on:
  { &g_DebugMsg, "g_DebugMsg", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

  { &g_lesson_NoVotes,  "g_lesson_NoVotes", "map, map_restart, nextmap", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },

  { &g_ReservedNameMatch,  "g_ReservedNameMatche", "SuperCow#", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },
  { &g_ReservedNameMinLev, "g_ReservedNameMinLev", "2",           CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

  { &g_AutoStrip_kpmTreshold,    "g_AutoStrip_kpmTreshold",       "0", CVAR_ARCHIVE, 0, qfalse }, 
  { &g_AutoStrip_kpmAllowed,     "g_AutoStrip_kpmAllowed",        "0", CVAR_ARCHIVE, 0, qfalse }, 
  { &g_AutoStrip,                "g_AutoStrip",                   "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue },
  { &g_AutoStrip_DebugMsg,       "g_AutoStrip_DebugMsg",          "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_MinKillToFeed,  "g_AutoStrip_MinKillToFeed",   "1.0", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_MinKills,       "g_AutoStrip_MinKills",          "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_KillsPerStage,  "g_AutoStrip_KillsPerStage",     "4", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_BetterThanTeam, "g_AutoStrip_BetterThanTeam",  "100", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_BetterThanEnemy,"g_AutoStrip_BetterThanEnemy", "100", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_KillingSpreeLvl,      "g_AutoStrip_KillingSpreeLvl",      "6", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_KillingSpreePerStage, "g_AutoStrip_KillingSpreePerStage", "2", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_KillingSpreeStageDif, "g_AutoStrip_KillingSpreeStageDif", "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_AutoStrip_NotInLesson,       "g_AutoStrip_NotInLesson",          "0", CVAR_ARCHIVE, 0, qfalse },

  { &g_StripEqChange,     "g_StripEqChange",     "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },
  { &g_StripEqChange_def, "g_StripEqChange_def", "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },
  
  { &g_StructNoDmg, "g_StructNoDmg", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_ShowKillerHealth, "g_ShowKillerHealth", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_lesson, "g_lesson", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
  { &g_lesson_BlockEqStr, "g_lesson_BlockEqStr", "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },

  { &g_strip_PlayerDmgPrcnt,     "g_strip_PlayerDmgPrcnt",     "100", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 100, qtrue },
  { &g_strip_PlayerDmgPrcnt_def, "g_strip_PlayerDmgPrcnt_def", "100", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 100, qtrue },
  { &g_strip_StructDmgPrcnt,     "g_strip_StructDmgPrcnt",     "100", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 100, qtrue },
  { &g_strip_StructDmgPrcnt_def, "g_strip_StructDmgPrcnt_def", "100", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 100, qtrue },
  { &g_connectedStripPrcnt,      "g_connectedStripPrcnt",       "50", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART,  50, qtrue },
//ZdrytchX
//  { &g_bunnyhop,         "g_bunnyhop",                          "0", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE,  0, qfalse },//TODO
  { &g_weapswitchtime,   "g_weapswitchtime","250", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE,  0, qfalse },//cannot happen, useless var
  { &g_blaster_ammoregen,   "g_blaster_ammoregen",              "1250", CVAR_ARCHIVE,  0, qfalse },

  { &g_bot_name_showskill,   "g_bot_name_showskill",              "1", CVAR_ARCHIVE,  0, qfalse },
  { &g_bot_granger,   "g_bot_granger",                            "0", CVAR_ARCHIVE,  0, qfalse },
  { &g_bot_dodge_jump,   "g_bot_dodge_jump",                      "1", CVAR_ARCHIVE,  1, qfalse },
  { &g_bot_dodge_crouch,   "g_bot_dodge_crouch",                  "1", CVAR_ARCHIVE,  1, qfalse },
  { &g_bot_ping,   "g_bot_ping",                                 "10", CVAR_ARCHIVE,  0, qfalse },
  //this actually just takes away the g_bot_ping aim behind effect, so it can't simulate real ping compensation
  //Idea: Use g_bot_ping with unlagged to track back players, THEN aim, then track players to normal position
  //THEN use g_bot_ping_compensate to predict their position.
  { &g_bot_ping_compensate,   "g_bot_ping_compensate",           "70", CVAR_ARCHIVE,  0, qfalse },
  { &g_bot_ping_unlagged,   "g_bot_ping_unlagged",                "0", CVAR_ARCHIVE,  0, qfalse },

  { &g_bot_gren,   "g_bot_gren",                                 "1",  CVAR_ARCHIVE,  0, qfalse },
  { &g_bot_bsuit,  " g_bot_bsuit",                              "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_bot_reactiontime, "g_bot_reactiontime",                  "600", CVAR_ARCHIVE, 0, qfalse },
  { &g_bot_gren_buypercent, "g_bot_gren_buypercent",             "20",           CVAR_ARCHIVE,  0, qfalse },
  { &g_bot_gren_buildablesonlypercent, "g_bot_gren_buildablesonlypercent", "20",  CVAR_ARCHIVE, 0, qfalse },
  { &g_teamStatus, "g_teamStatus",                               "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_teamStatusTime, "g_teamStatusTime",                       "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_demoState, "sv_demoState",                                "", 0,             0, qfalse },
  { &g_allowBlackInNames, "g_allowBlackInNames",                 "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_killingSpree, "g_killingSpree",                           "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_feedingSpree, "g_feedingSpree",                           "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_minDeconLevel, "g_minDeconLevel",                         "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_bot_alien_secondaryonly, "g_bot_alien_secondaryonly",     "0", CVAR_ARCHIVE, 0, qfalse },//TODO
  { &g_myStatstime, "g_myStatstime",                             "5", CVAR_ARCHIVE, 0, qfalse },
  { &g_allStats, "g_allStats",                                   "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_allStatsTime, "g_allStatsTime",                          "25", CVAR_ARCHIVE, 0, qfalse },
  { &g_noclip_speed, "g_noclip_speed",                           "2", CVAR_ARCHIVE, 0, qfalse },//nil
  { &g_extendVotesPercent, "g_extendVotesPercent",              "75", CVAR_ARCHIVE, 0, qfalse },
  { &g_extendVotesTime, "g_extendVotesTime",                    "15", CVAR_ARCHIVE, 0, qfalse },
  { &g_extendVotesCount, "g_extendVotesCount",                   "2", CVAR_ARCHIVE, 0, qfalse },
  { &g_adminTempSpec, "g_adminTempSpec",                       "120", CVAR_ARCHIVE, 0, qfalse },

  { &g_autoGhost, "g_autoGhost",                                 "1", CVAR_ARCHIVE, 0, qfalse },
  //g_hitsounds_type is an bitfield variable from 0-7
  //4 = No teammates, 2 = no buildables, 1 = monotone
  { &g_hitsounds, "g_hitsounds",                                 "1", CVAR_ARCHIVE, 0, qfalse },
  { &g_hitsounds_type, "g_hitsounds_type",                       "4", CVAR_ARCHIVE, 0, qfalse },  { &g_level4_trample_buildable_percent, "g_level4_trample_buildable_percent","35", CVAR_ARCHIVE,0, qfalse },

  //Special Modes
  { &g_mode_cpm, "g_mode_cpm",                            "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },
  { &g_mode_vampire, "g_mode_vampire",                    "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },
  { &g_mode_vampireVotePercent, "g_mode_vampireVotePercent",               "76", CVAR_ARCHIVE, 0, qfalse },
  { &g_mode_vampirebuildables,      "g_mode_vampirebuildables",             "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_mode_vampirebuildables_take,      "g_mode_vampirebuildables_take",  "50", CVAR_ARCHIVE, 0, qfalse },
  { &g_mode_teamkill, "g_mode_teamkill",                                    "0", CVAR_ARCHIVE, 0, qfalse },
  { &g_mode_teamkillVotePercent, "g_mode_teamkillVotePercent",             "80", CVAR_ARCHIVE, 0, qfalse },
  { &g_mode_teamkill_human_knockback, "g_mode_teamkill_human_knockback",  "1.8", CVAR_ARCHIVE, 0, qfalse },
};

static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[ 0 ] );


void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void CheckExitRules( void );

void G_DemoSetClient( void );
void G_DemoRemoveClient( void );
void G_DemoSetStage( void );

void G_CountSpawns( void );
void G_CalculateBuildPoints( void );
void G_CalculateWaves(void);

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4,
                              int arg5, int arg6, int arg7, int arg8, int arg9,
                              int arg10, int arg11 )
{
  switch( command )
  {
    case GAME_INIT:
      G_InitGame( arg0, arg1, arg2 );
      return 0;

    case GAME_SHUTDOWN:
      G_ShutdownGame( arg0 );
      return 0;

    case GAME_CLIENT_CONNECT:
      return (intptr_t)ClientConnect( arg0, arg1 );

    case GAME_CLIENT_THINK:
      ClientThink( arg0 );
      return 0;

    case GAME_CLIENT_USERINFO_CHANGED:
      ClientUserinfoChanged( arg0 );
      return 0;

    case GAME_CLIENT_DISCONNECT:
      ClientDisconnect( arg0 );
      return 0;

    case GAME_CLIENT_BEGIN:
      ClientBegin( arg0 );
      return 0;

    case GAME_CLIENT_COMMAND:
      ClientCommand( arg0 );
      return 0;

    case GAME_RUN_FRAME:
      G_RunFrame( arg0 );
      return 0;

    case GAME_CONSOLE_COMMAND:
      return ConsoleCommand( );

    case GAME_DEMO_COMMAND:
      switch ( arg0 )
      {
      case DC_CLIENT_SET:
        G_DemoSetClient( );
        break;
      case DC_CLIENT_REMOVE:
        G_DemoRemoveClient( );
        break;
      case DC_SET_STAGE:
        G_DemoSetStage( );
        break;
      }
      return 0;
  }

  return -1;
}


void QDECL G_Printf( const char *fmt, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, fmt );
  vsprintf( text, fmt, argptr );
  va_end( argptr );

  trap_Printf( text );
}

void QDECL G_Error( const char *fmt, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, fmt );
  vsprintf( text, fmt, argptr );
  va_end( argptr );

  trap_Error( text );
}





/* 
 *  cicho-sza add-on
 *
 *    StringList - string such like "map, next_map, map_restart" containing set
 *                 of values delimited by , or ;. Space is allowed (will be ommited
 *                 while checking)
 *
 *    Item - string that we want to check if it is contained in StringList
 *
 *  returns: qtrue if Item is member of StringList
 */
qboolean IsStringListMember( char* StringList, char * Item )
{
  char StringListClearStr[MAX_CVAR_VALUE_STRING+3];
  char ItemToFind[MAX_CVAR_VALUE_STRING+3];

  int i;
  int p;
  int cnt;

  // sanity checks
  if (!Item || Item[ 0 ] == '\0' )
    return qfalse;

  if (!StringList || StringList[ 0 ] == '\0' )
    return qfalse;


  // normalise (remove whitespace, make , from ; etc) g_lesson_BlockEqStr value
  StringListClearStr[0] = ',';
  p = 1;
  cnt = strlen(StringList);
  for( i = 0; i < cnt; ++i )
  {
    if (StringList[i] != ' ')
    {
      if (StringList[i] == ';')
        StringListClearStr[p] = ',';
      else
        StringListClearStr[p] = StringList[i];
      ++p;
    }
  }
  StringListClearStr[p]   = ',';
  StringListClearStr[p+1] = '\0';


  // convert input parameter to ,something, string
  ItemToFind[0] = ',';
  p = 1;
  cnt = strlen(Item);
  for( i = 0; i < cnt; ++i )
  {
    if (Item[i] != ' ')
      {
        if (Item[i] == ';')
          ItemToFind[p] = ',';
        else
          ItemToFind[p] = Item[i];
        ++p;
      }
  }
  ItemToFind[p]   = ',';
  ItemToFind[p+1] = '\0';


  // if item is on list, return true otherwide false :)
  if (!strstr(StringListClearStr, ItemToFind))
    return qfalse;
  else
    return qtrue;
}





/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void )
{
  gentity_t *e, *e2;
  int       i, j;
  int       c, c2;

  c = 0;
  c2 = 0;

  for( i = 1, e = g_entities+i; i < level.num_entities; i++, e++ )
  {
    if( !e->inuse )
      continue;

    if( !e->team )
      continue;

    if( e->flags & FL_TEAMSLAVE )
      continue;

    e->teammaster = e;
    c++;
    c2++;

    for( j = i + 1, e2 = e + 1; j < level.num_entities; j++, e2++ )
    {
      if( !e2->inuse )
        continue;

      if( !e2->team )
        continue;

      if( e2->flags & FL_TEAMSLAVE )
        continue;

      if( !strcmp( e->team, e2->team ) )
      {
        c2++;
        e2->teamchain = e->teamchain;
        e->teamchain = e2;
        e2->teammaster = e;
        e2->flags |= FL_TEAMSLAVE;

        // make sure that targets only point at the master
        if( e2->targetname )
        {
          e->targetname = e2->targetname;
          e2->targetname = NULL;
        }
      }
    }
  }

  G_Printf( "%i teams with %i entities\n", c, c2 );
}

void G_RemapTeamShaders( void )
{
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void )
{
  int         i;
  cvarTable_t *cv;
  qboolean    remapped = qfalse;

  for( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ )
  {
    trap_Cvar_Register( cv->vmCvar, cv->cvarName,
      cv->defaultString, cv->cvarFlags );

    if( cv->vmCvar )
      cv->modificationCount = cv->vmCvar->modificationCount;
      
				// CPM: Detect if g_pro_mode has been changed
				if (!strcmp(cv->cvarName,"g_mode_cpm"))
				{
				/* kept this here because I'm still not sure what they're asking about, probably distinguishing between ctf and ffa etc.
					// Update all settings
					CPM_UpdateSettings((g_pro_mode.integer) ?
						((g_gametype.integer == GT_TEAM) ? 2 : 1) : 0);
				*/

					// Set the config string (so clients will be updated)
					trap_SetConfigstring(CS_PRO_MODE, va("%d", g_mode_cpm.integer));

					// Update all pro mode-dependent server-side cvars					
					if (g_mode_cpm.integer)
					{
						trap_Cvar_Set("g_blaster_ammoregen", "1250" ); // protrem default
						// we'll add more stuff here later
					}
					else
					{
						trap_Cvar_Set("g_blaster_ammoregen", "1400" ); // vtrem default
					}
				}
				// !CPM

    if( cv->teamShader )
      remapped = qtrue;
  }

  if( remapped )
    G_RemapTeamShaders( );
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void )
{
  int         i;
  cvarTable_t *cv;
  qboolean    remapped = qfalse;

  for( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ )
  {
    if( cv->vmCvar )
    {
      trap_Cvar_Update( cv->vmCvar );

      if( cv->modificationCount != cv->vmCvar->modificationCount )
      {
        cv->modificationCount = cv->vmCvar->modificationCount;

        if( cv->trackChange )
        {
          trap_SendServerCommand( -1, va( "print \"Server: %s changed to %s\n\"",
            cv->cvarName, cv->vmCvar->string ) );
          // update serverinfo in case this cvar is passed to clients indirectly
          CalculateRanks( );
        }

        if( cv->teamShader )
          remapped = qtrue;
      }
    }
  }

  if( remapped )
    G_RemapTeamShaders( );
}

/*
=================
G_MapConfigs
=================
*/
void G_MapConfigs( const char *mapname )
{

  if( !g_mapConfigs.string[0] )
    return;

  if( trap_Cvar_VariableIntegerValue( "g_mapConfigsLoaded" ) )
    return;

  trap_SendConsoleCommand( EXEC_APPEND,
    va( "exec \"%s/default.cfg\"\n", g_mapConfigs.string ) );
  
  trap_SendConsoleCommand( EXEC_APPEND,
    va( "exec \"%s/%s.cfg\"\n", g_mapConfigs.string, mapname ) );

  trap_Cvar_Set( "g_mapConfigsLoaded", "1" );
}
/*
============
G_PathLoad

============
*/

void G_PathLoad( void )
{
        int i = 0;
        int k = 0;
        int n = 0;
        
        fileHandle_t f;
        int len;
        char *path;
        char line[ MAX_STRING_CHARS ];
        char map[ MAX_QPATH ];
        level.numNodes = 0;
        trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
        for(i = 0; i < MAX_NODES;i++)
        {
                level.nodes[i].coord[0] = 0;
                level.nodes[i].coord[1] = 0;
                level.nodes[i].coord[2] = 0;
                level.nodes[i].nextid[0] = -1;
                level.nodes[i].nextid[1] = -1;
                level.nodes[i].nextid[2] = -1;
                level.nodes[i].nextid[3] = -1;
                level.nodes[i].nextid[4] = -1;
                level.nodes[i].random = -1;
                level.nodes[i].timeout = 10000;
                level.nodes[i].action = 0;
        }
        len = trap_FS_FOpenFile( va( "paths/%s/path.dat", map ), &f, FS_READ );
        if( len < 0 )
        {
                G_Printf( "Found no path.dat for map\n" );
                return;
        }
        path = G_Alloc( len + 1 );
        trap_FS_Read( path, len, f );
        *( path + len ) = '\0';
        trap_FS_FCloseFile( f );
        i = 0;
        while( *path )
        {
                if( i >= sizeof( line ) - 1 )
                {
                        G_Printf( "Error loading path.dat for map\n" );
                        return;
                }
                line[ i++ ] = *path;
                line[ i ] = '\0';
                if( *path == '\n' )
                {
                        i = 0; 
                        if( level.numNodes >= MAX_NODES ){G_Printf( "Reached path limit\n" );return;}
                        sscanf( line, "%d %f %f %f %d %d %d %d %d %d %d %d\n", 
                                                &level.numNodes,
                                                &level.nodes[level.numNodes].coord[0], 
                                                &level.nodes[level.numNodes].coord[1], 
                                                &level.nodes[level.numNodes].coord[2],
                                                &level.nodes[level.numNodes].nextid[0],
                                                &level.nodes[level.numNodes].nextid[1],
                                                &level.nodes[level.numNodes].nextid[2],
                                                &level.nodes[level.numNodes].nextid[3],
                                                &level.nodes[level.numNodes].nextid[4],
                                                &level.nodes[level.numNodes].random,
                                                &level.nodes[level.numNodes].timeout,
                                                &level.nodes[level.numNodes].action ); 
                        if(level.nodes[level.numNodes].timeout <= 0){level.nodes[level.numNodes].timeout = 10000;}
                        level.numNodes ++;
                }
        path++;
        }
        
        //generate a lookup table of the distances between connected nodes when the map starts
        //saves CPU during gameplay (distances are used for route finding)
        for(i=0;i<MAX_NODES;i++) {
            for(k=0;k<MAX_NODES;k++) {
                for(n=0;n<5;n++){
                    if(level.nodes[i].nextid[n] == k) {
                        level.distNode[i][k] = (int) Distance(level.nodes[i].coord,level.nodes[k].coord);
                        n=5;
                    }
                    else
                        level.distNode[i][k] = 999999999; //make distance huge to make route finding algorithm not use it
                }
            }
        }
        G_Printf( va("Loaded %d nodes\n", level.numNodes) );
}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart )
{
  int i;
  char buffer[ MAX_CVAR_VALUE_STRING ];
  int a, b;

  srand( randomSeed );

  G_RegisterCvars( );

  G_Printf( "------- Game Initialization -------\n" );
  G_Printf( "gamename: %s\n", GAME_VERSION );
  G_Printf( "gamedate: %s\n", __DATE__ );

  G_ProcessIPBans( );

  G_InitMemory( );
  
  // CPM: Initialize
	// Update all settings
	/* kept this here because I'm still not sure what they're asking about, probably distinguishing between ctf and ffa etc.
	CPM_UpdateSettings((g_pro_mode.integer) ?
		((g_gametype.integer == GT_TEAM) ? 2 : 1) : 0);
	*/
  // Set the config string
	trap_SetConfigstring(CS_PRO_MODE, va("%d", g_mode_cpm.integer)); //dafaq did 'd' come from?
	// !CPM

  // set some level globals
  memset( &level, 0, sizeof( level ) );
  level.time = levelTime;
  level.startTime = levelTime;
  level.alienStage2Time = level.alienStage3Time =
    level.humanStage2Time = level.humanStage3Time = level.startTime;
  trap_Cvar_VariableStringBuffer( "session", buffer, sizeof( buffer ) );
  sscanf( buffer, "%i %i", &a, &b );
  if ( a != trap_Cvar_VariableIntegerValue( "sv_maxclients" ) ||
       b != trap_Cvar_VariableIntegerValue( "sv_democlients" ) )
    level.newSession = qtrue;

  level.snd_fry = G_SoundIndex( "sound/misc/fry.wav" ); // FIXME standing in lava / slime

  trap_Cvar_Set( "qvm_version",
                 QVM_NAME " " QVM_VERSIONNUM " (" __DATE__ ", " __TIME__ ")" );

  if( g_logFile.string[ 0 ] )
  {
    if( g_logFileSync.integer )
      trap_FS_FOpenFile( g_logFile.string, &level.logFile, FS_APPEND_SYNC );
    else
      trap_FS_FOpenFile( g_logFile.string, &level.logFile, FS_APPEND );

    if( !level.logFile )
      G_Printf( "WARNING: Couldn't open logfile: %s\n", g_logFile.string );
    else
    {
      char serverinfo[ MAX_INFO_STRING ];
      qtime_t qt;
      int t;


      trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

      G_LogPrintf( "------------------------------------------------------------\n" );
      G_LogPrintf( "InitGame: %s\n", serverinfo );

      t = trap_RealTime( &qt );
      G_LogPrintf("RealTime: %04i/%02i/%02i %02i:%02i:%02i\n",
            qt.tm_year+1900, qt.tm_mon+1, qt.tm_mday, 
            qt.tm_hour, qt.tm_min, qt.tm_sec );

    }
  }
  else
    G_Printf( "Not logging to disk\n" );

  {
    char map[ MAX_CVAR_VALUE_STRING ] = {""};

    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
    G_MapConfigs( map );
  }

  // cicho-sza add-on:
  trap_Cvar_Set( "g_StripEqChange", g_StripEqChange_def.string );
  trap_Cvar_Set( "g_StructNoDmg", "0");
  trap_Cvar_Set( "g_lesson", "0" );
  trap_Cvar_Set( "g_lesson_BlockEqStr", "" );
  trap_Cvar_Set( "g_strip_PlayerDmgPrcnt", g_strip_PlayerDmgPrcnt_def.string );
  trap_Cvar_Set( "g_strip_StructDmgPrcnt", g_strip_StructDmgPrcnt_def.string );

  // we're done with g_mapConfigs, so reset this for the next map
  trap_Cvar_Set( "g_mapConfigsLoaded", "0" );
  trap_Cvar_Set( "g_LayoutConfigsLoaded", "0" );

  if ( g_admin.string[ 0 ] ) {
    G_admin_readconfig( NULL, 0 );
  }

  // initialize all entities for this game
  memset( g_entities, 0, MAX_GENTITIES * sizeof( g_entities[ 0 ] ) );
  level.gentities = g_entities;

  // initialize all clients for this game
  level.maxclients = g_maxclients.integer;
  memset( g_clients, 0, MAX_CLIENTS * sizeof( g_clients[ 0 ] ) );
  level.clients = g_clients;

  // set client fields on player ents
  for( i = 0; i < level.maxclients; i++ )
    g_entities[ i ].client = level.clients + i;

  // always leave room for the max number of clients,
  // even if they aren't all used, so numbers inside that
  // range are NEVER anything but clients
  level.num_entities = MAX_CLIENTS;

  // let the server system know where the entites are
  trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
    &level.clients[ 0 ].ps, sizeof( level.clients[ 0 ] ) );

  trap_SetConfigstring( CS_INTERMISSION, "0" );

  // update maplog
  G_admin_maplog_update( );

  // test to see if a custom buildable layout will be loaded
  G_LayoutSelect( );

  // parse the key/value pairs and spawn gentities
  G_SpawnEntitiesFromString( );

  // load up a custom building layout if there is one
  G_LayoutLoad( );

  // the map might disable some things
  BG_InitAllowedGameElements( );

  // general initialization
  G_FindTeams( );

  //load a path file
  G_PathLoad( );
  
  //TA:
  BG_InitClassOverrides( );
  BG_InitBuildableOverrides( );
  G_InitDamageLocations( );
  G_InitMapRotations( );
  G_InitSpawnQueue( &level.alienSpawnQueue );
  G_InitSpawnQueue( &level.humanSpawnQueue );

  if( g_debugMapRotation.integer )
    G_PrintRotations( );

  //reset stages
  trap_Cvar_Set( "g_alienStage", va( "%d", S1 ) );
  trap_Cvar_Set( "g_humanStage", va( "%d", S1 ) );
  trap_Cvar_Set( "g_alienKills", 0 );
  trap_Cvar_Set( "g_humanKills", 0 );
  trap_Cvar_Set( "g_suddenDeath", 0 );
  level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
  level.lastWaveTime = level.startTime;
  level.currentWave = 1;

  G_Printf( "-----------------------------------\n" );

  G_RemapTeamShaders( );

  //TA: so the server counts the spawns without a client attached
  G_CountSpawns( );

  G_ResetPTRConnections( );
  
  if(g_lockTeamsAtStart.integer)
  {
    level.alienTeamLocked=qtrue;
    level.humanTeamLocked=qtrue;
    trap_Cvar_Set( "g_lockTeamsAtStart", "0" );
  }
}

/*
==================
G_ClearVotes

remove all currently active votes
==================
*/
static void G_ClearVotes( void )
{
  level.voteTime = 0;
  trap_SetConfigstring( CS_VOTE_TIME, "" );
  trap_SetConfigstring( CS_VOTE_STRING, "" );
  level.teamVoteTime[ 0 ] = 0;
  trap_SetConfigstring( CS_TEAMVOTE_TIME, "" );
  trap_SetConfigstring( CS_TEAMVOTE_STRING, "" );
  level.teamVoteTime[ 1 ] = 0;
  trap_SetConfigstring( CS_TEAMVOTE_TIME + 1, "" );
  trap_SetConfigstring( CS_TEAMVOTE_STRING + 1, "" );
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart )
{
  // in case of a map_restart
  G_ClearVotes( );

  G_Printf( "==== ShutdownGame ====\n" );

  if( level.logFile )
  {
    G_LogPrintf( "ShutdownGame:\n" );
    G_LogPrintf( "------------------------------------------------------------\n" );
    trap_FS_FCloseFile( level.logFile );
  }

  // write all the client session data so we can get it back
  G_WriteSessionData( );

  G_admin_cleanup( );
  G_admin_namelog_cleanup( );
  G_admin_adminlog_cleanup( );

  level.restarted = qfalse;
  level.surrenderTeam = PTE_NONE;
  trap_SetConfigstring( CS_WINNER, "" );
}



//===================================================================

void QDECL Com_Error( int level, const char *error, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, error );
  vsprintf( text, error, argptr );
  va_end( argptr );

  G_Error( "%s", text );
}

void QDECL Com_Printf( const char *msg, ... )
{
  va_list argptr;
  char    text[ 1024 ];

  va_start( argptr, msg );
  vsprintf( text, msg, argptr );
  va_end( argptr );

  G_Printf( "%s", text );
}

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/


/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b )
{
  gclient_t *ca, *cb;

  ca = &level.clients[ *(int *)a ];
  cb = &level.clients[ *(int *)b ];

  // then sort by score
  if( ca->pers.score > cb->pers.score )
    return -1;
  else if( ca->pers.score < cb->pers.score )
    return 1;
  else
    return 0;
}

/*
============
G_InitSpawnQueue

Initialise a spawn queue
============
*/
void G_InitSpawnQueue( spawnQueue_t *sq )
{
  int i;

  sq->back = sq->front = 0;
  sq->back = QUEUE_MINUS1( sq->back );

  //0 is a valid clientNum, so use something else
  for( i = 0; i < MAX_CLIENTS; i++ )
    sq->clients[ i ] = -1;
}

/*
============
G_GetSpawnQueueLength

Return tha length of a spawn queue
============
*/
int G_GetSpawnQueueLength( spawnQueue_t *sq )
{
  int length = sq->back - sq->front + 1;

  while( length < 0 )
    length += MAX_CLIENTS;

  while( length >= MAX_CLIENTS )
    length -= MAX_CLIENTS;

  return length;
}


/*
============
G_PopSpawnQueue

Remove from front element from a spawn queue
============
*/
int G_PopSpawnQueue( spawnQueue_t *sq )
{
  int clientNum = sq->clients[ sq->front ];

  if( G_GetSpawnQueueLength( sq ) > 0 )
  {
    sq->clients[ sq->front ] = -1;
    sq->front = QUEUE_PLUS1( sq->front );
    g_entities[ clientNum ].client->ps.pm_flags &= ~PMF_QUEUED;

    return clientNum;
  }
  else
    return -1;
}

/*
============
G_PeekSpawnQueue

Look at front element from a spawn queue
============
*/
int G_PeekSpawnQueue( spawnQueue_t *sq )
{
  return sq->clients[ sq->front ];
}

/*
============
G_SearchSpawnQueue

Look to see if clientNum is already in the spawnQueue
============
*/
qboolean G_SearchSpawnQueue( spawnQueue_t *sq, int clientNum )
{
  int i;

  for( i = 0; i < MAX_CLIENTS; i++ )
    if( sq->clients[ i ] == clientNum )
      return qtrue;
  return qfalse;
}

/*
============
G_PushSpawnQueue

Add an element to the back of the spawn queue
============
*/
qboolean G_PushSpawnQueue( spawnQueue_t *sq, int clientNum )
{
  // don't add the same client more than once
  if( G_SearchSpawnQueue( sq, clientNum ) )
    return qfalse;

  sq->back = QUEUE_PLUS1( sq->back );
  sq->clients[ sq->back ] = clientNum;

  g_entities[ clientNum ].client->ps.pm_flags |= PMF_QUEUED;
  return qtrue;
}

/*
============
G_RemoveFromSpawnQueue

remove a specific client from a spawn queue
============
*/
qboolean G_RemoveFromSpawnQueue( spawnQueue_t *sq, int clientNum )
{
  int i = sq->front;

  if( G_GetSpawnQueueLength( sq ) )
  {
    do
    {
      if( sq->clients[ i ] == clientNum )
      {
        //and this kids is why it would have
        //been better to use an LL for internal
        //representation
        do
        {
          sq->clients[ i ] = sq->clients[ QUEUE_PLUS1( i ) ];

          i = QUEUE_PLUS1( i );
        } while( i != QUEUE_PLUS1( sq->back ) );

        sq->back = QUEUE_MINUS1( sq->back );
        g_entities[ clientNum ].client->ps.pm_flags &= ~PMF_QUEUED;

        return qtrue;
      }

      i = QUEUE_PLUS1( i );
    } while( i != QUEUE_PLUS1( sq->back ) );
  }

  return qfalse;
}

/*
============
G_GetPosInSpawnQueue

Get the position of a client in a spawn queue
============
*/
int G_GetPosInSpawnQueue( spawnQueue_t *sq, int clientNum )
{
  int i = sq->front;

  if( G_GetSpawnQueueLength( sq ) )
  {
    do
    {
      if( sq->clients[ i ] == clientNum )
      {
        if( i < sq->front )
          return i + MAX_CLIENTS - sq->front;
        else
          return i - sq->front;
      }

      i = QUEUE_PLUS1( i );
    } while( i != QUEUE_PLUS1( sq->back ) );
  }

  return -1;
}

/*
============
G_PrintSpawnQueue

Print the contents of a spawn queue
============
*/
void G_PrintSpawnQueue( spawnQueue_t *sq )
{
  int i = sq->front;
  int length = G_GetSpawnQueueLength( sq );

  G_Printf( "l:%d f:%d b:%d    :", length, sq->front, sq->back );

  if( length > 0 )
  {
    do
    {
      if( sq->clients[ i ] == -1 )
        G_Printf( "*:" );
      else
        G_Printf( "%d:", sq->clients[ i ] );

      i = QUEUE_PLUS1( i );
    } while( i != QUEUE_PLUS1( sq->back ) );
  }

  G_Printf( "\n" );
}

/*
============
G_SpawnClients

Spawn queued clients
============
*/
void G_SpawnClients( pTeam_t team )
{
  int           clientNum;
  gentity_t     *ent, *spawn;
  vec3_t        spawn_origin, spawn_angles;
  spawnQueue_t  *sq = NULL;
  int           numSpawns = 0;
  if( g_doWarmup.integer && ( g_warmupMode.integer==1 || g_warmupMode.integer == 2 ) &&
      level.time - level.startTime < g_warmup.integer * 1000 )
  {
    return;
  }
  if( team == PTE_ALIENS )
  {
    sq = &level.alienSpawnQueue;
    numSpawns = level.numAlienSpawns;
  }
  else if( team == PTE_HUMANS )
  {
    sq = &level.humanSpawnQueue;
    numSpawns = level.numHumanSpawns;
  }

  if( G_GetSpawnQueueLength( sq ) > 0 && numSpawns > 0 )
  {
    clientNum = G_PeekSpawnQueue( sq );
    ent = &g_entities[ clientNum ];

    if( ( spawn = G_SelectTremulousSpawnPoint( team,
            ent->client->pers.lastDeathLocation,
            spawn_origin, spawn_angles ) ) )
    {
      clientNum = G_PopSpawnQueue( sq );

      if( clientNum < 0 )
        return;

      ent = &g_entities[ clientNum ];

      ent->client->sess.sessionTeam = TEAM_FREE;
      ClientUserinfoChanged( clientNum );
      ClientSpawn( ent, spawn, spawn_origin, spawn_angles );
    }
  }
}

/*
============
G_CountSpawns

Counts the number of spawns for each team
============
*/
void G_CountSpawns( void )
{
  int i;
  gentity_t *ent;

  level.numAlienSpawns = 0;
  level.numHumanSpawns = 0;

  for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
  {
    if( !ent->inuse )
      continue;

    if( ent->s.modelindex == BA_A_SPAWN && ent->health > 0 )
      level.numAlienSpawns++;

    if( ent->s.modelindex == BA_H_SPAWN && ent->health > 0 )
      level.numHumanSpawns++;
  }

  //let the client know how many spawns there are
  trap_SetConfigstring( CS_SPAWNS, va( "%d %d",
        level.numAlienSpawns, level.numHumanSpawns ) );
}

/*
============
G_TimeTilSuddenDeath
============
*/
int G_TimeTilSuddenDeath( void )
{
  if( (!g_suddenDeathTime.integer && level.suddenDeathBeginTime==0 ) || level.suddenDeathBeginTime<0 )
    return 999999999; // Always some time away

  return ( ( level.suddenDeathBeginTime ) - ( level.time - level.startTime ) );
}


#define PLAYER_COUNT_MOD 5.0f

/*
============
G_CalculateBuildPoints

Recalculate the quantity of building points available to the teams
============
*/
void G_CalculateBuildPoints( void )
{
  int         i;
  buildable_t buildable;
  gentity_t   *ent;
  int         localHTP = g_humanBuildPoints.integer,
              localATP = g_alienBuildPoints.integer;

  // g_suddenDeath sets what state we want it to be.  
  // level.suddenDeath says whether we've calculated BPs at the 'start' of SD or not

  // reset if SD was on, but now it's off
  if(!g_suddenDeath.integer && level.suddenDeath) 
  {
    level.suddenDeath=qfalse;
    level.suddenDeathWarning=0;
    level.suddenDeathBeginTime = -1;
    if((level.time - level.startTime) < (g_suddenDeathTime.integer * 60000 ) )
      level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
    else
      level.suddenDeathBeginTime = -1;
  }

  if(!level.suddenDeath)
  {
    if(g_suddenDeath.integer || G_TimeTilSuddenDeath( ) <= 0 ) //Conditions to enter SD
    {
      //begin sudden death
      if( level.suddenDeathWarning < TW_PASSED )
      {
        trap_SendServerCommand( -1, "cp \"Sudden Death!\"" );
        G_LogPrintf("Beginning Sudden Death (Mode %d)\n",g_suddenDeathMode.integer);
        localHTP = 0;
        localATP = 0;

        if( g_suddenDeathMode.integer == SDMODE_SELECTIVE )
        {
          for( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
          {
            if( ent->s.eType != ET_BUILDABLE )
              continue;
        
            if( BG_FindReplaceableTestForBuildable( ent->s.modelindex ) )
            {
              int t = BG_FindTeamForBuildable( ent->s.modelindex );
        
              if( t == BIT_HUMANS )
                localHTP += BG_FindBuildPointsForBuildable( ent->s.modelindex );
              else if( t == BIT_ALIENS )
                localATP += BG_FindBuildPointsForBuildable( ent->s.modelindex );
            }
          }
        }
        level.suddenDeathHBuildPoints = localHTP;
        level.suddenDeathABuildPoints = localATP;
        level.suddenDeathBeginTime = level.time;
        level.suddenDeath=qtrue;
        trap_Cvar_Set( "g_suddenDeath", "1" );

        level.suddenDeathWarning = TW_PASSED;
      }
    }  
    else 
    {
       //warn about sudden death
       if( ( G_TimeTilSuddenDeath( ) <= 60000 ) &&
           (  level.suddenDeathWarning < TW_IMMINENT ) )
       {
         trap_SendServerCommand( -1, va("cp \"Sudden Death in %d seconds!\"", 
               (int)(G_TimeTilSuddenDeath() / 1000 ) ) );
         level.suddenDeathWarning = TW_IMMINENT;
       }
    }
  }
  
  //set BP at each cycle
  if( g_suddenDeath.integer )
  {
    localHTP = level.suddenDeathHBuildPoints;
    localATP = level.suddenDeathABuildPoints;
  }
  else
  {
    localHTP = g_humanBuildPoints.integer;
    localATP = g_alienBuildPoints.integer;
  }

  level.humanBuildPoints = level.humanBuildPointsPowered = localHTP;
  level.alienBuildPoints = localATP;

  level.reactorPresent = qfalse;
  level.overmindPresent = qfalse;

  for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
  {
    if( !ent->inuse )
      continue;

    if( ent->s.eType != ET_BUILDABLE )
      continue;

    buildable = ent->s.modelindex;

    if( buildable != BA_NONE )
    {
      if( buildable == BA_H_REACTOR && ent->spawned && ent->health > 0 )
        level.reactorPresent = qtrue;

      if( buildable == BA_A_OVERMIND && ent->spawned && ent->health > 0 )
        level.overmindPresent = qtrue;

      if( !g_suddenDeath.integer || BG_FindReplaceableTestForBuildable( buildable ) )
      {
        if( BG_FindTeamForBuildable( buildable ) == BIT_HUMANS )
        {
          level.humanBuildPoints -= BG_FindBuildPointsForBuildable( buildable );
          if( ent->powered )
            level.humanBuildPointsPowered -= BG_FindBuildPointsForBuildable( buildable );
        }
        else
        {
          level.alienBuildPoints -= BG_FindBuildPointsForBuildable( buildable );
        }
      }
    }
  }

  if( level.humanBuildPoints < 0 )
  {
    localHTP -= level.humanBuildPoints;
    level.humanBuildPointsPowered -= level.humanBuildPoints;
    level.humanBuildPoints = 0;
  }

  if( level.alienBuildPoints < 0 )
  {
    localATP -= level.alienBuildPoints;
    level.alienBuildPoints = 0;
  }

  trap_SetConfigstring( CS_BUILDPOINTS, va( "%d %d %d %d %d",
        level.alienBuildPoints, localATP,
        level.humanBuildPoints, localHTP,
        level.humanBuildPointsPowered ) );

  //may as well pump the stages here too
  {
    float alienPlayerCountMod = level.averageNumAlienClients / PLAYER_COUNT_MOD;
    float humanPlayerCountMod = level.averageNumHumanClients / PLAYER_COUNT_MOD;
    int   alienNextStageThreshold, humanNextStageThreshold;

    if( alienPlayerCountMod < 0.1f )
      alienPlayerCountMod = 0.1f;

    if( humanPlayerCountMod < 0.1f )
      humanPlayerCountMod = 0.1f;

    if( g_alienStage.integer == S1 && g_alienMaxStage.integer > S1 )
      alienNextStageThreshold = (int)( ceil( (float)g_alienStage2Threshold.integer * alienPlayerCountMod ) );
    else if( g_alienStage.integer == S2 && g_alienMaxStage.integer > S2 )
      alienNextStageThreshold = (int)( ceil( (float)g_alienStage3Threshold.integer * alienPlayerCountMod ) );
    else
      alienNextStageThreshold = -1;

    if( g_humanStage.integer == S1 && g_humanMaxStage.integer > S1 )
      humanNextStageThreshold = (int)( ceil( (float)g_humanStage2Threshold.integer * humanPlayerCountMod ) );
    else if( g_humanStage.integer == S2 && g_humanMaxStage.integer > S2 )
      humanNextStageThreshold = (int)( ceil( (float)g_humanStage3Threshold.integer * humanPlayerCountMod ) );
    else
      humanNextStageThreshold = -1;

    trap_SetConfigstring( CS_STAGES, va( "%d %d %d %d %d %d",
          g_alienStage.integer, g_humanStage.integer,
          g_alienKills.integer, g_humanKills.integer,
          alienNextStageThreshold, humanNextStageThreshold ) );
  }
}

/*
============
G_CalculateStages
============
*/
void G_CalculateStages( void )
{
  float         alienPlayerCountMod     = level.averageNumAlienClients / PLAYER_COUNT_MOD;
  float         humanPlayerCountMod     = level.averageNumHumanClients / PLAYER_COUNT_MOD;
  static int    lastAlienStageModCount  = 1;
  static int    lastHumanStageModCount  = 1;
  static int    lastAlienStage  = -1;
  static int    lastHumanStage  = -1;

  if( alienPlayerCountMod < 0.1f )
    alienPlayerCountMod = 0.1f;

  if( humanPlayerCountMod < 0.1f )
    humanPlayerCountMod = 0.1f;

  if( g_alienKills.integer >=
      (int)( ceil( (float)g_alienStage2Threshold.integer * alienPlayerCountMod ) ) &&
      g_alienStage.integer == S1 && g_alienMaxStage.integer > S1 )
  {
    trap_Cvar_Set( "g_alienStage", va( "%d", S2 ) );
    level.alienStage2Time = level.time;
    lastAlienStageModCount = g_alienStage.modificationCount;
    G_LogPrintf("Stage: A 2: Aliens reached Stage 2\n");
  }

  if( g_alienKills.integer >=
      (int)( ceil( (float)g_alienStage3Threshold.integer * alienPlayerCountMod ) ) &&
      g_alienStage.integer == S2 && g_alienMaxStage.integer > S2 )
  {
    trap_Cvar_Set( "g_alienStage", va( "%d", S3 ) );
    level.alienStage3Time = level.time;
    lastAlienStageModCount = g_alienStage.modificationCount;
    G_LogPrintf("Stage: A 3: Aliens reached Stage 3\n");
  }

  if( g_humanKills.integer >=
      (int)( ceil( (float)g_humanStage2Threshold.integer * humanPlayerCountMod ) ) &&
      g_humanStage.integer == S1 && g_humanMaxStage.integer > S1 )
  {
    trap_Cvar_Set( "g_humanStage", va( "%d", S2 ) );
    level.humanStage2Time = level.time;
    lastHumanStageModCount = g_humanStage.modificationCount;
    G_LogPrintf("Stage: H 2: Humans reached Stage 2\n");
  }

  if( g_humanKills.integer >=
      (int)( ceil( (float)g_humanStage3Threshold.integer * humanPlayerCountMod ) ) &&
      g_humanStage.integer == S2 && g_humanMaxStage.integer > S2 )
  {
    trap_Cvar_Set( "g_humanStage", va( "%d", S3 ) );
    level.humanStage3Time = level.time;
    G_LogPrintf("Stage: H 3: Humans reached Stage 3\n");
    lastHumanStageModCount = g_humanStage.modificationCount;
  }
 
  if( g_alienStage.modificationCount > lastAlienStageModCount )
  {
    G_Checktrigger_stages( PTE_ALIENS, g_alienStage.integer );
      if( g_alienStage.integer == S2 )
      level.alienStage2Time = level.time;
    else if( g_alienStage.integer == S3 )
      level.alienStage3Time = level.time;
 
    lastAlienStageModCount = g_alienStage.modificationCount;
  }
 
  if( g_humanStage.modificationCount > lastHumanStageModCount )
  {
    G_Checktrigger_stages( PTE_HUMANS, g_humanStage.integer );

    if( g_humanStage.integer == S2 )
      level.humanStage2Time = level.time;
    else if( g_humanStage.integer == S3 )
      level.humanStage3Time = level.time;

    lastHumanStageModCount = g_humanStage.modificationCount;
  }

  if ( level.demoState == DS_RECORDING &&
       ( trap_Cvar_VariableIntegerValue( "g_alienStage" ) != lastAlienStage ||
         trap_Cvar_VariableIntegerValue( "g_humanStage" ) != lastHumanStage ) )
  {
    lastAlienStage = trap_Cvar_VariableIntegerValue( "g_alienStage" );
    lastHumanStage = trap_Cvar_VariableIntegerValue( "g_humanStage" );
    G_DemoCommand( DC_SET_STAGE, va( "%d %d", lastHumanStage, lastAlienStage ) );
  }
}
/*
============
CalculateWaves

Calculates which wave we should be in for bot survival
============
*/

void G_CalculateWaves( void ) {
    if(g_bot_survival.integer == 1) {
        if(g_bot_wave_interval.integer * 1000 + level.lastWaveTime < level.time && level.currentWave < 8) {
            level.currentWave++;
            level.lastWaveTime = level.time;
            switch(level.currentWave) {
                case 1: trap_Cvar_Set("g_bot_evolve","1");
                        trap_Cvar_Set("g_bot_buy","1");
                        trap_Cvar_Set("g_bot_dretch","1");
                        
                        break;
                case 2: trap_Cvar_Set("g_bot_level1","1");
                        trap_Cvar_Set("g_bot_rifle","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^32"));
                        break;
                case 3:
                        trap_Cvar_Set("g_bot_level1upg","1");
                        trap_Cvar_Set("g_bot_lgun","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^33"));
                        break;
                case 4:
                        trap_Cvar_Set("g_bot_level2","1");
                        trap_Cvar_Set("g_bot_mdriver","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^34"));
                        break;
                case 5:
                        trap_Cvar_Set("g_bot_level2upg","1");
                        trap_Cvar_Set("g_bot_chaingun","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^35"));
                        break;
                case 6:
                        trap_Cvar_Set("g_bot_level3","1");
                        trap_Cvar_Set("g_bot_prifle","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^36"));
                        break;
                case 7: trap_Cvar_Set("g_bot_level3upg","1");
                        trap_Cvar_Set("g_bot_flamer","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^37"));
                        break;
                case 8: trap_Cvar_Set("g_bot_level4","1");
                        trap_Cvar_Set("g_bot_lcannon","1");
                        trap_SendConsoleCommand( EXEC_APPEND, va("!cp %s\n", "Wave ^38"));
                        break;
            }
        }
    }
}
    
/*
============
CalculateAvgPlayers

Calculates the average number of players playing this game
============
*/
void G_CalculateAvgPlayers( void )
{
  //there are no clients or only spectators connected, so
  //reset the number of samples in order to avoid the situation
  //where the average tends to 0
  if( !level.numAlienClients )
  {
    level.numAlienSamples = 0;
    trap_Cvar_Set( "g_alienKills", "0" );
  }

  if( !level.numHumanClients )
  {
    level.numHumanSamples = 0;
    trap_Cvar_Set( "g_humanKills", "0" );
  }

  //calculate average number of clients for stats
  level.averageNumAlienClients =
    ( ( level.averageNumAlienClients * level.numAlienSamples )
      + level.numAlienClients ) /
    (float)( level.numAlienSamples + 1 );
  level.numAlienSamples++;

  level.averageNumHumanClients =
    ( ( level.averageNumHumanClients * level.numHumanSamples )
      + level.numHumanClients ) /
    (float)( level.numHumanSamples + 1 );
  level.numHumanSamples++;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void )
{
  int       i;
  char      P[ MAX_CLIENTS + 1 ] = {""};
  int       ff = 0;

  level.numConnectedClients = 0;
  level.numNonSpectatorClients = 0;
  level.numPlayingClients = 0;
  level.numVotingClients = 0;   // don't count bots
  level.numAlienClients = 0;
  level.numHumanClients = 0;
  level.numLiveAlienClients = 0;
  level.numLiveHumanClients = 0;

  for( i = 0; i < level.maxclients; i++ )
  {
    P[ i ] = '-';
    if ( level.clients[ i ].pers.connected != CON_DISCONNECTED || level.clients[ i ].pers.demoClient  )
    {
      level.sortedClients[ level.numConnectedClients ] = i;
      level.numConnectedClients++;
      P[ i ] = (char)'0' + level.clients[ i ].pers.teamSelection;

      if( level.clients[ i ].pers.connected != CON_CONNECTED || level.clients[ i ].pers.demoClient  )
        continue;

      level.numVotingClients++;
      if( level.clients[ i ].pers.teamSelection != PTE_NONE )
      {
        level.numPlayingClients++;
        if( level.clients[ i ].sess.sessionTeam != TEAM_SPECTATOR )
          level.numNonSpectatorClients++;

        if( level.clients[ i ].pers.teamSelection == PTE_ALIENS )
        {
          level.numAlienClients++;
          if( level.clients[ i ].sess.sessionTeam != TEAM_SPECTATOR )
            level.numLiveAlienClients++;
        }
        else if( level.clients[ i ].pers.teamSelection == PTE_HUMANS )
        {
          level.numHumanClients++;
          if( level.clients[ i ].sess.sessionTeam != TEAM_SPECTATOR )
            level.numLiveHumanClients++;
        }
      }
    }
  }
  level.numteamVotingClients[ 0 ] = level.numHumanClients;
  level.numteamVotingClients[ 1 ] = level.numAlienClients;
  P[ i ] = '\0';
  trap_Cvar_Set( "P", P );

  if( g_friendlyFire.value>0 )
    ff |= ( FFF_HUMANS | FFF_ALIENS );
  if( g_friendlyFireHumans.value>0  )
    ff |=  FFF_HUMANS;
  if( g_friendlyFireAliens.value>0  )
    ff |=  FFF_ALIENS;
  if( g_friendlyBuildableFire.value>0  )
    ff |=  FFF_BUILDABLES;
  trap_Cvar_Set( "ff", va( "%i", ff ) );

  qsort( level.sortedClients, level.numConnectedClients,
    sizeof( level.sortedClients[ 0 ] ), SortRanks );

  // see if it is time to end the level
  CheckExitRules( );

  // if we are at the intermission, send the new info to everyone
  if( level.intermissiontime )
    SendScoreboardMessageToAllClients( );
}

/*
============
G_DemoCommand

Store a demo command to a demo if we are recording
============
*/
void G_DemoCommand( demoCommand_t cmd, const char *string )
{
  if( level.demoState == DS_RECORDING )
    trap_DemoCommand( cmd, string );
}

/*
============
G_DemoSetClient

Mark a client as a demo client and load info into it
============
*/
void G_DemoSetClient( void )
{
  char buffer[ MAX_INFO_STRING ];
  gclient_t *client;
  char *s;

  trap_Argv( 0, buffer, sizeof( buffer ) );
  client = level.clients + atoi( buffer );
  client->pers.demoClient = qtrue;

  trap_Argv( 1, buffer, sizeof( buffer ) );
  s = Info_ValueForKey( buffer, "name" );
  if( *s )
    Q_strncpyz( client->pers.netname, s, sizeof( client->pers.netname ) );
  s = Info_ValueForKey( buffer, "ip" );
  if( *s )
    Q_strncpyz( client->pers.ip, s, sizeof( client->pers.ip ) );
  s = Info_ValueForKey( buffer, "team" );
  if( *s )
    client->pers.teamSelection = atoi( s );
  client->sess.spectatorState = SPECTATOR_NOT;
}

/*
============
G_DemoRemoveClient

Unmark a client as a demo client
============
*/
void G_DemoRemoveClient( void )
{
  char buffer[ 3 ];
  gclient_t *client;

  trap_Argv( 0, buffer, sizeof( buffer ) );
  client = level.clients + atoi( buffer );
  client->pers.demoClient = qfalse;
}

/*
============
G_DemoSetStage

Set the stages in a demo
============
*/
void G_DemoSetStage( void )
{
  char buffer[ 2 ];

  trap_Argv( 0, buffer, sizeof( buffer ) );
  trap_Cvar_Set( "g_humanStage", buffer );
  trap_Argv( 1, buffer, sizeof( buffer ) );
  trap_Cvar_Set( "g_alienStage", buffer );
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void )
{
  int   i;

  for( i = 0; i < level.maxclients; i++ )
  {
    if( level.clients[ i ].pers.connected == CON_CONNECTED )
      ScoreboardMessage( g_entities + i );
  }
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent )
{
  // take out of follow mode if needed
  if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
    G_StopFollowing( ent );

  // move to the spot
  VectorCopy( level.intermission_origin, ent->s.origin );
  VectorCopy( level.intermission_origin, ent->client->ps.origin );
  VectorCopy( level.intermission_angle, ent->client->ps.viewangles );
  ent->client->ps.pm_type = PM_INTERMISSION;

  // clean up powerup info
  memset( ent->client->ps.powerups, 0, sizeof( ent->client->ps.powerups ) );

  ent->client->ps.eFlags = 0;
  ent->s.eFlags = 0;
  ent->s.eType = ET_GENERAL;
  ent->s.modelindex = 0;
  ent->s.loopSound = 0;
  ent->s.event = 0;
  ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void )
{
  gentity_t *ent, *target;
  vec3_t    dir;

  // find the intermission spot
  ent = G_Find( NULL, FOFS( classname ), "info_player_intermission" );

  if( !ent )
  { // the map creator forgot to put in an intermission point...
    G_SelectSpawnPoint( vec3_origin, level.intermission_origin, level.intermission_angle );
  }
  else
  {
    VectorCopy( ent->s.origin, level.intermission_origin );
    VectorCopy( ent->s.angles, level.intermission_angle );
    // if it has a target, look towards it
    if( ent->target )
    {
      target = G_PickTarget( ent->target );

      if( target )
      {
        VectorSubtract( target->s.origin, level.intermission_origin, dir );
        vectoangles( dir, level.intermission_angle );
      }
    }
  }

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void )
{
  int     i;
  gentity_t *client;

  if( level.intermissiontime )
    return;   // already active

  level.numTeamWarnings = 99;
  
  level.intermissiontime = level.time;

  G_ClearVotes( );

  FindIntermissionPoint( );

  // move all clients to the intermission point
  for( i = 0; i < level.maxclients; i++ )
  {
    client = g_entities + i;

    if( !client->inuse )
      continue;

    // respawn if dead
    if( client->health <= 0 )
      respawn(client);

    MoveClientToIntermission( client );
  }

  // send the current scoring to all clients
  SendScoreboardMessageToAllClients( );
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either moved
to a new map based on the map rotation or the current map restarted
=============
*/
void ExitLevel( void )
{
  int       i;
  gclient_t *cl;
  buildHistory_t *tmp, *mark;

  while( ( tmp = level.buildHistory ) )
  {
    level.buildHistory = level.buildHistory->next;
    while( ( mark = tmp ) )
    {
      tmp = tmp->marked;
      G_Free( mark );
    }
  }

  if ( G_MapExists( g_nextMap.string ) )
    trap_SendConsoleCommand( EXEC_APPEND, va("!map %s\n", g_nextMap.string ) );
  else if( G_MapRotationActive( ) )
    G_AdvanceMapRotation( );
  else
    trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n" );

  trap_Cvar_Set( "g_nextMap", "" );

  level.restarted = qtrue;
  level.changemap = NULL;
  level.intermissiontime = 0;

  // reset all the scores so we don't enter the intermission again
  for( i = 0; i < g_maxclients.integer; i++ )
  {
    cl = level.clients + i;
    if( cl->pers.connected != CON_CONNECTED )
      continue;

    cl->ps.persistant[ PERS_SCORE ] = 0;
  }

  // we need to do this here before chaning to CON_CONNECTING
  G_WriteSessionData( );

  // change all client states to connecting, so the early players into the
  // next level will know the others aren't done reconnecting
  for( i = 0; i < g_maxclients.integer; i++ )
  {
    if( level.clients[ i ].pers.connected == CON_CONNECTED )
      level.clients[ i ].pers.connected = CON_CONNECTING;
  }

}
/*
=================
G_AdminsPrintf

Print to all active admins, and the logfile with a time stamp if it is open, and to the console
=================
*/
void QDECL G_AdminsPrintf( const char *fmt, ... )
{
  va_list argptr;
  char    string[ 1024 ];
  gentity_t   *tempent;
  int j;

  va_start( argptr, fmt );
  vsprintf( string, fmt,argptr );
  va_end( argptr );

  for( j = 0; j < level.maxclients; j++ )
  {
    tempent = &g_entities[ j ];
    if( G_admin_permission( tempent, ADMF_ADMINCHAT) ) 
    {
       trap_SendServerCommand(tempent-g_entities,va( "print \"^6[Admins]^7 %s\"", string) ); 
    }
  }
  
  G_LogPrintf("%s",string);

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... )
{
  va_list argptr;
  char    string[ 1024 ], decoloured[ 1024 ];
  int     min, tens, sec;

  sec = ( level.time - level.startTime ) / 1000;

  min = sec / 60;
  sec -= min * 60;
  tens = sec / 10;
  sec -= tens * 10;

  Com_sprintf( string, sizeof( string ), "%3i:%i%i ", min, tens, sec );

  va_start( argptr, fmt );
  vsprintf( string +7 , fmt,argptr );
  va_end( argptr );

  if( g_dedicated.integer )
    G_Printf( "%s", string + 7 );

  if( !level.logFile )
    return;

  if( g_decolourLogfiles.integer )
  {
    G_DecolorString( string, decoloured );
    trap_FS_Write( decoloured, strlen( decoloured ), level.logFile );
  }
  else
  {
    trap_FS_Write( string, strlen( string ), level.logFile );
  }
}

/*
=================
G_LogPrintfColoured

Bypasses g_decolourLogfiles for events that need colors in the logs
=================
*/
void QDECL G_LogPrintfColoured( const char *fmt, ... )
{
  va_list argptr;
  char    string[ 1024 ];
  int     min, tens, sec;

  sec = (level.time - level.startTime) / 1000;

  min = sec / 60;
  sec -= min * 60;
  tens = sec / 10;
  sec -= tens * 10;

  Com_sprintf( string, sizeof( string ), "%3i:%i%i ", min, tens, sec );

  va_start( argptr, fmt );
  vsprintf( string +7 , fmt,argptr );
  va_end( argptr );

  if( g_dedicated.integer )
    G_Printf( "%s", string + 7 );

  if( !level.logFile )
    return;

  trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
=================
G_LogOnlyPrintf

Print to the logfile only (not console) with a time stamp if it is open
=================
*/
void QDECL G_LogOnlyPrintf( const char *fmt, ... )
{
  va_list argptr;
  char    string[ 1024 ], decoloured[ 1024 ];
  int     min, tens, sec;

  sec = (level.time - level.startTime) / 1000;

  min = sec / 60;
  sec -= min * 60;
  tens = sec / 10;
  sec -= tens * 10;

  Com_sprintf( string, sizeof( string ), "%3i:%i%i ", min, tens, sec );

  va_start( argptr, fmt );
  vsprintf( string +7 , fmt,argptr );
  va_end( argptr );

  if( !level.logFile )
    return;

  if( g_decolourLogfiles.integer )
  {
    G_DecolorString( string, decoloured );
    trap_FS_Write( decoloured, strlen( decoloured ), level.logFile );
  }
  else
  {
    trap_FS_Write( string, strlen( string ), level.logFile );
  }
}

/*
=================
G_SendGameStat
=================
*/
void G_SendGameStat( pTeam_t team )
{
  char      map[ MAX_STRING_CHARS ];
  char      teamChar;
  char      data[ BIG_INFO_STRING ];
  char      entry[ MAX_STRING_CHARS ];
  int       i, dataLength, entryLength;
  gclient_t *cl;

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );

  switch( team )
  {
    case PTE_ALIENS:  teamChar = 'A'; break;
    case PTE_HUMANS:  teamChar = 'H'; break;
    case PTE_NONE:    teamChar = 'L'; break;
    default: return;
  }

  Com_sprintf( data, BIG_INFO_STRING,
      "%s %s T:%c A:%f H:%f M:%s D:%d SD:%d AS:%d AS2T:%d AS3T:%d HS:%d HS2T:%d HS3T:%d CL:%d",
      Q3_VERSION,
      g_tag.string,
      teamChar,
      level.averageNumAlienClients,
      level.averageNumHumanClients,
      map,
      level.time - level.startTime,
      G_TimeTilSuddenDeath( ),
      g_alienStage.integer,
      level.alienStage2Time - level.startTime,
      level.alienStage3Time - level.startTime,
      g_humanStage.integer,
      level.humanStage2Time - level.startTime,
      level.humanStage3Time - level.startTime,
      level.numConnectedClients );

  dataLength = strlen( data );

  for( i = 0; i < level.numConnectedClients; i++ )
  {
    int ping;

    cl = &level.clients[ level.sortedClients[ i ] ];

    if( cl->pers.connected == CON_CONNECTING )
      ping = -1;
    else
      ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

    switch( cl->ps.stats[ STAT_PTEAM ] )
    {
      case PTE_ALIENS:  teamChar = 'A'; break;
      case PTE_HUMANS:  teamChar = 'H'; break;
      case PTE_NONE:    teamChar = 'S'; break;
      default: return;
    }

    Com_sprintf( entry, MAX_STRING_CHARS,
      " \"%s\" %c %d %d %d",
      cl->pers.netname,
      teamChar,
      cl->ps.persistant[ PERS_SCORE ],
      ping,
      ( level.time - cl->pers.enterTime ) / 60000 );

    entryLength = strlen( entry );

    if( dataLength + entryLength >= BIG_INFO_STRING )
      break;

    strcpy( data + dataLength, entry );
    dataLength += entryLength;
  }

  trap_SendGameStat( data );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string )
{
  int         i, numSorted;
  gclient_t   *cl;
  gentity_t   *ent;

  G_LogPrintf( "Exit: %s\n", string );

  level.intermissionQueued = level.time;

  // this will keep the clients from playing any voice sounds
  // that will get cut off when the queued intermission starts
  trap_SetConfigstring( CS_INTERMISSION, "1" );

  // don't send more than 32 scores (FIXME?)
  numSorted = level.numConnectedClients;
  if( numSorted > 32 )
    numSorted = 32;

  for( i = 0; i < numSorted; i++ )
  {
    int   ping;

    cl = &level.clients[ level.sortedClients[ i ] ];

    if( cl->ps.stats[ STAT_PTEAM ] == PTE_NONE )
      continue;

    if( cl->pers.connected == CON_CONNECTING )
      continue;

    ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

    G_LogPrintf( "score: %i  ping: %i  client: %i %s\n",
      cl->ps.persistant[ PERS_SCORE ], ping, level.sortedClients[ i ],
      cl->pers.netname );

  }

  for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
  {
    if( !ent->inuse )
      continue;

    if( !Q_stricmp( ent->classname, "trigger_win" ) )
    {
      if( level.lastWin == ent->stageTeam )
        ent->use( ent, ent, ent );
    }
  }

  G_SendGameStat( level.lastWin );
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void )
{
  int       ready, notReady, numPlayers;
  int       i;
  gclient_t *cl;
  int       readyMask;

  //if no clients are connected, just exit
  if( !level.numConnectedClients )
  {
    ExitLevel( );
    return;
  }

  // see which players are ready
  ready = 0;
  notReady = 0;
  readyMask = 0;
  numPlayers = 0;
  for( i = 0; i < g_maxclients.integer; i++ )
  {
    cl = level.clients + i;
    if( cl->pers.connected != CON_CONNECTED )
      continue;

    if( cl->ps.stats[ STAT_PTEAM ] == PTE_NONE )
      continue;

    if( cl->readyToExit )
    {
      ready++;
      if( i < 16 )
        readyMask |= 1 << i;
    }
    else
      notReady++;

    numPlayers++;
  }

  trap_SetConfigstring( CS_CLIENTS_READY, va( "%d", readyMask ) );

  // never exit in less than five seconds
  if( level.time < level.intermissiontime + 5000 )
    return;

  // never let intermission go on for over 1 minute
  if( level.time > level.intermissiontime + 60000 )
  {
    ExitLevel( );
    return;
  }

  // if nobody wants to go, clear timer
  if( !ready && numPlayers )
  {
    level.readyToExit = qfalse;
    return;
  }

  // if everyone wants to go, go now
  if( !notReady )
  {
    ExitLevel( );
    return;
  }

  // the first person to ready starts the thirty second timeout
  if( !level.readyToExit )
  {
    level.readyToExit = qtrue;
    level.exitTime = level.time;
  }

  // if we have waited thirty seconds since at least one player
  // wanted to exit, go ahead
  if( level.time < level.exitTime + 30000 )
    return;

  ExitLevel( );
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void )
{
  int   a, b;

  if( level.numPlayingClients < 2 )
    return qfalse;

  a = level.clients[ level.sortedClients[ 0 ] ].ps.persistant[ PERS_SCORE ];
  b = level.clients[ level.sortedClients[ 1 ] ].ps.persistant[ PERS_SCORE ];

  return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void )
{
  // don't exit in demos
  if( level.demoState == DS_PLAYBACK )
    return;
  // if at the intermission, wait for all non-bots to
  // signal ready, then go to next level
  if( level.intermissiontime )
  {
    CheckIntermissionExit( );
    return;
  }

  if( level.intermissionQueued )
  {
    if( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME )
    {
      level.intermissionQueued = 0;
      BeginIntermission( );
    }

    return;
  }

  if( g_timelimit.integer )
  {
    if( level.time - level.startTime >= g_timelimit.integer * 60000 )
    {
      level.lastWin = PTE_NONE;
      trap_SendServerCommand( -1, "print \"Timelimit hit\n\"" );
      trap_SetConfigstring( CS_WINNER, "Stalemate" );
      LogExit( "Timelimit hit." );
      G_admin_maplog_result( "t" );
      return;
    }
    else if( level.time - level.startTime >= ( g_timelimit.integer - 5 ) * 60000 &&
          level.timelimitWarning < TW_IMMINENT )
    {
      trap_SendServerCommand( -1, "cp \"5 minutes remaining!\"" );
      level.timelimitWarning = TW_IMMINENT;
    }
    else if( level.time - level.startTime >= ( g_timelimit.integer - 1 ) * 60000 &&
          level.timelimitWarning < TW_PASSED )
    {
      trap_SendServerCommand( -1, "cp \"1 minute remaining!\"" );
      level.timelimitWarning = TW_PASSED;
    }
  }

  if( level.uncondHumanWin ||
      ( ( level.time > level.startTime + 1000 ) &&
        ( level.numAlienSpawns == 0 ) &&
        ( level.numLiveAlienClients == 0 ) ) )
  {
    //humans win
    level.lastWin = PTE_HUMANS;
    trap_SendServerCommand( -1, "print \"Humans win\n\"");
    trap_SetConfigstring( CS_WINNER, "Humans Win" );
    LogExit( "Humans win." );
    G_admin_maplog_result( "h" );
  }
  else if( level.uncondAlienWin ||
           ( ( level.time > level.startTime + 1000 ) &&
             ( level.numHumanSpawns == 0 ) &&
             ( level.numLiveHumanClients == 0 ) ) )
  {
    //aliens win
    level.lastWin = PTE_ALIENS;
    trap_SendServerCommand( -1, "print \"Aliens win\n\"");
    trap_SetConfigstring( CS_WINNER, "Aliens Win" );
    LogExit( "Aliens win." );
    G_admin_maplog_result( "a" );
  }
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
==================
CheckVote
==================
*/
void CheckVote( void )
{
  int votePassThreshold=level.votePassThreshold;
  int voteYesPercent;

  if( level.voteExecuteTime && level.voteExecuteTime < level.time )
  {
    level.voteExecuteTime = 0;

    if( !Q_stricmp( level.voteString, "map_restart" ) )
    {
      G_admin_maplog_result( "r" );
    }
    else if( !Q_stricmpn( level.voteString, "map", 3 ) )
    {
      G_admin_maplog_result( "m" );
    }


    if( !Q_stricmp( level.voteString, "suddendeath" ) )
    {
      level.suddenDeathBeginTime = level.time + ( 1000 * g_suddenDeathVoteDelay.integer ) - level.startTime;

      level.voteString[0] = '\0';

      if( g_suddenDeathVoteDelay.integer )
        trap_SendServerCommand( -1, va("cp \"Sudden Death will begin in %d seconds\n\"", g_suddenDeathVoteDelay.integer  ) );
    }

    if( level.voteString[0] )
      trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );

    if( !Q_stricmp( level.voteString, "map_restart" ) ||
        !Q_stricmpn( level.voteString, "map", 3 ) )
    {
      level.restarted = qtrue;
    }
  }

  if( !level.voteTime )
    return;

  if( level.voteYes + level.voteNo > 0 )
    voteYesPercent = (int)( 100 * ( level.voteYes ) / ( level.voteYes + level.voteNo ) );
  else
    voteYesPercent = 0; 
  
  if( ( level.time - level.voteTime >= VOTE_TIME ) || 
      ( level.voteYes + level.voteNo == level.numConnectedClients ) )
  {
    if( voteYesPercent> votePassThreshold || level.voteNo == 0 )
    {
      // execute the command, then remove the vote
      trap_SendServerCommand( -1, va("print \"Vote passed (%d - %d)\n\"", 
            level.voteYes, level.voteNo ) );
      G_LogPrintf( "Vote: Vote passed (%d-%d)\n", level.voteYes, level.voteNo );
      level.voteExecuteTime = level.time + 3000;
    }
    else
    {
      // same behavior as a timeout
      trap_SendServerCommand( -1, va("print \"Vote failed (%d - %d)\n\"",
            level.voteYes, level.voteNo ) );
      G_LogPrintf( "Vote: Vote failed (%d - %d)\n", level.voteYes, level.voteNo );
    }
  }
  else
  {
    if( level.voteYes > (int)( (double) level.numConnectedClients * 
                                 ( (double) votePassThreshold/100.0 ) ) )
    {
      // execute the command, then remove the vote
      trap_SendServerCommand( -1, va("print \"Vote passed (%d - %d)\n\"",
            level.voteYes, level.voteNo ) );
      G_LogPrintf( "Vote: Vote passed (%d - %d)\n", level.voteYes, level.voteNo );
      level.voteExecuteTime = level.time + 3000;
    }
    else if( level.voteNo > (int)( (double) level.numConnectedClients * 
                                     ( (double) ( 100.0-votePassThreshold )/ 100.0 ) ) )
    {
      // same behavior as a timeout
      trap_SendServerCommand( -1, va("print \"Vote failed (%d - %d)\n\"",
            level.voteYes, level.voteNo ) );
      G_LogPrintf("Vote failed\n");
    }
    else
    {
      // still waiting for a majority
      return;
    }
  }

  level.voteTime = 0;
  trap_SetConfigstring( CS_VOTE_TIME, "" );
  trap_SetConfigstring( CS_VOTE_STRING, "" );
}


/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team )
{
  int cs_offset;

  if ( team == PTE_HUMANS )
    cs_offset = 0;
  else if ( team == PTE_ALIENS )
    cs_offset = 1;
  else
    return;

  if( !level.teamVoteTime[ cs_offset ] )
    return;

  if( level.time - level.teamVoteTime[ cs_offset ] >= VOTE_TIME )
  {
    if( level.teamVoteYes[ cs_offset ] > level.teamVoteNo[ cs_offset ] && level.teamVoteYes[ cs_offset ] >= 2 )
    {
      // execute the command, then remove the vote
      trap_SendServerCommand( -1, va("print \"Team vote passed  (%d - %d)\n\"", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] ) );
      trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.teamVoteString[ cs_offset ] ) );
    }
    else
    {
      trap_SendServerCommand( -1, va("print \"Team vote failed  (%d - %d)\n\"", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] ) );
      G_LogPrintf( "Teamvote: Team vote failed (%d - %d)\n", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] );
    }
  }
  else
  {
    if( level.teamVoteYes[ cs_offset ] > level.numteamVotingClients[ cs_offset ] / 2 )
    {
      // execute the command, then remove the vote
      trap_SendServerCommand( -1, va("print \"Team vote passed  (%d - %d)\n\"", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] ) );
      G_LogPrintf( "Teamvote: Team vote passed (%d - %d)\n", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] );
      //
      trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.teamVoteString[ cs_offset ] ) );
    }
    else if( level.teamVoteNo[ cs_offset ] >= level.numteamVotingClients[ cs_offset ] / 2 )
    {
      // same behavior as a timeout
      trap_SendServerCommand( -1, va("print \"Team vote failed  (%d - %d)\n\"", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] ) );
      G_LogPrintf( "Teamvote: Team vote failed (%d - %d)\n", level.teamVoteYes[ cs_offset ], level.teamVoteNo[ cs_offset ] );
    }
    else
    {
      // still waiting for a majority
      return;
    }
  }

  level.teamVoteTime[ cs_offset ] = 0;
  trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );
  trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, "" );
}

/*
==================
CheckMsgTimer
==================
*/
void CheckMsgTimer( void )
{
  if( !g_msgTime.integer )
    return;

  if( level.time - level.lastMsgTime < abs( g_msgTime.integer ) * 60000 )
    return;

  // negative settings only print once per map
  if( ( level.lastMsgTime ) && g_msgTime.integer < 0 )
    return;

  level.lastMsgTime = level.time;

  if( g_msg.string[0] )
  {
    char buffer[ MAX_STRING_CHARS ];

    Q_strncpyz( buffer, g_msg.string, sizeof( buffer ) );
    G_ParseEscapedString( buffer );
    trap_SendServerCommand( -1, va( "cp \"%s\"", buffer ) );
    trap_SendServerCommand( -1, va( "print \"%s\n\"", buffer ) );
  }
}

/*
==================
CheckCountdown
==================
*/
void CheckCountdown( void )
{
  static int lastmsg = 0;
  int timeleft = g_warmup.integer - ( level.time - level.startTime ) / 1000;

  if( !g_doWarmup.integer || timeleft < 0 )
    return;

  if( level.time - lastmsg < 1000 )
    return;

  lastmsg = level.time;
  if( timeleft > 0 )
    trap_SendServerCommand( -1, va( "cp \"^1Warmup Time:^7\n^%i----- ^7%i ^%i-----\"", timeleft % 7, timeleft, timeleft % 7 ) );
  else if( timeleft == 0 ) 
    trap_SendServerCommand( -1, "cp \"^2----- GO! -----^7\"" );
}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void )
{
  static int lastPasswordModCount   = -1;
  static int lastMarkDeconModCount  = -1;
  static int lastSDTimeModCount = -1;

  if( g_password.modificationCount != lastPasswordModCount )
  {
    lastPasswordModCount = g_password.modificationCount;

    if( *g_password.string && Q_stricmp( g_password.string, "none" ) )
      trap_Cvar_Set( "g_needpass", "1" );
    else
      trap_Cvar_Set( "g_needpass", "0" );
  }

  // Unmark any structures for deconstruction when
  // the server setting is changed
  if( g_markDeconstruct.modificationCount != lastMarkDeconModCount )
  {
    int       i;
    gentity_t *ent;

    lastMarkDeconModCount = g_markDeconstruct.modificationCount;

    for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
    {
      if( !ent->inuse )
        continue;

      if( ent->s.eType != ET_BUILDABLE )
        continue;

      ent->deconstruct = qfalse;
    }
  }

  if( g_suddenDeathTime.modificationCount != lastSDTimeModCount )
  {
    lastSDTimeModCount = g_suddenDeathTime.modificationCount;
    level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
  }

  level.frameMsec = trap_Milliseconds( );
}

/*
==================
CheckDemo
==================
*/
void CheckDemo( void )
{
  int i;

  // Don't do anything if no change
  if ( g_demoState.integer == level.demoState )
    return;

  // log all connected clients
  if ( g_demoState.integer == DS_RECORDING )
  {
    char buffer[ MAX_INFO_STRING ] = "";
    for ( i = 0; i < level.maxclients; i++ )
    {
      if ( level.clients[ i ].pers.connected == CON_CONNECTED )
      {
        Info_SetValueForKey( buffer, "name", level.clients[ i ].pers.netname );
        Info_SetValueForKey( buffer, "ip", level.clients[ i ].pers.ip );
        Info_SetValueForKey( buffer, "team", va( "%d", level.clients[ i ].pers.teamSelection ) );
        G_DemoCommand( DC_CLIENT_SET, va( "%d %s", i, buffer ) );
      }
    }
  }

  // empty teams and display a message, also cancel all teamvotes
  else if ( g_demoState.integer == DS_PLAYBACK )
  {
    trap_SendServerCommand( -1, "print \"A demo has been started on the server.\n\"" );
    level.teamVoteNo[ 0 ] = level.numConnectedClients;
    level.teamVoteYes[ 0 ] = 0;
    CheckTeamVote( PTE_HUMANS );
    level.teamVoteNo[ 1 ] = level.numConnectedClients;
    level.teamVoteYes[ 1 ] = 0;
    CheckTeamVote( PTE_ALIENS );
    for ( i = 0; i < level.maxclients; i++ )
    {
      if ( level.clients[ i ].pers.teamSelection != PTE_NONE )
        G_ChangeTeam( g_entities + i, PTE_NONE );
    }
  }
  level.demoState = g_demoState.integer;
}
/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink( gentity_t *ent )
{
  float thinktime;

  thinktime = ent->nextthink;
  if( thinktime <= 0 )
    return;

  if( thinktime > level.time )
    return;

  ent->nextthink = 0;
  if( !ent->think )
    G_Error( "NULL ent->think" );

  ent->think( ent );
}

/*
=============
G_EvaluateAcceleration

Calculates the acceleration for an entity
=============
*/
void G_EvaluateAcceleration( gentity_t *ent, int msec )
{
  vec3_t  deltaVelocity;
  vec3_t  deltaAccel;

  VectorSubtract( ent->s.pos.trDelta, ent->oldVelocity, deltaVelocity );
  VectorScale( deltaVelocity, 1.0f / (float)msec, ent->acceleration );

  VectorSubtract( ent->acceleration, ent->oldAccel, deltaAccel );
  VectorScale( deltaAccel, 1.0f / (float)msec, ent->jerk );

  VectorCopy( ent->s.pos.trDelta, ent->oldVelocity );
  VectorCopy( ent->acceleration, ent->oldAccel );
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime )
{
  int       i;
  gentity_t *ent;
  int       msec;
  int       start, end;

  // if we are waiting for the level to restart, do nothing
  if( level.restarted )
    return;
  
  if( level.paused ) 
  {
    level.pausedTime = levelTime - level.time;
    if( ( level.pausedTime % 3000 ) == 0) 
      trap_SendServerCommand( -1, "cp \"The game has been paused. Please wait.\"" );
    
   for(i=0;i<level.maxclients;i++)
   {
     level.clients[ i ].ps.commandTime = levelTime;
   }

   return;
  }

  CheckMsgTimer( );
  CheckCountdown( );
  
  level.framenum++;
  level.previousTime = level.time;
  level.time = levelTime;
  msec = level.time - level.previousTime;

  //TA: seed the rng
  srand( level.framenum );

  // get any cvar changes
  G_UpdateCvars( );
  // check demo state
  CheckDemo( );

  //
  // go through all allocated objects
  //
  start = trap_Milliseconds( );
  ent = &g_entities[ 0 ];

  for( i = 0; i < ( level.demoState == DS_PLAYBACK ? g_maxclients.integer : level.num_entities ); i++, ent++ )
  {
    if( !ent->inuse )
      continue;

    // clear events that are too old
    if( level.time - ent->eventTime > EVENT_VALID_MSEC )
    {
      if( ent->s.event )
      {
        ent->s.event = 0; // &= EV_EVENT_BITS;
        if ( ent->client )
        {
          ent->client->ps.externalEvent = 0;
          //ent->client->ps.events[0] = 0;
          //ent->client->ps.events[1] = 0;
        }
      }

      if( ent->freeAfterEvent )
      {
        // tempEntities or dropped items completely go away after their event
        G_FreeEntity( ent );
        continue;
      }
      else if( ent->unlinkAfterEvent )
      {
        // items that will respawn will hide themselves after their pickup event
        ent->unlinkAfterEvent = qfalse;
        trap_UnlinkEntity( ent );
      }
    }

    // temporary entities don't think
    if( ent->freeAfterEvent )
      continue;

    //TA: calculate the acceleration of this entity
    if( ent->evaluateAcceleration )
      G_EvaluateAcceleration( ent, msec );

    if( !ent->r.linked && ent->neverFree )
      continue;

    if( ent->s.eType == ET_MISSILE )
    {
      G_RunMissile( ent );
      continue;
    }

    if( ent->s.eType == ET_BUILDABLE )
    {
      G_BuildableThink( ent, msec );
      continue;
    }

    if( ent->s.eType == ET_CORPSE || ent->physicsObject )
    {
      G_Physics( ent, msec );
      continue;
    }

    if( ent->s.eType == ET_MOVER )
    {
      G_RunMover( ent );
      continue;
    }

    if( i < MAX_CLIENTS )
    {
      G_RunClient( ent );
      continue;
    }

    G_RunThink( ent );
  }
  end = trap_Milliseconds();

  start = trap_Milliseconds();

  // perform final fixups on the players
  ent = &g_entities[ 0 ];

  for( i = 0; i < level.maxclients; i++, ent++ )
  {
    if( ent->inuse )
      ClientEndFrame( ent );
  }

  // save position information for all active clients 
  G_UnlaggedStore( );

  end = trap_Milliseconds();

  //TA:
  G_CountSpawns( );
  G_CalculateBuildPoints( );
  G_CalculateWaves( );
  G_CalculateStages( );
  G_SpawnClients( PTE_ALIENS );
  G_SpawnClients( PTE_HUMANS );
  G_CalculateAvgPlayers( );
  G_UpdateZaps( msec );

  // see if it is time to end the level
  CheckExitRules( );

  // update to team status?
  CheckTeamStatus( );

  // cancel vote if timed out
  CheckVote( );

  // check team votes
  CheckTeamVote( PTE_HUMANS );
  CheckTeamVote( PTE_ALIENS );

  // for tracking changes
  CheckCvars( );

  if( g_listEntity.integer )
  {
    for( i = 0; i < MAX_GENTITIES; i++ )
      G_Printf( "%4i: %s\n", i, g_entities[ i ].classname );

    trap_Cvar_Set( "g_listEntity", "0" );
  }
  
  level.pausedTime=0;
}

