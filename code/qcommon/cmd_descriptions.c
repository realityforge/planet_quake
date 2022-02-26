
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"



void Cmd_SetClientDescriptions( void ) {

	Cmd_SetDescription("cmd", "Send a command to server remote console\nUsage: cmd <command>");
	Cmd_SetDescription("configstrings", "List the current config strings in effect\nUsage: configstrings");
	Cmd_SetDescription("clientinfo", "Display name, rate, number of snaps, player model, rail color, and handicap\nUsage: clientinfo");
	Cmd_SetDescription("snd_restart", "Reinitialize sound\nUsage: snd_restart");
	Cmd_SetDescription("vid_restart", "Reinitialize video\nUsage: vid_restart");
	Cmd_SetDescription("disconnect", "Disconnect from a server, including local\nUsage: disconnect");
	Cmd_SetDescription("record", "Record a demo\nUsage: record <demoname>");
	Cmd_SetDescription("demo", "Play a demo\nUsage: demo <demoname>");
	Cmd_SetDescription("demo_pause", "Pause the demo playing\nUsage: pause");
	Cmd_SetDescription("rewind", "Rewind demo playback\nUsage: rewind (10|<seconds>)");
	Cmd_SetDescription("forward", "Fast forward demo playback\nUsage: forward (10|<seconds>)");
	Cmd_SetDescription("demo_play", "Unpause and continue playing\nUsage: play");
	Cmd_SetDescription("cinematic", "Play a video or RoQ file\nUsage: cinematic <videofile>");
	Cmd_SetDescription("stoprecord", "Stop recording a demo\nUsage: stoprecord");
	Cmd_SetDescription("connect", "Connect to a server\nUsage: connect ([-4|-6]) <serveraddress>");
	Cmd_SetDescription("reconnect", "Reinitialize the connection to the last server you were connected to\nUsage: reconnect");
	Cmd_SetDescription("localservers", "List servers on LAN or local sub net only\nUsage: localservers");
	Cmd_SetDescription("globalservers", "List public servers on the internet\nUsage: globalservers");
	Cmd_SetDescription("rcon", "Start a remote console to a server\nUsage: rcon");
	Cmd_SetDescription( "ping", "Manually ping a server\nUsage: ping <serveraddress>");
	Cmd_SetDescription( "serverstatus", "Display the current status of the connected server as well as connected users and their slot number\nUsage: serverstatus (<serveraddress>)");
	Cmd_SetDescription("showip", "Display your current TCP/IP address\nUsage: showip");
	Cmd_SetDescription("model", "Display the name of current player model if no parameters are given\nUsage: model (<modelname>)");
	Cmd_SetDescription("video", "Convert a demo playback to a video file/stream\nUsage: video (<videopipe>)");
	Cmd_SetDescription("video-pipe", "Set the video pipe to convert demo playback to a video file\nUsage: video <videopipe>");
	Cmd_SetDescription("stopvideo", "Stop convert a demo playback to video file\nUsage: stopvideo");
	Cmd_SetDescription("serverinfo", "Gives information about local server from the console of that server\nUsage: serverinfo");
	Cmd_SetDescription("systeminfo", "Returns values for g_syncronousclients, sv_serverid, and timescale\nUsage: systeminfo");
	Cmd_SetDescription("callvote", "Caller automatically votes yes vote has a 30 second timeout each client can only call 3 votes a level vote is displayed on screen with totals\nvote commands are: map_restart, nextmap, map, g_gametype and kick\nUsage: callvote <command> vote <y/n>");
#ifdef USE_CURL
	Cmd_SetDescription("download", "Download a file from the server\nUsage: download <mapname>");
	Cmd_SetDescription("dlmap", "Download a file from the server\nUsage: dlmap <mapname>");
#endif

  /// console
  Cmd_SetDescription("clear", "Clear all text from console\nUsage: clear");
	Cmd_SetDescription("condump", "Write the console text to a file\nUsage: condump <file>");
	Cmd_SetDescription("toggleconsole", "Usually bound to ~ the tilde key brings the console up and down\nUsage: bind <key> toggleconsole");
	Cmd_SetDescription("messagemode", "Send a message to everyone");
	Cmd_SetDescription("messagemode2", "Send a message to teammates");
	Cmd_SetDescription("messagemode3", "Send a message to targeted player");
	Cmd_SetDescription("messagemode4", "Send a message to last attacker");

}

void Cmd_SetServerDescriptions( void ) {
  
}

