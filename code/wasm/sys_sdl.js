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

  SDL_DestroyWindow__sig: 'vi',
  SDL_DestroyWindow: function (window) {},
  SDL_QuitSubSystem__sig: 'vi',
  SDL_QuitSubSystem: function (flags) {},
  SDL_MinimizeWindow__sig: 'vi',
  SDL_MinimizeWindow: function (window) {
    window.fullscreen = false
  },

  SDL_GL_SetAttribute__proxy: 'sync',
  SDL_GL_SetAttribute__sig: 'iii',
  SDL_GL_SetAttribute: function(attr, value) {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    SDL.glAttributes[attr] = value;
  },

  SDL_GL_GetAttribute__proxy: 'sync',
  SDL_GL_GetAttribute__sig: 'iii',
  SDL_GL_GetAttribute: function(attr, value) {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    if (value) {{{ makeSetValue('value', '0', 'SDL.glAttributes[attr]', 'i32') }}};

    return 0;
  },

  SDL_GetWindowDisplayIndex__sig: 'ii',
  SDL_GetWindowDisplayIndex: function () {
    return 0
  },

  SDL_GetError__proxy: 'sync',
  SDL_GetError__sig: 'i',
  SDL_GetError: function() {
    if (!SDL.errorMessage) {
      SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), ALLOC_NORMAL);
    }
    return SDL.errorMessage;
  },

  SDL_SetError__sig: 'vi',
  SDL_SetError: function(err) {},

  SDL_GetDesktopDisplayMode__sig: 'iii',
  SDL_GetDesktopDisplayMode: function () {
    throw new Error('TODO: return window sizes')
  },
  
  SDL_CreateWindow__sig: 'iiiiiii',
  SDL_CreateWindow: function (title, x, y, w, h, flags) {
    // TODO: this will be really cool with multiworld and Movie Maker Edition
    //   Make the whole engine capable of rendering multiple windows so 2 people
    //   Can use the same machine to play on 2 displays, true PC multiplayer
  },
  
  SDL_SetDisplayModeForDisplay__sig: 'iii',
  SDL_SetDisplayModeForDisplay: function () {
    throw new Error('TODO: just keep track of this or something')
  },

  SDL_GL_SetSwapInterval__sig: 'ii',
  SDL_GL_SetSwapInterval: function (interval) {
    throw new Error('TODO: GL_SetSwapInterval')
  },

  SDL_GL_GetDrawableSize__sig: 'viii',
  SDL_GL_GetDrawableSize: function () {
    throw new Error('TODO: GL_GetDrawableSize')
  },

  SDL_WarpMouseInWindow__sig: 'viii',
  SDL_WarpMouseInWindow: function () {
    // doesn't work in javascript, which causes a stupid error where if the
    //   console is up when GRABMOUSE() is called it will get stuck outside
    throw new Error('TODO: SDL_WarpMouseInWindow')
  },

  SDL_WasInit__deps: ['SDL_Init'],
  SDL_WasInit__proxy: 'sync',
  SDL_WasInit__sig: 'i',
  SDL_WasInit: function() {
    if (SDL.startTime === null) {
      _SDL_Init();
    }
    return 1;
  },

  SDL_Init__deps: ['_Sys_GLimpInit'],
  SDL_Init__proxy: 'sync',
  SDL_Init__sig: 'ii',
  SDL_Init__docs: '/** @param{number=} initFlags */', 
  SDL_Init: function(initFlags) {
    SDL.startTime = Date.now();
    SDL.initFlags = initFlags;
    
    _Sys_GLimpInit();
  },

  SDL_GetCurrentVideoDriver__sig: 'i',
  SDL_GetCurrentVideoDriver: function () {
    throw new Error('SDL_GetCurrentVideoDriver: emscripten')
  },

  SDL_GL_SwapWindow__sig: 'vi',
  SDL_GL_SwapWindow: function (window) {
    throw new Error('TODO: GL_SwapWindow')
  },

  SDL_StartTextInput__sig: 'v',
  SDL_StartTextInput: function () {
    
  },
  SDL_StopTextInput__sig: 'v',
  SDL_StopTextInput: function () {
    
  },
};

autoAddDeps(LibrarySDL, '$SDL');
mergeInto(LibraryManager.library, LibrarySDL);
