var LibrarySysNet = {
  $SYSN__deps: ['$SOCKFS'],
  $SYSN: {
    downloadAlternates: [],
    downloadTries: [],
    downloadLazy: [],
		downloadCount: 0,
		downloads: [],
		downloadSort: 0,
    lazyIterval: 0,
    multicasting: false,
    multicastBuffer: 0,
    receiveNetLoop: function (net, data) {
      if(!SYSN.multicastBuffer) {
        SYSN.multicastBuffer = allocate(new Int8Array(4096), 'i8', ALLOC_NORMAL)
      }
      SYSN.multicasting = true
      data.forEach(function (d, i) { HEAP8[SYSN.multicastBuffer+i] = d })
      _NET_SendLoopPacket(net, data.length, SYSN.multicastBuffer)
      SYSN.multicasting = false
    },
    LoadingDescription: function (desc) {
      var args = Array.from(arguments)
      if(SYS.dedicated) {
        SYSC.desc = args
        SYSC.Print(args)
        window.serverWorker.postMessage(['status', args])
        return
      }
			var flipper = document.getElementById('flipper')
			var progress = document.getElementById('loading-progress')
			var description = progress.querySelector('.description')
			if (!desc) {
				progress.style.display = 'none'
				flipper.style.display = 'none'
				SYSN.LoadingProgress(0)
			} else {
				progress.style.display = 'block'
				flipper.style.display = 'block'
			}
			description.innerHTML = desc
		},
		LoadingProgress: function (progress, total) {
      if(SYS.dedicated) {
        window.serverWorker.postMessage(['status', SYSC.desc, [progress, total]])
        return
      }
			var frac = progress / total
			var progress = document.getElementById('loading-progress')
			var bar = progress.querySelector('.bar')
			bar.style.width = (frac*100) + '%'
		},
    DoXHR: function (url, opts) {
      var xhrError = null
      if (!url) {
        return opts.onload(new Error('Must provide a URL'))
      }
      fetch(url, {credentials: 'omit'})
        .catch(function (e) { console.log(e) })
        .then(function (response) {
            if (!response || !(response.status >= 200 && response.status < 300 || response.status === 304)) {
              xhrError = new Error('Couldn\'t load ' + url + '. Status: ' + (response || {}).statusCode)
              if (opts.onload) {
                opts.onload(xhrError, null)
              }
              return
            }
            if(!xhrError)
              return response.arrayBuffer()
                .then(function (data) {
                  if (opts.dataType === 'json') {
                    try {
                      data = JSON.parse(data)
                    } catch (e) {
                      xhrError = e
                    }
                  }
                  if (opts.onload) {
                    opts.onload(xhrError, data)
                  }
                })
        })
    },
    DownloadLazyFinish: function (indexFilename, file) {
      SYSN.downloads = []
			SYSF.index[indexFilename].downloading = false
      SYSF.index[indexFilename].alreadyDownloaded += true
      var replaceFunc = function (path) {
        SYSF.fs_replace.forEach(function (r) { path = path.replace(r, '') })
        return path
      }
			if(file[1].match(/\.opus|\.wav|\.ogg/i)) {
				if(file[0]) {
					SYS.soundCallback.unshift(replaceFunc(file[0]))
				} else {
					SYS.soundCallback.unshift(replaceFunc(file[1]))
				}
				SYS.soundCallback = SYS.soundCallback.filter(function (s, i, arr) { return arr.indexOf(s) === i })
			} else if(file[1].match(/\.md3|\.iqm|\.mdr/i)) {
				if(file[0]) {
					SYS.modelCallback.unshift(replaceFunc(file[0]))
				} else {
					SYS.modelCallback.unshift(replaceFunc(file[1]))
				}
				SYS.modelCallback = SYS.modelCallback.filter(function (s, i, arr) { return arr.indexOf(s) === i })
			} else if(SYSF.index[indexFilename].shaders.length > 0) {
				if(file[0]) {
					SYS.shaderCallback.unshift.apply(SYS.shaderCallback, [replaceFunc(file[0])]
            .concat(SYSF.index[indexFilename].shaders))
				} else {
					SYS.shaderCallback.unshift.apply(SYS.shaderCallback, SYSF.index[indexFilename].shaders)
				}
				SYS.shaderCallback = SYS.shaderCallback.filter(function (s, i, arr) { return arr.indexOf(s) === i })
			}
		},
		DownloadLazySort: function () {
			SYSN.downloadLazy.sort(function (a, b) {
				var aIndex = typeof a == 'string'
					? PATH.join(SYSF.fs_basepath, a)
					: PATH.join(SYSF.fs_basepath, a[1])
				var aVal = typeof SYSF.index[aIndex.toLowerCase()] != 'undefined'
						? SYSF.index[aIndex.toLowerCase()].shaders.length + 1
						: 0
				var bIndex = typeof a == 'string'
					? PATH.join(SYSF.fs_basepath, b)
					: PATH.join(SYSF.fs_basepath, b[1])
				var bVal = typeof SYSF.index[bIndex.toLowerCase()] != 'undefined'
						? SYSF.index[bIndex.toLowerCase()].shaders.length + 1
						: 0
				return aVal - bVal
			})
		},
		DownloadLazy: function () {
			if(SYSN.downloadLazy.length == 0 || SYSN.downloads.length > 0) return
			// if we haven't sorted the list in a while, sort by number of references to file
			if(_Sys_Milliseconds() - SYSN.downloadSort > 1000) {
				//SYSN.DownloadLazySort() // get stuck in a loop which might be fixed by .alreadyDownloaded
				SYSN.downloadSort = _Sys_Milliseconds()
			}
			var file = SYSN.downloadLazy.pop()
			if(!file) return
			if(typeof file == 'string') {
				file = [0, file]
			}
			var indexFilename = PATH.join(SYSF.fs_basepath, file[1]).toLowerCase()
			// if already exists somehow just call the finishing function
			try {
				var handle = FS.stat(PATH.join(SYSF.fs_basepath, file[1]))
				if(handle) {
					return SYSN.DownloadLazyFinish(indexFilename, file)
				}
			} catch (e) {
				if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
					SYSC.Error('fatal', e.message)
				}
			}
      SYSN.downloads.push(file[1])
      SYSF.downloadImmediately(SYSN.DownloadLazyFinish.bind(null, indexFilename, file))
		},
    buildAlternateUrls: function (mod) {
      var origMod = mod.replace(/-cc*r*/ig, '')
      var hasOldURL = SYSC.oldDLURL.length > 0
      SYSN.downloadTries.push(SYSC.addProtocol(SYSC.newDLURL) + '/' + mod + '/')
      if(hasOldURL) {
        SYSN.downloadTries.push(SYSC.addProtocol(SYSC.oldDLURL) + '/' + mod + '/')
      }
      // all of these test links are in case someone fucks up conversion or startup
      if(SYSF.mods.map(function (f) {return f[0]}).includes(origMod)) {
        SYSN.downloadTries.push(SYSC.addProtocol(SYSC.newDLURL) + '/' + origMod + '-cc/')
        if(hasOldURL)
          SYSN.downloadTries.push(SYSC.addProtocol(SYSC.oldDLURL) + '/' + origMod + '-cc/')
        SYSN.downloadTries.push(SYSC.addProtocol(SYSC.newDLURL) + '/' + origMod + '-ccr/')
        if(hasOldURL)
          SYSN.downloadTries.push(SYSC.addProtocol(SYSC.oldDLURL) + '/' + origMod + '-ccr/')
      }
      SYSN.downloadTries = SYSN.downloadTries.filter(function (a, i, arr) {
        return arr.indexOf(a) === i
      })
    },
		DownloadIndex: function (index, cb) {
      var filename = index.includes('.json') ? index : index + '/index.json'
      var basename = PATH.dirname(filename)
      var mod = index.replace(/^\//ig, '').split(/\//ig)[0]
      var origMod = mod.replace(/-cc?r?\//ig, '\/')
      SYSN.buildAlternateUrls(mod)
			SYSC.DownloadAsset(filename, SYSN.LoadingProgress, function (err, data, baseUrl) {
				if(err) {
					SYSN.LoadingDescription('')
					cb()
					return
				}
        SYSN.downloadAlternates.push(baseUrl)
				var moreIndex = (JSON.parse((new TextDecoder("utf-8")).decode(data)) || [])
				SYSF.index = Object.keys(moreIndex).reduce(function (obj, k) {
          var newKey = k.toLowerCase().replace(new RegExp(origMod + '(-cc?r?)?\/', 'ig'), mod + '/')
          if(typeof obj[newKey] == 'undefined') {
            obj[newKey] = moreIndex[k]
            // we use the downloading key because it doesn't
            //   come with the index.json from the server
            //   so we only recreate local paths once because it is saved below
            //   then it can be used by the engine to check for remote resources
            //   like FS_MapInIndex() or demos and cinematics files
            if(typeof obj[newKey].downloading == 'undefined') {
              obj[newKey].name = PATH.join(basename, moreIndex[k].name)
              obj[newKey].shaders = []
              obj[newKey].downloading = false
            }
            obj[newKey].alreadyDownloaded = false
          }
					return obj
				}, SYSF.index || {})
				var bits = intArrayFromString('{' + Object.keys(SYSF.index)
					.map(function (k) { return '"' + k + '":' + JSON.stringify(SYSF.index[k]) })
          .join(',')
					+ '}')
        var gameIndex = PATH.join(SYSF.fs_basepath, basename, "index.json")
        SYSF.index[gameIndex.toLowerCase()] = {
          name: gameIndex,
          size: bits.length,
          shaders: [],
          downloading: false,
          alreadyDownloaded: true,
        }
				FS.writeFile(gameIndex,
				 	Uint8Array.from(bits.slice(0, bits.length-1)),
					{encoding: 'binary', flags: 'w', canOwn: true })
				cb()
			})
		},
  },
  Sys_BeginDownload__deps: ['$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
  Sys_BeginDownload: function () {
    var cl_downloadName = SYSC.Cvar_VariableString('cl_downloadName')
    SYSN.LoadingDescription('')
    SYSN.downloads.push(cl_downloadName)
    SYSN.downloadTries = []
    var alts = SYSN.downloadAlternates
    var mod = cl_downloadName.replace(/^\//ig, '').split(/\//ig)[0]
    SYSN.buildAlternateUrls(mod)
    SYSN.buildAlternateUrls(SYSF.fs_basegame)
    if(SYSF.fs_game.length > 0)
      SYSN.buildAlternateUrls(SYSF.fs_game)
    FS.syncfs(false, function (e) {
      if(e) console.log(e)
      SYSC.DownloadAsset(cl_downloadName, function (loaded, total) {
        SYSC.Cvar_SetValue('cl_downloadSize', total);
        SYSC.Cvar_SetValue('cl_downloadCount', loaded);
      }, function (err, data) {
        SYSN.downloads = []
        SYSN.downloadAlternates = SYSN.downloadTries = alts
        if(err) {
          SYSC.Error('drop', 'Download Error: ' + err.message)
        } else {
          var newKey = PATH.join(SYSF.fs_basepath, cl_downloadName)
          // TODO: don't need to save here because service-worker fetch() already did
          if(!SYS.servicable) {
            SYSC.mkdirp(PATH.join(SYSF.fs_basepath, PATH.dirname(cl_downloadName)))
            FS.writeFile(PATH.join(SYSF.fs_basepath, cl_downloadName), new Uint8Array(data), {
              encoding: 'binary', flags: 'w', canOwn: true })
          }
          // do need to add ad-hoc server downloads to the index
          SYSF.index[newKey.toLowerCase()] = {
            name: newKey.replace(SYSF.fs_basepath, ''), // would try to delete/origMod /baseq3/ but we just add it back on in DownloadIndex
            size: new Uint8Array(data).length,
            shaders: [],
            downloading: false,
            alreadyDownloaded: true,
          }
        }
        FS.syncfs(false, Browser.safeCallback(_CL_Outside_NextDownload))
      })
    })
  },
  Sys_SocksConnect__deps: ['$Browser', '$SOCKFS'],
  Sys_SocksConnect: function () {
    SYSN.socksfd = 0
    SYSN.socksConnect = setTimeout(function () {
      // broken
      SYSN.socksfd = 0
      clearInterval(SYSN.socksInterval)
      SYSN.socksInterval = 0
      Browser.safeCallback(_SOCKS_Frame_Proxy)()
    }, 10000)
    var callback = function () {
      if(SYSN.socksConnect) {
        clearTimeout(SYSN.socksConnect)
        SYSN.socksConnect = 0
      }
      Browser.safeCallback(_SOCKS_Frame_Proxy)()
    }
    if(!SYSN.socksInterval) {
      SYSN.socksInterval = setInterval(function () {
        if(!SYSN.peer) return
        if(SYSN.peer.socket.readyState == SYSN.peer.socket.OPEN) {
          SYSN.peer.socket.send(Uint8Array.from([0x05, 0x01, 0x00, 0x00]),
            { binary: true })
        } else if(SYSN.peer.socket.readyState == SYSN.peer.socket.CLOSED) {
          var newSocket = new WebSocket(SYSN.peer.socket.url)
          newSocket.binaryType = 'arraybuffer';
          SYSN.peer.socket = newSocket
          SOCKFS.websocket_sock_ops.handlePeerEvents(SYSN.sock, SYSN.peer)
          SYSN.reconnect = true
        }
      }, 1000)
    }
    var socksOpen = function (id) {
      SYSN.socksfd = id
      SYSN.sock = SOCKFS.getSocket(SYSN.socksfd)
      SYSN.peer = Object.values(SYSN.sock.peers)[0]
      SYSN.port = SYSC.Cvar_VariableIntegerValue('net_port')
      if(SYSN.reconnect) {
        SYSN.reconnect = false
        SYSN.peer.socket.send(Uint8Array.from([
          0xFF, 0xFF, 0xFF, 0xFF,
          'p'.charCodeAt(0), 'o'.charCodeAt(0), 'r'.charCodeAt(0), 't'.charCodeAt(0),
          (SYSN.port & 0xFF00) >> 8, (SYSN.port & 0xFF)
        ]))
      }
      callback()
    }
    Module['websocket'].on('open', socksOpen)
    Module['websocket'].on('message', callback)
    Module['websocket'].on('error', callback)
    //Module['websocket'].on('close', function () {
    //  SYSN.socksfd = 0
    //})
  },
  Sys_SocksMessage__deps: ['$Browser', '$SOCKFS'],
  Sys_SocksMessage: function () {},
  Sys_NET_MulticastLocal: function (net, length, data) {
    // prevent recursion because NET_SendLoopPacket will call here again
    if(SYSN.multicasting) return
    SYSN.multicasting = true
    window.serverWorker.postMessage([
      'net',
      net,
      Uint8Array.from(HEAP8.slice(data, data+length))])
    SYSN.multicasting = false
  }
}
autoAddDeps(LibrarySysNet, '$SYSN')
mergeInto(LibraryManager.library, LibrarySysNet);