void Cmd_SetInputDescriptions( void ) {
  Cmd_SetDescription("centerview", "Quickly move current view to the center of screen\nUsage: bind <key> centerview");
	Cmd_SetDescription("+moveup", "Start moving up (jump, climb up, swim up)\nUsage: bind <key> +moveup");
	Cmd_SetDescription("+movedown", "Start moving down (crouch, climb down, swim down)\nUsage: bind <key> +movedown");
	Cmd_SetDescription("+left", "Start turning left\nUsage: bind <key> +left");
	Cmd_SetDescription("+right", "Start turning right\nUsage: bind <key> +right");
	Cmd_SetDescription("+forward", "Start moving forward\nUsage: bind <key> +forward");
	Cmd_SetDescription("+back", "Start moving backwards\nUsage: bind <key> +back");
	Cmd_SetDescription("+lookup", "Start looking up\nUsage: bind <key> +lookup");
	Cmd_SetDescription("+lookdown", "Start looking down\nUsage: bind <key> +lookdown");
	Cmd_SetDescription("+strafe", "Start changing directional movement into strafing movement\nUsage: bind <key> +strafe");
	Cmd_SetDescription("+moveleft", "Start strafing to the left\nUsage: bind <key> +moveleft");
	Cmd_SetDescription("+moveright", "Start strafing to the right\nUsage: bind <key> +moveright");
	Cmd_SetDescription("+speed", "Speed toggle bound to shift key by default toggles run/walk\nUsage: bind <key> +speed");
	Cmd_SetDescription("+attack", "Start attacking (shooting, punching)\nUsage: bind <key> +attack");
	Cmd_SetDescription("+button0", "Start firing same as mouse button 1 (fires weapon)\nUsage: bind <key> +button0");
	Cmd_SetDescription("+button1", "Start displaying chat bubble\nUsage: bind <key> +button1");
	Cmd_SetDescription("+button2", "Start using items (same as enter)\nUsage: bind <key> +button2");
	Cmd_SetDescription("+button3", "Start player taunt animation\nUsage: bind <key> +button3");
	Cmd_SetDescription("+button4", "Fixed +button4 not causing footsteps\nUsage: bind <key> +button4");
	Cmd_SetDescription("+button5", "Used for MODS also used by Team Arena Mission Pack\nUsage: bind <key> +button5");
	Cmd_SetDescription("+button6", "Used for MODS also used by Team Arena Mission Pack\nUsage: bind <key> +button6");
	Cmd_SetDescription("+button7", "Start hand signal, player model looks like it's motioning to team \"move forward\"\nUsage: bind <key> +button7");
	Cmd_SetDescription("+button8", "Start hand signal, player model looks like it's motioning to team \"come here\"\nUsage: bind <key> +button8");
	Cmd_SetDescription("+button9", "Stop hand signal, player model looks like it's motioning to team \"come to my left side\"\nUsage: bind <key> +button9");
	Cmd_SetDescription("+button10", "Start hand signal, player model looks like it's motioning to team \"come to my right side\"\nUsage: bind <key> +button10");
	Cmd_SetDescription("+mlook", "Start using mouse movements to control head movement\nUsage: bind <key> +mlook");

}

void Cmd_SetFilesDescriptions( void ) {
  
}

void Cmd_SetSoundDescriptions( void ) {
  Cmd_SetDescription("play", "Play a sound file\nUsage: play <filename>");
  Cmd_SetDescription("music", "Play a specific music file\nUsage: music <filename>");
  Cmd_SetDescription("stopmusic", "Stop playing music\nUsage: stopmusic");
  Cmd_SetDescription("s_list", "Display paths and filenames of all sound files as they are played\nUsage: s_list");
  Cmd_SetDescription("soundlist", "Display paths and filenames of all sound files as they are played\nUsage: soundlist");
  Cmd_SetDescription("s_stop", "Stop whatever sound that is currently playing from playing\nUsage: s_stop");
  Cmd_SetDescription("stopsound", "Stop whatever sound that is currently playing from playing\nUsage: stopsound");
  Cmd_SetDescription("s_info", "Display information about sound system\nUsage: s_info");
  Cmd_SetDescription("soundinfo", "Display information about sound system\nUsage: soundinfo");
}

void Cmd_SetCommandDescriptions( void ) {
	Cmd_SetDescription("cmdlist", "List all available console commands\nUsage: cmdlist");
	Cmd_SetDescription("exec",    "Execute a config file or script\nUsage: exec <configfile>");
	Cmd_SetDescription("execq",   "Quietly execute a config file or script\nUsage: execq <configfile>");
	Cmd_SetDescription("vstr",    "Identifies the attached command as a variable string\nUsage: vstr <variable>");
	Cmd_SetDescription("echo",    "Echo a string to the message display to your console only\nUsage: echo <message>");
	Cmd_SetDescription("wait",    "Stop execution and wait one game tick\nUsage: wait (<# ticks> optional)" );
	Cmd_SetDescription("help",    "Display helpful description for any console command\nUsage: help <command>");

}

void Cmd_SetRendererDescriptions( void ) {
  
}

void Cmd_SetKnownDescriptions( void ) {
  
}
