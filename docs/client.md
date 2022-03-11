


## Client Features

This fork includes various experimental features. Most of them can be turned on and off during compiling because the feature is tagged with pre-compile `#if` statements to include the feature.
This is an incomplete list of features added.
Use `USE_FEATURE=1` with make to enable.

### USE_CVAR_UNCHEAT
Uncheat, removed the `CVAR_CHEAT` flag from specific cvars, then shares those cvars with the server for blocking.

### USE_LOCAL_DED
Start a dedicated server for single player mode in a seperate process, automatically join the match. This mod has communication changes, the server can send any command to the local client to execute, server info can be updated mid-game, single player functionality in the dedicated server setting, benefit of splitting math work between processors.

Rcon auto-complete, sends a `complete` command to server and response with an `autocomplete` key in an `infoResponse` which is an easy way to intercept messages without adding a command.

### USE_LAZY_LOAD
Lazy loading was implemented because graphics over web are traditionally loaded into the page asynchronously. The main principle is loading graphics in between frames from very low res sources. Read more about it here, [lazyloading.md](../docs/lazyloading.md).

[![Lazy](https://img.youtube.com/vi/lkp0A68ygBQ/0.jpg)](https://www.youtube.com/watch?v=lkp0A68ygBQ
)

### USE_LAZY_MEMORY
Similar to lazy loading, lazy memory reduces the number of times the renderer restarts. Instead of restarting the renderer every time the menu loads or the map changes, all the graphics are left in memory and slowly rotated out of memory as they are no longer used. This takes up more memory, but that's not a problem for most modern computers.

### USE_MASTER_LAN
Heavily modified "Local" multiplayer page that lists specific masters server using `cl_master1-24` as opposed to `sv_master1-24` like on the "Internet" page of the multiplayer menu, if admins want to list servers by geographically nearby.

### USE_MV
Cyrax's Multiview is what makes multiworld possible.
USE_MV_ZCMD - command compression

### USE_RMLUI
RmlUi adds supplementary initerfaces written in HTML.

![F1](../docs/rml.png?raw=true)

### USE_DRAGDROP
Allow files like pk3s to be dragged into the client for easy loading.

### USE_PRINT_CONSOLE
Use pre-compile templates to filter annoying debug messages. TODO: more documentation here

### USE_NO_CONSOLE
Remove console drop down in game functionality altogether.

### USE_PERSIST_CONSOLE
Persist console messages between games and also between launches

### USE_LIVE_RELOAD
Automatically reload game when the QVM changes from a new compile, or restart the process when the engine is recompiled from a code change, or reload the map when the .map file is saved.

### USE_DIDYOUMEAN
Show "Did you mean?" results for mis-typed map names, cvars, command names, etc.

### BUILD_SLIM_CLIENT
Slim client without extra file formats or server, only for connecting to games or rendering demos.

### define USE_UNLOCKED_CVARS
Adds a few additional Cvar for MAX_RENDER_COMMANDS, and one major change
  to auto expand the command buffer, and max poly lists.

TODO: The auto expansion is also important for multiworld CMD
  replay features that allow a lot of processing to be cut out when the FPS 
  is lowered for subordinate VMs.

This expansion is slightly larger than the original Quake III Arena size. 
  So assets built are guaranteed to fit within the first memory allocation. 
## Experimental server features

Some server features require changes to the client component and server component. Server features are listed here, [server.md](../docs/server.md).


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
  ![F1](../docs/et.png?raw=true)
  * move console background image API to server-side with console instead of mod side like in e+/freon.
  * Fix r_fbo and add pixel buffer objects for recordings. Send PBO to a worker thread to encode to VPX-wasm for live streaming.
  * Something wrong with r_fbo causing driver crash. Need it for hall of mirrors effect for portals.
  * Make all game code entry points asynchronous with engine to run all QVMs in web workers/pthreads. Pause VM for async calls between engine and VM.
  * Allow different VMs on file system by using more search directories and Pure server calls for filtering.
