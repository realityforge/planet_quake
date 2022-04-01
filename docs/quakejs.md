
I began working on this project by updating QuakeJS. At the time, around 2018, there was a long thread describing compilation errors. It didn't make sense to me to only update emscripten, but instead also update the latest ioq3 codebase. It also made more sense to integrate the changes cleanly into ioq3 and submit it as a build mode for ioq3. For political reasons, I switched to Q3e, and all the WebAssembly code exists under `code/wasm`, along-side the other OS dependent folders.

## WebAssembly Features

  * ...using OpenGL OpenGL ES 3.0 (WebGL 2.0 (OpenGL ES 3.0 Chromium))
  * 2,500+ available maps on [lvlworld.com](https://lvlworld.com) and another 15,000+ planned, `cl_returnURL` for redirecting on quit and disconnect
  * Removed SDL inputs, touch support on mobile works, copy/paste
  * Drag and drop for sharing game content with the browser
  * .cfg file uploads/local imports - uses home directory for drag and drop content
  * A working repack script to convert game assets to be more web compatible
  * [NippleJS](https://github.com/yoannmoinet/nipplejs) mobile support
  * A content server (NodeJS + express) to repack and live-reload the game as you develop
  * A SOCKS5 server to convert web-socket connections to UDP and connect to any standard Quake 3 server
  * Various mod disassemblies for associating hard-coded shaders with files even if the mod isn't open source
  * Various graphs of mods, including file names for repacked content
  * Offline mode for local and LAN games, just visit quake.games and run the command `\offline` in the console to cache all necessary files to local storage. [Google Reference](https://developers.google.com/web/fundamentals/codelabs/offline)
  * Web-worker dedicated local server for mesh networked gaming, game sharing over localized Socks proxy network.
  * Admin monitoring of cmd stream, Huffman decoding for proxy, Man-In-The-Middle proof-of-concept.
  * Checksum pk3 spoofing for playing with original clients using web converted files.
  * URL state management for accessing menus and for connecting to a server, i.e. https://quake.games?connect%20address using the [History API pushstate](https://caniuse.com/?search=pushstate)


## TODO Features

  * Drag and dropped content uploaded and accepted by the package manager for sharing with other clients/servers
  * Popup keyboard on mobile, somehow detecting a text box from UIVM, might not work on all mods. 
  * IN PROGRESS: Compile baseq3a from EC directly to WASM and load asynchronously, https://github.com/emscripten-core/emscripten/wiki/Linking
  * Switching renderers to WebGL 1/OpenGL 1/ES 1+2, closer with dlopen work
  * r_smp 2 Software renderer for rendering far distances in a web-worker, WebGL if OffscreenCanvas is available, low resolution software GL is not available
  * Repacking-as-a-service, uploader for repacking game content
  * Mesh networking with geographically distributed and load balanced proxy servers, using dedicated server web-workers.
  * Push notifications through web browser for pickup matches
  * (Short term tasks) Stop local server from dropping, kickall bots
  * Quit a server if all human clients disconnect 
  * Add websockets to native dedicated server instead of relying on proxy https://github.com/rohanrhu/cebsocket, 
  * Run without dedicated server worker in single thread on mobile (bring back r_smp 1 for dedicated server feature, r_smp 2 for renderer features, r_smp 0 for mobile/off)
  * Pre-download content like the repack graph mode sets up so less is downloaded in game
  * Use lvlworld.cfg and autoconfig after so people can save their settings (need a way to exit "preview mode" and play the game with networking, simple menu items in preview mode "start game" option with a sharable match hyperlink)
  * Add touch and mobile controls that look like this (https://jayanam.com/ugui-unity-mobile-touch-control/) but are actually generated from brushes and look in the direction they are being pulled like a 3D model like the head. Make all numbers and HUD controls 3D models. Since RMLUI will be simple we can focus on advanced HUD  features like resizing on demand. Player controls for video and demos https://www.google.com/search?q=simple+html+media+player+controls
  * SSE/SIMD support in vm_js.js Com_SnapVectors(), https://emscripten.org/docs/porting/simd.html
  * Combination solution would be best downloading and using pk3s for `sv_pure` validation, and using a directory of unpacked 'flat' files available for download.


## Running content server

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
  capable and thats more consistent.

## Repacking

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

### Repacking with make

  Included is a Makefile build script that repacks any mod directory into a single pk3 file suitable for use with QuakeJS.

  `make -f make/build_package.make SRCDIR=/Applications/ioquake3/baseq3`

  Graphing features are not included, so if there are extra source files, they will need to be TODO: filtered out. TODO: `NO_OVERWRITE=1` works as expected.

## Compilation and installation
  
  See [QuakeJS README](../QuakeJs.md) for more build instructions.

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


## Reasons I quit working on this:
  - Chrome Network inspect no longer translate messages to UTF-8, only shows up in Base64
  - Compiling is really freaking slow, Emscripten changed something in 2020 to cause this and didn't make clear how my Makefile should change
  - Emscripten started generating separate .js.mem files, using `--memory-init-file 0` didn't help
  - Firefox is incredibly slow with this engine/renderer2 with GLSL
  - Unexplainable/punishing bugs. Can't figure out which function is being called and getting an error `function signature mismatch` but emscripten never tells me which function
  - No debugging symbols. Nothing lines up with files, offsets are wrong. Jumps to wrong places in code when clicking on exception messages or `.wasm[0xababab12e1]` some random hex offset that means nothing.
  - An unoptimized build -O0 is incredibly large, 150MB, but a slightly optimized build is not debuggable -O1 comes  out to like 5-15MB depending on the backend
