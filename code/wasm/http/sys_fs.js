
var DB_STORE_NAME = 'FILE_DATA';

function openDatabase(noWait) {
  if(FS.database) {
    return Promise.resolve(FS.database)
  }
  if(!FS.database && (!FS.open || Date.now() - FS.openTime > 3000)) {
    FS.openTime = Date.now()
    // TODO: make a separate /home store for content to upload submissions to NPM-style packaging system
    // TODO: synchronize saved game states and config files out of /home database
    // TODO: on Native /base is manually configured, manually downloaded, /home is auto-downloaded
    //   on web /base is auto-downloaded and home is manually configured/drag-drop, fix this
    return new Promise(function (resolve) {
      FS.open = indexedDB.open('/base', 22)
      FS.open.onsuccess = function (evt) {
        FS.database = evt.target.result
        resolve(FS.database)
        //if(!Array.from(FS.database.objectStoreNames).includes(DB_STORE_NAME)) {
        //  FS.database.createObjectStore(DB_STORE_NAME)
        //}
      }
      FS.open.onupgradeneeded = function () {
        let fileStore = FS.open.result.createObjectStore(DB_STORE_NAME)
        if (!fileStore.indexNames.contains('timestamp')) {
          fileStore.createIndex('timestamp', 'timestamp', { unique: false });
        }
      }
      FS.open.onerror = function (error) {
        console.error(error)
        resolve(error)
      }
    })
  } else if (!noWait) {
    return new Promise(function (resolve) { setTimeout(function () {
      openDatabase(true).then(resolve)
    }, 1000) })
  } else {
    throw new Error('no database')
  }
}

function readStore(key) {
  return openDatabase()
  .then(function (db) {
    var transaction = db.transaction([DB_STORE_NAME], 'readwrite');
    var objStore = transaction.objectStore(DB_STORE_NAME);
    return new Promise(function (resolve) {
      let tranCursor = objStore.get(key)
      tranCursor.onsuccess = function () {
        resolve(tranCursor.result)
      }
      tranCursor.onerror = function (error) {
        console.error(error)
        resolve(error)
      }
      transaction.commit()
    })
  })
  .catch(function (e) {})
}

function writeStore(value, key) {
  return openDatabase()
  .then(function (db) {
    let transaction = db.transaction([DB_STORE_NAME], 'readwrite');
    let objStore = transaction.objectStore(DB_STORE_NAME);
    return new Promise(function (resolve) {
      let storeValue  
      if(value === false) {
        storeValue = objStore.delete(key)
      } else {
        storeValue = objStore.put(value, key)
      }
      storeValue.onsuccess = function () {}
      transaction.oncomplete = function () {
        resolve(storeValue.result)
        //FS.database.close()
        //FS.database = null
        //FS.open = null
      }
      storeValue.onerror = function (error) {
        console.error(error, value, key)
      }
      transaction.commit()
    })
  })
  .catch(function (e) {})
}


function Sys_Mkdir(filename) {
  let fileStr = addressToString(filename)
  let localName = fileStr
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)
  FS.virtual[localName] = {
    timestamp: new Date(),
    mode: 16895,
  }
  // async to filesystem
  // does it REALLY matter if it makes it? wont it just redownload?
  openDatabase().then(function (db) {
    writeStore(FS.virtual[localName], localName)
  })
}

function Sys_GetFileStats( filename, size, mtime, ctime ) {
  let fileStr = addressToString(filename)
  let localName = fileStr
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)
  if(typeof FS.virtual[localName] != 'undefined') {
    HEAP32[size >> 2] = (FS.virtual[localName].contents || []).length
    HEAP32[mtime >> 2] = FS.virtual[localName].timestamp.getTime()
    HEAP32[ctime >> 2] = FS.virtual[localName].timestamp.getTime()
    return 1
  } else {
    HEAP32[size >> 2] = 0
    HEAP32[mtime >> 2] = 0
    HEAP32[ctime >> 2] = 0
    return 0
  }
}

function Sys_FOpen(filename, mode) {
  let parentDirectory
  // now we don't have to do the indexing crap here because it's built into the engine already
  let fileStr = addressToString(filename)
  let modeStr = addressToString(mode)
  let localName = fileStr
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)

  let createFP = function () {
    FS.filePointer++
    FS.pointers[FS.filePointer] = [
      0, // seek/tell
      modeStr,
      FS.virtual[localName],
      localName
    ]
    return FS.filePointer // not zero
  }

  // TODO: check mode?
  if(typeof FS.virtual[localName] != 'undefined') {
    // open the file successfully
    return createFP()
  } else if (modeStr.includes('w')
    && (parentDirectory = localName.substring(0, localName.lastIndexOf('/')))
    && typeof FS.virtual[parentDirectory] != 'undefined') {
    // create the file for write because the parent directory exists
    FS.virtual[localName] = {
      timestamp: new Date(),
      mode: 33206,
      contents: new Uint8Array(0)
    }
    return createFP()
  } else {
    return 0 // POSIX
  }
}

