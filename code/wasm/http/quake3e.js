if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

function getQueryCommands() {
  // Wow, look at all the unfuckery I don't have to do with startup options because
  //   I'm not using emscripten anymore.
  let startup = [
    'quake3e_web',
    '+set', 'developer', '0',
    '+set', 'fs_basepath', '/base',
    '+set', 'fs_homepath', '/home',
    '+set', 'sv_pure', '0', // require for now, TODO: server side zips
    '+set', 'fs_basegame', 'multigame',
    '+set', 'cl_dlURL', '"http://local.games:8080/multigame"',
    '+set', 'r_mode', '-1',
    '+set', 'r_ext_framebuffer_object', '0',
    '+set', 'bot_enable', '0',

    //'+set', 'r_ext_multitexture', '0',
    //'+set', 'r_ext_framebuffer_multisample', '0',
    // this prevents lightmap from being wrong when switching maps
    //   renderer doesn't restart between maps, but BSP loading updates
    //   textures with lightmap by default, so this keeps them separate
    //'+set', 'r_mergeLightmaps', '0',
    //'+set', 'r_deluxeMapping', '0',
    //'+set', 'r_normalMapping', '0',
    //'+set', 'r_specularMapping', '0',


  ];
  var search = /([^&=]+)/g
  var query  = window.location.search.substring(1)
  var match
  while (match = search.exec(query)) {
    var val = decodeURIComponent(match[1])
    val = val.split(' ')
    val[0] = (val[0][0] != '+' ? '+' : '') + val[0]
    startup.push.apply(startup, val)
  }
  startup.unshift.apply(startup, [
    '+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
    '+set', 'r_customHeight', '' + window.innerHeight || 0,
    '+set', 'r_customWidth', '' + window.innerWidth || 0,
  ])
  return startup
}

function addressToString(addr, length) {
  let newString = ''
  if(!addr) return newString
  if(!length) length = 1024
  for(let i = 0; i < length; i++) {
    if(HEAP8[addr + i] == 0) {
      break;
    }
    newString += String.fromCharCode(HEAP8[addr + i])
  }

  if(HEAP32[g_bigcharsSize >> 2] == 0) {
    let bigchars = atob(document.getElementById('bigchars').src.substring(22))
      .split("").map(function(c) { return c.charCodeAt(0); })
    let startPos = HEAP32[g_bigcharsData >> 2] = malloc(bigchars.length)
    HEAP32[g_bigcharsSize >> 2] = bigchars.length
    for(let i = 0; i < bigchars.length; i++) {
      HEAP8[startPos] = bigchars[i]
      startPos++
    }
  }

  return newString
}

function stringToAddress(str, addr) {
  let start = Q3e.sharedMemory + Q3e.sharedCounter
  if(addr) start = addr
  for(let j = 0; j < str.length; j++) {
    HEAP8[start+j] = str.charCodeAt(j)
  }
  HEAP8[start+str.length] = 0
  HEAP8[start+str.length+1] = 0
  HEAP8[start+str.length+2] = 0
  if(!addr) {
    Q3e.sharedCounter += str.length + 3
    Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
    if(Q3e.sharedCounter > 1024 * 512) {
      Q3e.sharedCounter = 0
    }
  }
  return start
}


// here's the thing, I know for a fact that all the callers copy this stuff
//   so I don't need to increase my temporary storage because by the time it's
//   overwritten the data won't be needed, should only keep shared storage around
//   for events and stuff that might take more than 1 frame
function stringsToMemory(list, length) {
  // add list length so we can return addresses like char **
  let start = Q3e.sharedMemory + Q3e.sharedCounter
  let posInSeries = start + list.length * 4
  for (let i = 0; i < list.length; i++) {
    HEAP32[(start+i*4)>>2] = posInSeries // save the starting address in the list
    stringToAddress(list[i], posInSeries)
    posInSeries += list[i].length + 1
  }
  if(length) HEAP32[length >> 2] = posInSeries - start
  Q3e.sharedCounter = posInSeries - Q3e.sharedMemory
  Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
  if(Q3e.sharedCounter > 1024 * 512) {
    Q3e.sharedCounter = 0
  }
  return start
}


function Sys_UnloadLibrary() {

}

function Sys_LoadLibrary() {
  
}

function Sys_LoadFunction() {
  
}

function Sys_Microseconds() {
  if (window.performance.now) {
    return parseInt(window.performance.now(), 10);
  } else if (window.performance.webkitNow) {
    return parseInt(window.performance.webkitNow(), 10);
  }

  Q3e.sharedCounter += 8
  return Q3e.sharedMemory + Q3e.sharedCounter - 8
}

function Sys_Milliseconds() {
  if (!Q3e['timeBase']) {
    // javascript times are bigger, so start at zero
    //   pretend like we've been alive for at least a few seconds
    //   I actually had to do this because files it checking times and this caused a delay
    Q3e['timeBase'] = Date.now() - 5000;
  }

  //if (window.performance.now) {
  //  return parseInt(window.performance.now(), 10);
  //} else if (window.performance.webkitNow) {
  //  return parseInt(window.performance.webkitNow(), 10);
  //} else {
  return Date.now() - Q3e.timeBase;
  //}
}

function Sys_RandomBytes (string, len) {
	if(typeof crypto != 'undefined') {
		crypto.getRandomValues(HEAP8.subarray(string, string+(len / 4)))
	} else {
		for(let i = 0; i < (len / 4); i++) {
			HEAP8[string] = Math.random() * 255
		}
	}
	return true;
}

function Sys_Print(message) {
  console.log(addressToString(message))
}

