
#include "q_shared.h"
#include "qcommon.h"
#include "../client/client.h"

#undef assert
#define assert(x) {if(!(x)) { Com_Error( ERR_FATAL, "assert failed: %s", #x ); }}
#define PK3_HASH_SIZE 512
#define FS_HashFileName Com_GenerateHashValue
#define CACHE_FILE_NAME "pk3cache.dat"
#define FS_HashFileName Com_GenerateHashValue
#define MAX_NOTIFICATIONS 10

void Com_ExecuteCfg( void );
int FS_ReadFile( const char *qpath, void **buffer );
void Sys_FreeFileList( char **list );
void FS_AddGameDirectory( const char *path, const char *dir, int igvm );
int FS_FileLength( FILE* h );
qboolean FS_GeneralRef( const char *filename );


#ifdef _DEBUG
static const char *NEEDED = "";
#endif

#if defined(USE_LAZY_LOAD) || defined(USE_ASYNCHRONOUS)
// store a list of files to download this session
typedef struct downloadLazy_s {
  char *downloadName; // this is the of the file in the shader
	char *loadingName; // this is the name of the shader to update
	time_t lastRequested;
	int state;
  struct downloadLazy_s *next;
} downloadLazy_t;

extern qboolean		com_fullyInitialized;
static downloadLazy_t *readyCallback[PK3_HASH_SIZE]; // MAX_MOD_KNOWN
static int secondTimer = 0;
static int thirdTimer = 0;
extern cvar_t *cl_lazyLoad;
cvar_t *cl_dlSimultaneous;
char asyncShaderName[MAX_OSPATH * 2];
char asyncModelName[MAX_OSPATH * 2];
char asyncSoundName[MAX_OSPATH * 2];

#endif

#if defined(USE_LIVE_RELOAD) || defined(__WASM__)
static int lastVersionTime = 0;
#endif

#if _DEBUG
static char stupidPathError[MAX_OSPATH];
#endif

static int FS_HashPK3( const char *name )
{
	unsigned int c, hash = 0;
	while ( (c = *name++) != '\0' )
	{
		hash = hash * 101 + c;
	}
	hash = hash ^ (hash >> 16);
	return hash & (PK3_HASH_SIZE-1);
}


#ifdef USE_LIVE_RELOAD
void Sys_FileNeeded(const char *filename, int state);
cvar_t *fs_notify[MAX_NOTIFICATIONS];
int fileTimes[MAX_NOTIFICATIONS];
char realNames[MAX_NOTIFICATIONS][MAX_OSPATH];
static int changeTimer = 0;

// basic notification will just check file times once every few seconds
//   if we wan't we can add inotify APIs here later
//#ifndef __APPLE__
//#include <sys/inotify.h>
//#endif


//void FS_ReloadQVM( void ) {
//  Cbuf_AddText("wait; wait; wait; vid_restart;");

//void FS_ReloadGame( void ) {
//  Cbuf_AddText("wait; wait; wait; map_restart;");


void Sys_ChangeNotify(char *ready) {
	fileOffset_t size;
	fileTime_t mtime;
	fileTime_t ctime;
	for(int i = 0; i < MAX_NOTIFICATIONS; i++) {
		if(fileTimes[i] > 0 // make sure file was originally found
			&& fs_notify[i]->string[0]  // make sure it wasn't switched off
			&& realNames[i][0] // make sure it has a real path to check
		) {
			if(Sys_GetFileStats( realNames[i], &size, &mtime, &ctime )) {
				fileTimes[i] = mtime;
				// incase it gets reset by a command
				strcpy(ready, fs_notify[i]->string);
				break;
			}
		}
	}

}


void FS_NotifyChange(const char *localPath) {
	fileOffset_t size;
	fileTime_t mtime;
	fileTime_t ctime;
	for(int i = 0; i < MAX_NOTIFICATIONS; i++) {
		if(!fs_notify[i]->string[0] || !Q_stricmp(fs_notify[i]->string, localPath)) {
			if(!fs_notify[i]->string[0]) {
				Cvar_Set(va("fs_notify%d", i), localPath);
			}

			// set the initial file time so we know when to update
			char realPath[MAX_OSPATH];
			Com_sprintf( realPath, sizeof( realPath ), "%s/%s", 
				Cvar_VariableString("fs_homepath"), localPath );
			if(Sys_GetFileStats( realPath, &size, &mtime, &ctime )) {
				fileTimes[i] = mtime;
				strcpy(realNames[i], realPath);
			} else {
				realNames[i][0] = '\0';
			}
			break;
		}
	}
}

// TODO:
//int FS_NotifyChange_f(const char *localPath) {

void CL_ChangeNotifcations( void ) {
	char ready[MAX_OSPATH];
	int newTime = Sys_Milliseconds();

	if(newTime - changeTimer < 5000) {
		return;
	}

	changeTimer = newTime;

#ifdef USE_ASYNCHRONOUS
	// ping the server version.json file for a file time
	Sys_FileNeeded("version.json", qfalse);
#endif

	ready[0] = '\0';
	Sys_ChangeNotify(ready);
	if(strlen(ready) == 0) {
		return;
	}

	// TODO: use the same file update logic as USE_ASYNCHRONOUS
	//FS_UpdateFiles(ready, &ready[MAX_OSPATH]);

}

