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
    stencil: true,
    premultipliedAlpha: false,
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
    //if(typeof Module != 'undefined') {
    //  Module.useWebGL = true;
    //}
    //GLImmediate.clientColor = new Float32Array([ 1, 1, 1, 1 ]);
    //GLImmediate.init()
    //GLImmediate.getRenderer()
  }

  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  if(!INPUT.firstClick) {
    //Q3e.canvas.requestPointerLock();
  } else {
    SDL_ShowCursor()
  }

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
const	SE_JOYSTICK_AXIS = 6	// evValue is an axis number and evValue2 is the current state (-127 to 127)
const	SE_CONSOLE = 7	// evPtr is a char*
const	SE_MOUSE_ABS = 8
//#ifdef USE_DRAGDROP
const	SE_DROPBEGIN = 9
const	SE_DROPCOMPLETE = 10
const	SE_DROPFILE = 11
const	SE_DROPTEXT = 12
const	SE_MAX = 13

//#endif
//} sysEventType_t;


const KEYCATCH_CONSOLE = 0x0001
const KEYCATCH_UI      =  0x0002
const KEYCATCH_MESSAGE =  0x0004
const KEYCATCH_CGAME   =  0x0008

function InputPushFocusEvent (evt) {
  if(evt.type == 'pointerlockchange') {
    HEAP32[gw_active >> 2] = (document.pointerLockElement === Q3e.canvas)
    return
  }
  if (document.visibilityState != 'visible' || evt.type == 'blur') {
    Key_ClearStates();
    HEAP32[gw_active >> 2] = false;
  } else {
    Key_ClearStates();
    HEAP32[gw_active >> 2] = true;
    HEAP32[gw_minimized >> 2] = false;
  }
}


function InputPushMovedEvent (evt) {
  if (evt.toElement === null && evt.relatedTarget === null) {
    INPUT.firstClick = true
    if(Q3e.frameInterval) {
      clearInterval(Q3e.frameInterval)
    }
    Q3e.frameInterval = setInterval(Sys_Frame, 1000.0 / INPUT.fpsUnfocused)
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

/*
document.querySelector('element').bind('copy', function(event) {
  var selectedText = window.getSelection().toString(); 
  selectedText = selectedText.replace(/\u200B/g, '');

  clipboardData = event.clipboardData || window.clipboardData || event.originalEvent.clipboardData;
  clipboardData.setData('text/html', selectedText);

  event.preventDefault();
});
*/

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
    document.body.focus()
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
    SDL_ShowCursor()
    HEAP32[gw_active >> 2] = false
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
  }

  if( evt.ctrlKey && evt.keyCode >= 'a'.charCodeAt(0) && evt.keyCode <= 'z'.charCodeAt(0) ) {
    Sys_QueEvent( Sys_Milliseconds(), SE_CHAR, evt.charCode-('a'.charCodeAt(0))+1, 0, 0, null );
  } else

  if(evt.keyCode >= 65 && evt.keyCode <= 90) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['a'] + (evt.keyCode - 65), evt.type == 'keydown', 0, null );
  } else 

  if(evt.shiftKey && evt.keyCode >= 65 && evt.keyCode <= 90) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['a'] + (evt.keyCode - 65 + 97), evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode >= 48 && evt.keyCode <= 57) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['0'] + (evt.keyCode - 48), evt.type == 'keydown', 0, null );
  } else 

  if(evt.shiftKey && evt.keyCode >= 48 && evt.keyCode <= 57) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings[' '] + (evt.keyCode - 48), evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 37) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['LEFTARROW'], evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 38) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['UPARROW'], evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 39) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['RIGHTARROW'], evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 40) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['DOWNARROW'], evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 16) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['SHIFT'], evt.type == 'keydown', 0, null );
  }

  if(evt.keyCode == 32) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['SPACE'], evt.type == 'keydown', 0, null );
  }

  if(evt.keyCode == 13) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['ENTER'], evt.type == 'keydown', 0, null );
  } else 

  if(evt.keyCode == 27) {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
      INPUT.keystrings['ESCAPE'], evt.type == 'keydown', 0, null );
  }
}

