


## Client Features

  * Uncheat, removed the `CVAR_CHEAT` flag from specific cvars, then shares those cvars with the server for blocking.
  * Rcon auto-complete, sends a `complete` command to server and response with an `autocomplete` key in an `infoResponse` which is an easy way to intercept messages without adding a command.
  * Heavily modified "Local" multiplayer page that lists specific masters server using `cl_master1-24` as opposed to `sv_master1-24` like on the "Internet" page of the multiplayer menu, if admins want to list servers by geographically nearby.
  * Slim client without extra file formats or server, only for connecting to games or rendering demos.
  * RmlUi support for HTML/RCSS style menus.
  * Persistent console.


## TODOs

  * LOD (level of detail) based compression, loading different levels of detail in models and shaders, distance based mipmaps
  * Socks5 based cUrl downloads for downloading over the proxy and avoid content access controls
  * Brotli compression for game content from server UDP downloads
  * .Gif support with automatic frame binding in animMap
  * Ported IQM and MD5 bone structures from spearmint engine
  * Many BSP formats (Quake 1, Quake 2, Quake 4, Doom 1, Doom 2, Doom 3, Hexen maps) support and cross compatibility with other game content like Call of Duty, Half-Life, and Savage XR
  * move console background image API to server-side with console instead of mod side like in e+/freon.
  * Fix r_fbo and add pixel buffer objects for recordings. Send PBO to a worker thread to encode to VPX-wasm for live streaming.
  * Make all game code entry points asynchronous with engine to run all QVMs in web workers/pthreads. Pause VM for async calls between engine and VM.
  * Allow different VMs on file system by using more search directories and Pure server calls for filtering.