#endif


#ifdef USE_ASYNCHRONOUS
#define JSON_IMPLEMENTATION
#include "../qcommon/json.h"
#undef JSON_IMPLEMENTATION

extern int cmd_lazy;

void Sys_UpdateNeeded( downloadLazy_t **ready, downloadLazy_t **downloadNeeded ) {
	downloadLazy_t *highestDownload = NULL;
	downloadLazy_t *download;
	int time = Sys_Milliseconds();
	if(ready) {
		*ready = NULL;
	}
	if(downloadNeeded) {
		highestDownload = NULL;
		*downloadNeeded = NULL;
	}

	// TODO: loop over readyFiles instead?
	for(int i = 0; i < PK3_HASH_SIZE; i++) {
		download = readyCallback[i];
    if(!download) {
			continue;
		}

		do {
#ifdef _DEBUG
			//if(Q_stristr(download->downloadName, "default.cfg")) {
			//	assert(download->state > VFS_LATER);
			//}
			if(NEEDED[0] != '\0'
				&& !Q_stristr(download->downloadName, ".pk3dir/")
				&& Q_stristr(download->downloadName, NEEDED)) {
				assert(download->state > VFS_NOENT);
			}
#endif

			if(download->state >= VFS_DONE) {
				continue;
			}
			if(download->state == VFS_READY && ready && !*ready) {
				// memcpy because paths are seperated by special characters and zeros
				*ready = download;
				download->state = VFS_DONE; // incase its not cleared right away
				continue;
			}
			if(download->state > VFS_LATER && download->state < VFS_DL 
				&& downloadNeeded && !*downloadNeeded
				&& time - download->lastRequested > 1500
				&& (!highestDownload || highestDownload->state < download->state)
			) {
				highestDownload = download;
			}
			if((!downloadNeeded || *downloadNeeded) && (!ready || *ready)) {
				// we have both set
				break;
			}
		} while((download = download->next) != NULL);
		if((!downloadNeeded || *downloadNeeded) && (!ready || *ready)) {
			break;
    }
  }

	if(downloadNeeded) {
		//if(Q_stristr(highest->downloadName, "icona_grenade.jpeg")) {
		//	assert(highest->state <= VFS_NOENT);
		//}
		if(highestDownload && highestDownload->state >= VFS_NOW) {
			//cmd_lazy = 1;
		} else {
			cmd_lazy = 0;
		}
		*downloadNeeded = highestDownload;
	}
}


static qboolean printDebug = qfalse;