function Sys_Exit(code) {
  if(Q3e.frameInterval) {
    clearInterval(Q3e.frameInterval)
    Q3e.frameInterval = null
  }
  // redirect to lvlworld
  let returnUrl = addressToString(Cvar_VariableString('cl_returnURL'))
  if(returnUrl) {
    window.location = returnUrl
  }
}

function Sys_Error(fmt, args) {
  let len = BG_sprintf(Q3e.sharedMemory + Q3e.sharedCounter, fmt, args)
  if(len > 0)
    console.log('Sys_Error: ', addressToString(Q3e.sharedMemory + Q3e.sharedCounter))
  Sys_Exit( 1 )
  throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status, replacementStr) {
  // TODO: something like  window.title = , then setTimeout( window.title = 'Q3e' again)
  /*
  let desc = addressToString(status)
  if(desc.includes('Main menu')) {
    if(!Q3e.initialized) {
      Q3e.initialized = true
      document.body.className += ' done-loading '
    }
  }
  let description = document.querySelector('#loading-progress .description')
  let div = document.createElement('div')
  // TODO: use BG_sprintf like above?
  if(replacementStr) {
    div.innerHTML = desc.replace('%s', addressToString(replacementStr))
  }
  if(description.children.length == 0
    || div.innerText.toLowerCase() != description
      .children[description.children.length-1].innerText.toLowerCase())
    description.appendChild(div)
  */
}

function CL_MenuModified(oldValue, newValue, cvar) {
  if(INPUT.modifyingCrumb) {
    return // called from ourselves below from a user action
  }
  let newValueStr = addressToString(newValue)
  let newLocation = newValueStr.replace(/[^a-z0-9]/gi, '')
  //if(newValueStr.includes('MAIN MENU')) {
  if(!Q3e.initialized) {
    Q3e.initialized = true
    document.body.className += ' done-loading '
  }
  //}
  if(window.location.pathname.toString().includes(newLocation)) {
    return // don't add to stack because it creates a lot of annoying back pushes
  }
  history.pushState(
    {location: window.location.pathname}, 
    'Quake III Arena: ' + newValueStr, 
    newLocation)
}

function CL_ModifyMenu(event) {
  let oldLocation = window.location.pathname.toString().substring(1) || 'MAIN MENU'
  Cbuf_AddText( stringToAddress(`set ui_breadCrumb "${oldLocation}"\n`) );
}

function Sys_Frame() {
  requestAnimationFrame(function () {
    try {
      Com_Frame(false)
    } catch (e) {
      console.log(e)
    }
  })
}

function alignUp(x, multiple) {
  if (x % multiple > 0) {
   x += multiple - x % multiple;
  }
  return x;
}

function updateGlobalBufferAndViews(buf) {
  Q3e["HEAP8"] = window.HEAP8 = new Int8Array(buf);
  Q3e["HEAP16"] = window.HEAP16 = new Int16Array(buf);
  Q3e["HEAP32"] = window.HEAP32 = new Int32Array(buf);
  Q3e["HEAPU8"] = window.HEAPU8 = new Uint8Array(buf);
  Q3e["HEAPU16"] = window.HEAPU16 = new Uint16Array(buf);
  Q3e["HEAPU32"] = window.HEAPU32 = new Uint32Array(buf);
  Q3e["HEAPF32"] = window.HEAPF32 = new Float32Array(buf);
  Q3e["HEAPF64"] = window.HEAPF64 = new Float64Array(buf);
}
 
function emscripten_realloc_buffer(size) {
  try {
    Q3e.memory.grow(size - Q3e.memory.buffer.byteLength + 65535 >>> 16);
   updateGlobalBufferAndViews(Q3e.memory.buffer);
   return 1;
  } catch (e) {
   console.error("emscripten_realloc_buffer: Attempted to grow heap from " + buffer.byteLength + " bytes to " + size + " bytes, but got error: " + e);
  }
}

function _emscripten_resize_heap(requestedSize) {
  var oldSize = HEAPU8.length;
  requestedSize = requestedSize >>> 0;
  console.assert(requestedSize > oldSize);
  var maxHeapSize = 2147483648;
  if (requestedSize > maxHeapSize) {
   err("Cannot enlarge memory, asked to go up to " + requestedSize + " bytes, but the limit is " + maxHeapSize + " bytes!");
   return false;
  }
  for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
   var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
   overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
   var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), 65536));
   var t0 = Sys_Milliseconds();
   var replacement = emscripten_realloc_buffer(newSize);
   var t1 = Sys_Milliseconds();
   console.log("Heap resize call from " + oldSize + " to " + newSize + " took " + (t1 - t0) + " msecs. Success: " + !!replacement);
   if (replacement) {
    return true;
   }
  }
  err("Failed to grow the heap from " + oldSize + " bytes to " + newSize + " bytes, not enough memory!");
  return false;
}

function _emscripten_get_heap_size() {
  return HEAPU8.length;
 }
 
var SYS = {
  DebugBreak: function () { debugger },
  DebugTrace: function () { console.log(new Error()) },
  emscripten_resize_heap: _emscripten_resize_heap,
  emscripten_get_heap_size: _emscripten_get_heap_size,
  Sys_RandomBytes: Sys_RandomBytes,
  Sys_Milliseconds: Sys_Milliseconds,
  Sys_Microseconds: Sys_Microseconds,
  Sys_Exit: Sys_Exit,
  Sys_Frame: Sys_Frame,
  Sys_Error: Sys_Error,
  Sys_UnloadLibrary: Sys_UnloadLibrary,
  Sys_LoadLibrary: Sys_LoadLibrary,
  Sys_LoadFunction: Sys_LoadFunction,
  popen: function popen() {},
  Sys_Print: Sys_Print,
  Sys_SetStatus: Sys_SetStatus,
  CL_MenuModified: CL_MenuModified,

}
