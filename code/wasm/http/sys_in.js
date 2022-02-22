function GLimp_StartDriverAndSetMode(mode, modeFS, fullscreen, fallback) {
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
  Q3e['canvas'] = document.createElement('canvas')
  Q3e.canvas.setAttribute('width', document.body.clientWidth)
  Q3e.canvas.setAttribute('height', document.body.clientHeight)
  Q3e.canvas.width = document.body.clientWidth
  Q3e.canvas.height = document.body.clientHeight
  document.getElementById('viewport-frame').appendChild(Q3e.canvas)
  //Q3e.paged32[win>>2] = 1
  //window.title = addressToString(title)
  //return 1 //win;

  // TODO: keep track of multiple?
  let webGLContextAttributes = {
    failIfMajorPerformanceCaveat: true
  }


  Q3e['webgl'] = (!fallback)
    ? Q3e.canvas.getContext('webgl2', webGLContextAttributes)
    : (Q3e.canvas.getContext('webgl', webGLContextAttributes)
      || Q3e.canvas.getContext('experimental-webgl'))


  Q3e.webgl.viewport(0, 0, Q3e.canvas.width, Q3e.canvas.height);
  if (!Q3e.webgl) return 2
  //let handle = malloc(8);
  //Q3e.paged32[handle>>2] = 1

  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  if(false) {
    Q3e.canvas.requestPointerLock();
  } else {
    if (Q3e.canvas.exitPointerLock) {
      Q3e.canvas.exitPointerLock()
    } else if (Q3e.canvas.webkitPointerLock) {
      Q3e.canvas.webkitExitPointerLock()
    }
  }

  if(Cvar_VariableIntegerValue(stringToAddress('in_joystick'))) {
    InitNippleJoysticks()
  }
  InputInit()

  return 0 // no error
}

function updateVideoCmd () {
  if(!Q3e.canvas) {
    Q3e.canvas.setAttribute('width', canvas.clientWidth)
    Q3e.canvas.setAttribute('height', canvas.clientHeight)

  }
  // TODO: make this an SDL/Sys_Queue event to `vid_restart fast` on native
  Cvar_Set(stringToAddress('r_customWidth'), stringToAddress('' + Q3e.canvas.clientWidth))
  Cvar_Set(stringToAddress('r_customHeight'), stringToAddress('' + Q3e.canvas.clientHeight))
}

function resizeViewport () {
  // ignore if the canvas hasn't yet initialized
  if(!Q3e.canvas) return

  if (Q3e.resizeDelay) clearTimeout(Q3e.resizeDelay)
  Q3e.resizeDelay = setTimeout(updateVideoCmd, 100);
}


//typedef enum {
  // bk001129 - make sure SE_NONE is zero
const	SE_NONE = 0	// evTime is still valid
const	SE_KEY = 1		// evValue is a key code, evValue2 is the down flag
const	SE_FINGER_DOWN = 2
const	SE_FINGER_UP = 3
const	SE_CHAR = 4	// evValue is an ascii char
const	SE_MOUSE = 5	// evValue and evValue2 are relative signed x / y moves
const	SE_MOUSE_ABS = 6
const	SE_JOYSTICK_AXIS = 7	// evValue is an axis number and evValue2 is the current state (-127 to 127)
const	SE_CONSOLE = 8	// evPtr is a char*
const	SE_MAX = 9
//#ifdef USE_DRAGDROP
const	SE_DROPBEGIN = 10
const	SE_DROPCOMPLETE = 11
const	SE_DROPFILE = 12
const	SE_DROPTEXT = 13

//#endif
//} sysEventType_t;


function InputPushFocusEvent (evt) {
  if (document.visibilityState === 'visible'
    & (typeof evt.visible === 'undefined' || evt.visible !== false)) {
    Key_ClearStates();
    Q3e.exports.gw_active = false;
  } else {
    Key_ClearStates();
    Q3e.exports.gw_active = true;
    Q3e.exports.gw_minimized = false;
  }
}


function InputPushMovedEvent (evt) {
  if (evt.toElement === null && evt.relatedTarget === null) {
    //if outside the window...
    //if(!SYSI.interval)
    //SYSI.interval = setInterval(function () {
      //do something with evt.screenX/evt.screenY
    //}, 250);
    return
  }

  if(gw_active && !glw_state.isFullscreen) {
    Cvar_SetIntegerValue( 
      stringToAddress('vid_xpos'), 
      stringToAddress('' + (window.screenX || window.screenLeft)) );
    Cvar_SetIntegerValue( 
      stringToAddress('vid_ypos'), 
      stringToAddress('' + (window.screenY || window.screenTop)) );
  }
}

function InputPushTextEvent (evt) {
  if(!INPUT.consoleKeys) {
    INPUT.consoleKeys = addressToString(Cvar_VariableString(stringToAddress('cl_consoleKeys')))
  }
  // quick and dirty utf conversion?
  var text = stringToAddress(String.fromCharCode(evt.charCode))
  if ( INPUT.consoleKeys.includes(evt.key) )
  {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], true, 0, null );
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], false, 0, null );
  } else {
    Sys_QueEvent( Sys_Milliseconds(), SE_CHAR, text, 0, 0, null );
  }

}

function getMovementX(event) {
  return event['movementX'] ||
         event['mozMovementX'] ||
         event['webkitMovementX'] ||
         0;
}

function getMovementY(event) {
  return event['movementY'] ||
         event['mozMovementY'] ||
         event['webkitMovementY'] ||
         0;
}