static downloadLazy_t *Sys_FileNeeded_real(const char *filename, int state, qboolean isDirectoryList) {
  unsigned int hash;
	const char *loading;
	const char *s;
  downloadLazy_t *download;
	int downloadSize;
  downloadLazy_t *found = NULL;
	downloadLazy_t *parentDownload = NULL;
	qboolean isRoot = qfalse;
	char localName[MAX_OSPATH];
	const char *dir;
	int lengthGame = strlen(FS_GetCurrentGameDir());
	int newState = state;
	localName[0] = '\0';

	assert(state < VFS_DL);

	// skip leading slashes
	while ( *filename == '/' || *filename == '\\' )
		filename++;

	if(Q_stricmpn(filename, FS_GetCurrentGameDir(), lengthGame)) {
		strcat(localName, FS_GetCurrentGameDir());
		strcat(localName, "/");
	}
	strcat(localName, filename);
	if(localName[lengthGame] == '\0') {
		localName[lengthGame + 1] = '\0'; // because down below we use lengthGame + 1
	}
	// remove the last slash just for hashing in case it's a directory, but we don't know it yet
	if(localName[strlen(localName)-1] == '/') {
		state = VFS_INDEX;
		localName[strlen(localName)-1] = '\0';
	}


#ifdef _DEBUG
if(NEEDED[0] != '\0' && Q_stristr(localName, NEEDED)) {
printDebug = qtrue;
}
#endif


	// try to find the directory first, treat pk3dir like a second root
	if((dir = strrchr(localName, '/')) != NULL && !Q_stristr(dir, ".pk3dir")) {
		char lastDirectory[MAX_OSPATH];
		Q_strncpyz(lastDirectory, localName, (dir - localName) + 1);
		parentDownload = Sys_FileNeeded_real(lastDirectory, VFS_INDEX, isDirectoryList);
	} else {
		isRoot = qtrue;
	}

	// force indexes to get downloaded first in line
	// check index need to download
	loading = &asyncShaderName[0];
	if(!asyncShaderName[0]) {
		loading = &asyncSoundName[0];
		if(!asyncSoundName[0]) {
			loading = &asyncModelName[0];
			if(!asyncModelName[0]) {
				loading = localName;
			}
		}
	}

	// remove pk3dir from localName so we can find it again easily
	if((s = Q_stristr(localName, ".pk3dir/")) != NULL) {
		hash = FS_HashPK3( s + 8 );
		lengthGame = (s - localName) + 7;
	} else {
		// check pk3dir paths
		if(state == VFS_NOW) {
			char realPath[MAX_OSPATH];
			char			**pakdirs;
			int				pakdirsi = 0;
			int       numdirs;
			int       len;
			Com_sprintf( realPath, sizeof( realPath ), "%s/%s", 
				Cvar_VariableString("fs_homepath"), FS_GetCurrentGameDir() );
			// TODO: remove this shit and get it from searchpaths
			pakdirs = Sys_ListFiles( realPath, "/", NULL, &numdirs, qfalse );
			while (pakdirsi < numdirs) 
			{
				len = strlen(pakdirs[pakdirsi]);
				if (len && !Q_stricmpn(&pakdirs[pakdirsi][len - 7], ".pk3dir", 7)) {
					char pk3dirName[MAX_OSPATH];
					pk3dirName[0] = '\0';
					strcat(pk3dirName, pakdirs[pakdirsi]);
					strcat(pk3dirName, "/");
					strcat(pk3dirName, &localName[lengthGame + 1]);
					Sys_FileNeeded_real(pk3dirName, state, isDirectoryList);
				}
				pakdirsi++;
			}
		}
		hash = FS_HashPK3( &localName[lengthGame + 1] );
	}


	// absolute path to download item using hash of relative path
	if(readyCallback[hash]) {
		download = readyCallback[hash];
		do {
			if ( !Q_stricmp( localName, download->downloadName ) ) {
				found = download;
				if(loading[12] == ';')
					strcpy(download->loadingName, loading);
				if(download->state > VFS_NOENT && download->state < state) {
					download->state = state;
//Com_Printf("upgrading! %i, %i - %s, %s\n", hash, state, download->downloadName, loading);
				}
				// break; // used to stop here but since we need to check multiple paths
			} //else 
			// make this exception here, basically load from a pk3dir that has been added as a file but
			//   not yet added as a pk3dir to searchpaths. This gives searchpaths a starting point when
			//   the BSP is downloaded the pk3dir will be added and all subsequent requests will go through
			//   multiple root directory checks
			if ((s = Q_stristr(download->downloadName, ".pk3dir/")) != NULL 
				&& !Q_stricmp( &localName[lengthGame + 1], s + 8 )) {
				// fix shader name because the indexer will add files because they are loaded as shaders
				//   and this path tells the renderer which shader to update that loaded the image file
				if(loading[12] == ';')
					strcpy(download->loadingName, loading);
				// escalate download state but never go backwards from FAILED to DOWNLOADING
				if(download->state > VFS_NOENT && download->state < state) {
					download->state = state;
//Com_Printf("upgrading! %i, %i - %s, %s\n", hash, state, download->downloadName, loading);
				} else if (download->state == VFS_NOW) {
					// upgrade if a regular map request happened and the maplist arrives after
					newState = MAX(VFS_NOW, newState);
				}
			}

		} while ((download = download->next) != NULL);
	}

	if(found && found->state > VFS_NOENT) {
		found->state = MAX(found->state, newState);
	}


#ifdef _DEBUG
if(printDebug) {
Com_Printf("file testing! %i, %i - %s, %s\n", hash, state, localName, loading);
}
if(NEEDED[0] != '\0' && Q_stristr(localName, NEEDED)) {
printDebug = qfalse;
}
#endif


	if(!isRoot && !parentDownload && !isDirectoryList && state < VFS_INDEX) {
		if(printDebug) {
			Com_Printf("missing parent! %i, %i - %s, %s\n", hash, state, localName, loading);
		}
		// no parent? no children
		return found;
	}

	if(parentDownload && !isDirectoryList && (parentDownload->state == VFS_FAIL
		|| parentDownload->state <= VFS_NOENT)) {
		// parent is done, !partial, so no more adding files
		if(printDebug) {
			Com_Printf("parent failed! %i, %i - %s, %s\n", hash, state, localName, loading);
		}
		return found;
	}

	if(found) {
#if 0 //defined(USE_LIVE_RELOAD) || defined(__WASM__)
		// allow redownloading of this file because when it's received the game updates
		if(/* !download->failed && */ Q_stristr(localName, "version.json")) {
			download->state = VFS_NOW;
			// add 1500 millis to whatever requested it a second time
			download->lastRequested = Sys_Milliseconds(); 
		}

		if( download->state == VFS_FAIL && download->lastRequested < lastVersionTime) {
			download->state = state;
			download->lastRequested = Sys_Milliseconds(); 
		}
#endif
		if(printDebug) {
			Com_Printf("found! %i, %i - %s, %s\n", hash, state, localName, loading);
		}
		return found; // skip debug message below, that's all
	}

	if(parentDownload && parentDownload->state == VFS_DONE) {
		// parent is done, so don't add files
		if(!isDirectoryList) {
			// only accept additions from the subsequent MakeDirectoryBuffer(), none from FS_* calls
			if(printDebug) {
				Com_Printf("already done! %i, %i - %s, %s\n", hash, state, localName, loading);
			}
			return NULL;
		}
	}

Com_DPrintf("file needed! %i, %i - %s, %s\n", hash, state, localName, loading);

	downloadSize = sizeof(downloadLazy_t)
			+ MAX_OSPATH * 2 /* because it's replaced with temp download name strlen(loading) + 1 */ 
			+ strlen(localName) + 8;
	download = (downloadLazy_t *)Z_Malloc(downloadSize);
	memset(download, 0, downloadSize);
	download->loadingName = (void *)(download + 1);
	download->downloadName = download->loadingName + MAX_OSPATH * 2;
	strcpy(download->loadingName, loading);
	strcpy(download->downloadName, localName);
	download->loadingName[strlen(loading)] = '\0';
	download->downloadName[strlen(localName)] = '\0';
	download->next = readyCallback[hash];
	download->state = newState;
	readyCallback[hash] = download;
	download->lastRequested = 0; // request immediately, updated after first request

#if _DEBUG
	// why is it repeating gamedir?
	if(!stupidPathError[0]) {
		strcat(stupidPathError, FS_GetCurrentGameDir());
		strcat(stupidPathError, "/");
		strcat(stupidPathError, FS_GetCurrentGameDir());
	}

	assert(!Q_stristr(localName, stupidPathError));
#endif

	return download;
}

