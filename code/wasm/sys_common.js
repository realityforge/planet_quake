var global = window || global
global['SYSC'] = {
  varStr: 0,
  oldDLURL: null,
  newDLURL: null,
  returnURL: null,
  
}

function readAsmConstArgs (sigPtr, buf) {
  readAsmConstArgsArray.length = 0;
  var ch;
  // Most arguments are i32s, so shift the buffer pointer so it is a plain
  // index into HEAP32.
  buf >>= 2;
  while (ch = HEAPU8[sigPtr++]) {
    // A double takes two 32-bit slots, and must also be aligned - the backend
    // will emit padding to avoid that.
    var double = ch < 105;
    if (double && (buf & 1)) buf++;
    readAsmConstArgsArray.push(double ? HEAPF64[buf++ >> 1] : HEAP32[buf]);
    ++buf;
  }
  return readAsmConstArgsArray;
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


function Sys_ResizeHeap(requestedSize) {
  var oldSize = HEAPU8.length;
  requestedSize = requestedSize >>> 0;
  #if ALLOW_MEMORY_GROWTH == 0
  #if ABORTING_MALLOC
  abortOnCannotGrowMemory(requestedSize);
  #else
  return false; // malloc will report failure
  #endif // ABORTING_MALLOC
  #else // ALLOW_MEMORY_GROWTH == 0
  // With pthreads, races can happen (another thread might increase the size in between), so return a failure, and let the caller retry.
  #if USE_PTHREADS
  if (requestedSize <= oldSize) {
    return false;
  }
  #endif // USE_PTHREADS
  #if ASSERTIONS && !USE_PTHREADS
  assert(requestedSize > oldSize);
  #endif
  
  #if EMSCRIPTEN_TRACING
  // Report old layout one last time
  _emscripten_trace_report_memory_layout();
  #endif
  
  // Memory resize rules:
  // 1. Always increase heap size to at least the requested size, rounded up to next page multiple.
  // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap geometrically: increase the heap size according to 
  //                                         MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%),
  //                                         At most overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
  // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap linearly: increase the heap size by at least MEMORY_GROWTH_LINEAR_STEP bytes.
  // 3. Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
  // 4. If we were unable to allocate as much memory, it may be due to over-eager decision to excessively reserve due to (3) above.
  //    Hence if an allocation fails, cut down on the amount of excess growth, in an attempt to succeed to perform a smaller allocation.
  
  // A limit is set for how much we can grow. We should not exceed that
  // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
  // In CAN_ADDRESS_2GB mode, stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate full 4GB Wasm memories, the size will wrap
  // back to 0 bytes in Wasm side for any code that deals with heap sizes, which would require special casing all heap size related code to treat
  // 0 specially.
  var maxHeapSize = {{{ Math.min(MAXIMUM_MEMORY, FOUR_GB - WASM_PAGE_SIZE) }}};
  if (requestedSize > maxHeapSize) {
  #if ASSERTIONS
    err('Cannot enlarge memory, asked to go up to ' + requestedSize + ' bytes, but the limit is ' + maxHeapSize + ' bytes!');
  #endif
  #if ABORTING_MALLOC
    abortOnCannotGrowMemory(requestedSize);
  #else
    return false;
  #endif
  }
  
  // Loop through potential heap size increases. If we attempt a too eager reservation that fails, cut down on the
  // attempted size and reserve a smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
  for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
  #if MEMORY_GROWTH_LINEAR_STEP == -1
    var overGrownHeapSize = oldSize * (1 + {{{ MEMORY_GROWTH_GEOMETRIC_STEP }}} / cutDown); // ensure geometric growth
  #if MEMORY_GROWTH_GEOMETRIC_CAP
    // but limit overreserving (default to capping at +96MB overgrowth at most)
    overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + {{{ MEMORY_GROWTH_GEOMETRIC_CAP }}} );
  #endif
  
  #else
    var overGrownHeapSize = oldSize + {{{ MEMORY_GROWTH_LINEAR_STEP }}} / cutDown; // ensure linear growth
  #endif
  
    var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), {{{ WASM_PAGE_SIZE }}}));
  
  #if ASSERTIONS == 2
    var t0 = _emscripten_get_now();
  #endif
    var replacement = emscripten_realloc_buffer(newSize);
  #if ASSERTIONS == 2
    var t1 = _emscripten_get_now();
    console.log('Heap resize call from ' + oldSize + ' to ' + newSize + ' took ' + (t1 - t0) + ' msecs. Success: ' + !!replacement);
  #endif
    if (replacement) {
  #if ASSERTIONS && WASM2JS
      err('Warning: Enlarging memory arrays, this is not fast! ' + [oldSize, newSize]);
  #endif
  
  #if EMSCRIPTEN_TRACING
      _emscripten_trace_js_log_message("Emscripten", "Enlarging memory arrays from " + oldSize + " to " + newSize);
      // And now report the new layout
      _emscripten_trace_report_memory_layout();
  #endif
      return true;
    }
  }
  #if ASSERTIONS
  err('Failed to grow the heap from ' + oldSize + ' bytes to ' + newSize + ' bytes, not enough memory!');
  #endif
  #if ABORTING_MALLOC
  abortOnCannotGrowMemory(requestedSize);
  #else
  return false;
  #endif
  #endif // ALLOW_MEMORY_GROWTH
  },
  
}
