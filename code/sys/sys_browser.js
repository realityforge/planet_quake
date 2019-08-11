var LibrarySys = {
	$SYS__deps: ['$SYSC'],
	$SYS: {
		exited: false,
		timeBase: null,
		style: null,
		loading: null,
		css: '@import url(\'https://fonts.googleapis.com/css?family=Roboto&display=swap\');' +
			'.btn {' +
			'	display: inline-block;' +
			'	margin-bottom: 0;' +
			'	font-weight: 400;' +
			'	text-align: center;' +
			'	vertical-align: middle;' +
			'	cursor: pointer;' +
			'	background-image: none;' +
			'	border: 1px solid transparent;' +
			'	white-space: nowrap;' +
			'	padding: 6px 12px;' +
			'	font-size: 14px;' +
			'	line-height: 1.428571429;' +
			'	border-radius: 4px;' +
			'}' +
			'.btn-success {' +
			'	color: #fff;' +
			'	background-color: #5cb85c;' +
			'	border-color: #4cae4c;' +
			'}' +
			'.btn-success:hover {' +
			'	background-color: #47a447;' +
			'	border-color: #398439;' +
			'}' +
			'#eula-frame {' +
			'	display: none;' +
			'	position: absolute;' +
			'	top: 0;' +
			'	left: 0;' +
			'	bottom: 0;' +
			'	right: 0;' +
			'	overflow-y: auto;' +
			'}' +
			'#eula-frame-inner {' +
			'	max-width: 940px;' +
			'	margin: 0 auto;' +
			'	padding: 5em 0;' +
			'	background: rgba(0, 0, 0, 0.9);' +
			'	color: #eee;' +
			'	font-size: 1.2em;' +
			'}' +
			'#eula-frame p {' +
			'	font-family: Roboto,"Droid Sans","Google Sans Display",-apple-system,BlinkMacSystemFont,sans-serif,"Open Sans",Arial,Helvetica' +
			'}' +
			'#eula {' +
			'	color: #ccc;' +
			'	height: 25em;' +
			'	margin: 3em 0;' +
			'	overflow-y: scroll;' +
			'	white-space: pre-wrap;' +
			'}' +
			'#dialog {' +
			'	display: none;' +
			'	position: absolute;' +
			'	top: 50%;' +
			'	left: 50%;' +
			'	margin-top: -5em;' +
			'	margin-left: -12em;' +
			'	width: 24em;' +
			'	background: #2a2a2a;' +
			'}' +
			'#dialog .title {' +
			'	margin: 0;' +
			'	padding: 0.2em 0.5em;' +
			'	background: #333;' +
			'	color: #fff;' +
			'	font-family: Roboto,"Droid Sans","Google Sans Display",-apple-system,BlinkMacSystemFont,sans-serif,"Open Sans",Arial,Helvetica;' +
			'	font-weight: bold;' +
			'	text-transform: uppercase;' +
			'}' +
			'#dialog .description {' +
			'	margin: 0;' +
			'	padding: 0.5em;' +
			'	color: #fff;' +
			'	font-family: Roboto,"Droid Sans","Google Sans Display",-apple-system,BlinkMacSystemFont,sans-serif,"Open Sans",Arial,Helvetica;' +
			'}' +
			'#loading {' +
			'	display: none;' + 
			'	position: absolute;' +
			'	top: 0;' +
			'	left: 0;' +
			'	bottom: 0;' +
			'	right: 0;' +
			'	z-index: 1;' +
			'}' +
			'#loading-progress {' +
			'	position: absolute;' +
			'	top: 50%;' +
			'	left: 50%;' +
			'	margin-top: -1em;' +
			'	margin-left: -12em;' +
			'	padding: 0.4em;' +
			'	width: 24em;' +
			'	background: rgba(0, 0, 0, 0.7);' +
			'}' +
			'#loading-progress .description {' +
			'	text-align: center;' +
			'	color: #fff;' +
			'	font-family: Roboto,"Droid Sans","Google Sans Display",-apple-system,BlinkMacSystemFont,sans-serif,"Open Sans",Arial,Helvetica;' +
			'	text-shadow: 1px 1px 2px #000;' +
			'	text-transform: uppercase;' +
			'}' +
			'#loading-progress .bar-wrapper {' +
			'	background: #222;' +
			'	border-radius: 2px;' +
			'}' +
			'#loading-progress .bar {' +
			'	width: 0;' +
			'	background: #2dbb30;' +
			'	box-shadow: inset 0 -1px 0 rgba(0, 0, 0, .15);' +
			'}',
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
		LoadingDescription: function (desc) {
			var progress = document.getElementById('loading-progress');
			var description = progress.querySelector('.description');

			if (!desc) {
				SYS.loading.style.display = 'none';
				SYS.LoadingProgress(0);
			} else {
				SYS.loading.style.display = 'block';
			}

			description.innerHTML = desc;
		},
		LoadingProgress: function (frac) {
			var progress = document.getElementById('loading-progress');
			var bar = progress.querySelector('.bar');

			bar.style.width = (frac*100) + '%';
		},
		PromptEULA: function (callback) {
			var agree = document.getElementById('agree');
			var dontAgree = document.getElementById('dont-agree');

			SYS.eula.style.display = 'block';

			agree.onclick = function () {
				SYS.eula.style.display = 'none';
				agree.onclick = null;
				return callback();
			};

			dontAgree.onclick = function () {
				SYS.eula.style.display = 'none';
				dontAgree.onclick = null;
				return callback(new Error('You must agree to the EULA to continue'));
			};
		}
	},
	Sys_PlatformInit: function () {
		// embed our css
		var style = document.createElement('style');
		style.setAttribute('type', 'text/css');
		style.innerHTML = SYS.css;
		SYS.style = document.getElementsByTagName('head')[0].appendChild(style);

		// add dialog elements to viewport
		var dialog = document.createElement('div');
		dialog.id = 'dialog';
		dialog.innerHTML = '<h4 class="title"></h4>' +
			'<p class="description"></p>';
		SYS.dialog = Module['viewport'].appendChild(dialog);

		// add loading elements to viewport
		var loading = document.createElement('div');
		loading.id = 'loading';
		loading.innerHTML = '<div id="loading-progress">' +
			'	<div class="description"></div>' +
			'	<div class="bar-wrapper"><div class="bar">&nbsp;</div></div>' +
			'</div>';
		SYS.loading = Module['viewport'].appendChild(loading);

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
	},
	Sys_PlatformExit: function () {
		var handler = Module['exitHandler'];
		if (handler) {
			if (!SYS.exited) {
				SYS.exited = true;
				handler();
			}
			return;
		}

		// we want to leave around an error dialog if one exists
		// SYS.style.remove();
		// SYS.style = null;

		// SYS.dialog.remove();
		// SYS.dialog = null;

		if(SYS.loading) {
			SYS.loading.remove();
			SYS.loading = null;

			SYS.eula.remove();
			SYS.eula = null;
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
			canvas.id = 'viewport';
			canvas.width = viewport.offsetWidth;
			canvas.height = viewport.offsetHeight;

			Module['canvas'] = viewport.appendChild(canvas);
		}
	},
	Sys_GLimpSafeInit: function () {
	},
	Sys_FS_Startup__deps: ['$Browser', '$FS', '$IDBFS', '$SYSC'],
	Sys_FS_Startup: function (context) {
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
				//return SYSC.Error('fatal', err.message);
			}

			SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds');

			SYSC.FS_Startup(Browser.safeCallback(function (err) {
				if (err) {
					// FIXME cb_free_context(context)
					SYSC.Error('fatal', err);
					return;
				}

				SYSC.ProxyCallback(context);
			}));
		});
	},
	Sys_FS_Shutdown__deps: ['$Browser', '$FS', '$SYSC'],
	Sys_FS_Shutdown: function (context) {
		var name = allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK);
		var fs_homepath = UTF8ToString(_Cvar_VariableString(name));

		FS.syncfs(function (err) {
			SYSC.FS_Shutdown(Browser.safeCallback(function (err) {
				if (err) {
					// FIXME cb_free_context(context)
					SYSC.Error('fatal', err);
					return;
				}

				SYSC.ProxyCallback(context);
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
		error = UTF8ToString(error);

		var handler = Module['exitHandler'];
		if (handler) {
			SYS.exited = true;
			handler(error);
			return;
		}

		var title = SYS.dialog.querySelector('.title');
		title.className = 'title error';
		title.innerHTML = 'Error';

		var description = SYS.dialog.querySelector('.description');
		description.innerHTML = error;

		SYS.dialog.style.display = 'block';
	}
};

autoAddDeps(LibrarySys, '$SYS');
mergeInto(LibraryManager.library, LibrarySys);