void Sys_FileNeeded(const char *filename, int state) {
	Sys_FileNeeded_real(filename, state, qfalse);
}


static void ParseJSONFileList(char *buf, int len, char *list, int *count, int *max) {
	// count <tr/<li/<td/<ol/<ul/<div/<h
	//   until default.cfg is found, then add all the detected file names
	//   to the pk3cache.dat file. This will make it easy for the server
	//   provided q3cache.dat to checked at the same time.
	char link[MAX_OSPATH];
	int lenLink = 0;
	int length = strlen(FS_GetCurrentGameDir());
	int c = 0;
	qboolean insideAnchor = qfalse;
	qboolean insideHref = qfalse;
	if(max) *max = 0;
	if(count) *count = 0;
	while(c < len) {
		if(!insideAnchor && buf[c] == '"') {
			c++;

			// tags
			if(buf[c] == 'n' && buf[c+1] == 'a' && buf[c+2] == 'm' && buf[c+3] == 'e' && buf[c+4] == '"') {
				insideAnchor = qtrue;
				c += 5;
			}
		}

		if(!insideHref && buf[c] == ',') {
			insideAnchor = qfalse;
			insideHref = qfalse;
			// TODO: parse poorly formed links?
		}

		// attributes
		if(insideAnchor && !insideHref && buf[c] == '"') {
			c++;
			insideHref = qtrue;
			memset(link, 0, sizeof(link));
			lenLink = 0;
		}

		if(insideHref) {
			if(buf[c] == '"') {
				const char *s;
				insideHref = qfalse;
				link[lenLink] = '\0';
				if((s = Q_stristr(link, FS_GetCurrentGameDir()))
					&& lenLink > length + 2) {
					if((*count) < 1024)
						strcpy(list + (*max), s);
					if(count)
						(*count)++;
					if(max)
						(*max) += strlen(s) + 1;
				}
			} else {
				link[lenLink] = buf[c];
				lenLink++;
			}
		}

		c++;
	}
}


static void ParseHTMLFileList(char *buf, int len, char *list, int *count, int *max) {
	// count <tr/<li/<td/<ol/<ul/<div/<h
	//   until default.cfg is found, then add all the detected file names
	//   to the pk3cache.dat file. This will make it easy for the server
	//   provided q3cache.dat to checked at the same time.
	char link[MAX_OSPATH];
	int lenLink = 0;
	int length = strlen(FS_GetCurrentGameDir());
	int c = 0;
	qboolean insideAnchor = qfalse;
	qboolean insideHref = qfalse;
	if(max) *max = 0;
	if(count) *count = 0;
	while(c < len) {
		if(buf[c] == '<') {
			c++;

			// tags
			if(buf[c] == 'a' && buf[c+1] == ' ') {
				insideAnchor = qtrue;
				c++;
			}
		}

		if(buf[c] == '>') {
			insideAnchor = qfalse;
			insideHref = qfalse;
			// TODO: parse poorly formed links?
		}

		// attributes
		if(insideAnchor
			&& buf[c] == 'h' && buf[c+1] == 'r' && buf[c+2] == 'e' 
			&& buf[c+3] == 'f' && buf[c+4] == '=') {
			c += 5;
			if(buf[c] == '"') {
				c++;
			}
			insideHref = qtrue;
			lenLink = 0;
			memset(link, 0, sizeof(link));
		}

		if(insideHref) {
			if(buf[c] == '"') {
				insideHref = qfalse;
				link[lenLink] = '\0';
				if(Q_stristr(link, FS_GetCurrentGameDir())
					&& lenLink > length + 2) {
					if((*count) < 1024)
						strcpy(list + (*max), link);
					if(count)
						(*count)++;
					if(max)
						(*max) += strlen(link) + 1;
				}
			} else {
				link[lenLink] = buf[c];
				lenLink++;
			}
		}

		c++;
	}
}


