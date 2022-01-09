
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#define Cvar_SetDescription Cvar_SetDescriptionByName

void Cvar_SetUserinfoDescriptions( void ) {
  Cvar_SetDescription( "name", "Player name\nDefault: Commander Keen" );
  Cvar_SetDescription( "rate", "modem speed/rate of data transfer\nDefault: 25000" );
  Cvar_SetDescription( "model", "display the name of current player model if no parameters are given\nDefault: sarge" );
  Cvar_SetDescription( "headmodel", "changes only the head of the model to another model\nDefault: sarge" );
  Cvar_SetDescription( "team_model", "set player model that will only be used during team game play\nDefault: sarge" );
  Cvar_SetDescription( "team_headmodel", "set head of team_model to a head that will only be used during team game play\nDefault: sarge" );
//	Cvar_SetDescription( "g_redTeam", "Stroggs", CVAR_SERVERINFO | CVAR_ARCHIVE);
//	Cvar_SetDescription( "g_blueTeam", "Pagans", CVAR_SERVERINFO | CVAR_ARCHIVE);
  Cvar_SetDescription( "color1", "spiral rail trail color spiral core\nDefault: 4" );
  Cvar_SetDescription( "color2", "spiral rail trail color spiral ring\nDefault: 5" );
  Cvar_SetDescription( "handicap", "set player handicap (max health), valid values 1 - 99\nDefault: 100" );
//	Cvar_SetDescription( "teamtask", "0" );
  Cvar_SetDescription( "sex", "Sex of model characteristics (sounds, obituary's, etc)\nDefault: male" );
  Cvar_SetDescription( "cl_anonymous", "possibly to toggle anonymous connection to a server\nDefault: 0" );
  Cvar_SetDescription( "password", "set password for entering a password protected server\nDefault: empty");
  Cvar_SetDescription( "cg_predictItems", "toggle client-side item prediction. 0 option to not do local prediction of item pickup\nDefault: 1" );
  Cvar_SetDescription( "cg_viewsize", "changes view port size 30 - 100\nDefault: 100" );
  Cvar_SetDescription( "cg_stereoSeparation", "the amount of stereo separation\nDefault: 0" );

}

void Cvar_SetClientDescriptions( void ) {
  Cvar_SetDescription( "con_notifytime", "Defines how long messages (from players or the system) are on the screen\nDefault: 3 seconds" );
  Cvar_SetDescription( "scr_conspeed", "Set how fast the console goes up and down\nDefault: 3 seconds" );
  Cvar_SetDescription( "cl_noprint", "Don't printout messages to your screen, only the console\nDefault: 0");
  Cvar_SetDescription( "cl_motd", "Show the message of the day from the server\nDefault: 1");
  Cvar_SetDescription( "cl_timeout", "Seconds to wait before client drops from the server after a timeout\nDefault: 10 seconds");
  Cvar_SetDescription( "cl_autoNudge", "Automatically set cl_timeNudge value based on connection stream\nDefault: 0");
  Cvar_SetDescription( "cl_timeNudge", "Effectively adds local lag to interpolate movement instead of skipping (try 100 for a really laggy server)\nDefault: 0");
  Cvar_SetDescription( "cl_shownet", "Display network quality info\nDefault: 0" );
	Cvar_SetDescription( "cl_showTimeDelta", "Display time delta between server updates\nDefault: 0" );
	Cvar_SetDescription( "rconPassword", "Set the rcon password when connecting to a passworded server\nDefault: empty" );
	Cvar_SetDescription( "activeAction", "Variable holds a command to be executed upon connecting to a server\nDefault: empty" );
  Cvar_SetDescription( "cl_autoRecordDemo", "Automatically start a demo recording when the game start\nDefault: 0" );
  Cvar_SetDescription( "cl_aviFrameRate", "Frame rate for AVI video capture\nDefault: 1000" );
  Cvar_SetDescription( "cl_aviMotionJpeg", "Use the motion JPEG format for AVI video capture\nDefault: 1" );
  Cvar_SetDescription( "cl_forceavidemo", "Force the use of AVI video format for demo capture\nDefault: 0" );
  Cvar_SetDescription( "cl_aviPipeFormat", "Extra flags send to the AVI encoding pipeline\nDefault: -preset medium -r:a ...");
	Cvar_SetDescription( "rconAddress", "Set the server address for rcon commands, rcon can be used without being connected to a game\nDefault: empty");
#ifdef USE_MASTER_LAN
	Cvar_SetDescription( "cl_master1", "Set the URL of a master server used in the Local LAN list, for nearby servers\nDefault: empty");
#endif
#ifdef USE_DRAGDROP
  Cvar_SetDescription( "cl_dropAction", "What to do when a file is dropped in to the client\n0 - just read it and list whats inside, 1 - copy the file to the homepath like downloaded files, and recommend the command to run, 2 - move the file to the home and automatically open it\nDefault: 1");
#endif
#ifdef __WASM__
	Cvar_SetDescription( "cl_returnURL", "Set the return URL to go to when the client disconnects from the server\nDefault: empty");
#endif
#ifdef USE_CVAR_UNCHEAT
  Cvar_SetDescription( "cl_uncheat", "Remove the CVAR_CHEAT flag from any cvar, shares this info with server for banning\nit also shares the cheat value so server administrators can see and log it\nDefault: cg_gun cg_gunX cg_gunY cg_gunZ");
#endif
  Cvar_SetDescription( "cl_allowDownload", "Toggle automatic downloading of maps, models, sounds, and textures\n1 - allow downloads\n2 - disallow redirects, must download from the same server\n4 - Disallow UDP downloads\n8 - don't disconnect clients while they are downloading\nDefault: 1");
  Cvar_SetDescription( "cl_mapAutoDownload", "Automatically download map files\nDefault: 0" );
#ifdef USE_CURL_DLOPEN
  Cvar_SetDescription( "cl_cURLLib", "Name of the cURL library to link\nDefault: libcurl");
#endif
  Cvar_SetDescription( "cl_conXOffset", "Offset the console message display\n0 - top lef\n999 - extreme top right\nDefault: 0");
  Cvar_SetDescription( "cl_conColor", "Set the console background color, instead of the default animated\nDefault: empty");
	Cvar_SetDescription( "cl_serverStatusResendTime", "The rate of the heartbeats to the master server, or check server status\nDefault: 750 seconds" );
  Cvar_SetDescription( "cg_autoswitch", "	auto-switch weapons (on pick-up)\nDefault: 1");
  Cvar_SetDescription( "cl_maxPing", "Set the maximum ping of servers to list\nDefault: 800" );
  Cvar_SetDescription( "cl_motdString", "Holds the message of the day variable from the server\nDefault: empty");
  Cvar_SetDescription( "cl_lanForcePackets", "Send packets over LAN every frame whether the client state changes or not\nDefault: 1" );
  Cvar_SetDescription( "cl_guidServerUniq", "Generate a unique GUID for every server, based on server ID and Q3 key (more secure)\nDefault: 1" );
#ifdef USE_LAZY_LOAD
  Cvar_SetDescription( "cl_lazyLoad", "Download graphics over the network after the level loads\n"
    "1 - Load available graphics immediately, and missing graphics as they become available\n2 - Don't load any graphics immediately\n4 - Only load graphics during downtimes, intermission, respawn timeout, while spectating\nDefault: 0" );
#endif
	Cvar_SetDescription( "cl_dlURL", "Set the download URL for the client in case it isn't set by the server\nDefault: http://ws.q3df.org/getpk3bymapname.php/%1");
	Cvar_SetDescription( "cl_dlDirectory", va( "Save downloads initiated by \\dlmap and \\download commands in:\n"
		" 0 - current game directory\n"
		" 1 - fs_basegame (%s) directory\n", FS_GetBaseGameDir() ) );
  Cvar_SetDescription( "cl_reconnectArgs", "Holds the previous connection so \\reconnect command can be used\nDefault: empty" );
  Cvar_SetDescription( "snaps", "set the number of snapshots sever will send to a client\nDefault: 40" );
  Cvar_SetDescription( "cl_guid", "Holds the guid from the client to identify the client\nDefault: empty" );
#ifdef USE_LNBITS
  Cvar_SetDescription( "cl_lnInvoice", "The previous LNBits invoice code sent by the server requesting payment\nDefault: empty");
#endif
  Cvar_SetDescription( "timegraph", "Display the time graph\nDefault: 0");
	Cvar_SetDescription( "debuggraph", "Display the debug graph\nDefault: 0");
  Cvar_SetDescription( "graphheight", "Set the height of the graph\nDefault: 32");
  Cvar_SetDescription( "graphscale", "Set the scale of the size\nDefault: 1");
  Cvar_SetDescription( "graphshift", "Set the shift of the graph\nDefault: 0");
  Cvar_SetUserinfoDescriptions();
}

