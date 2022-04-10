if(typeof global != 'undefined' && typeof global.window == 'undefined') {
	global.window = {}
}

var Q3e = {}
window.Q3e = Q3e

function getQueryCommands() {
	// Wow, look at all the unfuckery I don't have to do with startup options because
	//   I'm not using emscripten anymore.
	let startup = [
		'quake3e_web',
		'+set', 'fs_basepath', '/base',
		'+set', 'fs_homepath', '/home',
		'+set', 'sv_pure', '0', // require for now, TODO: server side zips
		'+set', 'fs_basegame', 'multigame',
		'+set', 'r_mode', '-1',
		'+set', 'r_ext_framebuffer_object', '0',
		'+set', 'bot_enable', '0',
		'+set', 'net_socksServer', window.location.hostname || '',
		'+set', 'net_socksPort', window.location.port 
			|| (window.location.protocol == 'https:' ? '443' : '80'),
		'+set', 'sv_fps', '100',
		'+set', 'snaps', '100',
		//'+set', 'r_ext_multitexture', '0',
		//'+set', 'r_ext_framebuffer_multisample', '0',
		// this prevents lightmap from being wrong when switching maps
		//   renderer doesn't restart between maps, but BSP loading updates
		//   textures with lightmap by default, so this keeps them separate
		//'+set', 'r_mergeLightmaps', '0',
		//'+set', 'r_deluxeMapping', '0',
		//'+set', 'r_normalMapping', '0',
		//'+set', 'r_specularMapping', '0',
	]
	var search = /([^&=]+)/g
	var query  = window.location.search.substring(1)
	var match
	while (match = search.exec(query)) {
		var val = decodeURIComponent(match[1])
		val = val.split(' ')
		val[0] = (val[0][0] != '+' ? '+' : '') + val[0]
		startup.push.apply(startup, val)
	}
	startup.push.apply(startup, window.preStart)
	startup.unshift.apply(startup, [
		'+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
		'+set', 'r_customHeight', '' + window.innerHeight || 0,
		'+set', 'r_customWidth', '' + window.innerWidth || 0,
		'+exec', 'autoexec-' + window.location.hostname.replace('.quake.games', '') + '.cfg',
	])
	return startup
}

function startProgram(program) {
	// share the game with window for hackers
	if(!program) {
		throw new Error("no program!")
	}
	Q3e['program'] = program || {}
	Q3e['instance'] = Q3e['program'].instance || {}
	Q3e['exports'] = Q3e['instance'].exports || {}
	let newMethods = Object.keys(Q3e['exports'])
	for(let i = 0; i < newMethods.length; i++) {
		window[newMethods[i]] = Q3e['exports'][newMethods[i]] //.apply(Q3e['exports'])
	}
	if(typeof window['Z_Malloc'] == 'undefined') {
		window.Z_Malloc = window['Z_MallocDebug']
	}
	Object.assign(window, Q3e['exports'])

	// start a brand new call frame, in-case error bubbles up
	setTimeout(function () {
		try {
			// reserve some memory at the beginning for passing shit back and forth with JS
			//   not to use a complex HEAP, just loop around on bytes[b % 128] and if 
			//   something isn't cleared out, crash
			Q3e['sharedMemory'] = malloc(1024 * 1024) // store some strings and crap
			Q3e['sharedCounter'] = 0
			Q3e['exited'] = false

			// Startup args is expecting a char **
			let startup = getQueryCommands()
			RunGame(startup.length, stringsToMemory(startup))
			HEAPU32[fs_loading >> 2] = Q3e.fs_loading
			// should have Cvar system by now
			INPUT.fpsUnfocused = Cvar_VariableIntegerValue(stringToAddress('com_maxfpsUnfocused'));
			INPUT.fps = Cvar_VariableIntegerValue(stringToAddress('com_maxfps'))
			// this might help prevent this thing that krunker.io does where it lags when it first starts up
			Q3e.frameInterval = setInterval(Sys_Frame, 
				1000 / (HEAP32[gw_active >> 2] ? INPUT.fps : INPUT.fpsUnfocused));
		} catch (e) {
			console.log(e)
			Sys_Exit(1)
			throw e
		}
	}, 13)
	return true
}


