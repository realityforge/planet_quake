var global = window || global
global['SYSM'] = {
  index: {},
  fs_replace: [],
  fs_homepath: '/base/home',
  fs_basepath: '/base',
  fs_basegame: 'baseq3',
  fs_game: 'baseq3',
  pathname: 0,
  modeStr: 0,
  forceCDNUpdate: false,
  forceAssetUpdate: false,
  precacheConfig: [
    'index.html',
    'nipplejs.js',
    'quakejs-border.png',
    'quakejs-noborder-transparent.png',
    'quake3e.js',
    'quake3e.wasm',
    'quake3e_opengl2_js.wasm',
    'quake3e_libbots_js.wasm',
    'assets/baseq3-cc/index.json',
    'server-worker.js',
  ],
  mods: [
    // A list of supported mods, 'dirname-cc' (-ccr means combined converted repacked)
    //   To the right is the description text, atomatically creates a placeholder.pk3dir with description.txt inside
    // We use a list here because Object keys have no guarantee of order
    ['baseq3', 			 'Quake III Arena', '(players|player)\/(sarge|major)'],
    ['missionpack',  '0 Choice: Team Arena', '(players|player)\/(sarge|james)'],
    ['defrag',       '1 Choice: Defrag'],
    ['baseq3r',      '2 Choice: Q3Rally', '(players|player)\/(sidepipe)'],
    ['basemod', 		 '3 Choice: Monkeys of Doom'],
    ['generations',  '4 Choice: Generations Arena'],
    ['q3f2', 				 '5 Choice: Q3 Fortress 2'],
    ['cpma', 				 '6 Choice: Challenge ProMode', '(players|player)\/(sarge|mynx)'],
    ['q3ut4', 		 	 '7 Choice: Urban Terror 4', '(players|player)\/(athena|orion)'],
    ['freezetag', 	 '8 Choice: Freeze Tag'],
    ['corkscrew', 	 '9 Choice: Corkscrew'],
    ['freon', 			 'Excessive Plus: Freon'],
    ['baseoa', 			 'Open Arena', '(players|player)\/(sarge|gi)'],
    ['bfpq3', 			 'Bid For Power'],
    ['excessive', 	 'Excessive+'],
    ['q3ut3', 			 'Urban Terror 3'],
    ['edawn', 			 'eDawn'],
    ['geoball', 		 'Geoball'],
    ['neverball', 	 'Neverball'],
    ['omissionpack', 'OpenArena Mission Pack'],
    ['platformer', 	 'Platformer'],
    ['legoc', 			 'Lego Carnage'],
    ['osp', 				 'Orange Smoothie Productions'],
    ['quake2arena',  'Quake 2 Arena'],
    ['smokin', 			 'Smokin\' Guns'],
    ['wfa', 				 'Weapons Factory Arena'],
    ['uberarena', 	 'Uber Arena'],
    ['demoq3', 			 'Quake III Demo'],
    ['mfdata', 			 'Military Forces'],
    ['conjunction',  'Dark Conjunction'],
    ['chili', 			 'Chili Quake XXL'],
    ['hqq', 				 'High Quality Quake'],
    ['entityplus', 	 'Engine Of Creation: Entity Plus'],
    ['wop',          'World of Padman'],
    ['truecombat',   'True Combat 1.3', '(tc_players|player)\/(sarge|alpha)'],
    ['rocketarena',  'Coming Soon: Rocket Arena'],
    ['gpp',          'Coming Soon: Tremulous'],
    ['gppl',         'Coming Soon: Unvanquished'],
    ['iortcw',       'Coming Soon: Return to Castle Wolfenstien'],
    ['baset',        'Coming Soom: Wolfenstien: Enemy Territory'],
    ['openjk',       'Coming Soon: Jedi Knights: Jedi Academy'],
    ['baseef',       'Coming Soon: Star Trek: Elite Force'],
  ],
}

