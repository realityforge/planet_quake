/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(Module) { ..generated code.. }
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = typeof huffman !== 'undefined' ? huffman : {};

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// {{PRE_JSES}}

// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = {};
var key;
for (key in Module) {
  if (Module.hasOwnProperty(key)) {
    moduleOverrides[key] = Module[key];
  }
}

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = function(status, toThrow) {
  throw toThrow;
};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

var ENVIRONMENT_IS_WEB = false;
var ENVIRONMENT_IS_WORKER = false;
var ENVIRONMENT_IS_NODE = false;
var ENVIRONMENT_IS_SHELL = false;
ENVIRONMENT_IS_WEB = typeof window === 'object';
ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;




// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var read_,
    readAsync,
    readBinary,
    setWindowTitle;

var nodeFS;
var nodePath;

if (ENVIRONMENT_IS_NODE) {
  if (ENVIRONMENT_IS_WORKER) {
    scriptDirectory = require('path').dirname(scriptDirectory) + '/';
  } else {
    scriptDirectory = __dirname + '/';
  }


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

  read_ = function shell_read(filename, binary) {
    var ret = tryParseAsDataURI(filename);
    if (ret) {
      return binary ? ret : ret.toString();
    }
    if (!nodeFS) nodeFS = require('fs');
    if (!nodePath) nodePath = require('path');
    filename = nodePath['normalize'](filename);
    return nodeFS['readFileSync'](filename, binary ? null : 'utf8');
  };

  readBinary = function readBinary(filename) {
    var ret = read_(filename, true);
    if (!ret.buffer) {
      ret = new Uint8Array(ret);
    }
    assert(ret.buffer);
    return ret;
  };




  if (process['argv'].length > 1) {
    thisProgram = process['argv'][1].replace(/\\/g, '/');
  }

  arguments_ = process['argv'].slice(2);

  if (typeof module !== 'undefined') {
    module['exports'] = Module;
  }

  quit_ = function(status) {
    process['exit'](status);
  };

  Module['inspect'] = function () { return '[Emscripten Module object]'; };



} else
if (ENVIRONMENT_IS_SHELL) {


  if (typeof read != 'undefined') {
    read_ = function shell_read(f) {
      var data = tryParseAsDataURI(f);
      if (data) {
        return intArrayToString(data);
      }
      return read(f);
    };
  }

  readBinary = function readBinary(f) {
    var data;
    data = tryParseAsDataURI(f);
    if (data) {
      return data;
    }
    if (typeof readbuffer === 'function') {
      return new Uint8Array(readbuffer(f));
    }
    data = read(f, 'binary');
    assert(typeof data === 'object');
    return data;
  };

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

  if (typeof quit === 'function') {
    quit_ = function(status) {
      quit(status);
    };
  }

  if (typeof print !== 'undefined') {
    // Prefer to use print/printErr where they exist, as they usually work better.
    if (typeof console === 'undefined') console = /** @type{!Console} */({});
    console.log = /** @type{!function(this:Console, ...*): undefined} */ (print);
    console.warn = console.error = /** @type{!function(this:Console, ...*): undefined} */ (typeof printErr !== 'undefined' ? printErr : print);
  }


} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  if (scriptDirectory.indexOf('blob:') !== 0) {
    scriptDirectory = scriptDirectory.substr(0, scriptDirectory.lastIndexOf('/')+1);
  } else {
    scriptDirectory = '';
  }


  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
  {


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

  read_ = function shell_read(url) {
    try {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.send(null);
      return xhr.responseText;
    } catch (err) {
      var data = tryParseAsDataURI(url);
      if (data) {
        return intArrayToString(data);
      }
      throw err;
    }
  };

  if (ENVIRONMENT_IS_WORKER) {
    readBinary = function readBinary(url) {
      try {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, false);
        xhr.responseType = 'arraybuffer';
        xhr.send(null);
        return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
      } catch (err) {
        var data = tryParseAsDataURI(url);
        if (data) {
          return data;
        }
        throw err;
      }
    };
  }

  readAsync = function readAsync(url, onload, onerror) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onload = function xhr_onload() {
      if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
        onload(xhr.response);
        return;
      }
      var data = tryParseAsDataURI(url);
      if (data) {
        onload(data.buffer);
        return;
      }
      onerror();
    };
    xhr.onerror = onerror;
    xhr.send(null);
  };




  }

  setWindowTitle = function(title) { document.title = title };
} else
{
}


// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
var out = Module['print'] || console.log.bind(console);
var err = Module['printErr'] || console.warn.bind(console);

// Merge back in the overrides
for (key in moduleOverrides) {
  if (moduleOverrides.hasOwnProperty(key)) {
    Module[key] = moduleOverrides[key];
  }
}
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used e.g. in memoryInitializerRequest, which is a large typed array.
moduleOverrides = null;

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.
if (Module['arguments']) arguments_ = Module['arguments'];
if (Module['thisProgram']) thisProgram = Module['thisProgram'];
if (Module['quit']) quit_ = Module['quit'];

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message



/**
 * @license
 * Copyright 2017 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// {{PREAMBLE_ADDITIONS}}

var STACK_ALIGN = 16;


function dynamicAlloc(size) {
  var ret = HEAP32[DYNAMICTOP_PTR>>2];
  var end = (ret + size + 15) & -16;
  HEAP32[DYNAMICTOP_PTR>>2] = end;
  return ret;
}

function alignMemory(size, factor) {
  if (!factor) factor = STACK_ALIGN; // stack alignment (16-byte) by default
  return Math.ceil(size / factor) * factor;
}

function getNativeTypeSize(type) {
  switch (type) {
    case 'i1': case 'i8': return 1;
    case 'i16': return 2;
    case 'i32': return 4;
    case 'i64': return 8;
    case 'float': return 4;
    case 'double': return 8;
    default: {
      if (type[type.length-1] === '*') {
        return 4; // A pointer
      } else if (type[0] === 'i') {
        var bits = Number(type.substr(1));
        assert(bits % 8 === 0, 'getNativeTypeSize invalid bits ' + bits + ', type ' + type);
        return bits / 8;
      } else {
        return 0;
      }
    }
  }
}

function warnOnce(text) {
  if (!warnOnce.shown) warnOnce.shown = {};
  if (!warnOnce.shown[text]) {
    warnOnce.shown[text] = 1;
    err(text);
  }
}





/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */


// Wraps a JS function as a wasm function with a given signature.
function convertJsFunctionToWasm(func, sig) {
  return func;
}

var freeTableIndexes = [];

// Weak map of functions in the table to their indexes, created on first use.
var functionsInTableMap;

// Add a wasm function to the table.
function addFunctionWasm(func, sig) {
  var table = wasmTable;

  // Check if the function is already in the table, to ensure each function
  // gets a unique index. First, create the map if this is the first use.
  if (!functionsInTableMap) {
    functionsInTableMap = new WeakMap();
    for (var i = 0; i < table.length; i++) {
      var item = table.get(i);
      // Ignore null values.
      if (item) {
        functionsInTableMap.set(item, i);
      }
    }
  }
  if (functionsInTableMap.has(func)) {
    return functionsInTableMap.get(func);
  }

  // It's not in the table, add it now.
  var ret;
  // Reuse a free index if there is one, otherwise grow.
  if (freeTableIndexes.length) {
    ret = freeTableIndexes.pop();
  } else {
    ret = table.length;
    // Grow the table
    try {
      table.grow(1);
    } catch (err) {
      if (!(err instanceof RangeError)) {
        throw err;
      }
      throw 'Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.';
    }
  }

  // Set the new value.
  try {
    // Attempting to call this with JS function will cause of table.set() to fail
    table.set(ret, func);
  } catch (err) {
    if (!(err instanceof TypeError)) {
      throw err;
    }
    assert(typeof sig !== 'undefined', 'Missing signature argument to addFunction');
    var wrapped = convertJsFunctionToWasm(func, sig);
    table.set(ret, wrapped);
  }

  functionsInTableMap[func] = ret;

  return ret;
}

function removeFunctionWasm(index) {
  functionsInTableMap.delete(wasmTable.get(index));
  freeTableIndexes.push(index);
}

// 'sig' parameter is required for the llvm backend but only when func is not
// already a WebAssembly function.
function addFunction(func, sig) {

  return addFunctionWasm(func, sig);
}

function removeFunction(index) {
  removeFunctionWasm(index);
}



var funcWrappers = {};

function getFuncWrapper(func, sig) {
  if (!func) return; // on null pointer, return undefined
  assert(sig);
  if (!funcWrappers[sig]) {
    funcWrappers[sig] = {};
  }
  var sigCache = funcWrappers[sig];
  if (!sigCache[func]) {
    // optimize away arguments usage in common cases
    if (sig.length === 1) {
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func);
      };
    } else if (sig.length === 2) {
      sigCache[func] = function dynCall_wrapper(arg) {
        return dynCall(sig, func, [arg]);
      };
    } else {
      // general case
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func, Array.prototype.slice.call(arguments));
      };
    }
  }
  return sigCache[func];
}


/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */




function makeBigInt(low, high, unsigned) {
  return unsigned ? ((+((low>>>0)))+((+((high>>>0)))*4294967296.0)) : ((+((low>>>0)))+((+((high|0)))*4294967296.0));
}

/** @param {Array=} args */
function dynCall(sig, ptr, args) {
  if (args && args.length) {
    return Module['dynCall_' + sig].apply(null, [ptr].concat(args));
  } else {
    return Module['dynCall_' + sig].call(null, ptr);
  }
}

var tempRet0 = 0;

var setTempRet0 = function(value) {
  tempRet0 = value;
};

var getTempRet0 = function() {
  return tempRet0;
};


// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
var GLOBAL_BASE = 1024;



/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html


var wasmBinary;if (Module['wasmBinary']) wasmBinary = Module['wasmBinary'];
var noExitRuntime;if (Module['noExitRuntime']) noExitRuntime = Module['noExitRuntime'];


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.


