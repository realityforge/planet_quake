// loader for emjs build, inserted at the end of quake3e.js script
if(typeof global != 'undefined' && typeof global.window == 'undefined') {
  global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

let startup = [
  'quake3e_web',
  '+set', 'developer', '1',
  '+set', 'fs_basepath', '/base',
  '+set', 'fs_homepath', '/home',
  '+set', 'sv_pure', '0', // require for now, TODO: server side zips
  '+set', 'fs_basegame', 'multigame',
  '+set', 'cl_dlURL', '"http://local.games:8080/multigame"',

];

// unfuck some window variables because emscripten has to interfere with everything and rename
//   everything to match some atiquated compiler output that adds underscores to all names
Q3e.exports = SYS
Object.assign(window, SYS)
window.FS = SYS
window.NET = SYS
window.malloc = _malloc
if(typeof _Z_Malloc == 'undefined') {
  window.Z_Malloc = _Z_MallocDebug
} else {
  window.Z_Malloc = _Z_Malloc
}
window.Cvar_VariableString = _Cvar_VariableString
window.FS_GetCurrentGameDir = _FS_GetCurrentGameDir
window.Sys_FileReady = _Sys_FileReady

// start a brand new call frame, in-case error bubbles up
Module['onRuntimeInitialized'] = function () {
  try {

    // reserve some memory at the beginning for passing shit back and forth with JS
    //   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
    //   something isn't cleared out, crash
    Q3e['sharedMemory'] = malloc(1024 * 1024) // store some strings and crap
    Q3e['sharedCounter'] = 0
    
    // Startup args is expecting a char **
    _RunGame(startup.length, stringsToMemory(startup))
    setInterval(function () {
      requestAnimationFrame(function () {
        try {
          _Sys_Frame()
        } catch (e) {
          console.log(e)
        }
      })
    }, 1000 / 80);
  } catch (e) {
    console.log(e)
  }
}