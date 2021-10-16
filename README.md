
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

To see a live demo, check out https://quake.games

The feature list has become so long that I needed separate readme files to describe each piece. This list will only have a short, truncated description with links to the other read-mes.

More on what makes WebAssembly [difficult to build here](./docs/quakejs.md).

## New Features

  * [Docker support](./docs/docker.md)
  * [WebAssembly build mode](./docs/quakejs.md)
  * [Lazy loading](./docs/lazyloading.md)
  * [Server-side demos](./docs/demos.md)
  * [Multiworld, multi-map loading](./docs/multiworld.md)
  * [Lightning Network bitcoin transactions](./docs/payments.md)
  * [Procedural map generation](./docs/procedural.md)
  * [Multiple game styles](./docs/games.md)
  * [Slim client/auto connect and more](./docs/client.md)
  * [Referee, persistent sessions, event system](./docs/server.md)
  * PNG support
  * Drag and drop
  * Many, many bug fixes


## Coming soon!
  
  * Asynchronous lazy loading with zip file repackaging and coalescing.
  * Multi-world file-system and networking for connecting to specific worlds.
  * demoMap rendering that maps a .dm file to a surface in game.
  * Advanced teleporting features like replacing Voids with teleporting back, addressable.

## Future TODOs

  * Authenticated clients
  * Copy kubernetes support https://github.com/criticalstack/quake-kube
  * Asynchronous rendering for portals, mirrors, demos, videos, etc
  * Make a simple thread manager https://stackoverflow.com/questions/7269709/sending-information-with-a-signal-in-linux or use oneTBB as an alternative?
  * IN PROGRESS: removing Emscripten and compiling only to wasm with clang.
  * Move more features like EULA, file extension alternatives from ListFiles that comes from menu system, etc out of JS and in to C system.
  * Quake 3 1.16n and dm3 integrated support from https://github.com/zturtleman/lilium-arena-classic.
  * IN FAILURE: HTML and CSS menu renderer
  * Always on twitch.tv streaming at no expense to the game server
  * IN FAILURE: webm/VPX/vorbis video format, SVG


# Console

See the console commands from ioq3 https://github.com/ioquake/ioq3#console

Some client variables have been set by default for compatibility, those are listed here:
https://github.com/briancullinan/planet_quake/blob/ioq3-quakejs/code/sys/sys_browser.js


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
