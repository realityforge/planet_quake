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
  if(!Q3e.canvas) {
    Q3e['canvas'] = document.createElement('canvas')
    Q3e.canvas.setAttribute('width', document.body.clientWidth)
    Q3e.canvas.setAttribute('height', document.body.clientHeight)
    Q3e.canvas.width = document.body.clientWidth
    Q3e.canvas.height = document.body.clientHeight
    document.getElementById('viewport-frame').appendChild(Q3e.canvas)
  }

  //HEAP32[win>>2] = 1
  //window.title = addressToString(title)
  //return 1 //win;

  // TODO: keep track of multiple?
  let webGLContextAttributes = {
    failIfMajorPerformanceCaveat: true,
    alpha: false,
    stencil: false,
  }


  Q3e['webgl'] = (!fallback)
    ? Q3e.canvas.getContext('webgl2', webGLContextAttributes)
    : (Q3e.canvas.getContext('webgl', webGLContextAttributes)
      || Q3e.canvas.getContext('experimental-webgl'))

  Q3e.webgl.viewport(0, 0, Q3e.canvas.width, Q3e.canvas.height);
  if (!Q3e.webgl) return 2
  //let handle = malloc(8);
  //HEAP32[handle>>2] = 1
  if(typeof GL != 'undefined') {
    INPUT.handle = GL.registerContext(Q3e.webgl, webGLContextAttributes)
    GL.makeContextCurrent(INPUT.handle)
    Module.useWebGL = true;
    //GLImmediate.clientColor = new Float32Array([ 1, 1, 1, 1 ]);
    //GLImmediate.init()
    //GLImmediate.getRenderer()
  }

  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  if(!INPUT.firstClick) {
    Q3e.canvas.requestPointerLock();
  } else {
    SDL_ShowCursor()
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


const KEYCATCH_CONSOLE = 0x0001
const KEYCATCH_UI      =  0x0002
const KEYCATCH_MESSAGE =  0x0004
const KEYCATCH_CGAME   =  0x0008

function InputPushFocusEvent (evt) {
  if (document.visibilityState === 'visible'
    & (typeof evt.visible === 'undefined' || evt.visible !== false)) {
    Key_ClearStates();
    gw_active = false;
  } else {
    Key_ClearStates();
    gw_active = true;
    gw_minimized = false;
  }
}


function InputPushMovedEvent (evt) {
  if (evt.toElement === null && evt.relatedTarget === null) {
    INPUT.firstClick = false
    //if outside the window...
    //if(!SYSI.interval)
    //SYSI.interval = setInterval(function () {
      //do something with evt.screenX/evt.screenY
    //}, 250);
    return
  }

  let notFullscreen = !document.isFullScreen
    && !document.webkitIsFullScreen
    && !document.mozIsFullScreen

  if(gw_active && notFullscreen) {
    Cvar_SetIntegerValue( 
      stringToAddress('vid_xpos'), 
      stringToAddress('' + (window.screenX || window.screenLeft)) );
    Cvar_SetIntegerValue( 
      stringToAddress('vid_ypos'), 
      stringToAddress('' + (window.screenY || window.screenTop)) );
  }
}

function captureClipBoard () {
  // this is the same method I used on StudySauce
  var text = document.createElement('TEXTAREA')
  text.style.opacity = 0
  text.style.height = '1px'
  text.style.width = '1px'
  text.style.display = 'block'
  text.style.zIndex = 1000
  text.style.position = 'absolute'
  document.body.appendChild(text)
  text.focus()
  setTimeout(function () {
    INPUT.paste = text.value
    Module.viewport.focus()
    if(INPUT.field) {
      INPUT.paste.split('').forEach(function (k) {
        Field_CharEvent(INPUT.field, k.charCodeAt(0))
      })
      INPUT.paste = ''
      INPUT.field = 0
      text.remove()
    }
  }, 100)
}

function checkPasteEvent (evt) {
  // mac support
  if(evt.key == 'Meta') INPUT.superKey = evt.type == 'keydown'
  if(INPUT.superKey && (evt.key == 'v' || evt.key == 'V')) {
    if(INPUT.superKey && (evt.type == 'keypress' || evt.type == 'keydown')) {
      captureClipBoard()
    }
  }
}

function InputPushKeyEvent(evt) {
  if(evt.keyCode === 8) {
    INPUT.cancelBackspace = true;
    setTimeout(function () { INPUT.cancelBackspace = false }, 100)
  }
  if(evt.keyCode === 27) {
    INPUT.cancelBackspace = true;
    InputPushFocusEvent({visible: false})
  }

  checkPasteEvent(evt)
  
  if(evt.type == 'keydown') {
    if ( evt.repeat && Key_GetCatcher() == 0 )
      return

    if ( evt.keyCode == 13 /* ENTER */ && evt.altKey ) {
      let notFullscreen = !document.isFullScreen
      && !document.webkitIsFullScreen
      && !document.mozIsFullScreen
      Cvar_SetIntegerValue( stringToAddress('r_fullscreen'), notFullscreen ? 1 : 0 );
      Cbuf_AddText( stringToAddress('vid_restart\n') );
      return
    }

    if ( evt.keyCode == 8 /* BAKCSPACE */ )
      Sys_QueEvent( Sys_Milliseconds(), SE_CHAR, 8, 0, 0, null );

    if ( evt.keyCode ) {
      if(evt.keyCode >= 65 && evt.keyCode <= 90) {

      }
      Sys_QueEvent( Sys_Milliseconds(), SE_KEY, evt.keyCode, true, 0, null );

      if( evt.ctrlKey && key >= 'a' && evt.keyCode <= 'z' ) {
        Sys_QueEvent( Sys_Milliseconds(), SE_CHAR, evt.keyCode-'a'+1, 0, 0, null );
      }
    }
  } else if (evt.type == 'keyup') {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, evt.keyCode, false, 0, null );
  }

}

