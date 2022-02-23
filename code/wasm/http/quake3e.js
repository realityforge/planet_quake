if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

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
		crypto.getRandomValues(Q3e.paged.subarray(string, string+(len / 4)))
	} else {
		for(let i = 0; i < (len / 4); i++) {
			Q3e.paged[string] = Math.random() * 255
		}
	}
	return true;
}

function Sys_Print(message) {
  console.log(addressToString(message))
}

function Sys_Error(fmt, args) {
  let len = BG_sprintf(Q3e.sharedMemory + Q3e.sharedCounter, fmt, args)
  if(len > 0)
    console.log('Sys_Error: ', addressToString(Q3e.sharedMemory + Q3e.sharedCounter))
  throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status) {
  // TODO: something like  window.title = , then setTimeout( window.title = 'Q3e' again)
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
  div.innerHTML = desc.replace('%s', addressToString(arguments[1]))
  if(description.children.length == 0
    || div.innerText.toLowerCase() != description
      .children[description.children.length-1].innerText.toLowerCase())
    description.appendChild(div)
}

var SYS = {
  DebugError: function () { console.log(new Error('debug').stack) },
  DebugBreak: function () { debugger; },
  Sys_RandomBytes: Sys_RandomBytes,
  Sys_Milliseconds: Sys_Milliseconds,
  Sys_Microseconds: Sys_Microseconds,
  Sys_Exit: function () {}, // TODO: redirect to lvlworld, was Sys_Main_PlatformExit
  Sys_Error: Sys_Error,
  Sys_UnloadLibrary: Sys_UnloadLibrary,
  Sys_LoadLibrary: Sys_LoadLibrary,
  Sys_LoadFunction: Sys_LoadFunction,
  popen: function popen() {},
  Sys_Print: Sys_Print,
  Sys_SetStatus: Sys_SetStatus,
  
}

function startProgram(program) {
  // share the game with window for hackers
  Q3e['program'] = program
  Q3e['instance'] = program.instance
  Q3e['exports'] = program.instance.exports
  let newMethods = Object.keys(Q3e['exports'])
  for(let i = 0; i < newMethods.length; i++) {
    window[newMethods[i]] = Q3e['exports'][newMethods[i]] //.apply(Q3e['exports'])
  }
  if(typeof Q3e.exports['Z_Malloc'] == 'undefined') {
    window.Z_Malloc = Q3e.exports['Z_MallocDebug']
  }
  Object.assign(window, Q3e['exports'])

  // reserve some memory at the beginning for passing shit back and forth with JS
  //   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
  //   something isn't cleared out, crash
  //Q3e['inputMemory'] = malloc(1024 * 1024) // store inputs in it's own space
  Q3e['glProcAddresses'] = malloc(1024 * 4) // store function points between plugins
  Q3e['sharedMemory'] = malloc(1024 * 1024) // store some strings and crap
  Q3e['sharedCounter'] = 0

  // Wow, look at all the unfuckery I don't have to do with startup options because
  //   I'm not using emscripten anymore.
  let startup = [
    'quake3e_web',
    '+set', 'developer', '1',
    '+set', 'fs_basepath', '/base',
    '+set', 'fs_homepath', '/home',
    '+set', 'sv_pure', '0', // require for now, TODO: server side zips
    '+set', 'fs_basegame', 'multigame',
    '+set', 'cl_dlURL', '"http://local.games:8080/multigame"',

  ];

  // start a brand new call frame, in-case error bubbles up
  setTimeout(function () {
    try {
      // Startup args is expecting a char **
      RunGame(startup.length, stringsToMemory(startup))
      setInterval(function () {
        requestAnimationFrame(function () {
          try {
            Q3e.exports.Sys_Frame()
          } catch (e) {
            console.log(e)
          }
        })
      }, 1000 / 80);
    } catch (e) {
      console.log(e)
    }
  }, 13)
  return true
}


function instantiateWasm(bytes) {
  Q3e['imports'] = Q3e
  let libraries = {
    env: Q3e,
    SYS: SYS,
    GL: GLEmulation,
    MATH: MATHS,
    FS: FS,
    NET: NET,
    DATE: DATE,
    INPUT: INPUT,
    STD: STD,
  }
  // assign everything to env because this bullshit don't work
  Object.assign(Q3e, libraries)
  for(let i = 0; i < Object.keys(libraries).length; i++) {
    Object.assign(Q3e.env, Object.values(libraries)[i])
  }

  Q3e['table'] = Q3e['__indirect_function_table'] =
    new WebAssembly.Table({ initial: 1024, element: 'anyfunc' });
  Q3e['memory'] = new WebAssembly.Memory( {
    'initial': 2048,
    //'shared': true
  } )
  Q3e['paged'] = new Uint8Array( Q3e['memory'].buffer )
  Q3e['paged16'] = new Uint16Array( Q3e['memory'].buffer )
  Q3e['paged32'] = new Uint32Array( Q3e['memory'].buffer )
  Q3e['paged32f'] = new Float32Array( Q3e['memory'].buffer )
  Q3e['paged64f'] = new Float64Array( Q3e['memory'].buffer )
  return WebAssembly.instantiate(bytes, Q3e)
}


function init() {

  // TODO: bootstrap download function so it saves binary to disk
  fetch('./quake3e_mv.wasm').then(function(response) {
    if(response.status == 404) {
      return fetch('./quake3e_slim.wasm').then(function(response) {
        if(response.status == 404) {
          return fetch('./quake3e.wasm')
            .then(function(response2) { return response2.arrayBuffer() })
        }
        return response.arrayBuffer()
      })
    }
    return response.arrayBuffer()
  })
    .then(instantiateWasm)
    // TODO: change when hot reloading works
    .then(startProgram);
}


init()
