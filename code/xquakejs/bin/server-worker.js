window = {}
window.serviceWorker = self
window.location = new URL(location.origin + '?set dedicated 1&set ttycon 1&set net_socksEnabled 0')
window.performance = performance
importScripts('quake3e.js')

onmessage = function(e) {
  if(e.data[0] == 'vars') {
    
  } else if(e.data[0] == 'execute') {
    var cmd = allocate(intArrayFromString(e.data[1]), 'i8', ALLOC_STACK);
    _Cbuf_AddText(cmd)
    _Cbuf_Execute()
  } else if(e.data[0] == 'net') {
    
  } else {
    console.log('Command not found ', e.data)
  }
}

Module.onRuntimeInitialized = function() {
  var callback = () => {
    debugger
  }
  Module['websocket'].on('open', callback)
  Module['websocket'].on('message', callback)
}
