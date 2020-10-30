var LibrarySysCommon = {
	$SYSC__deps: ['$Browser', '$FS', '$PATH', '$SYS', 'Com_Printf', 'Com_Outside_Error'],
	$SYSC: {
		// next free handle to use for a loaded dso.
		// (handle=0 is avoided as it means "error" in dlopen)
		nextHandle: 1,

		loadedLibs: {         // handle -> dso [refcount, name, module, global]
			// program itself
			// XXX uglifyjs fails on "[-1]: {"
			'-1': {
				refcount: Infinity,   // = nodelete
				name:     '__self__',
				global:   true
			}
		},

		loadedLibNames: {     // name   -> handle
			// program itself
			'__self__': -1
		},
		varStr: 0,
		oldDLURL: null,
		newDLURL: null,
		returnURL: null,
		Cvar_VariableString: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return UTF8ToString(_Cvar_VariableString(SYSC.varStr))
		},
		Cvar_VariableIntegerValue: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_VariableIntegerValue(SYSC.varStr)
		},
		Cvar_SetValue: function (str, value) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_SetValue(SYSC.varStr, value)
		},
		Cvar_Get: function (str) {
			intArrayFromString(str).forEach(function (c, i) { HEAP8[(SYSC.varStr+i)] = c })
			HEAP8[(SYSC.varStr+str.length)] = 0
			return _Cvar_Get(SYSC.varStr, SYSC.varStr+str.length, 0)
		},
		Print: function (str) {
			if(!Array.isArray(str)) str = [str]
			//str = str.map(function (s) {
			//	return allocate(intArrayFromString(s + '\0'), 'i8', ALLOC_STACK);
			//})
			console.log.apply(null, str)
		},
		Error: function (level, errMsg) {
			if (level === 'fatal') {
				level = 0
			} else if (level === 'drop') {
				level = 1
			} else if (level === 'serverdisconnect') {
				level = 2
			} else if (level === 'disconnect') {
				level = 3
			} else if (level === 'need_cd') {
				level = 4
			} else {
				level = 0
			}

			Browser.safeCallback(_Com_Outside_Error)(level, 
				allocate(intArrayFromString(errMsg + '\n'), 'i8', ALLOC_STACK))
			// drop current stack frame and bubble out
			throw new Error(errMsg)
		},
		ProxyCallback: function () {
			Browser.safeCallback(function () {
				try {
					_Com_Frame_Proxy();
				} catch (e) {
					if (e instanceof ExitStatus) {
						return;
					}
					// TODO should we try and call back in using __Error?
					var message = _S_Malloc(e.message.length + 1);
					stringToUTF8(e.message, message, e.message.length+1);
					_Sys_ErrorDialog(message);
					throw e;
				}
			})()
		},
		mkdirp: function (p) {
			try {
				FS.mkdir(p, 16895);
			} catch (e) {
				// make the subdirectory and then retry
				if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
					SYSC.mkdirp(PATH.dirname(p));
					SYSC.mkdirp(p);
					return;
				}

				// if we got any other error, let's see if the directory already exists
				var stat;
				try {
					stat = FS.stat(p);
				}
				catch (e) {
					SYSC.Error('fatal', e.message || e);
					return;
				}

				if (!FS.isDir(stat.mode)) {
					SYSC.Error('fatal', e.message);
				}
			}
		},
		
		// fetchBinary fetches binaray data @ url. (async)
		fetchBinary: function (url) {
		  return fetch(url, { credentials: 'same-origin' }).then(function(response) {
		    if (!response['ok']) {
		      throw "failed to load binary file at '" + url + "'";
		    }
		    return response['arrayBuffer']();
		  }).then(function(buffer) {
		    return new Uint8Array(buffer);
		  });
		},
