
function SDL_GetDesktopDisplayMode(display, mode) {
  Q3e.paged32[(mode+1)>>2] = window.innerWidth
  Q3e.paged32[(mode+2)>>2] = window.innerHeight
}


function SDL_GL_GetDrawableSize(display, width, height) {
  Q3e.paged32[(width+0)>>2] = Q3e.canvas.width
  Q3e.paged32[(height+0)>>2] = Q3e.canvas.height
}

function SDL_CreateWindow (title, x, y, w, h, flags) {
  //let win = malloc(8)
  // TODO: multiple windows like a DVR?
  //   what kind of game needs two screens for one player to switch back and forth?
  /*
  SDL.windows[handle] = {
    x: x,
    y: y,
    w: w,
    h: h,
    title: title,
    flags: flags,
  }
  */
  Q3e.canvas = document.createElement('canvas')
  Q3e.canvas.width = document.body.innerWidth
  Q3e.canvas.height = document.body.innerHeight
  document.getElementById('viewport-frame').appendChild(Q3e.canvas)

  //Q3e.paged32[win>>2] = 1
  window.title = addressToString(title)
  return 1 //win;
}

function SDL_GL_CreateContext(canvas) {
  // TODO: keep track of multiple?
  let webGLContextAttributes = {
    failIfMajorPerformanceCaveat: true
  }
  Q3e['webgl'] = Q3e.canvas.getContext('webgl', webGLContextAttributes)
  /*
  // TODO: fallback
  Q3e['webgl'] = (Q3e.majorVersion > 1)
    ? Q3e.canvas.getContext('webgl2', webGLContextAttributes)
    : (Q3e.canvas.getContext('webgl', webGLContextAttributes)
      || Q3e.canvas.getContext('experimental-webgl'))
  */
  GLImmediate = new $GLImmediate()
  if (!Q3e['webgl']) return 0
  //let handle = malloc(8);
  //Q3e.paged32[handle>>2] = 1
  return 1 // handle
}

function SDL_GL_SetAttribute(attr, value) {
  Q3e.majorVersion = value
}

function SDL_GetError() {
  return stringToAddress('Unknown WebGL error.')
}

function SDL_SetWindowDisplayMode () { 
  // TODO: add a button or something for user
} 

function SDL_SetWindowGrab (window, grabbed) {
  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  if(grabbed) {
    Q3e.canvas.requestPointerLock();
  } else {
    if (Q3e.canvas.exitPointerLock) {
      Q3e.canvas.exitPointerLock();
    } else if (Q3e.canvas.webkitPointerLock) {
      Q3e.canvas.webkitExitPointerLock();
    }
  }
}

function SDL_StartTextInput() {

}

function SDL_MinimizeWindow (window) {
  window.fullscreen = false
}

function SDL_StopTextInput () {
  SDL.textInput = false;
}

function SDL_ShowCursor() {
  // TODO: some safety stuff?
  Q3e.canvas.exitPointerLock();
}

var SDL = {

  SDL_GetDesktopDisplayMode: SDL_GetDesktopDisplayMode,
  SDL_GL_SetAttribute: SDL_GL_SetAttribute,
  SDL_CreateWindow: SDL_CreateWindow,
  SDL_GetError: SDL_GetError,
  SDL_SetWindowDisplayMode: SDL_SetWindowDisplayMode,
  SDL_GL_CreateContext: SDL_GL_CreateContext,
  SDL_GL_GetDrawableSize: SDL_GL_GetDrawableSize,
  SDL_WasInit: function (device) { return 1; },
  SDL_Init: function () {},
  SDL_MinimizeWindow: SDL_MinimizeWindow,
  SDL_StartTextInput: SDL_StartTextInput,
  SDL_OpenAudioDevice: function () {},
  SDL_PauseAudioDevice: function () {},
  SDL_CloseAudioDevice: function () {},
  SDL_StopTextInput: SDL_StopTextInput,
  SDL_SetWindowGrab: SDL_SetWindowGrab,
  SDL_ShowCursor: SDL_ShowCursor,
}

