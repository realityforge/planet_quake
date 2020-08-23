var LibrarySysCommon = {
	$SYSC__deps: ['$Browser', '$FS', '$PATH', '$SYS', 'Com_Printf', 'Com_Outside_Error'],
	$SYSC: {
		varStr: 0,
		oldDLURL: null,
		newDLURL: null,
		Cvar_VariableString: function (str) {
			intArrayFromString(str).forEach((c, i) => HEAP8[(SYSC.varStr+i)] = c)
			HEAP8[(SYSC.varStr+str.length)] = 0
			return UTF8ToString(_Cvar_VariableString(SYSC.varStr))
		},
		Cvar_VariableIntegerValue: function (str) {
			intArrayFromString(str).forEach((c, i) => HEAP8[(SYSC.varStr+i)] = c)
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_VariableIntegerValue(SYSC.varStr)
		},
		Cvar_SetValue: function (str, value) {
			intArrayFromString(str).forEach((c, i) => HEAP8[(SYSC.varStr+i)] = c)
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_SetValue(SYSC.varStr, value)
		},
		Cvar_Get: function (str) {
			intArrayFromString(str).forEach((c, i) => HEAP8[(SYSC.varStr+i)] = c)
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_Get(SYSC.varStr, SYSC.varStr+str.length, 0)
		},
		Print: function (str) {
			str = allocate(intArrayFromString(str + '\n'), 'i8', ALLOC_STACK);
			_Com_Printf(str)
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

			Browser.safeCallback(_Com_Outside_Error)(level, 
				allocate(intArrayFromString(errMsg + '\n'), 'i8', ALLOC_STACK))
			// drop current stack frame and bubble out
			throw new Error(errMsg)
		},
		ProxyCallback: function () {
			Browser.safeCallback(() => {
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
			})()
		},
		addProtocol: function (url) { 
			return url.includes('://')
				? url
				: window
					? (window.location.protocol + '//' + url)
					: ('https://' + url)
		},
		DownloadAsset: function (asset, onprogress, onload) {
			var name = asset.replace(/^\//, '') //.replace(/(.+\/|)(.+?)$/, '$1' + asset.checksum + '-$2');
			var tryLinks = [
				SYSC.addProtocol(SYSC.newDLURL) + '/' + name,
				SYSC.addProtocol(SYSC.oldDLURL) + '/' + name,
			]
			// all of these test links are in case someone fucks up conversion or startup
			var tryMod = name.replace(/^\/|-cc?r?\//ig, '').split(/\//ig)[0]
			var noMod = name.replace(/^\/|-cc?r?\//ig, '').split(/\//ig)
				.slice(1).join('/')
			if(SYSF.mods.includes(tryMod + '-cc')) {
				tryLinks.push(SYSC.addProtocol(SYSC.newDLURL) + '/' + tryMod + '-ccr/' + noMod)
				tryLinks.push(SYSC.addProtocol(SYSC.newDLURL) + '/' + tryMod + '-cc/' + noMod)
			}
			var tryDownload = 0
			var doDownload = url => SYSN.DoXHR(url, {
				dataType: 'arraybuffer',
				onprogress: onprogress,
				onload: (err, data) => {
					if(err && tryDownload < tryLinks.length) {
						tryDownload++
						doDownload(tryLinks[tryDownload])
					} else {
						onload(err, data)
					}
				}
			})
			doDownload(tryLinks[0])
		},
		mkdirp: function (p) {
			try {
				FS.mkdir(p, 0777);
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
		return false;
	},
	Sys_LowPhysicalMemory: function () {
		return false;
	},
	Sys_Basename__deps: ['$PATH'],
	Sys_Basename: function (path) {
		path = PATH.basename(UTF8ToString(path));
		return allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
	},
	Sys_DllExtension__deps: ['$PATH'],
	Sys_DllExtension: function (path) {
		return PATH.extname(UTF8ToString(path)) == '.wasm';
	},
	Sys_Dirname__deps: ['$PATH'],
	Sys_Dirname: function (path) {
		path = PATH.dirname(UTF8ToString(path));
		return allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
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
			FS.mkdir(directory, 0777);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError)) {
				SYSC.Error('drop', e.message);
			}
			return e.errno === ERRNO_CODES.EEXIST;
		}
		return true;
	},
	Sys_Cwd: function () {
		var cwd = allocate(intArrayFromString(FS.cwd()), 'i8', ALLOC_STACK);
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
	Sys_LoadLibrary__deps: [],
	Sys_LoadLibrary: function (name) {
		return 0;
		return loadDynamicLibrary(name) // passing memory address
			.then(handle => SYSC.proxyCallback(handle))
	},
	Sys_LoadFunction: function () {
		throw new Error('TODO: Load DLL files')
	},
	Sys_UnloadLibrary: function () {
		
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
	}
};

autoAddDeps(LibrarySysCommon, '$SYSC');
mergeInto(LibraryManager.library, LibrarySysCommon);
