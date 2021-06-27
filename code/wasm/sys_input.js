var global = window || global
global['SYSI'] = {
  menus: {
    'baseq3': {
      'ARENASERVERS': 'multiplayer',
      'CHOOSELEVEL': 'singleplayer',
      'SETUP': 'setup',
      'PLAYERSETTINGS': 'player',
      'CONTROLS': 'controls',
      'SYSTEMSETUP': 'system',
      'GAMEOPTIONS': 'options',
      'CDKEY': 'cdkey',
      'CINEMATICS': 'cinematics',
      'MODS': 'mods',
      'GAMESERVER': 'create',
    }
  },
  propMapB: [
    [11, 12, 33],
    [49, 12, 31],
    [85, 12, 31],
    [120, 12, 30],
    [156, 12, 21],
    [183, 12, 21],
    [207, 12, 32],

    [13, 55, 30],
    [49, 55, 13],
    [66, 55, 29],
    [101, 55, 31],
    [135, 55, 21],
    [158, 55, 40],
    [204, 55, 32],

    [12, 97, 31],
    [48, 97, 31],
    [82, 97, 30],
    [118, 97, 30],
    [153, 97, 30],
    [185, 97, 25],
    [213, 97, 30],

    [11, 139, 32],
    [42, 139, 51],
    [93, 139, 32],
    [126, 139, 31],
    [158, 139, 25],
  ],
  resizeDelay: 0,
  cancelBackspace: true,
  banner: '',
  bannerTime: 0,
  joysticks: [],
  inputInterface: 0,
  inputHeap: 0,
  superKey: 0,
  paste: '',
  field: 0,
  //inputCount: 0,
  canvas: null,
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
    SYSI.paste = text.value
    Module.viewport.focus()
    if(SYSI.field) {
      SYSI.paste.split('').forEach(function (k) {
        _JS_Field_CharEvent(SYSI.field, k.charCodeAt(0))
      })
      SYSI.paste = ''
      SYSI.field = 0
      text.remove()
    }
  }, 100)
}

function checkPasteEvent (evt) {
  if(evt.key == 'Meta') SYSI.superKey = evt.type == 'keydown'
  if(SYSI.superKey && (evt.key == 'v' || evt.key == 'V')) {
    if(SYSI.superKey && (evt.type == 'keypress' || evt.type == 'keydown')) {
      SYSI.captureClipBoard()
    }
  }
}

function InputPushKeyEvent (evt) {
  var event = SYSI.inputHeap
  if(evt.keyCode === 8) {
    SYSI.cancelBackspace = true;
    setTimeout(function () { SYSI.cancelBackspace = false }, 100)
  }
  if(evt.keyCode === 27) {
    SYSI.cancelBackspace = true;
    SYSI.InputPushFocusEvent({visible: false})
  }
  
  SYSI.checkPasteEvent(evt)
  var modState = (evt.ctrlKey && evt.location === 1 ? 0x0040 : 0)
    | (evt.shiftKey && evt.location === 1 ? 0x0001 : 0)
    | (evt.altKey && evt.location === 1 ? 0x0100 : 0)
    | (evt.ctrlKey && evt.location === 2 ? 0x0080 : 0)
    | (evt.shiftKey && evt.location === 2 ? 0x0002 : 0)
    | (evt.altKey && evt.location === 2 ? 0x0200 : 0)
  
  HEAP32[((event+0)>>2)]= evt.type == 'keydown' ? 0x300 : 0x301 //Uint32 type; ::SDL_KEYDOWN or ::SDL_KEYUP
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds()
  HEAP32[((event+8)>>2)]=0 // windowID
  HEAP32[((event+12)>>2)]=(1 << 2) + (evt.repeat ? 1 : 0) // ::SDL_PRESSED or ::SDL_RELEASED
  
  var key = SDL.lookupKeyCodeForEvent(evt)
  var scan
  if (key >= 1024) {
    scan = key - 1024
    key = scan | 0x40000000
  } else {
    scan = SDL.scanCodes[key] || key
  }

  HEAP32[((event+16)>>2)]=scan
  HEAP32[((event+20)>>2)]=key
  HEAP32[((event+24)>>2)]=modState
  HEAP32[((event+28)>>2)]=0
  if(evt.type == 'keydown')
    _IN_PushEvent(SYSI.inputInterface[0], event)
  if(evt.type == 'keyup')
    _IN_PushEvent(SYSI.inputInterface[1], event)
}