function filterDownloads (mapname, modelname) {
  // create virtual file entries for everything in the directory list
  var keys = Object.keys(SYSF.index)
  var mapMatch = new RegExp(
    '\/levelshots\/' + mapname
    + '|\/' + mapname + '\.bsp'
    + '|\/' + mapname + '\.aas', 'i')
  var playerMatch = new RegExp('sarge\/'
    + '|' + modelname + '\/', 'i')
  var mod = SYSF.mods.filter(function (mod) {
    return mod[0] == SYSF.fs_game.replace(/-cc*r*$/ig, '')
  })[0]
  var defaultModel = new RegExp((mod || [])[2] || '(players|player)\/(sarge|major)', 'i')
  
  // servers need some map and model info for hitboxes up front
  for(var i = 0; i < keys.length; i++) {
    var file = SYSF.index[keys[i]]
    if(typeof file.size == 'undefined') { // skip directories, they are created during fetch
      continue
    }
  
    // TODO: remove this check when webworker is installed
    //   because it will check ETag and replace files
    // only download again if the file does not exist
    try {
      var handle = FS.stat(PATH.join(SYSF.fs_basepath, file.name))
      if(handle) {
        continue
      }
    } catch (e) {
      if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
        Sys_Error('fatal', e.message)
      }
    }

    // download files required to start
    if(file.name.match(/description\.txt|\.pk3$|\.wasm|\.qvm|\.cfg|\.arena|\.shader|\.font/i)
      // download files for menu system
      || file.name.match(/\.menu|menus\.txt|ingame\.txt|hud[0-9]*\.txt|arenas\.txt/i)
      || file.name.match(/ui\/.*\.h|\.crosshair|logo512|banner5|\/hit\.|\/2d\//i)
      // download required model and bot
      || file.name.match(/\.ini|\.hit|botfiles|\.bot|bots\.txt|gfx\//i)
      // download the current map if it is referred to
      || (modelname.length > 0 && file.name.match(playerMatch))
      || file.name.match(defaultModel)
      || (mapname.length > 0 && file.name.match(mapMatch))) {
      SYSF.index[keys[i]].downloading = true
      SYSN.downloads.push(file.name)
    } else if (
      // these files can be streamed in
      // download levelshots and common graphics
      file.name.match(/levelshots|(^|\/)ui\/|common\/|icons\/|menu\/|sfx\//i)
      // stream player icons so they show up in menu
      || file.name.match(/\/icon_|\.skin/i)
    ) {
      // TODO: reenable this when engine is stable, stream extra content
      // don't mark as downloading so if its required it can be upgraded to immediate
      //SYSN.downloadLazy.push(file.name)
    } else {
    }
  }
}

function downloadsDone () {
  // save to drive
  return FS.syncfs(false, function (e) {
    if(e) console.log(e)
    SYSN.downloads = []
    LoadingDescription('')
    if(SYSF.forceCDNUpdate) {
      window.location = window.location.toString()
    } else {
      ProxyCallback()
    }
  })
}

function downloadSingle (file, resolve) {
  SYSN.DownloadAsset(file, null, function (err, data) {
    if(err) return resolve()
    try {
      //if(!SYS.servicable) {
        Sys_mkdirp(PATH.join(SYSF.fs_basepath, PATH.dirname(file)))
        FS.writeFile(PATH.join(SYSF.fs_basepath, file.replace(/\?.*$/, '')), new Uint8Array(data), {
          encoding: 'binary', flags: 'w', canOwn: true })
      //}
    } catch (e) {
      if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
        Sys_Error('fatal', e.message)
      }
    }
    resolve(file, data)
  })
}

function checkForUpdates (cb) {
  var checkVersion = '/version.json?' + Math.floor(Date.now() / 1000 / 60) * 1000 * 60
  return new Promise(function (resolve) {
    downloadSingle(checkVersion, function (file, data) {
      if(!file) return resolve()
      try {
        var updatedVersion = JSON.parse(Array.from(new Uint8Array(data))
          .map(function (c) {return String.fromCharCode(c)}).join(''))
        var updatedDate = Date.parse(updatedVersion[0])
        var updatedAssets = Date.parse(updatedVersion[1])
        var compareCache = FS.stat(PATH.join(SYSF.fs_basepath, SYSF.precacheConfig[0]))
        if(updatedDate > compareCache.mtime.getTime()) {
          SYSF.forceCDNUpdate = updatedDate
        }
      } catch (e) {
        if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
          throw e
        } else {
          SYSF.forceCDNUpdate = updatedDate
        }
      }
      if(SYSF.forceCDNUpdate) {
        console.log('Forcing update ' + SYSF.forceCDNUpdate)
        LoadingDescription('Forcing update... Reloading.')
        SYSN.downloads.unshift.apply(SYSN.downloads, SYSF.precacheConfig
          .map(function (f) { return f + '?' + SYSF.forceCDNUpdate}))
      }
      resolve()
    })
  })
  .then(function () {
    buildAlternateUrls(SYSF.fs_game)
    var checkVersion = '/' + SYSF.fs_game + '/version.json?' + Math.floor(Date.now() / 1000 / 60) * 1000 * 60
    var compareAssets
    try {
      compareAssets = FS.stat(PATH.join(SYSF.basepath, SYSF.fs_game, 'version.json'))
    } catch (e) {
      if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
        throw e
      }
    }
    return new Promise(function (resolve) {
      SYSF.downloadSingle(checkVersion, function (file, data) {
        if(!file) return resolve()
        var updatedVersion = JSON.parse(Array.from(new Uint8Array(data))
          .map(function (c) {return String.fromCharCode(c)}).join(''))
        var updatedAssets = Date.parse(updatedVersion[1])
        //if(!compareAssets || updatedAssets > compareAssets.mtime.getTime()) {
          SYSF.forceAssetUpdate = updatedAssets
        //}
        resolve()
      })
    })
  })
  .catch(function (e) { console.error(e) })
  .then(cb)
}

