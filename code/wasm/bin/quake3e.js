
window.Q3e = {
  DebugBreak: function () {
    debugger;
    debugger;
  }
}


async function init() {
  fetch('./quake3e_slim.wasm').then(function(response) {
    return response.arrayBuffer()
  }).then(function(bytes) {
    return WebAssembly.instantiate(bytes, { env: Q3e })
  }).then(function(program) {
    console.log(program.instance.exports)
    return program.instance.exports.exported_func()
  });
}

init()