void Cvar_SetSoundDescriptions( void ) {
  Cvar_SetDescription( "s_khz", "Set the sampling frequency of sounds\nlower=performance higher=quality\nDefault: 22" );
  Cvar_SetDescription( "s_mixahead", "Mix sounds together because they are used to reduce skipping\nDefault: 0.2 seconds");
  Cvar_SetDescription( "s_mixOffset", "Mix sounds ahead of time to prevent delays while loading\nDefault: 0.05");
  Cvar_SetDescription( "s_show", "Display filenames of sounds while they are being played\nDefault: 0");
  Cvar_SetDescription( "s_testsound", "Toggle a test tone to test sound system\nDefault: 0" );
#if defined(__linux__) && !defined(USE_SDL)
  Cvar_SetDescription( "s_device", "Set ALSA output device\n"
    " Use \"default\", \"sysdefault\", \"front\", etc.\n"
    " Enter " S_COLOR_CYAN "aplay -L "S_COLOR_WHITE"in your shell to see all options.\n"
    S_COLOR_YELLOW " Please note that only mono/stereo devices are acceptable.\n" );
#endif
  Cvar_SetDescription( "s_debug", "Show error and status messages from sound\nDefault: 0");
	Cvar_SetDescription( "s_volume", "Sound FX volume\nDefault: 0.8");
  Cvar_SetDescription( "s_musicVolume", "Music volume level\nDefault: 0.25");
  Cvar_SetDescription( "s_doppler", "How much the sound changes based on the speed the source is moving\nDefault: 1");
  Cvar_SetDescription( "s_muteWhenUnfocused", "Mute the sound when the window is in the background\nDefault: 1" );
  Cvar_SetDescription( "s_muteWhenMinimized", "Mute the sound when the window is minimized\nDefault: 1");
  Cvar_SetDescription( "s_initsound", "Use sounds, or disable them entirely\nDefault: 1");
  Cvar_SetDescription( "com_soundMegs", "The megabytes to allocate for sound can be adjusted to provide better performance on systems with more than 64mb of memory\nDefault: 8");
  Cvar_SetDescription( "s_sdlBits", "Number of bits to use for SDL sound" );
  Cvar_SetDescription( "s_sdlChannels", "Number of channels to use for SDL sound" );
  Cvar_SetDescription( "s_sdlDevSamps", "Number of samples to use for SDL sound development" );
  Cvar_SetDescription( "s_sdlMixSamps", "Number of samples to use for SDL sound" );

}

void Cvar_SetCommonDescriptions( void ) {
  Cvar_SetDescription( "//trap_GetValue", "Entry point for VM cvars to shortcut API for better speed");
  Cvar_SetDescription( "sys_cpustring", "Hold the CPU descriptor\nDefault: detect" );
  Cvar_SetDescription( "cm_noAreas", "Create one giant area for the clipmap and don't use culling\nDefault: 0");
  Cvar_SetDescription( "cm_noCurves", "Exclude curves from clipmap, make all vertices triangular\nDefault: 0");
  Cvar_SetDescription( "cm_playerCurveClip", "Don't clip player bounding box around curves\nDefault: 1" );
  Cvar_SetDescription( "cm_saveEnts", "Export entities from the next map that is loaded by the same name with a .ent extension, usually in your fs_homepath/maps directory\nDefault: 0");
  Cvar_SetDescription( "cl_execTimeout", "Minimum milliseconds between executions to reset overflow detection\nDefault: 2000");
	Cvar_SetDescription( "cl_execOverflow", "Maximum milliseconds an execution can take before it becomes overflowed\nDefault: 200" );
  Cvar_SetDescription( "com_zoneMegs", "Set zoneMegs on the command line, the maximum size for each zone\nDefault: 25");
  Cvar_SetDescription( "com_hunkMegs", "The size of the hunk memory segment" );
#if (defined(_WIN32) && defined(_DEBUG)) || (defined(__WASM__) && defined(_DEBUG))
  Cvar_SetDescription( "com_noErrorInterrupt", "No interrupt with a debug break when an error occurs\nDefault: 0");
#endif
  Cvar_SetDescription( "cl_developer", "Show client debug messages\nDefault: 0" );
	Cvar_SetDescription( "sv_developer", "Show server debug messages\nDefault: 0" );
  Cvar_SetDescription( "bot_developer", "Show bot debug messages\nDefault: 0" );
  Cvar_SetDescription( "r_developer", "Show renderer debug messages\nDefault: 0" );
  Cvar_SetDescription( "cg_developer", "Show cgame debug messages\nDefault: 0" );
  Cvar_SetDescription( "ui_developer", "Show UI debug messages\nDefault: 0" );
  Cvar_SetDescription( "g_developer", "Show game debug messages\nDefault: 0" );
  Cvar_SetDescription( "net_developer", "Show net debug messages\nDefault: 0" );
  Cvar_SetDescription( "s_developer", "Show sound debug messages\nDefault: 0" );
  Cvar_SetDescription( "fs_developer", "Show file debug messages\nDefault: 0" );
  Cvar_SetDescription( "developer", "Set developer mode that includes extra logging information\nDefault: 0");
  Cvar_SetDescription( "vm_rtChecks", "Runtime checks in compiled vm code, bitmask:\n 1 - program stack overflow\n" \
		" 2 - opcode stack overflow\n 4 - jump target range\n 8 - data read/write range" );
	Cvar_SetDescription( "journal", "Use a detailed journal.dat file for many events\nDefault: 0");
  Cvar_SetDescription( "protocol", "Override the protocol indication sent to the server\nDefault: " XSTRING(PROTOCOL_VERSION));
  Cvar_SetDescription( "dedicated", "Start a server in dedicated mode, no graphics or gameplay, only server process\n1 - Server is dedicated and unlisted\n2 - Server is public and updated in master list\nDefault: 1" );
  Cvar_SetDescription( "com_maxfps", "Set the max number of frames per second across the whole system, client and server\nDefault: 125");
	Cvar_SetDescription( "com_maxfpsUnfocused", "Set the max number of frames per second when the client is minimized or not in the background\nDefault: 60");
	Cvar_SetDescription( "com_yieldCPU", "Use CPU yield inbetween frames to allow other background processes to run (more efficient)\nDefault: 1");
#ifdef USE_AFFINITY_MASK
	Cvar_SetDescription( "com_affinityMask", "Set the processor affinity for multi-CPU systems\nDefault: 0");
#endif
  Cvar_SetDescription( "com_blood", "Toggle the blood mist effect in gib animations\nDefault: 1");
	Cvar_SetDescription( "logfile", "System console logging:\n"
		" 0 - disabled\n"
		" 1 - overwrite mode, buffered\n"
		" 2 - overwrite mode, synced\n"
		" 3 - append mode, buffered\n"
		" 4 - append mode, synced\n" );
  Cvar_SetDescription( "timescale", "Set the ratio between game time and real time\nDefault: 1");
	Cvar_SetDescription( "fixedtime", "Toggle the rendering of each frame completely, before sending the next frame\nDefault: 0");
  Cvar_SetDescription( "com_showtrace", "Toggle the display of packet tracing\nDefault: 0");
  Cvar_SetDescription( "viewlog", "Toggle the display of the startup console window over the game screen\nDefault: 0");
  Cvar_SetDescription( "com_speeds", "Toggle display of frame counter, all, sv, cl, gm\nDefault: 0");
  Cvar_SetDescription( "com_cameraMode", "Toggle the view of your player model off and on when in 3D camera view\nDefault: 0");
  Cvar_SetDescription( "timedemo", "Times a demo and returns frames per second like a benchmark\nDefault: 0");
	Cvar_SetDescription( "cl_paused", "Holds the status of the paused flag on the client side\nDefault: 0");
  Cvar_SetDescription( "cl_packetdelay", "Stream network packets to the server instead of trying to send immediately\nDefault: 0");
  Cvar_SetDescription( "cl_running", "Shows whether or not a client game is running or weather we are in server/client mode\nDefault: 0");
	Cvar_SetDescription( "sv_paused", "Holds the status of the paused flag on the server side\nDefault: 0");
  Cvar_SetDescription( "sv_packetdelay", "Stream network packets to the client instead of sending immediately\nDefault: 0");
  Cvar_SetDescription( "sv_running", "Variable flag tells the console weather or not a local server is running\nDefault: 0" );
  Cvar_SetDescription( "com_buildScript", "Set whether the game is being automated as a part of test build script\nDefault: 0" );
  Cvar_SetDescription( "com_errorMessage", "Holds the previous error message\nDefault: empty" );
	Cvar_SetDescription( "com_introPlayed", "Toggle displaying of intro cinematic once it has been seen this variable keeps it from playing each time\nDefault: 0" );
  Cvar_SetDescription( "com_skipIdLogo", "Toggle skip playing the intro logo cinematic for the game\nDefault: 0");
  Cvar_SetDescription( "version", "Set the engine verion so it can be distinguished from similar clients\nDefault: " XSTRING(Q3_VERSION));
	Cvar_SetDescription( "com_gamename", "Set the name of the game played, usually this is set by the mod.\nDefault: " XSTRING(GAMENAME_FOR_MASTER));
#ifndef DEDICATED
  Cvar_SetDescription( "vm_ui", "Attempt to load the UI QVM and compile it to native assembly code\n2 - compile VM\n1 - interpreted VM\n0 - native VM using dynamic linking\nDefault: 2");
	Cvar_SetDescription( "vm_cgame", "Attempt to load the CGame QVM and compile it to native assembly code\n2 - compile VM\n1 - interpreted VM\n0 - native VM using dynamic linking\nDefault: 2");
#endif
#ifndef BUILD_SLIM_CLIENT
	Cvar_SetDescription( "vm_game", "Attempt to load the Game QVM and compile it to native assembly code\n2 - compile VM\n1 - interpreted VM\n0 - native VM using dynamic linking\nDefault: 2");
#endif
  Cvar_SetDescription( "arch", "Architechure the game was compiled on\nDefault: " OS_STRING " " ARCH_STRING);
  Cvar_SetDescription( "ui_singlePlayerActive", "Holds a value indicating the server should run in single player mode" );
}

