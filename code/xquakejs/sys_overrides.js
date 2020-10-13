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