// Emit "var WebAssembly" if definitely using wasm2js. Otherwise, in MAYBE_WASM2JS
// mode, we can't use a "var" since it would prevent normal wasm from working.
/** @suppress{const} */
var
WebAssembly = {
  Memory: /** @constructor */ function(opts) {
    return {
      buffer: new ArrayBuffer(opts['initial'] * 65536),
      grow: function(amount) {
        var ret = __growWasmMemory(amount);
        return ret;
      }
    };
  },

  Table: function(opts) {
    var ret = new Array(opts['initial']);
    ret.grow = function(by) {
      if (ret.length >= 1 + 0) {
        abort('Unable to grow wasm table. Use a higher value for RESERVED_FUNCTION_POINTERS or set ALLOW_TABLE_GROWTH.')
      }
      ret.push(null);
    };
    ret.set = function(i, func) {
      ret[i] = func;
    };
    ret.get = function(i) {
      return ret[i];
    };
    return ret;
  },

  Module: function(binary) {
    // TODO: use the binary and info somehow - right now the wasm2js output is embedded in
    // the main JS
    return {};
  },

  Instance: function(module, info) {
    // TODO: use the module and info somehow - right now the wasm2js output is embedded in
    // the main JS
    // This will be replaced by the actual wasm2js code.
    var exports = (
function instantiate(asmLibraryArg, wasmMemory, wasmTable) {

function asmFunc(global, env, buffer) {
 var memory = env.memory;
 var HEAP8 = new global.Int8Array(buffer);
 var HEAP16 = new global.Int16Array(buffer);
 var HEAP32 = new global.Int32Array(buffer);
 var HEAPU8 = new global.Uint8Array(buffer);
 var HEAPU16 = new global.Uint16Array(buffer);
 var HEAPU32 = new global.Uint32Array(buffer);
 var HEAPF32 = new global.Float32Array(buffer);
 var HEAPF64 = new global.Float64Array(buffer);
 var Math_imul = global.Math.imul;
 var Math_fround = global.Math.fround;
 var Math_abs = global.Math.abs;
 var Math_clz32 = global.Math.clz32;
 var Math_min = global.Math.min;
 var Math_max = global.Math.max;
 var Math_floor = global.Math.floor;
 var Math_ceil = global.Math.ceil;
 var Math_sqrt = global.Math.sqrt;
 var abort = env.abort;
 var nan = global.NaN;
 var infinity = global.Infinity;
 var fimport$0 = env.emscripten_memcpy_big;
 var fimport$1 = env.emscripten_resize_heap;
 var fimport$2 = env.segfault;
 var fimport$3 = env.alignfault;
 var global$0 = 5306944;
 var global$1 = 64052;
 var i64toi32_i32$HIGH_BITS = 0;
 // EMSCRIPTEN_START_FUNCS;
 function $1() {
  
 }
 
 function $2($0, $1_1) {
  var $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7 = 0, $8_1 = 0, $9_1 = 0;
  $9_1 = (($1_1 << 2) + $0 | 0) + 20 | 0;
  $4_1 = $31($9_1, 0);
  if (!$4_1) {
   $7 = $31($0, 0);
   $35($0, 0, $7 + 2 | 0);
   $5_1 = $0 + 1052 | 0;
   $6_1 = $7 + 1 | 0;
   $2_1 = $6_1 << 5;
   $4_1 = $5_1 + $2_1 | 0;
   $36($4_1, 24, 1, 257);
   $3_1 = $31($0, 12);
   $35($4_1, 12, $31($3_1, 12));
   $8_1 = $7 << 5;
   $3_1 = $31($3_1, 12);
   label$2 : {
    if ($3_1) {
     $35($3_1, 16, $4_1);
     if (($31($3_1, 24) | 0) == 1) {
      $35((($6_1 << 5) + $0 | 0) + 1072 | 0, 0, $31($3_1, 20));
      break label$2;
     }
     $3_1 = $3($0);
     $35((($6_1 << 5) + $0 | 0) + 1072 | 0, 0, $3_1);
     $35($3_1, 0, $4_1);
     break label$2;
    }
    $3_1 = $3($0);
    $35(($0 + $2_1 | 0) + 1072 | 0, 0, $3_1);
    $35($3_1, 0, $4_1);
   }
   $3_1 = $5_1 + $8_1 | 0;
   $5_1 = $31($0, 12);
   $35($5_1, 12, $4_1);
   $2_1 = $0 + 1052 | 0;
   $35($2_1 + ($6_1 << 5) | 0, 16, $31($0, 12));
   $8_1 = $7 << 5;
   $2_1 = $8_1 + $2_1 | 0;
   $35($2_1, 24, 1);
   $35($2_1, 28, $1_1);
   $35($2_1, 12, $31($5_1, 12));
   $1_1 = $31($5_1, 12);
   label$5 : {
    if ($1_1) {
     $35($1_1, 16, $3_1);
     if (($31($1_1, 24) | 0) == 1) {
      $35((($7 << 5) + $0 | 0) + 1072 | 0, 0, $31($1_1, 20));
      break label$5;
     }
     $1_1 = $3($0);
     $35((($7 << 5) + $0 | 0) + 1072 | 0, 0, $1_1);
     $35($1_1, 0, $4_1);
     break label$5;
    }
    $1_1 = $3($0);
    $35(($0 + $8_1 | 0) + 1072 | 0, 0, $1_1);
    $35($1_1, 0, $3_1);
   }
   $1_1 = $31($0, 12);
   $35($1_1, 12, $3_1);
   $5_1 = $31($0, 12);
   $2_1 = ($7 << 5) + $0 | 0;
   $35($2_1 + 1056 | 0, 0, 0);
   $35($2_1 + 1068 | 0, 0, $5_1);
   $35($3_1, 0, 0);
   $2_1 = $31($1_1, 8);
   label$8 : {
    if ($2_1) {
     $2_1 = ($31($2_1, 0) | 0) == ($1_1 | 0) ? $2_1 : $2_1 + 4 | 0;
     break label$8;
    }
    $2_1 = $0 + 8 | 0;
   }
   $35($2_1, 0, $4_1);
   $2_1 = $0 + 1052 | 0;
   $6_1 = $2_1 + ($6_1 << 5) | 0;
   $35($6_1, 4, $3_1);
   $35($4_1, 0, $5_1);
   $35($6_1, 8, $31($5_1, 8));
   $35($2_1 + ($7 << 5) | 0, 8, $4_1);
   $35($1_1, 8, $4_1);
   $35($9_1, 0, $3_1);
   $4($0, $31($6_1, 8));
   return;
  }
  $4($0, $4_1);
 }
 
 function $3($0) {
  var $1_1 = 0;
  $1_1 = $31($0, 1048);
  if (!$1_1) {
   $1_1 = $31($0, 4);
   $35($0, 4, $1_1 + 1 | 0);
   return (($1_1 << 2) + $0 | 0) + 25628 | 0;
  }
  $35($0, 1048, $31($1_1, 0));
  return $1_1;
 }
 
 function $4($0, $1_1) {
  var $2_1 = 0, $3_1 = 0;
  label$1 : {
   if (!$1_1) {
    break label$1
   }
   $2_1 = $31($1_1, 12);
   label$2 : {
    if (!$2_1) {
     break label$2
    }
    if (($31($2_1, 24) | 0) != ($31($1_1, 24) | 0)) {
     break label$2
    }
    $2_1 = $31($31($1_1, 20), 0);
    if (($31($1_1, 8) | 0) != ($2_1 | 0)) {
     $5($0, $2_1, $1_1)
    }
    $6($2_1, $1_1);
   }
   label$4 : {
    label$5 : {
     $2_1 = $31($1_1, 16);
     if (!$2_1) {
      break label$5
     }
     if (($31($2_1, 24) | 0) != ($31($1_1, 24) | 0)) {
      break label$5
     }
     $35($31($1_1, 20), 0, $2_1);
     break label$4;
    }
    $35($31($1_1, 20), 0, 0);
    $2_1 = $31($1_1, 20);
    $35($2_1, 0, $31($0, 1048));
    $35($0, 1048, $2_1);
   }
   $3_1 = $31($1_1, 24) + 1 | 0;
   $35($1_1, 24, $3_1);
   label$6 : {
    label$7 : {
     $2_1 = $31($1_1, 12);
     if (!$2_1) {
      break label$7
     }
     if (($31($2_1, 24) | 0) != ($3_1 | 0)) {
      break label$7
     }
     $35($1_1, 20, $31($2_1, 20));
     break label$6;
    }
    $2_1 = $3($0);
    $35($1_1, 20, $2_1);
    $35($2_1, 0, $1_1);
   }
   $2_1 = $31($1_1, 8);
   if (!$2_1) {
    break label$1
   }
   $4($0, $2_1);
   $2_1 = $31($1_1, 16);
   $0 = $31($1_1, 8);
   if (($2_1 | 0) != ($0 | 0)) {
    break label$1
   }
   $6($1_1, $0);
   $0 = $31($1_1, 20);
   if (($31($0, 0) | 0) != ($1_1 | 0)) {
    break label$1
   }
   $35($0, 0, $31($1_1, 8));
  }
 }
 
 function $5($0, $1_1, $2_1) {
  var $3_1 = 0, $4_1 = 0, $5_1 = 0;
  $3_1 = $31($2_1, 8);
  $4_1 = $31($1_1, 8);
  label$1 : {
   if ($4_1) {
    $5_1 = ($31($4_1, 0) | 0) == ($1_1 | 0) ? $4_1 : $4_1 + 4 | 0;
    break label$1;
   }
   $5_1 = $0 + 8 | 0;
  }
  $35($5_1, 0, $2_1);
  label$3 : {
   if ($3_1) {
    $0 = ($31($3_1, 0) | 0) == ($2_1 | 0) ? $3_1 : $3_1 + 4 | 0;
    break label$3;
   }
   $0 = $0 + 8 | 0;
  }
  $35($0, 0, $1_1);
  $35($1_1, 8, $3_1);
  $35($2_1, 8, $4_1);
 }
 
 function $6($0, $1_1) {
  var $2_1 = 0;
  $2_1 = $31($0, 12);
  $35($0, 12, $31($1_1, 12));
  $35($1_1, 12, $2_1);
  $2_1 = $31($0, 16);
  $35($0, 16, $31($1_1, 16));
  $35($1_1, 16, $2_1);
  if (($31($0, 12) | 0) == ($0 | 0)) {
   $35($0, 12, $1_1)
  }
  if (($31($1_1, 12) | 0) == ($1_1 | 0)) {
   $35($1_1, 12, $0)
  }
  $2_1 = $31($0, 12);
  if ($2_1) {
   $35($2_1, 16, $0)
  }
  $2_1 = $31($1_1, 12);
  if ($2_1) {
   $35($2_1, 16, $1_1)
  }
  $2_1 = $31($0, 16);
  if ($2_1) {
   $35($2_1, 12, $0)
  }
  $0 = $31($1_1, 16);
  if ($0) {
   $35($0, 12, $1_1)
  }
 }
 
 function $8($0, $1_1, $2_1) {
  var $3_1 = 0;
  if (!$0) {
   return
  }
  label$2 : {
   while (1) {
    $3_1 = $31($0, 28);
    if (($3_1 | 0) == 257) {
     $0 = $31(((($9($2_1) | 0) != 0) << 2) + $0 | 0, 0);
     if ($0) {
      continue
     }
     break label$2;
    }
    break;
   };
   $35($1_1, 0, $3_1);
  }
 }
 
 function $9($0) {
  var $1_1 = 0;
  $1_1 = $0;
  $0 = $31(0, 6144);
  $1_1 = $28($1_1 + ($0 >> 3) | 0, 0);
  $35(0, 6144, $0 + 1 | 0);
  return $1_1 >>> ($0 & 7) & 1;
 }
 
 function $10($0, $1_1, $2_1, $3_1) {
  var $4_1 = 0;
  label$1 : {
   $4_1 = $31((($1_1 << 2) + $0 | 0) + 20 | 0, 0);
   if (!$4_1) {
    $10($0, 256, $2_1, $3_1);
    $3_1 = 7;
    while (1) {
     $0 = $3_1;
     $11($1_1 >>> $3_1 & 1, $2_1);
     $3_1 = $3_1 + -1 | 0;
     if ($0) {
      continue
     }
     break;
    };
    break label$1;
   }
   $12($4_1, 0, $2_1, $3_1);
  }
 }
 
 function $11($0, $1_1) {
  var $2_1 = 0, $3_1 = 0;
  $2_1 = $31(0, 6144);
  $3_1 = $2_1 & 7;
  if (!$3_1) {
   $33(($2_1 >> 3) + $1_1 | 0, 0, 0)
  }
  $1_1 = ($2_1 >> 3) + $1_1 | 0;
  $33($1_1, 0, $28($1_1, 0) | $0 << $3_1);
  $35(0, 6144, $2_1 + 1 | 0);
 }
 
 function $12($0, $1_1, $2_1, $3_1) {
  var $4_1 = 0;
  $4_1 = $31($0, 8);
  if ($4_1) {
   $12($4_1, $0, $2_1, $3_1)
  }
  if ($1_1) {
   if (($31(0, 6144) | 0) >= ($3_1 | 0)) {
    $35(0, 6144, $3_1 + 1 | 0);
    return;
   }
   if (($31($0, 4) | 0) == ($1_1 | 0)) {
    $11(1, $2_1);
    return;
   }
   $11(0, $2_1);
  }
 }
 
 function $13($0, $1_1) {
  $0 = $0 | 0;
  $1_1 = $1_1 | 0;
  var $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7 = 0, $8_1 = 0, $9_1 = 0;
  $8_1 = global$0 - 94256 | 0;
  global$0 = $8_1;
  if (!$28(0, 6148)) {
   $14()
  }
  $9_1 = $31($0, 20) - $1_1 | 0;
  if (($9_1 | 0) >= 1) {
   $5_1 = $31($0, 12);
   $2_1 = $19($8_1, 28700);
   $4_1 = $31($2_1, 0);
   $3_1 = ($4_1 << 5) + $2_1 | 0;
   $6_1 = $3_1 + 1052 | 0;
   $35($2_1 + 1044 | 0, 0, $6_1);
   $35($2_1, 0, $4_1 + 1 | 0);
   $35($2_1, 16, $6_1);
   $35($2_1, 12, $6_1);
   $35($2_1, 8, $6_1);
   $36($3_1 + 1076 | 0, 0, 0, 256);
   $36($6_1, 0, 0, 0);
   $36($3_1 + 1060 | 0, 0, 0, 0);
   $35($3_1 + 1068 | 0, 0, 0);
   $6_1 = $1_1 + $5_1 | 0;
   $3_1 = $28($6_1, 1);
   $4_1 = $28($6_1, 0);
   $5_1 = $31($0, 16);
   $35(0, 6144, 16);
   $5_1 = $5_1 - $1_1 | 0;
   $3_1 = $3_1 | $4_1 << 8;
   $3_1 = ($3_1 | 0) > ($5_1 | 0) ? $5_1 : $3_1;
   label$3 : {
    if (($3_1 | 0) < 1) {
     break label$3
    }
    while (1) {
     $35($2_1, 94252, 0);
     if ($31(0, 6144) >> 3 > ($9_1 | 0)) {
      $33(($2_1 + 28704 | 0) + $7 | 0, 0, 0);
      break label$3;
     }
     $8($31($2_1, 8), $2_1 + 94252 | 0, $6_1);
     if (($31($2_1, 94252) | 0) == 256) {
      $4_1 = 0;
      $35($2_1, 94252, 0);
      $5_1 = 0;
      while (1) {
       $4_1 = $9($6_1) + ($4_1 << 1) | 0;
       $5_1 = $5_1 + 1 | 0;
       if (($5_1 | 0) != 8) {
        continue
       }
       break;
      };
      $35($2_1, 94252, $4_1);
     }
     $4_1 = $28($2_1, 94252);
     $33(($2_1 + 28704 | 0) + $7 | 0, 0, $4_1);
     $2($2_1, $4_1);
     $7 = $7 + 1 | 0;
     if (($7 | 0) < ($3_1 | 0)) {
      continue
     }
     break;
    };
   }
   $35($0, 20, $1_1 + $3_1 | 0);
   $18($31($0, 12) + $1_1 | 0, $2_1 + 28704 | 0, $3_1);
  }
  global$0 = $8_1 + 94256 | 0;
 }
 
 function $14() {
  var $0 = 0, $1_1 = 0, $2_1 = 0, $3_1 = 0;
  $33(0, 6148, 1);
  $15();
  while (1) {
   $2_1 = ($0 << 2) + 5120 | 0;
   if (($31($2_1, 0) | 0) >= 1) {
    $1_1 = 0;
    while (1) {
     $3_1 = $0 & 255;
     $2(6152, $3_1);
     $2(34852, $3_1);
     $1_1 = $1_1 + 1 | 0;
     if (($1_1 | 0) < ($31($2_1, 0) | 0)) {
      continue
     }
     break;
    };
   }
   $0 = $0 + 1 | 0;
   if (($0 | 0) != 256) {
    continue
   }
   break;
  };
 }
 
 function $15() {
  var $0 = 0, $1_1 = 0, $2_1 = 0;
  $1_1 = $19(6152, 57400);
  $0 = $31($1_1, 28700);
  $35($1_1, 28700, $0 + 1 | 0);
  $2_1 = ($1_1 + 28700 | 0) + ($0 << 5) | 0;
  $0 = $2_1 + 1052 | 0;
  $35($1_1 + 29744 | 0, 0, $0);
  $35($1_1 + 28716 | 0, 0, $0);
  $35($1_1 + 28712 | 0, 0, $0);
  $35($1_1 + 28708 | 0, 0, $0);
  $36($2_1 + 1076 | 0, 0, 0, 256);
  $36($0, 0, 0, 0);
  $36($2_1 + 1060 | 0, 0, 0, 0);
  $35($2_1 + 1068 | 0, 0, 0);
  $0 = $31($1_1, 0);
  $35($1_1, 0, $0 + 1 | 0);
  $0 = $1_1 + ($0 << 5) | 0;
  $2_1 = $0 + 1052 | 0;
  $35($1_1, 12, $2_1);
  $35($1_1, 8, $2_1);
  $35($1_1 + 1044 | 0, 0, $2_1);
  $36($0 + 1076 | 0, 0, 0, 256);
  $36($2_1, 0, 0, 0);
  $36($0 + 1060 | 0, 0, 0, 0);
  $35($0 + 1068 | 0, 0, 0);
 }
 
 function $16($0, $1_1) {
  $0 = $0 | 0;
  $1_1 = $1_1 | 0;
  var $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7 = 0, $8_1 = 0, $9_1 = 0;
  $6_1 = global$0 - 94240 | 0;
  global$0 = $6_1;
  if (!$28(0, 6148)) {
   $14()
  }
  $2_1 = $31($0, 20) - $1_1 | 0;
  if (($2_1 | 0) >= 1) {
   $9_1 = $31($0, 12) + $1_1 | 0;
   $3_1 = $19($6_1, 28700);
   $8_1 = $31($3_1, 0);
   $5_1 = ($8_1 << 5) + $3_1 | 0;
   $4_1 = $5_1 + 1052 | 0;
   $35($3_1 + 1044 | 0, 0, $4_1);
   $35(0, 6144, 16);
   $35($3_1, 0, $8_1 + 1 | 0);
   $35($3_1, 12, $4_1);
   $35($3_1, 8, $4_1);
   $36($5_1 + 1076 | 0, 0, 0, 256);
   $36($4_1, 0, 0, 0);
   $36($5_1 + 1060 | 0, 0, 0, 0);
   $35($5_1 + 1068 | 0, 0, 0);
   $34($3_1, ($2_1 << 8 & 16711680 | $2_1 << 24) >>> 16 | 0);
   $5_1 = $2_1 << 3;
   while (1) {
    $4_1 = $28($7 + $9_1 | 0, 0);
    $10($3_1, $4_1, $3_1 + 28704 | 0, $5_1);
    $2($3_1, $4_1);
    $7 = $7 + 1 | 0;
    if (($2_1 | 0) != ($7 | 0)) {
     continue
    }
    break;
   };
   $2_1 = $31(0, 6144) + 8 | 0;
   $35(0, 6144, $2_1);
   $2_1 = $2_1 >> 3;
   $35($0, 20, $2_1 + $1_1 | 0);
   $18($31($0, 12) + $1_1 | 0, $3_1 + 28704 | 0, $2_1);
  }
  global$0 = $6_1 + 94240 | 0;
 }
 
 function $17($0, $1_1, $2_1) {
  $0 = $0 | 0;
  $1_1 = $1_1 | 0;
  $2_1 = $2_1 | 0;
  var $3_1 = 0;
  $3_1 = $0;
  $0 = $29((($30(($2_1 >> 3) + $1_1 | 0) >>> ($2_1 & 7) & 2047) << 1) + 1024 | 0);
  $35($3_1, 0, $0 & 255);
  return $0 >>> 8 | 0;
 }
 
 function $18($0, $1_1, $2_1) {
  var $3_1 = 0, $4_1 = 0;
  if ($2_1 >>> 0 >= 512) {
   fimport$0($0 | 0, $1_1 | 0, $2_1 | 0) | 0;
   return;
  }
  $3_1 = $0 + $2_1 | 0;
  label$2 : {
   if (!(($0 ^ $1_1) & 3)) {
    label$4 : {
     if (($2_1 | 0) < 1) {
      $2_1 = $0;
      break label$4;
     }
     if (!($0 & 3)) {
      $2_1 = $0;
      break label$4;
     }
     $2_1 = $0;
     while (1) {
      $33($2_1, 0, $28($1_1, 0));
      $1_1 = $1_1 + 1 | 0;
      $2_1 = $2_1 + 1 | 0;
      if ($2_1 >>> 0 >= $3_1 >>> 0) {
       break label$4
      }
      if ($2_1 & 3) {
       continue
      }
      break;
     };
    }
    $0 = $3_1 & -4;
    label$8 : {
     if ($0 >>> 0 < 64) {
      break label$8
     }
     $4_1 = $0 + -64 | 0;
     if ($2_1 >>> 0 > $4_1 >>> 0) {
      break label$8
     }
     while (1) {
      $35($2_1, 0, $31($1_1, 0));
      $35($2_1, 4, $31($1_1, 4));
      $35($2_1, 8, $31($1_1, 8));
      $35($2_1, 12, $31($1_1, 12));
      $35($2_1, 16, $31($1_1, 16));
      $35($2_1, 20, $31($1_1, 20));
      $35($2_1, 24, $31($1_1, 24));
      $35($2_1, 28, $31($1_1, 28));
      $35($2_1, 32, $31($1_1, 32));
      $35($2_1, 36, $31($1_1, 36));
      $35($2_1, 40, $31($1_1, 40));
      $35($2_1, 44, $31($1_1, 44));
      $35($2_1, 48, $31($1_1, 48));
      $35($2_1, 52, $31($1_1, 52));
      $35($2_1, 56, $31($1_1, 56));
      $35($2_1, 60, $31($1_1, 60));
      $1_1 = $1_1 - -64 | 0;
      $2_1 = $2_1 - -64 | 0;
      if ($2_1 >>> 0 <= $4_1 >>> 0) {
       continue
      }
      break;
     };
    }
    if ($2_1 >>> 0 >= $0 >>> 0) {
     break label$2
    }
    while (1) {
     $35($2_1, 0, $31($1_1, 0));
     $1_1 = $1_1 + 4 | 0;
     $2_1 = $2_1 + 4 | 0;
     if ($2_1 >>> 0 < $0 >>> 0) {
      continue
     }
     break;
    };
    break label$2;
   }
   if ($3_1 >>> 0 < 4) {
    $2_1 = $0;
    break label$2;
   }
   $4_1 = $3_1 + -4 | 0;
   if ($4_1 >>> 0 < $0 >>> 0) {
    $2_1 = $0;
    break label$2;
   }
   $2_1 = $0;
   while (1) {
    $33($2_1, 0, $28($1_1, 0));
    $33($2_1, 1, $28($1_1, 1));
    $33($2_1, 2, $28($1_1, 2));
    $33($2_1, 3, $28($1_1, 3));
    $1_1 = $1_1 + 4 | 0;
    $2_1 = $2_1 + 4 | 0;
    if ($2_1 >>> 0 <= $4_1 >>> 0) {
     continue
    }
    break;
   };
  }
  if ($2_1 >>> 0 < $3_1 >>> 0) {
   while (1) {
    $33($2_1, 0, $28($1_1, 0));
    $1_1 = $1_1 + 1 | 0;
    $2_1 = $2_1 + 1 | 0;
    if (($3_1 | 0) != ($2_1 | 0)) {
     continue
    }
    break;
   }
  }
 }
 
 function $19($0, $1_1) {
  var $2_1 = 0, $3_1 = 0;
  label$1 : {
   if (!$1_1) {
    break label$1
   }
   $2_1 = $0 + $1_1 | 0;
   $33($2_1 + -1 | 0, 0, 0);
   $33($0, 0, 0);
   if ($1_1 >>> 0 < 3) {
    break label$1
   }
   $33($2_1 + -2 | 0, 0, 0);
   $33($0, 1, 0);
   $33($2_1 + -3 | 0, 0, 0);
   $33($0, 2, 0);
   if ($1_1 >>> 0 < 7) {
    break label$1
   }
   $33($2_1 + -4 | 0, 0, 0);
   $33($0, 3, 0);
   if ($1_1 >>> 0 < 9) {
    break label$1
   }
   $3_1 = 0 - $0 & 3;
   $2_1 = $3_1 + $0 | 0;
   $35($2_1, 0, 0);
   $3_1 = $1_1 - $3_1 & -4;
   $1_1 = $3_1 + $2_1 | 0;
   $35($1_1 + -4 | 0, 0, 0);
   if ($3_1 >>> 0 < 9) {
    break label$1
   }
   $35($2_1, 8, 0);
   $35($2_1, 4, 0);
   $35($1_1 + -8 | 0, 0, 0);
   $35($1_1 + -12 | 0, 0, 0);
   if ($3_1 >>> 0 < 25) {
    break label$1
   }
   $35($2_1, 24, 0);
   $35($2_1, 20, 0);
   $35($2_1, 16, 0);
   $35($2_1, 12, 0);
   $35($1_1 + -16 | 0, 0, 0);
   $35($1_1 + -20 | 0, 0, 0);
   $35($1_1 + -24 | 0, 0, 0);
   $35($1_1 + -28 | 0, 0, 0);
   $1_1 = $3_1;
   $3_1 = $2_1 & 4 | 24;
   $1_1 = $1_1 - $3_1 | 0;
   if ($1_1 >>> 0 < 32) {
    break label$1
   }
   $2_1 = $2_1 + $3_1 | 0;
   while (1) {
    $37($2_1, 24);
    $37($2_1, 16);
    $37($2_1, 8);
    $37($2_1, 0);
    $2_1 = $2_1 + 32 | 0;
    $1_1 = $1_1 + -32 | 0;
    if ($1_1 >>> 0 > 31) {
     continue
    }
    break;
   };
  }
  return $0;
 }
 
 function $20() {
  return 63552;
 }
 
 function $21($0) {
  var $1_1 = 0, $2_1 = 0;
  $2_1 = __wasm_memory_size();
  label$1 : {
   $1_1 = $31(64064, 0);
   $0 = $1_1 + ($0 + 3 & -4) | 0;
   if ($0 >>> 0 <= $2_1 << 16 >>> 0) {
    break label$1
   }
   if (fimport$1($0 | 0)) {
    break label$1
   }
   $35(63552, 0, 48);
   return -1;
  }
  $35(64064, 0, $0);
  return $1_1;
 }
 
 function $22($0) {
  $0 = $0 | 0;
  var $1_1 = 0, $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7 = 0, $8_1 = 0, $9_1 = 0, $10_1 = 0, $11_1 = 0;
  $11_1 = global$0 - 16 | 0;
  global$0 = $11_1;
  label$1 : {
   label$2 : {
    label$3 : {
     label$4 : {
      label$5 : {
       label$6 : {
        label$7 : {
         label$8 : {
          label$9 : {
           label$10 : {
            label$11 : {
             if ($0 >>> 0 <= 244) {
              $5_1 = $31(0, 63556);
              $6_1 = $0 >>> 0 < 11 ? 16 : $0 + 11 & -8;
              $0 = $6_1 >>> 3 | 0;
              $1_1 = $5_1 >>> $0 | 0;
              if ($1_1 & 3) {
               $2_1 = $0 + (($1_1 ^ -1) & 1) | 0;
               $4_1 = $2_1 << 3;
               $1_1 = $31($4_1 + 63604 | 0, 0);
               $0 = $1_1 + 8 | 0;
               $3_1 = $31($1_1, 8);
               $4_1 = $4_1 + 63596 | 0;
               label$14 : {
                if (($3_1 | 0) == ($4_1 | 0)) {
                 $35(0, 63556, __wasm_rotl_i32($2_1) & $5_1);
                 break label$14;
                }
                $31(0, 63572);
                $35($3_1, 12, $4_1);
                $35($4_1, 8, $3_1);
               }
               $2_1 = $2_1 << 3;
               $35($1_1, 4, $2_1 | 3);
               $1_1 = $1_1 + $2_1 | 0;
               $35($1_1, 4, $31($1_1, 4) | 1);
               break label$1;
              }
              $8_1 = $31(0, 63564);
              if ($6_1 >>> 0 <= $8_1 >>> 0) {
               break label$11
              }
              if ($1_1) {
               $2_1 = 2 << $0;
               $0 = (0 - $2_1 | $2_1) & $1_1 << $0;
               $0 = (0 - $0 & $0) + -1 | 0;
               $1_1 = $0 >>> 12 & 16;
               $2_1 = $1_1;
               $0 = $0 >>> $1_1 | 0;
               $1_1 = $0 >>> 5 & 8;
               $2_1 = $2_1 | $1_1;
               $0 = $0 >>> $1_1 | 0;
               $1_1 = $0 >>> 2 & 4;
               $2_1 = $2_1 | $1_1;
               $0 = $0 >>> $1_1 | 0;
               $1_1 = $0 >>> 1 & 2;
               $2_1 = $2_1 | $1_1;
               $0 = $0 >>> $1_1 | 0;
               $1_1 = $0 >>> 1 & 1;
               $2_1 = ($2_1 | $1_1) + ($0 >>> $1_1 | 0) | 0;
               $3_1 = $2_1 << 3;
               $1_1 = $31($3_1 + 63604 | 0, 0);
               $0 = $31($1_1, 8);
               $3_1 = $3_1 + 63596 | 0;
               label$17 : {
                if (($0 | 0) == ($3_1 | 0)) {
                 $5_1 = __wasm_rotl_i32($2_1) & $5_1;
                 $35(0, 63556, $5_1);
                 break label$17;
                }
                $31(0, 63572);
                $35($0, 12, $3_1);
                $35($3_1, 8, $0);
               }
               $0 = $1_1 + 8 | 0;
               $35($1_1, 4, $6_1 | 3);
               $7 = $1_1 + $6_1 | 0;
               $2_1 = $2_1 << 3;
               $3_1 = $2_1 - $6_1 | 0;
               $35($7, 4, $3_1 | 1);
               $35($1_1 + $2_1 | 0, 0, $3_1);
               if ($8_1) {
                $4_1 = $8_1 >>> 3 | 0;
                $1_1 = ($4_1 << 3) + 63596 | 0;
                $2_1 = $31(0, 63576);
                $4_1 = 1 << $4_1;
                label$20 : {
                 if (!($4_1 & $5_1)) {
                  $35(0, 63556, $4_1 | $5_1);
                  $4_1 = $1_1;
                  break label$20;
                 }
                 $4_1 = $31($1_1, 8);
                }
                $35($1_1, 8, $2_1);
                $35($4_1, 12, $2_1);
                $35($2_1, 12, $1_1);
                $35($2_1, 8, $4_1);
               }
               $35(0, 63576, $7);
               $35(0, 63564, $3_1);
               break label$1;
              }
              $10_1 = $31(0, 63560);
              if (!$10_1) {
               break label$11
              }
              $0 = ($10_1 & 0 - $10_1) + -1 | 0;
              $1_1 = $0 >>> 12 & 16;
              $2_1 = $1_1;
              $0 = $0 >>> $1_1 | 0;
              $1_1 = $0 >>> 5 & 8;
              $2_1 = $2_1 | $1_1;
              $0 = $0 >>> $1_1 | 0;
              $1_1 = $0 >>> 2 & 4;
              $2_1 = $2_1 | $1_1;
              $0 = $0 >>> $1_1 | 0;
              $1_1 = $0 >>> 1 & 2;
              $2_1 = $2_1 | $1_1;
              $0 = $0 >>> $1_1 | 0;
              $1_1 = $0 >>> 1 & 1;
              $1_1 = $31((($2_1 | $1_1) + ($0 >>> $1_1 | 0) << 2) + 63860 | 0, 0);
              $2_1 = ($31($1_1, 4) & -8) - $6_1 | 0;
              $3_1 = $1_1;
              while (1) {
               label$23 : {
                $0 = $31($3_1, 16);
                if (!$0) {
                 $0 = $31($3_1 + 20 | 0, 0);
                 if (!$0) {
                  break label$23
                 }
                }
                $4_1 = ($31($0, 4) & -8) - $6_1 | 0;
                $3_1 = $4_1 >>> 0 < $2_1 >>> 0;
                $2_1 = $3_1 ? $4_1 : $2_1;
                $1_1 = $3_1 ? $0 : $1_1;
                $3_1 = $0;
                continue;
               }
               break;
              };
              $9_1 = $31($1_1, 24);
              $4_1 = $31($1_1, 12);
              if (($4_1 | 0) != ($1_1 | 0)) {
               $3_1 = $31(0, 63572);
               $0 = $31($1_1, 8);
               if ($3_1 >>> 0 <= $0 >>> 0) {
                $31($0, 12)
               }
               $35($0, 12, $4_1);
               $35($4_1, 8, $0);
               break label$2;
              }
              $3_1 = $1_1 + 20 | 0;
              $0 = $31($3_1, 0);
              if (!$0) {
               $0 = $31($1_1, 16);
               if (!$0) {
                break label$10
               }
               $3_1 = $1_1 + 16 | 0;
              }
              while (1) {
               $7 = $3_1;
               $4_1 = $0;
               $3_1 = $0 + 20 | 0;
               $0 = $31($3_1, 0);
               if ($0) {
                continue
               }
               $3_1 = $4_1 + 16 | 0;
               $0 = $31($4_1, 16);
               if ($0) {
                continue
               }
               break;
              };
              $35($7, 0, 0);
              break label$2;
             }
             $6_1 = -1;
             if ($0 >>> 0 > 4294967231) {
              break label$11
             }
             $0 = $0 + 11 | 0;
             $6_1 = $0 & -8;
             $7 = $31(0, 63560);
             if (!$7) {
              break label$11
             }
             $3_1 = 0 - $6_1 | 0;
             $0 = $0 >>> 8 | 0;
             $8_1 = 0;
             label$29 : {
              if (!$0) {
               break label$29
              }
              $8_1 = 31;
              if ($6_1 >>> 0 > 16777215) {
               break label$29
              }
              $2_1 = $0 + 1048320 >>> 16 & 8;
              $1_1 = $0 << $2_1;
              $0 = $1_1 + 520192 >>> 16 & 4;
              $4_1 = $1_1 << $0;
              $1_1 = $4_1 + 245760 >>> 16 & 2;
              $0 = ($4_1 << $1_1 >>> 15 | 0) - ($1_1 | ($0 | $2_1)) | 0;
              $8_1 = ($0 << 1 | $6_1 >>> $0 + 21 & 1) + 28 | 0;
             }
             $2_1 = $31(($8_1 << 2) + 63860 | 0, 0);
             label$30 : {
              label$31 : {
               label$32 : {
                if (!$2_1) {
                 $0 = 0;
                 $4_1 = 0;
                 break label$32;
                }
                $1_1 = $6_1 << (($8_1 | 0) == 31 ? 0 : 25 - ($8_1 >>> 1 | 0) | 0);
                $0 = 0;
                $4_1 = 0;
                while (1) {
                 label$35 : {
                  $5_1 = ($31($2_1, 4) & -8) - $6_1 | 0;
                  if ($5_1 >>> 0 >= $3_1 >>> 0) {
                   break label$35
                  }
                  $4_1 = $2_1;
                  $3_1 = $5_1;
                  if ($3_1) {
                   break label$35
                  }
                  $3_1 = 0;
                  $0 = $2_1;
                  break label$31;
                 }
                 $5_1 = $31($2_1 + 20 | 0, 0);
                 $2_1 = $31((($1_1 >>> 29 & 4) + $2_1 | 0) + 16 | 0, 0);
                 $0 = $5_1 ? (($5_1 | 0) == ($2_1 | 0) ? $0 : $5_1) : $0;
                 $1_1 = $1_1 << (($2_1 | 0) != 0);
                 if ($2_1) {
                  continue
                 }
                 break;
                };
               }
               if (!($0 | $4_1)) {
                $0 = 2 << $8_1;
                $0 = (0 - $0 | $0) & $7;
                if (!$0) {
                 break label$11
                }
                $0 = ($0 & 0 - $0) + -1 | 0;
                $1_1 = $0 >>> 12 & 16;
                $2_1 = $1_1;
                $0 = $0 >>> $1_1 | 0;
                $1_1 = $0 >>> 5 & 8;
                $2_1 = $2_1 | $1_1;
                $0 = $0 >>> $1_1 | 0;
                $1_1 = $0 >>> 2 & 4;
                $2_1 = $2_1 | $1_1;
                $0 = $0 >>> $1_1 | 0;
                $1_1 = $0 >>> 1 & 2;
                $2_1 = $2_1 | $1_1;
                $0 = $0 >>> $1_1 | 0;
                $1_1 = $0 >>> 1 & 1;
                $0 = $31((($2_1 | $1_1) + ($0 >>> $1_1 | 0) << 2) + 63860 | 0, 0);
               }
               if (!$0) {
                break label$30
               }
              }
              while (1) {
               $2_1 = ($31($0, 4) & -8) - $6_1 | 0;
               $1_1 = $2_1 >>> 0 < $3_1 >>> 0;
               $3_1 = $1_1 ? $2_1 : $3_1;
               $4_1 = $1_1 ? $0 : $4_1;
               $1_1 = $31($0, 16);
               if ($1_1) {
                $0 = $1_1
               } else {
                $0 = $31($0 + 20 | 0, 0)
               }
               if ($0) {
                continue
               }
               break;
              };
             }
             if (!$4_1) {
              break label$11
             }
             if ($3_1 >>> 0 >= $31(0, 63564) - $6_1 >>> 0) {
              break label$11
             }
             $8_1 = $31($4_1, 24);
             $1_1 = $31($4_1, 12);
             if (($4_1 | 0) != ($1_1 | 0)) {
              $2_1 = $31(0, 63572);
              $0 = $31($4_1, 8);
              if ($2_1 >>> 0 <= $0 >>> 0) {
               $31($0, 12)
              }
              $35($0, 12, $1_1);
              $35($1_1, 8, $0);
              break label$3;
             }
             $2_1 = $4_1 + 20 | 0;
             $0 = $31($2_1, 0);
             if (!$0) {
              $0 = $31($4_1, 16);
              if (!$0) {
               break label$9
              }
              $2_1 = $4_1 + 16 | 0;
             }
             while (1) {
              $5_1 = $2_1;
              $1_1 = $0;
              $2_1 = $0 + 20 | 0;
              $0 = $31($2_1, 0);
              if ($0) {
               continue
              }
              $2_1 = $1_1 + 16 | 0;
              $0 = $31($1_1, 16);
              if ($0) {
               continue
              }
              break;
             };
             $35($5_1, 0, 0);
             break label$3;
            }
            $1_1 = $31(0, 63564);
            if ($1_1 >>> 0 >= $6_1 >>> 0) {
             $0 = $31(0, 63576);
             $2_1 = $1_1 - $6_1 | 0;
             label$45 : {
              if ($2_1 >>> 0 >= 16) {
               $35(0, 63564, $2_1);
               $3_1 = $0 + $6_1 | 0;
               $35(0, 63576, $3_1);
               $35($3_1, 4, $2_1 | 1);
               $35($0 + $1_1 | 0, 0, $2_1);
               $35($0, 4, $6_1 | 3);
               break label$45;
              }
              $35(0, 63576, 0);
              $35(0, 63564, 0);
              $35($0, 4, $1_1 | 3);
              $1_1 = $0 + $1_1 | 0;
              $35($1_1, 4, $31($1_1, 4) | 1);
             }
             $0 = $0 + 8 | 0;
             break label$1;
            }
            $1_1 = $31(0, 63568);
            if ($1_1 >>> 0 > $6_1 >>> 0) {
             $1_1 = $1_1 - $6_1 | 0;
             $35(0, 63568, $1_1);
             $0 = $31(0, 63580);
             $2_1 = $0 + $6_1 | 0;
             $35(0, 63580, $2_1);
             $35($2_1, 4, $1_1 | 1);
             $35($0, 4, $6_1 | 3);
             $0 = $0 + 8 | 0;
             break label$1;
            }
            $0 = 0;
            $4_1 = $6_1 + 47 | 0;
            $3_1 = $4_1;
            label$48 : {
             if ($31(0, 64028)) {
              $2_1 = $31(0, 64036);
              break label$48;
             }
             $36(0, 64040, -1, -1);
             $36(0, 64032, 4096, 4096);
             $35(0, 64028, $11_1 + 12 & -16 ^ 1431655768);
             $35(0, 64048, 0);
             $35(0, 64e3, 0);
             $2_1 = 4096;
            }
            $5_1 = $3_1 + $2_1 | 0;
            $7 = 0 - $2_1 | 0;
            $2_1 = $5_1 & $7;
            if ($2_1 >>> 0 <= $6_1 >>> 0) {
             break label$1
            }
            $3_1 = $31(0, 63996);
            if ($3_1) {
             $8_1 = $31(0, 63988);
             $9_1 = $8_1 + $2_1 | 0;
             if ($9_1 >>> 0 <= $8_1 >>> 0 | $9_1 >>> 0 > $3_1 >>> 0) {
              break label$1
             }
            }
            if ($28(0, 64e3) & 4) {
             break label$6
            }
            label$51 : {
             label$52 : {
              $3_1 = $31(0, 63580);
              if ($3_1) {
               $0 = 64004;
               while (1) {
                $8_1 = $31($0, 0);
                if ($8_1 >>> 0 <= $3_1 >>> 0) {
                 if ($31($0, 4) + $8_1 >>> 0 > $3_1 >>> 0) {
                  break label$52
                 }
                }
                $0 = $31($0, 8);
                if ($0) {
                 continue
                }
                break;
               };
              }
              $1_1 = $21(0);
              if (($1_1 | 0) == -1) {
               break label$7
              }
              $5_1 = $2_1;
              $0 = $31(0, 64032);
              $3_1 = $0 + -1 | 0;
              if ($3_1 & $1_1) {
               $5_1 = ($2_1 - $1_1 | 0) + ($1_1 + $3_1 & 0 - $0) | 0
              }
              if ($5_1 >>> 0 <= $6_1 >>> 0 | $5_1 >>> 0 > 2147483646) {
               break label$7
              }
              $0 = $31(0, 63996);
              if ($0) {
               $3_1 = $31(0, 63988);
               $7 = $3_1 + $5_1 | 0;
               if ($7 >>> 0 <= $3_1 >>> 0 | $7 >>> 0 > $0 >>> 0) {
                break label$7
               }
              }
              $0 = $21($5_1);
              if (($1_1 | 0) != ($0 | 0)) {
               break label$51
              }
              break label$5;
             }
             $5_1 = $7 & $5_1 - $1_1;
             if ($5_1 >>> 0 > 2147483646) {
              break label$7
             }
             $1_1 = $21($5_1);
             if (($1_1 | 0) == ($31($0, 0) + $31($0, 4) | 0)) {
              break label$8
             }
             $0 = $1_1;
            }
            $1_1 = $0;
            if (!($6_1 + 48 >>> 0 <= $5_1 >>> 0 | $5_1 >>> 0 > 2147483646 | ($0 | 0) == -1)) {
             $0 = $31(0, 64036);
             $0 = $0 + ($4_1 - $5_1 | 0) & 0 - $0;
             if ($0 >>> 0 > 2147483646) {
              break label$5
             }
             if (($21($0) | 0) != -1) {
              $5_1 = $0 + $5_1 | 0;
              break label$5;
             }
             $21(0 - $5_1 | 0);
             break label$7;
            }
            if (($1_1 | 0) != -1) {
             break label$5
            }
            break label$7;
           }
           $4_1 = 0;
           break label$2;
          }
          $1_1 = 0;
          break label$3;
         }
         if (($1_1 | 0) != -1) {
          break label$5
         }
        }
        $35(0, 64e3, $31(0, 64e3) | 4);
       }
       if ($2_1 >>> 0 > 2147483646) {
        break label$4
       }
       $1_1 = $21($2_1);
       $0 = $21(0);
       if ($1_1 >>> 0 >= $0 >>> 0 | ($1_1 | 0) == -1 | ($0 | 0) == -1) {
        break label$4
       }
       $5_1 = $0 - $1_1 | 0;
       if ($5_1 >>> 0 <= $6_1 + 40 >>> 0) {
        break label$4
       }
      }
      $0 = $31(0, 63988) + $5_1 | 0;
      $35(0, 63988, $0);
      if ($0 >>> 0 > $31(0, 63992) >>> 0) {
       $35(0, 63992, $0)
      }
      label$61 : {
       label$62 : {
        label$63 : {
         $3_1 = $31(0, 63580);
         if ($3_1) {
          $0 = 64004;
          while (1) {
           $2_1 = $31($0, 0);
           $4_1 = $31($0, 4);
           if (($2_1 + $4_1 | 0) == ($1_1 | 0)) {
            break label$63
           }
           $0 = $31($0, 8);
           if ($0) {
            continue
           }
           break;
          };
          break label$62;
         }
         $0 = $31(0, 63572);
         if (!($1_1 >>> 0 >= $0 >>> 0 ? $0 : 0)) {
          $35(0, 63572, $1_1)
         }
         $0 = 0;
         $35(0, 64008, $5_1);
         $35(0, 64004, $1_1);
         $35(0, 63588, -1);
         $35(0, 63592, $31(0, 64028));
         $35(0, 64016, 0);
         while (1) {
          $2_1 = $0 << 3;
          $3_1 = $2_1 + 63596 | 0;
          $35($2_1 + 63604 | 0, 0, $3_1);
          $35($2_1 + 63608 | 0, 0, $3_1);
          $0 = $0 + 1 | 0;
          if (($0 | 0) != 32) {
           continue
          }
          break;
         };
         $0 = $5_1 + -40 | 0;
         $2_1 = $1_1 + 8 & 7 ? -8 - $1_1 & 7 : 0;
         $3_1 = $0 - $2_1 | 0;
         $35(0, 63568, $3_1);
         $2_1 = $1_1 + $2_1 | 0;
         $35(0, 63580, $2_1);
         $35($2_1, 4, $3_1 | 1);
         $35($0 + $1_1 | 0, 4, 40);
         $35(0, 63584, $31(0, 64044));
         break label$61;
        }
        if ($28($0, 12) & 8 | $1_1 >>> 0 <= $3_1 >>> 0 | $2_1 >>> 0 > $3_1 >>> 0) {
         break label$62
        }
        $35($0, 4, $4_1 + $5_1 | 0);
        $0 = $3_1 + 8 & 7 ? -8 - $3_1 & 7 : 0;
        $1_1 = $0 + $3_1 | 0;
        $35(0, 63580, $1_1);
        $2_1 = $31(0, 63568) + $5_1 | 0;
        $0 = $2_1 - $0 | 0;
        $35(0, 63568, $0);
        $35($1_1, 4, $0 | 1);
        $35($2_1 + $3_1 | 0, 4, 40);
        $35(0, 63584, $31(0, 64044));
        break label$61;
       }
       $4_1 = $31(0, 63572);
       if ($1_1 >>> 0 < $4_1 >>> 0) {
        $35(0, 63572, $1_1);
        $4_1 = $1_1;
       }
       $2_1 = $1_1 + $5_1 | 0;
       $0 = 64004;
       label$69 : {
        label$70 : {
         label$71 : {
          label$72 : {
           label$73 : {
            label$74 : {
             while (1) {
              if (($31($0, 0) | 0) != ($2_1 | 0)) {
               $0 = $31($0, 8);
               if ($0) {
                continue
               }
               break label$74;
              }
              break;
             };
             if (!($28($0, 12) & 8)) {
              break label$73
             }
            }
            $0 = 64004;
            while (1) {
             $2_1 = $31($0, 0);
             if ($2_1 >>> 0 <= $3_1 >>> 0) {
              $4_1 = $31($0, 4) + $2_1 | 0;
              if ($4_1 >>> 0 > $3_1 >>> 0) {
               break label$72
              }
             }
             $0 = $31($0, 8);
             continue;
            };
           }
           $35($0, 0, $1_1);
           $35($0, 4, $31($0, 4) + $5_1 | 0);
           $9_1 = ($1_1 + 8 & 7 ? -8 - $1_1 & 7 : 0) + $1_1 | 0;
           $35($9_1, 4, $6_1 | 3);
           $1_1 = $2_1 + ($2_1 + 8 & 7 ? -8 - $2_1 & 7 : 0) | 0;
           $0 = ($1_1 - $9_1 | 0) - $6_1 | 0;
           $7 = $6_1 + $9_1 | 0;
           if (($1_1 | 0) == ($3_1 | 0)) {
            $35(0, 63580, $7);
            $0 = $31(0, 63568) + $0 | 0;
            $35(0, 63568, $0);
            $35($7, 4, $0 | 1);
            break label$70;
           }
           if (($31(0, 63576) | 0) == ($1_1 | 0)) {
            $35(0, 63576, $7);
            $0 = $31(0, 63564) + $0 | 0;
            $35(0, 63564, $0);
            $35($7, 4, $0 | 1);
            $35($0 + $7 | 0, 0, $0);
            break label$70;
           }
           $2_1 = $31($1_1, 4);
           if (($2_1 & 3) == 1) {
            $10_1 = $2_1 & -8;
            label$82 : {
             if ($2_1 >>> 0 <= 255) {
              $3_1 = $31($1_1, 12);
              $4_1 = $31($1_1, 8);
              $2_1 = $2_1 >>> 3 | 0;
              if (($3_1 | 0) == ($4_1 | 0)) {
               $35(0, 63556, $31(0, 63556) & __wasm_rotl_i32($2_1));
               break label$82;
              }
              $35($4_1, 12, $3_1);
              $35($3_1, 8, $4_1);
              break label$82;
             }
             $8_1 = $31($1_1, 24);
             $5_1 = $31($1_1, 12);
             label$85 : {
              if (($5_1 | 0) != ($1_1 | 0)) {
               $2_1 = $31($1_1, 8);
               if ($4_1 >>> 0 <= $2_1 >>> 0) {
                $31($2_1, 12)
               }
               $35($2_1, 12, $5_1);
               $35($5_1, 8, $2_1);
               break label$85;
              }
              label$88 : {
               $2_1 = $1_1 + 20 | 0;
               $6_1 = $31($2_1, 0);
               if ($6_1) {
                break label$88
               }
               $2_1 = $1_1 + 16 | 0;
               $6_1 = $31($2_1, 0);
               if ($6_1) {
                break label$88
               }
               $5_1 = 0;
               break label$85;
              }
              while (1) {
               $3_1 = $2_1;
               $5_1 = $6_1;
               $2_1 = $5_1 + 20 | 0;
               $6_1 = $31($2_1, 0);
               if ($6_1) {
                continue
               }
               $2_1 = $5_1 + 16 | 0;
               $6_1 = $31($5_1, 16);
               if ($6_1) {
                continue
               }
               break;
              };
              $35($3_1, 0, 0);
             }
             if (!$8_1) {
              break label$82
             }
             $2_1 = $31($1_1, 28);
             $3_1 = ($2_1 << 2) + 63860 | 0;
             label$90 : {
              if (($31($3_1, 0) | 0) == ($1_1 | 0)) {
               $35($3_1, 0, $5_1);
               if ($5_1) {
                break label$90
               }
               $35(0, 63560, $31(0, 63560) & __wasm_rotl_i32($2_1));
               break label$82;
              }
              $35($8_1 + (($31($8_1, 16) | 0) == ($1_1 | 0) ? 16 : 20) | 0, 0, $5_1);
              if (!$5_1) {
               break label$82
              }
             }
             $35($5_1, 24, $8_1);
             $2_1 = $31($1_1, 16);
             if ($2_1) {
              $35($5_1, 16, $2_1);
              $35($2_1, 24, $5_1);
             }
             $2_1 = $31($1_1, 20);
             if (!$2_1) {
              break label$82
             }
             $35($5_1 + 20 | 0, 0, $2_1);
             $35($2_1, 24, $5_1);
            }
            $1_1 = $1_1 + $10_1 | 0;
            $0 = $0 + $10_1 | 0;
           }
           $35($1_1, 4, $31($1_1, 4) & -2);
           $35($7, 4, $0 | 1);
           $35($0 + $7 | 0, 0, $0);
           if ($0 >>> 0 <= 255) {
            $1_1 = $0 >>> 3 | 0;
            $0 = ($1_1 << 3) + 63596 | 0;
            $2_1 = $31(0, 63556);
            $1_1 = 1 << $1_1;
            label$94 : {
             if (!($2_1 & $1_1)) {
              $35(0, 63556, $1_1 | $2_1);
              $1_1 = $0;
              break label$94;
             }
             $1_1 = $31($0, 8);
            }
            $35($0, 8, $7);
            $35($1_1, 12, $7);
            $35($7, 12, $0);
            $35($7, 8, $1_1);
            break label$70;
           }
           $5_1 = $7;
           $1_1 = $0 >>> 8 | 0;
           $2_1 = 0;
           label$96 : {
            if (!$1_1) {
             break label$96
            }
            $2_1 = 31;
            if ($0 >>> 0 > 16777215) {
             break label$96
            }
            $3_1 = $1_1 + 1048320 >>> 16 & 8;
            $2_1 = $1_1 << $3_1;
            $1_1 = $2_1 + 520192 >>> 16 & 4;
            $4_1 = $2_1 << $1_1;
            $2_1 = $4_1 + 245760 >>> 16 & 2;
            $1_1 = ($4_1 << $2_1 >>> 15 | 0) - ($2_1 | ($1_1 | $3_1)) | 0;
            $2_1 = ($1_1 << 1 | $0 >>> $1_1 + 21 & 1) + 28 | 0;
           }
           $1_1 = $2_1;
           $35($5_1, 28, $1_1);
           $36($7, 16, 0, 0);
           $4_1 = ($1_1 << 2) + 63860 | 0;
           $2_1 = $31(0, 63560);
           $3_1 = 1 << $1_1;
           label$97 : {
            if (!($2_1 & $3_1)) {
             $35(0, 63560, $2_1 | $3_1);
             $35($4_1, 0, $7);
             break label$97;
            }
            $2_1 = $0 << (($1_1 | 0) == 31 ? 0 : 25 - ($1_1 >>> 1 | 0) | 0);
            $1_1 = $31($4_1, 0);
            while (1) {
             $4_1 = $1_1;
             if (($31($1_1, 4) & -8) == ($0 | 0)) {
              break label$71
             }
             $1_1 = $2_1 >>> 29 | 0;
             $2_1 = $2_1 << 1;
             $3_1 = ($4_1 + ($1_1 & 4) | 0) + 16 | 0;
             $1_1 = $31($3_1, 0);
             if ($1_1) {
              continue
             }
             break;
            };
            $35($3_1, 0, $7);
           }
           $35($7, 24, $4_1);
           $35($7, 12, $7);
           $35($7, 8, $7);
           break label$70;
          }
          $0 = $5_1 + -40 | 0;
          $2_1 = $1_1 + 8 & 7 ? -8 - $1_1 & 7 : 0;
          $7 = $0 - $2_1 | 0;
          $35(0, 63568, $7);
          $2_1 = $1_1 + $2_1 | 0;
          $35(0, 63580, $2_1);
          $35($2_1, 4, $7 | 1);
          $35($0 + $1_1 | 0, 4, 40);
          $35(0, 63584, $31(0, 64044));
          $0 = ($4_1 + ($4_1 + -39 & 7 ? 39 - $4_1 & 7 : 0) | 0) + -47 | 0;
          $2_1 = $0 >>> 0 < $3_1 + 16 >>> 0 ? $3_1 : $0;
          $35($2_1, 4, 27);
          $36($2_1 + 16 | 0, 0, $32(64012), i64toi32_i32$HIGH_BITS);
          $36($2_1, 8, $32(64004), i64toi32_i32$HIGH_BITS);
          $35(0, 64012, $2_1 + 8 | 0);
          $35(0, 64008, $5_1);
          $35(0, 64004, $1_1);
          $35(0, 64016, 0);
          $0 = $2_1 + 24 | 0;
          while (1) {
           $35($0, 4, 7);
           $1_1 = $0 + 8 | 0;
           $0 = $0 + 4 | 0;
           if ($4_1 >>> 0 > $1_1 >>> 0) {
            continue
           }
           break;
          };
          if (($2_1 | 0) == ($3_1 | 0)) {
           break label$61
          }
          $35($2_1, 4, $31($2_1, 4) & -2);
          $4_1 = $2_1 - $3_1 | 0;
          $35($3_1, 4, $4_1 | 1);
          $35($2_1, 0, $4_1);
          if ($4_1 >>> 0 <= 255) {
           $1_1 = $4_1 >>> 3 | 0;
           $0 = ($1_1 << 3) + 63596 | 0;
           $2_1 = $31(0, 63556);
           $1_1 = 1 << $1_1;
           label$102 : {
            if (!($2_1 & $1_1)) {
             $35(0, 63556, $1_1 | $2_1);
             $1_1 = $0;
             break label$102;
            }
            $1_1 = $31($0, 8);
           }
           $35($0, 8, $3_1);
           $35($1_1, 12, $3_1);
           $35($3_1, 12, $0);
           $35($3_1, 8, $1_1);
           break label$61;
          }
          $36($3_1, 16, 0, 0);
          $7 = $3_1 + 28 | 0;
          $0 = $4_1 >>> 8 | 0;
          $1_1 = 0;
          label$104 : {
           if (!$0) {
            break label$104
           }
           $1_1 = 31;
           if ($4_1 >>> 0 > 16777215) {
            break label$104
           }
           $2_1 = $0 + 1048320 >>> 16 & 8;
           $1_1 = $0 << $2_1;
           $0 = $1_1 + 520192 >>> 16 & 4;
           $5_1 = $1_1 << $0;
           $1_1 = $5_1 + 245760 >>> 16 & 2;
           $0 = ($5_1 << $1_1 >>> 15 | 0) - ($1_1 | ($0 | $2_1)) | 0;
           $1_1 = ($0 << 1 | $4_1 >>> $0 + 21 & 1) + 28 | 0;
          }
          $0 = $1_1;
          $35($7, 0, $0);
          $1_1 = ($0 << 2) + 63860 | 0;
          $2_1 = $31(0, 63560);
          $5_1 = 1 << $0;
          label$105 : {
           if (!($2_1 & $5_1)) {
            $35(0, 63560, $2_1 | $5_1);
            $35($1_1, 0, $3_1);
            $35($3_1 + 24 | 0, 0, $1_1);
            break label$105;
           }
           $0 = $4_1 << (($0 | 0) == 31 ? 0 : 25 - ($0 >>> 1 | 0) | 0);
           $1_1 = $31($1_1, 0);
           while (1) {
            $2_1 = $1_1;
            if (($4_1 | 0) == ($31($1_1, 4) & -8)) {
             break label$69
            }
            $1_1 = $0 >>> 29 | 0;
            $0 = $0 << 1;
            $5_1 = ($2_1 + ($1_1 & 4) | 0) + 16 | 0;
            $1_1 = $31($5_1, 0);
            if ($1_1) {
             continue
            }
            break;
           };
           $35($5_1, 0, $3_1);
           $35($3_1 + 24 | 0, 0, $2_1);
          }
          $35($3_1, 12, $3_1);
          $35($3_1, 8, $3_1);
          break label$61;
         }
         $0 = $31($4_1, 8);
         $35($0, 12, $7);
         $35($4_1, 8, $7);
         $35($7, 24, 0);
         $35($7, 12, $4_1);
         $35($7, 8, $0);
        }
        $0 = $9_1 + 8 | 0;
        break label$1;
       }
       $0 = $31($2_1, 8);
       $35($0, 12, $3_1);
       $35($2_1, 8, $3_1);
       $35($3_1 + 24 | 0, 0, 0);
       $35($3_1, 12, $2_1);
       $35($3_1, 8, $0);
      }
      $0 = $31(0, 63568);
      if ($0 >>> 0 <= $6_1 >>> 0) {
       break label$4
      }
      $1_1 = $0 - $6_1 | 0;
      $35(0, 63568, $1_1);
      $0 = $31(0, 63580);
      $2_1 = $0 + $6_1 | 0;
      $35(0, 63580, $2_1);
      $35($2_1, 4, $1_1 | 1);
      $35($0, 4, $6_1 | 3);
      $0 = $0 + 8 | 0;
      break label$1;
     }
     $35(63552, 0, 48);
     $0 = 0;
     break label$1;
    }
    label$108 : {
     if (!$8_1) {
      break label$108
     }
     $0 = $31($4_1, 28);
     $2_1 = ($0 << 2) + 63860 | 0;
     label$109 : {
      if (($31($2_1, 0) | 0) == ($4_1 | 0)) {
       $35($2_1, 0, $1_1);
       if ($1_1) {
        break label$109
       }
       $7 = __wasm_rotl_i32($0) & $7;
       $35(0, 63560, $7);
       break label$108;
      }
      $35($8_1 + (($31($8_1, 16) | 0) == ($4_1 | 0) ? 16 : 20) | 0, 0, $1_1);
      if (!$1_1) {
       break label$108
      }
     }
     $35($1_1, 24, $8_1);
     $0 = $31($4_1, 16);
     if ($0) {
      $35($1_1, 16, $0);
      $35($0, 24, $1_1);
     }
     $0 = $31($4_1 + 20 | 0, 0);
     if (!$0) {
      break label$108
     }
     $35($1_1 + 20 | 0, 0, $0);
     $35($0, 24, $1_1);
    }
    label$112 : {
     if ($3_1 >>> 0 <= 15) {
      $0 = $3_1 + $6_1 | 0;
      $35($4_1, 4, $0 | 3);
      $0 = $0 + $4_1 | 0;
      $35($0, 4, $31($0, 4) | 1);
      break label$112;
     }
     $35($4_1, 4, $6_1 | 3);
     $1_1 = $4_1 + $6_1 | 0;
     $35($1_1, 4, $3_1 | 1);
     $35($1_1 + $3_1 | 0, 0, $3_1);
     if ($3_1 >>> 0 <= 255) {
      $2_1 = $3_1 >>> 3 | 0;
      $0 = ($2_1 << 3) + 63596 | 0;
      $3_1 = $31(0, 63556);
      $2_1 = 1 << $2_1;
      label$115 : {
       if (!($3_1 & $2_1)) {
        $35(0, 63556, $2_1 | $3_1);
        $2_1 = $0;
        break label$115;
       }
       $2_1 = $31($0, 8);
      }
      $35($0, 8, $1_1);
      $35($2_1, 12, $1_1);
      $35($1_1, 12, $0);
      $35($1_1, 8, $2_1);
      break label$112;
     }
     $8_1 = $1_1;
     $0 = $3_1 >>> 8 | 0;
     $2_1 = 0;
     label$117 : {
      if (!$0) {
       break label$117
      }
      $2_1 = 31;
      if ($3_1 >>> 0 > 16777215) {
       break label$117
      }
      $6_1 = $0 + 1048320 >>> 16 & 8;
      $2_1 = $0 << $6_1;
      $0 = $2_1 + 520192 >>> 16 & 4;
      $5_1 = $2_1 << $0;
      $2_1 = $5_1 + 245760 >>> 16 & 2;
      $0 = ($5_1 << $2_1 >>> 15 | 0) - ($2_1 | ($0 | $6_1)) | 0;
      $2_1 = ($0 << 1 | $3_1 >>> $0 + 21 & 1) + 28 | 0;
     }
     $0 = $2_1;
     $35($8_1, 28, $0);
     $36($1_1, 16, 0, 0);
     $2_1 = ($0 << 2) + 63860 | 0;
     label$118 : {
      $6_1 = 1 << $0;
      label$119 : {
       if (!($6_1 & $7)) {
        $35(0, 63560, $6_1 | $7);
        $35($2_1, 0, $1_1);
        break label$119;
       }
       $0 = $3_1 << (($0 | 0) == 31 ? 0 : 25 - ($0 >>> 1 | 0) | 0);
       $6_1 = $31($2_1, 0);
       while (1) {
        $2_1 = $6_1;
        if (($31($2_1, 4) & -8) == ($3_1 | 0)) {
         break label$118
        }
        $6_1 = $0 >>> 29 | 0;
        $0 = $0 << 1;
        $5_1 = ($2_1 + ($6_1 & 4) | 0) + 16 | 0;
        $6_1 = $31($5_1, 0);
        if ($6_1) {
         continue
        }
        break;
       };
       $35($5_1, 0, $1_1);
      }
      $35($1_1, 24, $2_1);
      $35($1_1, 12, $1_1);
      $35($1_1, 8, $1_1);
      break label$112;
     }
     $0 = $31($2_1, 8);
     $35($0, 12, $1_1);
     $35($2_1, 8, $1_1);
     $35($1_1, 24, 0);
     $35($1_1, 12, $2_1);
     $35($1_1, 8, $0);
    }
    $0 = $4_1 + 8 | 0;
    break label$1;
   }
   label$122 : {
    if (!$9_1) {
     break label$122
    }
    $0 = $31($1_1, 28);
    $3_1 = ($0 << 2) + 63860 | 0;
    label$123 : {
     if (($31($3_1, 0) | 0) == ($1_1 | 0)) {
      $35($3_1, 0, $4_1);
      if ($4_1) {
       break label$123
      }
      $35(0, 63560, __wasm_rotl_i32($0) & $10_1);
      break label$122;
     }
     $35((($31($9_1, 16) | 0) == ($1_1 | 0) ? 16 : 20) + $9_1 | 0, 0, $4_1);
     if (!$4_1) {
      break label$122
     }
    }
    $35($4_1, 24, $9_1);
    $0 = $31($1_1, 16);
    if ($0) {
     $35($4_1, 16, $0);
     $35($0, 24, $4_1);
    }
    $0 = $31($1_1 + 20 | 0, 0);
    if (!$0) {
     break label$122
    }
    $35($4_1 + 20 | 0, 0, $0);
    $35($0, 24, $4_1);
   }
   label$126 : {
    if ($2_1 >>> 0 <= 15) {
     $0 = $2_1 + $6_1 | 0;
     $35($1_1, 4, $0 | 3);
     $0 = $0 + $1_1 | 0;
     $35($0, 4, $31($0, 4) | 1);
     break label$126;
    }
    $35($1_1, 4, $6_1 | 3);
    $4_1 = $1_1 + $6_1 | 0;
    $35($4_1, 4, $2_1 | 1);
    $35($2_1 + $4_1 | 0, 0, $2_1);
    if ($8_1) {
     $6_1 = $8_1 >>> 3 | 0;
     $0 = ($6_1 << 3) + 63596 | 0;
     $3_1 = $31(0, 63576);
     $6_1 = 1 << $6_1;
     label$129 : {
      if (!($6_1 & $5_1)) {
       $35(0, 63556, $5_1 | $6_1);
       $6_1 = $0;
       break label$129;
      }
      $6_1 = $31($0, 8);
     }
     $35($0, 8, $3_1);
     $35($6_1, 12, $3_1);
     $35($3_1, 12, $0);
     $35($3_1, 8, $6_1);
    }
    $35(0, 63576, $4_1);
    $35(0, 63564, $2_1);
   }
   $0 = $1_1 + 8 | 0;
  }
  global$0 = $11_1 + 16 | 0;
  return $0 | 0;
 }
 
 function $23($0) {
  $0 = $0 | 0;
  var $1_1 = 0, $2_1 = 0, $3_1 = 0, $4_1 = 0, $5_1 = 0, $6_1 = 0, $7 = 0;
  label$1 : {
   if (!$0) {
    break label$1
   }
   $3_1 = $0 + -8 | 0;
   $1_1 = $31($0 + -4 | 0, 0);
   $0 = $1_1 & -8;
   $6_1 = $3_1 + $0 | 0;
   label$2 : {
    if ($1_1 & 1) {
     break label$2
    }
    if (!($1_1 & 3)) {
     break label$1
    }
    $1_1 = $31($3_1, 0);
    $3_1 = $3_1 - $1_1 | 0;
    $2_1 = $31(0, 63572);
    if ($3_1 >>> 0 < $2_1 >>> 0) {
     break label$1
    }
    $0 = $0 + $1_1 | 0;
    if (($31(0, 63576) | 0) != ($3_1 | 0)) {
     if ($1_1 >>> 0 <= 255) {
      $4_1 = $31($3_1, 12);
      $2_1 = $31($3_1, 8);
      $1_1 = $1_1 >>> 3 | 0;
      if (($2_1 | 0) == ($4_1 | 0)) {
       $35(0, 63556, $31(0, 63556) & __wasm_rotl_i32($1_1));
       break label$2;
      }
      $35($2_1, 12, $4_1);
      $35($4_1, 8, $2_1);
      break label$2;
     }
     $7 = $31($3_1, 24);
     $1_1 = $31($3_1, 12);
     label$6 : {
      if (($1_1 | 0) != ($3_1 | 0)) {
       $5_1 = $2_1;
       $2_1 = $31($3_1, 8);
       if ($5_1 >>> 0 <= $2_1 >>> 0) {
        $31($2_1, 12)
       }
       $35($2_1, 12, $1_1);
       $35($1_1, 8, $2_1);
       break label$6;
      }
      label$9 : {
       $4_1 = $3_1 + 20 | 0;
       $2_1 = $31($4_1, 0);
       if ($2_1) {
        break label$9
       }
       $4_1 = $3_1 + 16 | 0;
       $2_1 = $31($4_1, 0);
       if ($2_1) {
        break label$9
       }
       $1_1 = 0;
       break label$6;
      }
      while (1) {
       $5_1 = $4_1;
       $1_1 = $2_1;
       $4_1 = $1_1 + 20 | 0;
       $2_1 = $31($4_1, 0);
       if ($2_1) {
        continue
       }
       $4_1 = $1_1 + 16 | 0;
       $2_1 = $31($1_1, 16);
       if ($2_1) {
        continue
       }
       break;
      };
      $35($5_1, 0, 0);
     }
     if (!$7) {
      break label$2
     }
     $4_1 = $31($3_1, 28);
     $2_1 = ($4_1 << 2) + 63860 | 0;
     label$11 : {
      if (($31($2_1, 0) | 0) == ($3_1 | 0)) {
       $35($2_1, 0, $1_1);
       if ($1_1) {
        break label$11
       }
       $35(0, 63560, $31(0, 63560) & __wasm_rotl_i32($4_1));
       break label$2;
      }
      $35($7 + (($31($7, 16) | 0) == ($3_1 | 0) ? 16 : 20) | 0, 0, $1_1);
      if (!$1_1) {
       break label$2
      }
     }
     $35($1_1, 24, $7);
     $2_1 = $31($3_1, 16);
     if ($2_1) {
      $35($1_1, 16, $2_1);
      $35($2_1, 24, $1_1);
     }
     $2_1 = $31($3_1, 20);
     if (!$2_1) {
      break label$2
     }
     $35($1_1 + 20 | 0, 0, $2_1);
     $35($2_1, 24, $1_1);
     break label$2;
    }
    $1_1 = $31($6_1, 4);
    if (($1_1 & 3) != 3) {
     break label$2
    }
    $35(0, 63564, $0);
    $35($6_1, 4, $1_1 & -2);
    $35($3_1, 4, $0 | 1);
    $35($0 + $3_1 | 0, 0, $0);
    return;
   }
   if ($6_1 >>> 0 <= $3_1 >>> 0) {
    break label$1
   }
   $1_1 = $31($6_1, 4);
   if (!($1_1 & 1)) {
    break label$1
   }
   label$14 : {
    if (!($1_1 & 2)) {
     if (($31(0, 63580) | 0) == ($6_1 | 0)) {
      $35(0, 63580, $3_1);
      $0 = $31(0, 63568) + $0 | 0;
      $35(0, 63568, $0);
      $35($3_1, 4, $0 | 1);
      if (($31(0, 63576) | 0) != ($3_1 | 0)) {
       break label$1
      }
      $35(0, 63564, 0);
      $35(0, 63576, 0);
      return;
     }
     if (($31(0, 63576) | 0) == ($6_1 | 0)) {
      $35(0, 63576, $3_1);
      $0 = $31(0, 63564) + $0 | 0;
      $35(0, 63564, $0);
      $35($3_1, 4, $0 | 1);
      $35($0 + $3_1 | 0, 0, $0);
      return;
     }
     $0 = ($1_1 & -8) + $0 | 0;
     label$18 : {
      if ($1_1 >>> 0 <= 255) {
       $5_1 = $31($6_1, 12);
       $4_1 = $31($6_1, 8);
       $2_1 = $1_1 >>> 3 | 0;
       $1_1 = ($2_1 << 3) + 63596 | 0;
       if (($4_1 | 0) != ($1_1 | 0)) {
        $31(0, 63572)
       }
       if (($4_1 | 0) == ($5_1 | 0)) {
        $35(0, 63556, $31(0, 63556) & __wasm_rotl_i32($2_1));
        break label$18;
       }
       if (($1_1 | 0) != ($5_1 | 0)) {
        $31(0, 63572)
       }
       $35($4_1, 12, $5_1);
       $35($5_1, 8, $4_1);
       break label$18;
      }
      $7 = $31($6_1, 24);
      $1_1 = $31($6_1, 12);
      label$23 : {
       if (($6_1 | 0) != ($1_1 | 0)) {
        $5_1 = $31(0, 63572);
        $2_1 = $31($6_1, 8);
        if ($5_1 >>> 0 <= $2_1 >>> 0) {
         $31($2_1, 12)
        }
        $35($2_1, 12, $1_1);
        $35($1_1, 8, $2_1);
        break label$23;
       }
       label$26 : {
        $4_1 = $6_1 + 20 | 0;
        $2_1 = $31($4_1, 0);
        if ($2_1) {
         break label$26
        }
        $4_1 = $6_1 + 16 | 0;
        $2_1 = $31($4_1, 0);
        if ($2_1) {
         break label$26
        }
        $1_1 = 0;
        break label$23;
       }
       while (1) {
        $5_1 = $4_1;
        $1_1 = $2_1;
        $4_1 = $1_1 + 20 | 0;
        $2_1 = $31($4_1, 0);
        if ($2_1) {
         continue
        }
        $4_1 = $1_1 + 16 | 0;
        $2_1 = $31($1_1, 16);
        if ($2_1) {
         continue
        }
        break;
       };
       $35($5_1, 0, 0);
      }
      if (!$7) {
       break label$18
      }
      $4_1 = $31($6_1, 28);
      $2_1 = ($4_1 << 2) + 63860 | 0;
      label$28 : {
       if (($31($2_1, 0) | 0) == ($6_1 | 0)) {
        $35($2_1, 0, $1_1);
        if ($1_1) {
         break label$28
        }
        $35(0, 63560, $31(0, 63560) & __wasm_rotl_i32($4_1));
        break label$18;
       }
       $35($7 + (($31($7, 16) | 0) == ($6_1 | 0) ? 16 : 20) | 0, 0, $1_1);
       if (!$1_1) {
        break label$18
       }
      }
      $35($1_1, 24, $7);
      $2_1 = $31($6_1, 16);
      if ($2_1) {
       $35($1_1, 16, $2_1);
       $35($2_1, 24, $1_1);
      }
      $2_1 = $31($6_1, 20);
      if (!$2_1) {
       break label$18
      }
      $35($1_1 + 20 | 0, 0, $2_1);
      $35($2_1, 24, $1_1);
     }
     $35($3_1, 4, $0 | 1);
     $35($0 + $3_1 | 0, 0, $0);
     if (($31(0, 63576) | 0) != ($3_1 | 0)) {
      break label$14
     }
     $35(0, 63564, $0);
     return;
    }
    $35($6_1, 4, $1_1 & -2);
    $35($3_1, 4, $0 | 1);
    $35($0 + $3_1 | 0, 0, $0);
   }
   if ($0 >>> 0 <= 255) {
    $0 = $0 >>> 3 | 0;
    $1_1 = ($0 << 3) + 63596 | 0;
    $2_1 = $31(0, 63556);
    $0 = 1 << $0;
    label$32 : {
     if (!($2_1 & $0)) {
      $35(0, 63556, $0 | $2_1);
      $0 = $1_1;
      break label$32;
     }
     $0 = $31($1_1, 8);
    }
    $35($1_1, 8, $3_1);
    $35($0, 12, $3_1);
    $35($3_1, 12, $1_1);
    $35($3_1, 8, $0);
    return;
   }
   $36($3_1, 16, 0, 0);
   $4_1 = $3_1 + 28 | 0;
   $2_1 = $0 >>> 8 | 0;
   $1_1 = 0;
   label$34 : {
    if (!$2_1) {
     break label$34
    }
    $1_1 = 31;
    if ($0 >>> 0 > 16777215) {
     break label$34
    }
    $1_1 = $2_1;
    $2_1 = $2_1 + 1048320 >>> 16 & 8;
    $1_1 = $1_1 << $2_1;
    $7 = $1_1 + 520192 >>> 16 & 4;
    $1_1 = $1_1 << $7;
    $5_1 = $1_1 + 245760 >>> 16 & 2;
    $1_1 = ($1_1 << $5_1 >>> 15 | 0) - ($5_1 | ($2_1 | $7)) | 0;
    $1_1 = ($1_1 << 1 | $0 >>> $1_1 + 21 & 1) + 28 | 0;
   }
   $35($4_1, 0, $1_1);
   $5_1 = ($1_1 << 2) + 63860 | 0;
   label$35 : {
    label$36 : {
     $4_1 = $31(0, 63560);
     $2_1 = 1 << $1_1;
     label$37 : {
      if (!($4_1 & $2_1)) {
       $35(0, 63560, $2_1 | $4_1);
       $35($5_1, 0, $3_1);
       $35($3_1 + 24 | 0, 0, $5_1);
       break label$37;
      }
      $4_1 = $0 << (($1_1 | 0) == 31 ? 0 : 25 - ($1_1 >>> 1 | 0) | 0);
      $1_1 = $31($5_1, 0);
      while (1) {
       $2_1 = $1_1;
       if (($31($1_1, 4) & -8) == ($0 | 0)) {
        break label$36
       }
       $1_1 = $4_1 >>> 29 | 0;
       $4_1 = $4_1 << 1;
       $5_1 = ($2_1 + ($1_1 & 4) | 0) + 16 | 0;
       $1_1 = $31($5_1, 0);
       if ($1_1) {
        continue
       }
       break;
      };
      $35($5_1, 0, $3_1);
      $35($3_1 + 24 | 0, 0, $2_1);
     }
     $35($3_1, 12, $3_1);
     $35($3_1, 8, $3_1);
     break label$35;
    }
    $0 = $31($2_1, 8);
    $35($0, 12, $3_1);
    $35($2_1, 8, $3_1);
    $35($3_1 + 24 | 0, 0, 0);
    $35($3_1, 12, $2_1);
    $35($3_1, 8, $0);
   }
   $0 = $31(0, 63588) + -1 | 0;
   $35(0, 63588, $0);
   if ($0) {
    break label$1
   }
   $3_1 = 64012;
   while (1) {
    $0 = $31($3_1, 0);
    $3_1 = $0 + 8 | 0;
    if ($0) {
     continue
    }
    break;
   };
   $35(0, 63588, -1);
  }
 }
 
 function $24() {
  return global$0 | 0;
 }
 
 function $25($0) {
  $0 = $0 | 0;
  $0 = global$0 - $0 & -16;
  global$0 = $0;
  return $0 | 0;
 }
 
 function $26($0) {
  $0 = $0 | 0;
  global$0 = $0;
 }
 
 function $27($0) {
  $0 = $0 | 0;
  return abort() | 0;
 }
 
 function $28($0, $1_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 1 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  return HEAPU8[$0 | 0];
 }
 
 function $29($0) {
  if ($0 >>> 0 < 1024 | $0 + 2 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 1) {
   fimport$3()
  }
  return HEAPU16[$0 >> 1];
 }
 
 function $30($0) {
  if ($0 >>> 0 < 1024 | $0 + 4 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  return HEAPU8[$0 | 0] | HEAPU8[$0 + 1 | 0] << 8 | (HEAPU8[$0 + 2 | 0] << 16 | HEAPU8[$0 + 3 | 0] << 24);
 }
 
 function $31($0, $1_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 4 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 3) {
   fimport$3()
  }
  return HEAP32[$0 >> 2];
 }
 
 function $32($0) {
  if ($0 >>> 0 < 1024 | $0 + 8 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 3) {
   fimport$3()
  }
  i64toi32_i32$HIGH_BITS = HEAP32[$0 + 4 >> 2];
  return HEAP32[$0 >> 2];
 }
 
 function $33($0, $1_1, $2_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 1 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  HEAP8[$0 | 0] = $2_1;
 }
 
 function $34($0, $1_1) {
  $0 = $0 + 28704 | 0;
  if ($0 >>> 0 < 1024 | $0 + 2 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 1) {
   fimport$3()
  }
  HEAP16[$0 >> 1] = $1_1;
 }
 
 function $35($0, $1_1, $2_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 4 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 3) {
   fimport$3()
  }
  HEAP32[$0 >> 2] = $2_1;
 }
 
 function $36($0, $1_1, $2_1, $3_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 8 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 3) {
   fimport$3()
  }
  HEAP32[$0 >> 2] = $2_1;
  HEAP32[$0 + 4 >> 2] = $3_1;
 }
 
 function $37($0, $1_1) {
  $0 = $0 + $1_1 | 0;
  if ($0 >>> 0 < 1024 | $0 + 8 >>> 0 > HEAPU32[16016]) {
   fimport$2()
  }
  if ($0 & 7) {
   fimport$3()
  }
  HEAP32[$0 >> 2] = 0;
  HEAP32[$0 + 4 >> 2] = 0;
 }
 
 function __wasm_rotl_i32($0) {
  var $1_1 = 0;
  $1_1 = $0 & 31;
  $0 = 0 - $0 & 31;
  return (-1 >>> $1_1 & -2) << $1_1 | (-1 << $0 & -2) >>> $0;
 }
 
 // EMSCRIPTEN_END_FUNCS;
 var FUNCTION_TABLE = [];
 function __wasm_memory_size() {
  return buffer.byteLength / 65536 | 0;
 }
 
 return {
  "__wasm_call_ctors": $1, 
  "Huff_Decompress": $13, 
  "MSG_initHuffman": $14, 
  "Huff_Compress": $16, 
  "HuffmanGetSymbol": $17, 
  "__errno_location": $20, 
  "malloc": $22, 
  "free": $23, 
  "stackSave": $24, 
  "stackAlloc": $25, 
  "stackRestore": $26, 
  "__growWasmMemory": $27
 };
}

var writeSegment = (
    function(mem) {
      var _mem = new Uint8Array(mem);
      return function(offset, s) {
        var bytes, i;
        if (typeof Buffer === 'undefined') {
          bytes = atob(s);
          for (i = 0; i < bytes.length; i++)
            _mem[offset + i] = bytes.charCodeAt(i);
        } else {
          bytes = Buffer.from(s, 'base64');
          for (i = 0; i < bytes.length; i++)
            _mem[offset + i] = bytes[i];
        }
      }
    }
  )(wasmMemory.buffer);
writeSegment(1024, "0AmGCAACywpDB/gKAAI4CX4HxAcAAgsHbgg2CQAChAgGBwUIAAIMBysKCAUAAnYI7Qh3CAACAQVoBs8KAAKABsMGRAgAAuQK/wYQBwACMAdpCIEHAAKUCqMKDQYAAv4HZQedCgACKgoJBwgFAAIgBrAJDggAAgEFYQiXCgACBwZ1B2YIAAIdCCIJygoAAnIIAgfoBwACeAiCB48JAAIeCUEHRQgAAjQIwAgIBQACiggRCXQHAAIBBWgGwggAAoAGwwapCgACSQr/Bn8HAAKICH0HCgcAApYKAwcNBgACfAhHCeUKAAJ7CIsICAUAAiAGLwhzCAACAQVCB4MHAAIHBkYJjgkAAoUIQwdwCAAChwh+B8QHAAILB3EIvwoAAm0JBgfhCQACDAcoCQgFAAJkCWwI4goAAgEFaAZRCgACgAbDBgQIAAJ6CP8GEAcAAjAHWwmBBwACGgoPCQ0GAAL+B2UHNQgAAm8ICQcIBQACIAbHCNUKAAIBBT0JagkAAgcGdQfdCgACMQhrCbQKAAJcCQIH6AcAAq4KggetCgACQAhBB+AJAAIhCh8ICAUAAlgKXwh0BwACAQVoBtIKAAKABsMGeQgAApoK/wZ/BwACiQh9BwoHAAJeCQMHDQYAAjII8Qo5CQACYAkYCQgFAAIgBt8KmQoAAgEFQgeDBwACBwYAC4YIAALBCUMHXQkAAuoKfgfEBwACCwduCNQKAAKECAYHBQgAAgwHoAkIBQACdgjtCHcIAAIBBWgGzAoAAoAGwwZECAACPAr/BhAHAAIwB2kIgQcAApwKUAkNBgAC/gdlB5AJAAKiCgkHCAUAAiAGqAkOCAACAQVhCEgJAAIHBnUHZggAAh0IPgmVCgACcggCB+gHAAJ4CIIHrwoAAmcJQQdFCAACNAjACAgFAAKKCNsKdAcAAgEFaAbCCAACgAbDBjcJAAKRCv8GfwcAAogIfQcKBwACUgoDBw0GAAJ8CFoJVQoAAnsIiwgIBQACIAYvCHMIAAIBBUIHgwcAAgcGkgnTCgAChQhDB3AIAAKHCH4HxAcAAgsHcQgZCgACYwkGBzsKAAIMB9kKCAUAAjMJbAg6CQACAQVoBkwJAAKABsMGBAgAAnoI/wYQBwACMAf7CoEHAAJiCSkKDQYAAv4HZQc1CAACbwgJBwgFAAIgBscInwoAAgEFuwrYCgACBwZ1B6UKAAIxCI0JzQoAAk4KAgfoBwACjAmCB9YKAAJACEEHpwoAAlQKHwgIBQAC8ApfCHQHAAIBBWgGFAoAAoAGwwZ5CAACVgr/Bn8HAAKJCH0HCgcAAhwKAwcNBgACMggSCcgJAAI/Ci4JCAUAAiAGEwmhCgACAQVCB4MHAAIHBtAJhggAAroKQwfuCgACOAl+B8QHAAILB24INgkAAoQIBgcFCAACDAe5CggFAAJ2CO0IdwgAAgEFaAb2CgACgAbDBkQIAALsCv8GEAcAAjAHaQiBBwACFgrJCg0GAAL+B2UH6QoAAlcKCQcIBQACIAawCQ4IAAIBBWEIsgoAAgcGdQdmCAACHQgiCZsKAAJyCAIH6AcAAngIggePCQACHglBB0UIAAI0CMAICAUAAooIEQl0BwACAQVoBsIIAAKABsMG6woAAr4K/wZ/BwACiAh9BwoHAAKsCgMHDQYAAnwIRwksCgACewiLCAgFAAIgBi8IcwgAAgEFQgeDBwACBwZGCY4JAAKFCEMHcAgAAocIfgfEBwACCwdxCP0KAAJtCQYH4QkAAgwHKAkIBQACZAlsCLcKAAIBBWgGvAoAAoAGwwYECAACegj/BhAHAAIwB1sJgQcAAk0KDwkNBgAC/gdlBzUIAAJvCAkHCAUAAiAGxwj8CgACAQU9CWoJAAIHBnUH7woAAjEIawnyCgACXAkCB+gHAAJZCoIHIwoAAkAIQQfgCQAC5gofCAgFAAJKCl8IdAcAAgEFaAakCgACgAbDBnkIAAKqCv8GfwcAAokIfQcKBwACXgkDBw0GAAIyCC0KOQkAAmAJGAkIBQACIAbjCvoKAAIBBUIHgwcAAgcG8wqGCAACwQlDB10JAALGCn4HxAcAAgsHbggmCgAChAgGBwUIAAIMB6AJCAUAAnYI7Qh3CAACAQVoBqYKAAKABsMGRAgAAhcK/wYQBwACMAdpCIEHAAKYClAJDQYAAv4HZQeQCQACTwoJBwgFAAIgBqgJDggAAgEFYQhICQACBwZ1B2YIAAIdCD4JqwoAAnIIAgfoBwACeAiCB84KAAJnCUEHRQgAAjQIwAgIBQACigj5CnQHAAIBBWgGwggAAoAGwwY3CQACGwr/Bn8HAAKICH0HCgcAAlMKAwcNBgACfAhaCUsKAAJ7CIsICAUAAiAGLwhzCAACAQVCB4MHAAIHBpIJvQoAAoUIQwdwCAAChwh+B8QHAAILB3EI2goAAmMJBgfnCgACDAfXCggFAAIzCWwIOgkAAgEFaAZMCQACgAbDBgQIAAJ6CP8GEAcAAjAH9QqBBwACYgm1Cg0GAAL+B2UHNQgAAm8ICQcIBQACIAbHCNEKAAIBBbMK3AoAAgcGdQexCgACMQiNCSQKAALFCgIH6AcAAowJggcnCgACQAhBB/QKAAK4Ch8ICAUAApMKXwh0BwACAQVoBt4KAAKABsMGeQgAArYK/wZ/BwACiQh9BwoHAAKeCgMHDQYAAjIIEgnICQACFQouCQgFAAIgBhMJJQoAAgEFQgeDBwACBwbQCYYIAALLCkMH+AoAAjgJfgfEBwACCwduCDYJAAKECAYHBQgAAgwHKwoIBQACdgjtCHcIAAIBBWgGzwoAAoAGwwZECAAC5Ar/BhAHAAIwB2kIgQcAApQKowoNBgAC/gdlB50KAAIqCgkHCAUAAiAGsAkOCAACAQVhCJcKAAIHBnUHZggAAh0IIgnKCgACcggCB+gHAAJ4CIIHjwkAAh4JQQdFCAACNAjACAgFAAKKCBEJdAcAAgEFaAbCCAACgAbDBqkKAAJJCv8GfwcAAogIfQcKBwAClgoDBw0GAAJ8CEcJ5QoAAnsIiwgIBQACIAYvCHMIAAIBBUIHgwcAAgcGRgmOCQAChQhDB3AIAAKHCH4HxAcAAgsHcQi/CgACbQkGB+EJAAIMBygJCAUAAmQJbAjiCgACAQVoBlEKAAKABsMGBAgAAnoI/wYQBwACMAdbCYEHAAIaCg8JDQYAAv4HZQc1CAACbwgJBwgFAAIgBscI1QoAAgEFPQlqCQACBwZ1B90KAAIxCGsJtAoAAlwJAgfoBwACrgqCB60KAAJACEEH4AkAAiEKHwgIBQACWApfCHQHAAIBBWgG0goAAoAGwwZ5CAACmgr/Bn8HAAKJCH0HCgcAAl4JAwcNBgACMgjxCjkJAAJgCRgJCAUAAiAG3wqZCgACAQVCB4MHAAIHBvcLhggAAsEJQwddCQAC6gp+B8QHAAILB24I1AoAAoQIBgcFCAACDAegCQgFAAJ2CO0IdwgAAgEFaAbMCgACgAbDBkQIAAI8Cv8GEAcAAjAHaQiBBwACnApQCQ0GAAL+B2UHkAkAAqIKCQcIBQACIAaoCQ4IAAIBBWEISAkAAgcGdQdmCAACHQg+CZUKAAJyCAIH6AcAAngIggevCgACZwlBB0UIAAI0CMAICAUAAooI2wp0BwACAQVoBsIIAAKABsMGNwkAApEK/wZ/BwACiAh9BwoHAAJSCgMHDQYAAnwIWglVCgACewiLCAgFAAIgBi8IcwgAAgEFQgeDBwACBwaSCdMKAAKFCEMHcAgAAocIfgfEBwACCwdxCBkKAAJjCQYHOwoAAgwH2QoIBQACMwlsCDoJAAIBBWgGTAkAAoAGwwYECAACegj/BhAHAAIwB/sKgQcAAmIJKQoNBgAC/gdlBzUIAAJvCAkHCAUAAiAGxwifCgACAQW7CtgKAAIHBnUHpQoAAjEIjQnNCgACTgoCB+gHAAKMCYIH1goAAkAIQQenCgACVAofCAgFAALwCl8IdAcAAgEFaAYUCgACgAbDBnkIAAJWCv8GfwcAAokIfQcKBwACHAoDBw0GAAIyCBIJyAkAAj8KLgkIBQACIAYTCaEKAAIBBUIHgwcAAgcG0AmGCAACugpDB+4KAAI4CX4HxAcAAgsHbgg2CQAChAgGBwUIAAIMB7kKCAUAAnYI7Qh3CAACAQVoBvYKAAKABsMGRAgAAuwK/wYQBwACMAdpCIEHAAIWCskKDQYAAv4HZQfpCgACVwoJBwgFAAIgBrAJDggAAgEFYQiyCgACBwZ1B2YIAAIdCCIJmwoAAnIIAgfoBwACeAiCB48JAAIeCUEHRQgAAjQIwAgIBQACiggRCXQHAAIBBWgGwggAAoAGwwbrCgACvgr/Bn8HAAKICH0HCgcAAqwKAwcNBgACfAhHCSwKAAJ7CIsICAUAAiAGLwhzCAACAQVCB4MHAAIHBkYJjgkAAoUIQwdwCAAChwh+B8QHAAILB3EI/QoAAm0JBgfhCQACDAcoCQgFAAJkCWwItwoAAgEFaAa8CgACgAbDBgQIAAJ6CP8GEAcAAjAHWwmBBwACTQoPCQ0GAAL+B2UHNQgAAm8ICQcIBQACIAbHCPwKAAIBBT0JagkAAgcGdQfvCgACMQhrCfIKAAJcCQIH6AcAAlkKggcjCgACQAhBB+AJAALmCh8ICAUAAkoKXwh0BwACAQVoBqQKAAKABsMGeQgAAqoK/wZ/BwACiQh9BwoHAAJeCQMHDQYAAjIILQo5CQACYAkYCQgFAAIgBuMK+goAAgEFQgeDBwACBwbzCoYIAALBCUMHXQkAAsYKfgfEBwACCwduCCYKAAKECAYHBQgAAgwHoAkIBQACdgjtCHcIAAIBBWgGpgoAAoAGwwZECAACFwr/BhAHAAIwB2kIgQcAApgKUAkNBgAC/gdlB5AJAAJPCgkHCAUAAiAGqAkOCAACAQVhCEgJAAIHBnUHZggAAh0IPgmrCgACcggCB+gHAAJ4CIIHzgoAAmcJQQdFCAACNAjACAgFAAKKCPkKdAcAAgEFaAbCCAACgAbDBjcJAAIbCv8GfwcAAogIfQcKBwACUwoDBw0GAAJ8CFoJSwoAAnsIiwgIBQACIAYvCHMIAAIBBUIHgwcAAgcGkgm9CgAChQhDB3AIAAKHCH4HxAcAAgsHcQjaCgACYwkGB+cKAAIMB9cKCAUAAjMJbAg6CQACAQVoBkwJAAKABsMGBAgAAnoI/wYQBwACMAf1CoEHAAJiCbUKDQYAAv4HZQc1CAACbwgJBwgFAAIgBscI0QoAAgEFswrcCgACBwZ1B7EKAAIxCI0JJAoAAsUKAgfoBwACjAmCBycKAAJACEEH9AoAArgKHwgIBQACkwpfCHQHAAIBBWgG3goAAoAGwwZ5CAACtgr/Bn8HAAKJCH0HCgcAAp4KAwcNBgACMggSCcgJAAIVCi4JCAUAAiAGEwklCgACAQVCB4MHAAIHBg==");
writeSegment(5120, "y9EDAOmgAACUGAAAwhsAAJIOAACmDgAA3hcAAPNaAAAlggAAJhsAAJ4eAADyJQAAKSQAAGtDAABtDwAA8gYAAGAgAABEBgAANgYAAH8GAABMBAAAvQQAANYEAABuBAAA1QYAACMEAADeBAAAfQQAAPkEAACGEQAA9QoAAJANAAA7VQAAhwQAAIYGAAAqBAAAEwQAAPQDAAAdBAAALgQAAL4GAAB4AwAAnAQAAFIDAADAAwAADAMAANgGAADgDAAAhikAAKIRAAD5FgAAfQoAACoSAAD9DgAALQgAAEsHAAAYCgAAnQcAALQHAACsAwAAbgQAAPwGAACGBgAAtgQAAFcWAADwFwAANhwAAP4ZAAB+DgAA0w4AANQFAAD0BQAApwgAAHQEAABLBQAAywMAAIQIAADgBAAAMAUAAKsEAADqBgAANgQAAPAEAADyBAAAkAQAAMUDAACDBAAAogQAAEMFAADMBAAA+QUAAEAGAAA5CgAAAAgAAPIJAADLDAAAagkAAAEOAADICQAA8AoAAHMKAAACGAAATw4AABgLAACtNwAAXAwAAK0IAACXBgAAiAwAALMKAAC4DQAAvBIAAPsPAAC7DQAAqBQAALAPAAABHwAAjxcAAPAUAABUDwAAHBMAAJ8OAADWEQAAxxIAANwWAAAAGQAAURgAAGMgAADLWgAAnh4AAKEbAADnIgAAPRUAAIMRAAA5DgAAiBQAAMAUAADQFAAA+hQAAKQNAACaCQAAngYAAB0HAABJCAAAfAcAAH0EAADsBQAAVwUAANQEAAAFBAAA6gQAAFAEAADdBAAA7gMAAH0EAAABBAAA2QQAALgDAAAHBQAA5QMAALEGAADxAwAAowQAAG8DAABLBAAAoQMAADYEAAC3AwAAeAYAAKIDAACBBAAABgQAAO4EAAAmBAAAvgQAACQEAABVBgAAogMAAFIEAACQAwAACgQAAHwDAACGBAAA3gMAAJcEAABSAwAAYQQAAIcDAAA/BAAAmAMAAHgEAAAgBAAAhg0AAMAIAAAtEQAAaC8AAE4eAABBBQAAGwUAAM4MAACeBwAAdgMAAP8DAABYBAAANQQAABIEAAAlBAAALwQAAMwFAADpAwAASAQAAJMDAAAcBAAA4wMAAC4EAABsAwAAVwQAAFMDAAAjBAAAJQMAAFgEAACbAwAATwQAADEDAABrBwAAUAcAANADAABJAwAAZwQAALwDAACHBAAAtgMAAG8eAAC6AwAACQUAAKUDAABnBAAAhwwAAPwDAACfAwAASwUAAAADAAAQBAAA6QIAALgDAAAlAwAAMQQAAOQCAAD1AwAAJQMAAPADAAAcAwAA5AMAACEEAADBLAAAwDQ=");
return asmFunc({
    'Int8Array': Int8Array,
    'Int16Array': Int16Array,
    'Int32Array': Int32Array,
    'Uint8Array': Uint8Array,
    'Uint16Array': Uint16Array,
    'Uint32Array': Uint32Array,
    'Float32Array': Float32Array,
    'Float64Array': Float64Array,
    'NaN': NaN,
    'Infinity': Infinity,
    'Math': Math
  },
  asmLibraryArg,
  wasmMemory.buffer
)

}
)(asmLibraryArg, wasmMemory, wasmTable);
    return {
      'exports': exports
    };
  },

  instantiate: /** @suppress{checkTypes} */ function(binary, info) {
    return {
      then: function(ok) {
        ok({
          'instance': new WebAssembly.Instance(new WebAssembly.Module(binary))
        });
      }
    };
  },

  RuntimeError: Error
};