void Cvar_SetInputDescriptions( void ) {
  Cvar_SetDescription( "cl_nodelta", "Disable delta compression (slows net performance, not recommended)\nDefault: 0" );
  Cvar_SetDescription( "cl_debugMove", "Used for debugging movement, shown in debug graph\nDefault: 0" );
  Cvar_SetDescription( "cl_showSend", "Show network packets as they are sent\nDefault: 0" );
  Cvar_SetDescription( "cl_yawspeed", "Set the yaw rate when +left and/or +right are active\nDefault: 140" );
  Cvar_SetDescription( "cl_pitchspeed", "Set the pitch rate when +lookup and/or +lookdown are active\nDefault: 140" );
  Cvar_SetDescription( "cl_anglespeedkey", "Set the speed that the direction keys (not mouse) change the view angle\nDefault: 1.5" );
  Cvar_SetDescription( "cl_maxpackets", "Set the transmission packet size or how many packets are sent to client\nDefault: 60");
	Cvar_SetDescription( "cl_packetdup", "How many times should a packet try to resend to the server\nDefault: 1");
	Cvar_SetDescription( "cl_run", "Default to player running instead of walking\nDefault: 1");
	Cvar_SetDescription( "sensitivity", "Set how far your mouse moves in relation to travel on the mouse pad\nDefault: 5");
	Cvar_SetDescription( "cl_mouseAccel", "Toggle the use of mouse acceleration\nDefault: 0");
	Cvar_SetDescription( "cl_freelook", "Toggle the use of freelook with the mouse, looking up or down\nDefault: 1");
  Cvar_SetDescription( "cl_mouseAccelStyle", "Change the style of mouse acceleration in a given direction\n1 - the mouse speeds up\n2 - becomes more sensitive as it continues in one direction\nDefault 0");
	Cvar_SetDescription( "cl_mouseAccelOffset", "Mouse acceleration aplifier\nDefault: 0.001");
	Cvar_SetDescription( "cl_showMouseRate", "Show the mouse rate of mouse samples per frame\nDefault: 0");
	Cvar_SetDescription( "m_pitch", "Set the up and down movement distance of the player in relation to how much the mouse moves\nDefault: 0.022");
	Cvar_SetDescription( "m_yaw", "Set the speed at which the player's screen moves left and right while using the mouse\nDefault: 0.022");
	Cvar_SetDescription( "m_forward", "Set the up and down movement distance of the player in relation to how much the mouse moves\nDefault: 0.25");
  Cvar_SetDescription( "m_side", "Set the strafe movement distance of the player in relation to how much the mouse moves\nDefault: 0.25");
  Cvar_SetDescription( "m_filter", "Toggle use of mouse smoothing\nDefault: 1");
  Cvar_SetDescription( "m_filter", "Toggle use of mouse smoothing\nDefault: 0");
  Cvar_SetDescription( "in_keyboardDebug", "Show keyboard debug messages for every key press\nDefault: 0");
	Cvar_SetDescription( "in_mouse", "Toggle initialization of the mouse as an input device\nDefault: 1");
#ifdef USE_JOYSTICK
	Cvar_SetDescription( "in_joystick", "Toggle the initialization of the joystick\nDefault: 0" );
	Cvar_SetDescription( "in_joystickThreshold", "Set the maximum value of joystick in every direction\nDefault: 0.15" );
#endif
	Cvar_SetDescription( "cl_consoleKeys", "Set the characters that toggle the in game console\nDefault: ~ `");
  Cvar_SetDescription( "in_nograb", "Don't grab mouse when client in not in fullscreen mode\nDefault: 0");
  Cvar_SetDescription( "r_allowSoftwareGL", "Toggle the use of the default software OpenGL driver\nDefault: 0");
  Cvar_SetDescription( "r_swapInterval", "Toggle frame swapping\nDefault: 0" );
  Cvar_SetDescription( "r_stereoEnabled", "Enable stereo rendering for use with virtual reality headsets\nDefault: 0");
  Cvar_SetDescription( "in_mouse", "Mouse data input source:\n" \
		"  0 - disable mouse input\n" \
		"  1 - di/raw mouse\n" \
		" -1 - win32 mouse" );
#ifdef _WIN32
	Cvar_SetDescription( "in_lagged", "Mouse movement processing order:\n" \
		" 0 - before rendering\n" \
		" 1 - before framerate limiter" );
  Cvar_SetDescription( "s_driver", "Specify sound subsystem in win32 environment:\n"
		" dsound - DirectSound\n"
		" wasapi - WASAPI\n" );
#endif
}

void Cvar_SetFilesDescriptions( void ) {
  Cvar_SetDescription( "fs_debug", "Toggle filesystem debug logging, every file open is shown\nDefault: 0");
  Cvar_SetDescription( "fs_copyfiles", "Toggle if files can be copied after downloading from the server\nDefault: 0");
  Cvar_SetDescription( "fs_basepath", "Set the path of the engine where files can be downloaded\nDefault: varies by operating system");
  Cvar_SetDescription( "fs_basegame", "Set the base game for the engine, baseq3, baseoa, baseef\nDefault: " XSTRING(BASEGAME));
  Cvar_SetDescription( "fs_steampath", "The search path for Steam data when the engine is downloaded through Steam");
#if 0
  Cvar_SetDescription( "fs_locked", "Set file handle policy for pk3 files:\n"
    " 0 - release after use, unlimited number of pk3 files can be loaded\n"
    " 1 - keep file handle locked, more consistent, total pk3 files count limited to ~1k-4k\n" );
#endif
  Cvar_SetDescription( "fs_homepath", "Directory to store user configuration and downloaded files." );
	Cvar_SetDescription( "fs_game", "Set gamedir set the game folder/dir default is baseq3 in addition to the fs_basegame\nDefault: empty");
  Cvar_SetDescription( "fs_excludeReference", "Exclude specified pak files from download list on client side.\n"
		"Format is <moddir>/<pakname> (without .pk3 suffix), you may list multiple entries separated by space." );

}

void Cvar_SetNetDescriptions( void ) {
  Cvar_SetDescription( "showpackets", "Toggle display of all packets sent and received\nDefault: 0");
  Cvar_SetDescription( "showdrop", "Toggle display of dropped packets\nDefault: 0");
  Cvar_SetDescription( "net_qport", "Set internal network port. Use different ports when playing on a NAT network\nDefault: varies, usually 27960");
  Cvar_SetDescription( "net_enabled", "Networking options, bitmask:\n"
		" 1 - enable IPv4\n"
#ifdef USE_IPV6
		" 2 - enable IPv6\n"
		" 4 - prioritize IPv6 connections over IPv4\n"
		" 8 - disable IPv6 multicast"
#endif
		);
  Cvar_SetDescription( "net_ip", "The IP of the local machine, supplied by the OS\nDefault: 0.0.0.0 (bind all)");
  Cvar_SetDescription( "net_port", "Set port number server will use if you want to run more than one instance\nDefault: 27960");
  Cvar_SetDescription( "net_ip6", "The IPv6 of the local machine\nDefault: :: (bind all)");
  Cvar_SetDescription( "net_port6", "Set port number server will use if you want to run more than one instance\nDefault: 27960");
  Cvar_SetDescription( "net_mcast6addr", "The IPv6 multicast address\nDefault: " XSTRING(NET_MULTICAST_IP6));
  Cvar_SetDescription( "net_mcast6iface", "IPv6 multicast interface\nDefault: empty");
  Cvar_SetDescription( "net_socksEnabled", "Toggle the use of network socks 5 protocol enabling firewall access\nDefault: 0");
  Cvar_SetDescription( "net_socksServer", "Set the address (name or IP number) of the SOCKS server\nDefault: empty");
  Cvar_SetDescription( "net_socksPort", "Set proxy and/or firewall port default is 1080\nDefault: 1080");
  Cvar_SetDescription( "net_socksUsername", "Username for socks firewall supports no authentication and username/password authentication method\nDefault: empty");
  Cvar_SetDescription( "net_socksPassword", "Password for socks firewall access supports no authentication and username/password authentication method\nDefault: empty");
  Cvar_SetDescription( "net_dropsim", "Simulate packet dropping events for debugging purposes in percent\nDefault: empty");

}

