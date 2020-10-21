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
      } else if (FS.isLink(entry['mode'])) {
        FS.symlink(intArrayToString(entry['contents']), path)
      } else {
        return callback(new Error('node type not supported'))
      }
    } catch (e) {
      if (!(e instanceof FS.ErrnoError) || e.code !== 'EEXIST') {
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
  IDBFS.loadLocalEntry = function(path, callback) {
    var stat, node;

    try {
      var lookup = FS.lookupPath(path);
      node = lookup.node;
      stat = FS.lstat(path);
    } catch (e) {
      return callback(e);
    }

    if (FS.isDir(stat.mode)) {
      return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode });
    } else if (FS.isFile(stat.mode)) {
      // Performance consideration: storing a normal JavaScript array to a IndexedDB is much slower than storing a typed array.
      // Therefore always convert the file contents to a typed array first before writing the data to IndexedDB.
      node.contents = MEMFS.getFileDataAsTypedArray(node);
      return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode, 'contents': node.contents });
    } else if (FS.isLink(stat.mode)) {
      node.contents = Uint8Array.from(FS.readlink(path).split('').map(c => c.charCodeAt(0)));
      return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode, 'contents': node.contents });
    } else {
      return callback(new Error('node type not supported'));
    }
  }
  IDBFS.getLocalSet = function(mount, callback) {
    var entries = {};

    function isRealDir(p) {
      return p !== '.' && p !== '..';
    };
    function toAbsolute(root) {
      return function(p) {
        return PATH.join2(root, p);
      }
    };

    var check = FS.readdir(mount.mountpoint).filter(isRealDir).map(toAbsolute(mount.mountpoint));

    while (check.length) {
      var path = check.pop();
      var stat;

      try {
        stat = FS.lstat(path);
      } catch (e) {
        return callback(e);
      }

      if (FS.isDir(stat.mode)) {
        check.push.apply(check, FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
      }

      entries[path] = { 'timestamp': stat.mtime };
    }

    return callback(null, { type: 'local', entries: entries });
  }
  IDBFS.reconcile = function(src, dst, callback) {
    var total = 0;

    var create = [];
    Object.keys(src.entries).forEach(function (key) {
      var e = src.entries[key];
      var e2 = dst.entries[key];
      if (!e2 || e['timestamp'] > e2['timestamp']) {
        create.push(key);
        total++;
      }
    });

    var remove = [];
    Object.keys(dst.entries).forEach(function (key) {
      var e = dst.entries[key];
      var e2 = src.entries[key];
      if (!e2 || e2['timestamp'] > e['timestamp']) {
        remove.push(key);
        total++;
      }
    });

    if (!total) {
      return callback(null);
    }

    var errored = false;
    var db = src.type === 'remote' ? src.db : dst.db;
    var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
    var store = transaction.objectStore(IDBFS.DB_STORE_NAME);

    function done(err) {
      /*
      if (err && !errored) {
        errored = true;
        return callback(err);
      }
      */
    };

    transaction.onerror = function(e) {
      done(this.error);
      e.preventDefault();
    };

    transaction.oncomplete = function(e) {
      if (!errored) {
        callback(null);
      }
    };

    // sort paths in ascending order so directory entries are created
    // before the files inside them
    create.sort().forEach(function (path) {
      if (dst.type === 'local') {
        IDBFS.loadRemoteEntry(store, path, function (err, entry) {
          if (err) return done(err);
          IDBFS.storeLocalEntry(path, entry, done);
        });
      } else {
        IDBFS.loadLocalEntry(path, function (err, entry) {
          if (err) return done(err);
          IDBFS.storeRemoteEntry(store, path, entry, done);
        });
      }
    });

    // TODO: add rsync with timebased event handling and actual delete auditing
    //   fuck this, its so lazy
    remove.sort().forEach(function (path) {
      if (dst.type === 'local') {
        IDBFS.loadLocalEntry(path, function (err, entry) {
          if (err) return done(err);
          IDBFS.storeRemoteEntry(store, path, entry, done);
        });
      } else {
        IDBFS.loadRemoteEntry(store, path, function (err, entry) {
          if (err) return done(err);
          IDBFS.storeLocalEntry(path, entry, done);
        });
      }
    });
    
    // sort paths in descending order so files are deleted before their
    // parent directories
    /*
    remove.sort().reverse().forEach(function(path) {
      if (dst.type === 'local') {
        IDBFS.removeLocalEntry(path, done);
      } else {
        IDBFS.removeRemoteEntry(store, path, done);
      }
    });
    */
  }
}
if(typeof GL != 'undefined') {
  GL.createContext = function(canvas, webGLContextAttributes) {
    webGLContextAttributes.failIfMajorPerformanceCaveat = true
    var ctx = (webGLContextAttributes.majorVersion > 1)
      ? canvas.getContext("webgl2", webGLContextAttributes)
      : (canvas.getContext("webgl", webGLContextAttributes)
        || canvas.getContext('experimental-webgl'))
    if (!ctx) return 0
    var handle = GL.registerContext(ctx, webGLContextAttributes)
    return handle
  }
}
