window = {}
window.location = new URL('http://worker.js/?set dedicated 1&set ttycon 1')
window.performance = performance
importScripts('quake3e.js')

onmessage = function(e) {
  var workerResult = 'Result: ' + (e.data[0] * e.data[1])
  console.log('Posting message back to main script ' + workerResult)
  postMessage(workerResult)
}

Module.onRuntimeInitialized = function() {
  Module.callMain()
}
