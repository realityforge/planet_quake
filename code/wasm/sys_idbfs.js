const DB_STORE_NAME = 'FILE_DATA';

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
    return new Promise(function (resolve) { 
      let count = 0
      let interval
      interval = setInterval(function () {
        if(FS.database || count == 10) {
          clearInterval(interval)
          openDatabase(true).then(resolve)
        } else {
          count++
        }
      }, 300)
    })
  } else {
    throw new Error('no database')
  }
}


function loadEntry(cursor) {
  if(!cursor) {
    return resolve()
  }
  if(cursor.key.endsWith('default.cfg')) {
    FS.hadDefault = cursor.key
  }
  // already exists on filesystem, 
  //   it must have come with page
  if(FS.virtual[cursor.key]
    && FS.virtual[cursor.key].timestamp 
      > cursor.value.timestamp) {
    // embedded file is newer, start with that
    return cursor.continue()
  }
  FS.virtual[cursor.key] = {
    timestamp: cursor.value.timestamp,
    mode: cursor.value.mode,
    contents: cursor.value.contents
  }
  return cursor.continue()
}



function readAll() {
  let startTime = Date.now()
  FS.isSyncing = 1
  // FIX FOR "QKEY could not open" ERROR
  FS.virtual['home'] = {
    timestamp: new Date(),
    mode: FS_DIR,
  }
  if(typeof window.fs_loading != 'undefined') {
    HEAPU32[fs_loading >> 2] = FS.isSyncing
  }
  console.log('sync started at ', new Date())
  readPreFS()
  return openDatabase().then(function(db) {

    let transaction = db.transaction([DB_STORE_NAME], 'readonly')
    let objStore = transaction.objectStore(DB_STORE_NAME)
    let tranCursor = objStore.openCursor()
    return new Promise(function (resolve) {
      tranCursor.onsuccess = function (event) {
        let cursor = event.target.result
        if(!cursor) {
          return resolve()
        }
        loadEntry(cursor)
      }
      tranCursor.onerror = function (error) {
        console.error(error)
        resolve(error)
      }
    }).then(function () { 
      transaction.commit()
      let tookTime = Date.now() - startTime
      console.log('sync completed', new Date())
      console.log('sync took', 
        (tookTime > 60 * 1000 ? (Math.floor(tookTime / 1000 / 60) + ' minutes, ') : '')
        + Math.floor(tookTime / 1000) % 60 + ' seconds, '
        + (tookTime % 1000) + ' milliseconds')
      FS.isSyncing = 0
      if(typeof window.fs_loading != 'undefined') {
        HEAPU32[fs_loading >> 2] = 0
        if(FS.hadDefault) {
          HEAPU32[com_fullyInitialized >> 2] = 1
          setTimeout(function () {
            Sys_FileReady(stringToAddress('default.cfg'), stringToAddress(FS.hadDefault))
          }, 100)
        }
      }
      if(typeof window.updateFilelist != 'undefined') {
        updateFilelist()
      }
    })
  })
  .catch(function (e) {
    console.log(e)
    debugger
  })

}



function readStore(key) {
  return openDatabase()
  .then(function (db) {
    let transaction = db.transaction([DB_STORE_NAME], 'readwrite');
    let objStore = transaction.objectStore(DB_STORE_NAME);
    return new Promise(function (resolve) {
      let tranCursor = objStore.get(key)
      tranCursor.onsuccess = function (event) {
        resolve(event.target.result)
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
      transaction.oncomplete = function (event) {
        resolve(event.target.result)
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

function _base64ToArrayBuffer(base64) {
	var binary_string = window.atob(base64);
	var len = binary_string.length;
	var bytes = new Uint8Array(len);
	for (var i = 0; i < len; i++) {
			bytes[i] = binary_string.charCodeAt(i);
	}
	return bytes;
}


function readPreFS() {
	// TODO: offline download so it saves binary to IndexedDB
	if(typeof window.preFS == 'undefined') {
		return
	}

	let preloadedPaths = Object.keys(window.preFS)
	for(let i = 0; i < preloadedPaths.length; i++) {
		if(preloadedPaths[i].endsWith('_timestamp')) {
			continue
		}
		let newFiletime = window.preFS[
				preloadedPaths[i] + '_timestamp'] 
						|| new Date()
		FS.virtual[preloadedPaths[i]] = {
			timestamp: newFiletime,
			mode: FS_FILE,
			contents: _base64ToArrayBuffer(window.preFS[preloadedPaths[i]])
		}
	}

}