function InputPushTextEvent (evt) {
  if(!INPUT.consoleKeys) {
    INPUT.consoleKeys = addressToString(Cvar_VariableString(stringToAddress('cl_consoleKeys')))
  }
  // quick and dirty utf conversion?
  if ( INPUT.consoleKeys.includes(evt.key) )
  {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], true, 0, null )
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], false, 0, null )
    setTimeout(function () {
      if(Key_GetCatcher() & KEYCATCH_CONSOLE) {
        SDL_ShowCursor()
      } else {
        if(!INPUT.firstClick)
          Q3e.canvas.requestPointerLock();      
      }
    }, 100)
  } else {
    Sys_QueEvent( Sys_Milliseconds(), SE_CHAR, evt.charCode, 0, 0, null )
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

function InputPushWheelEvent(evt) {
	if( evt.deltaY > 0 ) {
		Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['MWHEELUP'], true, 0, null );
		Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['MWHEELUP'], false, 0, null );
	} else if( evt.deltaY < 0 ) {
		Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['MWHEELDOWN'], true, 0, null );
		Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['MWHEELDOWN'], false, 0, null );
	}
}


function InputPushMouseEvent (evt) {
  if (evt.type != 'mousemove') {
    let down = evt.type == 'mousedown'
    // TODO: fix this maybe?
    //if(!mouseActive || in_joystick->integer) {
    //  return;
    //}
    if(down) {
      // TODO: start sound, capture mouse
      if(INPUT.firstClick && !(Key_GetCatcher() & KEYCATCH_CONSOLE)) {
        Q3e.canvas.requestPointerLock();
        INPUT.firstClick = false
      }
    }
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['MOUSE1'] + evt.button, down, 0, null );
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
  window.addEventListener('keydown', InputPushKeyEvent, false)
  window.addEventListener('keyup', InputPushKeyEvent, false)
  window.addEventListener('keypress', InputPushTextEvent, false)
  window.addEventListener('mouseout', InputPushMovedEvent, false)
  window.addEventListener('resize', resizeViewport, false)

  Q3e.canvas.addEventListener('mousemove', InputPushMouseEvent, false)
  Q3e.canvas.addEventListener('mousedown', InputPushMouseEvent, false)
  Q3e.canvas.addEventListener('mouseup', InputPushMouseEvent, false)
  
  document.addEventListener('mousewheel', InputPushWheelEvent, {capture: false, passive: true})
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
  if(document.exitPointerLock)
  document.exitPointerLock()
  else if (document.webkitExitPointerLock)
  document.webkitExitPointerLock()
  else if (document.mozExitPointerLock)
  document.mozExitPointerLock()
}

function GLimp_Shutdown(destroy) {
  window.removeEventListener('resize', resizeViewport)
  window.removeEventListener('keydown', InputPushKeyEvent)
  window.removeEventListener('keyup', InputPushKeyEvent)
  window.removeEventListener('keypress', InputPushTextEvent)

  document.removeEventListener('mousewheel', InputPushWheelEvent)
  document.removeEventListener('visibilitychange', InputPushFocusEvent)
  //document.removeEventListener('drop', dropHandler)
  //document.removeEventListener('dragenter', SYSI.dragEnterHandler)
  //document.removeEventListener('dragover', SYSI.dragOverHandler)

  if (destroy && Q3e.canvas) {
    Q3e.canvas.removeEventListener('mousemove', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mousedown', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mouseup', InputPushMouseEvent)
    GL.deleteContext(Q3e.webgl);
    Q3e.canvas.remove()
    delete Q3e['canvas']
  }
}

var INPUT = {
  keystrings: {},
  firstClick: true,
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
    HEAP32[(width+0)>>2] = Q3e.canvas.width
    HEAP32[(height+0)>>2] = Q3e.canvas.height
  }
}

