var LibrarySys = {
	$SYS__deps: ['$SYSC'],
	$SYS: {
		exited: false,
		timeBase: null,
		style: null,
		loading: null,
		args: [
			'+set', 'sv_dlURL', '"http://localhost:8080"',
			'+set', 'fs_basegame', 'baseq3',
			'+set', 'fs_game', 'baseq3',
			'+set', 'net_noudp', '1',
			'+set', 'net_enabled', '0',
			'+set', 'developer', '0',
			'+set', 'fs_debug', '0',
			'+set', 'r_mode', '-1',
			'+set', 'r_customPixelAspect', '1',
			'+set', 'sv_pure', '0',
			// these settings were set by the emscripten build
			//'+connect', 'proxy.quake.games:443',
			/*
			'+set', 'r_normalMapping', '0',
			'+set', 'r_specularMapping', '0',
			'+set', 'r_deluxeMapping', '0',
			'+set', 'r_hdr', '0',
			'+set', 'r_picmip', '0',
			'+set', 'g_spVideos', '\\tier1\\1\\tier2\\2\\tier3\\3\\tier4\\4\\tier5\\5\\tier6\\6\\tier7\\7\\tier8\\8',
			'+set', 'g_spSkill', '5',
			'+set', 'g_spScores5', '\\l21\\5\\l14\\5\\l22\\5\\l25\\5\\l5\\5\\l3\\5\\l2\\5\\l20\\2\\l19\\1\\l1\\5\\l0\\5\\l24\\1',
			'+iamacheater',
			'+iamamonkey',
			'+exec', 'client.cfg',
			//	'+map', 'Q3DM17'
			*/
		],
		DoXHR: function (url, opts) {
			if (!url) {
				return opts.onload(new Error('Must provide a URL'));
			}

			var req = new XMLHttpRequest();
			req.open('GET', url, true);
			if (opts.dataType &&
				// responseType json not implemented in webkit, we'll do it manually later on
				opts.dataType !== 'json') {
				req.responseType = opts.dataType;
			}
			req.onprogress = function (ev) {
				if (opts.onprogress) {
					opts.onprogress(ev.loaded, ev.total);
				}
			};
			req.onload = function () {
				var err = null;
				var data = req.response;

				if (!(req.status >= 200 && req.status < 300 || req.status === 304)) {
					err = new Error('Couldn\'t load ' + url + '. Status: ' + req.statusCode);
				} else {
					// manually parse out a request expecting a JSON response
					if (opts.dataType === 'json') {
						try {
							data = JSON.parse(data);
						} catch (e) {
							err = e;
						}
					}
				}

				if (opts.onload) {
					opts.onload(err, data);
				}
			};
			req.send(null);
		},
		getQueryCommands: function () {
			var search = /([^&=]+)/g;
			var query  = window.location.search.substring(1);

			var args = [];

			var match;
			while (match = search.exec(query)) {
				var val = decodeURIComponent(match[1]);
				val = val.split(' ');
				val[0] = '+' + val[0];
				args.push.apply(args, val);
			}
			args.push.apply(args, [
				'+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
				'+set', 'r_customHeight', '' + window.innerHeight,
				'+set', 'r_customWidth', '' + window.innerWidth,
			]);

			return args;
		}
	},
	Sys_PlatformInit: function () {
		SYS.loading = document.getElementById('loading');
		SYS.dialog = document.getElementById('dialog');
		if(SYSC.eula) {
			// add eula frame to viewport
			var eula = document.createElement('div');
			eula.id = 'eula-frame';
			eula.innerHTML = '<div id="eula-frame-inner">' +
				'<p>In order to continue, the official Quake3 demo will need to be installed into the browser\'s persistent storage.</p>' +
				'<p>Please read through the demo\'s EULA and click "I Agree" if you agree to it and would like to continue.</p>' +
				'<pre id="eula">' + SYSC.eula + '</pre>' +
				'<button id="agree" class="btn btn-success">I Agree</button>' +
				'<button id="dont-agree" class="btn btn-success">I Don\'t Agree</button>' +
				'</div>';
			SYS.eula = Module['viewport'].appendChild(eula);
		}
	},
	Sys_PlatformExit: function () {
		var handler = Module['exitHandler'];
		if (handler) {
			if (!SYS.exited) {
				handler();
			}
			return;
		}
		window.removeEventListener('resize', resizeViewport);
		if(SYS.loading) {
			SYS.loading.remove();
			SYS.loading = null;

			if(SYS.eula) {
				SYS.eula.remove();
				SYS.eula = null;
			}
		}

		if (Module['canvas']) {
			Module['canvas'].remove();
		}
	},
	Sys_GLimpInit: function () {
		var viewport = Module['viewport'];

		// create a canvas element at this point if one doesnt' already exist
		if (!Module['canvas']) {
			var canvas = document.createElement('canvas');
			canvas.id = 'canvas';
			canvas.width = viewport.offsetWidth;
			canvas.height = viewport.offsetHeight;

			Module['canvas'] = viewport.appendChild(canvas);
		}
	},
	Sys_GLimpSafeInit: function () {
	},
	Sys_FS_Startup__deps: ['$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
	Sys_FS_Startup: function () {
		var fs_homepath = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK)));
		var fs_basepath = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_basepath'), 'i8', ALLOC_STACK)));
		var fs_basegame = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_basegame'), 'i8', ALLOC_STACK)));
		var sv_pure = UTF8ToString(_Cvar_VariableIntegerValue(
				allocate(intArrayFromString('sv_pure'), 'i8', ALLOC_STACK)));
		// mount a persistable filesystem into base
		try {
			FS.mkdir(fs_homepath);
			FS.mkdir(fs_basepath);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
				SYSC.Error('fatal', e.message);
			}
		}

		try {
			FS.mount(IDBFS, {}, fs_homepath);
			FS.mount(IDBFS, {}, fs_basepath);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EBUSY) {
				SYSC.Error('fatal', e.message);
			}
		}

		var start = Date.now();

		FS.syncfs(true, function (err) {
			if (err) {
				SYSC.Print(err.message);
				return SYSC.Error('fatal', err.message);
			}

			SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds');
			
			try {
				FS.mkdir(PATH.join(fs_basepath, fs_basegame));
			} catch (e) {
				if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
					SYSC.Error('fatal', e.message);
				}
			}
			// TODO: remove this in favor of new remote FS code
			var downloads = []
			SYSC.DownloadAsset('/index.json', () => {}, (err, data) => {
				var json = JSON.parse((new TextDecoder("utf-8")).decode(data));
				// create virtual file entries for everything in the directory list
				Object.keys(json).forEach(k => {
					const blankFile = new Uint8Array(4);
					if(!json[k].checksum) { // create a directory
						try {
							FS.mkdir(PATH.join(fs_basepath, fs_basegame, json[k].name));
						} catch (e) {
							if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
								SYSC.Error('fatal', e.message);
							}
						}
					} else {
						// temporary FIX
						// TODO: remove this with when Async file system loading works,
						//   renderer, client, deferred loading cg_deferPlayers|loaddeferred
						if(PATH.extname(json[k].name) === '.pk3' || !sv_pure) {
							downloads.push(json[k].name);
						} else {
							FS.writeFile(PATH.join(fs_basepath, fs_basegame, json[k].name), blankFile, {
								encoding: 'binary', flags: 'w', canOwn: true });
						}
					}
				});
				
				if(downloads.length === 0) {
					Browser.safeCallback(SYSC.ProxyCallback)();
				} else {
					Promise.all(downloads.map(file => new Promise(resolve => {
						SYSC.DownloadAsset(file, () => {
							// TODO: update in game download status
						}, (err, data) => {
							if(err) return resolve(err);
							FS.writeFile(PATH.join(fs_basepath, fs_basegame, file), new Uint8Array(data), {
								encoding: 'binary', flags: 'w', canOwn: true });
							resolve(file);
						});
					}))).then(Browser.safeCallback(SYSC.ProxyCallback));
				}
				
				// TODO: create an icon for the favicon so we know we did it right
				/*
				var buf = FS.readFile('/foo/bar');
		    var blob = new Blob([buf],  {"type" : "application/octet-stream" });
		    var url = URL.createObjectURL(blob);
				var link = document.querySelector("link[rel*='icon']") || document.createElement('link');
		    link.type = 'image/x-icon';
		    link.rel = 'shortcut icon';
		    link.href = url;
		    document.getElementsByTagName('head')[0].appendChild(link);
				*/
			});
		});
	},
	Sys_FS_Shutdown__deps: ['$Browser', '$FS', '$SYSC'],
	Sys_FS_Shutdown: function () {
		FS.syncfs(function (err) {
			SYSC.FS_Shutdown(Browser.safeCallback(function (err) {
				if (err) {
					// FIXME cb_free_context(context)
					SYSC.Error('fatal', err);
					return;
				}
				
				SYSC.ProxyCallback();
			}));
		});
	},
	Sys_Milliseconds: function () {
		if (!SYS.timeBase) {
			SYS.timeBase = Date.now();
		}

		if (window.performance.now) {
			return parseInt(window.performance.now(), 10);
		} else if (window.performance.webkitNow) {
			return parseInt(window.performance.webkitNow(), 10);
		} else {
			return Date.now() - SYS.timeBase();
		}
	},
	Sys_GetCurrentUser: function () {
		var stack = stackSave();
		var ret = allocate(intArrayFromString('player'), 'i8', ALLOC_STACK);
		stackRestore(stack);
		return ret;
	},
	Sys_Dialog: function (type, message, title) {
		SYSC.Error('SYS_Dialog not implemented');
	},
	Sys_ErrorDialog: function (error) {
		var errorStr = UTF8ToString(error);

		if (typeof Module.exitHandler !== 'undefined') {
			SYS.exited = true;
			Module.exitHandler(errorStr);
			return;
		}

		var title = SYS.dialog.querySelector('.title');
		if(title) {
			title.className = 'title error';
			title.innerHTML = 'Error';

			var description = SYS.dialog.querySelector('.description');
			description.innerHTML = errorStr;

			SYS.dialog.style.display = 'block';
		}
	},
	Sys_CmdArgs__deps: ['stackAlloc'],
	Sys_CmdArgs: function () {
		var argv = ['ioq3'].concat(SYS.args).concat(SYS.getQueryCommands());
		var argc = argv.length;
		// merge default args with query string args
		var list = stackAlloc((argc + 1) * {{{ Runtime.POINTER_SIZE }}});
		for (var i = 0; i < argv.length; i++) {
			HEAP32[(list >> 2) + i] = allocateUTF8OnStack(argv[i]);
		}
		HEAP32[(list >> 2) + argc] = 0;
		return list;
	},
	Sys_CmdArgsC: function () {
		return SYS.args.length + SYS.getQueryCommands().length + 1;
	}
};

autoAddDeps(LibrarySys, '$SYS');
mergeInto(LibraryManager.library, LibrarySys);