void Cvar_SetRendererDescriptions( void ) {
  // also in client
  Cvar_SetDescription( "r_inGameVideo", "Controls whether in game video should be drawn\nDefault: 1" );
  Cvar_SetDescription( "r_stencilbits", "Stencil buffer size (0, 8bit, and 16bit)\nDefault: 8" );
  Cvar_SetDescription( "r_depthbits", "Set the number of depth bits\nDefault: 0");
  Cvar_SetDescription( "r_drawBuffer", "Set which frame buffer to draw into using framebuffers\nDefault: GL_BACK");
#ifdef USE_RENDERER_DLOPEN
  Cvar_SetDescription( "cl_renderer", "Set the name of the dynamically linked renderer\nDefault: opengl2");
#endif
  Cvar_SetDescription( "r_allowSoftwareGL", "Toggle the use of the default software OpenGL driver\nDefault: 0" );
  Cvar_SetDescription( "r_swapInterval", "Toggle frame swapping\nDefault: 0" );
  Cvar_SetDescription( "r_glDriver", "Used OpenGL driver by name\nDefault: opengl32" );
  Cvar_SetDescription( "r_displayRefresh", "Set the display refresh rate - not used\nDefault: 0 (set by display)" );
  Cvar_SetDescription( "vid_xpos", "Set the window x starting position on the screen\nDefault: 3" );
  Cvar_SetDescription( "vid_ypos", "Set the window y starting position on the screen\nDefault: 22" );
  Cvar_SetDescription( "r_noborder", "Set window borderless mode usually set by SDL and fullscreen mode\nDefault: 0" );
  Cvar_SetDescription( "r_mode", "Set video mode:\n -2 - use current desktop resolution\n -1 - use \\r_customWidth and \\r_customHeight\n  0..N - enter \\modelist for details" );
	Cvar_SetDescription( "r_modeFullscreen", "Dedicated fullscreen mode, set to \"\" to use \\r_mode in all cases" );
  Cvar_SetDescription( "r_fullscreen", "Set fullscreen mode on startup\nDefault: 1" );
  Cvar_SetDescription( "r_customPixelAspect", "Custom pixel aspect to use with \\r_mode -1\nDefault: 1" );
  Cvar_SetDescription( "r_customwidth", "Custom width to use with \\r_mode -1" );
	Cvar_SetDescription( "r_customheight", "Custom height to use with \\r_mode -1" );
  Cvar_SetDescription( "r_colorbits", "Set number of bits used for each color from 0 to 32 bit, usually set by SDL\nDefault: 0" );
  Cvar_SetDescription( "r_picmip", "Set texture quality, lower is better" );
#ifdef USE_VULKAN
  Cvar_SetDescription( "r_device", "Select physical device to render:\n" \
    " 0+ - use explicit device index\n" \
    " -1 - first discrete GPU\n" \
    " -2 - first integrated GPU" );
  Cvar_SetDescription( "r_renderScale", "Scaling mode to be used with custom render resolution:\n"
    " 0 - disabled\n"
    " 1 - nearest filtering, stretch to full size\n"
    " 2 - nearest filtering, preserve aspect ratio (black bars on sides)\n"
    " 3 - linear filtering, stretch to full size\n"
    " 4 - linear filtering, preserve aspect ratio (black bars on sides)\n" );
#endif
  Cvar_SetDescription( "r_picmip", "Set texture quality, lower is better" );
  // TODO: if renderer1
#if 0
  Cvar_SetDescription( "r_presentBits", "Select color bits used for presentation surfaces\nRequires " S_COLOR_CYAN "\\r_fbo 1" );
  Cvar_SetDescription( "r_dither", "Set dithering mode:\n 0 - disabled\n 1 - ordered\nRequires " S_COLOR_CYAN "\\r_fbo 1" );
	Cvar_SetDescription( "r_nomip", "Apply picmip only on worldspawn textures" );
#endif
  Cvar_SetDescription( "r_allowExtensions", "Use all of the OpenGL extensions the card is capable of\nDefault: 1");
	Cvar_SetDescription( "r_ext_compressed_textures", "Compress textures as they are loaded\nDefault: 1");
	Cvar_SetDescription( "r_ext_multitexture", "Enable multitexture, not used\nDefault: 1");
	Cvar_SetDescription( "r_ext_compiled_vertex_array", "Enable vertex arrays\nDefault: 1");
	Cvar_SetDescription( "r_ext_texture_env_add", "Enable texture environment\nDefault: 1");
  Cvar_SetDescription( "r_ext_framebuffer_object", "Enable framebuffer objects (FBOs) for buffered rendering\nDefault: 1");
  Cvar_SetDescription( "r_ext_texture_float", "Enable texture floats\nDefault: 1");
  Cvar_SetDescription( "r_ext_framebuffer_multisample", "Enable framebuffer multisampling\nDefault: 0");
  Cvar_SetDescription( "r_arb_seamless_cube_map", "Enable seamless cube mapping\nDefault: 0");
  Cvar_SetDescription( "r_arb_vertex_array_object", "Enable ARB vertex array objects\nDefault: 1");
  Cvar_SetDescription( "r_ext_direct_state_access", "Enable direct state access\nDefault: 1");
  Cvar_SetDescription( "r_ext_texture_filter_anisotropic", "Enable texture anisotropic filtering\nDefault: 0");
	Cvar_SetDescription( "r_ext_max_anisotropy", "Enable max anisotropy\nDefault: 2");
	Cvar_SetDescription( "r_picmip", "Set maximum texture size\n0 - best quality\n4 - fastest\nDefault: 1");
	Cvar_SetDescription( "r_roundImagesDown", "Set rounding down amount\nDefault: 1");
	Cvar_SetDescription( "r_colorMipLevels", "Enable texture visualizations for debugging\nDefault: 0");
  Cvar_SetDescription( "r_detailTextures", "Enable detailed textures\nDefault: 1");
  Cvar_SetDescription( "r_texturebits", "Set the number of bits used for a texture\nDefault: 0");
  Cvar_SetDescription( "r_stencilbits", "Set the number of stencil bits, 0, 8, 16\nDefault: 8");
  Cvar_SetDescription( "r_ext_multisample", "Enable frame multisampling, 0 - 4\nDefault: 0");
  Cvar_SetDescription( "r_overBrightBits", "Set intensity level of lights reflected from textures\nDefault: 1");
	Cvar_SetDescription( "r_ignorehwgamma", "Toggle the use of video driver gamma correction\nDefault: 0");
	Cvar_SetDescription( "r_simpleMipMaps", "Toggle the use of simple mip mapping for slower machines\nDefault: 1");
	Cvar_SetDescription( "r_vertexLight", "Enable vertex lighting (faster, lower quality than lightmap) removes lightmaps, forces every shader to only use a single rendering pass\nDefault: 0");
  Cvar_SetDescription( "r_subdivisions", "Set maximum level of detail\nDefault: 4");
  Cvar_SetDescription( "r_greyscale", "Enable grayscale effect where all images are converted to grayscale\nDefault: 0");
  Cvar_SetDescription( "r_externalGLSL", "Support loading GLSL files externally, Mods can supply their own rendering\nDefault: 0");
	Cvar_SetDescription( "r_hdr", "Enable High definition light maps\nDefault: 1");
	Cvar_SetDescription( "r_floatLightmap", "Allow lightmaps to use floats for alignment, not used\nDefault: 0");
	Cvar_SetDescription( "r_postProcess", "Enable post processing effects such as motion blur\nDefault: 1");
  Cvar_SetDescription( "r_toneMap", "Enable tone mapping, like in photographer, colors are evened out based on the generated scene, requires postprocessing\nDefault: 1");
  Cvar_SetDescription( "r_forceToneMap", "Force tone mapping on every frame using custom values\nDefault: 0");
  Cvar_SetDescription( "r_forceToneMapMin", "Set the tone map minimum darkness\nDefault: -8.0");
  Cvar_SetDescription( "r_forceToneMapAvg", "Set the tone map average brightness/darkness\nDefault: -2.0");
  Cvar_SetDescription( "r_forceToneMapMax", "Set the max tone map brightness\nDefault: 0.0");
	Cvar_SetDescription( "r_autoExposure", "Enable tone map auto exposure, just like in photography, affects how lighting is rendered\nDefault: 1");
  Cvar_SetDescription( "r_forceAutoExposure", "Force auto exposure values in post processing\nDefault: 0");
  Cvar_SetDescription( "r_forceAutoExposureMin", "Set the minimum for exposure\nDefault: -2.0");
  Cvar_SetDescription( "r_forceAutoExposureMax", "Set the maximum for exposure\nDefault: 2.0");
  Cvar_SetDescription( "r_cameraExposure", "Enable camera exposure when looking through portals\nDefault: 1");
	Cvar_SetDescription( "r_depthPrepass", "Enable prepass depth rendering\nDefault 1");
	Cvar_SetDescription( "r_ssao", "Enable screen space ambient occlusion for more realistic lighting\nDefault: 0");
	Cvar_SetDescription( "r_normalMapping", "Enable normal mapping for lighting\nDefault: 1");
	Cvar_SetDescription( "r_specularMapping", "Enable specular mapping for lighting\nDefault: 1");
  Cvar_SetDescription( "r_deluxeMapping", "Enable deluxe mapping for lighting\nDefault: 1");
  Cvar_SetDescription( "r_parallaxMapping", "Enable parallax mapping for lighting\nDefault: 0");
  Cvar_SetDescription( "r_parallaxMapShadows", "Enable parallax mapping for shadows\nDefault: 0");
  Cvar_SetDescription( "r_cubeMapping", "Enable cube mapping for lighting\nDefault: 0");
  Cvar_SetDescription( "r_cubemapSize", "Set the cube mapping size\nDefault: 128");
	Cvar_SetDescription( "r_deluxeSpecular", "Enable delux specular mapping for lighting\nDefault: 0.3");
	Cvar_SetDescription( "r_pbr", "Enable physics based renderering\nDefault: 0");
  Cvar_SetDescription( "r_baseNormalX", "lighting setting");
  Cvar_SetDescription( "r_baseNormalY", "lighting setting");
	Cvar_SetDescription( "r_baseParallax", "lighting setting");
  Cvar_SetDescription( "r_baseSpecular", "lighting setting");
  Cvar_SetDescription( "r_baseGloss", "lighting setting");
  Cvar_SetDescription( "r_glossType", "lighting setting");
  Cvar_SetDescription( "r_dlightMode", "lighting setting");
  Cvar_SetDescription( "r_pshadowDist", "lighting setting");
  Cvar_SetDescription( "r_mergeLightmaps", "lighting setting");
  Cvar_SetDescription( "r_imageUpsample", "lighting setting");
  Cvar_SetDescription( "r_imageUpsampleMaxSize", "lighting setting");
	Cvar_SetDescription( "r_imageUpsampleType", "lighting setting");
  Cvar_SetDescription( "r_genNormalMaps", "lighting setting");
  Cvar_SetDescription( "r_forceSun", "lighting setting");
	Cvar_SetDescription( "r_forceSunLightScale", "lighting setting");
	Cvar_SetDescription( "r_forceSunAmbientScale", "lighting setting");
	Cvar_SetDescription( "r_drawSunRays", "lighting setting");
  Cvar_SetDescription( "r_sunlightMode", "lighting setting");
  Cvar_SetDescription( "r_sunShadows", "lighting setting");
	Cvar_SetDescription( "r_shadowFilter", "lighting setting");
	Cvar_SetDescription( "r_shadowBlur", "lighting setting");
	Cvar_SetDescription( "r_shadowMapSize", "lighting setting");
  Cvar_SetDescription( "r_shadowCascadeZNear", "lighting setting");
  Cvar_SetDescription( "r_shadowCascadeZFar", "lighting setting");
  Cvar_SetDescription( "r_shadowCascadeZBias", "lighting setting");
  Cvar_SetDescription( "r_ignoreDstAlpha", "lighting setting");
  Cvar_SetDescription( "r_fullbright", "Toggle textures to full brightness level\nDefault: 0");
	Cvar_SetDescription( "r_mapOverBrightBits", "Set intensity level of lights reflected from textures\nDefault: 2");
	Cvar_SetDescription( "r_intensity", "Increase brightness of texture colors\nDefault: 1");
	Cvar_SetDescription( "r_singleShader", "Toggles use of 1 default shader for all objects\nDefault: 0");
  Cvar_SetDescription( "r_lodCurveError", "Level of detail error on curved surface grids." );
  Cvar_SetDescription( "r_lodCurveError", "Level of detail setting if set to 10000, don't drop curve rows for a long time\nDefault: 250");
  Cvar_SetDescription( "r_lodbias", "Change the geometric level of detail, 0 - high detail, 4 - low detail\nDefault: 0");
  Cvar_SetDescription( "r_lodCurveError", "Level of detail error on curved surface grids." );
  Cvar_SetDescription( "r_flares", "Toggle projectile flare and lighting effect\nDefault: 0");
  Cvar_SetDescription( "r_znear", "Set how close objects can be to the player before they're clipped out of the scene, so you can't see your nose or shoulders\nDefault: 0.001");
	Cvar_SetDescription( "r_zfar", "Set how far objects are before they are clipped out, usually automatically calculated based on map size, 0 - infinity, 2048 - used for menus\nDefault: 0 infinity" );
	Cvar_SetDescription( "r_zproj", "distance of observer camera to projection plane" );
	Cvar_SetDescription( "r_stereoSeparation", "Set the distance between stereo frame renders, as if your eyes are separated\nDefault: 64");
	Cvar_SetDescription( "r_ignoreGLErrors", "Ignores OpenGL errors that occur\nDefault: 1");
	Cvar_SetDescription( "r_fastsky", "Toggle fast rendering of sky if set to 1, also disables portals\nDefault: 0");
	Cvar_SetDescription( "r_drawSun", "Toggle rendering of sunlight in lighting effects\nDefault: 0");
  Cvar_SetDescription( "r_dynamiclight", "Toggle dynamic lighting, where all visuals are darker or brighter depending on lightmap\nDefault: 1");
  Cvar_SetDescription( "r_dlightBacks", "Brighter areas are changed more by dlights than dark areas\nDefault: 1");
  Cvar_SetDescription( "r_finish", "Toggle synchronization of rendered frames\nDefault: 0");
	Cvar_SetDescription( "r_textureMode", "Select texture mode\nDefault: GL_LINEAR_MIPMAP_NEAREST");
	Cvar_SetDescription( "r_gamma", "Set gamma correction, 0.5 - 3\nDefault: 1");
	Cvar_SetDescription( "r_facePlaneCull", "Toggle culling of brush faces not in view, 0 affects performance\nDefault: 1");
	Cvar_SetDescription( "r_railWidth", "Set width of the rail trail\nDefault: 16");
  Cvar_SetDescription( "r_railCoreWidth", "Set size of the rail trail's core\nDefault: 6");
  Cvar_SetDescription( "r_railSegmentLength", "Set distance between rail sun bursts\nDefault: 32");
  Cvar_SetDescription( "r_ambientScale", "Set the scale or intensity of ambient light\nDefault: 0.6");
	Cvar_SetDescription( "r_directedScale", "Set scale/intensity of light shinning directly upon objects\nDefault: 1");
  Cvar_SetDescription( "r_anaglyphMode", "Enable anaglyph mode when using the red and blue lense 3D glasses\nDefault: 0");
  Cvar_SetDescription( "r_showImages", "Toggle displaying a collage of all image files when set to 1\nDefault: 0");
	Cvar_SetDescription( "r_debugLight", "Toggle debugging of lighting effects\nDefault: 0");
  Cvar_SetDescription( "r_debugSort", "Toggle debugging of sorting of polygons for depth\nDefault: 0");
  Cvar_SetDescription( "r_printShaders", "Toggle the printing on console of the number of shaders\nDefault: 0");
  Cvar_SetDescription( "r_saveFontData", "Enable saving of font image data when they are loaded in game, for developers\nDefault: 0");
  Cvar_SetDescription( "r_nocurves", "Map diagnostic command toggle the use of curved geometry\nDefault: 0");
	Cvar_SetDescription( "r_drawworld", "Toggle rendering of map architecture\nDefault: 1");
  Cvar_SetDescription( "r_lightmap", "Toggle entire map to full brightness level all textures become blurred with light\nDefault: 0");
  Cvar_SetDescription( "r_portalOnly", "When set to 1 turns off stencil buffering for portals, this allows you to see the entire portal before it's clipped\nDefault: 0");
	Cvar_SetDescription( "r_flareSize", "Set the size of flares\nDefault: 40");
  Cvar_SetDescription( "r_flareFade", "Set scale of fading of flares in relation to distance\nDefault: 7");
  Cvar_SetDescription( "r_flareCoeff", "Set the flare coefficient\nDefault: " XSTRING(FLARE_STDCOEFF));
  Cvar_SetDescription( "r_skipBackEnd", "Toggle the skipping of the backend video buffer\nDefault: 0");
  Cvar_SetDescription( "r_measureOverdraw", "Measure overdraw, when the same pixel is written to more than once when rendering a scene\nDefault: 0");
	Cvar_SetDescription( "r_lodscale", "Set scale for level of detail adjustment\nDefault: 5");
	Cvar_SetDescription( "r_norefresh", "Toggle the refreshing of the rendered display, for debugging\nDefault: 0");
	Cvar_SetDescription( "r_drawentities", "Toggle display of brush entities\nDefault: 1");
  Cvar_SetDescription( "r_ignore", "Ignores hardware driver settings in favor of variable settings\nDefault: 1");
  Cvar_SetDescription( "r_nocull", "Toggle rendering of hidden objects\nDefault: 0");
  Cvar_SetDescription( "r_novis", "Disable VIS tables that hold information about which areas should be displayed, draw all polygons\nDefault: 0");
  Cvar_SetDescription( "r_showcluster", "Toggle the display of clusters by number as the player enters the area\nDefault: 0");
  Cvar_SetDescription( "r_speeds", "Show rendering info e.g. how many triangles are drawn added, timing info\nDefault: 0");
  Cvar_SetDescription( "r_debugSurface", "Enable drawing of surface debug information on the polygon surface\nDefault: 0");
  Cvar_SetDescription( "r_nobind", "Toggle the binding of textures to triangles\nDefault: 0");
  Cvar_SetDescription( "r_showtris", "Enable map diagnostics and show triangles around each polygon\nDefault: 0");
  Cvar_SetDescription( "r_showsky", "Enable rendering sky in front of other objects\nDefault: 0");
  Cvar_SetDescription( "r_shownormals", "Toggle the drawing of short lines indicating brush and entity polygon vertices, useful when debugging model lighting\nDefault: 0");
  Cvar_SetDescription( "r_clear", "Toggle the clearing of the screen between frames\nDefault: 0");
	Cvar_SetDescription( "r_offsetFactor", "Control the polygon offset factor, when you see lines appearing in decals, or they seem to flick on and off\nDefault: -1");
  Cvar_SetDescription( "r_offsetUnits", "Control the polygon offset units, when you see lines appearing in decals, or they seem to flick on and off\nDefault: -2");
  Cvar_SetDescription( "r_drawBuffer", "Set which frame buffer to draw into while simultaneously showing the GL_FRONT buffer\nDefault: GL_BACK");
  Cvar_SetDescription( "r_lockpvs", "Disable update to PVS table as player moves through map (new areas not rendered) for debugging\nDefault: 0");
  Cvar_SetDescription( "r_noportals", "Do not render portals\nDefault: 0");
  Cvar_SetDescription( "cg_shadows", "Render player shadows\nDefault: 1");
	Cvar_SetDescription( "r_marksOnTriangleMeshes", "Show marks on triangle meshes for debugging\nDefault: 0");
	Cvar_SetDescription( "r_aviMotionJpegQuality", "Sets the quality for the AVI video recording\nDefault: 90");
  Cvar_SetDescription( "r_screenshotJpegQuality", "Sets the quality for the JPEG video recording\nDefault: 90");
  Cvar_SetDescription( "r_maxpolys", "Max number of polygons\nDefault: " XSTRING(MAX_POLYS));
	Cvar_SetDescription( "r_maxpolyverts", "Max number of polygon vertices to display at a time\nDefault: " XSTRING(MAX_POLYVERTS));
  Cvar_SetDescription( "r_paletteMode", "Replace missing images with plain color maps, and import colors from pallete.shader file" );
  Cvar_SetDescription( "r_seeThroughWalls", "Make all shaders partially transparent" );
}