// basically just storing a virtual path system
//   in addition to the files that are already there
// when a valid but not yet existing file is request, queue it 
//   for download and indicate to the renderer/client that it is valid
//   and will be update when it arrives (i.e. returns 1 for a file length)
// TODO: add header longs for the new file-based checksum system

void MakeDirectoryBuffer(char *paths, int count, int length, const char *parentDirectory, qboolean incomplete) {
	const char *s;
	long hash;
  downloadLazy_t *download;
  downloadLazy_t *defaultCfg = NULL;
	int lengthDir = strlen(parentDirectory);

	// when a directory index is received, remove any files from the download list
	//   that aren't in the now "known" directory listing
	// mark all as failed
	if(!incomplete) {
		for(int i = 0; i < PK3_HASH_SIZE; i++) {
			download = readyCallback[i];
			if(!download) {
				continue;
			}

			do {
#ifdef _DEBUG
				//if(!Q_stristr(download->downloadName, ".pk3dir/")
				//	&& Q_stristr(download->downloadName, "default.cfg")) {
				//	defaultCfg = download;
				//}
				//if(!Q_stristr(download->downloadName, ".pk3dir/")
				//	&& Q_stristr(download->downloadName, "vm/ui.qvm")) {
				//	defaultCfg = download;
				//}
				//if(Q_stristr(download->downloadName, "palette.shader")) {
				//	defaultCfg = download;
				//}
				if(NEEDED[0] != '\0' 
					&& !Q_stristr(download->downloadName, ".pk3dir/")
					&& Q_stristr(download->downloadName, NEEDED)) {
					defaultCfg = download;
				}
				//if(Q_stristr(parentDirectory, "icons")
				//	&& Q_stristr(download->downloadName, "icona_grenade.jpeg")) {
				//	defaultCfg = download;
				//}
#endif
				if(incomplete && !Q_stricmp( download->downloadName, parentDirectory )) {
					download->state = VFS_PARTIAL;
				}

				if(download->state > VFS_NOENT && download->state < VFS_DL
					// don't purge indexes this way
					&& download->state != VFS_INDEX
					// reset file states for all files in the directory just received
					&& !Q_stricmpn(download->downloadName, parentDirectory, lengthDir)
					&& download->downloadName[lengthDir] == '/'
					// last folder in the path
					&& strchr(&download->downloadName[lengthDir + 1], '/') == NULL 
				) {
//if(Q_stristr(parentDirectory, "menu/art"))
//Com_Printf("purging: %s\n", download->downloadName);
					download->state = VFS_NOENT - download->state;
				} else {
//Com_Printf("skipping: %s != %s\n", download->downloadName, parentDirectory);
				}
			} while ((download = download->next) != NULL);
		}
	}

	// insert into the hash table
	char *currentPath = paths;
	for ( int i = 0; i < count; i++ )
	{
		// remove game dir
		if(Q_stristr(currentPath, FS_GetCurrentGameDir())) {
			currentPath += strlen(FS_GetCurrentGameDir()) + 1;
		}

		// remove leading /
		if(currentPath[0] == '/') {
			currentPath++;
		}

		if(strlen(currentPath) == 0) {
			currentPath++;
			continue;
		}

#ifdef USE_LIVE_RELOAD

		// FIXME: move mtime to Sys_FileNeeded_real()
		if(FS_FileExists(currentPath)) {
			//fileOffset_t _s;
			//fileTime_t _c;
			// TODO: FS_RealPath() so pk3 files can be bundled as updates
			//Sys_GetFileStats(FS_BuildOSPath(fs_homepath->string, FS_GetCurrentGameDir(), currentPath),
			//	&_s, &curFile->mtime, &_c);
		} else {
			// set mtime to at least when the engine starts up
			//curFile->mtime = 0;
		}
#endif
		// store the file position in the zip
		// TODO: update this when Accept-Ranges is implemented
		s = Q_stristr(currentPath, ".pk3dir/");
		if(s) {
			hash = FS_HashPK3( s + 8 );
		} else {
			hash = FS_HashPK3( currentPath );
		}

		// download more directories recursively, at least for like 2 levels
		if(qfalse
			//!strrchr(currentPath, '.')
			//&& (Q_stristr(currentPath, "/maps")
			//|| Q_stristr(currentPath, "/vm")
			|| Q_stristr(currentPath, "botfiles/")
			|| !FS_GeneralRef(currentPath)
		) {
			// TODO: fix this, only works if directory listing allows slashes at the end?
			// TODO: right thing to do would be test the content-type and duck out early 
			//   on big files for anything that doesn't have a . dot in the name.
			download = Sys_FileNeeded_real(currentPath, VFS_NOW, qtrue);
		} else {
			download = Sys_FileNeeded_real(currentPath, VFS_LATER, qtrue);
		}

		assert(download != NULL);

		// then it was already in the list which means we intended to download now,
		if(download->state < VFS_NOENT) {
			// and now we know it's there because its in the directory listing
			download->state = -download->state + VFS_NOENT;
//if(Q_stristr(parentDirectory, "weapons2/bfg")) {
//if(Q_stristr(parentDirectory, "icons")) {
//Com_Printf("keeping %li, %s\n", hash, currentPath);
//}
		} else {
//if(Q_stristr(parentDirectory, "lsdm3_v1.pk3dir/maps")) {
//if(Q_stristr(parentDirectory, "icons")) {
//Com_Printf("adding %li, %s\n", hash, currentPath);
//}
		}

		currentPath += strlen( currentPath ) + 1;
	}

	// default.cfg purged from directory listing
	//assert(!defaultCfg || defaultCfg->state > VFS_NOENT);
	assert(!defaultCfg || defaultCfg->state <= VFS_NOENT);

}


