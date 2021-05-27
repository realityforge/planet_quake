var LibrarySDL = {
  // TRIM this way down BECAUSE! Q3 has it's own internal event queue, so by 
  //   Using SDLs event queue, we are double queuing. Instead, we just inject
  //   events directly into the games event stack, since JS must have some sort
  //   of memory semaphore built in, we don't need to worry about similar pthread(mutexes)
  //   DROP SDL input support all together and use sys_input.js, this stuff is for n00bz

  $SDL__deps: [
#if FILESYSTEM
    '$FS',
#endif
    '$PATH', '$Browser', 'SDL_GetTicks', 'SDL_LockSurface',
    '$SDL_unicode', '$SDL_ttfContext', '$SDL_audio'
  ],
  $SDL: {
  },

  SDL_SetVideoMode__deps: ['$GL3'],
  SDL_SetVideoMode__proxy: 'sync',
  SDL_SetVideoMode__sig: 'iiiii',
  SDL_SetVideoMode: function(width, height, depth, flags) {
    ['touchstart', 'touchend', 'touchmove', 'mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'wheel', 'mouseout'].forEach(function(event) {
      Module['canvas'].addEventListener(event, SDL.receiveEvent, true);
    });

    var canvas = Module['canvas'];

    // (0,0) means 'use fullscreen' in native; in Emscripten, use the current canvas size.
    if (width == 0 && height == 0) {
      width = canvas.width;
      height = canvas.height;
    }

    SDL.settingVideoMode = true; // SetVideoMode itself should not trigger resize events
    Browser.setCanvasSize(width, height);
    SDL.settingVideoMode = false;

    // Free the old surface first if there is one
    if (SDL.screen) {
      SDL.freeSurface(SDL.screen);
      assert(!SDL.screen);
    }

    if (SDL.GL) flags = flags | 0x04000000; // SDL_OPENGL - if we are using GL, then later calls to SetVideoMode may not mention GL, but we do need it. Once in GL mode, we never leave it.

    SDL.screen = SDL.makeSurface(width, height, flags, true, 'screen');

    return SDL.screen;
  },

  SDL_GL_CreateContext__sig: 'ii',
  SDL_GL_CreateContext: function (window) {
    return GL3.createContext
  },

  
};

autoAddDeps(LibrarySDL, '$SDL');
mergeInto(LibraryManager.library, LibrarySDL);