function InputPushTextEvent (evt) {
  var event = SYSI.inputHeap
  SYSI.checkPasteEvent(evt)
  HEAP32[((event+0)>>2)]= 0x303; //Uint32 type; ::SDL_TEXTINPUT
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds()
  HEAP32[((event+8)>>2)]=0; // windowID
  var text = intArrayFromString(String.fromCharCode(evt.charCode))
  var j = 0
  for (var i = 12; i < 24; i+=4) {
    HEAP32[((event+i)>>2)]=text[j]
    j++
  }
  _IN_PushEvent(SYSI.inputInterface[2], event)
}

function InputPushMouseEvent (evt) {
  var event = SYSI.inputHeap
  if (evt.type != 'mousemove') {
    SYSI.cancelBackspace = false
    var down = evt.type == 'mousedown'
    HEAP32[((event+0)>>2)]=down ? 0x401 : 0x402
    HEAP32[((event+4)>>2)]=_Sys_Milliseconds() // timestamp
    HEAP32[((event+8)>>2)]=0 // windowid
    HEAP32[((event+12)>>2)]=0 // mouseid
    HEAP32[((event+16)>>2)]=((down ? 1 : 0) << 8) + (evt.button+1) // DOM buttons are 0-2, SDL 1-3
    HEAP32[((event+20)>>2)]=evt.pageX
    HEAP32[((event+24)>>2)]=evt.pageY
  } else {
    HEAP32[((event+0)>>2)]=0x400
    HEAP32[((event+4)>>2)]=_Sys_Milliseconds()
    HEAP32[((event+8)>>2)]=0
    HEAP32[((event+12)>>2)]=0
    HEAP32[((event+16)>>2)]=SDL.buttonState
    HEAP32[((event+20)>>2)]=evt.pageX
    HEAP32[((event+24)>>2)]=evt.pageY
    HEAP32[((event+28)>>2)]=Browser.getMovementX(evt)
    HEAP32[((event+32)>>2)]=Browser.getMovementY(evt)
  }
  if (evt.type == 'mousemove')
    _IN_PushEvent(SYSI.inputInterface[3], event)
  else
    _IN_PushEvent(SYSI.inputInterface[4], event)
}

function InputPushWheelEvent (evt) {
  var event = SYSI.inputHeap
  HEAP32[((event+0)>>2)]=0x403;
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds(); // timestamp
  HEAP32[((event+8)>>2)]=0; // windowid
  HEAP32[((event+12)>>2)]=0; // mouseid
  HEAP32[((event+16)>>2)]=evt.deltaX;
  HEAP32[((event+20)>>2)]=evt.deltaY;
  _IN_PushEvent(SYSI.inputInterface[5], event)
}