#ifdef __WASM__
Q_EXPORT
#endif
void Sys_FileReady(const char *filename, const char* tempname) {
  downloadLazy_t *download;
	unsigned int hash;
	char localName[ MAX_OSPATH ];
	const char *s;
	int length;
	qboolean found = qfalse;
	qboolean isDirectory = filename[strlen(filename) - 1] == '/';
	int lengthGame = strlen(FS_GetCurrentGameDir());
	memset(localName, 0, sizeof(localName));

	// skip leading slashes
	while ( *filename == '/' || *filename == '\\' ) {
		filename++;
	}

	// mark the correct file as ready
	if(!Q_stricmpn(filename, FS_GetCurrentGameDir(), lengthGame)) {
		// special exception because this is the only file we download outside the gamedir
		// TODO: make a special exception for updating the EXE from Github?
		Com_sprintf(localName, sizeof(localName), "%s", filename);
	} else {
		Com_sprintf(localName, sizeof(localName), "%s/%s", FS_GetCurrentGameDir(), filename);
	}

	if(localName[lengthGame + 1] == '\0') {
		isDirectory = qtrue;
	}

	if(isDirectory) {
		localName[strlen(localName) - 1] = '\0';
	}

	// remove pk3dir from file hash
	if((s = Q_stristr(localName, ".pk3dir/")) != NULL) {
		hash = FS_HashPK3( s + 8 );
		lengthGame = (s - localName) + 7;
	} else {
		hash = FS_HashPK3( &localName[lengthGame + 1] );
	}

	length = strlen(localName);

	// mark the file as downloaded
	for(int i = 0; i < PK3_HASH_SIZE; i++) {
		download = readyCallback[i];
    if(!download) {
			continue;
		}

		do {
			s = Q_stristr(download->downloadName, ".pk3dir/");
			// find an exact match
			if( i == hash && !Q_stricmp( download->downloadName, localName ) ) {
				found = qtrue;
				if(tempname) {
					download->state = VFS_READY;
					strcpy(&download->loadingName[MAX_OSPATH], tempname);
				} else {
					download->state = VFS_FAIL;
				}
				break;
			} else 

#if 0
			if ( i == hash 
				// ignore pk3dir and mark all as ready
				&& ((!s && !Q_stricmp( &download->downloadName[lengthGame + 1], &localName[lengthGame + 1] ))
					|| (s && !Q_stricmp( s + 8, &localName[lengthGame + 1] )))
			) {
				// file is ready for processing!
				if(tempname) {
					download->state = VFS_READY;
					strcpy(&download->loadingName[MAX_OSPATH], tempname);
#ifdef USE_LIVE_RELOAD
					// update file mtime so we know if it changed again
					download->lastRequested = lastVersionTime;
#endif
				} else {
					// download failed!
					download->state = VFS_FAIL;
					Com_Printf("WARNING: %i %s download failed.\n", hash, localName);
				}
				found = qtrue;
				// break; // used to stop here, but now this will cover .pk3dir paths
			} else 
#endif

			// really reduce misfires by removing missing download files once directory index is received
			//   we know we won't find shader images with alternate extensions, so those downloads are marked
			if( isDirectory && !tempname 
				// match the first part of the path
				&& !Q_stricmpn( download->downloadName, localName, length )
				&& download->downloadName[length] == '/'
				// last folder in the path
				// if parent path doesn't exist, neither do children
				//&& strchr(&download->downloadName[length + 1], '/') == NULL 
			) {
//Com_Printf("purging 2: %s - %s\n", download->downloadName, filename);
				if(download->state > VFS_NOENT) {
					download->state = VFS_NOENT - download->state;
				}
			}
		} while ((download = download->next) != NULL);
	}

	if(!found && tempname) {
		// should never happen!
		Com_Error(ERR_FATAL, "WARNING: %i %s not found in download list.\n", hash, localName);
	}

}


