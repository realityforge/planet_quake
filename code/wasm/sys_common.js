var global = window || global
global['SYSC'] = {
  varStr: 0,
  oldDLURL: null,
  newDLURL: null,
  returnURL: null,
  
}

function Cvar_VariableString (str) {
  intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
  HEAP8[(SYSC.varStr+str.length)] = 0
  return UTF8ToString(_Cvar_VariableString(SYSC.varStr))
}

function Cvar_VariableIntegerValue (str) {
  intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
  HEAP8[(SYSC.varStr+str.length)] = 0
  return _Cvar_VariableIntegerValue(SYSC.varStr)
}

function Cvar_SetValue (str, value) {
  intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
  HEAP8[(SYSC.varStr+str.length)] = 0
  return _Cvar_SetValue(SYSC.varStr, value)
}

function Cvar_SetString (str, value) {
  intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
  HEAP8[(SYSC.varStr+str.length)] = 0
  intArrayFromString(value).forEach(function (c, i) { HEAP8[(SYSC.varStr+str.length+i+1)] = c })
  HEAP8[(SYSC.varStr+str.length+value.length+1)] = 0
  return _Cvar_Set(SYSC.varStr, SYSC.varStr + str.length + 1)
}

function Cvar_Get (str) {
  intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
  HEAP8[(SYSC.varStr+str.length)] = 0
  return _Cvar_Get(SYSC.varStr, SYSC.varStr+str.length, 0)
}

function Sys_Print (str) {
  if(!Array.isArray(str)) str = [str]
  //str = str.map(function (s) {
  //	return allocate(intArrayFromString(s + '\0'), ALLOC_STACK);
  //})
  console.log.apply(null, str)
}

function Sys_Error (level, errMsg) {
  if (level === 'fatal') {
    level = 0
  } else if (level === 'drop') {
    level = 1
  } else if (level === 'serverdisconnect') {
    level = 2
  } else if (level === 'disconnect') {
    level = 3
  } else if (level === 'need_cd') {
    level = 4
  } else {
    level = 0
  }

  _Com_Outside_Error(level, 
    allocate(intArrayFromString(errMsg + '\n'), ALLOC_STACK))
  // drop current stack frame and bubble out
  throw new Error(errMsg)
}

function ProxyCallback (handle) {
  try {
    _Com_Frame_Proxy(handle || 0);
  } catch (e) {
    if (e instanceof ExitStatus) {
      return;
    }
    // TODO should we try and call back in using __Error?
    var message = _S_Malloc(e.message.length + 1);
    stringToUTF8(e.message, message, e.message.length+1);
    _Sys_ErrorDialog(message);
    throw e;
  }
}

function Sys_mkdirp (p) {
  try {
    FS.mkdir(p, 16895);
  } catch (e) {
    // make the subdirectory and then retry
    if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
      Sys_mkdirp(PATH.dirname(p));
      Sys_mkdirp(p);
      return;
    }

    // if we got any other error, let's see if the directory already exists
    var stat;
    try {
      stat = FS.stat(p);
    }
    catch (e) {
      Sys_Error('fatal', e.message || e);
      return;
    }

    if (!FS.isDir(stat.mode)) {
      Sys_Error('fatal', e.message);
    }
  }
}

function Sys_DefaultHomePath () {
	return 0;
}

function Sys_RandomBytes (string, len) {
	if(typeof crypto != 'undefined') {
		crypto.getRandomValues(HEAP8.subarray(string, string+len))
	} else {
		for(var i = 0; i < len; i++) {
			HEAP8[string] = Math.random() * 255
		}
	}
	return true;
}

function Sys_LowPhysicalMemory () {
	return false;
}

function Sys_Basename (path) {
	path = PATH.basename(UTF8ToString(path));
	return allocate(intArrayFromString(path), ALLOC_STACK);
}

function Sys_DllExtension (path) {
	return PATH.extname(UTF8ToString(path)) == '.wasm';
}

function Sys_Dirname (path) {
	path = PATH.dirname(UTF8ToString(path));
	return allocate(intArrayFromString(path), ALLOC_STACK);
}

function Sys_Mkfifo (path) {
	return 0;
}

function Sys_FreeFileList (list) {
	if (!list) {
		return;
	}

	var ptr;
	for (var i = 0; (ptr = HEAP32[(list+i*4)>>2]); i++) {
		_Z_Free(ptr);
	}

	_Z_Free(list);
}

function Sys_Mkdir (directory) {
	directory = UTF8ToString(directory);
	try {
		FS.mkdir(directory, 16895);
	} catch (e) {
		if (!(e instanceof FS.ErrnoError)) {
			Sys_Error('drop', e.message);
		}
		return e.errno === ERRNO_CODES.EEXIST;
	}
	return true;
}

function Sys_Cwd () {
	var cwd = allocate(intArrayFromString(FS.cwd()), ALLOC_STACK);
	return cwd;
}

function Sys_Sleep () {
}

function Sys_SetEnv (name, value) {
	name = UTF8ToString(name);
	value = UTF8ToString(value);
}

function Sys_PID () {
	return 0;
}

function Sys_PIDIsRunning (pid) {
	return 1;
}

function Sys_SetAffinityMask () {
	throw new Error('TODO: support using background workers or not')
}

function Sys_ShowConsole () {
	// not implemented
}

function Sys_GetFileStats (filename, size, mtime, ctime) {
	try {
		var stat = FS.stat(UTF8ToString(name))
		HEAP32[(size+0)>>2] = stat.size
		HEAP32[(size+0)>>2] = stat.size
		HEAP32[(size+0)>>2] = stat.size
		return true
	} catch (e) {
		if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
			HEAP32[(size+0)>>2] = stat.size
			HEAP32[(size+0)>>2] = stat.size
			HEAP32[(size+0)>>2] = stat.size
			return false
		}
		throw e
	}
}

function Sys_Debug () {
	try {
		throw new Error('debugging error')
	} catch (e) {
		console.log(e)
	}
}
