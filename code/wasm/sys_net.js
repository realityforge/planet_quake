var global = window || global
global['SYSN'] = {
  downloadAlternates: [],
  downloadTries: [],
  downloadLazy: [],
  downloadCount: 0,
  downloads: [],
  downloadsCompleted: [],
  downloadsSyncing: false,
  downloadSort: 0,
  lazyIterval: 0,
  multicasting: false,
  multicastBuffer: 0,
}

function receiveNetLoop (net, data) {
  if(!SYSN.multicastBuffer) {
    SYSN.multicastBuffer = allocate(new Int8Array(4096), ALLOC_NORMAL)
  }
  SYSN.multicasting = true
  data.forEach(function (d, i) { HEAP8[SYSN.multicastBuffer+i] = d })
  _NET_SendLoopPacket(net, data.length, SYSN.multicastBuffer)
  SYSN.multicasting = false
}

function LoadingDescription (desc) {
  var args = Array.from(arguments)
  if(SYS.dedicated) {
    SYSC.desc = args
    Sys_Print(args)
    window.serverWorker.postMessage(['status', args])
    return
  }
  var flipper = document.getElementById('flipper')
  var progress = document.getElementById('loading-progress')
  var description = progress.querySelector('.description')
  if (!desc) {
    progress.style.display = 'none'
    flipper.style.display = 'none'
    SYS.previousProgress = [0, 0]
    SYSN.LoadingProgress(0)
  } else {
    progress.style.display = 'block'
    flipper.style.display = 'block'
  }
  
  var div = document.createElement('div')
  div.innerHTML = desc.replace('%s', args[1])
  if(description.children.length == 0
    || div.innerText.toLowerCase() != description.children[description.children.length-1].innerText.toLowerCase())
    description.appendChild(div)
}

function LoadingProgress (progress, total) {
  if(SYS.dedicated) {
    window.serverWorker.postMessage(['status', SYSC.desc, [progress, total]])
    return
  }
  if(!progress) progress = SYS.previousProgress[0]
  if(!total) total = SYS.previousProgress[1]
  SYS.previousProgress = [progress, total]
  var frac = SYS.serviceProgress 
    ? (progress + SYS.serviceProgress[0]) / (total + SYS.serviceProgress[0])
    : progress / total
  var progressBar = document.getElementById('loading-progress').querySelector('.bar')
  progressBar.style.width = (frac*100) + '%'
}