function InputPushTouchEvent (joystick, id, evt, data) {
  var event = SYSI.inputHeap
  SYSI.cancelBackspace = false
  if(id == 1) {
    if (data.vector && data.vector.y > .4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 87})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 87})
    }
    if (data.vector && data.vector.y < -.4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 83})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 83})
    }
    if (data.vector && data.vector.x < -.4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 65})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 65})
    }
    if (data.vector && data.vector.x > .4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 68})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 68})
    }
  }
  
  if(id == 2) {
    if (data.vector && data.vector.y > .4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 40})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 40})
    }
    if (data.vector && data.vector.y < -.4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 38})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 38})
    }
    if (data.vector && data.vector.x < -.4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 37})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 37})
    }
    if (data.vector && data.vector.x > .4) {
      SYSI.InputPushKeyEvent({type: 'keydown', repeat: true, keyCode: 39})
    } else {
      SYSI.InputPushKeyEvent({type: 'keyup', keyCode: 39})
    }
  }

  var w = SYSI.canvas.width;
  var h = SYSI.canvas.height;
  var dx = data.angle ? (Math.cos(data.angle.radian) * data.distance) : 0
  var dy = data.angle ? (Math.sin(data.angle.radian) * data.distance) : 0
  var x = data.angle ? dx : Math.round(data.position.x)
  var y = data.angle ? dy : Math.round(data.position.y)

  HEAP32[((event+0)>>2)]=evt.type == 'start' ? 0x700 : evt.type == 'end' ? 0x701 : 0x702
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds()
  HEAP32[((event+8)>>2)] = id
  HEAP32[((event+12)>>2)] = 0
  HEAP32[((event+16)>>2)] = id
  HEAP32[((event+20)>>2)] = 0
  HEAPF32[((event+24)>>2)]=x / w
  HEAPF32[((event+28)>>2)]=y / h
  HEAPF32[((event+32)>>2)]=dx / w
  HEAPF32[((event+36)>>2)]=dy / h
  if (data.force !== undefined) {
    HEAPF32[((event+(40))>>2)]=data.force
  } else { // No pressure data, send a digital 0/1 pressure.
    HEAPF32[((event+(40))>>2)]=data.type == 'end' ? 0 : 1
  }
  _IN_PushEvent(SYSI.inputInterface[6], event)
}

function InputPushMovedEvent (evt) {
  // Source: https://stackoverflow.com/a/18717721/8037972
  if (evt.toElement === null && evt.relatedTarget === null) {
    //if outside the window...
    if(SYSI.interval)
      SYSI.interval = setInterval(function () {
        //do something with evt.screenX/evt.screenY
      }, 250);
  } else {
    //if inside the window...
    clearInterval(SYSI.interval);
    var event = SYSI.inputHeap
    HEAP32[((event+0)>>2)]=0x200;
    HEAP32[((event+4)>>2)]=_Sys_Milliseconds(); // timestamp
    HEAP32[((event+8)>>2)]=0; // windowid
    HEAP32[((event+12)>>2)]=0x004; // event id
    // padding?
    HEAP32[((event+16)>>2)]=window.screenX || window.screenLeft;
    HEAP32[((event+20)>>2)]=window.screenY || window.screenTop;
    _IN_PushEvent(SYSI.inputInterface[7], event)
  }
}

function InputPushFocusEvent (evt) {
  var event = SYSI.inputHeap
  HEAP32[((event+0)>>2)]=0x200;
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds(); // timestamp
  HEAP32[((event+8)>>2)]=0; // windowid
  if (document.visibilityState === 'visible'
    & (typeof evt.visible === 'undefined' || evt.visible !== false)) {
    HEAP32[((event+12)>>2)]=0x00C; // event id
  } else {
    HEAP32[((event+12)>>2)]=0x00D; // event id
  }
  // padding?
  HEAP32[((event+16)>>2)]=0;
  HEAP32[((event+20)>>2)]=0;
  _IN_PushEvent(SYSI.inputInterface[7], event)
}

function InputPushDropEvent (filename) {
  var event = SYSI.inputHeap
  HEAP32[((event+4)>>2)]=_Sys_Milliseconds(); // timestamp
  HEAP32[((event+12)>>2)]=0; // windowid
  if(typeof filename == 'string') {
    intArrayFromString(filename).forEach(function (c, i) { HEAP8[(SYSF.pathname+i)] = c })
    HEAP8[(SYSF.pathname+filename.length)] = 0;
    HEAP32[((event+8)>>2)]=SYSF.pathname; // filename
    HEAP32[((event+0)>>2)]=0x1000;
    _IN_PushEvent(SYSI.inputInterface[8], event)
  } else if(filename === true) {
    HEAP32[((event+0)>>2)]=0x1002;
    HEAP8[(SYSF.pathname)] = 0;
    HEAP32[((event+8)>>2)]=SYSF.pathname; // filename
    _IN_PushEvent(SYSI.inputInterface[8], event)
  } else if (filename === false) {
    HEAP32[((event+0)>>2)]=0x1003;
    HEAP8[(SYSF.pathname)] = 0;
    HEAP32[((event+8)>>2)]=SYSF.pathname; // filename
    _IN_PushEvent(SYSI.inputInterface[8], event)
  }
}

