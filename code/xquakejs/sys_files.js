var LibrarySysFiles = {
  $SYSF__deps: ['$SYSC', '$IDBFS'],
  $SYSF: {
    index: [],
    fs_basepath: '/base',
		fs_game: 'baseq3-cc',
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
			['rocketarena-cc',  'Coming Soon: Rocket Arena'],
			['gpp-cc',          'Coming Soon: Tremulous'],
			['gppl-cc',         'Coming Soon: Unvanquished'],
			['iortcw-cc',       'Coming Soon: Return to Castle Wolfenstien'],
      ['baset-cc',        'Coming Soom: Wolfenstien: Enemy Territory'],
      ['openjk-cc',       'Coming Soon: Jedi Knights: Jedi Academy'],
      ['baseef-cc',       'Coming Soon: Star Trek: Elite Force'],
		],
    filterDownloads: function (mapname) {
      // create virtual file entries for everything in the directory list
      var keys = Object.keys(SYSF.index)
      // servers need some map and model info for hitboxes up front
      for(var i = 0; i < keys.length; i++) {
        var file = SYSF.index[keys[i]]
        if(typeof file.size == 'undefined') { // create a directory
          SYSC.mkdirp(PATH.join(SYSF.fs_basepath, file.name))
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
        // temporary FIX
        // TODO: remove this with when Async file system loading works,
        //   renderer, client, deferred loading cg_deferPlayers|loaddeferred
        // always download these files beforehand
        if(file.name.match(/\.pk3dir/)) {
          // create the directory to make sure it makes it in to Q3s search paths
          SYSC.mkdirp(PATH.dirname(PATH.join(SYSF.fs_basepath, file.name)))
        }
        if(file.name.match(/\.pk3$|\.wasm|\.qvm|\.cfg|\.arena|\.shader|\.font/i)
        // download files for menu system
          || file.name.match(/\.menu|menus\.txt|ingame\.txt|hud.txt|arenas\.txt/i)
          || file.name.match(/ui\/.*\.h|\.crosshair|logo512|banner5|\/hit\.|\/2d\//i)
        // download required model and bot
          || file.name.match(/\/sarge\/icon_|sarge\/.*\.skin|botfiles|\.bot|bots\.txt/i)
        // download the current map if it is referred to
          || file.name.match(new RegExp('\/levelshots\/' + mapname, 'i'))
          || file.name.match(new RegExp('\/' + mapname + '\.bsp', 'i'))
          || file.name.match(new RegExp('\/' + mapname + '\.aas', 'i'))) {
          SYSF.index[keys[i]].downloading = true
          SYSN.downloads.push(file.name)
        } else if (
          // these files can be streamed in
          file.name.match(/(players|player)\/(sarge|major|sidepipe|athena|orion)\//i)
          // download levelshots and common graphics
          || file.name.match(/description\.txt|levelshots|^ui\/|common\/|icons\/|menu\/|gfx\/|sfx\//i)
          // stream player icons so they show up in menu
          || file.name.match(/\/icon_|\.skin/i)
        ) {
          SYSF.index[keys[i]].downloading = true
          SYSN.downloadLazy.push(file.name)
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
    downloadImmediately: function () {
      var totals = []
      var progresses = []
      if(SYSN.downloads.length === 0) {
        SYSF.downloadsDone()
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
      }))).then(SYSF.downloadsDone)
    },
  },
  Sys_FS_Startup__deps: ['$SYS', '$Browser', '$FS', '$PATH', '$IDBFS', '$SYSC'],
  Sys_FS_Startup: function () {
    var newDLURL = SYSC.Cvar_VariableString('sv_dlURL')
    if(newDLURL.length > 0) {
      SYSC.newDLURL = newDLURL
    }
    SYSF.pathname = allocate(new Int8Array(4096), 'i8', ALLOC_NORMAL)
    SYSF.modeStr = allocate(new Int8Array(4), 'i8', ALLOC_NORMAL)
    var fs_homepath = SYSC.Cvar_VariableString('fs_homepath')
    var fs_basepath = SYSC.Cvar_VariableString('fs_basepath')
    SYSF.fs_basepath = fs_basepath;
    var fs_basegame = SYSC.Cvar_VariableString('fs_basegame')
    var sv_pure = SYSC.Cvar_VariableString('sv_pure')
    var fs_game = SYSC.Cvar_VariableString('fs_game')
    SYSF.fs_game = fs_game;
    var mapname = SYSC.Cvar_VariableString('mapname')
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
        console.warn(err.message)
      }

      SYSC.Print('initial sync completed in ' + ((Date.now() - start) / 1000).toFixed(2) + ' seconds')
      SYSC.mkdirp(PATH.join(fs_basepath, fs_basegame))
      SYSC.mkdirp(PATH.join(fs_basepath, fsMountPath))

      for(var i = 0; i < (SYSF.mods || []).length; i++) {
        var desc = PATH.join(fs_basepath, SYSF.mods[i][0], 'description.txt')
        SYSC.mkdirp(PATH.join(PATH.dirname(desc), '0000placeholder.pk3dir'))
        FS.writeFile(desc, Uint8Array.from(intArrayFromString(SYSF.mods[i][1]).slice(0, SYSF.mods[i][1].length-1)), {
          encoding: 'binary', flags: 'w', canOwn: true })
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
      if(fsMountPath != fs_basegame) {
        SYSN.DownloadIndex(fs_basegame, () => {
          SYSN.DownloadIndex(fsMountPath, () => {
            SYSF.filterDownloads(mapname)
            SYSF.downloadImmediately()
          })
        })
      } else {
        SYSN.DownloadIndex(fsMountPath, () => {
          SYSF.filterDownloads(mapname)
          SYSF.downloadImmediately()
        })
      }
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
      var exists = false
      try { exists = HEAP8[(SYSF.modeStr+0)] === 'w'.charCodeAt(0)
        || FS.lookupPath(filename) } catch (e) { exists = false }
      if(exists) {
        intArrayFromString(filename).forEach((c, i) => HEAP8[(SYSF.pathname+i)] = c)
        HEAP8[(SYSF.pathname+filename.length)] = 0
        handle = _fopen(SYSF.pathname, SYSF.modeStr)
      }
      //if(handle === 0) {
        // use the index to make a case insensitive lookup
        var indexFilename = filename.toLowerCase()
        if(SYSF.index && typeof SYSF.index[indexFilename] != 'undefined') {
          var altName = filename.substr(0, filename.length
            - SYSF.index[indexFilename].name.length)
            + SYSF.index[indexFilename].name
          try { exists = FS.lookupPath(altName) } catch (e) { exists = false }
          if(handle === 0 && altName != filename && exists) {
            intArrayFromString(altName).forEach((c, i) => HEAP8[(SYSF.pathname+i)] = c)
            HEAP8[(SYSF.pathname+altName.length)] = 0
            handle = _fopen(SYSF.pathname, SYSF.modeStr)
            //if(handle > 0) {
            //	return handle
            //}
          }
          var loading = SYSC.Cvar_VariableString('r_loadingShader')
          if(loading.length === 0) {
            loading = SYSC.Cvar_VariableString('snd_loadingSound')
            if(loading.length === 0) {
              loading = SYSC.Cvar_VariableString('r_loadingModel')
            }
          }
          if(!SYSF.index[indexFilename].downloading) {
            SYSN.downloadLazy.push([loading, SYSF.index[indexFilename].name])
            SYSF.index[indexFilename].shaders.push(loading)
            SYSF.index[indexFilename].downloading = true
          } else if (!SYSF.index[indexFilename].shaders.includes(loading)) {
            SYSF.index[indexFilename].shaders.push(loading)
          }
        }
      //}
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
      contents = FS.readdir(directory)
        .filter(f => !dironly || FS.isDir(FS.stat(PATH.join(directory, f)).mode))
      contents = contents.concat(Object.keys(SYSF.index)
        .filter(k => k.match(new RegExp(directory + '\\/[^\\/]+\\/?$', 'i'))
          && (!dironly || typeof SYSF.index[k].size == 'undefined'))
        .map(k => PATH.basename(SYSF.index[k].name)))
        .filter((f, i, arr) => f && arr.indexOf(f) === i)
      if(contents.length > 5000) {
        debugger
      }
    } catch (e) {
      {{{ makeSetValue('numfiles', '0', '0', 'i32') }}};
      return null;
    }

    var matches = [];
    for (var i = 0; i < contents.length; i++) {
      var name = contents[i].toLowerCase();
      if (!ext || name.lastIndexOf(ext) === (name.length - ext.length)
        || (ext.match(/tga/i) && name.lastIndexOf('png') === (name.length - ext.length))
        || (ext.match(/tga/i) && name.lastIndexOf('jpg') === (name.length - ext.length))
      ) {
        matches.push(contents[i]);
      }
    }

    {{{ makeSetValue('numfiles', '0', 'matches.length', 'i32') }}};

    if (!matches.length) {
      return null;
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
