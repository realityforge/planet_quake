"use strict";
var window = {}
if(typeof global != 'undefined')
  global.window = window
var serverWorker = self
window.serverWorker = self

self.onmessage = function(e) {
  if(e.data[0] == 'init') {
    importScripts('quake3e.js')
    Module.args = e.data[1]
  } else if(typeof Module != 'undefined' && e.data[0] == 'execute') {
    var cmd = allocate(intArrayFromString(e.data[1]), ALLOC_STACK)
    _Cbuf_AddText(cmd)
  } else if(typeof Module != 'undefined' && e.data[0] == 'net') {
    if(typeof Module.SYSN != 'undefined' && Module.SYSN.lazyInterval)
      // to see if Sys_PlatformInit has been called
      Module.SYSN.receiveNetLoop(e.data[1], e.data[2])
  } else {
    console.log('Command not found ', e.data)
  }
}
