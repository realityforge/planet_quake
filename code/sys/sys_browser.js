var LibrarySys = {
	$SYS__deps: ['$SYSC'],
	$SYS: {
		exited: false,
		timeBase: null,
		style: null,
		loading: null,
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
	Sys_FS_Startup__deps: ['$Browser', '$FS', '$IDBFS', '$SYSC'],
	Sys_FS_Startup: function () {
		var name = allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK);
		var fs_homepath = UTF8ToString(_Cvar_VariableString(name));

		// mount a persistable filesystem into base
		var dir;
		try {
			dir = FS.mkdir(fs_homepath);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
				SYSC.Error('fatal', e.message);
			}
		}

		try {
			FS.mount(IDBFS, {}, fs_homepath);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EBUSY) {
				SYSC.Error('fatal', e.message);
			}
		}

		var start = Date.now();

		FS.syncfs(true, function (err) {
			if (err) {
				SYSC.Print(err.message)
				return SYSC.Error('fatal', err.message);
			}

			SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds');

			// TODO: remove this in favor of new remote FS code
			SYSC.FS_Startup(Browser.safeCallback(function (err) {
				if (err) {
					SYSC.Error('fatal', err);
					return;
				}

				SYSC.ProxyCallback();
			}));
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
	}
};

autoAddDeps(LibrarySys, '$SYS');
mergeInto(LibraryManager.library, LibrarySys);
