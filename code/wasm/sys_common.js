var LibrarySysCommon = {
	$SYSC__deps: ['$Browser', '$FS', '$PATH', '$SYS', 'Com_Printf', 'Com_Outside_Error'],
	$SYSC: {
		// next free handle to use for a loaded dso.
		// (handle=0 is avoided as it means "error" in dlopen)
		nextHandle: 1,
		loadedLibs: {         // handle -> dso [refcount, name, module, global]
			// program itself
			// XXX uglifyjs fails on "[-1]: {"
			'-1': {
				refcount: Infinity,   // = nodelete
				name:     '__self__',
				global:   true
			}
		},

		loadedLibNames: {     // name   -> handle
			// program itself
			'__self__': -1
		},
		varStr: 0,
		oldDLURL: null,
		newDLURL: null,
		returnURL: null,
		Cvar_VariableString: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return UTF8ToString(_Cvar_VariableString(SYSC.varStr))
		},
		Cvar_VariableIntegerValue: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_VariableIntegerValue(SYSC.varStr)
		},
		Cvar_SetValue: function (str, value) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_SetValue(SYSC.varStr, value)
		},
		Cvar_SetString: function (str, value) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			intArrayFromString(value).forEach(function (c, i) { HEAP8[(SYSC.varStr+str.length+i+1)] = c })
			HEAP8[(SYSC.varStr+str.length+value.length+1)] = 0
			return _Cvar_Set(SYSC.varStr, SYSC.varStr + str.length + 1)
		},
		Cvar_Get: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_Get(SYSC.varStr, SYSC.varStr+str.length, 0)
		},
		Print: function (str) {
			if(!Array.isArray(str)) str = [str]
			//str = str.map(function (s) {
			//	return allocate(intArrayFromString(s + '\0'), ALLOC_STACK);
			//})
			console.log.apply(null, str)
		},
		Error: function (level, errMsg) {
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
		},
		ProxyCallback: function () {
			try {
				_Com_Frame_Proxy();
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
		},
		mkdirp: function (p) {
			try {
				FS.mkdir(p, 16895);
			} catch (e) {
				// make the subdirectory and then retry
				if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
					SYSC.mkdirp(PATH.dirname(p));
					SYSC.mkdirp(p);
					return;
				}

				// if we got any other error, let's see if the directory already exists
				var stat;
				try {
					stat = FS.stat(p);
				}
				catch (e) {
					SYSC.Error('fatal', e.message || e);
					return;
				}

				if (!FS.isDir(stat.mode)) {
					SYSC.Error('fatal', e.message);
				}
			}
		},
	},
	Sys_DefaultHomePath: function () {
		return 0;
	},
	Sys_RandomBytes: function (string, len) {
		if(typeof crypto != 'undefined') {
			crypto.getRandomValues(HEAP8.subarray(string, string+len))
		} else {
			for(var i = 0; i < len; i++) {
				HEAP8[string] = Math.random() * 255
			}
		}
		return true;
	},
	Sys_LowPhysicalMemory: function () {
		return false;
	},
	Sys_Basename__deps: ['$PATH'],
	Sys_Basename: function (path) {
		path = PATH.basename(UTF8ToString(path));
		return allocate(intArrayFromString(path), ALLOC_STACK);
	},
	Sys_DllExtension__deps: ['$PATH'],
	Sys_DllExtension: function (path) {
		return PATH.extname(UTF8ToString(path)) == '.wasm';
	},
	Sys_Dirname__deps: ['$PATH'],
	Sys_Dirname: function (path) {
		path = PATH.dirname(UTF8ToString(path));
		return allocate(intArrayFromString(path), ALLOC_STACK);
	},
	Sys_Mkfifo: function (path) {
		return 0;
	},
	Sys_FreeFileList__deps: ['Z_Free'],
	Sys_FreeFileList: function (list) {
		if (!list) {
			return;
		}

		var ptr;
		for (var i = 0; (ptr = {{{ makeGetValue('list', 'i*4', 'i32') }}}); i++) {
			_Z_Free(ptr);
		}

		_Z_Free(list);
	},
	Sys_Mkdir: function (directory) {
		directory = UTF8ToString(directory);
		try {
			FS.mkdir(directory, 16895);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError)) {
				SYSC.Error('drop', e.message);
			}
			return e.errno === ERRNO_CODES.EEXIST;
		}
		return true;
	},
	Sys_Cwd: function () {
		var cwd = allocate(intArrayFromString(FS.cwd()), ALLOC_STACK);
		return cwd;
	},
	Sys_Sleep: function () {
	},
	Sys_SetEnv: function (name, value) {
		name = UTF8ToString(name);
		value = UTF8ToString(value);
	},
	Sys_PID: function () {
		return 0;
	},
	Sys_PIDIsRunning: function (pid) {
		return 1;
	},
	Sys_SetAffinityMask: function () {
		throw new Error('TODO: support using background workers or not')
	},
	Sys_ShowConsole: function () {
		// not implemented
	},
	Sys_GetFileStats: function(filename, size, mtime, ctime) {
		try {
			var stat = FS.stat(UTF8ToString(name))
			{{{ makeSetValue('size', '0', 'stat.size', 'i32') }}}
			{{{ makeSetValue('mtime', '0', 'stat.mtime', 'i32') }}}
			{{{ makeSetValue('ctime', '0', 'stat.ctime', 'i32') }}}
			return true
		} catch (e) {
			if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
				{{{ makeSetValue('size', '0', '0', 'i32') }}}
				{{{ makeSetValue('mtime', '0', '0', 'i32') }}}
				{{{ makeSetValue('ctime', '0', '0', 'i32') }}}
				return false
			}
			throw e
		}
	},
	Sys_Debug: function () {
		try {
			throw new Error('debugging error')
		} catch (e) {
			console.log(e)
		}
	},
};

autoAddDeps(LibrarySysCommon, '$SYSC');
mergeInto(LibraryManager.library, LibrarySysCommon);
