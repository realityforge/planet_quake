window = {}
window.serverWorker = self
window.location = new URL(location.origin + '?set dedicated 1&set ttycon 1&set net_socksEnabled 0')
window.performance = performance
quake3e = {}
quake3e.noInitialRun = true
quake3e.printErr = console.log
importScripts('quake3e.js')
var initIsFirst = false
var runIsFirst = false

onmessage = function(e) {
  if(e.data[0] == 'init') {
    initIsFirst = true
    if(runIsFirst) Module.callMain()
  } else if(e.data[0] == 'vars') {
    
  } else if(e.data[0] == 'execute') {
    var cmd = allocate(intArrayFromString(e.data[1]), 'i8', ALLOC_STACK)
    _Cbuf_AddText(cmd)
    _Cbuf_Execute()
  } else if(e.data[0] == 'net') {
    SYSN.receiveNetLoop(e.data[1], e.data[2])
  } else {
    console.log('Command not found ', e.data)
  }
}

quake3e.onRuntimeInitialized = function() {
  runIsFirst = true
  if(initIsFirst) Module.callMain()
  var callback = () => {}
  Module['websocket'].on('open', callback)
  Module['websocket'].on('message', callback)
}