void Cvar_SetBotDescriptions( void ) {
  Cvar_SetDescription( "bot_enable", "Enable and disable adding of bots to the map/game\nDefault: 1");
  Cvar_SetDescription( "bot_developer", "Show bot debug messages\nDefault: 0" );
  Cvar_SetDescription( "bot_debug", "Show bot debug information\nDefault: 0" );
  Cvar_SetDescription( "bot_maxdebugpolys", "Maximum number of bot debug polys\nDefault: 2" );
  Cvar_SetDescription( "bot_groundonly", "Only show ground faces of areas\nDefault: 1" );
  Cvar_SetDescription( "bot_reachability", "show all reachabilities to other areas\nDefault: 0" );
	Cvar_SetDescription( "bot_visualizejumppads", "show jumppads\nDefault: 0" );
	Cvar_SetDescription( "bot_forceclustering", "force cluster calculations\nDefault: 0" );
	Cvar_SetDescription( "bot_forcereachability", "force reachability calculations\nDefault: 0" );
	Cvar_SetDescription( "bot_forcewrite", "force writing aas file\nDefault: 0" );
	Cvar_SetDescription( "bot_aasoptimize", "no aas file optimisation\nDefault: 0" );
	Cvar_SetDescription( "bot_saveroutingcache", "save routing cache\nDefault: 0" );
	Cvar_SetDescription( "bot_thinktime", "msec the bots thinks\nDefault: 100" );
	Cvar_SetDescription( "bot_reloadcharacters", "reload the bot characters each time\nDefault: 0" );
	Cvar_SetDescription( "bot_testichat", "test ichats\nDefault: 0" );
	Cvar_SetDescription( "bot_testrchat", "test rchats\nDefault: 0" );
	Cvar_SetDescription( "bot_testsolid", "test for solid areas\nDefault: 0" );
	Cvar_SetDescription( "bot_testclusters", "test the AAS clusters\nDefault: 0" );
	Cvar_SetDescription( "bot_fastchat", "fast chatting bots\nDefault: 0" );
	Cvar_SetDescription( "bot_nochat", "disable chats\nDefault: 0" );
	Cvar_SetDescription( "bot_pause", "pause the bots thinking\nDefault: 0" );
	Cvar_SetDescription( "bot_report", "get a full report in ctf\nDefault: 0" );
	Cvar_SetDescription( "bot_grapple", "enable grapple\nDefault: 0" );
	Cvar_SetDescription( "bot_rocketjump", "enable rocket jumping\nDefault: 1" );
	Cvar_SetDescription( "bot_challenge", "challenging bot\nDefault: 0" );
	Cvar_SetDescription( "bot_minplayers", "minimum players in a team or the game\nDefault: 0" );
	Cvar_SetDescription( "bot_interbreedchar", "bot character used for interbreeding\nDefault: " );
	Cvar_SetDescription( "bot_interbreedbots", "number of bots used for interbreeding\nDefault: 10" );
	Cvar_SetDescription( "bot_interbreedcycle", "bot interbreeding cycle\nDefault: 20" );
	Cvar_SetDescription( "bot_interbreedwrite", "write interbreeded bots to this file\nDefault: " );
}