// We don't need to actually download a wasm binary, mark it as present but empty.
wasmBinary = [];



if (typeof WebAssembly !== 'object') {
  err('no native wasm support detected');
}


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// In MINIMAL_RUNTIME, setValue() and getValue() are only available when building with safe heap enabled, for heap safety checking.
// In traditional runtime, setValue() and getValue() are always available (although their use is highly discouraged due to perf penalties)

/** @param {number} ptr
    @param {number} value
    @param {string} type
    @param {number|boolean=} noSafe */
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
  if (noSafe) {
    switch(type) {
      case 'i1': HEAP8[((ptr)>>0)]=value; break;
      case 'i8': HEAP8[((ptr)>>0)]=value; break;
      case 'i16': HEAP16[((ptr)>>1)]=value; break;
      case 'i32': HEAP32[((ptr)>>2)]=value; break;
      case 'i64': (tempI64 = [value>>>0,(tempDouble=value,(+(Math_abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math_min((+(Math_floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math_ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[((ptr)>>2)]=tempI64[0],HEAP32[(((ptr)+(4))>>2)]=tempI64[1]); break;
      case 'float': HEAPF32[((ptr)>>2)]=value; break;
      case 'double': HEAPF64[((ptr)>>3)]=value; break;
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
    switch(type) {
      case 'i1': SAFE_HEAP_STORE(((ptr)|0), ((value)|0), 1); break;
      case 'i8': SAFE_HEAP_STORE(((ptr)|0), ((value)|0), 1); break;
      case 'i16': SAFE_HEAP_STORE(((ptr)|0), ((value)|0), 2); break;
      case 'i32': SAFE_HEAP_STORE(((ptr)|0), ((value)|0), 4); break;
      case 'i64': (tempI64 = [value>>>0,(tempDouble=value,(+(Math_abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math_min((+(Math_floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math_ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],SAFE_HEAP_STORE(((ptr)|0), ((tempI64[0])|0), 4),SAFE_HEAP_STORE((((ptr)+(4))|0), ((tempI64[1])|0), 4)); break;
      case 'float': SAFE_HEAP_STORE_D(((ptr)|0), Math_fround(value), 4); break;
      case 'double': SAFE_HEAP_STORE_D(((ptr)|0), (+(value)), 8); break;
      default: abort('invalid type for setValue: ' + type);
    }
  }
}

/** @param {number} ptr
    @param {string} type
    @param {number|boolean=} noSafe */
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
  if (noSafe) {
    switch(type) {
      case 'i1': return HEAP8[((ptr)>>0)];
      case 'i8': return HEAP8[((ptr)>>0)];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP32[((ptr)>>2)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      default: abort('invalid type for getValue: ' + type);
    }
  } else {
    switch(type) {
      case 'i1': return ((SAFE_HEAP_LOAD(((ptr)|0), 1, 0))|0);
      case 'i8': return ((SAFE_HEAP_LOAD(((ptr)|0), 1, 0))|0);
      case 'i16': return ((SAFE_HEAP_LOAD(((ptr)|0), 2, 0))|0);
      case 'i32': return ((SAFE_HEAP_LOAD(((ptr)|0), 4, 0))|0);
      case 'i64': return ((SAFE_HEAP_LOAD(((ptr)|0), 8, 0))|0);
      case 'float': return Math_fround(SAFE_HEAP_LOAD_D(((ptr)|0), 4, 0));
      case 'double': return (+(SAFE_HEAP_LOAD_D(((ptr)|0), 8, 0)));
      default: abort('invalid type for getValue: ' + type);
    }
  }
  return null;
}


/** @param {number|boolean=} isFloat */
function getSafeHeapType(bytes, isFloat) {
  switch (bytes) {
    case 1: return 'i8';
    case 2: return 'i16';
    case 4: return isFloat ? 'float' : 'i32';
    case 8: return 'double';
    default: assert(0);
  }
}


/** @param {number|boolean=} isFloat */
function SAFE_HEAP_STORE(dest, value, bytes, isFloat) {
  if (dest <= 0) abort('segmentation fault storing ' + bytes + ' bytes to address ' + dest);
  if (dest % bytes !== 0) abort('alignment error storing to address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (dest + bytes > HEAPU32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when storing ' + bytes + ' bytes to address ' + dest + '. DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
  assert(DYNAMICTOP_PTR);
  assert(HEAP32[DYNAMICTOP_PTR>>2] <= HEAP8.length);
  setValue(dest, value, getSafeHeapType(bytes, isFloat), 1);
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  SAFE_HEAP_STORE(dest, value, bytes, true);
}

/** @param {number|boolean=} isFloat */
function SAFE_HEAP_LOAD(dest, bytes, unsigned, isFloat) {
  if (dest <= 0) abort('segmentation fault loading ' + bytes + ' bytes from address ' + dest);
  if (dest % bytes !== 0) abort('alignment error loading from address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (dest + bytes > HEAPU32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when loading ' + bytes + ' bytes from address ' + dest + '. DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
  assert(DYNAMICTOP_PTR);
  assert(HEAP32[DYNAMICTOP_PTR>>2] <= HEAP8.length);
  var type = getSafeHeapType(bytes, isFloat);
  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1), 10), 1);
  return ret;
}
function SAFE_HEAP_LOAD_D(dest, bytes, unsigned) {
  return SAFE_HEAP_LOAD(dest, bytes, unsigned, true);
}

function SAFE_FT_MASK(value, mask) {
  var ret = value & mask;
  if (ret !== value) {
    abort('Function table mask error: function pointer is ' + value + ' which is masked by ' + mask + ', the likely cause of this is that the function pointer is being called by the wrong type.');
  }
  return ret;
}

function segfault() {
  abort('segmentation fault');
}
function alignfault() {
  abort('alignment fault');
}
function ftfault() {
  abort('Function table mask error');
}



// Wasm globals

var wasmMemory;

// In fastcomp asm.js, we don't need a wasm Table at all.
// In the wasm backend, we polyfill the WebAssembly object,
// so this creates a (non-native-wasm) table for us.
var wasmTable = new WebAssembly.Table({
  'initial': 1,
  'maximum': 1 + 0,
  'element': 'anyfunc'
});


//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS = 0;

/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
  assert(func, 'Cannot call unknown function ' + ident + ', make sure it is exported');
  return func;
}

// C calling interface.
/** @param {string|null=} returnType
    @param {Array=} argTypes
    @param {Arguments|Array=} args
    @param {Object=} opts */
function ccall(ident, returnType, argTypes, args, opts) {
  // For fast lookup of conversion functions
  var toC = {
    'string': function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    },
    'array': function(arr) {
      var ret = stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    }
  };

  function convertReturnValue(ret) {
    if (returnType === 'string') return UTF8ToString(ret);
    if (returnType === 'boolean') return Boolean(ret);
    return ret;
  }

  var func = getCFunc(ident);
  var cArgs = [];
  var stack = 0;
  if (args) {
    for (var i = 0; i < args.length; i++) {
      var converter = toC[argTypes[i]];
      if (converter) {
        if (stack === 0) stack = stackSave();
        cArgs[i] = converter(args[i]);
      } else {
        cArgs[i] = args[i];
      }
    }
  }
  var ret = func.apply(null, cArgs);

  ret = convertReturnValue(ret);
  if (stack !== 0) stackRestore(stack);
  return ret;
}

/** @param {string=} returnType
    @param {Array=} argTypes
    @param {Object=} opts */
function cwrap(ident, returnType, argTypes, opts) {
  argTypes = argTypes || [];
  // When the function takes numbers and returns a number, we can just return
  // the original function
  var numericArgs = argTypes.every(function(type){ return type === 'number'});
  var numericRet = returnType !== 'string';
  if (numericRet && numericArgs && !opts) {
    return getCFunc(ident);
  }
  return function() {
    return ccall(ident, returnType, argTypes, arguments, opts);
  }
}

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_DYNAMIC = 2; // Cannot be freed except through sbrk
var ALLOC_NONE = 3; // Do not allocate

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
/** @type {function((TypedArray|Array<number>|number), string, number, number=)} */
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [_malloc,
    stackAlloc,
    dynamicAlloc][allocator](Math.max(size, singleType ? 1 : types.length));
  }

  if (zeroinit) {
    var stop;
    ptr = ret;
    assert((ret & 3) == 0);
    stop = ret + (size & ~3);
    for (; ptr < stop; ptr += 4) {
      HEAP32[((ptr)>>2)]=0;
    }
    stop = ret + size;
    while (ptr < stop) {
      HEAP8[((ptr++)>>0)]=0;
    }
    return ret;
  }

  if (singleType === 'i8') {
    if (slab.subarray || slab.slice) {
      HEAPU8.set(/** @type {!Uint8Array} */ (slab), ret);
    } else {
      HEAPU8.set(new Uint8Array(slab), ret);
    }
    return ret;
  }

  var i = 0, type, typeSize, previousType;
  while (i < size) {
    var curr = slab[i];

    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }

    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later

    setValue(ret+i, curr, type);

    // no need to look up size unless type changes, so cache it
    if (previousType !== type) {
      typeSize = getNativeTypeSize(type);
      previousType = type;
    }
    i += typeSize;
  }

  return ret;
}

// Allocate memory during any stage of startup - static memory early on, dynamic memory later, malloc when ready
function getMemory(size) {
  if (!runtimeInitialized) return dynamicAlloc(size);
  return _malloc(size);
}


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// runtime_strings.js: Strings related runtime functions that are part of both MINIMAL_RUNTIME and regular runtime.

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the given array that contains uint8 values, returns
// a copy of that string as a Javascript String object.

var UTF8Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf8') : undefined;

/**
 * @param {number} idx
 * @param {number=} maxBytesToRead
 * @return {string}
 */
function UTF8ArrayToString(heap, idx, maxBytesToRead) {
  var endIdx = idx + maxBytesToRead;
  var endPtr = idx;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  // (As a tiny code save trick, compare endPtr against endIdx using a negation, so that undefined means Infinity)
  while (heap[endPtr] && !(endPtr >= endIdx)) ++endPtr;

  if (endPtr - idx > 16 && heap.subarray && UTF8Decoder) {
    return UTF8Decoder.decode(heap.subarray(idx, endPtr));
  } else {
    var str = '';
    // If building with TextDecoder, we have already computed the string length above, so test loop end condition against that
    while (idx < endPtr) {
      // For UTF8 byte structure, see:
      // http://en.wikipedia.org/wiki/UTF-8#Description
      // https://www.ietf.org/rfc/rfc2279.txt
      // https://tools.ietf.org/html/rfc3629
      var u0 = heap[idx++];
      if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
      var u1 = heap[idx++] & 63;
      if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
      var u2 = heap[idx++] & 63;
      if ((u0 & 0xF0) == 0xE0) {
        u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
      } else {
        u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heap[idx++] & 63);
      }

      if (u0 < 0x10000) {
        str += String.fromCharCode(u0);
      } else {
        var ch = u0 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      }
    }
  }
  return str;
}

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the emscripten HEAP, returns a
// copy of that string as a Javascript String object.
// maxBytesToRead: an optional length that specifies the maximum number of bytes to read. You can omit
//                 this parameter to scan the string until the first \0 byte. If maxBytesToRead is
//                 passed, and the string at [ptr, ptr+maxBytesToReadr[ contains a null byte in the
//                 middle, then the string will cut short at that byte index (i.e. maxBytesToRead will
//                 not produce a string of exact length [ptr, ptr+maxBytesToRead[)
//                 N.B. mixing frequent uses of UTF8ToString() with and without maxBytesToRead may
//                 throw JS JIT optimizations off, so it is worth to consider consistently using one
//                 style or the other.
/**
 * @param {number} ptr
 * @param {number=} maxBytesToRead
 * @return {string}
 */
function UTF8ToString(ptr, maxBytesToRead) {
  return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
}

// Copies the given Javascript String object 'str' to the given byte array at address 'outIdx',
// encoded in UTF8 form and null-terminated. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   heap: the array to copy to. Each index in this array is assumed to be one 8-byte element.
//   outIdx: The starting offset in the array to begin the copying.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array.
//                    This count should include the null terminator,
//                    i.e. if maxBytesToWrite=1, only the null terminator will be written and nothing else.
//                    maxBytesToWrite=0 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
  if (!(maxBytesToWrite > 0)) // Parameter maxBytesToWrite is not optional. Negative values, 0, null, undefined and false each don't write out any bytes.
    return 0;

  var startIdx = outIdx;
  var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) {
      var u1 = str.charCodeAt(++i);
      u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
    }
    if (u <= 0x7F) {
      if (outIdx >= endIdx) break;
      heap[outIdx++] = u;
    } else if (u <= 0x7FF) {
      if (outIdx + 1 >= endIdx) break;
      heap[outIdx++] = 0xC0 | (u >> 6);
      heap[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0xFFFF) {
      if (outIdx + 2 >= endIdx) break;
      heap[outIdx++] = 0xE0 | (u >> 12);
      heap[outIdx++] = 0x80 | ((u >> 6) & 63);
      heap[outIdx++] = 0x80 | (u & 63);
    } else {
      if (outIdx + 3 >= endIdx) break;
      heap[outIdx++] = 0xF0 | (u >> 18);
      heap[outIdx++] = 0x80 | ((u >> 12) & 63);
      heap[outIdx++] = 0x80 | ((u >> 6) & 63);
      heap[outIdx++] = 0x80 | (u & 63);
    }
  }
  // Null-terminate the pointer to the buffer.
  heap[outIdx] = 0;
  return outIdx - startIdx;
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF8 form. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8(str, outPtr, maxBytesToWrite) {
  return stringToUTF8Array(str, HEAPU8,outPtr, maxBytesToWrite);
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF8 byte array, EXCLUDING the null terminator byte.
function lengthBytesUTF8(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) ++len;
    else if (u <= 0x7FF) len += 2;
    else if (u <= 0xFFFF) len += 3;
    else len += 4;
  }
  return len;
}



/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// runtime_strings_extra.js: Strings related runtime functions that are available only in regular runtime.

// Given a pointer 'ptr' to a null-terminated ASCII-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function AsciiToString(ptr) {
  var str = '';
  while (1) {
    var ch = ((SAFE_HEAP_LOAD(((ptr++)|0), 1, 1))|0);
    if (!ch) return str;
    str += String.fromCharCode(ch);
  }
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in ASCII form. The copy will require at most str.length+1 bytes of space in the HEAP.

function stringToAscii(str, outPtr) {
  return writeAsciiToMemory(str, outPtr, false);
}

// Given a pointer 'ptr' to a null-terminated UTF16LE-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

var UTF16Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf-16le') : undefined;

function UTF16ToString(ptr) {
  var endPtr = ptr;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  var idx = endPtr >> 1;
  while (HEAP16[idx]) ++idx;
  endPtr = idx << 1;

  if (endPtr - ptr > 32 && UTF16Decoder) {
    return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
  } else {
    var i = 0;

    var str = '';
    while (1) {
      var codeUnit = ((SAFE_HEAP_LOAD((((ptr)+(i*2))|0), 2, 0))|0);
      if (codeUnit == 0) return str;
      ++i;
      // fromCharCode constructs a character from a UTF-16 code unit, so we can pass the UTF16 string right through.
      str += String.fromCharCode(codeUnit);
    }
  }
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF16 form. The copy will require at most str.length*4+2 bytes of space in the HEAP.
// Use the function lengthBytesUTF16() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=2, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<2 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF16(str, outPtr, maxBytesToWrite) {
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 2) return 0;
  maxBytesToWrite -= 2; // Null terminator.
  var startPtr = outPtr;
  var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
  for (var i = 0; i < numCharsToWrite; ++i) {
    // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    SAFE_HEAP_STORE(((outPtr)|0), ((codeUnit)|0), 2);
    outPtr += 2;
  }
  // Null-terminate the pointer to the HEAP.
  SAFE_HEAP_STORE(((outPtr)|0), ((0)|0), 2);
  return outPtr - startPtr;
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF16(str) {
  return str.length*2;
}

function UTF32ToString(ptr) {
  var i = 0;

  var str = '';
  while (1) {
    var utf32 = ((SAFE_HEAP_LOAD((((ptr)+(i*4))|0), 4, 0))|0);
    if (utf32 == 0) return str;
    ++i;
    // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    if (utf32 >= 0x10000) {
      var ch = utf32 - 0x10000;
      str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
    } else {
      str += String.fromCharCode(utf32);
    }
  }
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF32 form. The copy will require at most str.length*4+4 bytes of space in the HEAP.
// Use the function lengthBytesUTF32() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=4, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<4 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF32(str, outPtr, maxBytesToWrite) {
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 4) return 0;
  var startPtr = outPtr;
  var endPtr = startPtr + maxBytesToWrite - 4;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
      var trailSurrogate = str.charCodeAt(++i);
      codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
    }
    SAFE_HEAP_STORE(((outPtr)|0), ((codeUnit)|0), 4);
    outPtr += 4;
    if (outPtr + 4 > endPtr) break;
  }
  // Null-terminate the pointer to the HEAP.
  SAFE_HEAP_STORE(((outPtr)|0), ((0)|0), 4);
  return outPtr - startPtr;
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF32(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i);
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
    len += 4;
  }

  return len;
}

