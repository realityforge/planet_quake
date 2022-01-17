


## Client Features

  * Rcon auto-complete, sends a `complete` command to server and response with an `autocomplete` key in an `infoResponse` which is an easy way to intercept messages without adding a command.

This fork includes various experimental features. Most of them can be turned on and off during compiling because the feature is tagged with pre-compile `#if` statements to include the feature.
This is an incomplete list of features added.
Use `USE_FEATURE=1` with make to enable.

### USE_CVAR_UNCHEAT
Uncheat, removed the `CVAR_CHEAT` flag from specific cvars, then shares those cvars with the server for blocking.

### USE_LOCAL_DED
start a dedicated server even for single player mode, automatically join a match

### USE_LAZY_LOAD
allow loading graphics after the BSP and world has been entered

### USE_LAZY_MEMORY
minimize the number of times the renderer restarts

### USE_MASTER_LAN
Heavily modified "Local" multiplayer page that lists specific masters server using `cl_master1-24` as opposed to `sv_master1-24` like on the "Internet" page of the multiplayer menu, if admins want to list servers by geographically nearby.

### USE_MV
Cyrax's Multiview is what makes multiworld possible.
USE_MV_ZCMD - command compression

### USE_RMLUI
RmlUi adds supplementary initerfaces written in HTML.

### USE_CURSOR_SPY
spy on the cursors position for absolute mouse control

### USE_DRAGDROP
allow files like pk3s to be dragged into the client for easy loading

### USE_PRINT_CONSOLE
use pre-compile templates to filter annoying debug messages

### USE_NO_CONSOLE
remove console drop down in game functionality altogether

### USE_PERSIST_CONSOLE
Persist console messages between games and also between launches

### USE_LIVE_RELOAD
automatically reload game when the QVM changes from a new compile

### USE_DIDYOUMEAN
show did you mean? results for map names, cvars, command names, etc

### BUILD_CLIENT_SLIM
Slim client without extra file formats or server, only for connecting to games or rendering demos.


## Multigame

Multigame is a mod I've been working on that adds Runes and alternate fire modes and stuff, [games.md](../docs/games.md#game-features).


## TODOs

  * Fix defaults and limitations by looking at other engine forks, raise minimums like sv_fps, com_maxfps, snaps, etc, call the build quake3e-unlocked in Github Actions.
  * Fix Xonotic map shaders by ignoring an entire line after an unsupported parameter?
  * LOD (level of detail) based compression, loading different levels of detail in models and shaders, distance based mipmaps
  * Socks5 based cUrl downloads for downloading over the proxy and avoid content access controls
  * Brotli compression for game content from server UDP downloads
  * .Gif support with automatic frame binding in animMap
  * Ported IQM and MD5 bone structures from spearmint engine
  * Many BSP formats (Quake 1, Quake 2, Quake 4, Doom 1, Doom 2, Doom 3, Hexen maps) support and cross compatibility with other game content like Call of Duty, Half-Life, and Savage XR
  * move console background image API to server-side with console instead of mod side like in e+/freon.
  * Fix r_fbo and add pixel buffer objects for recordings. Send PBO to a worker thread to encode to VPX-wasm for live streaming.
  * Something wrong with r_fbo causing driver crash. Need it for hall of mirrors effect for portals.
  * Make all game code entry points asynchronous with engine to run all QVMs in web workers/pthreads. Pause VM for async calls between engine and VM.
  * Allow different VMs on file system by using more search directories and Pure server calls for filtering.
  * Set fog with a foglevel setting instead of using brushes?