/*
function get_string(memory, addr) {
	let buffer = new Uint8Array(memory.buffer, addr, memory.buffer.byteLength - addr);
	let term = buffer.indexOf(0);

	return new TextDecoder().decode(buffer.subarray(0, term));
}
*/


function addressToString(addr, length) {
	let newString = ''
	if(!addr) return newString
	if(!length) length = 1024
	for(let i = 0; i < length; i++) {
		if(HEAPU8[addr + i] == 0) {
			break;
		}
		newString += String.fromCharCode(HEAPU8[addr + i])
	}

	return newString
}

function stringToAddress(str, addr) {
	let start = Q3e.sharedMemory + Q3e.sharedCounter
	if(typeof str != 'string') {
		str = str + ''
	}
	if(addr) start = addr
	for(let j = 0; j < str.length; j++) {
		HEAPU8[start+j] = str.charCodeAt(j)
	}
	HEAPU8[start+str.length] = 0
	HEAPU8[start+str.length+1] = 0
	HEAPU8[start+str.length+2] = 0
	if(!addr) {
		Q3e.sharedCounter += str.length + 3
		Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
		if(Q3e.sharedCounter > 1024 * 512) {
			Q3e.sharedCounter = 0
		}
	}
	return start
}


// here's the thing, I know for a fact that all the callers copy this stuff
//   so I don't need to increase my temporary storage because by the time it's
//   overwritten the data won't be needed, should only keep shared storage around
//   for events and stuff that might take more than 1 frame
function stringsToMemory(list, length) {
	// add list length so we can return addresses like char **
	let start = Q3e.sharedMemory + Q3e.sharedCounter
	let posInSeries = start + list.length * 4
	for (let i = 0; i < list.length; i++) {
		HEAPU32[(start+i*4)>>2] = posInSeries // save the starting address in the list
		stringToAddress(list[i], posInSeries)
		posInSeries += list[i].length + 1
	}
	if(length) HEAPU32[length >> 2] = posInSeries - start
	Q3e.sharedCounter = posInSeries - Q3e.sharedMemory
	Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
	if(Q3e.sharedCounter > 1024 * 512) {
		Q3e.sharedCounter = 0
	}
	return start
}

function Com_RealTime(outAddress) {
	let now = new Date()
	let t = t.now() / 1000
	HEAP32[(tm >> 2) + 5] = now.getFullYear() - 1900
	HEAP32[(tm >> 2) + 4] = now.getMonth() // already subtracted by 1
	HEAP32[(tm >> 2) + 3] = now.getDate() 
	HEAP32[(tm >> 2) + 2] = (t / 60 / 60) % 24
	HEAP32[(tm >> 2) + 1] = (t / 60) % 60
	HEAP32[(tm >> 2) + 0] = t % 60
	return t
}

function Sys_UnloadLibrary() {

}

function Sys_LoadLibrary() {
	
}

function Sys_LoadFunction() {
	
}

function Sys_Microseconds() {
	if (window.performance.now) {
		return parseInt(window.performance.now(), 10);
	} else if (window.performance.webkitNow) {
		return parseInt(window.performance.webkitNow(), 10);
	}

	Q3e.sharedCounter += 8
	return Q3e.sharedMemory + Q3e.sharedCounter - 8
}

function Sys_Milliseconds() {
	if (!Q3e['timeBase']) {
		// javascript times are bigger, so start at zero
		//   pretend like we've been alive for at least a few seconds
		//   I actually had to do this because files it checking times and this caused a delay
		Q3e['timeBase'] = Date.now() - 5000;
	}

	//if (window.performance.now) {
	//  return parseInt(window.performance.now(), 10);
	//} else if (window.performance.webkitNow) {
	//  return parseInt(window.performance.webkitNow(), 10);
	//} else {
	return Date.now() - Q3e.timeBase;
	//}
}

function Sys_RandomBytes (string, len) {
	if(typeof crypto != 'undefined') {
		crypto.getRandomValues(HEAP8.subarray(string, string+(len / 4)))
	} else {
		for(let i = 0; i < (len / 4); i++) {
			HEAP8[string] = Math.random() * 255
		}
	}
	return true;
}