// Allocate heap space for a JS string, and write it there.
// It is the responsibility of the caller to free() that memory.
function allocateUTF8(str) {
  var size = lengthBytesUTF8(str) + 1;
  var ret = _malloc(size);
  if (ret) stringToUTF8Array(str, HEAP8, ret, size);
  return ret;
}

// Allocate stack space for a JS string, and write it there.
function allocateUTF8OnStack(str) {
  var size = lengthBytesUTF8(str) + 1;
  var ret = stackAlloc(size);
  stringToUTF8Array(str, HEAP8, ret, size);
  return ret;
}

// Deprecated: This function should not be called because it is unsafe and does not provide
// a maximum length limit of how many bytes it is allowed to write. Prefer calling the
// function stringToUTF8Array() instead, which takes in a maximum length that can be used
// to be secure from out of bounds writes.
/** @deprecated
    @param {boolean=} dontAddNull */
function writeStringToMemory(string, buffer, dontAddNull) {
  warnOnce('writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!');

  var /** @type {number} */ lastChar, /** @type {number} */ end;
  if (dontAddNull) {
    // stringToUTF8Array always appends null. If we don't want to do that, remember the
    // character that existed at the location where the null will be placed, and restore
    // that after the write (below).
    end = buffer + lengthBytesUTF8(string);
    lastChar = HEAP8[end];
  }
  stringToUTF8(string, buffer, Infinity);
  if (dontAddNull) HEAP8[end] = lastChar; // Restore the value under the null character.
}

