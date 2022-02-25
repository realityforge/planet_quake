// this bastard only connects the functions together for emscripten's screwy non-standard, 
//   obviously written by people who only know how to write python's, module system.

let LibrarySys = {
	$SYS__deps: [],
	$SYS: {
    openDatabase: openDatabase,
    writeStore: writeStore,
    DB_STORE_NAME: DB_STORE_NAME,
    stringToAddress: stringToAddress,
    addressToString: addressToString,
    stringsToMemory: stringsToMemory,
    controller: null,
    pointers: {},
    filePointer: 0,
    virtual: {}, // temporarily store items as they go in and out of memory
  }
}

// assign everything to env because this bullshit don't work
Object.assign(LibrarySys, Q3e, SYS, FS, NET, INPUT)

autoAddDeps(LibrarySys, '$SYS')
mergeInto(LibraryManager.library, LibrarySys)