#if WASM == 0 || WASM2JS
		/** @param {Object=} module */
		getFunctionTables: function (module) {
		  if (!module) module = Module;
		  var tables = {};
		  for (var t in module) {
		    if (/^FUNCTION_TABLE_.*/.test(t)) {
		      var table = module[t];
		      if (typeof table === 'object') tables[t.substr('FUNCTION_TABLE_'.length)] = table;
		    }
		  }
		  return tables;
		},

		/** @param {Object=} module */
		alignFunctionTables: function (module) {
		  var tables = SYSC.getFunctionTables(module);
		  var maxx = 0;
		  for (var sig in tables) {
		    maxx = Math.max(maxx, tables[sig].length);
		  }
		  assert(maxx >= 0);
		  for (var sig in tables) {
		    var table = tables[sig];
		    while (table.length < maxx) table.push(0);
		  }
		  return maxx;
		},
#endif // WASM == 0

		// loadDynamicLibrary loads dynamic library @ lib URL / path and returns handle for loaded DSO.
		//
		// Several flags affect the loading:
		//
		// - if flags.global=true, symbols from the loaded library are merged into global
		//   process namespace. Flags.global is thus similar to RTLD_GLOBAL in ELF.
		//
		// - if flags.nodelete=true, the library will be never unloaded. Flags.nodelete
		//   is thus similar to RTLD_NODELETE in ELF.
		//
		// - if flags.loadAsync=true, the loading is performed asynchronously and
		//   loadDynamicLibrary returns corresponding promise.
		//
		// - if flags.fs is provided, it is used as FS-like interface to load library data.
		//   By default, when flags.fs=undefined, native loading capabilities of the
		//   environment are used.
		//
		// If a library was already loaded, it is not loaded a second time. However
		// flags.global and flags.nodelete are handled every time a load request is made.
		// Once a library becomes "global" or "nodelete", it cannot be removed or unloaded.
		loadDynamicLibrary: function (lib, flags) {
		  // when loadDynamicLibrary did not have flags, libraries were loaded globally & permanently
		  flags = flags || {global: true, nodelete: true}

		  var handle = SYSC.loadedLibNames[lib];
		  var dso;
		  if (handle) {
		    // the library is being loaded or has been loaded already.
		    //
		    // however it could be previously loaded only locally and if we get
		    // load request with global=true we have to make it globally visible now.
		    dso = SYSC.loadedLibs[handle];
		    if (flags.global && !dso.global) {
		      dso.global = true;
		      if (dso.module !== 'loading') {
		        // ^^^ if module is 'loading' - symbols merging will be eventually done by the loader.
		        mergeLibSymbols(dso.module)
		      }
		    }
		    // same for "nodelete"
		    if (flags.nodelete && dso.refcount !== Infinity) {
		      dso.refcount = Infinity;
		    }
		    dso.refcount++
		    return flags.loadAsync ? Promise.resolve(handle) : handle;
		  }

		  // allocate new DSO & handle
		  handle = SYSC.nextHandle++;
		  dso = {
		    refcount: flags.nodelete ? Infinity : 1,
		    name:     lib,
		    module:   'loading',
		    global:   flags.global,
		  };
		  SYSC.loadedLibNames[lib] = handle;
		  SYSC.loadedLibs[handle] = dso;

		  // libData <- libFile
		  function loadLibData(libFile) {
#if WASM && !WASM2JS
		    // for wasm, we can use fetch for async, but for fs mode we can only imitate it
		    if (flags.fs) {
		      var libData = flags.fs.readFile(libFile, {encoding: 'binary'});
		      if (!(libData instanceof Uint8Array)) {
		        libData = new Uint8Array(lib_data);
		      }
		      return flags.loadAsync ? Promise.resolve(libData) : libData;
		    }

		    if (flags.loadAsync) {
					try {
						FS.lookupPath(libFile)
						return Promise.resolve(FS.readFile(libFile, {encoding: 'binary'}))
					} catch (e) {
						if (!(e instanceof FS.ErrnoError) || e.errno !== ERRNO_CODES.ENOENT) {
							SYSC.Error('fatal', e.message)
						}
					}
					return SYSC.fetchBinary(libFile);
		    }
		    // load the binary synchronously
		    return FS.readFile(libFile, {encoding: 'binary'});
#else
		    // for js we only imitate async for both native & fs modes.
		    var libData;
		    if (flags.fs) {
		      libData = flags.fs.readFile(libFile, {encoding: 'utf8'});
		    } else {
		      libData = read_(libFile);
		    }
		    return flags.loadAsync ? Promise.resolve(libData) : libData;
#endif
		  }

		  // libModule <- libData
		  function createLibModule(libData) {
#if WASM && !WASM2JS
		    return SYSC.loadWebAssemblyModule(libData, flags)
#else
		    var libModule = /**@type{function(...)}*/(eval(
					'(function (wasmTable, Module) {\n' + libData + '\nreturn Module})'))(
		      SYSC.alignFunctionTables(),
		      Module
		    );
		    // load dynamic libraries that this js lib depends on
		    // (wasm loads needed libraries _before_ lib in its own codepath)
		    if (libModule.dynamicLibraries) {
		      if (flags.loadAsync) {
		        return Promise.all(libModule.dynamicLibraries.map(function(dynNeeded) {
		          return loadDynamicLibrary(dynNeeded, flags);
		        })).then(function() {
		          return libModule;
		        });
		      }

		      libModule.dynamicLibraries.forEach(function(dynNeeded) {
		        loadDynamicLibrary(dynNeeded, flags);
		      });
		    }
		    return libModule;
#endif
			}

		  // libModule <- lib
		  function getLibModule() {
		    // lookup preloaded cache first
		    if (Module['preloadedWasm'] !== undefined &&
		        Module['preloadedWasm'][lib] !== undefined) {
		      var libModule = Module['preloadedWasm'][lib];
		      return flags.loadAsync ? Promise.resolve(libModule) : libModule;
		    }

		    // module not preloaded - load lib data and create new module from it
		    if (flags.loadAsync) {
		      return loadLibData(lib).then(function(libData) {
		        return createLibModule(libData);
		      });
		    }

		    return createLibModule(loadLibData(lib));
		  }

		  // Module.symbols <- libModule.symbols (flags.global handler)
		  function mergeLibSymbols(libModule) {
		    // add symbols into global namespace TODO: weak linking etc.
		    for (var sym in libModule) {
		      if (!libModule.hasOwnProperty(sym)) {
		        continue;
		      }

		      // When RTLD_GLOBAL is enable, the symbols defined by this shared object will be made
		      // available for symbol resolution of subsequently loaded shared objects.
		      //
		      // We should copy the symbols (which include methods and variables) from SIDE_MODULE to MAIN_MODULE.

		      var module_sym = sym;
#if WASM_BACKEND
		      module_sym = '_' + sym;
#else
		      // Module of SIDE_MODULE has not only the symbols (which should be copied)
		      // but also others (print*, asmGlobal*, FUNCTION_TABLE_**, NAMED_GLOBALS, and so on).
		      //
		      // When the symbol (which should be copied) is method, Module.* 's type becomes function.
		      // When the symbol (which should be copied) is variable, Module.* 's type becomes number.
		      // Except for the symbol prefix (_), there is no difference in the symbols (which should be copied) and others.
		      // So this just copies over compiled symbols (which start with _).
		      if (sym[0] !== '_') {
		        continue;
		      }
#endif

		      if (!Module.hasOwnProperty(module_sym)) {
		        Module[module_sym] = libModule[sym];
		      }
		    }
		  }

		  // module for lib is loaded - update the dso & global namespace
		  function moduleLoaded(libModule) {
		    if (dso.global) {
		      mergeLibSymbols(libModule);
		    }
		    dso.module = libModule;
		  }

		  if (flags.loadAsync) {
		    return getLibModule().then(function(libModule) {
		      moduleLoaded(libModule);
		      return handle;
		    })
		  }

		  moduleLoaded(getLibModule());
		  return handle;
		},

