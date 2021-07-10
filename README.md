
- [Compilation and installation](#compilation-and-installation)
- [Console](#console)
- [Docker](#docker)
- [Running content server](#running-content-server)
- [Repacking](#repacking)
- [Building](#building)
- [Contributing](#contributing)
- [Credits](#credits)

```
,-------------------------------------------------------------------------------.
 _____  _               _   _ ______ _______    ____  _    _         _  ________ 
|  __ \| |        /\   | \ | |  ____|__   __|  / __ \| |  | |  /\   | |/ /  ____|
| |__) | |       /  \  |  \| | |__     | |    | |  | | |  | | /  \  | ' /| |__   
|  ___/| |      / /\ \ | . ` |  __|    | |    | |  | | |  | |/ /\ \ |  < |  __|  
| |    | |____ / ____ \| |\  | |____   | |    | |__| | |__| / ____ \| . \| |____ 
|_|    |______/_/    \_\_| \_|______|  |_|     \___\_\\____/_/    \_\_|\_\______|

'----------------------------- https://quake.games -----------------------------'
```


This project derives from https://github.com/ec-/Quake3e and https://github.com/inolen/quakejs.
It is not the intention of this project to add features that are not compatible with
either of those two forks or the original Quake 3 source code release by id Software.
The intention of this project is to give developers the tools needed to run Quake 3
in a modern web browser, including Android Mobile Chrome.

QuakeJS is a port of [ioquake3](https://github.com/ioquake/ioq3) to JavaScript with the help of [Emscripten](https://emscripten.org/index.html).

To see a live demo, check out https://quake.games or http://www.quakejs.com

Some of the major features currently implemented are:

  * ...using OpenGL OpenGL ES 3.0 (WebGL 2.0 (OpenGL ES 3.0 Chromium))
  * A working repack script to convert game assets to be more web compatible
  * [NippleJS](https://github.com/yoannmoinet/nipplejs) mobile support
  * A content server (NodeJS + express) to repack and live-reload the game as you develop
  * A SOCKS5 server to convert web-socket connections to UDP and connect to any standard Quake 3 server
  * Various mod disassemblies for associating hard-coded shaders with files even if the mod isn't open source
  * Various graphs of mods, including file names for repacked content
  * PNG support
  * Docker support, TODO: copy kubernetes support https://github.com/criticalstack/quake-kube
  * 2,500+ available maps on [lvlworld.com](https://lvlworld.com) and another 15,000+ planned, `cl_returnURL` for redirecting on quit and disconnect
  * Removed SDL inputs, touch support on mobile works, copy/paste, Drag and drop for sharing game content with the browser. .cfg file uploads/local imports. Uses home directory for drag and drop content. TODO: until it is uploaded and accepted by the package manager.
  * Deferred (lazy) loading of all game content, entities, models, textures. New `cl_lazyLoad` cvar, 0 - turn off lazy loading, only load textures available on disk, 1 - load all textures available and fetch remotely as needed, 2 - set all to default and try to load from `sv_dlURL`, TODO: 3 - load textures only during INTERMISSION or dead or SPECTATING, 4 - set all to default and load during intermission (this is specifically for subordinate VMs in multiVM/multi-render modes). TODO: leave files open for changing mip levels (especially on DDS)? TODO: lazy loading file streaming out of pk3 on server over UDP or cURL interface.
  * Shader palettes for pre-rendering colors and TODO: changing the theme of maps at runtime using `if` in shaders
  * Offline mode for local and LAN games, just visit quake.games and run the command `\offline` in the console to cache all necessary files to local storage. [Google Reference](https://developers.google.com/web/fundamentals/codelabs/offline)
  * Web-worker dedicated local server for mesh networked gaming, game sharing over localized Socks proxy network. TODO: authenticated clients
  * Rcon auto-complete, sends a `complete` command to server and response with an `autocomplete` key in an `infoResponse` which is an easy way to intercept messages without adding a command.
  * Server-side demos, recording for every client, [TheDoctor's method](http://openarena.ws/board/index.php?topic=4437.0). Server-side demos, [lrq3000 implementation](https://github.com/lrq3000/ioq3-server-side-demos) recording entire server state and spectating playback. [Cyrax' multiview protocol](http://edawn-mod.org/forum/viewtopic.php?f=6&t=7) for viewing all clients from one demo file. MultiVM command `\load <ui,cgame,game>; \mvjoin`. `\mvtile` display views in a grid. TODO: playing back dm_68/mv-demo files for all players. TODO: fast forward, rewind 10 seconds using baseline indexes in demo files.
  * Multiple map loader in parallel with teleport switch. Multiple QVM loading for supplemental UIs and multiview. Multiview for movie making, example `+spdevmap q3dm1 +activeAction "+wait 1000 +load cgame +wait +world 0 +tile -1 -1 +tile 0 +tile 1 +tile 0 0 0 +wait 100 +mvjoin"`  Multiple worlds at once `+devmap q3dm1 +wait +load game q3dm2 +set activeAction "+wait 500 +game 1 +wait 500 +game 0 +wait 100 +tile -1 -1 0 +tile 0 0 0 +tile 1 0 1 +tile 0 0 0"`. `dvr [clientnum] x y h w` command for setting a view in a specific location. Multiworld working with bots and sounds, multi-world-demos. Drag and drop for multiQVM views https://www.youtube.com/watch?v=xvmdETvvBo8. TODO: "demoMap" surface parm which renders demos to an arbitrary surface. TODO: use scripting in kiosk mode multiple demo files at once.  TODO: fix SAMEORIGIN spawn type location works but camera angles change. TODO: add filesystem switching mask so multiple mods can be loaded at the same time. TODO: connect to multiple servers at the same time, closer with demo work.
  * Heavily modified "Local" multiplayer page that lists specific masters server using `cl_master1-24` as opposed to `sv_master1-24` like on the "Internet" page of the multiplayer menu, if admins want to list servers by geographically nearby.
  * Lightning Network bitcoin transactions, see `sv_ln*` settings for more information. QR code generation by [Nayuki](https://www.nayuki.io/page/qr-code-generator-library).
  * Admin monitoring of cmd stream, Huffman decoding for proxy, Man-In-The-Middle proof-of-concept.
  * Checksum pk3 spoofing for playing with original clients using web converted files.
  * Referee controls. `freeze/unfreeze` for server side pausing. `lock/lockred/lockblue` to lock team joining.
  * Roles based rcon access. Set up to 24 rcon passwords each with a specific role assigned to it. Set the names of the roles with `sv_roles` (default: admin referee moderator). Role names must be alphanumeric with spaces in between, up to 24 names can be specified. Roles are set with `sv_roleName` where Name one of the role names set in `sv_roles`. Role capabilities are set with the name of the console command, e.g. `ban kick map`. Authorized users can access the entire command, that is, if they have `exec` permissions they can execute ANY script. If you want to limit executions to specific commands or combinations an alias has to be used and the alias would be listed in the `sv_roleName` instead of the command.
  * Event streaming. When specific things happen on the server, rcon can query the list of events and get a JSON style response of what happened since the last time it was checked. E.g. `{"type":1,"value":"q3dm1"}` where type `1` is SV_EVENT_MAPCHANGE. This is also used for the discord integration, and rankings. TODO: piped output for "pushing" events (as opposed to polling). TODO: add location information capable of making a heatmap
  * URL state management for accessing menus and for connecting to a server, i.e. https://quake.games?connect%20address using the [History API pushstate](https://caniuse.com/?search=pushstate)
  * Add player state to UI QVM so multiple cursors can be shared, and players can see map and game configuration.
  * Procedurally generated game content and maps, sv_bsp procedurally generates a map of any size and transfers to the client. Use `+map \*memory0` to see a generated skybox, the source `.map` and generated bsp file are stored in the game home path. TODO: connect virtual fs for image loading, TODO: create voxelized model on server aka "destructible model" and stream to client, replace using z-index? TODO: add checksum to skip download, also minimize download screen. TODO: remove dependency on client side changes by building a temporary pk3 on the server with zlib (or whatever) and allowing legacy clients to download from that. TODO: fix multithreading so console can respond and cancel map generations like a process list/print queue, use this feature for faster map development.
  * Persistent client sessions, that is, saving and restoring client score/health/location after a full disconnect and reconnect, use `sv_clSession` to specify how many seconds a session is valid to be restored.
  * Slim client without extra file formats or server, only for connecting to games or rendering demos.
  * Persistent console.
  * Many, many bug fixes


New game features:

  * Automatically skip to multiplayer menu TODO: and that connects to first available command server that we can request a map command. TODO: Remove the auto-connect code from javascript.
  * Configurable vote options based on roles.
  * Launch a program like opening a web page from a map trigger.
  * More configurable physics.

Coming soon!
  * Make a simple thread manager https://stackoverflow.com/questions/7269709/sending-information-with-a-signal-in-linux
  * IN PROGRESS: removing Emscripten and compiling only to wasm with clang.
  * Move more features like the http downloading, file-system journaling, EULA, file extension alternatives from ListFiles that comes from menu system, etc out of JS and in to C system.
  * Add touch and mobile controls that look like this (https://jayanam.com/ugui-unity-mobile-touch-control/) but are actually generated from brushes and look in the direction they are being pulled like a 3D model like the head. Make all numbers and HUD controls 3D models. Since RMLUI will be simple we can focus on advanced HUD  features like resizing on demand. Player controls for video and demos https://www.google.com/search?q=simple+html+media+player+controls
  * TODO: Stop crash after loading 10 maps with use_lazy_memory. Fix in cm_load, vm_create, renderer2, bots? sound? 
  * (Short term tasks) Stop local server from dropping, kickall bots, quit a server if all human clients disconnect, cl_lazyLoad 2 and 3 for only loading during network downtimes (warmup and between matches and during respawn timeout), add websockets to native dedicated server instead of relying on proxy https://github.com/rohanrhu/cebsocket, run without dedicated server worker in single thread on mobile (bring back r_smp 1 for dedicated server feature, r_smp 2 for renderer features, r_smp 0 for mobile/off), bring back download commands for grabbing new content, pre-download content like the repack graph mode sets up so less is downloaded in game, use lvlworld.cfg and autoconfig after so people can save their settings (need a way to exit "preview mode" and play the game with networking, simple menu items in preview mode "start game" option), move console background image API to server-side with console instead of mod side like in e+/freon.
  * Popup keyboard on mobile, somehow detecting a text box from UIVM, might not work on all mods. 
  * Fix r_fbo and add pixel buffer objects for recordings. Send PBO to a worker thread to encode to VPX-wasm for live streaming.
  * Make all game code entry points asynchronous with engine to run all QVMs in web workers/pthreads. Pause VM for async calls between engine and VM. Allow different VMs on file system by using more search directories and Pure server calls for filtering.
  * Payment API for micropayments to access server or content https://developers.google.com/web/fundamentals/codelabs/payment-request-api
  * SSE/SIMD support in vm_js.js Com_SnapVectors(), https://emscripten.org/docs/porting/simd.html
  * Compile baseq3a from EC directly to WASM and load asynchronously, https://github.com/emscripten-core/emscripten/wiki/Linking
  * Switching renderers to WebGL 1/OpenGL 1/ES 1+2, closer with dlopen work
  * Extra UI menus with multiQVM, for voting on maps and bitcoin setup, Instant replay, consolidate all VM UIs scoreboard/postgame/HUD/menus in to 1 UI system, replace the menu address with an API call.
  * Use q3cache.dat instead of index.json (or manifest.json in quakejs)
  * Download files using offsets out of pk3 files, like streaming a part of the zip file, add this to native dedicated server and UDP downloads, this won't work on Google CDN because there is no accept-ranges support with brotli compression, https://cloud.google.com/storage/docs/xml-api/get-object-download service in front of that needs to re-encode the individual file based on the offset provided with brotli.
  * IN FAILURE: HTML and CSS menu renderer
  * r_smp 2 Software renderer for rendering far distances in a web-worker, WebGL if OffscreenCanvas is available, low resolution software GL is not available
  * Always on twitch.tv streaming at no expense to the game server
  * Socks5 based cUrl downloads for downloading over the proxy and avoid content access controls
  * LOD (level of detail) based compression, loading different levels of detail in models and shaders, distance based mipmaps
  * Brotli compression for game content from server UDP downloads
  * Asynchronous rendering for portals, mirrors, demos, videos, etc
  * IN FAILURE: webm/VPX/vorbis video format, SVG
  * .Gif support with automatic frame binding in animMap
  * Event history with demo streaming as a service in the browser for splicing all those sweet frags, SQS/Message Queue
  * Ported IQM and MD5 bone structures from spearmint engine
  * Synchronized server/AI for offline and connection interruptions
  * Repacking-as-a-service, uploader for repacking game content
  * Mesh networking with geographically distributed and load balanced proxy servers, using dedicated server web-workers.
  * Push notifications through web browser for pickup matches
  * Many mod support, compiling and playing lots of different game types, capture the flag with 3+ teams
  * Many BSP formats (Quake 1, Quake 2, Quake 4, Doom 1, Doom 2, Doom 3, Hexen maps) support and cross compatibility with other game content like Call of Duty, Half-Life, and Savage XR
  * Campaign mode, playing older engine content and playing as enemy characters, new AI for old enemies
  * New weapons mod that includes guns from every game, weapon switching is controlled by new EV_ events, that count the number of alternate weapon modes, and also count the number of classes. 
  e.g. EV_WEAPON_RESET + EV_WEAPON_COUNT + EV_WEAPON_COUNT would translate to the second mode for the current weapon, this would occur right after a EV_WEAPON_CHANGE event. So the cgame and qagame can match states without having to modify the number of bits used to describe the weapon 0-10. Similarly, an entire weapon class can be swapped out with a EV_CLASS_RESET + EV_CLASS_COUNT + EV_CLASS_COUNT would switch to the 2nd class of weapons. Both ends, cgame, and qagame have a known list of available weapons, and adding a "counter" instead of taking up bits, means we don't have to change the network model to accomodate more models/weapons/effects.  Same thing can be done for powerups/projectiles.
  * Updated WebGL renderer

The map editor and associated compiling tools are not included. We suggest you
use a modern copy from http://icculus.org/gtkradiant/.

# Compilation and installation
See [QuakeJS README](https://github.com/briancullinan/planet_quake/tree/master/code/wasm#quakejs) for more build instructions.

As a prerequisite, you will need to install the dependencies specific to your
 operating system from ioq3 https://github.com/ioquake/ioq3#compilation-and-installation

```
# install python2? python3?
git clone --recurse-submodules --remote-submodules git@github.com:briancullinan/planet_quake.git
cd planet_quake
```
or 
```
git submodule update --init
```
then
```
./libs/emsdk/emsdk install latest-upstream
./libs/emsdk/emsdk activate latest
./libs/emsdk/upstream/emscripten/embuilder.py build sdl2 vorbis ogg zlib
make PLATFORM=js
```

Binaries will be placed in `planet_quake/build/release-js-js/`.

For instructions on how to build a native dedicated server please see the
 requirements on ioq3 https://github.com/ioquake/ioq3#compilation-and-installation

# Console

See the console commands from ioq3 https://github.com/ioquake/ioq3#console

Some client variables have been set by default for compatibility, those are listed here:
https://github.com/briancullinan/planet_quake/blob/ioq3-quakejs/code/sys/sys_browser.js

# Docker (NEW)

Build the image from this repository:

`docker build .` or `docker build --target ...`

The Dockerfile has been updated to use https://docs.docker.com/develop/develop-images/multistage-build/

All images extend from either:

```
FROM node:12.15-slim as serve-tools
FROM debian:bullseye-slim AS build-tools
```

For some reason, some essential compile tool was missing from node-slim. This is a list of all the build targets and their reasoning:

* build-tools - install all the build tools needed for both builds.
* build-latest - update the copy from cache to latest from github.
* build-ded - dedicated server build.
* build-js - quake js based emscripten build.

* serve-tools - installs just the stuff needed to run and copied compiled output.
* serve-content - just serve the content in the build directory.
* serve-quake3e - start a quake3e dedicated server with run options.
* repack - all tools needed to repack game content for web.
* full - baseq3 testing content with everything in latest.

Might be easier to grab latest from dockerhub:

`docker run -ti -p 8080:8080 -p 27960:27960/udp briancullinan/quake3e:full`

Add files to the container by attaching a volume containing pk3s:

`docker run -ti -p 8080:8080 -p 27960:27960/udp -v quake3/baseq3:/home/baseq3 --name quake3e briancullinan/quake3e:full`

Then store the converted files for future runs:

`docker commit quake3e`

Visit to view:

http://127.0.0.1:8080/?connect%20127.0.0.1

To copy the built dedicated server out of the docker container, probably should just use cross-compiling with make:

`docker cp quake3e:/tmp/build/planet_quake/build/release-linux-x86_64/quake3e.ded.x64 ./build/`

# Running content server

`npm run start -- /assets/baseq3-cc ~/.quake3/baseq3-cc`

This starts the web server with converted files from all pk3s.

Long version: repak.js has some limitations, defrag crashed on unicode,
then again in the parser because of backtracking. After the bugs we fixed all
kinds of assets were missing, some maps had no textures, some weapons don't
load. This led me to _really_ think about the graphing problem. 
QVMs have compiled strings and all different
sorts of methods for loading files. Maps are fairly consistent but also have
shaders that different games or engines interpret. Map makers and modders also
leave files hanging around, some have entire copies of websites and
documentation. We can't get a perfect graph, so I made the render lazy loading
capable and thats more consistent, TODO: a combination solution would be best
downloading and using pk3s for `sv_pure` validation, and using a directory
of unpacked 'flat' files available for download.

# Repacking

`npm run repack -- /Applications/ioquake3/baseq3`

Run repack on the baseq3 mod, default location is HOME directory/.quake3/baseq3-cc

`npm run repack -- --no-graph --no-overwrite /Applications/ioquake3/baseq3`

Turn off graphics and repack content into same pk3s. This mode is good if there
is a small amount of content for a particular mod, and a very clear distinction
between map content or downloadable content.

Basic repacking steps:

1) baseq3-c - unpacks the mod and all pk3s
2) Analyze QVMs (requires a python2 environment)
3) baseq3-cc - flat files with converted images and audio
4) baseq3-ccr - repacked files using new index.json

# Building

Derives from [Quake3e Github](https://github.com/ec-/Quake3e#build-instructions)

and

[QuakeJS Github](https://github.com/inolen/quakejs#building-binaries)

It's also good to have understanding of Emscripten:

[Emscripten docs](https://emscripten.org/docs/building_from_source/toolchain_what_is_needed.html)

# Multiworld


Server:
```
./build/debug-darwin-x86_64/quake3e +set fs_basepath /Applications/ioquake3 +set fs_game baseq3 +set sv_pure 0 +set net_enable 1 +set dedicated 2 +set bot_enable 1 +set activeAction \"load\ game\ q3dm2\" +spmap q3dm1
```

then Client:

```
./build/debug-darwin-x86_64/quake3e +set fs_basepath /Applications/ioquake3 +set fs_game baseq3 +set sv_pure 0 +set net_enable 1 +set dedicated 0 +set cl_nodelta 1 +bind g \"game\;\ wait\ 10\;\ tile\ 0\ 0\ 0\;\ tile\ -1\ -1\ 1\;\ tile\ 0\ 0\ 0\" +bind h \"game\;\ world\ 1\;\" +bind j \"game\;\ wait\ 10\;\ tile\ 0\ 0\ 0\;\ tile\ 1\ 0\ 1\;\ tile\ 0\ 0\ 0\" +connect local.games
```

Explanation: Server starts on map q3dm1, the activeAction is added and launches another single player VM on q3dm2. Dedicated is required with USE_LOCAL_DED to prevent automatically starting multiple processes.

Client cl_nodelta is required for the time-being, deltas work but it causes server errors. Binds set the DVR for q3dm1 to `g` key, and q3dm2 to `h` key. Level only needs to load first time it's used.

TODO: fix deltas, add game options like switching maps at the end of round, moving around in other maps or moving to spectator or disconnecting. Options for transferring game stats between worlds.

# Contributing

Use [Issue tracker on Github](https://github.com/briancullinan/planet_quake/issues)

# Credits

Maintainers

  * Brian J. Cullinan <megamindbrian@gmail.com>
  * Anyone else? Looking for volunteers.

Significant contributions from

  * @klaussilveira, @inolen, @NTT123 (JS build mode)
  * Ryan C. Gordon <icculus@icculus.org>
  * Andreas Kohn <andreas@syndrom23.de>
  * Joerg Dietrich <Dietrich_Joerg@t-online.de>
  * Stuart Dalton <badcdev@gmail.com>
  * Vincent S. Cojot <vincent at cojot dot name>
  * optical <alex@rigbo.se>
  * Aaron Gyes <floam@aaron.gy>

# Reasons I quit working on this:
  - Chrome Network inspect no longer translate messages to UTF-8, only shows up in Base64
  - Compiling is really freaking slow, Emscripten changed something in 2020 to cause this and didn't make clear how my Makefile should change
  - Emscripten started generating separate .js.mem files, using `--memory-init-file 0` didn't help
  - Firefox is incredibly slow with this engine/renderer2 with GLSL
  - Unexplainable/punishing bugs. Can't figure out which function is being called and getting an error `function signature mismatch` but emscripten never tells me which function
  - No debugging symbols. Nothing lines up with files, offsets are wrong. Jumps to wrong places in code when clicking on exception messages or `.wasm[0xababab12e1]` some random hex offset that means nothing.
  - An unoptimized build -O0 is incredibly large, 150MB, but a slightly optimized build is not debuggable -O1 comes  out to like 5-15MB depending on the backend
