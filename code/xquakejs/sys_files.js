var LibrarySysFiles = {
  $SYSF__deps: ['$SYSC', '$IDBFS'],
  $SYSF: {
    index: [],
    fs_replace: [],
    fs_basepath: '/base',
    pathname: 0,
    modeStr: 0,
    firstTime: false,
    mods: [
      // A list of supported mods, 'dirname-cc' (-ccr means combined converted repacked)
  		//   To the right is the description text, atomatically creates a placeholder.pk3dir with description.txt inside
  		// We use a list here because Object keys have no guarantee of order
			['baseq3-cc', 			'Quake III Arena'],
			['missionpack-cc',  '0 Choice: Team Arena'],
			['defrag-cc',       '1 Choice: Defrag'],
			['baseq3r-cc',      '2 Choice: Q3Rally'],
			['basemod-cc', 		  '3 Choice: Monkeys of Doom'],
			['generations-cc',  '4 Choice: Generations Arena'],
			['q3f2-cc', 				'5 Choice: Q3 Fortress 2'],
			['cpma-cc', 				'6 Choice: Challenge ProMode'],
			['q3ut4-cc', 		 	  '7 Choice: Urban Terror 4'],
			['freezetag-cc', 	  '8 Choice: Freeze Tag'],
			['corkscrew-cc', 	  '9 Choice: Corkscrew'],
			['freon-cc', 			  'Excessive Plus: Freon'],
      ['baseoa-cc', 			'Open Arena'],
			['bfpq3-cc', 			  'Bid For Power'],
			['excessive-cc', 	  'Excessive+'],
			['q3ut3-cc', 			  'Urban Terror 3'],
			['edawn-cc', 			  'eDawn'],
			['geoball-cc', 		  'Geoball'],
			['neverball-cc', 	  'Neverball'],
			['omissionpack-cc', 'OpenArena Mission Pack'],
			['platformer-cc', 	'Platformer'],
			['legoc-cc', 			  'Lego Carnage'],
			['osp-cc', 				  'Orange Smoothie Productions'],
			['quake2arena-cc',  'Quake 2 Arena'],
			['smokin-cc', 			'Smokin\' Guns'],
			['wfa-cc', 				  'Weapons Factory Arena'],
			['uberarena-cc', 	  'Uber Arena'],
			['demoq3-cc', 			'Quake III Demo'],
			['mfdata-cc', 			'Military Forces'],
			['conjunction-cc',  'Dark Conjunction'],
			['chili-cc', 			  'Chili Quake XXL'],
			['hqq-cc', 				  'High Quality Quake'],
      ['entityplus-cc', 	'Engine Of Creation: Entity Plus'],
      ['wop-cc',          'World of Padman'],
			['rocketarena-cc',  'Coming Soon: Rocket Arena'],
			['gpp-cc',          'Coming Soon: Tremulous'],
			['gppl-cc',         'Coming Soon: Unvanquished'],
			['iortcw-cc',       'Coming Soon: Return to Castle Wolfenstien'],
      ['baset-cc',        'Coming Soom: Wolfenstien: Enemy Territory'],
      ['openjk-cc',       'Coming Soon: Jedi Knights: Jedi Academy'],
      ['baseef-cc',       'Coming Soon: Star Trek: Elite Force'],
		],
    filterDownloads: function (mapname, modelname) {
      // create virtual file entries for everything in the directory list
      var keys = Object.keys(SYSF.index)
      var mapMatch = new RegExp(
        '\/levelshots\/' + mapname
        + '|\/' + mapname + '\.bsp'
        + '|\/' + mapname + '\.aas', 'i')
      var playerMatch = new RegExp('sarge\/'
        + '|' + modelname + '\/', 'i')
      // servers need some map and model info for hitboxes up front
      for(var i = 0; i < keys.length; i++) {
        var file = SYSF.index[keys[i]]
        if(typeof file.size == 'undefined') { // create a directory
          try {
          //  SYSC.mkdirp(PATH.join(SYSF.fs_basepath, file.name))
          } catch (e) {
            debugger
          }
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
            SYSC.Error('fatal', e.message)
          }
        }

        if(file.name.match(/\.pk3$|\.wasm|\.qvm|\.cfg|\.arena|\.shader|\.font/i)
        // download files for menu system
          || file.name.match(/\.menu|menus\.txt|ingame\.txt|hud[0-9]*\.txt|arenas\.txt/i)
          || file.name.match(/ui\/.*\.h|\.crosshair|logo512|banner5|\/hit\.|\/2d\//i)
        // download required model and bot
          || file.name.match(/\.hit|sarge\/|botfiles|\.bot|bots\.txt|gfx\//i)
        // download the current map if it is referred to
          || (modelname.length > 0 && file.name.match(playerMatch))
          || (mapname.length > 0 && file.name.match(mapMatch))) {
          SYSF.index[keys[i]].downloading = true
          SYSN.downloads.push(file.name)
        } else if (
          // these files can be streamed in
          file.name.match(/(players|player)\/(sarge|major|sidepipe|athena|orion|gi)\//i)
          // download levelshots and common graphics
          || file.name.match(/description\.txt|levelshots|(^|\/)ui\/|common\/|icons\/|menu\/|sfx\//i)
          // stream player icons so they show up in menu
          || file.name.match(/\/icon_|\.skin/i)
        ) {
          //SYSF.index[keys[i]].downloading = true
          //SYSN.downloadLazy.push(file.name)
        } else {
        }
      }
    },
    downloadsDone: function () {
      return FS.syncfs(false, (e) => {
        if(e) console.log(e)
        SYSN.downloads = []
        SYSN.LoadingDescription('')
        SYSC.ProxyCallback()
        if(SYSF.firstTime && typeof window.serverWorker != 'undefined') {
          SYSF.firstTime = false
          //_Com_WriteConfigToFile('interlinked.cfg')
          window.serverWorker.postMessage(['init', ['+exec', 'interlinked.cfg']
            .concat(SYSM.getQueryCommands())])
        }
      })
    },
    downloadImmediately: function (cb) {
      var totals = []
      var progresses = []
      if(SYSN.downloads.length === 0) {
        cb()
        return
      }
      Promise.all(SYSN.downloads.map((file, i) => new Promise(resolve => {
        total = 0
        progresses[i] = 0
        SYSC.DownloadAsset(file, null, (err, data) => {
          progresses[i] = totals[i]
          SYSN.LoadingProgress(++total, SYSN.downloads.length)
          if(err) return resolve(err)
          try {
            SYSC.mkdirp(PATH.join(SYSF.fs_basepath, PATH.dirname(file)))
            FS.writeFile(PATH.join(SYSF.fs_basepath, file), new Uint8Array(data), {
              encoding: 'binary', flags: 'w', canOwn: true })
          } catch (e) {
            if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
              SYSC.Error('fatal', e.message)
            }
          }
          resolve(file)
        })
        // save to drive
      }))).then(cb)
    },
  },
  Sys_FS_Startup__deps: ['$SYS', '$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
  Sys_FS_Startup: function (cb) {
    if(!cb) cb = SYSF.downloadImmediately.bind(null, SYSF.downloadsDone)
    SYSF.fs_replace = []
    SYSF.fs_replace.push(new RegExp('\/\/', 'ig'))
    SYSF.cl_lazyLoad = SYSC.Cvar_Get('cl_lazyLoad')
    var newDLURL = SYSC.Cvar_VariableString('sv_dlURL')
    if(newDLURL.length > 0) {
      if(SYSC.oldDLURL.length == 0) {
        SYSC.oldDLURL = SYSC.newDLURL
      }
      SYSC.newDLURL = newDLURL
    }
    //SYSN.downloadLazy.splice(0) // reset lazy list to start of map
    SYSF.pathname = allocate(new Int8Array(4096), 'i8', ALLOC_NORMAL)
    SYSF.modeStr = allocate(new Int8Array(4), 'i8', ALLOC_NORMAL)
    var fs_homepath = SYSC.Cvar_VariableString('fs_homepath')
    var fs_basepath = SYSC.Cvar_VariableString('fs_basepath')
    SYSF.fs_basepath = fs_basepath;
    var fs_basegame = SYSC.Cvar_VariableString('fs_basegame')
    if(fs_basegame.length > 0)
      SYSF.fs_replace.push(new RegExp('\/*' + fs_basegame + '\/', 'ig'))
    SYSF.fs_basegame = fs_basegame
    var sv_pure = SYSC.Cvar_VariableString('sv_pure')
    var fs_game = SYSC.Cvar_VariableString('fs_game')
    if(fs_game.length > 0)
      SYSF.fs_replace.push(new RegExp('\/*' + fs_game + '\/', 'ig'))
    SYSF.fs_replace.sort((a, b) => b.source.length - a.source.length)
    var mapname = SYSC.Cvar_VariableString('mapname')
    var modelname = SYSC.Cvar_VariableString('model')
    var playername = SYSC.Cvar_VariableString('name')
    var clcState = _CL_GetClientState()
    const blankFile = new Uint8Array(4)
    
    SYSN.LoadingDescription('Loading Game UI...')
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
        console.error(err)
      }

      SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds')
      SYSC.mkdirp(PATH.join(fs_basepath, fs_basegame))
      SYSC.mkdirp(PATH.join(fs_basepath, fsMountPath))

      for(var i = 0; i < (SYSF.mods || []).length; i++) {
        var desc = PATH.join(fs_basepath, SYSF.mods[i][0], 'description.txt')
        var prettyDesc = Uint8Array.from(intArrayFromString(SYSF.mods[i][1]).slice(0, SYSF.mods[i][1].length-1))
        SYSC.mkdirp(PATH.join(PATH.dirname(desc), '0000placeholder.pk3dir'))
        FS.writeFile(desc, prettyDesc, {
          encoding: 'binary', flags: 'w', canOwn: true })
        SYSF.index[desc.toLowerCase()] = {
          name: desc,
          size: prettyDesc.length
        }
      }

      // TODO: is this right? exit early without downloading anything so the server can force it instead
      // server will tell us what pk3s we need
      /*
      if(clcState < 4 && sv_pure && fs_game.localeCompare(fs_basegame) !== 0) {
        SYSN.LoadingDescription('')
        FS.syncfs(false, () => SYSC.ProxyCallback(cb))
        return
      }
      */

      SYSN.downloads = []
      var indexes = [
        fs_basegame
      ]
      if(fsMountPath != fs_basegame) {
        indexes.push(fsMountPath)
      }
      if(mapname.length > 0) {
        indexes.push(fsMountPath + '/index-' + mapname.toLowerCase() + '.json')
      }
      if(playername.length > 0) {
        indexes.push(fsMountPath + '/index-' + playername.toLowerCase() + '.json')
      }
      var current = 0
      var download;
      download = () => {
        if(current < indexes.length) {
          SYSN.DownloadIndex(indexes[current], download)
          current++
        } else {
          SYSF.filterDownloads(mapname, modelname)
          cb()
        }
      }
      download()
    })
  },
  Sys_FOpen__deps: ['$SYS', '$FS', '$PATH', 'fopen'],
  Sys_FOpen: function (ospath, mode) {
    var handle = 0
    try {
      intArrayFromString(UTF8ToString(mode)
        .replace('b', '')).forEach((c, i) => HEAP8[(SYSF.modeStr+i)] = c)
      HEAP8[(SYSF.modeStr+2)] = 0
      var filename = UTF8ToString(ospath).replace(/\/\//ig, '/')
      // use the index to make a case insensitive lookup
      var indexFilename = filename.toLowerCase()
      if(SYSF.index && typeof SYSF.index[indexFilename] != 'undefined') {
        var altName = filename.substr(0, filename.length
          - SYSF.index[indexFilename].name.length)
          + SYSF.index[indexFilename].name
        var exists = false
        try { exists = FS.lookupPath(altName) } catch (e) { exists = false }
        if(handle === 0 && exists) {
          intArrayFromString(altName).forEach((c, i) => HEAP8[(SYSF.pathname+i)] = c)
          HEAP8[(SYSF.pathname+altName.length)] = 0
          handle = _fopen(SYSF.pathname, SYSF.modeStr)
        }
        var loading = SYSC.Cvar_VariableString('r_loadingShader')
        if(loading.length === 0) {
          loading = SYSC.Cvar_VariableString('snd_loadingSound')
          if(loading.length === 0) {
            loading = SYSC.Cvar_VariableString('r_loadingModel')
          }
        } else if (!SYSF.index[indexFilename].shaders.includes(loading)) {
          SYSF.index[indexFilename].shaders.push(loading)
        }

        if((handle === 0
          || HEAP8[SYSF.cl_lazyLoad+8*4] === 2)
          && !SYSF.index[indexFilename].downloading) {
          if(SYSF.index[indexFilename].alreadyDownloaded)
            SYSN.downloadLazy.unshift([loading, SYSF.index[indexFilename].name])
          else
            SYSN.downloadLazy.push([loading, SYSF.index[indexFilename].name])
          SYSF.index[indexFilename].downloading = true
        }
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
  Sys_ListFiles__deps: ['$PATH', 'Z_Malloc', 'S_Malloc'],
  Sys_ListFiles: function (directory, ext, filter, numfiles, dironly) {
    directory = UTF8ToString(directory);
    ext = UTF8ToString(ext);
    if (ext === '/') {
      ext = null;
      dironly = true;
    }

    // TODO support filter
    
    var contents;
    try {
      //contents = FS.readdir(directory)
      contents = Object.keys(SYSF.index)
        .filter(k => k.match(new RegExp(directory + '\\/[^\\/]+\\/?$', 'i'))
          && (!dironly || typeof SYSF.index[k].size == 'undefined'))
        .map(k => PATH.basename(SYSF.index[k].name))
        .filter((f, i, arr) => f && arr.indexOf(f) === i)
        .filter(f => {
          try {
            var stat = FS.stat(PATH.join(directory, f))
            return stat && (!dironly || FS.isDir(stat.mode))
          } catch (e) {
            if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
              throw e
            }
          }
        })
      if(directory.match(/\/demos/i)) {
        contents = contents
          .concat(FS.readdir(PATH.join(PATH.dirname(directory), '/svdemos')))
          .filter(f => !dironly || FS.isDir(FS.stat(PATH.join(directory, f)).mode))
      }
      if(contents.length > 5000) {
        debugger
        return null
      }
    } catch (e) {
      {{{ makeSetValue('numfiles', '0', '0', 'i32') }}};
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

    {{{ makeSetValue('numfiles', '0', 'matches.length', 'i32') }}};

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
      {{{ makeSetValue('list', 'i*4', 'filename', 'i32') }}};
    }

    // add a NULL terminator to the list
    {{{ makeSetValue('list', 'i*4', '0', 'i32') }}};

    return list;
  },
  Sys_FS_Shutdown__deps: ['$Browser', '$FS', '$SYSC'],
  Sys_FS_Shutdown: function (cb) {
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
        SYSC.Error('fatal', err)
        return
      }
      
      SYSC.ProxyCallback(cb)
    })
  },
  Sys_DefaultBasePath: function () {
		return allocate(intArrayFromString('/base'), 'i8', ALLOC_STACK)
	},
	Sys_Pwd: function () {
		return allocate(intArrayFromString('/base'), 'i8', ALLOC_STACK)
	},
  Sys_FS_Offline: function () {
    // call startup, it's idempotent and won't hurt to call multiple times in a row
    _Sys_Startup(() => {
      // but instead of calling filter, we add ALL files to download immediately
      Object.keys(SYSF.index).forEach(k => {
        SYSN.downloads.push(SYSF[k].name)
      })
      var filecount = SYSN.downloads.length
      // download now, then report status
      SYSF.downloadImmediately(() => {
        SYSC.Print("Downloads finished: " + filecount + " files saved for offline.")
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
}
autoAddDeps(LibrarySysFiles, '$SYSF')
mergeInto(LibraryManager.library, LibrarySysFiles);
if(typeof IDBFS != 'undefined') {
  IDBFS.loadRemoteEntry = function (store, path, callback) {
    var req = store.get(path)
    req.onsuccess = function (event) {
      callback(null, {
        timestamp: event.target.result.timestamp,
        mode: event.target.result.mode,
        contents: MEMFS.getFileDataAsTypedArray(event.target.result)
      })
    }
    req.onerror = function (e) {
      callback(this.error)
      e.preventDefault()
    }
  }
  IDBFS.storeLocalEntry = function(path, entry, callback) {
    if(path.includes('http')) {
      debugger
    }
    try {
      if (FS.isDir(entry['mode'])) {
        FS.mkdir(path, entry['mode'])
      } else if (FS.isFile(entry['mode'])) {
        FS.writeFile(path, entry['contents'], { canOwn: true })
      } else {
        return callback(new Error('node type not supported'))
      }
    } catch (e) {
      if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.EEXIST) {
        return callback(e)
      }
    }
    try {
      FS.chmod(path, entry['mode'])
      FS.utime(path, entry['timestamp'], entry['timestamp'])
    } catch (e) {
      return callback(e)
    }

    callback(null)
  }
}