#if WASM && !WASM2JS
		// Applies relocations to exported things.
		relocateExports: function (exports, memoryBase, tableBase, moduleLocal) {
		  var relocated = {};

		  for (var e in exports) {
		    var value = exports[e];
		    if (typeof value === 'object') {
		      // a breaking change in the wasm spec, globals are now objects
		      // https://github.com/WebAssembly/mutable-global/issues/1
		      value = value.value;
		    }
		    if (typeof value === 'number') {
		      // relocate it - modules export the absolute value, they can't relocate before they export
#if EMULATE_FUNCTION_POINTER_CASTS
		      // it may be a function pointer
		      if (e.substr(0, 3) == 'fp$' && typeof exports[e.substr(3)] === 'function') {
		        value += tableBase;
		      } else {
#endif
		        value += memoryBase;
#if EMULATE_FUNCTION_POINTER_CASTS
		      }
#endif
		    }
		    relocated[e] = value;
		    if (moduleLocal) {
#if WASM_BACKEND
		      moduleLocal['_' + e] = value;
#else
		      moduleLocal[e] = value;
#endif
		    }
		  }
		  return relocated;
		},

		// Loads a side module from binary data
		loadWebAssemblyModule: function (binary, flags) {
		  var int32View = new Uint32Array(new Uint8Array(binary.subarray(0, 24)).buffer);
		  assert(int32View[0] == 0x6d736100, 'need to see wasm magic number'); // \0asm
		  // we should see the dylink section right after the magic number and wasm version
		  assert(binary[8] === 0, 'need the dylink section to be first')
		  var next = 9;
		  function getLEB() {
		    var ret = 0;
		    var mul = 1;
		    while (1) {
		      var byte = binary[next++];
		      ret += ((byte & 0x7f) * mul);
		      mul *= 0x80;
		      if (!(byte & 0x80)) break;
		    }
		    return ret;
		  }
		  var sectionSize = getLEB();
		  assert(binary[next] === 6);                 next++; // size of "dylink" string
		  assert(binary[next] === 'd'.charCodeAt(0)); next++;
		  assert(binary[next] === 'y'.charCodeAt(0)); next++;
		  assert(binary[next] === 'l'.charCodeAt(0)); next++;
		  assert(binary[next] === 'i'.charCodeAt(0)); next++;
		  assert(binary[next] === 'n'.charCodeAt(0)); next++;
		  assert(binary[next] === 'k'.charCodeAt(0)); next++;
		  var memorySize = getLEB();
		  var memoryAlign = getLEB();
		  var tableSize = getLEB();
		  var tableAlign = getLEB();

		  // shared libraries this module needs. We need to load them first, so that
		  // current module could resolve its imports. (see tools/shared.py
		  // WebAssembly.make_shared_library() for "dylink" section extension format)
		  var neededDynlibsCount = getLEB();
		  var neededDynlibs = [];
		  for (var i = 0; i < neededDynlibsCount; ++i) {
		    var nameLen = getLEB();
		    var nameUTF8 = binary.subarray(next, next + nameLen);
		    next += nameLen;
		    var name = UTF8ArrayToString(nameUTF8, 0);
		    neededDynlibs.push(name);
		  }

		  // loadModule loads the wasm module after all its dependencies have been loaded.
		  // can be called both sync/async.
		  function loadModule() {
		    // alignments are powers of 2
		    memoryAlign = Math.pow(2, memoryAlign);
		    tableAlign = Math.pow(2, tableAlign);
		    // finalize alignments and verify them
		    memoryAlign = Math.max(memoryAlign, STACK_ALIGN); // we at least need stack alignment
		    // prepare memory
		    var memoryBase = alignMemory(getMemory(memorySize + memoryAlign), memoryAlign); // TODO: add to cleanups
		    // prepare env imports
		    var env = asmLibraryArg;
		    // TODO: use only __memory_base and __table_base, need to update asm.js backend
		    var table = wasmTable;
		    var tableBase = table.length;
		    var originalTable = table;
		    table.grow(tableSize);
		    assert(table === originalTable);
		    // zero-initialize memory and table
		    // The static area consists of explicitly initialized data, followed by zero-initialized data.
		    // The latter may need zeroing out if the MAIN_MODULE has already used this memory area before
		    // dlopen'ing the SIDE_MODULE.  Since we don't know the size of the explicitly initialized data
		    // here, we just zero the whole thing, which is suboptimal, but should at least resolve bugs
		    // from uninitialized memory.
		    for (var i = memoryBase; i < memoryBase + memorySize; i++) {
		      HEAP8[i] = 0;
		    }
		    for (var i = tableBase; i < tableBase + tableSize; i++) {
		      table.set(i, null);
		    }

		    // We resolve symbols against the global Module but failing that also
		    // against the local symbols exported a side module.  This is because
		    // a) Module sometime need to import their own symbols
		    // b) Symbols from loaded modules are not always added to the global Module.
		    var moduleLocal = {};

		    var resolveSymbol = function(sym, type, legalized) {
#if WASM_BIGINT
		      assert(!legalized);
#else
		      if (legalized) {
		        sym = 'orig$' + sym;
		      }
#endif

		      var resolved = Module["asm"][sym];
		      if (!resolved) {
		        resolved = Module['_' + sym]
		      }
					if (!resolved) {
						resolved = moduleLocal['_' + sym]
					}
					if(!resolved) {
						resolved = window['_' + sym]
					}
					if(!resolved && WebGLRenderingContext) {
						resolved = WebGLRenderingContext.prototype[sym[0].toLowerCase() + sym.substr(1)]
					}
					if(!resolved) {
						resolved = window['Math_' + sym]
					}
		      return resolved;
		    }

		    // copy currently exported symbols so the new module can import them
		    for (var x in Module) {
		      if (!(x in env)) {
		        env[x] = Module[x];
		      }
		    }

		    // TODO kill ↓↓↓ (except "symbols local to this module", it will likely be
		    // not needed if we require that if A wants symbols from B it has to link
		    // to B explicitly: similarly to -Wl,--no-undefined)
		    //
		    // wasm dynamic libraries are pure wasm, so they cannot assist in
		    // their own loading. When side module A wants to import something
		    // provided by a side module B that is loaded later, we need to
		    // add a layer of indirection, but worse, we can't even tell what
		    // to add the indirection for, without inspecting what A's imports
		    // are. To do that here, we use a JS proxy (another option would
		    // be to inspect the binary directly).
		    var proxyHandler = {
		      'get': function(obj, prop) {
		        // symbols that should be local to this module
		        switch (prop) {
		          case '__memory_base':
		          case 'gb':
		            return memoryBase;
		          case '__table_base':
		          case 'fb':
		            return tableBase;
		        }

		        if (prop in obj) {
		          return obj[prop]; // already present
		        }
		        if (prop.startsWith('g$')) {
		          // a global. the g$ function returns the global address.
		          var name = prop.substr(2); // without g$ prefix
		          return obj[prop] = function() {
		            return resolveSymbol(name, 'global');
		          };
		        }
		        if (prop.startsWith('fp$')) {
		          // the fp$ function returns the address (table index) of the function
		          var parts = prop.split('$');
		          assert(parts.length == 3)
		          var name = parts[1];
		          var sig = parts[2];
#if WASM_BIGINT
		          var legalized = false;
#else
		          var legalized = sig.indexOf('j') >= 0; // check for i64s
#endif
		          var fp = 0;
		          return obj[prop] = function() {
		            if (!fp) {
		              var f = resolveSymbol(name, 'function', legalized)
									if(!f) {
										f = resolveSymbol('emscripten_' + name, 'function', legalized)
									}
									if(!f) return 0;
		              fp = addFunction(f, sig);
		            }
		            return fp;
		          };
		        }
		        if (prop.startsWith('invoke_')) {
		          // A missing invoke, i.e., an invoke for a function type
		          // present in the dynamic library but not in the main JS,
		          // and the dynamic library cannot provide JS for it. Use
		          // the generic "X" invoke for it.
		          return obj[prop] = invoke_X;
		        }
		        // otherwise this is regular function import - call it indirectly
		        return obj[prop] = function() {
							var func = resolveSymbol(prop, 'function')
							if(!func) {
								throw new Error('Couldn\'t load function ' + prop)
							}
		          return func.apply(null, arguments)
		        };
		      }
		    };
		    var proxy = new Proxy(env, proxyHandler);
		    var info = {
		      global: {
		        'NaN': NaN,
		        'Infinity': Infinity,
		      },
		      'global.Math': Math,
		      env: proxy,
		      {{{ WASI_MODULE_NAME }}}: proxy,
#if !WASM_BACKEND
		      'asm2wasm': asm2wasmImports
#endif
		    };

		    function postInstantiation(instance, moduleLocal) {
		      var exports = SYSC.relocateExports(instance.exports, memoryBase, tableBase, moduleLocal);
		      // initialize the module
		      var init = exports['__post_instantiate'];
		      if (init) {
		        if (runtimeInitialized) {
		          init();
		        } else {
		          // we aren't ready to run compiled code yet
		          __ATINIT__.push(init);
		        }
		      }
		      return exports;
		    }

		    if (flags.loadAsync) {
		      return WebAssembly.instantiate(binary, info).then(function(result) {
		        return postInstantiation(result.instance, moduleLocal);
		      });
		    } else {
		      var instance = new WebAssembly.Instance(new WebAssembly.Module(binary), info);
		      return postInstantiation(instance, moduleLocal);
		    }
		  }

		  // now load needed libraries and the module itself.
		  if (flags.loadAsync) {
		    return Promise.all(neededDynlibs.map(function(dynNeeded) {
		      return loadDynamicLibrary(dynNeeded, flags);
		    })).then(function() {
		      return loadModule();
		    });
		  }

		  neededDynlibs.forEach(function(dynNeeded) {
		    loadDynamicLibrary(dynNeeded, flags);
		  });
		  return loadModule();
		},