function DoXHR (url, opts) {
  var xhrError = null
  if (!url) {
    return opts.onload(new Error('Must provide a URL'))
  }
  /*
  return new Promise(resolve => {
    var request = new XMLHttpRequest();
    request.open('GET', url)
    request.responseType = 'blob'
    request.credentials = 'omit'
    request.mode = 'no-cors'

    request.onload = function() {
      request.response.status = request.status
      resolve(request.response)
    }
    
    request.onerror = function() {
      request.response.status = request.status
      resolve(request.error)
    }

    request.send()
  })
  */
  fetch(url, {
    mode: opts.mode || 'cors',
    responseType: 'arraybuffer',
    credentials: opts.credentials || 'omit'
  }).catch(function (e) { console.log(e) })
    .then(function (response) {
        if (!response || !(response.status >= 200 && response.status < 300 || response.status === 304)) {
          xhrError = new Error('Couldn\'t load ' + url + '. Status: ' + (response || {}).statusCode)
          return
        } else
          return response.arrayBuffer()
    })
    .then(function (data) {
      if(xhrError) {
        if (opts.onload) {
          opts.onload(xhrError, null)
        }
        return
      }
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
}

function DownloadLazyFinish (file) {
  var indexFilename = PATH.join(SYSF.fs_basepath, file[1]).toLowerCase()
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
}

function DownloadLazySort () {
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
}

function DownloadLazy () {
  // don't download lazy if we are downloading immediately already
  if(SYSN.downloads.length > 0) return
  // if we haven't sorted the list in a while, sort by number of references to file
  // TODO: uncomment this when syncfs is working
  if(Date.now() - SYSN.downloadSort > 1000
    && SYSN.downloadsCompleted.length > 0
    && !SYSN.downloadsSyncing) {
    SYSN.downloadsSyncing = true
    //SYSN.DownloadLazySort() // get stuck in a loop which might be fixed by .alreadyDownloaded
    SYSN.downloadSort = Date.now()
    FS.syncfs(true, function () {
      var complete
      while((complete = SYSN.downloadsCompleted.pop())) {
        SYSN.DownloadLazyFinish(complete)
      }
      SYSN.downloadsSyncing = false
    })
  }
  
  if(SYSN.downloadLazy.length == 0) return
  var file = SYSN.downloadLazy.pop()
  if(!file) return
  if(typeof file == 'string') {
    file = [0, file]
  }
  // if already exists somehow just call the finishing function
  try {
    var handle = FS.stat(PATH.join(SYSF.fs_basepath, file[1]))
    if(handle) {
      return SYSN.downloadsCompleted.push(file)
    }
  } catch (e) {
    if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
      Sys_Error('fatal', e.message)
    }
  }
  SYSF.downloadSingle(file[1], function () {
    SYSN.downloadsCompleted.push(file)
  })
}

function buildAlternateUrls (mod) {
  var origMod = mod.replace(/-cc*r*/ig, '')
  var hasOldURL = SYSC.oldDLURL.length > 0
  SYSN.downloadTries.push(SYSN.addProtocol(SYSC.newDLURL) + '/' + mod + '/')
  if(hasOldURL) {
    SYSN.downloadTries.push(SYSN.addProtocol(SYSC.oldDLURL) + '/' + mod + '/')
  }
  // all of these test links are in case someone fucks up conversion or startup
  //if(SYSF.mods.map(function (f) {return f[0]}).includes(origMod)) {
    SYSN.downloadTries.push(SYSN.addProtocol(SYSC.newDLURL) + '/' + origMod + '-cc/')
    if(hasOldURL)
      SYSN.downloadTries.push(SYSN.addProtocol(SYSC.oldDLURL) + '/' + origMod + '-cc/')
    SYSN.downloadTries.push(SYSN.addProtocol(SYSC.newDLURL) + '/' + origMod + '-ccr/')
    if(hasOldURL)
      SYSN.downloadTries.push(SYSN.addProtocol(SYSC.oldDLURL) + '/' + origMod + '-ccr/')
  //}
  SYSN.downloadTries = SYSN.downloadTries.filter(function (a, i, arr) {
    return arr.indexOf(a) === i
  })
}

function DownloadIndex (index, cb) {
  var filename = index.includes('.json') ? index : index + '/index.json'
  var basename = PATH.dirname(filename)
  var mod = index.replace(/^\//ig, '').split(/\//ig)[0]
  var origMod = mod.replace(/-cc?r?\//ig, '\/')
  SYSN.buildAlternateUrls(mod)
  SYSN.DownloadAsset(filename, SYSN.LoadingProgress, function (err, data, baseUrl) {
    if(err) {
      LoadingDescription('')
      cb()
      return
    }
    SYSN.downloadAlternates.push(baseUrl)
    var moreIndex = (JSON.parse((new TextDecoder("utf-8")).decode(data)) || [])
    SYSF.index = Object.keys(moreIndex).reduce(function (obj, k) {
      // TODO: make this more generalized and replace first directory after /base
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
      .map(function (k) {
        return '"' + k + '":' + JSON.stringify(SYSF.index[k].checksums ? {
          checksums: SYSF.index[k].checksums
        } : {})
      })
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
}

function addProtocol (url) { 
  return url.includes('://')
    ? url
    : window
      ? (window.location.protocol + '//' + url)
      : ('https://' + url)
}

function DownloadAsset (asset, onprogress, onload) {
  var segments = asset.replace(/^\//ig, '').split(/\//ig)
  var noMod = segments.length > 1 ? segments.slice(1).join('/') : segments[0]
  var tryDownload = 0
  SYSN.downloadTries.sort(function (a, b) { 
    return b.includes(segments[0]) - a.includes(segments[0])
  })
  var doDownload = function (baseUrl, opaque) {
    SYSN.DoXHR(baseUrl + noMod, {
      credentials: opaque ? 'omit' : 'same-origin',
      mode: opaque ? 'no-cors' : 'cors',
      dataType: 'arraybuffer',
      onprogress: onprogress,
function       onload (err, data) {
        if(err && !opaque) {
          doDownload(baseUrl, true)
          return
        }
        tryDownload++
        if(err && tryDownload < SYSN.downloadTries.length) {
          doDownload(SYSN.downloadTries[tryDownload], false)
          return
        }
        if(onload)
          onload(err, data, baseUrl)
      }
    })
  }
  if(segments.length > 1) {
    doDownload(SYSN.downloadTries[0], false)
  } else {
    doDownload('', false)
  }
}


function Sys_BeginDownload () {
  var cl_downloadName = Cvar_VariableString('cl_downloadName')
  LoadingDescription('')
  SYSN.downloads.push(cl_downloadName)
  SYSN.downloadTries = []
  var alts = SYSN.downloadAlternates
  var mod = cl_downloadName.replace(/^\//ig, '').split(/\//ig)[0]
  SYSN.buildAlternateUrls(mod)
  SYSN.buildAlternateUrls(SYSF.fs_basegame)
  if(SYSF.fs_game.length > 0)
    SYSN.buildAlternateUrls(SYSF.fs_game)
  FS.syncfs(!SYS.servicable, function (e) {
    if(e) console.log(e)
    SYSN.DownloadAsset(cl_downloadName, function (loaded, total) {
      SYSC.Cvar_SetValue('cl_downloadSize', total);
      SYSC.Cvar_SetValue('cl_downloadCount', loaded);
    }

function (err, data) {
      SYSN.downloads = []
      SYSN.downloadAlternates = SYSN.downloadTries = alts
      if(err) {
        SYSC.Cvar_SetString('fs_excludeReference', 
          Cvar_VariableString('fs_excludeReference')
          + cl_downloadName.replace(/\.pk3$/ig, ''))
        Sys_Print('Download Error: ' + err.message)
      } else {
        var newKey = PATH.join(SYSF.fs_basepath, cl_downloadName)
        // TODO: don't need to save here because service-worker fetch() already did
        //if(!SYS.servicable) {
          Sys_mkdirp(PATH.join(SYSF.fs_basepath, PATH.dirname(cl_downloadName)))
          FS.writeFile(PATH.join(SYSF.fs_basepath, cl_downloadName), new Uint8Array(data), {
            encoding: 'binary', flags: 'w', canOwn: true })
        //}
        // do need to add ad-hoc server downloads to the index
        SYSF.index[newKey.toLowerCase()] = {
          name: newKey.replace(SYSF.fs_basepath, ''),
          // would try to delete/origMod /baseq3/ but we just add it back on in DownloadIndex
          size: new Uint8Array(data).length,
          shaders: [],
          downloading: false,
          alreadyDownloaded: true,
        }
      }
      FS.syncfs(SYS.servicable, _CL_Outside_NextDownload)
    })
  })
}

function Sys_SocksConnect () {
  SYSN.socksfd = 0
  SYSN.socksConnect = setTimeout(function () {
    // broken
    SYSN.socksfd = 0
    clearInterval(SYSN.socksInterval)
    SYSN.socksInterval = 0
    _SOCKS_Frame_Proxy()
  }, 10000)
  var callback = function () {
    if(SYSN.socksConnect) {
      clearTimeout(SYSN.socksConnect)
      SYSN.socksConnect = 0
    }
    _SOCKS_Frame_Proxy()
  }
  if(!SYSN.socksInterval) {
    SYSN.socksInterval = setInterval(function () {
      if(!SYSN.peer || !SYSN.sock || !SYSN.socksfd) return
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
    if(SYSN.socksfd) {
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
    }
    callback()
  }
  Module['websocket'].on('open', socksOpen)
  Module['websocket'].on('message', callback)
  Module['websocket'].on('error', callback)
  //Module['websocket'].on('close', function () {
  //  SYSN.socksfd = 0
  //})
}

function Sys_SocksMessage () {}

function Sys_NET_MulticastLocal (net, length, data) {
  // prevent recursion because NET_SendLoopPacket will call here again
  if(SYSN.multicasting
    || typeof window.serverWorker == 'undefined') return
  SYSN.multicasting = true
  window.serverWorker.postMessage([
    'net',
    net,
    Uint8Array.from(HEAP8.slice(data, data+length))])
  SYSN.multicasting = false
}