void Cvar_SetServerDescriptions( void ) {
  Cvar_SetDescription( "dmflags", "set deathmatch flags\nDefault: 0" );
	Cvar_SetDescription( "fraglimit", "set fraglimit on a server\nDefault: 20" );
	Cvar_SetDescription( "timelimit", "amount of time before new map loads or next match begins, 0 - unlimited\nDefault: 0" );
  Cvar_SetDescription( "g_gametype", "Holds the game style for the current match included in server info\nDefault: 0");
	Cvar_SetDescription( "sv_keywords", "Holds the search string entered in the internet connection menu\nDefault: empty");
	Cvar_SetDescription( "protocol", "Holds the network protocol version\nDefault: " XSTRING(PROTOCOL_VERSION));
  Cvar_SetDescription( "mapname", "Holds the name of the current map\nDefault: nomap");
	Cvar_SetDescription( "sv_privateClients", "The number of spots, out of sv_maxclients, reserved for players with the server password\nDefault: 0");
	Cvar_SetDescription( "sv_hostname", "Set the name of the server\nDefault: noname");
	Cvar_SetDescription( "sv_maxclients", "Maximum number of people allowed to join the server\nDefault: 8");
  Cvar_SetDescription( "sv_maxclientsPerIP", "Limits number of simultaneous connections from the same IP address.\nDefault: 3" );
	Cvar_SetDescription( "sv_clientTLD", "Include client locations in status and demo recordings\nDefault: 0");
#ifdef USE_MV
	Cvar_SetDescription( "sv_mvAutoRecord", "Automatically record a multiview demo\nDefault: 0");
	Cvar_SetDescription( "sv_mvFlags", "Record scoring in the multiview demo\nDefault: 3");
  Cvar_SetDescription( "sv_mvClients", "Number of multiview clients allowed\nDefault: 8");
  Cvar_SetDescription( "sv_mvPassword", "Set the password for multiview clients\nDefault: empty");
  Cvar_SetDescription( "sv_mvFileCount", "Set the maximum number of multiview recordings before it reuses numeric names\nDefault: 1024");
	Cvar_SetDescription( "sv_mvFolderSize", "Set the multiview folder size for automatic recordings, rotate when maxed out\nDefault: 768");
	Cvar_SetDescription( "sv_mvWorld", "Micromanage the client view by sending world commands that update which camera view should be visible on screen. This gives servers/games/admins a more scripted control over client displays. Turn off to force clients to manage their own displays by using the `tile` or `popout` commands.\nDefault: 1");
	Cvar_SetDescription( "sv_mvSyncPS", "Synchronize player state across worlds. If a player picks up a weapon in one world, that weapon slot will be filled in every other world. Only works with the same mods.\nDefault: 0");
	Cvar_SetDescription( "sv_mvSyncXYZ", "Force players to occupy the same XYZ coordinates in every world. Useful in shadow/mirror dimension type games.\nDefault: 0");
	Cvar_SetDescription( "sv_mvSyncMove", "Sync movement across multiple worlds, copy movement from the active screen.\nDefault: 0");
	Cvar_SetDescription( "sv_mvOmnipresent", "Occupy an active player position in multiple worlds.\n 0 - disconnected except for one, 1 - active in all worlds automatically, 2 - spectator in other worlds, 3 - active in all worlds client has joined\nDefault: 0");
#endif
  Cvar_SetDescription( "sv_minRate", "Force clients to play with a minimum latency\nDefault: 0");
	Cvar_SetDescription( "sv_maxRate", "Force all clients to play with a max rate, limit an advantage for having a low latency\nDefault: 0");
  Cvar_SetDescription( "sv_dlRate", "Set the maximum rate for server downloads\nDefault: 100");
  Cvar_SetDescription( "sv_floodProtect", "Toggle server flood protection to keep players from bringing the server down\nDefault: 1" );
	Cvar_SetDescription( "sv_serverid", "Hold the server ID sent to clients");
  Cvar_SetDescription( "sv_pure", "Make sure clients load the same pak files as the server, disallow native VMs\nDefault: 1");
  Cvar_SetDescription( "sv_referencedPakNames", "Holds the names of paks referenced by the server for comparison client-side\nDefault: empty");
  Cvar_SetDescription( "sv_referencedPaks", "Holds the crc hash of paks referenced by the server for comparison client-side\nDefault: empty");
  Cvar_SetDescription( "sv_pakNames", "Holds the names of paks referenced by the server for transmission client-side\nDefault: empty");
  Cvar_SetDescription( "sv_paks", "Holds the crc hash of paks referenced by the server for transmission client-side\nDefault: empty");
#ifdef USE_SERVER_ROLES
	Cvar_SetDescription( "sv_roles", "Space seperated list of roles to configure\nDefault: referee moderator admin");
#endif
#ifdef USE_PERSIST_CLIENT
	Cvar_SetDescription( "sv_clSessions", "Save client state in sessions files. Specify the number of seconds the session can be restored. -1 for forever, 0 for off, 30 for 30 seconds if a client gets disconnected.\nDefault: 0");
#endif
#ifdef USE_RECENT_EVENTS
	Cvar_SetDescription( "sv_recentPassword", "Set the required to get a response of key events that happened on the server since the last check\nDefault: empty");
#endif
#ifdef DEDICATED
	Cvar_SetDescription( "rconPassword", "Set the rcon password required to send the server commands\nDefault: empty");
#endif
  Cvar_SetDescription( "sv_privatePassword", "Set password for private clients to login\nDefault: empty");
  Cvar_SetDescription( "sv_fps", "Set the max frames per second the server sends the client\nDefault: 20");
	Cvar_SetDescription( "sv_timeout", "Seconds without any message before automatic client disconnect" );
	Cvar_SetDescription( "sv_zombietime", "Seconds to sink messages after disconnect" );
  Cvar_SetDescription( "nextmap", "Holds the value for the script to execute at the end of a match\nDefault: empty" );
	Cvar_SetDescription( "sv_allowDownload", "Toggle the ability for clients to download files maps from server\nDefault: 1" );
	Cvar_SetDescription( "sv_dlURL", "Set the download URL for clients to download content\nDefault: empty");
  for(int i = 1; i < 25; i++)
    Cvar_SetDescription( va("sv_master%i", i), va("Set URL or address to master server %i\nDefault: empty", i) );
  Cvar_SetDescription( "sv_reconnectlimit", "Number of times a disconnected client can come back and reconnect\nDefault: 12");
  Cvar_SetDescription( "sv_padPackets", "Toggles the padding of network packets\nDefault: 0" );
  Cvar_SetDescription( "sv_killserver", "Set to a one the server goes down\nDefault: 0");
	Cvar_SetDescription( "sv_mapChecksum", "Allows clients to compare the map checksum\nDefault: empty");
  Cvar_SetDescription( "sv_lanForceRate", "Force clients to use the same packet rate as the server\nDefault: 1" );
#ifdef USE_CVAR_UNCHEAT
  Cvar_SetDescription( "sv_banCheats", "Ban specific cheat values allowed by cl_uncheat setting\ne.g. cg_fov used to see behind you\nDefault: empty");
	Cvar_SetDescription( "sv_banFile", "Set the file to store a cache of all the player bans\nDefault: serverbans.dat");
#endif
#ifdef USE_LNBITS
	Cvar_SetDescription( "sv_lnMatchPrice", "Set the LNBits price for the match, each player must pay this amount to enter\nDefault: 0 (free)");
  Cvar_SetDescription( "sv_lnMatchCut", "Set how much from each payment is withheld in the wallet, not included in the reward\nDefault: 0");
  Cvar_SetDescription( "sv_lnMatchReward", "Holds the match reward amount calculated during the match when all the players have paid and connected\nDefault: 0");
  Cvar_SetDescription( "sv_lnWallet", "The LNBits wallet key for deposits/payment\nDefault: emtpy");
  Cvar_SetDescription( "sv_lnKey", "The LNBits wallet key for withdrawals\nDefault: empty");
	Cvar_SetDescription( "sv_lnAPI", "The LNBits base API URL for other API calls\nDefault: https://lnbits.com/api/v1");
	Cvar_SetDescription( "sv_lnWithdraw", "The LNBits withdraw API URL\nDefault: https://lnbits.com/withdraw/api/v1");
	Cvar_SetDescription( "cl_dlDirectory", "Save downloads initiated by \\dlmap and \\download commands\nDefault: 0");
	Cvar_SetDescription( "cl_cURLLib", "Name of the cURL library to link\nDefault: libcurl");
#endif
  Cvar_SetDescription( "sv_demoState", "Hold the state of server-side demos\n"
		"0 - none, no demo started\n"
		"1 - await playback, playback is starting\n"
		"2 - playing a demo\n"
		"3 - waiting to stop\n"
		"4 - recording a demo\n"
		"Default: 0" );
  Cvar_SetDescription( "sv_democlients", "The number of demo clients allowed to connect while a demo is player\nDefault: 0" );
	Cvar_SetDescription( "sv_autoDemo", "Automatically record a server-side demo for the match\nDefault: 0" );
	Cvar_SetDescription( "sv_autoRecord", "Automatically record a client demo for every client in the match\nDefault: 0" );
	Cvar_SetDescription( "sv_autoRecordThreshold", "Automatically save a client demo recording if this threshold is met for their score\n0 - Save all clients\n0.9 - Save only clients with 90% of the points\nDefault: 0.9" );
	Cvar_SetDescription( "cl_freezeDemo", "Freeze the playing demo using this variable, just like in client demo mode\nDefault: 0" );
	Cvar_SetDescription( "sv_demoTolerant", "Tolerance mode for recorded serve-side demos, ignore when a message doesn't work\nDefault: 0" );
  Cvar_SetDescription( "sv_levelTimeReset", "Reset the clock in between matches\nDefault: 0");
  Cvar_SetDescription( "sv_filter", "Set the ban filter file\nDefault: filter.txt");
  Cvar_SetDescription( "sv_cheats", "enable cheating commands (give all)" );
}

