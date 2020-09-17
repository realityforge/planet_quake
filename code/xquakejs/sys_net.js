var LibrarySysNet = {
  $SYSN__deps: ['$SOCKFS'],
  $SYSN: {
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
      if(typeof document == 'undefined') {
        console.log(desc)
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
      if(typeof document == 'undefined') {
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
			SYSF.index[indexFilename].downloading = false
      SYSF.index[indexFilename].alreadyDownloaded = true
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
				//SYSN.DownloadLazySort()
				SYSN.downloadSort = _Sys_Milliseconds()
			}
			var file = SYSN.downloadLazy.pop()
			if(!file) return
			if(typeof file == 'string') {
				file = [0, file]
			}
			var indexFilename = PATH.join(SYSF.fs_basepath, file[1]).toLowerCase()
			SYSC.mkdirp(PATH.join(SYSF.fs_basepath, PATH.dirname(file[1])))
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
			SYSC.DownloadAsset(file[1], function () {}, function (err, data) {
				if(err) {
					return
				}
        try {
          FS.writeFile(PATH.join(SYSF.fs_basepath, file[1]), new Uint8Array(data), {
  					encoding: 'binary', flags: 'w', canOwn: true })
        } catch (e) {
          if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
            SYSC.Error('fatal', e.message)
          }
        }
				SYSN.DownloadLazyFinish(indexFilename, file)
			})
		},
		DownloadIndex: function (index, cb) {
      var filename = index.includes('.json') ? index : index + '/index.json'
      var basename = PATH.dirname(filename)
			SYSC.DownloadAsset(filename, SYSN.LoadingProgress, function (err, data) {
				if(err) {
					SYSN.LoadingDescription('')
					cb()
					return
				}
				var moreIndex = (JSON.parse((new TextDecoder("utf-8")).decode(data)) || [])
				SYSF.index = Object.keys(moreIndex).reduce(function (obj, k) {
          if(typeof obj[k.toLowerCase()] == 'undefined') {
            obj[k.toLowerCase()] = moreIndex[k]
            // we use the downloading key because it doesn't
            //   come with the index.json from the server
            //   so we only recreate local paths once because it is saved below
            //   then it can be used by the engine to check for remote resources
            //   like FS_MapInIndex() or demos and cinematics files
            if(typeof obj[k.toLowerCase()].downloading == 'undefined') {
              obj[k.toLowerCase()].name = PATH.join(basename, moreIndex[k].name)
              obj[k.toLowerCase()].shaders = []
              obj[k.toLowerCase()].downloading = false
            }
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
          size: bits.length
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
    var fs_basepath = SYSC.Cvar_VariableString('fs_basepath')
    
    FS.syncfs(false, function (e) {
      if(e) console.log(e)
      SYSC.mkdirp(PATH.join(fs_basepath, PATH.dirname(cl_downloadName)))
      SYSC.DownloadAsset(cl_downloadName, function (loaded, total) {
        SYSC.Cvar_SetValue('cl_downloadSize', total);
        SYSC.Cvar_SetValue('cl_downloadCount', loaded);
      }, function (err, data) {
        if(err) {
          SYSC.Error('drop', 'Download Error: ' + err.message)
        } else {
          //FS.writeFile(PATH.join(fs_basepath, cl_downloadName), new Uint8Array(data), {
          //  encoding: 'binary', flags: 'w', canOwn: true })
        }
        FS.syncfs(false, Browser.safeCallback(_CL_NextDownload))
      })
    })
  },
  Sys_SocksConnect__deps: ['$Browser', '$SOCKFS'],
  Sys_SocksConnect: function () {
    var timer = setTimeout(Browser.safeCallback(_SOCKS_Frame_Proxy), 10000)
    var callback = function () {
      clearTimeout(timer)
      Browser.safeCallback(_SOCKS_Frame_Proxy)()
    }
    Module['websocket'].on('open', callback)
    Module['websocket'].on('message', callback)
    Module['websocket'].on('error', callback)
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
