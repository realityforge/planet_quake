
- [Compilation and installation](#compilation-and-installation)
- [Console](#console)
- [Docker](#docker)
- [Running content server](#running-content-server)
- [Repacking](#repacking)
- [Building](#building)
- [Contributing](#contributing)
- [Credits](#credits)



                ,---------------------------------------------.
                |   ____              _             _  _____  |
                |  / __ \            | |           | |/ ____| |
                | | |  | |_   _  __ _| | _____     | | (___   |
                | | |  | | | | |/ _` | |/ / _ \_   | |\___ \  |
                | | |__| | |_| | (_| |   |  __| |__| |____) | |
                |  \___\_\\__,_|\__,_|_|\_\___|\____/|_____/  |
                |                                             |
                '----------- https://quake.games -------------'
                                        
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
  * Docker support
  * 2,500+ available maps on [lvlworld.com](https://lvlworld.com) and another 15,000+ planned, `cl_returnURL` for redirecting on quit and disconnect
  * Removed SDL inputs, touch support on mobile works, copy/pase, TODO: drag and drop for loading local custom content
  * Deferred (lazy) loading of all game content, entities, models, textures. New cl_lazyLoad cvar, 1 for on load lowest quality until displayed, 2 set all to default and try to load from sv_dlURL
  * Offline mode for local and LAN games, just visit quake.games and run the command `\offline` in the console to cache all necessary files to local storage. [Google Reference](https://developers.google.com/web/fundamentals/codelabs/offline)
  * Web-worker dedicated local server for mesh networked gaming, game sharing over localized Socks proxy network. TODO: authenticated clients that allow local commands to be run, good for browser, might make native client vulnerable. TODO: if map doesn't load report it to client.
  * Rcon auto-complete, sends a `complete` command to server and response with an `autocomplete` key in an `infoResponse` which is an easy way to intercept messages without adding a command.
  * Server-side demos, recording for every client, [TheDoctor's method](http://openarena.ws/board/index.php?topic=4437.0). Server-side demos, [lrq3000 implementation](https://github.com/lrq3000/ioq3-server-side-demos) recording entire server state and spectating playback. [Cyrax' multiview protocol](http://edawn-mod.org/forum/viewtopic.php?f=6&t=7) for viewing all clients from one demo file. TODO: playing back dm_68 files for all players. TODO: adding my multi-world mod and multi-qvm for ultimate administration. TODO: bug shutting down client times out because it isn't in svc.clients anymore?
  * Heavily modified "Local" multiplayer page that lists specific masters server using `cl_master1-24` as opposed to `sv_master1-24` like on the "Internet" page of the multiplayer menu.
  * Lightning Network bitcoin transactions, see `sv_ln*` settings for more information. QR code generation by [Nayuki](https://www.nayuki.io/page/qr-code-generator-library).
  * Admin monitoring of cmd stream, Huffman decoding for proxy, Man-In-The-Middle POC
  * Many, many bug fixes

Coming soon!
  * TODO: (Short term tasks) Stop local server from dropping, kickall bots, quit a server if all human clients disconnect, fix loading on parent's computer (webgl 1?), add swGL https://github.com/h0MER247/swGL as last chance, pure server compatibility by spoofing checksums for known (memory finger printing?) paks, cl_lazyLoad 2 and 3 for only loading during network downtimes (warmup and between matches and during respawn timeout), add websockets to native dedicated server instead of relying on proxy, run without dedicated server worker in single thread on mobile, bring back download commands for grabbing new content
  * Payment API for micropayments to access server or content https://developers.google.com/web/fundamentals/codelabs/payment-request-api
  * SSE/SIMD support in vm_js.js Com_SnapVectors(), https://emscripten.org/docs/porting/simd.html
  * Compile baseq3a from EC directly to WASM and load asynchronously, https://github.com/emscripten-core/emscripten/wiki/Linking
  * Switching renderers to WebGL 1/OpenGL 1/ES 1+2
  * Alternate chat server integration, discord and telegram
  * Extra UI menus with multiQVM, for voting on maps and bitcoin setup
  * Use com_journal instead of index.json (or manifest.json in quakejs)
  * Download files using offsets out of pk3 files, like streaming a part of the zip file, add this to native dedicated server and UDP downloads, this won't work on Google CDN because there is no accept-ranges support with compression
  * Drag and drop for sharing game content with the browser. .cfg file uploads/local imports
  * Multiple QVM loader, multiple map loader in parallel with teleport switch, compile QVMs in native mode (wasm) and load with Webassembly asynchronously
  * Multi-view, instant replay
  * HTML and CSS menu renderer
  * URL state management for accessing menus and for connecting to a server, i.e. https://quake.games?connect%20address using the [History API pushstate](https://caniuse.com/?search=pushstate)
  * Language agnostic events API for writing new cgames/games/uis in other languages like Python, JavaScript, Lua
  * Software renderer for rendering far distances in a web-worker, WebGL if OffscreenCanvas is available, low resolution software GL is not available
  * Always on twitch.tv streaming at no expense to the game server
  * Shader palettes for pre-rendering colors and changing the theme of maps
  * Socks5 based cUrl downloads for downloading over the proxy and avoid content access controls
  * LOD (level of detail) based compression, loading different levels of detail in models and shaders, distance based mipmaps
  * Brotli compression for game content from server UDP downloads
  * Asynchronous rendering for portals, mirrors, demos, videos, etc
  * webm/VPX/vorbis video format, "demoMap" surface parm which renders demos to an arbitrary surface. .Gif support with automatic frame binding in animMap
  * Ported IQM and MD5 from spearmint engine
  * Synchronized server/AI for offline and connection interruptions
  * Repacking-as-a-service, uploader for repacking game content
  * Mesh networking with geographically distributed and load balanced proxy servers, using dedicated server web-workers.
  * Push notifications through web browser for pickup matches
  * Procedurally generated game content and maps
  * Many mod support, compiling and playing lots of different game types, capture the flag with 3+ teams
  * Many BSP formats (Quake 1, Quake 2, Quake 4, Doom 1, Doom 2, Doom 3, Hexen maps) support and cross compatibility with other game content like Call of Duty, Half-Life, and Savage XR
  * Campaign mode, playing older engine content and playing as enemy characters, new AI for old enemies
  * Server moderator permissions, admins can set sv_modCmds and sv_modCvars to allow moderator passwords modpassword1-3 to only change specific settings, like maps, bans, allowing voting, but not change games like fs_basepath and fs_game.
  * Updated WebGL renderer

The map editor and associated compiling tools are not included. We suggest you
use a modern copy from http://icculus.org/gtkradiant/.

# Compilation and installation
See [QuakeJS README](https://github.com/briancullinan/planet_quake/tree/master/code/xquakejs#quakejs) for more build instructions.

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
./code/xquakejs/lib/emsdk/emsdk install latest-upstream
./code/xquakejs/lib/emsdk/emsdk activate latest
./code/xquakejs/lib/emsdk/upstream/emscripten/embuilder.py build sdl2 vorbis ogg zlib
make PLATFORM=js
```

Binaries will be placed in `planet_quake/build/release-js-js/`.

For instructions on how to build a native dedicated server please see the
 requirements on ioq3 https://github.com/ioquake/ioq3#compilation-and-installation

# Console

See the console commands from ioq3 https://github.com/ioquake/ioq3#console

Some client variables have been set by default for compatibility, those are listed here:
https://github.com/briancullinan/planet_quake/blob/ioq3-quakejs/code/sys/sys_browser.js

# Docker

Build the image from this repository:

`docker build -t quake3e .` or `docker build --target builder .`

Grab latest from dockerhub

`docker run -ti -v ~/Quake3e:/tmp/Quake3e -v /Applications/ioquake3/baseq3:/tmp/baseq3 -p 8080:8080 -p 1081:1081 -p 27960:27960/udp --name quake3e briancullinan/quake3e:latest`

After the image is built and running, you can skip repeating the conversion process:

`docker commit quake3e quake3e`

`docker start -i quake3e`

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