#ifndef DEDICATED
void Cvar_SetKnownDescriptions(vmIndex_t index, recognizedVM_t knownVM) {
  if(index == VM_CGAME) {
    Cvar_SetDescription( "capturelimit", "set # of times a team must grab the others flag before the win is declared\nDefault: 8" );
  } else if (index == VM_UI) {
    Cvar_SetDescription( "cg_brassTime", "the amount of time it takes to eject brass from the machine gun\nDefault 2500");
    Cvar_SetDescription( "cg_drawCrosshair", "10 crosshairs to select from (cg_drawCrosshair 1 - 10)\nDefault: 4");
    Cvar_SetDescription( "cg_drawCrosshairNames", "toggle displaying of the name of the player you're aiming at\nDefault: 1");
    Cvar_SetDescription( "cg_marks", "toggle the marks the projectiles leave on the wall (bullet holes, etc)\nDefault: 1");
    Cvar_SetDescription( "cg_shadows", "set shadow detail level\nDefault: 1" );
    Cvar_SetDescription( "g_arenasFile", "sets the file name to use for map rotation and bot names and game type for each arena\nDefault: scripts/arenas.txt" );
    Cvar_SetDescription( "g_botsFile", "sets the file name to use for setting up the bots configuration and characters for each bot\nDefault: scripts/bots.txt" );
    Cvar_SetDescription( "g_spAwards", "variable holds the names of the award icons that have been earned in the tier levels in single player mode");
    Cvar_SetDescription( "g_spScores1", "holds your scores on skill level 1 in single player games" );
    Cvar_SetDescription( "g_spScores2", "holds your scores on skill level 2 in single player games" );
    Cvar_SetDescription( "g_spScores3", "holds your scores on skill level 3 in single player games" );
    Cvar_SetDescription( "g_spScores4", "holds your scores on skill level 4 in single player games" );
    Cvar_SetDescription( "g_spScores5", "holds your scores on skill level 5 in single player games" );
    Cvar_SetDescription( "g_spSkill", "holds your current skill level for single player 1 - I can win, 2 - bring it on, 3 - hurt me plenty 4 - hardcore and 5 - nightmare" );
    Cvar_SetDescription( "ui_spSelection", "Holds a value indicating the user selected single player skirmish menu" );
    Cvar_SetDescription( "g_spVideos", "	variable holds the names of the cinematic videos that are unlocked at the end of each tier completion" );
    for(int i = 1; i < 17; i++)
      Cvar_SetDescription( va("server%i", i), va("Favorited server %i", i) );
    Cvar_SetDescription( "ui_browserMaster", "Holds UI multiplayer browser master server being queried" );
    Cvar_SetDescription( "ui_browserGameType", "Holds UI multiplayer menu game type selected" );
    Cvar_SetDescription( "ui_browserSortKey", "Holds UI multiplayer sort direction" );
    Cvar_SetDescription( "ui_browserShowFull", "Holds UI multiplayer menu full setting" );
    Cvar_SetDescription( "ui_browserShowEmpty", "Holds UI multiplayer menu empty setting" );
    Cvar_SetDescription( "ui_cdkeychecked", "Holds UI value indicating if the user has been prompted for a CD key");
    Cvar_SetDescription( "ui_ffa_fraglimit", "Holds free-for-all fraglimit" );
    Cvar_SetDescription( "ui_ffa_timelimit", "Holds free-for-all timelimit" );
    Cvar_SetDescription( "ui_tourney_fraglimit", "Holds tournament fraglimit" );
    Cvar_SetDescription( "ui_tourney_timelimit", "Holds tournament timelimit" );
    Cvar_SetDescription( "ui_team_fraglimit", "Holds team-play fraglimit" );
    Cvar_SetDescription( "ui_team_timelimit", "Holds team-play timelimit" );
    Cvar_SetDescription( "ui_team_friendly", "Holds team-play friendly-fire setting" );
    Cvar_SetDescription( "ui_ctf_capturelimit", "Holds ctf capturelimit" );
    Cvar_SetDescription( "ui_ctf_timelimit", "Holds ctf timelimit" );
    Cvar_SetDescription( "ui_ctf_friendly", "Holds ctf friendly-fire setting" );

/*
    Cvar_SetDescription( "ui_new", "Holds value indicating the UI menu system was just loaded" );
    Cvar_SetDescription( "ui_debug", "Show UI debug messages" );
    Cvar_SetDescription( "ui_initialized", "Holds value indicating the UI is fully initialized" );
    Cvar_SetDescription( "ui_teamName", "Name of your team" );
    Cvar_SetDescription( "ui_opponentName", "Name of opponents team" );
    Cvar_SetDescription( "ui_redteam", "Name of red team" );
    Cvar_SetDescription( "ui_blueteam", "Name of blue team" );
    Cvar_SetDescription( "ui_dedicated", "Holds setting for starting a dedicated server" );
    Cvar_SetDescription( "ui_gametype", "Holds a value for the selected gametype" );
    Cvar_SetDescription( "ui_joinGametype", "Holds a value for the gametype joined from the multiplayer menu" );
    Cvar_SetDescription( "ui_netGametype", "Holds a value for the selected net gametype filter setting" );
    Cvar_SetDescription( "ui_actualNetGametype", "Holds a value for the selected gametype" );
    for(int i = 1; i < 6; i++)
      Cvar_SetDescription( va("ui_redteam%i", i), "Holds value for alternate team names" );
    for(int i = 1; i < 6; i++)
      Cvar_SetDescription( va("ui_blueteam%i", i), "Holds value for alternate team names" );
    Cvar_SetDescription( "ui_netSource", "Holds a value for the current multiplayer network screen" );
    Cvar_SetDescription( "ui_menuFiles", "Name of the file that list mod menu files\nDefault: ui/menus.txt" );
    Cvar_SetDescription( "ui_currentTier", "Holds a value indicating the highest single player tier reached" );
    Cvar_SetDescription( "ui_currentMap", "" );
    Cvar_SetDescription( "ui_currentNetMap", "" );
    Cvar_SetDescription( "ui_mapIndex", "" );
    Cvar_SetDescription( "ui_currentOpponent", "" );
    Cvar_SetDescription( "cg_selectedPlayer", "" );
    Cvar_SetDescription( "cg_selectedPlayerName", "" );
    for(int i = 0; i < 4; i++)
      Cvar_SetDescription( va("ui_lastServerRefresh_%i", i), "Holds the value from the last server retrieved from multiplayer refresh" );
    Cvar_SetDescription( "ui_scoreAccuracy", "Indicates how many Accuracy awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreImpressives", "Indicates how many Impressive awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreExcellents", "Indicates how many Excellent awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreCaptures", "Indicates how many Capture awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreDefends", "Indicates how many Defence awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreAssists", "Indicates how many Assist awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreGauntlets", "Indicates how many Gauntlet awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreScore", "Indicates the player score from the previous match" );
    Cvar_SetDescription( "ui_scorePerfect", "Indicates how many Perfect awards were given in the previous match" );
    Cvar_SetDescription( "ui_scoreTeam", "The total team score for the previous match" );
    Cvar_SetDescription( "ui_scoreBase", "The base time for scoring the previous match" );
    Cvar_SetDescription( "ui_scoreTime", "When the last match was scored" );
    Cvar_SetDescription( "ui_scoreTimeBonus", "The amount of bonus time overrun" );
    Cvar_SetDescription( "ui_scoreSkillBonus", "Skill bonus points" );
    Cvar_SetDescription( "ui_scoreShutoutBonus", "The number of shoutout bonus points" );
    Cvar_SetDescription( "ui_fragLimit", "The selected capture limit configuring a server\n10" );
    Cvar_SetDescription( "ui_captureLimit", "The selected capture limit configuring a server\nDefault: 5" );
    Cvar_SetDescription( "ui_smallFont", "The scaling for small fonts\nDefault: 0.25" );
    Cvar_SetDescription( "ui_bigFont", "The scaling for big fonts\nDefault: 0.4" );
    Cvar_SetDescription( "ui_findPlayer", "Search for this player model or something is wrong\nDefault: Sarge" );
    Cvar_SetDescription( "ui_q3model", "The default selected player model" );
    Cvar_SetDescription( "cg_hudFiles", "A list of files with extra HUD layout definitions\nDefault: ui/hud.txt" );
    Cvar_SetDescription( "ui_recordSPDemo", "Should record a demo file even in single player mode" );
    Cvar_SetDescription( "ui_teamArenaFirstRun", "Indicates if this is the first time Team Arena has been run" );
    Cvar_SetDescription( "g_warmup", "The warmup time before a match starts\nDefault: 20" );
    Cvar_SetDescription( "capturelimit", "The default capturelimit for CTF mode\nDefault: 8" );
    Cvar_SetDescription( "ui_serverStatusTimeOut", "How long the UI should wait for a response from a remote server\nDefault: 7000" );
*/
  }
}
#endif