#endif
	},
	Sys_DefaultHomePath: function () {
		return 0;
	},
	Sys_RandomBytes: function (string, len) {
		if(typeof crypto != 'undefined') {
			crypto.getRandomValues(HEAP8.subarray(string, string+len))
		} else {
			for(var i = 0; i < len; i++) {
				HEAP8[string] = Math.random() * 255
			}
		}
		return true;
	},
	Sys_LowPhysicalMemory: function () {
		return false;
	},
	Sys_Basename__deps: ['$PATH'],
	Sys_Basename: function (path) {
		path = PATH.basename(UTF8ToString(path));
		return allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
	},
	Sys_DllExtension__deps: ['$PATH'],
	Sys_DllExtension: function (path) {
		return PATH.extname(UTF8ToString(path)) == '.wasm';
	},
	Sys_Dirname__deps: ['$PATH'],
	Sys_Dirname: function (path) {
		path = PATH.dirname(UTF8ToString(path));
		return allocate(intArrayFromString(path), 'i8', ALLOC_STACK);
	},
	Sys_Mkfifo: function (path) {
		return 0;
	},
	Sys_FreeFileList__deps: ['Z_Free'],
	Sys_FreeFileList: function (list) {
		if (!list) {
			return;
		}

		var ptr;
		for (var i = 0; (ptr = {{{ makeGetValue('list', 'i*4', 'i32') }}}); i++) {
			_Z_Free(ptr);
		}

		_Z_Free(list);
	},
	Sys_Mkdir: function (directory) {
		directory = UTF8ToString(directory);
		try {
			FS.mkdir(directory, 16895);
		} catch (e) {
			if (!(e instanceof FS.ErrnoError)) {
				SYSC.Error('drop', e.message);
			}
			return e.errno === ERRNO_CODES.EEXIST;
		}
		return true;
	},
	Sys_Cwd: function () {
		var cwd = allocate(intArrayFromString(FS.cwd()), 'i8', ALLOC_STACK);
		return cwd;
	},
	Sys_Sleep: function () {
	},
	Sys_SetEnv: function (name, value) {
		name = UTF8ToString(name);
		value = UTF8ToString(value);
	},
	Sys_PID: function () {
		return 0;
	},
	Sys_PIDIsRunning: function (pid) {
		return 1;
	},
	Sys_LoadLibrary__deps: [],
	Sys_LoadLibrary: function (name) {
		var file = UTF8ToString(name)
		file = file.replace(/\/?base\//ig, '')
		SYSN.DownloadAsset(file, null, function (err, data) {
			if(!err) {
				try {
					FS.writeFile(PATH.join(SYSF.fs_basepath, file), new Uint8Array(data), {
						encoding: 'binary', flags: 'w', canOwn: true })
				} catch (e) {
					console.error(e)
				}
			}
						
			return SYSC.loadDynamicLibrary(file, {
				loadAsync: true, global: true, nodelete: true
			})
			.catch(function(e) { err = e })
			.then(Browser.safeCallback(function (handle) {
				_CL_InitRef_After_Load_Callback(err ? 0 : handle)
			}))
		})
		return 0
	},
	Sys_LoadFunction: function(handle, symbol) {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = UTF8ToString(symbol);

    if (!SYSC.loadedLibs[handle]) {
      return 0;
    }

    var lib = SYSC.loadedLibs[handle];
    var isMainModule = lib.module == Module;

    var mangled = '_' + symbol;
    var modSymbol = mangled;
#if WASM_BACKEND
    if (!isMainModule) {
      modSymbol = symbol;
    }
#endif

    if (!lib.module.hasOwnProperty(modSymbol)) {
			if(lib.module.hasOwnProperty(mangled)) {
				modSymbol = mangled;
			} else
      	return 0;
    }

    var result = lib.module[modSymbol];
#if WASM && !WASM2JS
    // Attempt to get the real "unwrapped" symbol so we have more chance of
    // getting wasm function which can be added to a table.
    if (isMainModule) {
#if WASM_BACKEND
      var asmSymbol = symbol;
#else
      var asmSymbol = mangled;
#endif
      if (lib.module["asm"][asmSymbol]) {
        result = lib.module["asm"][asmSymbol];
      }
    }
#endif
    if (typeof result !== 'function')
      return result;

#if WASM && EMULATE_FUNCTION_POINTER_CASTS && !WASM2JS
    // for wasm with emulated function pointers, the i64 ABI is used for all
    // function calls, so we can't just call addFunction on something JS
    // can call (which does not use that ABI), as the function pointer would
    // not be usable from wasm. instead, the wasm has exported function pointers
    // for everything we need, with prefix fp$, use those
    result = lib.module['fp$' + symbol];
    if (typeof result === 'object') {
      // a breaking change in the wasm spec, globals are now objects
      // https://github.com/WebAssembly/mutable-global/issues/1
      result = result.value;
    }
    return result;
#else // WASM && EMULATE_FUNCTION_POINTER_CASTS

#if WASM && !WASM2JS
    // Insert the function into the wasm table.  Since we know the function
    // comes directly from the loaded wasm module we can insert it directly
    // into the table, avoiding any JS interaction.
    return addFunctionWasm(result);
#else
    // convert the exported function into a function pointer using our generic
    // JS mechanism.
    return addFunction(result);
#endif // WASM
#endif // WASM && EMULATE_FUNCTION_POINTER_CASTS
	},
	Sys_UnloadLibrary: function () {
		
	},
	Sys_SetAffinityMask: function () {
		throw new Error('TODO: support using background workers or not')
	},
	Sys_ShowConsole: function () {
		// not implemented
	},
	Sys_GetFileStats: function(filename, size, mtime, ctime) {
		try {
			var stat = FS.stat(UTF8ToString(name))
			{{{ makeSetValue('size', '0', 'stat.size', 'i32') }}}
			{{{ makeSetValue('mtime', '0', 'stat.mtime', 'i32') }}}
			{{{ makeSetValue('ctime', '0', 'stat.ctime', 'i32') }}}
			return true
		} catch (e) {
			if ((e instanceof FS.ErrnoError) && e.errno === ERRNO_CODES.ENOENT) {
				{{{ makeSetValue('size', '0', '0', 'i32') }}}
				{{{ makeSetValue('mtime', '0', '0', 'i32') }}}
				{{{ makeSetValue('ctime', '0', '0', 'i32') }}}
				return false
			}
			throw e
		}
	},
	Sys_Debug: function () { debugger },
};

autoAddDeps(LibrarySysCommon, '$SYSC');
mergeInto(LibraryManager.library, LibrarySysCommon);
