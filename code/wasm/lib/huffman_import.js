var fs = require('fs')

async function init() {
  var binary = new Uint8Array(fs.readFileSync('./build/debug-js-js/huffman_js.wasm'))
  let imports = {};
  imports['memory'] = new WebAssembly['Memory']( {'initial': 32} )
  memory = new Uint8Array( imports['memory']['buffer'] )
  let program = await WebAssembly.instantiate(binary, { env: imports })
  console.log(program.instance.exports)
}

init()
