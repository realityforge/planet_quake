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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cmd.c -- Quake script command processing module

#include "q_shared.h"
#include "qcommon.h"

#define MAX_CMD_BUFFER  65536

typedef struct {
	byte *data;
	int maxsize;
	int cursize;
	qboolean filtered;
	int tag;
} cmd_t;

int    cmd_wait;
cmd_t  cmd_text[32];
byte   cmd_text_buf[32][MAX_CMD_BUFFER];
int    insCmdI;
int    execCmdI;

#ifdef USE_SERVER_ROLES
static qboolean limited;
#endif

static void Cmd_Help_f( void );

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "cmd use rocket ; +attack ; wait ; -attack ; cmd use blaster"
============
*/
static void Cmd_Wait_f( void ) {
	if ( Cmd_Argc() == 2 ) {
		cmd_wait = atoi( Cmd_Argv( 1 ) );
		if ( cmd_wait < 0 )
			cmd_wait = 1; // ignore the argument
	} else {
		cmd_wait = 1;
	}
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

/*
============
Cbuf_Init
============
*/
void Cbuf_Init( void )
{
	for(int ci = 0; ci < 32; ci++) {
		cmd_text[ci].data = cmd_text_buf[ci];
		cmd_text[ci].maxsize = MAX_CMD_BUFFER;
		cmd_text[ci].cursize = 0;
	}
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void Cbuf_AddText( const char *text ) {
	int l;

	l = strlen (text);

	if (cmd_text[insCmdI].cursize + l >= cmd_text[insCmdI].maxsize)
	{
		Com_Printf ("Cbuf_AddText: overflow\n");
		return;
	}
	Com_Memcpy(&cmd_text[insCmdI].data[cmd_text[insCmdI].cursize], text, l);
	cmd_text[insCmdI].cursize += l;
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void Cbuf_InsertText( const char *text ) {
	int		len;
	int		i;

	len = strlen( text ) + 1;

	if ( len + cmd_text[insCmdI].cursize > cmd_text[insCmdI].maxsize ) {
		Com_Printf( "Cbuf_InsertText overflowed\n" );
		return;
	}

	// move the existing command text
	for ( i = cmd_text[insCmdI].cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text[insCmdI].data[ i + len ] = cmd_text[insCmdI].data[ i ];
	}

	// copy the new text in
	Com_Memcpy( cmd_text[insCmdI].data, text, len - 1 );

	// add a \n
	cmd_text[insCmdI].data[ len - 1 ] = '\n';

	cmd_text[insCmdI].cursize += len;
}


/*
============
Cbuf_ExecuteText
============
*/
static void Cbuf_ExecuteInternal( cbufExec_t exec_when, const char *text )
{
	switch (exec_when)
	{
	case EXEC_NOW:
		if ( text && text[0] != '\0' ) {
			Com_DPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", text);
			Cmd_ExecuteString (text, qfalse, 0);
		} else {
			Cbuf_Execute();
			Com_DPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", cmd_text[insCmdI].data);
		}
		break;
	case EXEC_INSERT:
		Cbuf_InsertText (text);
		break;
	case EXEC_APPEND:
		Cbuf_AddText (text);
		break;
	default:
		Com_Error (ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}


/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute( void )
{
	int i;
	char *text;
	char line[MAX_CMD_LINE];
	int quotes;

	insCmdI++;
	if(insCmdI == 32) {
		insCmdI = 0;
	}

	while(execCmdI != insCmdI) {

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	qboolean in_star_comment = qfalse;
	qboolean in_slash_comment = qfalse;
	while ( cmd_text[execCmdI].cursize > 0 )
	{
		if ( cmd_wait > 0 ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}

		// find a \n or ; line break or comment: // or /* */
		text = (char *)cmd_text[execCmdI].data;

		quotes = 0;
		for ( i = 0 ; i< cmd_text[execCmdI].cursize ; i++ )
		{
			if (text[i] == '"')
				quotes++;

			if ( !(quotes&1)) {
				if ( i < cmd_text[execCmdI].cursize - 1 ) {
					if ( !in_star_comment && text[i] == '/' && text[i+1] == '/' )
						in_slash_comment = qtrue;
					else if ( !in_slash_comment && text[i] == '/' && text[i+1] == '*' )
						in_star_comment = qtrue;
					else if ( in_star_comment && text[i] == '*' && text[i+1] == '/' ) {
						in_star_comment = qfalse;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if ( !in_slash_comment && !in_star_comment && text[i] == ';')
					break;
			}
			if ( !in_star_comment && (text[i] == '\n' || text[i] == '\r') ) {
				in_slash_comment = qfalse;
				break;
			}
		}

		if ( i >= (MAX_CMD_LINE - 1) )
			i = MAX_CMD_LINE - 1;

		Com_Memcpy( line, text, i );
		line[i] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if ( i == cmd_text[execCmdI].cursize ) {
			cmd_text[execCmdI].cursize = 0;
			cmd_text[execCmdI].tag = 0;
			cmd_text[execCmdI].filtered = qfalse;
		} 
		else
		{
			i++;
			cmd_text[execCmdI].cursize -= i;
			// skip all repeating newlines/semicolons
			while ( ( text[i] == '\n' || text[i] == '\r' || text[i] == ';' ) && cmd_text[execCmdI].cursize > 0 ) {
				cmd_text[execCmdI].cursize--;
				i++;
			}
			memmove( text, text+i, cmd_text[execCmdI].cursize );
		}

		// execute the command line
		if(cmd_text[execCmdI].filtered) {
			Cmd_ExecuteString( line, qfalse, cmd_text[execCmdI].tag );
		} else {
			Cmd_ExecuteString( line, qfalse, 0 );
		}
#ifdef USE_ASYNCHRONOUS
		// if an execution invoked a callback event like `\fs_restart`, run the rest next frame
		if(!FS_Initialized()) {
			return;
		}
#endif
	}
		
		execCmdI++;
		if(execCmdI == 32) {
			execCmdI = 0;
		}
	}
}


/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
Cmd_Exec_f
===============
*/
int overflowCounter = 0;
int lastExec = 0;
char lastScript[MAX_QPATH*10];
static void Cmd_Exec_f( void ) {
	qboolean quiet;
	union {
		char *c;
		void *v;
	} f;
	char filename[MAX_QPATH];
	int execTime = Com_Milliseconds();

	quiet = !Q_stricmp(Cmd_Argv(0), "execq");

	if (Cmd_Argc () != 2) {
		Com_Printf ("exec%s <filename> : execute a script file%s\n",
			quiet ? "q" : "", quiet ? " without notification" : "");
		return;
	}

	Q_strncpyz( filename, Cmd_Argv(1), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	if(lastExec == 0 || execTime - lastExec > cl_execTimeout->integer) {
		lastExec = execTime;
		lastScript[0] = 0;
		overflowCounter = 0;
	} else {
		overflowCounter++;
	}
	if(overflowCounter >= cl_execOverflow->integer) {
		if(!Q_stristr(lastScript, filename)) {
			int addFile = strlen(lastScript);
			lastScript[addFile] = ';';
			Com_Memcpy(&lastScript[addFile+1], filename, strlen(filename));
		}
		// TODO: show a line number of where the repeat exec came from in the cfg?
		Com_Printf( "EXEC OVERFLOW ERROR! not executing %s for at least %i milliseconds\n", filename, cl_execTimeout->integer );
		return;
	}
	FS_BypassPure();
	FS_ReadFile( filename, &f.v );
	FS_RestorePure();
	if ( f.v == NULL ) {
		Com_Printf( "couldn't exec %s\n", filename );
		return;
	}
	if (!quiet)
		Com_Printf ("execing %s: %s\n", filename, f.c);

	Cbuf_InsertText( f.c );

#ifdef DELAY_WRITECONFIG
	if ( !Q_stricmp( filename, Q3CONFIG_CFG ) ) {
		Com_WriteConfiguration(); // to avoid loading outdated values
	}
#endif

	FS_FreeFile( f.v );
}


/*
===============
Cmd_Vstr_f

Inserts the current value of a variable as command text
===============
*/
static void Cmd_Vstr_f( void ) {
	const char *v;

	if ( Cmd_Argc () != 2 ) {
		Com_Printf( "vstr <variablename> : execute a variable command\n" );
		return;
	}

	v = Cvar_VariableString( Cmd_Argv( 1 ) );
	Cbuf_InsertText( v );
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
static void Cmd_Echo_f( void )
{
	Com_Printf( "%s\n", Cmd_ArgsFrom( 1 ) );
}


/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	char					*description;
	xcommand_t				function;
	completionFunc_t	complete;
	qboolean limited;
} cmd_function_t;


static	int			cmd_argc;
static	char		*cmd_argv[MAX_STRING_TOKENS];		// points into cmd_tokenized
static	char		cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];	// will have 0 bytes inserted
static	char		cmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

static	cmd_function_t	*cmd_functions;		// possible commands to execute
typedef struct cmdContext_s
{
	int		argc;
	char	*argv[ MAX_STRING_TOKENS ];	// points into cmd.tokenized
	char	tokenized[ BIG_INFO_STRING + MAX_STRING_TOKENS ];	// will have 0 bytes inserted
	char	cmd[ BIG_INFO_STRING ]; // the original command we received (no token processing)
} cmdContext_t;

static cmdContext_t		cmd;
static cmdContext_t		savedCmd;

/*
============
Cmd_Argc
============
*/
int Cmd_Argc( void ) {
	return cmd_argc;
}


/*
============
Cmd_Clear
============
*/
void Cmd_Clear( void ) {
	cmd_cmd[0] = '\0';
	cmd_argc = 0;
}


/*
============
Cmd_Argv
============
*/
char *Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= cmd_argc ) {
		return "";
	}
	return cmd_argv[arg];
}


/*
============
Cmd_ArgvBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_Argv( arg ), bufferLength );
}


/*
============
Cmd_Args

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
char *Cmd_ArgsFrom( int arg ) {
	static char cmd_args[BIG_INFO_STRING], *s;
	int i;

	s = cmd_args;
	*s = '\0';
	if (arg < 0)
		arg = 0;
	for ( i = arg ; i < cmd_argc ; i++ ) {
		s = Q_stradd( s, cmd_argv[i] );
		if ( i != cmd_argc-1 ) {
			s = Q_stradd( s, " " );
		}
	}

	return cmd_args;
}


/*
============
Cmd_ArgsBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void Cmd_ArgsBuffer( char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_ArgsFrom( 1 ), bufferLength );
}


/*
============
Cmd_Cmd

Retrieve the unmodified command string
For rcon use when you want to transmit without altering quoting
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
============
*/
char *Cmd_Cmd( void )
{
	return cmd_cmd;
}


/*
   Replace command separators with space to prevent interpretation
   This is a hack to protect buggy qvms
   https://bugzilla.icculus.org/show_bug.cgi?id=3593
   https://bugzilla.icculus.org/show_bug.cgi?id=4769
*/
void Cmd_Args_Sanitize( const char *separators )
{
	int i;

	for( i = 1; i < cmd_argc; i++ )
	{
		char *c = cmd_argv[i];

		while ( ( c = strpbrk( c, separators ) ) != NULL ) {
			*c = ' ';
			++c;
		}
	}
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
The text is copied to a separate buffer and 0 characters
are inserted in the appropriate place, The argv array
will point into this temporary buffer.
============
*/
// NOTE TTimo define that to track tokenization issues
//#define TKN_DBG
static void Cmd_TokenizeString2( const char *text_in, qboolean ignoreQuotes ) {
	const char *text;
	char *textOut;

#ifdef TKN_DBG
	// FIXME TTimo blunt hook to try to find the tokenization of userinfo
	Com_DPrintf("Cmd_TokenizeString: %s\n", text_in);
#endif

	// clear previous args
	cmd_argc = 0;
	cmd_cmd[0] = '\0';

	if ( !text_in ) {
		return;
	}

	Q_strncpyz( cmd_cmd, text_in, sizeof( cmd_cmd ) );

	text = cmd_cmd; // read from safe-length buffer
	textOut = cmd_tokenized;

	while ( 1 ) {
		if ( cmd_argc >= ARRAY_LEN( cmd_argv ) ) {
			return;			// this is usually something malicious
		}

		while ( 1 ) {
			// skip whitespace
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;			// all tokens parsed
			}

			// skip // comments
			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					return; // all tokens parsed
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text += 2;
			} else {
				break;			// we are ready to parse a token
			}
		}

		// handle quoted strings
		// NOTE TTimo this doesn't handle \" escaping
		if ( !ignoreQuotes && *text == '"' ) {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			text++;
			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}
			*textOut++ = '\0';
			if ( !*text ) {
				return;		// all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		cmd_argv[cmd_argc] = textOut;
		cmd_argc++;

		// skip until whitespace, quote, or command
		while ( *text > ' ' ) {
			if ( !ignoreQuotes && text[0] == '"' ) {
				break;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					break;
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = '\0';

		if ( !*text ) {
			return;		// all tokens parsed
		}
	}
}


/*
============
Cmd_TokenizeString
============
*/
void Cmd_TokenizeString( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qfalse );
}


/*
============
Cmd_TokenizeStringIgnoreQuotes
============
*/
void Cmd_TokenizeStringIgnoreQuotes( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qtrue );
}


/*
============
Cmd_FindCommand
============
*/
static cmd_function_t *Cmd_FindCommand( const char *cmd_name )
{
	cmd_function_t *cmd;
	for( cmd = cmd_functions; cmd; cmd = cmd->next )
		if( !Q_stricmp( cmd_name, cmd->name ) )
			return cmd;
	return NULL;
}


/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand( const char *cmd_name, xcommand_t function ) {
	cmd_function_t *cmd;

	// fail if the command already exists
	if ( Cmd_FindCommand( cmd_name ) )
	{
		// allow completion-only commands to be silently doubled
		if ( function != NULL )
			Com_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name );
		return;
	}

	// use a small malloc to avoid zone fragmentation
	cmd = S_Malloc( sizeof( *cmd ) );
	cmd->name = CopyString( cmd_name );
	cmd->description = NULL;
	cmd->function = function;
	cmd->complete = NULL;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}


/*
============
Cmd_SetCommandCompletionFunc
============
*/
void Cmd_SetCommandCompletionFunc( const char *command, completionFunc_t complete ) {
	cmd_function_t *cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if( !Q_stricmp( command, cmd->name ) ) {
			cmd->complete = complete;
			return;
		}
	}
}


/*
============
Cmd_RemoveCommand
============
*/
void Cmd_RemoveCommand( const char *cmd_name ) {
	cmd_function_t *cmd, **back;

	back = &cmd_functions;
	while( 1 ) {
		cmd = *back;
		if ( !cmd ) {
			// command wasn't active
			return;
		}
		if ( !Q_stricmp( cmd_name, cmd->name ) ) {
			*back = cmd->next;
			if (cmd->name) {
				Z_Free(cmd->name);
			}
			if (cmd->description) {
				Z_Free(cmd->description);
			}
			Z_Free (cmd);
			return;
		}
		back = &cmd->next;
	}
}


/*
============
Cmd_RemoveCommandSafe

Only remove commands with no associated function
============
*/
void Cmd_RemoveCommandSafe( const char *cmd_name )
{
	cmd_function_t *cmd = Cmd_FindCommand( cmd_name );

	if( !cmd )
		return;
	if( cmd->function )
	{
		Com_Error( ERR_DROP, "Restricted source tried to remove "
			"system command \"%s\"", cmd_name );
		return;
	}

	Cmd_RemoveCommand( cmd_name );
}


/*
============
Cmd_RemoveCgameCommands

Remove cgame-created commands
============
*/
void Cmd_RemoveCgameCommands( void )
{
	cmd_function_t *cmd;
	qboolean removed;

	do {
		removed = qfalse;
		for ( cmd = cmd_functions ; cmd ; cmd = cmd->next ) {
			if ( cmd->function == NULL ) {
				Cmd_RemoveCommand( cmd->name );
				removed = qtrue;
				break;
			}
		}
	} while ( removed );
}


/*
============
Cmd_CommandCompletion
============
*/
void Cmd_CommandCompletion( void(*callback)(const char *s) ) {
	const cmd_function_t *cmd;

	for ( cmd = cmd_functions ; cmd ; cmd=cmd->next ) {
		callback( cmd->name );
	}
}


/*
============
Cmd_CompleteArgument
============
*/
qboolean Cmd_CompleteArgument( const char *command, char *args, int argNum ) {
	const cmd_function_t *cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if ( !Q_stricmp( command, cmd->name ) ) {
			if ( cmd->complete ) {
				cmd->complete( args, argNum );
			}
			return qtrue;
		}
	}

	return qfalse;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
============
*/
qboolean Cmd_ExecuteString( const char *text, qboolean noServer, int tag ) {
	cmd_function_t *cmd, **prev;

	// execute the command line
	Cmd_TokenizeString( text );
	if ( !Cmd_Argc() ) {
		return qfalse;		// no tokens
	}

	// check registered command functions
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( !Q_stricmp( cmd_argv[0], cmd->name ) ) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			*prev = cmd->next;
			cmd->next = cmd_functions;
			cmd_functions = cmd;

			// perform the action
			if ( !cmd->function ) {
				// let the cgame or game handle it
				break;
			} else {
#ifdef USE_SERVER_ROLES
				if(limited && !cmd->limited)
					Com_Printf("Could not execute command %s, you do not have permission.\n", cmd->name);
				else
#endif
				cmd->function();
			}
			return qtrue;
		}
	}

	// check cvars
	if ( Cvar_Command() ) {
		return qtrue;
	}
	
#ifndef DEDICATED
	// check client game commands
	if ( com_dedicated && !com_dedicated->integer && com_cl_running && com_cl_running->integer && CL_GameCommand(tag) ) {
		return qtrue;
	}
#endif

#ifndef BUILD_SLIM_CLIENT
#ifdef USE_LOCAL_DED
	if (com_dedicated->integer)
#endif
	// check server game commands
	if ( !noServer && com_sv_running && com_sv_running->integer && SV_GameCommand(tag) ) {
		return qtrue;
	}
#endif

#ifndef DEDICATED
	// check ui commands
	if ( com_dedicated && !com_dedicated->integer && com_cl_running && com_cl_running->integer && UI_GameCommand(tag) ) {
		return qtrue;
	}

	if(noServer && com_dedicated && com_dedicated->integer) {
		return qfalse;
	}

	// send it as a server command if we are connected
	// this will usually result in a chat message
	if(!noServer && com_dedicated && !com_dedicated->integer) {
		CL_ForwardCommandToServer( text );
		return qtrue;		
	} else {
		// ForwardCommandToClient
		return qfalse;
	}
#else
	return qfalse;
#endif
}


/*
============
Cmd_List_f
============
*/
static void Cmd_List_f( void )
{
	const cmd_function_t *cmd;
	const char *match;
	int i;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( cmd = cmd_functions ; cmd ; cmd=cmd->next ) {
		if ( match && !Com_Filter( match, cmd->name ) )
			continue;
		Com_Printf( "%s\n", cmd->name );
		i++;
	}
	Com_Printf( "%i commands\n", i );
}


/*
==================
Cmd_CompleteCfgName
==================
*/
static void Cmd_CompleteCfgName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_ANY | FS_MATCH_STICK );
	}
}


