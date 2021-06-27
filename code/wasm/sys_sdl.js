// TRIM this way down BECAUSE! Q3 has it's own internal event queue, so by 
//   Using SDLs event queue, we are double queuing. Instead, we just inject
//   events directly into the games event stack, since JS must have some sort
//   of memory semaphore built in, we don't need to worry about similar pthread(mutexes)
//   DROP SDL input support all together and use sys_input.js, this stuff is for n00bz
var global = window || global
global['SDL'] = {}

function SDL_SetVideoMode (width, height, depth, flags) {
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
}

function SDL_DestroyWindow (window) {}

function SDL_QuitSubSystem (flags) {}

function SDL_MinimizeWindow (window) {
  window.fullscreen = false
}

function SDL_GL_SetAttribute (attr, value) {
  if (!(attr in SDL.glAttributes)) {
    abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
  }

  SDL.glAttributes[attr] = value;
}

function SDL_GL_GetAttribute (attr, value) {
  if (!(attr in SDL.glAttributes)) {
    abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
  }

  if (value) HEAP32[(value+0)>>2] = SDL.glAttributes[attr];

  return 0;
}

function SDL_GetWindowDisplayIndex () {
  return 0
}

function SDL_GetError () {
  if (!SDL.errorMessage) {
    SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), ALLOC_NORMAL);
  }
  return SDL.errorMessage;
}

function SDL_SetError (err) {}

function SDL_GetDesktopDisplayMode () {
  throw new Error('TODO: return window sizes')
}

function SDL_CreateWindow (title, x, y, w, h, flags) {
  // TODO: this will be really cool with multiworld and Movie Maker Edition
  //   Make the whole engine capable of rendering multiple windows so 2 people
  //   Can use the same machine to play on 2 displays, true PC multiplayer
}

function SDL_SetDisplayModeForDisplay () {
  throw new Error('TODO: just keep track of this or something')
}

function SDL_GL_SetSwapInterval (interval) {
  throw new Error('TODO: GL_SetSwapInterval')
}

function SDL_GL_GetDrawableSize () {
  throw new Error('TODO: GL_GetDrawableSize')
}

function SDL_WarpMouseInWindow () {
  // doesn't work in javascript, which causes a stupid error where if the
  //   console is up when GRABMOUSE() is called it will get stuck outside
  throw new Error('TODO: SDL_WarpMouseInWindow')
}

function SDL_WasInit () {
  if (SDL.startTime === null) {
    Sys_GLimpInit();
  }
  return 1;
}

function SDL_GetCurrentVideoDriver () {
  throw new Error('SDL_GetCurrentVideoDriver: emscripten')
}

function SDL_GL_SwapWindow (window) {
  throw new Error('TODO: GL_SwapWindow')
}

function SDL_StartTextInput () {
  
}

function SDL_StopTextInput () {
  
}