void FS_UpdateFiles(const char *filename, const char *tempname) {
Com_Printf("updating files: %s -> %s\n", filename, tempname);

	// do some extra processing, restart UI if default.cfg is found
	if(Q_stristr(tempname, "default.cfg")) {
		// will restart automatically from NextDownload()
		if(!FS_Initialized()) {
			FS_Restart(0);
		}
		// TODO: check on networking, shaderlist, anything else we skipped, etc again
		com_fullyInitialized = qtrue;
		Cbuf_AddText("exec default.cfg; vid_restart lazy;");
	} else 
	
#if defined(USE_LIVE_RELOAD) || defined(__WASM__)
	if(Q_stristr(tempname, "version.json")) {
		int y, mo, d, h, m, s;
		struct tm tmi;
		int approxTime = 0;
		int versionLength;
		const char *versionFile;
		versionLength = FS_ReadFile("version.json", (void **)&versionFile);
		if (versionLength < 1)
			return;
		const char *serverTime = JSON_ArrayGetValue(versionFile, versionFile + versionLength, 0);
		// convert string to approx timestamp, skip quote
		if ( sscanf(serverTime + 1, "%i-%i-%iT%i:%i:%i.", &y, &mo, &d, &h, &m, &s) ) {
			tmi.tm_year = y - 1900;
			tmi.tm_mon = mo - 1;
			tmi.tm_mday = d;
			tmi.tm_hour = h;
			tmi.tm_min = m;
			tmi.tm_sec = s;
			approxTime = mktime(&tmi);
			if (lastVersionTime == 0 || approxTime > lastVersionTime) {
				// TODO: some kind of reloading action?
				// clear out any failed states so they can be rechecked
				lastVersionTime = approxTime;
				// version was supposed to describe what changed in each index, 
				//   0 - main program, 1 - asset files, 2 - scripts, 3 - sounds, 4 - qvms?
				//Cbuf_AddText("wait; vid_restart lazy;");
			}
		}
	} else
#endif

	if(Q_stristr(tempname, ".bsp") && Q_stristr(tempname, "maps/")) {
		// load map if we just tried to start it
		if(Q_stristr(tempname, ".pk3dir/")) {
			FS_AddGameDirectory( Cvar_VariableString("fs_homepath"), FS_GetCurrentGameDir(), 0 );
		}
		if(Q_stristr(tempname, Cvar_VariableString("mapname")) && !com_sv_running->integer) {
			Cbuf_AddText(va("wait lazy; devmap %s;", Cvar_VariableString("mapname")));
		}
	} else 

	// TODO:
	if(Q_stristr(tempname, "description.txt")) {
		// refresh mod list
	} else 

#ifndef DEDICATED
	// try to reload UI with current game if needed
	if(Q_stristr(tempname, "vm/ui.qvm") && !cls.uiStarted) {
    Cvar_Set("com_skipLoadUI", "0");
		//Cbuf_AddText("wait; vid_restart lazy;");
		CL_StartHunkUsers();
	} else 

	// TODO: load default model and current player model
	if(Q_stristr(tempname, "vm/cgame.qvm") && !cls.cgameStarted) {
		CL_StartHunkUsers();
	} else 

#endif

#ifndef BUILD_SLIM_CLIENT
	// try to reload UI with current game if needed
	if(Q_stristr(tempname, "vm/qagame.qvm") && !com_sv_running->integer) {
		if(Q_stricmp(Cvar_VariableString("mapname"), "nomap")) {
			Cbuf_AddText(va("wait lazy; devmap %s;", Cvar_VariableString("mapname")));
		}
	} else 
#endif

	// scan index files for HTTP directories and add links to q3cache.dat
	if ((Q_stristr(tempname, ".tmp") && Q_stristr(tempname, "/."))
		|| Q_stristr(tempname, "maps/maplist.json")) {
		// left in the temp directory, must be an index file
		char realPath[MAX_OSPATH];
		const char *s;
		memset(realPath, 0, sizeof(realPath));
		s = strchr( tempname, '/' );
		if(s) {
			s++;
		} else {
			s = tempname;
		}
		Com_sprintf( realPath, sizeof( realPath ), "%s/%s", 
			Cvar_VariableString("fs_homepath"), tempname );

//Com_Printf("Adding %s (from: %s) to directory index.\n", filename, realPath);

		// don't use FS_* here because that would create a loop
		FILE *indexFile = Sys_FOpen(realPath, "rb");
		int len = FS_FileLength(indexFile);
		if(len < 1024 * 1024 * 50) {
			char paths[1024 * MAX_OSPATH];
			int count = 0, nameLength = 0;
			memset(paths, 0, sizeof(paths));
			char *buf = (char *)Z_TagMalloc(len + 1, TAG_PACK);
			fread(buf, len, 1, indexFile);
			buf[len] = '\0';
			if(buf[0] == '{' || buf[0] == '[') {
				ParseJSONFileList(buf, len, paths, &count, &nameLength);
			} else {
				ParseHTMLFileList(buf, len, paths, &count, &nameLength);
			}
			// can't purge missing files from an incomplete file index
			MakeDirectoryBuffer(paths, count, nameLength, filename, !!Q_stristr(buf, "nextPageToken"));
			// TODO: add next page token every time it's requested as if we're actually paging too
			Z_Free(buf);
		}
		fclose(indexFile);
		FS_HomeRemove( s );
		// we should have a directory index by now to check for VMs and files we need
		if(Q_stristr(tempname, "maps/maplist.json")
			&& Q_stricmp(Cvar_VariableString("mapname"), "nomap")) {
			const char *mapname = Cvar_VariableString("mapname");
			Sys_FileNeeded_real(va("maps/%s.bsp", mapname), VFS_NOW, qtrue);
			// force download of map shaders from specific pk3dir
			Sys_FileNeeded_real(va("scripts/%s.shader", mapname), VFS_NOW, qtrue);
		} else {
			//Cbuf_AddText("wait; vid_restart lazy;");
		}
	}
}