function Sys_Print(message) {
	console.log(addressToString(message))
}

function Sys_Exit(code) {
	Q3e.exited = true
	GLimp_Shutdown();
	NET_Shutdown();
	if(Q3e.frameInterval) {
		clearInterval(Q3e.frameInterval)
		Q3e.frameInterval = null
	}
	// redirect to lvlworld
	let returnUrl = addressToString(Cvar_VariableString('cl_returnURL'))
	if(returnUrl) {
		window.location = returnUrl
	}
}

function Sys_Error(fmt, args) {
	let len = BG_sprintf(Q3e.sharedMemory + Q3e.sharedCounter, fmt, args)
	if(len > 0)
		console.error('Sys_Error: ', addressToString(Q3e.sharedMemory + Q3e.sharedCounter))
	Sys_Exit( 1 )
	throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status, replacementStr) {
	// TODO: something like  window.title = , then setTimeout( window.title = 'Q3e' again)
	/*
	let desc = addressToString(status)
	if(desc.includes('Main menu')) {
		if(!Q3e.initialized) {
			Q3e.initialized = true
			document.body.className += ' done-loading '
		}
	}
	let description = document.querySelector('#loading-progress .description')
	let div = document.createElement('div')
	// TODO: use BG_sprintf like above?
	if(replacementStr) {
		div.innerHTML = desc.replace('%s', addressToString(replacementStr))
	}
	if(description.children.length == 0
		|| div.innerText.toLowerCase() != description
			.children[description.children.length-1].innerText.toLowerCase())
		description.appendChild(div)
	*/
}

function CL_MenuModified(oldValue, newValue, cvar) {
	if(INPUT.modifyingCrumb) {
		return // called from ourselves below from a user action
	}
	if(window.location.orgin == null) {
		return
	}
	let newValueStr = addressToString(newValue)
	let newLocation = newValueStr.replace(/[^a-z0-9]/gi, '')
	if(!Q3e.initialized) {
		Q3e.initialized = true
		document.body.className += ' done-loading '
	}
	if(window.location.pathname.toString().includes(newLocation)) {
		// don't add to stack because it creates a lot of annoying back pushes
		return
	}
	history.pushState(
		{location: window.location.pathname}, 
		'Quake III Arena: ' + newValueStr, 
		newLocation)
}

function CL_ModifyMenu(event) {
	let oldLocation = window.location.pathname.toString().substring(1) || 'MAIN MENU'
	Cbuf_AddText( stringToAddress(`set ui_breadCrumb "${oldLocation}"\n`) );
}

function Sys_Frame() {
	if(Q3e.inFrame) {
		return
	}
	function doFrame() {
		Q3e.inFrame = true
		Q3e.running = !Q3e.running
		try {
			Com_Frame(Q3e.running)
		} catch (e) {
			if(!Q3e.exited && e.message == 'longjmp') {
				// let game Com_Frame handle it, it will restart UIVM
				Cbuf_AddText(stringToAddress('vid_restart;'));
				console.error(e)
			} else
			if(!Q3e.exited || e.message != 'unreachable') {
				Sys_Exit(1)
				throw e
			}
		}
		Q3e.inFrame = false
	}
	if(HEAP32[gw_active >> 2]) {
		requestAnimationFrame(doFrame)
	} else {
		doFrame()
	}
}

function alignUp(x, multiple) {
	if (x % multiple > 0) {
	x += multiple - x % multiple;
	}
	return x;
}

function updateGlobalBufferAndViews(buf) {
	Q3e["HEAP8"] = window.HEAP8 = new Int8Array(buf);
	Q3e["HEAPU8"] = window.HEAPU8 = new Uint8Array(buf);
	Q3e["HEAP16"] = window.HEAP16 = new Int16Array(buf);
	Q3e["HEAPU16"] = window.HEAPU16 = new Uint16Array(buf);
	Q3e["HEAP32"] = window.HEAP32 = new Int32Array(buf);
	Q3e["HEAPU32"] = window.HEAPU32 = new Uint32Array(buf);
	Q3e["HEAPF32"] = window.HEAPF32 = new Float32Array(buf);
	Q3e["HEAPF64"] = window.HEAPF64 = new Float64Array(buf);
}

