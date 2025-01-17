
// loader for emjs build, inserted at the end of quake3e.js script
if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

// unfuck some window variables because emscripten has to interfere with everything and rename
//   everything to match some atiquated compiler output that adds underscores to all names
Object.assign(window, SYS)


window.FS = SYS
window.NET = SYS
window.INPUT = SYS
window.ARB = SYS
window.EMGL = SYS
window.SND = SYS



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
window.BG_sprintf = _BG_sprintf
window.Cvar_SetIntegerValue = _Cvar_SetIntegerValue
window.Key_GetCatcher = _Key_GetCatcher
window.Key_SetCatcher = _Key_SetCatcher
window.Cbuf_AddText = _Cbuf_AddText
window.Cvar_Set = _Cvar_Set
window.Cvar_Get = _Cvar_Get
window.Cvar_CheckRange = _Cvar_CheckRange
window.FS_CopyString = _FS_CopyString
window.FS_ReadFile = _FS_ReadFile
window.Com_Frame = _Com_Frame
window.RunGame = _RunGame
window.GLimp_Shutdown = _GLimp_Shutdown
window.SNDDMA_Init = _SNDDMA_Init
window.s_soundStarted = _s_soundStarted
window.s_soundMuted = _s_soundMuted
window.S_Base_SoundInfo = _S_Base_SoundInfo

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
  Q3e['memory'] = wasmMemory
  Promise.resolve({})
    .then(SYS.startProgram)
}
