"use strict";
var window = {}
if(typeof global != 'undefined')
  global.window = window
var serverWorker = self
window.serverWorker = self
importScripts('quake3e.js')
var initIsFirst = false
var runIsFirst = false

onmessage = function(e) {
  if(e.data[0] == 'init') {
    initIsFirst = true
    SYSM.args.push.apply(SYSM.args, e.data[1])
    if(!SYSM.args.includes('+spmap')
      && !SYSM.args.includes('+map')
      && !SYSM.args.includes('+devmap')
      && !SYSM.args.includes('+spdevmap')
      && !SYSM.args.includes('+demo_play')) {
        SYSM.args.push.apply(SYSM.args, [
          '+spmap', 'q3dm0',
        ])
      }
    if(runIsFirst) Module.callMain()
  } else if(e.data[0] == 'execute') {
    var cmd = allocate(intArrayFromString(e.data[1]), 'i8', ALLOC_STACK)
    _Cbuf_AddText(cmd)
    _Cbuf_Execute()
  } else if(e.data[0] == 'net') {
    if(runIsFirst || initIsFirst)
      SYSN.receiveNetLoop(e.data[1], e.data[2])
  } else {
    console.log('Command not found ', e.data)
  }
}

Module.onRuntimeInitialized = function() {
  SYSM.args.unshift.apply(SYSM.args, [
    '+set', 'ttycon', '1',
    '+set', 'sv_master1', '207.246.91.235',
    '+set', 'sv_hostname', 'Local Host',
    '+set', 'sv_motd', 'For instant replays and stuff',
    '+set', 'rconPassword', 'password123!',
    '+set', 'sv_reconnectlimit', '0',
  //  '+set', 'sv_autoDemo', '1',
  //  '+set', 'sv_autoRecord', '1',
  //  '+set', 'net_socksEnabled', '0',
  ])
  runIsFirst = true
  if(initIsFirst) Module.callMain()
}