var _emscripten_get_now_is_monotonic = true;

function _emscripten_get_now() {
	return performance.now()
}

function clock_gettime(clk_id, tp) {
	var now;
	if (clk_id === 0) {
			now = Date.now()
	} else if ((clk_id === 1 || clk_id === 4) && _emscripten_get_now_is_monotonic) {
			now = _emscripten_get_now()
	} else {
			HEAPU32[errno >> 2] = 28
			return -1
	}
	HEAP32[tp >> 2] = now / 1e3 | 0;
	HEAP32[tp + 4 >> 2] = now % 1e3 * 1e3 * 1e3 | 0;
	return 0
}

function emscripten_realloc_buffer(size) {
	try {
		Q3e.memory.grow(size - Q3e.memory.buffer.byteLength + 65535 >>> 16);
		updateGlobalBufferAndViews(Q3e.memory.buffer);
		return 1;
	} catch (e) {
		console.error("emscripten_realloc_buffer: Attempted to grow heap from " 
			+ Q3e.memory.buffer.byteLength + " bytes to " 
			+ size + " bytes, but got error: " + e);
	}
}

function _emscripten_resize_heap(requestedSize) {
	var oldSize = HEAPU8.length;
	requestedSize = requestedSize >>> 0;
	console.assert(requestedSize > oldSize);
	var maxHeapSize = 2147483648;
	if (requestedSize > maxHeapSize) {
	err("Cannot enlarge memory, asked to go up to " + requestedSize + " bytes, but the limit is " + maxHeapSize + " bytes!");
	return false;
	}
	for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
	var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
	overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
	var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), 65536));
	var t0 = Sys_Milliseconds();
	var replacement = emscripten_realloc_buffer(newSize);
	var t1 = Sys_Milliseconds();
	console.log("Heap resize call from " + oldSize + " to " + newSize + " took " + (t1 - t0) + " msecs. Success: " + !!replacement);
	if (replacement) {
		return true;
	}
	}
	err("Failed to grow the heap from " + oldSize + " bytes to " + newSize + " bytes, not enough memory!");
	return false;
}

function _emscripten_get_heap_size() {
	return HEAPU8.length;
}

function dynCall(ret, func, args) {
	return Q3e.table.get(func).apply(null, args)
}

function CreateAndCall(code, params, vargs) {
	let func
	if(typeof SYS.evaledFuncs[code] != 'undefined') {
		func = SYS.evaledFuncs[code]
	} else {
		let paramStr = addressToString(params)
		func = SYS.evaledFuncs[code] = eval('(function func'
			+ ++SYS.evaledCount + '($0, $1, $2, $3)'
			+ addressToString(code, 4096) + ')')
		func.paramCount = paramStr.split(',').filter(function (name) {
			return name.length > 0
		}).length
	}
	let args = HEAPU32.slice(vargs >> 2, (vargs >> 2) + func.paramCount)
	return func.apply(func, args)
}

var SYS = {
	evaledFuncs: {},
	evaledCount: 0,
	DebugBreak: function () { debugger },
	DebugTrace: function () { console.log(new Error()) },
	emscripten_resize_heap: _emscripten_resize_heap,
	emscripten_get_heap_size: _emscripten_get_heap_size,
	Sys_RandomBytes: Sys_RandomBytes,
	Sys_Milliseconds: Sys_Milliseconds,
	Sys_Microseconds: Sys_Microseconds,
	Sys_Exit: Sys_Exit,
	exit: Sys_Exit,
	Sys_Frame: Sys_Frame,
	Sys_Error: Sys_Error,
	Sys_UnloadLibrary: Sys_UnloadLibrary,
	Sys_LoadLibrary: Sys_LoadLibrary,
	Sys_LoadFunction: Sys_LoadFunction,
	popen: function popen() {},
	Sys_Print: Sys_Print,
	Sys_SetStatus: Sys_SetStatus,
	CL_MenuModified: CL_MenuModified,
	Com_RealTime: Com_RealTime,
	CreateAndCall: CreateAndCall,
	clock_gettime: clock_gettime,
}
