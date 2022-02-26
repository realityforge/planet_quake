// Launcher program for web browser and .wasm builds

function startProgram(program) {
  // share the game with window for hackers
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

  // reserve some memory at the beginning for passing shit back and forth with JS
  //   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
  //   something isn't cleared out, crash
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
            window.Sys_Frame()
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
