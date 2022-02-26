if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

function Sys_UnloadLibrary() {

}

function Sys_LoadLibrary() {
  
}

function Sys_LoadFunction() {
  
}

function Sys_Microseconds() {
  if (window.performance.now) {
    return parseInt(window.performance.now(), 10);
  } else if (window.performance.webkitNow) {
    return parseInt(window.performance.webkitNow(), 10);
  }

  Q3e.sharedCounter += 8
  return Q3e.sharedMemory + Q3e.sharedCounter - 8
}

function Sys_Milliseconds() {
  if (!Q3e['timeBase']) {
    // javascript times are bigger, so start at zero
    //   pretend like we've been alive for at least a few seconds
    //   I actually had to do this because files it checking times and this caused a delay
    Q3e['timeBase'] = Date.now() - 5000;
  }

  //if (window.performance.now) {
  //  return parseInt(window.performance.now(), 10);
  //} else if (window.performance.webkitNow) {
  //  return parseInt(window.performance.webkitNow(), 10);
  //} else {
  return Date.now() - Q3e.timeBase;
  //}
}

function Sys_RandomBytes (string, len) {
	if(typeof crypto != 'undefined') {
		crypto.getRandomValues(HEAP8.subarray(string, string+(len / 4)))
	} else {
		for(let i = 0; i < (len / 4); i++) {
			HEAP8[string] = Math.random() * 255
		}
	}
	return true;
}

function Sys_Print(message) {
  console.log(addressToString(message))
}

function Sys_Error(fmt, args) {
  let len = BG_sprintf(Q3e.sharedMemory + Q3e.sharedCounter, fmt, args)
  if(len > 0)
    console.log('Sys_Error: ', addressToString(Q3e.sharedMemory + Q3e.sharedCounter))
  throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status, replacementStr) {
  // TODO: something like  window.title = , then setTimeout( window.title = 'Q3e' again)
  let desc = addressToString(status)
  if(desc.includes('Main menu')) {
    if(!Q3e.initialized) {
      Q3e.initialized = true
    }

  }
  let description = document.querySelector('#loading-progress .description')
  let div = document.createElement('div')
  // TODO: use BG_sprintf like above?
  if(replacementStr) {
    div.innerHTML = desc.replace('%s', addressToString(replacementStr))
  }
  if(description.children.length == 0
    || div.innerText.toLowerCase() != description
      .children[description.children.length-1].innerText.toLowerCase())
    description.appendChild(div)
}

var SYS = {
  DebugError: function () { console.log(new Error('debug').stack) },
  DebugBreak: function () { debugger; },
  Sys_RandomBytes: Sys_RandomBytes,
  Sys_Milliseconds: Sys_Milliseconds,
  Sys_Microseconds: Sys_Microseconds,
  Sys_Exit: function () {}, // TODO: redirect to lvlworld, was Sys_Main_PlatformExit
  Sys_Error: Sys_Error,
  Sys_UnloadLibrary: Sys_UnloadLibrary,
  Sys_LoadLibrary: Sys_LoadLibrary,
  Sys_LoadFunction: Sys_LoadFunction,
  popen: function popen() {},
  Sys_Print: Sys_Print,
  Sys_SetStatus: Sys_SetStatus,
  
}
