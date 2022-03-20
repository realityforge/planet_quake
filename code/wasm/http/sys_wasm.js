// Launcher program for web browser and .wasm builds

function startProgram(program) {
  // share the game with window for hackers
  if(!program) {
    throw new Error("no program!")
  }
  Q3e['program'] = program
  Q3e['instance'] = program.instance
  Q3e['exports'] = program.instance.exports
  let newMethods = Object.keys(Q3e['exports'])
  for(let i = 0; i < newMethods.length; i++) {
    window[newMethods[i]] = Q3e['exports'][newMethods[i]] //.apply(Q3e['exports'])
  }
  if(typeof window['Z_Malloc'] == 'undefined') {
    window.Z_Malloc = window['Z_MallocDebug']
  }
  Object.assign(window, Q3e['exports'])

  // start a brand new call frame, in-case error bubbles up
  setTimeout(function () {
    try {
      // reserve some memory at the beginning for passing shit back and forth with JS
      //   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
      //   something isn't cleared out, crash
      Q3e['sharedMemory'] = malloc(1024 * 1024) // store some strings and crap
      Q3e['sharedCounter'] = 0
      Q3e['exited'] = false

      // Startup args is expecting a char **
      let startup = getQueryCommands()
      RunGame(startup.length, stringsToMemory(startup))
      // should have Cvar system by now
      INPUT.fpsUnfocused = Cvar_VariableIntegerValue(stringToAddress('com_maxfpsUnfocused'));
      INPUT.fps = Cvar_VariableIntegerValue(stringToAddress('com_maxfps'))
      // this might help prevent this thing that krunker.io does where it lags when it first starts up
      Q3e.frameInterval = setInterval(Sys_Frame, 
        1000 / (HEAP32[gw_active >> 2] ? INPUT.fps : INPUT.fpsUnfocused));
    } catch (e) {
      console.log(e)
      Sys_Exit(1)
      throw e
    }
  }, 13)
  return true
}


function instantiateWasm(bytes) {
  let libraries = {
    env: Q3e,
    SYS: SYS,
    GL: EMGL,
    MATH: MATHS,
    FS: FS,
    NET: NET,
    DATE: DATE,
    INPUT: INPUT,
    STD: STD,
  }
  if(!bytes) {
    throw new Error('Couldn\'t find wasm!')
  }
  // assign everything to env because this bullshit don't work
  Object.assign(Q3e, libraries)
  for(let i = 0; i < Object.keys(libraries).length; i++) {
    Object.assign(Q3e.env, Object.values(libraries)[i])
  }

  Q3e['table'] = Q3e['__indirect_function_table'] =
    new WebAssembly.Table({ initial: 2048, element: 'anyfunc' })
  Q3e['memory'] = new WebAssembly.Memory({ 'initial': 2048, /* 'shared': true */ })
  updateGlobalBufferAndViews(Q3e.memory.buffer)
  return WebAssembly.instantiate(bytes, Q3e)
}


function init() {
  Q3e['imports'] = Q3e
  Q3e['cacheBuster'] = Date.now()

  // TODO: bootstrap download function so it saves binary to disk
  fetch('./quake3e_mv.wasm?time=' + Q3e.cacheBuster)
  .catch(function (e) {})
  .then(function(response) {
    if(!response || response.status == 404) {
      return fetch('./quake3e_slim.wasm?time=' + Q3e.cacheBuster)
    }
  })
  .catch(function (e) {})
  .then(function(response) {
    if(!response || response.status == 404) {
      return fetch('./quake3e.wasm?time=' + Q3e.cacheBuster)
    }
  })
  .catch(function (e) {})
  .then(function (response) {
    if(response && response.status == 200) {
      return response.arrayBuffer()
    }
  })
    .then(instantiateWasm)
    // TODO: change when hot reloading works
    .then(startProgram);
}

window.addEventListener('load', function () {
  if(typeof Q3e.imports == 'undefined') {
    init()
  }
}, false)
