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
    //HEAP32[((event+16)>>2)]=((down ? 1 : 0) << 8) + (evt.button+1) 
    //HEAP32[((event+20)>>2)]=evt.pageX
    //HEAP32[((event+24)>>2)]=evt.pageY
  } else {
		Com_QueueEvent( Sys_Milliseconds(), SE_MOUSE, 
      Browser.getMovementX(evt), Browser.getMovementY(evt), 0, NULL );
  }
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

function Sys_GetClipboardData () {
	return 0;
}

function Sys_SetClipboardData (field) {
  SYSI.field = field
}

function Sys_FocusInput () {
  
}