static qboolean first = qfalse;
void CL_CheckLazyUpdates( void ) {
	downloadLazy_t *downloadNeeded;
	downloadLazy_t *ready;
	int newTime = Sys_Milliseconds();

	// TODO: remove this, good to see 1 color during testing
	if(!first) {
		first = qtrue;
		Sys_FileNeeded("multigame/lsdm3_v1.pk3dir/textures/lsdm3/nunuk-bluedark.jpg", VFS_LAZY);
	}

	// files must process faster otherwise we will have a bunch of indexes queued  
	//   while downloading errors are happening
	if(newTime - secondTimer > 9) { // = 10ms, or about once per frame at 100 FPS
		secondTimer = newTime;
	}

	if(newTime - thirdTimer > 10) { 
		thirdTimer = newTime;
	}
	
	if(secondTimer < newTime && thirdTimer < newTime) {
		return;
	}

	downloadNeeded = NULL;
	ready = NULL;
	// lazy load only if the screen it out of view in multiworld
#ifdef USE_LAZY_LOAD
	if(cl_lazyLoad->integer <= 2 || cls.lazyLoading)
#endif

	// always returns the first download requested by FS_*
	Sys_UpdateNeeded(
		secondTimer == newTime ? &ready : NULL, 
		thirdTimer == newTime ? &downloadNeeded : NULL);

	// check for files that need to be downloaded, runs on separate thread!?
	if(thirdTimer == newTime && downloadNeeded) {

#if defined(USE_CURL) || defined(__WASM__)
		// we don't care if the USE_ASYNCHRONOUS code in the call cancels 
		//   because it is requeued 1.5 seconds later
		if(CL_Download( "lazydl", downloadNeeded->state == VFS_INDEX 
			? va("%s/", downloadNeeded->downloadName) 
			: downloadNeeded->downloadName, qfalse )
		) {
			downloadNeeded->lastRequested = newTime;
			downloadNeeded->state = VFS_DL;
		}
#endif
	}

	// if we break here, nothing will update while download is in progress
	if(secondTimer != newTime || !ready) {
		return;
	}

#ifdef USE_LAZY_LOAD
	const char *ext = COM_GetExtension(ready->downloadName);
	if(!Q_stricmp(ext, "md3") || !Q_stricmp(ext, "mdr")
		|| !Q_stricmp(ext, "md5")) {
		if(cls.rendererStarted) {
			re.UpdateModel(ready->loadingName);
			secondTimer += 10;
		}
	}

	if(!Q_stricmp(ext, "wav") || !Q_stricmp(ext, "ogg")
		|| !Q_stricmp(ext, "mp3") || !Q_stricmp(ext, "opus")) {
		if(cls.soundRegistered) {
			//S_UpdateSound(ready->loadingName, qtrue);
		}
	}

	if(ready->loadingName[12] == ';') {
		ready->loadingName[12] = '\0';
		if(cls.rendererStarted) {
			re.UpdateShader(&ready->loadingName[13], atoi(&ready->loadingName[0]));
			secondTimer += 10;
		}
		ready->loadingName[12] = ';';
	}

	// intercept this here because it's client only code
	if(Q_stristr(&ready->loadingName[MAX_OSPATH], "/scripts/")
		&& Q_stristr(&ready->loadingName[MAX_OSPATH], ".shader")) {
		re.ReloadShaders(qfalse);
	}
#endif

	ready->loadingName[MAX_OSPATH - 1] = '\0';
	FS_UpdateFiles(ready->downloadName, &ready->loadingName[MAX_OSPATH]);

#if defined(USE_ASYNCHRONOUS) || defined(__WASM__)
	// multigame has a special feature to reload an missing assets when INIT is called
	if(((cls.uiStarted && uivm) || (cls.cgameStarted && cgvm))
		&& !Q_stricmp(FS_GetCurrentGameDir(), "multigame")) {
		cl_lazyLoad->modificationCount++;
		cl_lazyLoad->string = ready->downloadName;
	} else {
		cl_lazyLoad->string = "";
	}
#endif

	// something updated, that's good for this frame
}


#endif