function InputPushTextEvent (evt) {
  if ( INPUT.consoleKeys.includes(String.fromCharCode(evt.charCode)) )
  {
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], true, 0, null )
    Sys_QueEvent( Sys_Milliseconds(), SE_KEY, INPUT.keystrings['CONSOLE'], false, 0, null )
    setTimeout(function () {
      if(Key_GetCatcher() & KEYCATCH_CONSOLE) {
        SDL_ShowCursor()
        HEAP32[gw_active >> 2] = false
      } else if(!INPUT.firstClick) {
        //Q3e.canvas.requestPointerLock();      
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
  let down = evt.type == 'mousedown'

  if(down && INPUT.firstClick) {
    INPUT.firstClick = false
    SNDDMA_Init()

    HEAP32[gw_active >> 2] = 1
    HEAP32[s_soundStarted >> 2] = 1
    HEAP32[s_soundMuted >> 2] = 0
    S_Base_SoundInfo();
  }

  if(!(Key_GetCatcher() & KEYCATCH_CONSOLE)) {
    if (evt.type == 'mousemove') {
      if(Key_GetCatcher() === 0) {
        Sys_QueEvent( Sys_Milliseconds(), SE_MOUSE, 
          getMovementX(evt), getMovementY(evt), 0, null );
      } else {
        Sys_QueEvent( Sys_Milliseconds(), SE_MOUSE_ABS, 
          evt.clientX, evt.clientY, 0, null );
      }
    } else {
      Sys_QueEvent( Sys_Milliseconds(), SE_KEY, 
        INPUT.keystrings['MOUSE1'] + evt.button, down, 0, null );
    }
  }

  // always unlock on menus because it's position absolute now
  if ( Key_GetCatcher() !== 0 ) {
    if(document.pointerLockElement) {
      SDL_ShowCursor()
      // ruins sound //HEAP32[gw_active >> 2] = false
    }
    //INPUT.firstClick = true
    return;
  }

  // TODO: fix this maybe?
  //if(!mouseActive || in_joystick->integer) {
  //  return;
  //}
  // Basically, whenever the requestPointerLock() is finally triggered when cgame starts,
  //   the unfocusedFPS is cancelled and changed to real FPS, 200+!
  if(down && document.pointerLockElement != Q3e.canvas) {
    // TODO: start sound, capture mouse
    HEAP32[gw_active >> 2] = 1
    Q3e.canvas.requestPointerLock();

    if(Q3e.frameInterval) {
      clearInterval(Q3e.frameInterval)
    }
    Q3e.frameInterval = setInterval(Sys_Frame, 1000.0 / INPUT.fps);
  }
}

function Com_MaxFPSChanged() {
  INPUT.fpsUnfocused = Cvar_VariableIntegerValue(stringToAddress('com_maxfpsUnfocused'));
  INPUT.fps = Cvar_VariableIntegerValue(stringToAddress('com_maxfps'));
  if(Q3e.frameInterval) {
    clearInterval(Q3e.frameInterval)
  }
  Q3e.frameInterval = setInterval(Sys_Frame, 1000.0 / (HEAP32[gw_active >> 2] 
    ? INPUT.fps : INPUT.fpsUnfocused))
}

function Sys_ConsoleInput() {
  let command = window.location.hash
  window.location.hash = ''
  return command.length ? stringToAddress(command) : null
}


const CVAR_ARCHIVE = 0x0001
const CVAR_NODEFAULT = 0x4000
const CVAR_LATCH =  0x0020
const CVAR_ARCHIVE_ND = (CVAR_ARCHIVE|CVAR_NODEFAULT)
const CV_INTEGER = 2


function IN_Init() {

	console.log( '\n------- Input Initialization -------\n' );

  if(Cvar_VariableIntegerValue(stringToAddress('in_joystick'))) {
    InitNippleJoysticks()
  }

	let in_keyboardDebug = Cvar_Get( stringToAddress('in_keyboardDebug'), stringToAddress('0'), CVAR_ARCHIVE );
	let in_mouse = Cvar_Get( stringToAddress('in_mouse'), stringToAddress('1'), CVAR_ARCHIVE );
	Cvar_CheckRange( in_mouse, stringToAddress('-1'), stringToAddress('1'), CV_INTEGER );

  /*
	in_joystick = Cvar_Get( 'in_joystick', '0', CVAR_ARCHIVE );
	in_joystickThreshold = Cvar_Get( 'joy_threshold', '0.15', CVAR_ARCHIVE );
	j_pitch =        Cvar_Get( 'j_pitch',        '0.022', CVAR_ARCHIVE_ND );
	j_yaw =          Cvar_Get( 'j_yaw',          '-0.022', CVAR_ARCHIVE_ND );
	j_forward =      Cvar_Get( 'j_forward',      '-0.25', CVAR_ARCHIVE_ND );
	j_side =         Cvar_Get( 'j_side',         '0.25', CVAR_ARCHIVE_ND );
	j_up =           Cvar_Get( 'j_up',           '0', CVAR_ARCHIVE_ND );

	j_pitch_axis =   Cvar_Get( 'j_pitch_axis',   '3', CVAR_ARCHIVE_ND );
	j_yaw_axis =     Cvar_Get( 'j_yaw_axis',     '2', CVAR_ARCHIVE_ND );
	j_forward_axis = Cvar_Get( 'j_forward_axis', '1', CVAR_ARCHIVE_ND );
	j_side_axis =    Cvar_Get( 'j_side_axis',    '0', CVAR_ARCHIVE_ND );
	j_up_axis =      Cvar_Get( 'j_up_axis',      '4', CVAR_ARCHIVE_ND );

	Cvar_CheckRange( j_pitch_axis,   '0', va('%i',MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_yaw_axis,     '0', va('%i',MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_forward_axis, '0', va('%i',MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_side_axis,    '0', va('%i',MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_up_axis,      '0', va('%i',MAX_JOYSTICK_AXIS-1), CV_INTEGER );
  */

	// ~ and `, as keys and characters
	Cvar_Get( stringToAddress('cl_consoleKeys'), stringToAddress('~ ` \u007e \u0060'), CVAR_ARCHIVE );
  if(!INPUT.consoleKeys) {
    INPUT.consoleKeys = addressToString(Cvar_VariableString(stringToAddress('cl_consoleKeys')))
      .split(' ').map(function (c) {
        return c[0] == '0' && c[1] == 'x' 
          ? String.fromCharCode(parseInt(c.substr(2), 16)) 
          : c
      })
  }

	// TODO: activate text input for text fields
	//SDL_StartTextInput();

  //let in_nograb = Cvar_Get( 'in_nograb', '0', CVAR_ARCHIVE );
  //let r_allowSoftwareGL = Cvar_Get( 'r_allowSoftwareGL', '0', CVAR_LATCH );
  //let r_swapInterval = Cvar_Get( 'r_swapInterval', '0', CVAR_ARCHIVE | CVAR_LATCH );
  //let r_stereoEnabled = Cvar_Get( 'r_stereoEnabled', '0', CVAR_ARCHIVE | CVAR_LATCH );

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
  window.addEventListener('popstate', CL_ModifyMenu, false)

  Q3e.canvas.addEventListener('mousemove', InputPushMouseEvent, false)
  Q3e.canvas.addEventListener('mousedown', InputPushMouseEvent, false)
  Q3e.canvas.addEventListener('mouseup', InputPushMouseEvent, false)
  
  document.addEventListener('mousewheel', InputPushWheelEvent, {capture: false, passive: true})
  document.addEventListener('visibilitychange', InputPushFocusEvent, false)
  document.addEventListener('focus', InputPushFocusEvent, false)
  document.addEventListener('blur', InputPushFocusEvent, false)
  //document.addEventListener('drop', SYSI.dropHandler, false)
  //document.addEventListener('dragenter', SYSI.dragEnterHandler, false)
  //document.addEventListener('dragover', SYSI.dragOverHandler, false)

  document.addEventListener('pointerlockchange', InputPushFocusEvent, false)

  /*
  let nipple handle touch events
  SYSI.canvas.addEventListener('touchstart', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchend', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchmove', SYSI.InputPushTouchEvent, false)
  SYSI.canvas.addEventListener('touchcancel', SYSI.InputPushTouchEvent, false)
  */

  console.log( '------------------------------------\n' )
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
  window.removeEventListener('mouseout', InputPushMovedEvent)
  window.removeEventListener('popstate', CL_ModifyMenu)

  document.removeEventListener('mousewheel', InputPushWheelEvent)
  document.removeEventListener('visibilitychange', InputPushFocusEvent)
  document.removeEventListener('focus', InputPushFocusEvent)
  document.removeEventListener('blur', InputPushFocusEvent)
  //document.removeEventListener('drop', dropHandler)
  //document.removeEventListener('dragenter', SYSI.dragEnterHandler)
  //document.removeEventListener('dragover', SYSI.dragOverHandler)
  document.removeEventListener('pointerlockchange', InputPushFocusEvent);

  if (destroy && Q3e.canvas) {
    Q3e.canvas.removeEventListener('mousemove', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mousedown', InputPushMouseEvent)
    Q3e.canvas.removeEventListener('mouseup', InputPushMouseEvent)
    GL.deleteContext(INPUT.handle);
    Q3e.canvas.remove()
    delete Q3e['canvas']
  }
}

var INPUT = {
  keystrings: {},
  firstClick: true,
  IN_Init: IN_Init,
  GLimp_Shutdown: GLimp_Shutdown,
  GLimp_StartDriverAndSetMode:GLimp_StartDriverAndSetMode,
  SDL_WasInit: function (device) { return 1; },
  SDL_StartTextInput: SDL_StartTextInput,
  SDL_StopTextInput: SDL_StopTextInput,
  SDL_ShowCursor: SDL_ShowCursor,
  SDL_SetWindowGrab: SDL_SetWindowGrab,
  Com_MaxFPSChanged: Com_MaxFPSChanged,
  Sys_ConsoleInput: Sys_ConsoleInput,
  IN_FirstClick: function () {
    return INPUT.firstClick
  }
}

