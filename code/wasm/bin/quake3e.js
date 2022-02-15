function getLEB(binary) {
  var next = 0;
  var ret = 0;
  var mul = 1;
  while (1) {
    var byte = binary[next++];
    ret += ((byte & 0x7f) * mul);
    mul *= 0x80;
    if (!(byte & 0x80)) break;
  }
  return [ret, next];
}

window.Q3e = {
}

function init() {
  fetch('./quake3e_slim.wasm').then(function(response) {
    return response.arrayBuffer()
  }).then(function(bytes) {
    var currentSection = 8;
    var data = new Uint8Array(bytes);
    while(currentSection < data.length) {
      var type = data[currentSection]
      console.log(type)
      if(type > 12) {
        throw new Error('goddamnit')
      }
      var size = getLEB(Array.from(data.slice(currentSection + 1, currentSection + 8)))
      if(size[0] < 0) {
        throw new Error('goddamnit')
      }
      if(currentSection > 1024 * 1024) {
        debugger;
        break;
      }
      currentSection += size[0] + size[1] + 1;
    }
    return WebAssembly.instantiate(bytes, { env: Q3e })
  }).then(function(program) {
    console.log(program.instance.exports)
    return program.instance.exports.exported_func()
  });
}

init()