function Sys_FTell(pointer) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  return FS.pointers[pointer][0]
}

function Sys_FSeek(pointer, position, mode) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  if(mode == 0 /* SEEK_SET */) {
    FS.pointers[pointer][0] = position
  } else if (mode == 1 /* SEEK_CUR */) {
    FS.pointers[pointer][0] += position
  } else if (mode == 2 /* SEEK_END */) {
    FS.pointers[pointer][0] = FS.pointers[pointer][2].contents.length + position
  }
  return FS.pointers[pointer][0]
}

function Sys_FClose(pointer) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  writeStore(FS.pointers[pointer][2], FS.pointers[pointer][3])
  FS.pointers[pointer] = void 0
}

function Sys_FWrite(buf, count, size, pointer) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  let tmp = FS.pointers[pointer][2].contents
  if(FS.pointers[pointer][0] + count * size > FS.pointers[pointer][2].contents.length) {
    tmp = new Uint8Array(FS.pointers[pointer][2].contents.length + count * size);
    tmp.set(new Uint8Array(FS.pointers[pointer][2].contents), 0);
  }
  tmp.set(new Uint8Array(HEAP8.slice(buf, buf + count * size)), FS.pointers[pointer][0]);
  FS.pointers[pointer][2].contents = tmp
  return count * size
}

function Sys_FFlush(pointer) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  writeStore(FS.pointers[pointer][2], FS.pointers[pointer][3])
}

function Sys_FRead(bufferAddress, byteSize, count, pointer) {
  if(typeof FS.pointers[pointer] == 'undefined') {
    throw new Error('File IO Error') // TODO: POSIX
  }
  let i = 0
  for(; i < count * byteSize; i++ ) {
    if(FS.pointers[pointer][0] + i >= FS.pointers[pointer][2].contents.length) {
      break
    }
    HEAP8[bufferAddress + i] = FS.pointers[pointer][2].contents[FS.pointers[pointer][0] + i]
  }
  return i
}

function Sys_Remove(file) {
  let fileStr = addressToString(file)
  let localName = fileStr
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)
  if(typeof FS.virtual[localName] != 'undefined') {
    delete FS.virtual[localName]
    // remove from IDB
    writeStore(false, localName)
  }
}

function Sys_Rename(src, dest) {
  let strStr = addressToString(src)
  let srcName = strStr
  if(srcName.startsWith('/base')
    || srcName.startsWith('/home'))
    srcName = srcName.substring('/base'.length)
  if(srcName[0] == '/')
    srcName = srcName.substring(1)
  let fileStr = addressToString(dest)
  let destName = fileStr
  if(destName.startsWith('/base')
    || destName.startsWith('/home'))
    destName = destName.substring('/base'.length)
  if(destName[0] == '/')
    destName = destName.substring(1)
  debugger
}


function Sys_ListFiles (directory, extension, filter, numfiles, wantsubs) {
  let files = {
    'default.cfg': {
      mtime: 0,
      size: 1024,
    }
  }
  // TODO: don't combine /home and /base?
  let localName = addressToString(directory)
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)
  let extensionStr = addressToString(extension)
  //let matches = []
  // can't use utility because FS_* frees and moves stuff around
  let matches = Object.keys(FS.virtual).filter(function (key) { 
    return (!extensionStr || key.endsWith(extensionStr) 
      || (extensionStr == '/' && FS.virtual[key].mode == 16895))
      // TODO: match directory 
      && (!localName || key.startsWith(localName))
      && (!wantsubs || FS.virtual[key].mode == 16895)
  })
  // return a copy!
  let listInMemory = Z_Malloc( ( matches.length + 1 ) * 4 )
  for(let i = 0; i < matches.length; i++) {
    let relativeName = matches[i]
    if(localName && relativeName.startsWith(localName)) {
      relativeName = relativeName.substring(localName.length)
    }
    if(relativeName[0] == '/')
      relativeName = relativeName.substring(1)
    //matches.push(files[i])
    HEAP32[(listInMemory + i*4)>>2] = FS_CopyString(stringToAddress(relativeName));
  }
  HEAP32[(listInMemory>>2)+matches.length] = 0
  HEAP32[numfiles >> 2] = matches.length
  // skip address-list because for-loop counts \0 with numfiles
  return listInMemory
}

var FS = {
  pointers: {},
  filePointer: 0,
  virtual: {}, // temporarily store items as they go in and out of memory
  Sys_ListFiles: Sys_ListFiles,
  Sys_FTell: Sys_FTell,
  Sys_FSeek: Sys_FSeek,
  Sys_FClose: Sys_FClose,
  Sys_FWrite: Sys_FWrite,
  Sys_FFlush: Sys_FFlush,
  Sys_FRead: Sys_FRead,
  Sys_FOpen: Sys_FOpen,
  Sys_Remove: Sys_Remove,
  Sys_Rename: Sys_Rename,
  Sys_GetFileStats: Sys_GetFileStats,
  Sys_Mkdir: Sys_Mkdir,
}
