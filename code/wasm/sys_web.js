// loader for emjs build, inserted at the end of quake3e.js script
if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

let startup = [
  'quake3e_web',
  '+set', 'developer', '0',
  '+set', 'fs_basepath', '/base',
  '+set', 'fs_homepath', '/home',
  '+set', 'sv_pure', '0', // require for now, TODO: server side zips
  '+set', 'fs_basegame', 'multigame',
  '+set', 'cl_dlURL', '"http://local.games:8080/multigame"',
  '+set', 'r_ext_framebuffer_object', '0',

  //'+set', 'r_ext_multitexture', '0',
  //'+set', 'r_ext_framebuffer_multisample', '0',
  //'+set', 'r_ext_framebuffer_object', '0',
  // this prevents lightmap from being wrong when switching maps
  //   renderer doesn't restart between maps, but BSP loading updates
  //   textures with lightmap by default, so this keeps them separate
  //'+set', 'r_mergeLightmaps', '0',
  //'+set', 'r_deluxeMapping', '0',
  //'+set', 'r_normalMapping', '0',
  //'+set', 'r_specularMapping', '0',


];

// unfuck some window variables because emscripten has to interfere with everything and rename
//   everything to match some atiquated compiler output that adds underscores to all names
Object.assign(window, SYS)
window.FS = SYS
window.NET = SYS
window.INPUT = SYS
window.ARB = SYS
window.malloc = _malloc
if(typeof _Z_Malloc == 'undefined') {
  window.Z_Malloc = _Z_MallocDebug
} else {
  window.Z_Malloc = _Z_Malloc
}
window.Cvar_VariableIntegerValue = _Cvar_VariableIntegerValue
window.Cvar_VariableString = _Cvar_VariableString
window.FS_GetCurrentGameDir = _FS_GetCurrentGameDir
window.Sys_FileReady = _Sys_FileReady
window.Key_KeynumToString = _Key_KeynumToString
window.Sys_QueEvent = _Sys_QueEvent
window.Sys_Milliseconds = _Sys_Milliseconds
window.Key_ClearStates = _Key_ClearStates
window.gw_active = _gw_active
window.gw_minimized = _gw_minimized
window.g_bigcharsData = _g_bigcharsData
window.g_bigcharsSize = _g_bigcharsSize
window.BG_sprintf = _BG_sprintf
window.Cvar_SetIntegerValue = _Cvar_SetIntegerValue
window.Key_GetCatcher = _Key_GetCatcher
window.Key_SetCatcher = _Key_SetCatcher
window.Cbuf_AddText = _Cbuf_AddText
window.Cvar_Set = _Cvar_Set
window.FS_CopyString = _FS_CopyString

var _emscripten_glDrawArrays = null
var _emscripten_glDrawElements = null
var _emscripten_glActiveTexture = null
var _emscripten_glEnable = null
var _emscripten_glDisable = null
var _emscripten_glTexEnvf = null
var _emscripten_glTexEnvi = null
var _emscripten_glTexEnvfv = null
var _emscripten_glGetIntegerv = null
var _emscripten_glIsEnabled = null
var _emscripten_glGetBooleanv = null
var _emscripten_glGetString = null
var _emscripten_glCreateShader = null
var _emscripten_glShaderSource = null
var _emscripten_glCompileShader = null
var _emscripten_glAttachShader = null
var _emscripten_glDetachShader = null
var _emscripten_glUseProgram = null
var _emscripten_glDeleteProgram = null
var _emscripten_glBindAttribLocation = null
var _emscripten_glLinkProgram = null
var _emscripten_glBindBuffer = null
var _emscripten_glGetFloatv = null
var _emscripten_glHint = null
var _emscripten_glEnableVertexAttribArray = null
var _emscripten_glDisableVertexAttribArray = null
var _emscripten_glVertexAttribPointer = null
var _glTexEnvf = null
var _glTexEnvi = null
var _glTexEnvfv = null
var _glGetTexEnviv = null
var _glGetTexEnvfv = null

//GLImmediate.setupFuncs()
// start a brand new call frame, in-case error bubbles up
Module['noFSInit'] = true
Module['onRuntimeInitialized'] = function () {
  try {

    // reserve some memory at the beginning for passing shit back and forth with JS
    //   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
    //   something isn't cleared out, crash
    Q3e['sharedMemory'] = malloc(1024 * 1024) // store some strings and crap
    Q3e['sharedCounter'] = 0
    if(HEAP32[g_bigcharsSize >> 2] == 0) {
      let bigchars = atob(document.getElementById('bigchars').src.substring(22))
        .split("").map(function(c) { return c.charCodeAt(0); })
      let startPos = HEAP32[g_bigcharsData >> 2] = malloc(bigchars.length)
      HEAP32[g_bigcharsSize >> 2] = bigchars.length
      for(let i = 0; i < bigchars.length; i++) {
        HEAP8[startPos] = bigchars[i]
        startPos++
      }
    }
  
    // Startup args is expecting a char **
    _RunGame(startup.length, stringsToMemory(startup))
    // should have Cvar system by now
    let fps = Cvar_VariableIntegerValue(stringToAddress('com_maxfps'))
    Q3e.frameInterval = setInterval(function () {
      requestAnimationFrame(function () {
        try {
          _Sys_Frame()
        } catch (e) {
          console.log(e)
        }
      })
    }, 1000 / fps);
  } catch (e) {
    console.log(e)
  }
}