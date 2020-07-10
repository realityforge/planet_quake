window = {}
serverWorker = self
window.serverWorker = self
window.location = new URL(location.origin + '?set dedicated 2')
window.performance = performance
quake3e = {}
quake3e.noInitialRun = true
quake3e.printErr = function (...args) {
  var args = Array.from(arguments)
  if(args[0] && (args[0].includes('Sys_Error:')
    || args[0].includes('Error:')
    || args[0].includes('ERROR:')
    || args[0].includes('server:')
    || args[0].includes('Hunk_Clear:')
    || args[0].includes('Frame Setup')))
    console.error.apply(console, ['DedServer: '].concat(args))
  else
    console.log.apply(console, ['DedServer: '].concat(args))
}
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
      && !SYSM.args.includes('+spdevmap')) {
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
    SYSN.receiveNetLoop(e.data[1], e.data[2])
  } else {
    console.log('Command not found ', e.data)
  }
}

quake3e.onRuntimeInitialized = function() {
  SYSM.args.unshift.apply(SYSM.args, [
    '+set', 'ttycon', '1',
    '+set', 'sv_master1', '207.246.91.235',
    '+set', 'sv_hostname', 'Local Host',
    '+set', 'sv_motd', 'For instant replays and stuff',
    '+set', 'rconPassword', 'password123!',
    '+set', 'sv_reconnectlimit', '0',
  //  '+set', 'net_socksEnabled', '0',
  ])
  runIsFirst = true
  if(initIsFirst) Module.callMain()
}