function writeArrayToMemory(array, buffer) {
  HEAP8.set(array, buffer);
}

/** @param {boolean=} dontAddNull */
function writeAsciiToMemory(str, buffer, dontAddNull) {
  for (var i = 0; i < str.length; ++i) {
    SAFE_HEAP_STORE(((buffer++)|0), ((str.charCodeAt(i))|0), 1);
  }
  // Null-terminate the pointer to the HEAP.
  if (!dontAddNull) SAFE_HEAP_STORE(((buffer)|0), ((0)|0), 1);
}



// Memory management

var PAGE_SIZE = 16384;
var WASM_PAGE_SIZE = 65536;
var ASMJS_PAGE_SIZE = 16777216;

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

var HEAP,
/** @type {ArrayBuffer} */
  buffer,
/** @type {Int8Array} */
  HEAP8,
/** @type {Uint8Array} */
  HEAPU8,
/** @type {Int16Array} */
  HEAP16,
/** @type {Uint16Array} */
  HEAPU16,
/** @type {Int32Array} */
  HEAP32,
/** @type {Uint32Array} */
  HEAPU32,
/** @type {Float32Array} */
  HEAPF32,
/** @type {Float64Array} */
  HEAPF64;

function updateGlobalBufferAndViews(buf) {
  buffer = buf;
  Module['HEAP8'] = HEAP8 = new Int8Array(buf);
  Module['HEAP16'] = HEAP16 = new Int16Array(buf);
  Module['HEAP32'] = HEAP32 = new Int32Array(buf);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buf);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buf);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buf);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buf);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buf);
}

