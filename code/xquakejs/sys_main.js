var LibrarySysMain = {
  $SYSM: {
    exited: false,
    loading: null,
    dialog: null,
    eula: null,
    timeBase: null,
    args: [
      '+set', 'fs_basepath', '/base',
      //'+set', 'sv_dlURL', '"http://localhost:8080/assets"',
      //'+set', 'cl_allowDownload', '1',
      '+set', 'fs_basegame', 'baseq3',
      '+set', 'fs_game', 'baseq3',
      //'+set', 'developer', '0',
      //'+set', 'fs_debug', '0',
      '+set', 'r_mode', '-1',
      '+set', 'r_customPixelAspect', '1',
      '+set', 'sv_pure', '1',
      //'+set', 'cg_simpleItems', '0',
      // these control the proxy server
      '+set', 'net_enabled', '1', // 1 for IPv4
      '+set', 'net_socksPort', '1081', // default 1080 but 1081 for websocket
      '+set', 'net_socksEnabled', '1',
      '+set', 'cl_lazyLoad', '1',
      '+set', 'rconpassword', 'password123!',
      '+set', 'cg_drawfps', '1',
      '+set', 'cl_guidServerUniq', '1', // more randomness for server cl_guid so lnbitcoin never overlaps clients
      
      // settings for browser that might help keep garbage low
      //'+set', 'com_hunkMegs', '256',
      //'+set', 'com_maxfps', '125',
      //'+set', 'com_maxfpsUnfocused', '10',
      //'+set', 'com_maxfpsMinimized', '10',

      // FBO specific settings
      '+set', 'r_ext_framebuffer_multisample', '0',
      '+set', 'r_ext_framebuffer_object', '0',
      // this prevents lightmap from being wrong when switching maps
      //   renderer doesn't restart between maps, but BSP loading updates
      //   textures with lightmap by default, so this keeps them separate
      '+set', 'r_mergeLightmaps', '0',
      '+set', 'r_deluxeMapping', '0',
      '+set', 'r_normalMapping', '0',
      '+set', 'r_specularMapping', '0',
      '+set', 'r_gamma', '1.5',
      '+set', 'r_picmip', '0',
      /*
      '+set', 'r_ext_direct_state_access', '0',
      '+set', 'r_cubeMapping', '0',
      '+set', 'r_postProcess', '0',
      '+set', 'r_sunlightMode', '0',
      '+set', 'r_shadowCascadeZFar', '0',
      '+set', 'r_shadowBlur', '0',
      '+set', 'r_ssao', '0',
      '+set', 'r_drawSun', '0',
      '+set', 'r_drawSunRays', '0',
      '+set', 'r_shadows', '0',
      
      // other settings QuakeJS may have changed in code
      '+set', 'r_normalMapping', '0',
      '+set', 'r_specularMapping', '0',
      '+set', 'r_deluxeMapping', '0',
      '+set', 'r_hdr', '0',
      '+set', 'r_lodbias', '1',
      '+set', 'r_postProcess', '0',
      '+set', 's_compression', '1',
      '+set', 'r_ext_compressed_textures', '1',
      */
      //'+connect', 'proxy.quake.games:443',
      
      // cheats for enabling content
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
    getQueryCommands: function () {
      var search = /([^&=]+)/g
      var query  = window.location.search.substring(1)
      var args = Array.from(SYSM.args)
      var match
      while (match = search.exec(query)) {
        var val = decodeURIComponent(match[1])
        val = val.split(' ')
        val[0] = (val[0][0] != '+' ? '+' : '') + val[0]
        args.push.apply(args, val)
      }
      args.unshift.apply(args, [
        '+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
        '+set', 'r_customHeight', '' + window.innerHeight || 0,
        '+set', 'r_customWidth', '' + window.innerWidth || 0,
      ])
      if(navigator && navigator.userAgent
        && navigator.userAgent.match(/mobile/i)) {
        args.unshift.apply(args, [
          '+set', 'com_hunkMegs', '96',
          '+set', 'in_joystick', '1',
          '+set', 'in_nograb', '1',
          '+set', 'in_mouse', '0',
          '+bind', 'mouse1', '"+attack"',
          '+bind', 'UPARROW', '"+attack"',
          '+bind', 'DOWNARROW', '"+moveup"',
          '+bind', 'LEFTARROW', '"+moveleft"',
          '+bind', 'RIGHTARROW', '"+moveright"',
          '+unbind', 'A',
          '+unbind', 'D',
        ])
      } else {
        args.unshift.apply(args, [
          '+set', 'in_joystick', '0',
          '+set', 'in_nograb', '0',
          '+set', 'in_mouse', '1',
        ])
      }
      if(window.location.hostname.match(/quake\.games/i)) {
        var match
        if (!args.includes('sv_dlURL')) {
          args.push.apply(args, [
            '+set', 'sv_dlURL', '"https://quake.games/assets"',
          ])
        }
        if(window.location.hostname.match(/lvl\.quake\.games/i)) {
          var detectMapId = (((/(&|\?|\/)id[:=]([0-9]+)($|[^0-9])/igm)
            .exec(window.location.search) || [])[2] || '')
          args.push.apply(args, [
            '+set', 'com_maxfps', '30',
            '+set', 'net_enable', '0',
            '+set', 'cg_drawGun', '0',
            '+set', 'cg_simpleItems', '0',
            '+set', 'cg_draw2D', '0',
            '+exec', 'lvl-default.cfg',
          ])
          if (!args.includes('cl_returnURL')) {
            args.push.apply(args, [
              '+set', 'cl_returnURL', '"https://lvlworld.com/review/id:' + detectMapId + '"',
            ])
          }
        } else if((match = (/(.+)\.quake\.games/i).exec(window.location.hostname))) {
          if (!args.includes('net_socksServer')) {
            args.push.apply(args, [
              '+set', 'net_socksServer', 'wss://' + window.location.hostname,
              '+set', 'net_socksPort', '443',
            ])
          }
          if(SYSF.mods.filter(function (f) { return f.includes(match[1]) }).length > 0) {
            args.unshift.apply(args, [
              '+set', 'fs_basegame', match[1],
              '+set', 'fs_game', match[1],
              '+exec', match[1] + '-default.cfg',
            ])
          }
          if (!args.includes('+map') && !args.includes('+spmap')
            && !args.includes('+devmap') && !args.includes('+spdevmap') 
            && !args.includes('+connect')) {
            args.push.apply(args, [
              '+connect', window.location.hostname,
            ])
          }
        } else if (!args.includes('net_socksServer')) {
          args.push.apply(args, [
            '+set', 'net_socksServer', 'wss://proxy.quake.games',
            '+set', 'net_socksPort', '443',
          ])
        }
      } else {
        if(window.location.hostname.match(/quake\.money/i)) {
          if (!args.includes('+map') && !args.includes('+spmap')
            && !args.includes('+devmap') && !args.includes('+spdevmap') 
            && !args.includes('+connect')) {
            args.push.apply(args, [
              '+connect', window.location.hostname,
            ])
          }
        }
        if (!args.includes('net_socksServer')) {
          args.push.apply(args, [
            '+set', 'net_socksServer', window.location.hostname,
            '+set', 'net_socksPort', SYSM.isSecured(window.location.origin) ? '443' : '1081'
          ])
        }
        if (!args.includes('sv_dlURL')) {
          args.push.apply(args, [
            '+set', 'sv_dlURL', '"' + window.location.origin + '/assets"',
          ])
        }
      }
      if(!SYS.dedicated) {
        if(!args.includes('+connect')) {
          args.push.apply(args, [
            '+connect', 'localhost'
          ])
        }
      } else {
        // IS dedicated settings
        if(!args.includes('+spmap')
          && !args.includes('+map')
          && !args.includes('+devmap')
          && !args.includes('+spdevmap')
          && !args.includes('+demo_play')) {
          args.push.apply(args, [
            '+spmap', 'q3dm0',
          ])
        }
        args.unshift.apply(args, [
          '+set', 'ttycon', '1',
          '+set', 'sv_hostname', 'Local Host',
          '+set', 'sv_motd', 'For instant replays and stuff',
          '+set', 'rconPassword', 'password123!',
          '+set', 'sv_reconnectlimit', '0',
          //  '+set', 'sv_autoDemo', '1',
          //  '+set', 'sv_autoRecord', '1',
          //  '+set', 'net_socksEnabled', '0',
        ])
      }
      return args
    },
    updateVideoCmd: function () {
			var update = 'set r_fullscreen %fs; set r_mode -1; set r_customWidth %w; set r_customHeight %h; vid_restart; '
				.replace('%fs', window.fullscreen ? '1' : '0')
				.replace('%w', window.innerWidth)
				.replace('%h', window.innerHeight)
			_Cbuf_AddText(allocate(intArrayFromString(update), 'i8', ALLOC_STACK));
		},
    resizeViewport: function () {
			if (!Module['canvas']) {
				// ignore if the canvas hasn't yet initialized
				return;
			}

			if (SYSM.resizeDelay) clearTimeout(SYSM.resizeDelay);
			SYSM.resizeDelay = setTimeout(Browser.safeCallback(SYSM.updateVideoCmd), 100);
		},
    isSecured: function (socksServer) {
      return (window.location.search.includes('https://')
        || window.location.protocol.includes('https')
        || window.location.search.includes('wss://')
        || socksServer.includes('wss:')
        || socksServer.includes('https:'))
        && !socksServer.includes('http:')
        && !socksServer.includes('ws:')
        && !window.location.search.includes('ws://')
    }
  },
  Sys_PlatformInit__deps: ['$SYSC', '$SYSM'],
  Sys_PlatformInit: function () {
    SYSC.varStr = allocate(new Int8Array(4096), 'i8', ALLOC_NORMAL)
    SYSC.newDLURL = SYSC.oldDLURL = SYSC.Cvar_VariableString('sv_dlURL')
    Object.assign(Module, {
      websocket: Object.assign(Module.websocket || {}, {
        url: SYSM.isSecured(SYSC.Cvar_VariableString('net_socksServer'))
        ? 'wss://'
        : 'ws://'
      })
    })
    SYSN.lazyInterval = setInterval(SYSN.DownloadLazy, 50)

    if(typeof window.serviceWorker != 'undefined')
      window.serverWorker.postMessage(['init', SYSM.getQueryCommands()])
    
    if(SYS.dedicated) return

    SYSM.loading = document.getElementById('loading')
    SYSM.dialog = document.getElementById('dialog')
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
      SYSM.eula = Module['viewport'].appendChild(eula)
    }
    window.addEventListener('resize', SYSM.resizeViewport)
  },
  Sys_PlatformExit: function () {
    SYSC.returnURL = SYSC.Cvar_VariableString('cl_returnURL')
    if(SYSN.lazyInterval)
      clearInterval(SYSN.lazyInterval)
    if(SYSN.socksInterval)
      clearInterval(SYSN.socksInterval)
    if(SYSI.interval)
      clearInterval(SYSI.interval)
    /*
    if(SYSC.varStr) {
      _free(SYSC.varStr)
      SYSC.varStr = 0
    }
    if(SYSI.inputHeap) {
      _free(SYSI.inputHeap)
      SYSI.inputHeap = 0
    }
    */
    if(Module.SDL2) {
      delete Module.SDL2.audio
      delete Module.SDL2.capture
    }
    if(typeof flipper != 'undefined') {
      flipper.style.display = 'block'
      flipper.style.animation = 'none'
    }
    SYSM.exited = true
    if(!SYS.dedicated) {
      window.removeEventListener('resize', SYSM.resizeViewport)
      window.removeEventListener('keydown', SYSI.InputPushKeyEvent)
      window.removeEventListener('keyup', SYSI.InputPushKeyEvent)
      window.removeEventListener('keypress', SYSI.InputPushTextEvent)
    }
    
    //Module['canvas'].addEventListener('mousemove', SYSI.InputPushMouseEvent, false)
    //Module['canvas'].addEventListener('mousedown', SYSI.InputPushMouseEvent, false)
    //Module['canvas'].addEventListener('mouseup', SYSI.InputPushMouseEvent, false)
    //Module['canvas'].addEventListener('mousewheel', SYSI.InputPushWheelEvent, false)

    if (Module['canvas']) {
      Module['canvas'].remove()
    }
    if(SYSC.returnURL) {
      window.location = SYSC.returnURL
    }
    if(typeof Module.exitHandler != 'undefined') {
      Module.exitHandler()
    }
  },
  Sys_Milliseconds: function () {
		if (!SYSM.timeBase) {
			SYSM.timeBase = Date.now()
		}

		if (window.performance && window.performance.now) {
			return parseInt(window.performance.now(), 10)
		} else if (window.performance && window.performance.webkitNow) {
			return parseInt(window.performance.webkitNow(), 10)
		} else {
			return Date.now() - SYSM.timeBase
		}
	},
	Sys_GetCurrentUser: function () {
		return allocate(intArrayFromString('player'), 'i8', ALLOC_STACK)
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
    var title = SYSM.dialog.querySelector('.title')
    if(title) {
      title.className = 'title error'
      title.innerHTML = 'Error'
      var description = SYSM.dialog.querySelector('.description')
      description.innerHTML = errorStr
      SYSM.dialog.style.display = 'block'
    }
    if (typeof Module.exitHandler != 'undefined') {
      SYSM.exited = true
      Module.exitHandler(errorStr)
      return
    }
  },
  Sys_SetStatus__deps: ['$SYSN'],
  Sys_SetStatus: function (s) {
    var args = Array.from(arguments)
      .map(function (a) { return UTF8ToString(a) })
    SYSC.Print(args)
    window.serverWorker.postMessage(['status', args])
  },
  Sys_CmdArgs: function () {
    var argv = ['ioq3'].concat(SYSM.getQueryCommands())
    var argc = argv.length
    // merge default args with query string args
    var size = (argc + 1) * {{{ Runtime.POINTER_SIZE }}}
    var list = allocate(new Int8Array(size), 'i8', ALLOC_NORMAL)
    for (var i = 0; i < argv.length; i++) {
      HEAP32[(list >> 2) + i] = allocateUTF8OnStack(argv[i])
    }
    HEAP32[(list >> 2) + argc] = 0
    return list
  },
  Sys_CmdArgsC: function () {
    return SYSM.getQueryCommands().length + 1
  },
}
autoAddDeps(LibrarySysMain, '$SYSM')
mergeInto(LibraryManager.library, LibrarySysMain);