function InputInit () {
  // TODO: clear JSEvents.eventHandlers
  _IN_PushInit(SYSI.inputHeap)
  SYSI.inputInterface = []
  for(var ei = 0; ei < 20; ei++) {
    SYSI.inputInterface[ei] = getValue(SYSI.inputHeap + 4 * ei, 'i32', true)
  }
  window.addEventListener('keydown', SYSI.InputPushKeyEvent, false)
  window.addEventListener('keyup', SYSI.InputPushKeyEvent, false)
  window.addEventListener('keypress', SYSI.InputPushTextEvent, false)
  window.addEventListener('mouseout', SYSI.InputPushMovedEvent, false)

  SYSI.canvas.addEventListener('mousemove', SYSI.InputPushMouseEvent, false)
  SYSI.canvas.addEventListener('mousedown', SYSI.InputPushMouseEvent, false)
  SYSI.canvas.addEventListener('mouseup', SYSI.InputPushMouseEvent, false)
  
  document.addEventListener('mousewheel', SYSI.InputPushWheelEvent, {capture: false, passive: true})
  document.addEventListener('visibilitychange', SYSI.InputPushFocusEvent, false)
  document.addEventListener('drop', SYSI.dropHandler, false)
  document.addEventListener('dragenter', SYSI.dragEnterHandler, false)
  document.addEventListener('dragover', SYSI.dragOverHandler, false)
  //document.addEventListener('pointerlockchange', SYSI.InputPushFocusEvent, false);
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

function updateVideoCmd () {
  if(!SYSI.canvas) return
  var oldHeight = SYSI.canvas.getAttribute('height')
  var oldWidth = SYSI.canvas.getAttribute('width')
  // only update size if the canvas changes by more than 0.1
  if(!((SYSI.canvas.clientWidth / SYSI.canvas.clientHeight) - (oldWidth / oldHeight) > 0.1
    || (SYSI.canvas.clientWidth / SYSI.canvas.clientHeight) - (oldWidth / oldHeight) < -0.1))
    return
  SYSI.canvas.setAttribute('width', canvas.clientWidth)
  SYSI.canvas.setAttribute('height', canvas.clientHeight)
  var update = 'set r_fullscreen %fs; set r_mode -1;'
    + ' set r_customWidth %w; set r_customHeight %h;'
    + ' set cg_gunX %i; cg_gunZ %i; vid_restart;'
    .replace('%fs', window.fullscreen ? '1' : '0')
    .replace('%w', SYSI.canvas.clientWidth)
    .replace('%h', SYSI.canvas.clientHeight)
    .replace('%i', (SYSI.canvas.clientWidth / SYSI.canvas.clientHeight) < 0.8
      ? -5 : 0)

  _Cbuf_AddText(allocate(intArrayFromString(update), ALLOC_STACK))
}

function resizeViewport () {
  // ignore if the canvas hasn't yet initialized
  if(!SYSI.canvas) return

  if (SYSI.resizeDelay) clearTimeout(SYSI.resizeDelay)
  SYSI.resizeDelay = setTimeout(SYSI.updateVideoCmd, 100);
}

function dropHandler (ev) {
  var files = []
  // Prevent default behavior (Prevent file from being opened in a new tab)
  ev.preventDefault();
  var handleFile
  handleFile = function (file, done) {
    var newPath
    if(file.name.match(/\.svdm_/ig)) {
      newPath = PATH.join(SYSF.fs_basepath, SYSF.fs_game, 'svdemos', file.name)
    } else if (file.name.match(/\.dm_/ig)) {
      newPath = PATH.join(SYSF.fs_basepath, SYSF.fs_game, 'demos', file.name)
    } else {
      newPath = PATH.join(SYSF.fs_basepath, SYSF.fs_game)
    }
    Sys_mkdirp(PATH.dirname(newPath))
    var reader = new FileReader();
    reader.onload = function(e) {
      FS.writeFile(newPath, new Uint8Array(e.target.result), {
        encoding: 'binary', flags: 'w', canOwn: true })
      SYSI.InputPushDropEvent(file.name)
      if(files.length) {
        handleFile(files.pop())
      } else {
        done()
      }
    }
    reader.readAsArrayBuffer(file)
  }

  if (ev.dataTransfer.items) {
    // Use DataTransferItemList interface to access the file(s)
    for (var i = 0; i < ev.dataTransfer.items.length; i++) {
      // If dropped items aren't files, reject them
      if (ev.dataTransfer.items[i].kind === 'file') {
        var file = ev.dataTransfer.items[i].getAsFile();
        files.push(file)
      }
    }
  } else {
    // Use DataTransfer interface to access the file(s)
    for (var i = 0; i < ev.dataTransfer.files.length; i++) {
      files.push(file)
    }
  }
  
  handleFile(files.pop(), function () {
    FS.syncfs(false, function () {
      SYSI.InputPushDropEvent(false)
    })
  })
}

function dragEnterHandler (ev) {
  SYSI.InputPushDropEvent(true)
  ev.preventDefault();
}

function dragOverHandler (ev) {
  ev.preventDefault();
}

function Sys_GLimpInit (initFlags) {
  var in_joystick = SYSC.Cvar_VariableIntegerValue('in_joystick')
  var developer = SYSC.Cvar_VariableIntegerValue('developer')
  SDL.startTime = Date.now();
  SDL.initFlags = initFlags;

  if(!SYSI.inputHeap)
    SYSI.inputHeap = allocate(new Int32Array(60>>2), ALLOC_NORMAL)

	var viewport = document.getElementById('viewport-frame')
	// create a canvas element at this point if one doesnt' already exist
	if (!SYSI.canvas) {
		SYSI.canvas = document.createElement('canvas')
		SYSI.canvas.id = 'canvas'
		SYSI.canvas.width = viewport.offsetWidth
		SYSI.canvas.height = viewport.offsetHeight
		viewport.appendChild(SYSI.canvas)
	}
  window.addEventListener('beforeunload', function (e) {
    _S_DisableSounds();
    if(SYSI.cancelBackspace) {
      e.preventDefault();
      e.returnValue = 'Do you really want to quit?';
      return e.returnValue
    } else {
      delete e.returnValue
    }
  })
  window.addEventListener('resize', SYSI.resizeViewport)

  if(in_joystick) {
    SYSI.InitNippleJoysticks()
  }
  SYSI.InputInit()
  
  if(!developer) return
  
  function throwOnGLError(err, funcName, args) {
    //console.error(WebGLDebugUtils.glEnumToString(err) + " was caused by call to: " + funcName)
  }
  function logGLCall(functionName, args) {   
    console.log("gl." + functionName + "(" + 
        WebGLDebugUtils.glFunctionArgsToString(functionName, args) + ")") 
  }
  //GLctx = WebGLDebugUtils.makeDebugContext(GLctx, throwOnGLError /*, logGLCall */)

}

function Sys_GLimpSafeInit () {
}

function Sys_GetClipboardData () {
	return 0;
}

function Sys_Input_SetClipboardData (field) {
  SYSI.field = field
}

function Sys_FocusInput () {
  
}

function Sys_EventMenuChanged (x, y) {
  var milli = _Sys_Milliseconds()
  if(milli - SYSI.bannerTime > 1000) {
    SYSI.banner = ''
    SYSI.bannerTime = milli
  } else if (milli < SYSI.bannerTime) return
  var found = false
  var index = 0
  SYSI.propMapB.forEach(function (coords, i) {
    if(coords[0] == Math.round(x * 256.0) && coords[1] == Math.round(y * 256.0)) {
      found = true
      index = i
    }
  })
  if(found) SYSI.banner += String.fromCharCode('A'.charCodeAt(0)+index)
  if(typeof history != 'undefined') {
    var url = Object.keys(SYSI.menus['baseq3']).filter(function (m) {
      return SYSI.banner.includes(m)
    })[0]
    if(url) {
      //history.pushState({location: window.location.toString()}, window.title, SYSI.menus['baseq3'][url])
      // deflood
      SYSI.banner = ''
      SYSI.bannerTime += 1000
    }
  }
}