function InputPushMouseEvent (evt) {
  if (evt.type != 'mousemove') {
    //HEAP32[((event+16)>>2)]=((down ? 1 : 0) << 8) + (evt.button+1) 
    //HEAP32[((event+20)>>2)]=evt.pageX
    //HEAP32[((event+24)>>2)]=evt.pageY
  } else {
		Sys_QueEvent( Sys_Milliseconds(), SE_MOUSE, 
      getMovementX(evt), getMovementY(evt), 0, null );
  }
}

function InputInit () {
  for(let i = 0; i < 1024; i++) {
    let name = addressToString(Key_KeynumToString(i))
    if(name.length == 0) continue
    INPUT.keystrings[name] = i
  }
  //window.addEventListener('keydown', SYSI.InputPushKeyEvent, false)
  //window.addEventListener('keyup', SYSI.InputPushKeyEvent, false)
  window.addEventListener('keypress', InputPushTextEvent, false)
  window.addEventListener('mouseout', InputPushMovedEvent, false)
  //window.addEventListener('resize', resizeViewport, false)

  Q3e.canvas.addEventListener('mousemove', InputPushMouseEvent, false)
  //SYSI.canvas.addEventListener('mousedown', SYSI.InputPushMouseEvent, false)
  //SYSI.canvas.addEventListener('mouseup', SYSI.InputPushMouseEvent, false)
  
  //document.addEventListener('mousewheel', SYSI.InputPushWheelEvent, {capture: false, passive: true})
  document.addEventListener('visibilitychange', InputPushFocusEvent, false)
  //document.addEventListener('drop', SYSI.dropHandler, false)
  //document.addEventListener('dragenter', SYSI.dragEnterHandler, false)
  //document.addEventListener('dragover', SYSI.dragOverHandler, false)

  document.addEventListener('pointerlockchange', InputPushFocusEvent, false);

  /*
  let nipple handle touch events
  SYSI.canvas.addEventListener('touchstart', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchend', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchmove', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchcancel', SYSI.InputPushTouchEvent, false)
  */
}

function InitNippleJoysticks () {
  document.body.classList.add('joysticks')
  if(SYSI.joysticks.length > 0) {
    for(var i = 0; i < SYSI.joysticks.length; i++) {
      SYSI.joysticks[i].destroy()
    }
  }
  SYSI.joysticks[0] = nipplejs.create({
    zone: document.getElementById('left-joystick'),
    multitouch: false,
    mode: 'semi',
    size: 100,
    catchDistance: 50,
    maxNumberOfNipples: 1,
    position: {bottom: '50px', left: '50px'},
  })
  SYSI.joysticks[1] = nipplejs.create({
    zone: document.getElementById('right-joystick'),
    multitouch: false,
    mode: 'semi',
    size: 100,
    catchDistance: 50,
    maxNumberOfNipples: 1,
    position: {bottom: '50px', right: '50px'},
  })
  SYSI.joysticks[2] = nipplejs.create({
    dataOnly: true,
    zone: document.body,
    multitouch: false,
    mode: 'dynamic',
    size: 2,
    catchDistance: 2,
    maxNumberOfNipples: 1,
  })
  SYSI.joysticks[0].on('start end move', SYSI.InputPushTouchEvent.bind(null, SYSI.joysticks[0], 1))
  SYSI.joysticks[1].on('start end move', SYSI.InputPushTouchEvent.bind(null, SYSI.joysticks[1], 2))
  SYSI.joysticks[2].on('start end move', SYSI.InputPushTouchEvent.bind(null, SYSI.joysticks[2], 3))
}


function SDL_SetWindowGrab() {
  
}


function SDL_StartTextInput() {

}

function SDL_StopTextInput () {
  SDL.textInput = false;
}

function SDL_ShowCursor() {
  // TODO: some safety stuff?
  if(Q3e.canvas.exitPointerLock)
    Q3e.canvas.exitPointerLock()
  else if (Q3e.canvas.webkitPointerLock)
    Q3e.canvas.webkitExitPointerLock()
}

function GLimp_Shutdown() {
  window.removeEventListener('resize', resizeViewport)
  //window.removeEventListener('keydown', SYSI.InputPushKeyEvent)
  //window.removeEventListener('keyup', SYSI.InputPushKeyEvent)
  window.removeEventListener('keypress', InputPushTextEvent)

  //document.removeEventListener('mousewheel', SYSI.InputPushWheelEvent)
  document.removeEventListener('visibilitychange', InputPushFocusEvent)
  //document.removeEventListener('drop', SYSI.dropHandler)
  //document.removeEventListener('dragenter', SYSI.dragEnterHandler)
  //document.removeEventListener('dragover', SYSI.dragOverHandler)

  if (Q3e.canvas) {
    Q3e.canvas.removeEventListener('mousemove', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mousedown', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mouseup', InputPushMouseEvent)
    Q3e.canvas.remove()
    delete Q3e['canvas']
  }
  let returnUrl = addressToString(Cvar_VariableString('cl_returnURL'))
  if(returnUrl) {
    window.location = returnUrl
  }
}

var INPUT = {
  keystrings: {},

  GLimp_Shutdown: GLimp_Shutdown,
  GLimp_StartDriverAndSetMode:GLimp_StartDriverAndSetMode,
  SDL_WasInit: function (device) { return 1; },
  SDL_Init: function () {},
  SDL_StartTextInput: SDL_StartTextInput,
  SDL_OpenAudioDevice: function () {},
  SDL_PauseAudioDevice: function () {},
  SDL_CloseAudioDevice: function () {},
  SDL_StopTextInput: SDL_StopTextInput,
  SDL_ShowCursor: SDL_ShowCursor,
  SDL_SetWindowGrab: SDL_SetWindowGrab,
  GL_GetDrawableSize: function (width, height) {
    Q3e.paged32[(width+0)>>2] = Q3e.canvas.width
    Q3e.paged32[(height+0)>>2] = Q3e.canvas.height
  }
}