var STATIC_BASE = 1024,
    STACK_BASE = 5307104,
    STACKTOP = STACK_BASE,
    STACK_MAX = 64224,
    DYNAMIC_BASE = 5307104,
    DYNAMICTOP_PTR = 64064;




var TOTAL_STACK = 5242880;

var INITIAL_INITIAL_MEMORY = Module['INITIAL_MEMORY'] || 16777216;




/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */




// In standalone mode, the wasm creates the memory, and the user can't provide it.
// In non-standalone/normal mode, we create the memory here.

/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Create the main memory. (Note: this isn't used in STANDALONE_WASM mode since the wasm
// memory is created in the wasm, not in JS.)

  if (Module['wasmMemory']) {
    wasmMemory = Module['wasmMemory'];
  } else
  {
    wasmMemory = new WebAssembly.Memory({
      'initial': INITIAL_INITIAL_MEMORY / WASM_PAGE_SIZE
      ,
      'maximum': INITIAL_INITIAL_MEMORY / WASM_PAGE_SIZE
    });
  }


if (wasmMemory) {
  buffer = wasmMemory.buffer;
}

// If the user provides an incorrect length, just use that length instead rather than providing the user to
// specifically provide the memory length with Module['INITIAL_MEMORY'].
INITIAL_INITIAL_MEMORY = buffer.byteLength;
updateGlobalBufferAndViews(buffer);

HEAP32[DYNAMICTOP_PTR>>2] = DYNAMIC_BASE;




/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */




/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */




function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    if (typeof callback == 'function') {
      callback(Module); // Pass the module as the first argument.
      continue;
    }
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Module['dynCall_v'](func);
      } else {
        Module['dynCall_vi'](func, callback.arg);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the main() is called

var runtimeInitialized = false;
var runtimeExited = false;


function preRun() {

  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }

  callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
  runtimeInitialized = true;
  
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
  
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
  runtimeExited = true;
}