/*
==================
Cmd_CompleteWriteCfgName
==================
*/
void Cmd_CompleteWriteCfgName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_EXTERN | FS_MATCH_STICK );
	}
}


/*
============
Cmd_Init
============
*/
void Cmd_Init( void ) {
	Cmd_AddCommand ("cmdlist",Cmd_List_f);
	Cmd_SetDescription("cmdlist", "List all available console commands\nUsage: cmdlist");
	Cmd_AddCommand ("exec",Cmd_Exec_f);
	Cmd_SetCommandCompletionFunc( "exec", Cmd_CompleteCfgName );
	Cmd_SetDescription("exec", "Execute a config file or script\nUsage: exec <configfile>");
	Cmd_AddCommand ("execq", Cmd_Exec_f);
	Cmd_SetCommandCompletionFunc( "execq", Cmd_CompleteCfgName );
	Cmd_SetDescription("execq", "Quietly execute a config file or script\nUsage: execq <configfile>");
	Cmd_AddCommand ("vstr",Cmd_Vstr_f);
	Cmd_SetCommandCompletionFunc( "vstr", Cvar_CompleteCvarName );
	Cmd_SetDescription("vstr", "Identifies the attached command as a variable string\nUsage: vstr <variable>");
	Cmd_AddCommand ("echo",Cmd_Echo_f);
	Cmd_SetDescription( "echo", "Echo a string to the message display to your console only\nUsage: echo <message>");
	Cmd_AddCommand ("wait", Cmd_Wait_f);
	Cmd_SetDescription( "wait", "Stop execution and wait one game tick\nUsage: wait (<# ticks> optional)" );
	Cmd_AddCommand ("help", Cmd_Help_f);
	Cmd_SetDescription("help", "Display helpful description for any console command\nUsage: help <command>");

	cl_execTimeout = Cvar_Get("cl_execTimeout", "2000", CVAR_ARCHIVE | CV_INTEGER);
	cl_execOverflow = Cvar_Get("cl_execOverflow", "200", CVAR_ARCHIVE | CV_INTEGER);
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText( cbufExec_t exec_when, const char *text )
{
	if(cmd_text[insCmdI].filtered == qfalse) {
		// use same buffer
	} else {
		insCmdI++;
		if(insCmdI == 32) {
			insCmdI = 0;
		}
	}
	cmd_text[insCmdI].filtered = qfalse;
	cmd_text[insCmdI].tag = 0;
	Cbuf_ExecuteInternal( exec_when, text );
}


void Cbuf_ExecuteTagged( cbufExec_t exec_when, const char *text, int tag )
{
	if(cmd_text[insCmdI].filtered == qtrue
		&& cmd_text[insCmdI].tag == tag) {
		// use same buffer
	} else {
		insCmdI++;
		if(insCmdI == 32) {
			insCmdI = 0;
		}
	}
	cmd_text[insCmdI].filtered = qtrue;
	cmd_text[insCmdI].tag = tag;
	Cbuf_ExecuteInternal( exec_when, text );
}


/*
============
Cmd_SaveCmdContext

Save the tokenized strings and cmd so that later we can restore them and the engine will continue its usual processing normally
============
*/
void Cmd_SaveCmdContext( void )
{
	Com_Memcpy( &savedCmd, &cmd, sizeof( cmdContext_t ) );
}

/*
============
Cmd_RestoreCmdContext

Restore the tokenized strings and cmd saved previously so that the engine can continue its usual processing
============
*/
void Cmd_RestoreCmdContext( void )
{
	Com_Memcpy( &cmd, &savedCmd, sizeof( cmdContext_t ) );
}


/*
=====================
Cmd_SetDescription
=====================
*/
void Cmd_SetDescription( const char *cmd_name, char *cmd_description )
{
	cmd_function_t *cmd = Cmd_FindCommand( cmd_name );
	if(!cmd) return;

	if( cmd_description && cmd_description[0] != '\0' )
	{
		if( cmd->description != NULL )
		{
			Z_Free( cmd->description );
		}
		cmd->description = CopyString( cmd_description );
	}
}


/*
============
Cmd_Help

Prints the value, default, and latched string of the given variable
============
*/
static void Cmd_Help( const cmd_function_t *cmd ) {	
	Com_Printf ("\"%s\" " S_COLOR_WHITE "",
		cmd->name );

	Com_Printf (" autocomplete:\"%s" S_COLOR_WHITE "\"",
		cmd->complete ? "yes" : "no" );

	Com_Printf ("\n");

	if ( cmd->description ) {
		Com_Printf( "%s\n", cmd->description );
	}
}


static	char		props[BIG_INFO_STRING];
char *Cmd_TokenizeAlphanumeric(const char *text_in, int *count) {
	int c = 0, r = 0, len = strlen(text_in);
  *count = 0;
	props[0] = 0;
	while(c < len) {
		if((text_in[c] >= 'a' && text_in[c] <= 'z')
			|| (text_in[c] >= 'A' && text_in[c] <= 'Z')
			|| (text_in[c] >= '0' && text_in[c] <= '9')
      || text_in[c] == '_') {
			props[r] = text_in[c];
			r++;
		} else {
			if(r > 0 && props[r-1] != 0) {
				props[r] = '\0';
				(*count)++;
				r++;
			}
		}
		c++;
	}
	if(r > 0 && props[r-1] != '\0') {
		props[r] = '\0';
		(*count)++;
		r++;
	}
	return props;
}


#ifdef USE_SERVER_ROLES
qboolean Cmd_ExecuteLimitedString( const char *text, qboolean noServer, int role ) {
	limited = qtrue;
	qboolean result = Cmd_ExecuteString(text, noServer, 0);
	limited = qfalse;
	return result;
}

void Cmd_FilterLimited(char *commandList) {
	cmd_function_t *cmd, **prev;
	int cmdCount = 0;
	// force 3 roles to be available?
	char *cmds = Cmd_TokenizeAlphanumeric(commandList, &cmdCount);
	// loop through each command and mark it  as limited
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		cmd->limited = qfalse;
		// check if command  is in  command whitelist for the role
		for(int i = 0; i < cmdCount; i++) {
			if(Q_stricmp(&cmds[i], cmd->name)==0) {
				cmd->limited = qtrue;
			}
			cmds = &cmds[strlen(cmds)+1];
		}
	}
	
}
#endif


/*
============
Cmd_Help_f

Prints the contents of a cvar 
(preferred over Cvar_Command where cvar names and commands conflict)
============
*/
static void Cmd_Help_f( void )
{
	char *name;
	cmd_function_t *cmd;

	if(Cmd_Argc() != 2)
	{
		Com_Printf ("List all commands using \\cmdlist\nUsage: help <command>\n");
		return;
	}

	name = CopyString(Cmd_Argv(1));

	if(!Q_stricmp("all", name)) {
		for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
			Cmd_Help(cmd);
		}
		return;
	}

	Cmd_TokenizeString(name);
	if(Cvar_Command()) {
		Z_Free(name);
		return;
	}
	
	cmd = Cmd_FindCommand( name );
	
	if(cmd)
		Cmd_Help(cmd);
	else
		Com_Printf ("Command %s does not exist.\n", name);

	Z_Free(name);
}
