var global = window || global
global['SYS'] = {
  serviceProgress: [0, 0],
  previousProgress: [0, 0],
  servicable: false,
  dedicated: false,
  resizeDelay: null,
  style: null,
  shaderCallback: [],
  soundCallback: [],
  modelCallback: [],
}

function quitGameOnUnload (e) {
  if(SYSI.canvas) {
    _Cbuf_AddText(allocate(intArrayFromString('quit;'), ALLOC_STACK));
    SYSI.canvas.remove()
    SYSI.canvas = null
  }
  return false
}

function onWorkerMessage (e) {
  if(typeof Module != 'undefined' 
    && typeof Module.SYSC != 'undefined' && !Module.SYSM.exited)
    return

  if(e.data[0] == 'init') {
    Module.SYSC.Cvar_SetValue('sv_running', 1)
  } else if(e.data[0] == 'net') {
    Module.SYSN.receiveNetLoop(e.data[1], e.data[2])
  } else if(e.data[0] == 'status') {
    Module.LoadingDescription.apply(null, e.data[1])
    if(e.data[2]) {
      Module.SYS.serviceProgress = e.data[2]
      Module.SYSN.LoadingProgress()
    } else {
      Module.SYS.serviceProgress = [0, 0]
    }
  }
}

function Sys_UpdateShader () {
	var nextFile = SYS.shaderCallback.pop()
	if(!nextFile) return 0;
	var filename = _S_Malloc(nextFile.length + 1);
	stringToUTF8(nextFile + '\0', filename, nextFile.length+1);
	return filename
}


function Sys_UpdateSound () {
	var nextFile = SYS.soundCallback.pop()
	if(!nextFile) return 0;
	var filename = _S_Malloc(nextFile.length + 1);
	stringToUTF8(nextFile + '\0', filename, nextFile.length+1);
	return filename
}


function Sys_UpdateModel () {
	var nextFile = SYS.modelCallback.pop()
	if(!nextFile) return 0;
	var filename = _S_Malloc(nextFile.length + 1);
	stringToUTF8(nextFile + '\0', filename, nextFile.length+1);
	return filename
}
