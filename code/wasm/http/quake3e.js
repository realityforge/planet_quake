if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

function getQueryCommands() {
  // Wow, look at all the unfuckery I don't have to do with startup options because
  //   I'm not using emscripten anymore.
  let startup = [
    'quake3e_web',
    '+set', 'developer', '0',
    '+set', 'fs_basepath', '/base',
    '+set', 'fs_homepath', '/home',
    '+set', 'sv_pure', '0', // require for now, TODO: server side zips
    '+set', 'fs_basegame', 'multigame',
    '+set', 'cl_dlURL', '"http://local.games:8080/multigame"',
    '+set', 'r_mode', '-1',
    '+set', 'r_ext_framebuffer_object', '0',

    //'+set', 'r_ext_multitexture', '0',
    //'+set', 'r_ext_framebuffer_multisample', '0',
    // this prevents lightmap from being wrong when switching maps
    //   renderer doesn't restart between maps, but BSP loading updates
    //   textures with lightmap by default, so this keeps them separate
    //'+set', 'r_mergeLightmaps', '0',
    //'+set', 'r_deluxeMapping', '0',
    //'+set', 'r_normalMapping', '0',
    //'+set', 'r_specularMapping', '0',


  ];
  var search = /([^&=]+)/g
  var query  = window.location.search.substring(1)
  var match
  while (match = search.exec(query)) {
    var val = decodeURIComponent(match[1])
    val = val.split(' ')
    val[0] = (val[0][0] != '+' ? '+' : '') + val[0]
    startup.push.apply(startup, val)
  }
  startup.unshift.apply(startup, [
    '+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
    '+set', 'r_customHeight', '' + window.innerHeight || 0,
    '+set', 'r_customWidth', '' + window.innerWidth || 0,
  ])
  return startup
}

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
  _Sys_Exit( 1 )
  throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status, replacementStr) {
  // TODO: something like  window.title = , then setTimeout( window.title = 'Q3e' again)
  /*
  let desc = addressToString(status)
  if(desc.includes('Main menu')) {
    if(!Q3e.initialized) {
      Q3e.initialized = true
      document.body.className += ' done-loading '
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
  */
}

function CL_MenuModified(oldValue, newValue, cvar) {
  if(INPUT.modifyingCrumb) {
    return // called from ourselves below from a user action
  }
  let newValueStr = addressToString(newValue)
  let newLocation = newValueStr.replace(/[^a-z0-9]/gi, '')
  //if(newValueStr.includes('MAIN MENU')) {
  if(!Q3e.initialized) {
    Q3e.initialized = true
    document.body.className += ' done-loading '
  }
  //}
  if(window.location.pathname.toString().includes(newLocation)) {
    return // don't add to stack because it creates a lot of annoying back pushes
  }
  history.pushState(
    {location: window.location.pathname}, 
    'Quake III Arena: ' + newValueStr, 
    newLocation)
}

function CL_ModifyMenu(event) {
  let oldLocation = window.location.pathname.toString().substring(1) || 'MAIN MENU'
  Cbuf_AddText( stringToAddress(`set ui_breadCrumb "${oldLocation}"\n`) );
}


var SYS = {
  DebugBreak: function () { debugger; },
  Sys_RandomBytes: Sys_RandomBytes,
  Sys_Milliseconds: Sys_Milliseconds,
  Sys_Microseconds: Sys_Microseconds,
  Sys_Exit: function () {
    if(Q3e.frameInterval) {
      clearInterval(Q3e.frameInterval)
    }
    // redirect to lvlworld
    let returnUrl = addressToString(Cvar_VariableString('cl_returnURL'))
    if(returnUrl) {
      window.location = returnUrl
    }
  },
  Sys_Error: Sys_Error,
  Sys_UnloadLibrary: Sys_UnloadLibrary,
  Sys_LoadLibrary: Sys_LoadLibrary,
  Sys_LoadFunction: Sys_LoadFunction,
  popen: function popen() {},
  Sys_Print: Sys_Print,
  Sys_SetStatus: Sys_SetStatus,
  CL_MenuModified: CL_MenuModified,

}
