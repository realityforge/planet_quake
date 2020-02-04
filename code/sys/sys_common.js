var LibrarySysCommon = {
	$SYSC__deps: ['$Browser', '$FS', '$PATH', '$SYS', 'Com_Printf', 'Com_Error'],
	$SYSC: {
		cb_context_t: {
			__size__: 8,
			data: 0,
			cb: 4
		},
		startup_data_t: {
			__size__: 4100,
			gameName: 0,
			after: 4096
		},
		download_progress_data_t: {
			__size__: 8,
			loaded: 0,
			total: 4
		},
		download_complete_data_t: {
			__size__: 4,
			progress: 0,
		},
		eula: '',
		manifest: null,
		Print: function (str) {
			str = allocate(intArrayFromString(str + '\n'), 'i8', ALLOC_STACK);

			_Com_Printf(str);
		},
		Error: function (level, err) {
			if (level === 'fatal') {
				level = 0;
			} else if (level === 'drop') {
				level = 1;
			} else if (level === 'serverdisconnect') {
				level = 2;
			} else if (level === 'disconnect') {
				level = 3;
			} else if (level === 'need_cd') {
				level = 4;
			} else {
				level = 0;
			}

			err = allocate(intArrayFromString(err + '\n'), 'i8', ALLOC_STACK);
			if(!err) err = UTF8ToString(err);

			_Com_Error(level, err);
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
		DownloadAsset: function (asset, onprogress, onload) {
			var sv_dlURL = UTF8ToString(_Cvar_VariableString(allocate(intArrayFromString('sv_dlURL'), 'i8', ALLOC_STACK)));
			var name = asset.replace(/^\//, ''); //.replace(/(.+\/|)(.+?)$/, '$1' + asset.checksum + '-$2');
			var url = (sv_dlURL.includes('://')
				? sv_dlURL
				: window
					? (window.location.protocol + '//' + sv_dlURL)
					: ('https://' + sv_dlURL)) + '/assets/' + name;

			SYS.DoXHR(url, {
				dataType: 'arraybuffer',
				onprogress: onprogress,
				onload: onload
			});
		},
		SavePak: function (name, buffer, callback) {
			var fs_homepath = UTF8ToString(_Cvar_VariableString(allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK)));
			var localPath = PATH.join(fs_homepath, name);

			try {
				FS.mkdir(PATH.dirname(localPath), 0777);
			} catch (e) {
				if (e.errno !== ERRNO_CODES.EEXIST) {
					return callback(e);
				}
			}

			try {
				FS.writeFile(localPath, new Uint8Array(buffer), {
					encoding: 'binary', flags: 'w', canOwn: true });
			} catch (e) {
				throw e;
			}
			
			FS.syncfs(callback);
		},
		ValidatePak: function (asset) {
			var fs_homepath = UTF8ToString(_Cvar_VariableString(allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK)));
			var localPath = PATH.join(fs_homepath, asset.name);
			//var crc = SYSC.CRC32File(localPath);
			return true;
			//return crc === asset.checksum;
		},
		FS_Startup: function (callback) {
			Browser.safeCallback(callback)();
			// do something?
			/*
			SYSC.SyncDependencies(function (err) {
				if (err) return callback(err);

				SYSC.SyncPaks(Browser.safeCallback(callback));
			});
			*/
		},
		FS_Shutdown: function (callback) {
			callback(null);
		},
	},
	Sys_DefaultHomePath: function () {
		return 0;
	},
	Sys_RandomBytes: function (string, len) {
		return false;
	},
	Sys_GetClipboardData: function () {
		return 0;
	},
	Sys_LowPhysicalMemory: function () {
		return false;
	},
	Sys_Basename__deps: ['$PATH'],
	Sys_Basename: function (path) {
		path = UTF8ToString(path);
		path = PATH.basename(path);
		var basename = allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
		return basename;
	},
	Sys_DllExtension: function () {
		return false;
	},
	Sys_Dirname__deps: ['$PATH'],
	Sys_Dirname: function (path) {
		path = UTF8ToString(path);
		path = PATH.dirname(path);
		var dirname = allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
		return dirname;
	},
	Sys_Mkfifo: function (path) {
		return 0;
	},
	Sys_ListFiles__deps: ['$PATH', 'Z_Malloc', 'S_Malloc'],
	Sys_ListFiles: function (directory, ext, filter, numfiles, dironly) {
		directory = UTF8ToString(directory);
		ext = UTF8ToString(ext);
		if (ext === '/') {
			ext = null;
			dironly = true;
		}

		// TODO support filter
		
		var contents;
		try {
			contents = FS.readdir(directory);
		} catch (e) {
			{{{ makeSetValue('numfiles', '0', '0', 'i32') }}};
			return null;
		}

		var matches = [];
		for (var i = 0; i < contents.length; i++) {
			var name = contents[i];
			var stat = FS.stat(PATH.join(directory, name));

			if (dironly && !FS.isDir(stat.mode)) {
				continue;
			}

			if ((!ext || name.lastIndexOf(ext) === (name.length - ext.length))) {
				matches.push(name);
			}
		}

		{{{ makeSetValue('numfiles', '0', 'matches.length', 'i32') }}};

		if (!matches.length) {
			return null;
		}

		// return a copy of the match list
		var list = _Z_Malloc((matches.length + 1) * 4);

		var i;
		for (i = 0; i < matches.length; i++) {
			var filename = _S_Malloc(matches[i].length + 1);

			stringToUTF8(matches[i], filename, matches[i].length+1);

			// write the string's pointer back to the main array
			{{{ makeSetValue('list', 'i*4', 'filename', 'i32') }}};
		}

		// add a NULL terminator to the list
		{{{ makeSetValue('list', 'i*4', '0', 'i32') }}};

		return list;
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
	Sys_FOpen__deps: ['$FS', 'fopen'],
	Sys_FOpen: function (ospath, mode) {
		var handle;
		try {
			ospath = allocate(intArrayFromString(UTF8ToString(ospath)
				.replace(/\/\//ig, '/')), 'i8', ALLOC_STACK);
			mode = allocate(intArrayFromString(UTF8ToString(mode)
				.replace('b', '')), 'i8', ALLOC_STACK);
			handle = _fopen(ospath, mode);
		} catch (e) {
			// short for fstat check in sys_unix.c!!!
			if(e.code === 'ENOENT') {
				return 0;
			}
			throw e;
		}
		return handle;
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
	}
};

autoAddDeps(LibrarySysCommon, '$SYSC');
mergeInto(LibraryManager.library, LibrarySysCommon);