function downloadImmediately (cb) {
  var total = 0
  if(SYSN.downloads.length === 0) {
    cb()
    return
  }
  var total = SYSN.downloads.length
  var count = 0
  var chunks = []
  var doUntilEmpty
  doUntilEmpty = function (resolve) {
    var file = SYSN.downloads.pop()
    if(file) {
      downloadSingle(file, function (file, data) {
        LoadingProgress(++count, total)
        doUntilEmpty(resolve)
      })
    } else {
      resolve()
    }
  }
  for(var c = 0; c < 10; c++) {
    chunks[c] = new Promise(doUntilEmpty)
  }
  return Promise.all(chunks).then(cb)
}

function Sys_FS_Startup () {
  var callback = SYSF.downloadImmediately.bind(null, SYSF.downloadsDone)
  SYSF.fs_replace = []
  SYSF.fs_replace.push(new RegExp('\/\/', 'ig'))
  SYSF.cl_lazyLoad = Cvar_Get('cl_lazyLoad')
  SYSC.newDLURL = Cvar_VariableString('cl_dlURL')
  SYSC.oldDLURL = Cvar_VariableString('sv_dlURL')

  var server = Cvar_VariableString('cl_currentServerAddress')
  if(server.length)
    history.pushState({location: window.location.toString()}, window.title, '?connect ' + server)

  // doesn't work with cors anyways, need to download over SOCKS proxy
  /*
  var cdn = Cvar_VariableString('fs_cdn')
  if(cdn.length > 0 && SYSC.oldDLURL == SYSC.newDLURL) {
    SYSC.newDLURL = cdn
  }
  */
  //SYSN.downloadLazy.splice(0) // reset lazy list to start of map
  SYSF.pathname = allocate(new Int8Array(4096), ALLOC_NORMAL)
  SYSF.modeStr = allocate(new Int8Array(4), ALLOC_NORMAL)
  SYSF.fs_homepath = Cvar_VariableString('fs_homepath')
  SYSF.fs_basepath = Cvar_VariableString('fs_basepath')
  SYSF.fs_basegame = Cvar_VariableString('fs_basegame')
  if(SYSF.fs_basegame.length > 0)
    SYSF.fs_replace.push(new RegExp('\/*' + SYSF.fs_basegame + '\/', 'ig'))
  var sv_pure = Cvar_VariableString('sv_pure')
  SYSF.fs_game = Cvar_VariableString('fs_game')
  if(SYSF.fs_game.length > 0)
    SYSF.fs_replace.push(new RegExp('\/*' + SYSF.fs_game + '\/', 'ig'))
  SYSF.fs_replace.sort(function (a, b) { return b.source.length - a.source.length })
  var mapname = Cvar_VariableString('mapname')
  var modelname = Cvar_VariableString('model')
  var playername = Cvar_VariableString('name')
  var blankFile = new Uint8Array(4)
  
  LoadingDescription('Loading Game UI...')
  if(!SYSF.fs_game || SYSF.fs_game.localeCompare(SYSF.fs_basegame) === 0) {
    SYSF.fs_game = SYSF.fs_basegame // TODO: comment this out to test server induced downloading
  }

  // mount a persistable filesystem into base
  Sys_mkdirp(SYSF.fs_basepath)

  try {
    FS.mount(IDBFS, {}, SYSF.fs_basepath)
  } catch (e) {
    if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EBUSY) {
      Sys_Error('fatal', e.message)
    }
  }

  var start = Date.now()
  // read from drive
  FS.syncfs(true, function (err) {
    if (err) {
      console.error(err)
    }

    Sys_Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds')

    // add the current fs_game to the mods list so it shows up on the mods menu
    if(!SYSF.mods.map(function (mod) {
      return mod[0]
    }).includes(SYSF.fs_game)) {
      SYSF.mods.push([SYSF.fs_game, SYSF.fs_game])
    }

    for(var i = 0; i < (SYSF.mods || []).length; i++) {
      var desc = PATH.join(SYSF.fs_basepath, SYSF.mods[i][0], 'description.txt')
      var prettyDesc = Uint8Array.from(intArrayFromString(SYSF.mods[i][1]).slice(0, SYSF.mods[i][1].length))
      var origMod = PATH.join(SYSF.fs_basepath, SYSF.mods[i][0].replace(/-cc*r*$/ig, ''))
      Sys_mkdirp(origMod)
      var symLinks = [
        PATH.dirname(desc),
        origMod + '-cc',
        origMod + '-ccr',
      ]
      symLinks.forEach(function (link) {
        try {
          FS.symlink(origMod, link)
        } catch (e) {
          if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
            throw e
          }
        }
      })
      // mods must have at least one pk3 to be considered valid, since we download individual files there likely isn't one
      var markerPk3Dir = PATH.join(PATH.dirname(desc), '0000placeholder.pk3dir')
      Sys_mkdirp(markerPk3Dir)
      FS.writeFile(desc, prettyDesc, {encoding: 'binary', flags: 'w', canOwn: true })
      SYSF.index[PATH.dirname(desc).toLowerCase() + '/'] = {
        name: PATH.dirname(desc.replace(SYSF.fs_basepath, '')),
      }
      SYSF.index[markerPk3Dir.toLowerCase() + '/'] = {
        name: markerPk3Dir.replace(SYSF.fs_basepath, ''),
      }
      SYSF.index[desc.toLowerCase()] = {
        name: desc.replace(SYSF.fs_basepath, ''),
        size: prettyDesc.length
      }
    }

    Sys_mkdirp(PATH.join(SYSF.fs_basepath, SYSF.fs_basegame))
    if(SYSF.fs_game != SYSF.fs_basegame) {
      Sys_mkdirp(PATH.join(SYSF.fs_basepath, SYSF.fs_game))
    }

    SYSN.downloads = []
    var indexes = [
      SYSF.fs_basegame + '/index.json'
    ]
    if(SYSF.fs_game != SYSF.fs_basegame) {
      indexes.push(SYSF.fs_game + '/index.json')
    }
    if(mapname.length > 0) {
      indexes.push(SYSF.fs_game + '/index-' + mapname.toLowerCase() + '.json')
    }
    if(playername.length > 0) {
      indexes.push(SYSF.fs_game + '/index-' + playername.toLowerCase() + '.json')
    }
    var current = 0
    var download;
    SYSN.downloadAlternates = []
    download = function () {
      if(current < indexes.length) {
        SYSN.downloadTries = []
        DownloadIndex(indexes[current] + (SYSF.forceAssetUpdate 
          ? ('?' + SYSF.forceAssetUpdate)
          : ''), download)
        current++
      } else {
        SYSN.downloadTries = SYSN.downloadAlternates
        filterDownloads(mapname, modelname)
        callback()
      }
    }
    checkForUpdates(download)
  })
}

