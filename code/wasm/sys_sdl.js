// TRIM this way down BECAUSE! Q3 has it's own internal event queue, so by 
//   Using SDLs event queue, we are double queuing. Instead, we just inject
//   events directly into the games event stack, since JS must have some sort
//   of memory semaphore built in, we don't need to worry about similar pthread(mutexes)
//   DROP SDL input support all together and use sys_input.js, this stuff is for n00bz
var global = window || global
global['SDL'] = {}

function SDL_SetVideoMode (width, height, depth, flags) {
  // (0,0) means 'use fullscreen' in native; in Emscripten, use the current canvas size.
  if (width == 0 && height == 0) {
    width = SYSI.canvas.width;
    height = SYSI.canvas.height;
  }

  SDL.settingVideoMode = true; // SetVideoMode itself should not trigger resize events
  SYSI.canvas.width = width
  SYSI.canvas.height = height
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

function SDL_DestroyWindow (window) {
  // TODO: delete window?
}

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

function SDL_ShowCursor (toggle) {
  // TODO: wait for first click because browsers do this really annoying
  //   thing where it can request the pointer lock from outside the window
  //   causing the pointer to be stuck and not receive mouse move events
  //   this is apparent when hiding the Q3 console with a ~ key press when
  //   the mouse is still outside
  switch (toggle) {
    case 0: // SDL_DISABLE
      //if (Browser.isFullscreen) { // only try to lock the pointer when in full screen mode
      //} else { // else return SDL_ENABLE to indicate the failure
      //  return 1;
      //}
      SYSI.canvas.requestPointerLock();
      return 0;
      break;
    case 1: // SDL_ENABLE
      SYSI.canvas.exitPointerLock();
      return 1;
      break;
    case -1: // SDL_QUERY
      return !Browser.pointerLock;
      break;
    default:
      console.log( "SDL_ShowCursor called with unknown toggle parameter value: " + toggle + "." );
      break;
  }
}

function SDL_GetWindowFlags (x, y) {
  if (Browser.isFullscreen) {
     return 1;
  }

  return 0;
}

function SDL_SetWindowGrab (window, grabbed) {
  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  SDL_ShowCursor(grabbed)
}

function SDL_GetError () {
  if (!SDL.errorMessage) {
    SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), ALLOC_NORMAL);
  }
  return SDL.errorMessage;
}

function SDL_SetError (err) {}

function Sys_GetDisplayMode (index, mode) {
  SDL.mode = allocate(new Int32Array(6>>2), ALLOC_NORMAL)
  SDL.mode[1] = window.innerWidth
  SDL.mode[2] = window.innerHeight
  return 0
}

function SDL_SetWindowTitle (window, title) {
  document.title = UTF8ToString(title)
}

function SDL_GL_GetDrawableSize (window, w, h) {
  HEAP32[w] = SYSI.canvas.width
  HEAP32[h] = SYSI.canvas.height
}

function SDL_CreateWindow (title, x, y, w, h, flags) {
  // TODO: this will be really cool with multiworld and Movie Maker Edition
  //   Make the whole engine capable of rendering multiple windows so 2 people
  //   Can use the same machine to play on 2 displays, true PC multiplayer
  var win = _malloc(8)
  SDL.windows[handle] = {
    x: x,
    y: y,
    w: w,
    h: h,
    title: title,
    flags: flags,
  }
  SDL_SetWindowTitle(win, title)
  return win;
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
  SDL.textInput = true;
}

function SDL_StopTextInput () {
  SDL.textInput = false;
}
