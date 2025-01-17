
// this was for a web-worker setup
if(typeof global != 'undefined' && typeof global.window == 'undefined') {
	global.window = {}
}


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
	startup.push.apply(startup, window.preStart)
	startup.push.apply(startup, [
		'+set', 'r_fullscreen', window.fullscreen ? '1' : '0',
		'+set', 'r_customHeight', '' + GL.canvas.clientHeight || 0,
		'+set', 'r_customWidth', '' + GL.canvas.clientWidth || 0,
	])
	// meant to do this a lot sooner, with a download, we can just package
	//   whatever pk3/autoexec we want with the game.
	// but with web, we might be serving multiple sources, file:///index.html
	//   http://localhost/ and public quake.games/lvlworld. so i don't have
	//   to repackage for every source, check the domain we're on.
	let hostname = (/^(.*?)\./i).exec(window.location.hostname)
	let basename = false
	if(hostname) {
		basename = hostname[1]
	} else
	if(window.location.protocol == 'file:') {
		basename = 'localhost'
	}

	if(basename) {
		startup.push.apply(startup, [
			'+set', 'fs_basegame', basename,
			'+set', 'fs_game', 'lobby',
		])
	}
	var search = /([^&=]+)/g
	var query  = window.location.search.substring(1)
	var match
	while (match = search.exec(query)) {
		var val = decodeURIComponent(match[1])
		val = val.split(' ')
		val[0] = (val[0][0] != '+' ? '+' : '') + val[0]
		startup.push.apply(startup, val)
	}
	return startup
}


function Sys_UnloadLibrary() {

}

function Sys_LoadLibrary() {
	
}

function Sys_LoadFunction() {
	
}


function Sys_Print(message) {
	console.log(addressToString(message))
}

function Sys_Edit() {
	if(typeof window.ace == 'undefined') {
		return
	}

	if(Cmd_Argc() < 2) {
		Com_Printf(stringToAddress('Usage: edit [filename]\n'))
		return
	}

	let basegamedir = addressToString(FS_GetBaseGameDir())
	let gamedir = addressToString(FS_GetCurrentGameDir())
	let filename = Cmd_Argv(1)
	let filenameStr = addressToString(filename)
	if(filenameStr.startsWith('/')) {
		filenameStr = filenameStr.substr(1)
	}
	if(filenameStr.startsWith(gamedir)) {
		filenameStr = filenameStr.substr(gamedir.length)
	}
	if(filenameStr.startsWith(basegamedir)) {
		filenameStr = filenameStr.substr(basegamedir.length)
	}
	if(filenameStr.startsWith('/')) {
		filenameStr = filenameStr.substr(1)
	}
	if(!filenameStr || !filenameStr.length) {
		Com_Printf(stringToAddress('Usage: edit [filename]\n'))
		return
	}
	let openFilename = stringToAddress(filenameStr)

	let buf = stringToAddress('DEADBEEF') // pointer to pointer
	let length
	if ((length = FS_ReadFile(openFilename, buf)) > 0 && HEAPU32[buf >> 2] > 0) {
		let imageView = Array.from(HEAPU8.slice(HEAPU32[buf >> 2], HEAPU32[buf >> 2] + length))
		let utfEncoded = imageView.map(function (c) { return String.fromCharCode(c) }).join('')
		FS_FreeFile(HEAPU32[buf >> 2])
		ace.setValue(utfEncoded)
		// TODO: show relationships in Jarvis, 
		//   one module refers to another module
		//   these are the leaves of change that worry code reviewers
		ACE.filename = filenameStr
	} else {
		let vargs = stringToAddress('DEADBEEF') // pointer to pointer
		HEAPU32[vargs >> 2] = openFilename
		HEAPU32[(vargs >> 2) + 1] = 0
		Com_Printf(stringToAddress('File not found \'%s\'.\nUsage: edit [filename]\n'), vargs)
	}
}