function postRun() {

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }

  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}

function addOnExit(cb) {
}

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}

/** @param {number|boolean=} ignore */
function unSign(value, bits, ignore) {
  if (value >= 0) {
    return value;
  }
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
}
/** @param {number|boolean=} ignore */
function reSign(value, bits, ignore) {
  if (value <= 0) {
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
  if (value >= half && (bits <= 32 || value > half)) { // for huge values, we can hit the precision limit and always get true here. so don't do that
                                                       // but, in general there is no perfect solution here. With 64-bit ints, we get rounding and errors
                                                       // TODO: In i64 mode 1, resign the two parts separately and safely
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
  return value;
}


/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/imul

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/fround

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/clz32

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/trunc


var Math_abs = Math.abs;
var Math_cos = Math.cos;
var Math_sin = Math.sin;
var Math_tan = Math.tan;
var Math_acos = Math.acos;
var Math_asin = Math.asin;
var Math_atan = Math.atan;
var Math_atan2 = Math.atan2;
var Math_exp = Math.exp;
var Math_log = Math.log;
var Math_sqrt = Math.sqrt;
var Math_ceil = Math.ceil;
var Math_floor = Math.floor;
var Math_pow = Math.pow;
var Math_imul = Math.imul;
var Math_fround = Math.fround;
var Math_round = Math.round;
var Math_min = Math.min;
var Math_max = Math.max;
var Math_clz32 = Math.clz32;
var Math_trunc = Math.trunc;



// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled

function getUniqueRunDependency(id) {
  return id;
}

function addRunDependency(id) {
  runDependencies++;

  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }

}

function removeRunDependency(id) {
  runDependencies--;

  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }

  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data


/** @param {string|number=} what */
function abort(what) {
  if (Module['onAbort']) {
    Module['onAbort'](what);
  }

  what += '';
  out(what);
  err(what);

  ABORT = true;
  EXITSTATUS = 1;

  what = 'abort(' + what + '). Build with -s ASSERTIONS=1 for more info.';

  // Throw a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  throw new WebAssembly.RuntimeError(what);
}


var memoryInitializer = null;


/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */







/**
 * @license
 * Copyright 2017 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

function hasPrefix(str, prefix) {
  return String.prototype.startsWith ?
      str.startsWith(prefix) :
      str.indexOf(prefix) === 0;
}

// Prefix of data URIs emitted by SINGLE_FILE and related options.
var dataURIPrefix = 'data:application/octet-stream;base64,';

// Indicates whether filename is a base64 data URI.
function isDataURI(filename) {
  return hasPrefix(filename, dataURIPrefix);
}

var fileURIPrefix = "file://";

// Indicates whether filename is delivered via file protocol (as opposed to http/https)
function isFileURI(filename) {
  return hasPrefix(filename, fileURIPrefix);
}



var wasmBinaryFile = 'huffman.wasm';
if (!isDataURI(wasmBinaryFile)) {
  wasmBinaryFile = locateFile(wasmBinaryFile);
}

function getBinary() {
  try {
    if (wasmBinary) {
      return new Uint8Array(wasmBinary);
    }

    var binary = tryParseAsDataURI(wasmBinaryFile);
    if (binary) {
      return binary;
    }
    if (readBinary) {
      return readBinary(wasmBinaryFile);
    } else {
      throw "both async and sync fetching of the wasm failed";
    }
  }
  catch (err) {
    abort(err);
  }
}

function getBinaryPromise() {
  // If we don't have the binary yet, and have the Fetch api, use that;
  // in some environments, like Electron's render process, Fetch api may be present, but have a different context than expected, let's only use it on the Web
  if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && typeof fetch === 'function'
      // Let's not use fetch to get objects over file:// as it's most likely Cordova which doesn't support fetch for file://
      && !isFileURI(wasmBinaryFile)
      ) {
    return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function(response) {
      if (!response['ok']) {
        throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
      }
      return response['arrayBuffer']();
    }).catch(function () {
      return getBinary();
    });
  }
  // Otherwise, getBinary should be able to get it synchronously
  return new Promise(function(resolve, reject) {
    resolve(getBinary());
  });
}



// Create the wasm instance.
// Receives the wasm imports, returns the exports.
function createWasm() {
  // prepare imports
  var info = {
    'env': asmLibraryArg,
    'wasi_snapshot_preview1': asmLibraryArg
  };
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    var exports = instance.exports;
    Module['asm'] = exports;
    removeRunDependency('wasm-instantiate');
  }
  // we can't run yet (except in a pthread, where we have a custom sync instantiator)
  addRunDependency('wasm-instantiate');


  function receiveInstantiatedSource(output) {
    // 'output' is a WebAssemblyInstantiatedSource object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above USE_PTHREADS-enabled path.
    receiveInstance(output['instance']);
  }


  function instantiateArrayBuffer(receiver) {
    return getBinaryPromise().then(function(binary) {
      return WebAssembly.instantiate(binary, info);
    }).then(receiver, function(reason) {
      err('failed to asynchronously prepare wasm: ' + reason);
      abort(reason);
    });
  }

  // Prefer streaming instantiation if available.
  function instantiateAsync() {
    if (!wasmBinary &&
        typeof WebAssembly.instantiateStreaming === 'function' &&
        !isDataURI(wasmBinaryFile) &&
        // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
        !isFileURI(wasmBinaryFile) &&
        typeof fetch === 'function') {
      fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function (response) {
        var result = WebAssembly.instantiateStreaming(response, info);
        return result.then(receiveInstantiatedSource, function(reason) {
            // We expect the most common failure cause to be a bad MIME type for the binary,
            // in which case falling back to ArrayBuffer instantiation should work.
            err('wasm streaming compile failed: ' + reason);
            err('falling back to ArrayBuffer instantiation');
            instantiateArrayBuffer(receiveInstantiatedSource);
          });
      });
    } else {
      return instantiateArrayBuffer(receiveInstantiatedSource);
    }
  }
  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
  // to any other async startup actions they are performing.
  if (Module['instantiateWasm']) {
    try {
      var exports = Module['instantiateWasm'](info, receiveInstance);
      return exports;
    } catch(e) {
      err('Module.instantiateWasm callback failed with error: ' + e);
      return false;
    }
  }

  instantiateAsync();
  return {}; // no exports yet; we'll fill them in later
}


// Globals used by JS i64 conversions
var tempDouble;
var tempI64;

// === Body ===

var ASM_CONSTS = {
  
};




// STATICTOP = STATIC_BASE + 63200;
/* global initializers */  __ATINIT__.push({ func: function() { ___wasm_call_ctors() } });




/* no memory initializer */
// {{PRE_LIBRARY}}


  function demangle(func) {
      return func;
    }

  function demangleAll(text) {
      var regex =
        /\b_Z[\w\d_]+/g;
      return text.replace(regex,
        function(x) {
          var y = demangle(x);
          return x === y ? x : (y + ' [' + x + ']');
        });
    }

  function jsStackTrace() {
      var err = new Error();
      if (!err.stack) {
        // IE10+ special cases: It does have callstack info, but it is only populated if an Error object is thrown,
        // so try that as a special-case.
        try {
          throw new Error();
        } catch(e) {
          err = e;
        }
        if (!err.stack) {
          return '(no stack trace available)';
        }
      }
      return err.stack.toString();
    }

  function stackTrace() {
      var js = jsStackTrace();
      if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
      return demangleAll(js);
    }

  function _emscripten_get_sbrk_ptr() {
      return 64064;
    }

  function _emscripten_memcpy_big(dest, src, num) {
      HEAPU8.copyWithin(dest, src, src + num);
    }

  
  function _emscripten_get_heap_size() {
      return HEAPU8.length;
    }
  
  function abortOnCannotGrowMemory(requestedSize) {
      abort('OOM');
    }function _emscripten_resize_heap(requestedSize) {
      abortOnCannotGrowMemory(requestedSize);
    }
var ASSERTIONS = false;

/**
 * @license
 * Copyright 2017 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/** @type {function(string, boolean=, number=)} */
function intArrayFromString(stringy, dontAddNull, length) {
  var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
}

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
      if (ASSERTIONS) {
        assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
      }
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}


