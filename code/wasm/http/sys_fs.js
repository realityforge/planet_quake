
var DB_STORE_NAME = 'FILE_DATA';

function openDatabase() {
  if(!FS.open || Date.now() - FS.openTime > 1000) {
    FS.openTime = Date.now()
    FS.open = indexedDB.open('/base', 22)
    FS.promise = new Promise(function (resolve) {
      FS.resolve = resolve
    })
  }
  FS.open.onsuccess = function () {
    FS.database = FS.open.result
    //if(!Array.from(FS.database.objectStoreNames).includes(DB_STORE_NAME)) {
    //  FS.database.createObjectStore(DB_STORE_NAME)
    //}
    FS.resolve()
  }
  FS.open.onupgradeneeded = function () {
    let fileStore = FS.open.result.createObjectStore(DB_STORE_NAME)
    if (!fileStore.indexNames.contains('timestamp')) {
      fileStore.createIndex('timestamp', 'timestamp', { unique: false });
    }
  }
  FS.open.onerror = function (error) {
    console.error(error)
  }
}

function writeStore(value, key) {
  if(!FS.database) {
    openDatabase()
    new Promise(function (resolve2) {
      let oldResolve = FS.resolve
      FS.resolve = function () {
        oldResolve()
        writeStore(value, key)
        resolve2()
      }
    })
    return
  }
  let transaction = FS.database.transaction([DB_STORE_NAME], 'readwrite');
  let objStore = transaction.objectStore(DB_STORE_NAME);
  let storeValue  
  if(value === false) {
    storeValue = objStore.delete(key)
  } else {
    storeValue = objStore.put(value, key)
  }
  storeValue.onsuccess = function () {}
  transaction.oncomplete = function () {
    //FS.database.close()
    //FS.database = null
    //FS.open = null
  }
  storeValue.onerror = function (error) {
    console.error(error, value, key)
  }
  transaction.commit()
}


function Sys_Mkdir(filename) {
  let nameStr = addressToString(filename)
  if(!FS.database) {
    openDatabase()
  }
  FS.virtual[nameStr] = {
    timestamp: new Date(),
    mode: 16895,
  }
  // async to filesystem
  // does it REALLY matter if it makes it? wont it just redownload?
  writeStore(FS.virtual[nameStr], nameStr)

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
  // now we don't have to do the indexing crap here because it's built into the engine already
  let fileStr = addressToString(filename)
  let modeStr = addressToString(mode)
  let localName = fileStr
  if(localName.startsWith('/base')
    || localName.startsWith('/home'))
    localName = localName.substring('/base'.length)
  if(localName[0] == '/')
    localName = localName.substring(1)
  // TODO: check mode?
  if(typeof FS.virtual[localName] != 'undefined') {
    // open the file successfully
    FS.filePointer++
    FS.pointers[FS.filePointer] = [
      0, // seek/tell
      modeStr,
      FS.virtual[localName],
      localName
    ]
    return FS.filePointer // not zero
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
  FS.pointers[pointer] = void 0
}

function Sys_FWrite() {
  debugger
}

function Sys_FFlush() {
  debugger
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

function Sys_Rename() {
  debugger
}


function Sys_ListFiles (directory, extension, filter, numfiles, wantsubs) {
  let files = {
    'default.cfg': {
      mtime: 0,
      size: 1024,
    }
  }
  //let matches = []
  // can't use utility because FS_* frees and moves stuff around
  let matches = Object.keys(FS.virtual).filter(function (key) { 
    return (!extension || key.endsWith(extension))
      // TODO: match directory 
      && (!wantsubs || FS.virtual[key].mode == 16895)
  })
  // return a copy!
  let listInMemory = Z_Malloc( ( matches.length + 1 ) * 4 )
  for(let i = 0; i < matches.length; i++) {
    //matches.push(files[i])
    HEAP32[(listInMemory + i*4)>>2] = FS_CopyString(stringToAddress(matches[i]));
  }
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