function Sys_FOpen (ospath, mode) {
  var whitelist = new RegExp('demos\/|\.svdm_|screenshots\/|default.cfg|qkey|q3key|q3history|q3console.log|q3config.cfg|\.pk3$', 'gi')
  var handle = 0
  var exists = false
  try {
    intArrayFromString(UTF8ToString(mode)
      .replace('b', '')).forEach(function (c, i) { HEAP8[(SYSF.modeStr+i)] = c })
    HEAP8[(SYSF.modeStr+2)] = 0
    var filename = UTF8ToString(ospath).replace(/\/\//ig, '/')
    // use the index to make a case insensitive lookup
    var indexFilename = filename.toLowerCase()
    if(SYSF.index && typeof SYSF.index[indexFilename] != 'undefined') {
      var altName = filename.substr(0, filename.length
        - SYSF.index[indexFilename].name.length)
        + SYSF.index[indexFilename].name
      try { exists = FS.lookupPath(altName) } catch (e) { exists = false }
      if(handle === 0 && exists) {
        intArrayFromString(altName).forEach(function (c, i) { HEAP8[(SYSF.pathname+i)] = c })
        HEAP8[(SYSF.pathname+altName.length)] = 0
        handle = _fopen(SYSF.pathname, SYSF.modeStr)
      }
      var loading = Cvar_VariableString('r_loadingShader')
      if(loading.length === 0) {
        loading = Cvar_VariableString('snd_loadingSound')
        if(loading.length === 0) {
          loading = Cvar_VariableString('r_loadingModel')
        }
      } else if (!SYSF.index[indexFilename].shaders.includes(loading)) {
        SYSF.index[indexFilename].shaders.push(loading)
      }

      if((handle === 0
        || HEAP8[SYSF.cl_lazyLoad+8*4] === 2)
        && !SYSF.index[indexFilename].downloading) {
        if(SYSF.index[indexFilename].alreadyDownloaded < 3) {
          if(SYSF.index[indexFilename].alreadyDownloaded)
            SYSN.downloadLazy.unshift([loading, SYSF.index[indexFilename].name])
          else
            SYSN.downloadLazy.push([loading, SYSF.index[indexFilename].name])
        }
        SYSF.index[indexFilename].downloading = true
      }
    } else if (indexFilename.match(whitelist)) {
      intArrayFromString(filename).forEach(function (c, i) { HEAP8[(SYSF.pathname+i)] = c })
      HEAP8[(SYSF.pathname+filename.length)] = 0
      handle = _fopen(SYSF.pathname, SYSF.modeStr)
    }
  } catch (e) {
    // short for fstat check in sys_unix.c!!!
    if(e.code == 'ENOENT') {
      return 0
    }
    throw e
  }
  return handle
},
#if ASSERTIONS

#else

#endif
function Sys_ListFiles (directory, ext, filter, numfiles, dironly) {
  directory = UTF8ToString(directory);
  ext = UTF8ToString(ext);
  if (ext === '/') {
    ext = null;
    dironly = true;
  }

  // TODO support filter
  
  var contents;
  try {
    contents = Object.keys(SYSF.index)
      .filter(function (k) { 
        return k.match(new RegExp(directory + '\\/[^\\/]+\\/?$', 'i'))
          && (!dironly || typeof SYSF.index[k].size == 'undefined') 
      }).map(function (k) { return PATH.basename(SYSF.index[k].name) })
      .filter(function (f, i, arr) { return f && arr.indexOf(f) === i })
      .filter(function (f) {
        try {
          var stat = FS.stat(PATH.join(directory, f))
          return stat && (!dironly || FS.isDir(stat.mode))
        } catch (e) {
          if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
            throw e
          }
        }
      })
      .concat(FS.readdir(directory).filter(function (f) {
        // allow additional downloaded pk3s NOT in index to processed
        return f.match(/\.pk3$/ig)
      }))
    if(directory.match(/\/demos|\/svdemos/i)) {
      contents = contents
        .concat(FS.readdir(directory))
        .filter(function (f) { 
          return !dironly || FS.isDir(FS.stat(PATH.join(directory, f)).mode)
        })
    }
    if(contents.length > 5000) {
      debugger
      return null
    }
  } catch (e) {
    HEAP32[(numfiles+0)>>2] = 0;
    return null
  }

  var matches = [];
  for (var i = 0; i < contents.length; i++) {
    var name = contents[i].toLowerCase();
    if (!ext || name.lastIndexOf(ext) === (name.length - ext.length)
      || (ext.match(/tga/i) && name.lastIndexOf('png') === (name.length - ext.length))
      || (ext.match(/tga/i) && name.lastIndexOf('jpg') === (name.length - ext.length))
      || (ext.match(/dm_68/i) && name.lastIndexOf('svdm_68') === (name.length - 7))
      || (ext.match(/dm_68/i) && name.lastIndexOf('dm_71') === (name.length - 5))
    ) {
      matches.push(contents[i])
    }
  }

  HEAP32[numfiles >> 2] = 0;
  HEAP32[numfiles >> 2] = matches.length;

  HEAP32[(numfiles+0)>>2] = matches.length;

  if (!matches.length) {
    return null
  }

  // return a copy of the match list
  var list = _Z_Malloc((matches.length + 1) * 4);

  var i;
  for (i = 0; i < matches.length; i++) {
    var filename = _S_Malloc(matches[i].length + 1);

    stringToUTF8(matches[i], filename, matches[i].length+1);

    // write the string's pointer back to the main array
    HEAP32[(list+i*4)>>2] = filename;
  }

  // add a NULL terminator to the list
  HEAP32[(list+i*4)>>2] = 0;

  return list;
}

function Sys_FS_Shutdown () {
  /*
  if(SYSF.pathname) {
    _free(SYSF.pathname)
    SYSF.pathname = 0
  }
  if(SYSF.modeStr) {
    _free(SYSF.modeStr)
    SYSF.modeStr = 0
  }
  */
  // save to drive
  FS.syncfs(false, function (err) {
    if (err) {
      // FIXME cb_free_context(context)
      Sys_Error('fatal', err)
      return
    }
    
    ProxyCallback()
  })
}

function Sys_FS_Offline () {
  // call startup, it's idempotent and won't hurt to call multiple times in a row
  _Sys_Startup(function () {
    // but instead of calling filter, we add ALL files to download immediately
    Object.keys(SYSF.index).forEach(function (k) {
      SYSN.downloads.push(SYSF[k].name)
    })
    var filecount = SYSN.downloads.length
    // download now, then report status
    downloadImmediately(function () {
      Sys_Print("Downloads finished: " + filecount + " files saved for offline.")
    })
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