// Copied from https://github.com/strophe/strophejs/blob/e06d027/src/polyfills.js#L149

// This code was written by Tyler Akins and has been placed in the
// public domain.  It would be nice if you left this header intact.
// Base64 code from Tyler Akins -- http://rumkin.com

/**
 * Decodes a base64 string.
 * @param {string} input The string to decode.
 */
var decodeBase64 = typeof atob === 'function' ? atob : function (input) {
  var keyStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';

  var output = '';
  var chr1, chr2, chr3;
  var enc1, enc2, enc3, enc4;
  var i = 0;
  // remove all characters that are not A-Z, a-z, 0-9, +, /, or =
  input = input.replace(/[^A-Za-z0-9\+\/\=]/g, '');
  do {
    enc1 = keyStr.indexOf(input.charAt(i++));
    enc2 = keyStr.indexOf(input.charAt(i++));
    enc3 = keyStr.indexOf(input.charAt(i++));
    enc4 = keyStr.indexOf(input.charAt(i++));

    chr1 = (enc1 << 2) | (enc2 >> 4);
    chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
    chr3 = ((enc3 & 3) << 6) | enc4;

    output = output + String.fromCharCode(chr1);

    if (enc3 !== 64) {
      output = output + String.fromCharCode(chr2);
    }
    if (enc4 !== 64) {
      output = output + String.fromCharCode(chr3);
    }
  } while (i < input.length);
  return output;
};

// Converts a string of base64 into a byte array.
// Throws error on invalid input.
function intArrayFromBase64(s) {
  if (typeof ENVIRONMENT_IS_NODE === 'boolean' && ENVIRONMENT_IS_NODE) {
    var buf;
    try {
      // TODO: Update Node.js externs, Closure does not recognize the following Buffer.from()
      /**@suppress{checkTypes}*/
      buf = Buffer.from(s, 'base64');
    } catch (_) {
      buf = new Buffer(s, 'base64');
    }
    return new Uint8Array(buf['buffer'], buf['byteOffset'], buf['byteLength']);
  }

  try {
    var decoded = decodeBase64(s);
    var bytes = new Uint8Array(decoded.length);
    for (var i = 0 ; i < decoded.length ; ++i) {
      bytes[i] = decoded.charCodeAt(i);
    }
    return bytes;
  } catch (_) {
    throw new Error('Converting base64 string to bytes failed.');
  }
}

// If filename is a base64 data URI, parses and returns data (Buffer on node,
// Uint8Array otherwise). If filename is not a base64 data URI, returns undefined.
function tryParseAsDataURI(filename) {
  if (!isDataURI(filename)) {
    return;
  }

  return intArrayFromBase64(filename.slice(dataURIPrefix.length));
}


var asmGlobalArg = {};
var asmLibraryArg = { "alignfault": alignfault, "emscripten_get_sbrk_ptr": _emscripten_get_sbrk_ptr, "emscripten_memcpy_big": _emscripten_memcpy_big, "emscripten_resize_heap": _emscripten_resize_heap, "getTempRet0": getTempRet0, "memory": wasmMemory, "segfault": segfault, "setTempRet0": setTempRet0, "table": wasmTable };
var asm = createWasm();
Module["asm"] = asm;
/** @type {function(...*):?} */
var ___wasm_call_ctors = Module["___wasm_call_ctors"] = function() {
  return (___wasm_call_ctors = Module["___wasm_call_ctors"] = Module["asm"]["__wasm_call_ctors"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _Huff_Decompress = Module["_Huff_Decompress"] = function() {
  return (_Huff_Decompress = Module["_Huff_Decompress"] = Module["asm"]["Huff_Decompress"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _MSG_initHuffman = Module["_MSG_initHuffman"] = function() {
  return (_MSG_initHuffman = Module["_MSG_initHuffman"] = Module["asm"]["MSG_initHuffman"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _Huff_Compress = Module["_Huff_Compress"] = function() {
  return (_Huff_Compress = Module["_Huff_Compress"] = Module["asm"]["Huff_Compress"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _HuffmanGetSymbol = Module["_HuffmanGetSymbol"] = function() {
  return (_HuffmanGetSymbol = Module["_HuffmanGetSymbol"] = Module["asm"]["HuffmanGetSymbol"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var ___errno_location = Module["___errno_location"] = function() {
  return (___errno_location = Module["___errno_location"] = Module["asm"]["__errno_location"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _malloc = Module["_malloc"] = function() {
  return (_malloc = Module["_malloc"] = Module["asm"]["malloc"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _free = Module["_free"] = function() {
  return (_free = Module["_free"] = Module["asm"]["free"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var stackSave = Module["stackSave"] = function() {
  return (stackSave = Module["stackSave"] = Module["asm"]["stackSave"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var stackAlloc = Module["stackAlloc"] = function() {
  return (stackAlloc = Module["stackAlloc"] = Module["asm"]["stackAlloc"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var stackRestore = Module["stackRestore"] = function() {
  return (stackRestore = Module["stackRestore"] = Module["asm"]["stackRestore"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var __growWasmMemory = Module["__growWasmMemory"] = function() {
  return (__growWasmMemory = Module["__growWasmMemory"] = Module["asm"]["__growWasmMemory"]).apply(null, arguments);
};



/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Auto-generated postamble setup entry stuff ===

Module['asm'] = asm;






Module["getValue"] = getValue;
Module["allocate"] = allocate;
































































































































var calledRun;


/**
 * @constructor
 * @this {ExitStatus}
 */
function ExitStatus(status) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + status + ")";
  this.status = status;
}

var calledMain = false;


dependenciesFulfilled = function runCaller() {
  // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
  if (!calledRun) run();
  if (!calledRun) dependenciesFulfilled = runCaller; // try this again later, after new deps are fulfilled
};





/** @type {function(Array=)} */
function run(args) {
  args = args || arguments_;

  if (runDependencies > 0) {
    return;
  }


  preRun();

  if (runDependencies > 0) return; // a preRun added a dependency, run will be called later

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    if (calledRun) return;
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

    if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();


    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
}
Module['run'] = run;


/** @param {boolean|number=} implicit */
function exit(status, implicit) {

  // if this is just main exit-ing implicitly, and the status is 0, then we
  // don't need to do anything here and can just leave. if the status is
  // non-zero, though, then we need to report it.
  // (we may have warned about this earlier, if a situation justifies doing so)
  if (implicit && noExitRuntime && status === 0) {
    return;
  }

  if (noExitRuntime) {
  } else {

    ABORT = true;
    EXITSTATUS = status;

    exitRuntime();

    if (Module['onExit']) Module['onExit'](status);
  }

  quit_(status, new ExitStatus(status));
}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}


  noExitRuntime = true;

run();





// {{MODULE_ADDITIONS}}