function Sys_Exit(code) {
	SYS.exited = true
	GLimp_Shutdown();
	NET_Shutdown();
	if(SYS.frameInterval) {
		clearInterval(SYS.frameInterval)
		SYS.frameInterval = null
	}
	// redirect to lvlworld
	let returnUrl = addressToString(Cvar_VariableString(stringToAddress('cl_returnURL')))
	if(returnUrl) {
	//	window.location = returnUrl
	}
}

function Sys_Error(fmt, args) {
	let len = BG_sprintf(STD.sharedMemory + STD.sharedCounter, fmt, args)
	if(len > 0)
		console.error('Sys_Error: ', addressToString(STD.sharedMemory + STD.sharedCounter))
	Sys_Exit( 1 )
	throw new Error(addressToString(fmt))
}

function Sys_SetStatus(status, replacementStr) {
	// TODO: something like  window.title = , then setTimeout( window.title = 'Quake3e' again)
	
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
	if(!SYS.menuInited) { // keep track of first time the ui.qvm appears
		SYS.menuInited = true
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
	if(SYS.inFrame) {
		return
	}
	function doFrame() {
		SYS.inFrame = true
		SYS.running = !SYS.running
		try {
			if(typeof window.ace != 'undefined') {
				Ace_Frame()
			}
			Com_Frame(SYS.running)
		} catch (e) {
			if(!SYS.exited && e.message == 'longjmp') {
				// let game Com_Frame handle it, it will restart UIVM
				Cbuf_AddText(stringToAddress('vid_restart\n'));
				console.error(e)
			} else
			if(!SYS.exited || e.message != 'unreachable') {
				Sys_Exit(1)
				throw e
			}
		}
		SYS.inFrame = false
	}
	if(HEAP32[gw_active >> 2]) {
		requestAnimationFrame(doFrame)
	} else {
		doFrame()
	}
}

function Sys_notify(ifile, path, fp) {
	openDatabase().then(function (db) {
		writeStore(ifile, path)
	})
	// TODO: ADD FILESYSTEM WATCHERS API INOTIFY 
	//   THAT READS A LIST GENERATED HERE
	if(typeof window.updateFilelist != 'undefined'
		&& !ACE.filestimer) {
		ACE.filestimer = setTimeout(updateFilelist, 100)
	}
}

/*

function alignUp(x, multiple) {
	if (x % multiple > 0) {
	x += multiple - x % multiple;
	}
	return x;
}

var _emscripten_get_now_is_monotonic = true;

function _emscripten_get_now() {
	return performance.now()
}

function emscripten_realloc_buffer(size) {
	try {
		Module.memory.grow(size - Module.memory.buffer.byteLength + 65535 >>> 16);
		updateGlobalBufferAndViews(Module.memory.buffer);
		return 1;
	} catch (e) {
		console.error("emscripten_realloc_buffer: Attempted to grow heap from " 
			+ Module.memory.buffer.byteLength + " bytes to " 
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
*/


function dynCall(ret, func, args) {
	return Module.table.get(func).apply(null, args)
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

function Sys_RandomBytes (string, len) {
  if(!HEAP8.buffer.length) {
    updateGlobalBufferAndViews()
  }
	if(typeof crypto != 'undefined') {
		crypto.getRandomValues(HEAP8.subarray(string, string+(len / 4)))
	} else {
		for(let i = 0; i < (len / 4); i++) {
			HEAP8[string] = Math.random() * 255
		}
	}
	return true;
}

var SYS = {
	evaledFuncs: {},
	evaledCount: 0,
	DebugBreak: function () { debugger },
	DebugTrace: function () { console.log(new Error()) },
	//emscripten_resize_heap: _emscripten_resize_heap,
	//emscripten_get_heap_size: _emscripten_get_heap_size,
	Sys_RandomBytes: Sys_RandomBytes,
	Sys_Exit: Sys_Exit,
	Sys_Edit: Sys_Edit,
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
	CreateAndCall: CreateAndCall,
}
