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
