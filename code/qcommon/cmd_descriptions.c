
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"



void Cmd_SetClientDescriptions( void ) {
  
}

void Cmd_SetServerDescriptions( void ) {
  
}

void Cmd_SetInputDescriptions( void ) {
  
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
