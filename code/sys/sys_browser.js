var LibrarySys = {
	$SYS__deps: ['$SYSC'],
	$SYS: {
		exited: false,
		timeBase: null,
		style: null,
		// Lets make a list of supported mods, 'dirname-ccr' (-ccr means combined converted repacked)
		//   To the right is the description text, atomatically creates a placeholder.pk3dir with description.txt inside
		// We use a list here because Object keys have no guarantee of order
		mods: [
			['missionpack-ccr', '0 Choice: Team Arena'],
			['defrag-ccr',      '1 Choice: Defrag'],
			['q3rally-ccr',     '2 Choice: Q3Rally'],
			['baseoq-ccr', 			'3 Choice: Open Arena'],
			['generations-ccr', '4 Choice: Generations Arena'],
			['q3f2-ccr', 				'5 Choice: Q3 Fortress 2'],
			['cpma-ccr', 				'6 Choice: Challenge ProMode'],
			['q3ut4-ccr', 			'7 Choice: Urban Terror 4'],
			['freezetag-ccr', 	'8 Choice: Freeze Tag'],
			['corkscrew-ccr', 	'9 Choice: Corkscrew'],
			['bfpq3-ccr', 			'Bid For Power'],
			['excessive-ccr', 	'Excessive+'],
			['q3ut3-ccr', 			'Urban Terror 3'],
			['edawn-ccr', 			'eDawn'],
			['geoball-ccr', 		'Geoball'],
			['neverball-ccr', 	'Neverball'],
			['platformer-ccr', 	'Platformer'],
			['legoc-ccr', 			'Lego Carnage'],
			['osp-ccr', 				'Orange Smoothie Productions'],
			['quake2arena-ccr', 'Quake 2 Arena'],
			['wfa-ccr', 				'Weapons Factory Arena'],
			['uberarena-ccr', 	'Uber Arena'],
			['demoq3-ccr', 			'Quake III Demo'],
			['mfdata-ccr', 			'Military Forces'],
			['conjunction-ccr', 'Dark Conjunction'],
			['chili-ccr', 			'Chili Quake XXL'],
			['hqq-ccr', 				'High Quality Quake'],
			['rocketarena-ccr', 'Coming Soon: Rocket Arena'],
			['gpp-ccr',         'Coming Soon: Tremulous'],
			['gppl-ccr',        'Coming Soon: Unvanquished'],
			['iortcw-ccr',      'Coming Soon: Return to Castle Wolfenstien'],
		],
		args: [
			'+set', 'sv_dlURL', '"http://localhost:8080/assets"',
			'+set', 'cl_allowDownload', '1',
			'+set', 'fs_basegame', 'baseq3-ccr',
			'+set', 'fs_game', 'baseq3-ccr',
			'+set', 'developer', '0',
			'+set', 'fs_debug', '0',
			'+set', 'r_mode', '-1',
			'+set', 'r_customPixelAspect', '1',
			'+set', 'sv_pure', '0',
			'+set', 'cg_simpleItems', '0',
			// these control the proxy server
			'+set', 'net_enabled', '1', // 1 for IPv4
			'+set', 'net_socksServer', '127.0.0.1',
			'+set', 'net_socksPort', '1081', // default 1080 but 1081 for websocket
			'+set', 'net_socksEnabled', '1',
			// these settings were set by the emscripten build
			//'+set', 'r_normalMapping', '0',
			//'+set', 'r_specularMapping', '0',
			//'+set', 'r_deluxeMapping', '0',
			//'+set', 'r_hdr', '0',
			//'+set', 'r_picmip', '0',
			//'+set', 'r_postProcess', '0',
			'+set', 'cg_drawfps', '1',
			//'+connect', 'proxy.quake.games:443',
			/*
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
				return opts.onload(new Error('Must provide a URL'))
			}

			var req = new XMLHttpRequest()
			req.open('GET', url, true)
			if (opts.dataType &&
				// responseType json not implemented in webkit, we'll do it manually later on
				opts.dataType !== 'json') {
				req.responseType = opts.dataType
			}
			req.onprogress = function (ev) {
				if (opts.onprogress) {
					opts.onprogress(ev.loaded, ev.total)
				}
			}
			req.onload = function () {
				var err = null
				var data = req.response
				if (!(req.status >= 200 && req.status < 300 || req.status === 304)) {
					err = new Error('Couldn\'t load ' + url + '. Status: ' + req.statusCode)
				} else {
					// manually parse out a request expecting a JSON response
					if (opts.dataType === 'json') {
						try {
							data = JSON.parse(data)
						} catch (e) {
							err = e
						}
					}
				}

				if (opts.onload) {
					opts.onload(err, data)
				}
			}
			req.send(null)
		},
		getQueryCommands: function () {
			var search = /([^&=]+)/g
			var query  = window.location.search.substring(1)
			var args = []
			var match
			while (match = search.exec(query)) {
				var val = decodeURIComponent(match[1])
				val = val.split(' ')
				val[0] = '+' + val[0]
				args.push.apply(args, val)
			}
			args.push.apply(args, [
				'+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
				'+set', 'r_customHeight', '' + window.innerHeight,
				'+set', 'r_customWidth', '' + window.innerWidth,
			])
			return args
		},
		updateVideoCmd: function () {
			var update = 'set r_fullscreen %fs; set r_mode -1; set r_customWidth %w; set r_customHeight %h; vid_restart; '
				.replace('%fs', window.fullscreen ? '1' : '0')
				.replace('%w', window.innerWidth)
				.replace('%h', window.innerHeight)
			Module._Cbuf_AddText(allocate(intArrayFromString(update), 'i8', ALLOC_STACK));
			Module._Cbuf_Execute();
		},
		resizeDelay: null,
		resizeViewport: function () {
			if (!Module['canvas']) {
				// ignore if the canvas hasn't yet initialized
				return;
			}

			if (SYS.resizeDelay) clearTimeout(SYS.resizeDelay);
			SYS.resizeDelay = setTimeout(SYS.updateVideoCmd, 100);
		},
		quitGameOnUnload: function (e) {
			if(Module['canvas']) {
				Module._Cbuf_AddText(allocate(intArrayFromString('quit;'), 'i8', ALLOC_STACK));
				Module._Cbuf_Execute();
				Module['canvas'].remove()
				Module['canvas'] = null
			}
			return false
		},
		LoadingDescription: function (desc) {
			var flipper = document.getElementById('flipper')
			var progress = document.getElementById('loading-progress')
			var description = progress.querySelector('.description')
			if (!desc) {
				progress.style.display = 'none'
				flipper.style.display = 'none'
				SYS.LoadingProgress(0)
			} else {
				progress.style.display = 'block'
				flipper.style.display = 'block'
			}
			description.innerHTML = desc
		},
		LoadingProgress: function (progress, total) {
			var frac = progress / total
			var progress = document.getElementById('loading-progress')
			var bar = progress.querySelector('.bar')
			bar.style.width = (frac*100) + '%'
		},
	},
	Sys_PlatformInit: function () {
		SYS.loading = document.getElementById('loading')
		SYS.dialog = document.getElementById('dialog')
		
		// TODO: load this the same way demo does
		if(SYSC.eula) {
			// add eula frame to viewport
			var eula = document.createElement('div')
			eula.id = 'eula-frame'
			eula.innerHTML = '<div id="eula-frame-inner">' +
				'<p>In order to continue, the official Quake3 demo will need to be installed into the browser\'s persistent storage.</p>' +
				'<p>Please read through the demo\'s EULA and click "I Agree" if you agree to it and would like to continue.</p>' +
				'<pre id="eula">' + SYSC.eula + '</pre>' +
				'<button id="agree" class="btn btn-success">I Agree</button>' +
				'<button id="dont-agree" class="btn btn-success">I Don\'t Agree</button>' +
				'</div>'
			SYS.eula = Module['viewport'].appendChild(eula)
		}
		Object.assign(Module, {
			websocket: Object.assign(Module.websocket || {}, {
				url: window.location.search.includes('https://')
				? 'wss://'
				: 'ws://'
			})
		})
		window.addEventListener('resize', SYS.resizeViewport)
	},
	Sys_PlatformExit: function () {
		flipper.style.display = 'block'
		flipper.style.animation = 'none'
		SYS.exited = true
		window.removeEventListener('resize', SYS.resizeViewport)

		if (Module['canvas']) {
			Module['canvas'].remove()
		}
		if(typeof Module.exitHandler != 'undefined') {
			Module.exitHandler()
		}
	},
	Sys_GLimpInit: function () {
		var viewport = document.getElementById('viewport-frame')
		// create a canvas element at this point if one doesnt' already exist
		if (!Module['canvas']) {
			var canvas = document.createElement('canvas')
			canvas.id = 'canvas'
			canvas.width = viewport.offsetWidth
			canvas.height = viewport.offsetHeight
			Module['canvas'] = viewport.appendChild(canvas)
		}
	},
	Sys_GLimpSafeInit: function () {
	},
	Sys_BeginDownload__deps: ['$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
	Sys_BeginDownload: function () {
		var cl_downloadName = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('cl_downloadName'), 'i8', ALLOC_STACK)))
		var fs_basepath = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_basepath'), 'i8', ALLOC_STACK)))
		var fs_game = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_game'), 'i8', ALLOC_STACK)))
		
		SYSC.mkdirp(PATH.join(fs_basepath, PATH.dirname(cl_downloadName)))
		
		SYSC.DownloadAsset(cl_downloadName, (loaded, total) => {
			_Cvar_SetValue(allocate(intArrayFromString('cl_downloadSize'), 'i8', ALLOC_STACK), total );
			_Cvar_SetValue(allocate(intArrayFromString('cl_downloadCount'), 'i8', ALLOC_STACK), loaded );
		}, (err, data) => {
			if(err) {
				SYSC.Error('drop', 'Download Error: ' + err.message)
				return
			} else {
				FS.writeFile(PATH.join(fs_basepath, cl_downloadName), new Uint8Array(data), {
					encoding: 'binary', flags: 'w', canOwn: true })
			}
			FS.syncfs(false, Browser.safeCallback(_CL_NextDownload))
		})
	},
	Sys_FS_Startup__deps: ['$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
	Sys_FS_Startup: function (cb) {
		var fs_homepath = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_homepath'), 'i8', ALLOC_STACK)))
		var fs_basepath = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_basepath'), 'i8', ALLOC_STACK)))
		var fs_basegame = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_basegame'), 'i8', ALLOC_STACK)))
		var sv_pure = _Cvar_VariableIntegerValue(
			allocate(intArrayFromString('sv_pure'), 'i8', ALLOC_STACK))
		var fs_game = UTF8ToString(_Cvar_VariableString(
			allocate(intArrayFromString('fs_game'), 'i8', ALLOC_STACK)))
		var clcState = _CL_GetClientState()
		const blankFile = new Uint8Array(4)
		
		SYS.LoadingDescription('Manifest')
		var fsMountPath = fs_basegame
		if(fs_game && fs_game.localeCompare(fs_basegame) !== 0) {
			fsMountPath = fs_game // TODO: comment this out to test server induced downloading
		}
		
		// mount a persistable filesystem into base
		SYSC.mkdirp(fs_basepath)

		try {
			FS.mount(IDBFS, {}, fs_basepath)
		} catch (e) {
			if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EBUSY) {
				SYSC.Error('fatal', e.message)
			}
		}
		
		var start = Date.now()
		// read from drive
		FS.syncfs(true, function (err) {
			if (err) {
				debugger
				SYSC.Print(err.message)
				return SYSC.Error('fatal', err.message)
			}

			SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds')
			SYSC.mkdirp(PATH.join(fs_basepath, fsMountPath))

			// TODO: is this right? exit early without downloading anything so the server can force it instead
			// server will tell us what pk3s we need
			if(clcState < 4 && sv_pure && fs_game.localeCompare(fs_basegame) !== 0) {
				SYS.LoadingDescription('')
				FS.syncfs(false, () => SYSC.ProxyCallback(cb))
				return
			}

			// TODO: remove this in favor of new remote FS code
			var downloads = []
			SYSC.DownloadAsset(fsMountPath + '/index.json', SYS.LoadingProgress, (err, data) => {
				if(err) {
					SYSC.ProxyCallback(cb)
					return
				}
				FS.writeFile(PATH.join(fs_basepath, fsMountPath, "index.json"), new Uint8Array(data), {
					encoding: 'binary', flags: 'w', canOwn: true })				
				var json = JSON.parse((new TextDecoder("utf-8")).decode(data))
				// create virtual file entries for everything in the directory list
				var keys = Object.keys(json)
				var menu = keys.filter(k => k.includes('menu/'))
				var game = keys.filter(k => k.includes('game/'))
				if(clcState >= 4 && game.length) keys = game;
				if(clcState < 4 && menu.length) keys = menu;
				for(var i = 0; i < keys.length; i++) {
					var file = json[keys[i]]
					if(typeof file.size == 'undefined') { // create a directory
						SYSC.mkdirp(PATH.join(fs_basepath, fsMountPath, file.name))
					} else {
						// TODO: remove this check when webworker is installed
						//   because it will check ETag and replace files
						// only download again if the file does not exist
						try {
							var handle = FS.open(PATH.join(fs_basepath, fsMountPath, file.name), 'r')
							FS.close(handle)
							continue
						} catch (e) {
							if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
								SYSC.Error('fatal', e.message)
							}
						}
						// temporary FIX
						// TODO: remove this with when Async file system loading works,
						//   renderer, client, deferred loading cg_deferPlayers|loaddeferred
						if(PATH.extname(file.name) === '.pk3'
						  || PATH.extname(file.name) === '.wasm'
							|| PATH.extname(file.name) === '.qvm'
							|| !sv_pure) {
							downloads.push(PATH.join(fsMountPath, file.name))
						} else {
							try {
								FS.writeFile(PATH.join(fs_basepath, fsMountPath, file.name), blankFile, {
									encoding: 'binary', flags: 'w', canOwn: true })
							} catch (e) {
								if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
									SYSC.Error('fatal', e.message)
								}
							}
						}
					}
				}
				
				var totals = []
				var progresses = []
				if(downloads.length === 0) {
					SYS.LoadingDescription('')
					SYSC.ProxyCallback(cb)
				} else {
					Promise.all(downloads.map((file, i) => new Promise(resolve => {
						totals[i] = 0
						progresses[i] = 0
						SYS.LoadingDescription(file)
						SYSC.DownloadAsset(file, (progress, total) => {
							totals[i] = Math.max(progress, total || 10*1024*1024) // assume its somewhere around 10 MB per pak
							progresses[i] = progress
							SYS.LoadingProgress(
								progresses.reduce((s, p) => s + p, 0),
								totals.reduce((s, p) => s + p, 0))
						}, (err, data) => {
							progresses[i] = data.byteLength
							SYS.LoadingProgress(
								progresses.reduce((s, p) => s + p, 0),
								totals.reduce((s, p) => s + p, 0))
							if(err) return resolve(err)
							try {
								FS.writeFile(PATH.join(fs_basepath, file), new Uint8Array(data), {
									encoding: 'binary', flags: 'w', canOwn: true })
							} catch (e) {
								if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
									SYSC.Error('fatal', e.message)
								}
							}
							resolve(file)
						})
						// save to drive
					}))).then(() => {
						SYS.LoadingDescription('')
						FS.syncfs(false, () => SYSC.ProxyCallback(cb))
					})
				}
				
				// TODO: create an icon for the favicon so we know we did it right
				/*
				var buf = FS.readFile('/foo/bar')
		    var blob = new Blob([buf],  {"type" : "application/octet-stream" })
		    var url = URL.createObjectURL(blob)
				var link = document.querySelector("link[rel*='icon']") || document.createElement('link')
		    link.type = 'image/x-icon'
		    link.rel = 'shortcut icon'
		    link.href = url
		    document.getElementsByTagName('head')[0].appendChild(link)
				*/
			})
		})
	},
	Sys_FS_Shutdown__deps: ['$Browser', '$FS', '$SYSC'],
	Sys_FS_Shutdown: function (cb) {
		// save to drive
		FS.syncfs(function (err) {
			SYSC.FS_Shutdown(function (err) {
				if (err) {
					// FIXME cb_free_context(context)
					SYSC.Error('fatal', err)
					return
				}
				
				SYSC.ProxyCallback(cb)
			})
		})
	},
	Sys_SocksConnect__deps: ['$Browser', '$SOCKFS'],
	Sys_SocksConnect: function () {
		Module['websocket'].on('open', Browser.safeCallback(() => {
			_SOCKS_Frame_Proxy()
		}))
		Module['websocket'].on('message', Browser.safeCallback(() => {
			_SOCKS_Frame_Proxy()
		}))
		Module['websocket'].on('error', Browser.safeCallback(() => {
			_SOCKS_Frame_Proxy()
		}))
	},
	Sys_SocksMessage__deps: ['$Browser', '$SOCKFS'],
	Sys_SocksMessage: function () {
	},
	Sys_Milliseconds: function () {
		if (!SYS.timeBase) {
			SYS.timeBase = Date.now()
		}

		if (window.performance.now) {
			return parseInt(window.performance.now(), 10)
		} else if (window.performance.webkitNow) {
			return parseInt(window.performance.webkitNow(), 10)
		} else {
			return Date.now() - SYS.timeBase()
		}
	},
	Sys_GetCurrentUser: function () {
		var stack = stackSave()
		var ret = allocate(intArrayFromString('player'), 'i8', ALLOC_STACK)
		stackRestore(stack)
		return ret
	},
	Sys_Dialog: function (type, message, title) {
		SYSC.Error('SYS_Dialog not implemented')
	},
	Sys_ErrorDialog: function (error) {
		var errorStr = UTF8ToString(error)
		// print call stack so we know where the error came from
		try {
			throw new Error(errorStr)
		} catch (e) {
			console.log(e)
		}
		var title = SYS.dialog.querySelector('.title')
		if(title) {
			title.className = 'title error'
			title.innerHTML = 'Error'
			var description = SYS.dialog.querySelector('.description')
			description.innerHTML = errorStr
			SYS.dialog.style.display = 'block'
		}
		if (typeof Module.exitHandler != 'undefined') {
			SYS.exited = true
			Module.exitHandler(errorStr)
			return
		}
	},
	Sys_CmdArgs__deps: ['stackAlloc'],
	Sys_CmdArgs: function () {
		var argv = ['ioq3'].concat(SYS.args).concat(SYS.getQueryCommands())
		var argc = argv.length
		// merge default args with query string args
		var list = stackAlloc((argc + 1) * {{{ Runtime.POINTER_SIZE }}})
		for (var i = 0; i < argv.length; i++) {
			HEAP32[(list >> 2) + i] = allocateUTF8OnStack(argv[i])
		}
		HEAP32[(list >> 2) + argc] = 0
		return list
	},
	Sys_CmdArgsC: function () {
		return SYS.args.length + SYS.getQueryCommands().length + 1
	}
}
autoAddDeps(LibrarySys, '$SYS')
mergeInto(LibraryManager.library, LibrarySys);
