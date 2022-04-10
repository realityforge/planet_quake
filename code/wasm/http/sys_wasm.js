// Launcher program for web browser and .wasm builds

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

  return WebAssembly.instantiate(bytes, Q3e)
}

function _base64ToArrayBuffer(base64) {
  var binary_string = window.atob(base64);
  var len = binary_string.length;
  var bytes = new Uint8Array(len);
  for (var i = 0; i < len; i++) {
      bytes[i] = binary_string.charCodeAt(i);
  }
  return bytes;
}

function init() {
  window.Module = Q3e
  Q3e['imports'] = Q3e
  // might as well start this early, transfer IndexedDB from disk/memory to application memory
  Q3e['cacheBuster'] = Date.now()
  Q3e['table'] = Q3e['__indirect_function_table'] =
    new WebAssembly.Table({ initial: 1000, element: 'anyfunc', maximum: 10000 })
  Q3e['memory'] = new WebAssembly.Memory({ 'initial': 2048, /* 'shared': true */ })
  updateGlobalBufferAndViews(Q3e.memory.buffer)
  readAll()

  // TODO: offline download so it saves binary to IndexedDB
  if(typeof window.preFS != 'undefined') {
    let preloadedPaths = Object.keys(window.preFS)
    for(let i = 0; i < preloadedPaths.length; i++) {

      FS.virtual[preloadedPaths[i]] = {
        timestamp: new Date(),
        mode: 33206,
        contents: _base64ToArrayBuffer(window.preFS[preloadedPaths[i]])
      }
    }

    if(typeof FS.virtual['quake3e.wasm'] != 'undefined') {
      return Promise.resolve(FS.virtual['quake3e.wasm'].contents)
    }
  }

  return fetch('./quake3e_mv.wasm?time=' + Q3e.cacheBuster)
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
}

// TODO: change when hot reloading works
window.addEventListener('load', function () {
  if(typeof Q3e.imports == 'undefined') {
    init()
    .then(instantiateWasm)
    .then(startProgram);
  }
}, false)
