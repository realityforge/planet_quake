var LibrarySysMain = {
  $SYSM: {
    exited: false,
    loading: null,
    dialog: null,
    eula: null,
    timeBase: null,
    args: [
      '+unbind', 'F11',
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
      '+set', 'r_ext_multitexture', '0',
      '+set', 'r_ext_framebuffer_multisample', '0',
      '+set', 'r_ext_framebuffer_object', '0',
      // this prevents lightmap from being wrong when switching maps
      //   renderer doesn't restart between maps, but BSP loading updates
      //   textures with lightmap by default, so this keeps them separate
      '+set', 'r_mergeLightmaps', '0',
      '+set', 'r_deluxeMapping', '0',
      '+set', 'r_normalMapping', '0',
      '+set', 'r_specularMapping', '0',
      '+set', 'r_gamma', '1.1',
      '+set', 'r_picmip', '1',
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
          '+unbind', 'LEFTARROW',
          '+unbind', 'RIGHTARROW',
          '+unbind', 'A',
          '+unbind', 'D',
          '+bind', 'A', '"+moveleft"',
          '+bind', 'D', '"+moveright"',
          '+set', 'sensitivity', '5',
          '+set', 'cl_mouseAccel', '0.2',
          '+set', 'cl_mouseAccelStyle', '1',
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
        if(window.location.hostname.match(/lvl\.quake\.games/i)) {
          var detectMapId = (((/(&|\?|\/)id[:=]([0-9]+)($|[^0-9])/igm)
            .exec(window.location.search) || [])[2] || '')
          args.push.apply(args, [
            '+set', 'sv_dlURL', '"https://lvl.quake.games/assets"',
            '+set', 'net_socksEnabled', '0',
            '+set', 'net_enabled', '0',
            '+set', 'com_maxfps', '30',
            '+set', 'net_enable', '0',
            '+set', 'cg_drawGun', '0',
            '+set', 'cg_simpleItems', '0',
            '+set', 'cg_draw2D', '0',
            '+set', 'sensitivity', '3.5',
            '+set', 'sv_shareError', '1',
            '+set', 'sv_hostname', '"Planet Quake with lvlworld.com"',
            '+set', 'g_motd', '"The all around best game created thus far"',
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
              '+set', 'net_socksServer', (SYSM.isSecured(window.location.origin) ? 'wss://' : 'ws://') + window.location.hostname,
              '+set', 'net_socksPort', SYSM.isSecured(window.location.origin) ? '443' : window.location.port,
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
            '+set', 'net_socksServer', (SYSM.isSecured(window.location.origin) ? 'wss://' : 'ws://') + 'proxy.quake.games',
            '+set', 'net_socksPort', SYSM.isSecured(window.location.origin) ? '443' : window.location.port,
          ])
        }
        if (!args.includes('sv_dlURL')) {
          args.push.apply(args, [
            '+set', 'sv_dlURL', '"https://quake.games/assets"',
          ])
        }
      } else {
        if(window.location.hostname.match(/quake\.money/i)
          || window.location.hostname.match(/quakeiiiarena\.com/i)) {
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
          ])
        }
        if (!args.includes('net_socksPort')) {
          args.push.apply(args, [
            '+set', 'net_socksPort', SYSM.isSecured(window.location.origin) ? '443' : window.location.port
          ])
        }
        if (!args.includes('sv_dlURL')) {
          args.push.apply(args, [
            '+set', 'sv_dlURL', '"' + window.location.origin + '/assets"',
          ])
        }
      }
      if(!args.includes('sv_master23')) {
        args.push.apply(args, [
          '+set', 'sv_master23', '"' + window.location.origin + '"',
        ])
      }
      if(!args.includes('cl_master23')) {
        args.push.apply(args, [
          '+set', 'cl_master23', '"' + window.location.origin + '"',
        ])
      }
      if(!SYS.dedicated) {
        if(!args.includes('+connect')) {
          args.push.apply(args, [
            '+connect', 'localhost'
          ])
        }
      } else { // IS dedicated settings
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
          '+set', 'sv_hostname', '"http://quake.games"',
          '+set', 'sv_motd', 'For instant replays and stuff',
          '+set', 'rconPassword', 'password123!',
          '+set', 'sv_reconnectlimit', '0',
          //  '+set', 'sv_autoDemo', '1',
          //  '+set', 'sv_autoRecord', '1',
          //  '+set', 'net_socksEnabled', '0',
        ])
      }
      if(window.innerWidth / window.innerHeight < .8
        && !args.includes('cg_gunZ') && !args.includes('cg_gunX')) {
        args.push.apply(args, [
          '+set', 'cg_gunZ', '-5',
          '+set', 'cg_gunX', '-5',
        ])
      } else {
        args.push.apply(args, [
          '+set', 'cg_gunZ', '0',
          '+set', 'cg_gunX', '0',
        ])
      }
      return args
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
    SYSC.varStr = allocate(new Int8Array(4096), ALLOC_NORMAL)
    SYSC.newDLURL = SYSC.Cvar_VariableString('cl_dlURL')
    SYSC.oldDLURL = SYSC.Cvar_VariableString('sv_dlURL')
    Object.assign(Module, {
      websocket: Object.assign(Module.websocket || {}, {
        url: SYSM.isSecured(SYSC.Cvar_VariableString('net_socksServer'))
        ? 'wss://'
        : 'ws://'
      })
    })
    SYSN.lazyInterval = setInterval(SYSN.DownloadLazy, 10)

    if(SYS.dedicated) return

    if(false && 'Worker' in window) {
      window.serverWorker = new Worker('server-worker.js')
      window.serverWorker.onmessage = SYS.onWorkerMessage
      window.serverWorker.postMessage(['init', SYSM.getQueryCommands()])
    }
    
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
    SYSI.cancelBackspace = false
    SYSM.exited = true
    if(!SYS.dedicated) {
      window.removeEventListener('resize', SYSI.resizeViewport)
      window.removeEventListener('keydown', SYSI.InputPushKeyEvent)
      window.removeEventListener('keyup', SYSI.InputPushKeyEvent)
      window.removeEventListener('keypress', SYSI.InputPushTextEvent)
      document.removeEventListener('mousewheel', SYSI.InputPushWheelEvent)
      document.removeEventListener('visibilitychange', SYSI.InputPushFocusEvent)
      document.removeEventListener('drop', SYSI.dropHandler)
      document.removeEventListener('dragenter', SYSI.dragEnterHandler)
      document.removeEventListener('dragover', SYSI.dragOverHandler)    
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
		return allocate(intArrayFromString('player'), ALLOC_STACK)
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
      .map(function (a, i) { return i == 0
        ? UTF8ToString(a)
        // TODO: fix this for numbers and strings, 
        //   return the correct type based on the position of variable
        : UTF8ToString(HEAP32[a>>2]) })
    SYSC.Print(args)
    if(SYS.dedicated) {
      window.serverWorker.postMessage(['status', args])
    }
  },
  Sys_CmdArgs: function () {
    var argv = ['ioq3'].concat(SYSM.getQueryCommands())
    var argc = argv.length
    // merge default args with query string args
    var size = (argc + 1) * {{{ Runtime.POINTER_SIZE }}}
    var list = allocate(new Int8Array(size), ALLOC_NORMAL)
    for (var i = 0; i < argv.length; i++) {
      HEAP32[(list >> 2) + i] = allocateUTF8OnStack(argv[i])
    }
    HEAP32[(list >> 2) + argc] = 0
    return list
  },
  Sys_CmdArgsC: function () {
    return SYSM.getQueryCommands().length + 1
  },
  Sys_DownloadLocalFile: function (fileName) {
    // used to download screenshots to the computer after they are taken
    fileName = PATH.join(SYSF.fs_basepath, SYSF.fs_game, UTF8ToString(fileName))
    let file = new File([FS.readFile(fileName)],
      fileName, {
      type: 'image/' + PATH.extname(fileName).substr(1)
    });
    let exportUrl = URL.createObjectURL(file);
    var popout = window.open(exportUrl, '_blank', 'location=yes,height=570,width=520,scrollbars=yes,status=yes');
    URL.revokeObjectURL(exportUrl);
    setTimeout(function () {
      if(popout) popout.close()
    }, 1000)
  }
}
autoAddDeps(LibrarySysMain, '$SYSM')
mergeInto(LibraryManager.library, LibrarySysMain);
