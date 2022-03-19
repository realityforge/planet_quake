
function NET_AdrToString(net) {
  if(HEAPU32[net >> 2] == 2) {
    return stringToAddress('localhost')
  } else

  if(HEAPU32[net >> 2] == 4) {
    return stringToAddress(HEAPU8[net + 4] + '.'
      + HEAPU8[net + 5] + '.' + HEAPU8[net + 6] + '.'
      + HEAPU8[net + 7])
  }
}

function Sys_SockaddrToString() {
  // DNS doesn't work in the browser, but UDP works with SOCKS
  //   How complicated to add DNS lookup through SOCK?
  debugger
}

function Sys_StringToAdr(addr, net) {
  let addrStr = addressToString(addr)
  if(addrStr.match(/localhost/i)) {
    HEAPU32[net >> 2] = 2 /* NA_LOOPBACK */
    NET.lookup[addrStr] = [127, 0, 0, 1]
  } else

  if(typeof NET.lookup[addrStr] == 'undefined') {
    if(NET.lookupCount1 == 256) {
      NET.lookupCount2++;
      NET.lookupCount1 = 1;
    } else {
      NET.lookupCount1++;
    }
    HEAPU32[net >> 2] = 4 /* NA_IP */
		let ip = addrStr.match(/^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$/)
		if(ip) {
			NET.lookup[addrStr] = [
				parseInt(ip[1]), parseInt(ip[2]), 
				parseInt(ip[3]), parseInt(ip[4]), 
			]
		} else {
			NET.lookup[addrStr] = [10, 0, NET.lookupCount2, NET.lookupCount1]
		}
  }
  HEAPU8[net + 4] = NET.lookup[addrStr][0];
  HEAPU8[net + 5] = NET.lookup[addrStr][1];
  HEAPU8[net + 6] = NET.lookup[addrStr][2];
  HEAPU8[net + 7] = NET.lookup[addrStr][3];
  return true
}

function Sys_StringToSockaddr() {
  
}

function Sys_SendPacket(length, data, to) {
  let nameStr = addressToString(to + 8*8)
	let fullMessage = new Uint8Array(
		4
		+ (nameStr.length ? (nameStr.length + 1) : 4)
		+ 2 + length)
	fullMessage[0] = 0x00 // reserved 0x05
	fullMessage[1] = 0x00 // reserved 0x01
	fullMessage[2] = 0x00
	if(nameStr.length) {
		fullMessage[3] = 0x03
		fullMessage.set(nameStr.split('').map(c => c.charCodeAt(0)), 4)
	} else {
		fullMessage[3] = 0x01
		fullMessage[4] = HEAPU8[to + 4]
		fullMessage[5] = HEAPU8[to + 5]
		fullMessage[6] = HEAPU8[to + 6]
		fullMessage[7] = HEAPU8[to + 7]
	}
	fullMessage[8] = HEAPU8[to + 8]
	fullMessage[9] = HEAPU8[to + 9]
	fullMessage.set(HEAPU8.slice(data, data + length), fullMessage.length - length);
	if(NET.socket1) {
		NET.socket1.send(fullMessage)
	}
	if(NET.socket2) {
		NET.socket2.send(fullMessage)
	}
}

function NET_GetPacket() {
  debugger
}

function NET_Sleep() {

}

function sendHeartbeat(sock) {
  if(sock.readyState == sock.OPEN) {
		sock.fresh = 4
    sock.send(Uint8Array.from([0x05, 0x01, 0x00, 0x00]),
      { binary: true })
  } else if(sock.readyState == sock.CLOSED) {
    if(sock == NET.socket1) {
      NET.socket1 = null
      NET_OpenIP(true)
    } else {
      NET.socket2 = null
      NET_OpenIP(true)
    }
  }
}


function socketOpen(reconnect, socket, port) {
	socket.fresh = 1
	socket.send(Uint8Array.from([
    0x05, 0x01, 0x00, // no password caps?
  ]))
	if(!NET.heartbeat) {
		NET.heartbeat = setInterval(function () {
			sendHeartbeat(NET.socket1)
			NET.heartbeatTimeout = setTimeout(function () {
				sendHeartbeat(NET.socket2)
			}, 8000)
		}, 10000)
	}
  if(!reconnect) return
  socket.send(Uint8Array.from([
    0xFF, 0xFF, 0xFF, 0xFF,
    'p'.charCodeAt(0), 'o'.charCodeAt(0), 'r'.charCodeAt(0), 't'.charCodeAt(0),
    (port & 0xFF00) >> 8, (port & 0xFF)
  ]))
}

function socketMessage(socket, port, evt) {
  let message = new Uint8Array(evt.data)
  switch(socket.fresh) {
    case 1:
      if(message.length != 2) {
        throw new Error('wtf? this socket no worky')
      } else

      if(message[1] != 0) {
        throw new Error('this socket requires a password, dude')
      }

      // send the UDP associate request
      socket.send(Uint8Array.from([
				0x05, 0x03, 0x00, 0x01, 
				0x00, 0x00, 0x00, 0x00, // ip address
				(port & 0xFF00) >> 8, (port & 0xFF)
			]))
      socket.fresh = 2
    break
		case 2:
			if(message.length < 5) {
				throw new Error('denied, can\'t have ports')
			}

			if(message[3] != 1) {
				throw new Error('relay address is not IPV4')
			}

			socket.fresh = 3
		break
		case 3:
		case 4:
			// add messages to queue for processing
			if(socket.fresh == 4 && message.length == 2) {
				socket.fresh = 3
				return
			}
			debugger
		break
  }
}

function socketError(socket) {
  debugger
}

function NET_OpenIP(reconnect) {
  let port = addressToString(Cvar_VariableString(stringToAddress('net_port')))
  let peerAddress = addressToString(Cvar_VariableString(stringToAddress('net_socksServer')))
  let peerPort = addressToString(Cvar_VariableString(stringToAddress('net_socksPort')))
  if(!NET.buffer) {
    NET.buffer = malloc(16384 + 8 + 24) // from NET_Event() + netmsg_t
  }
  if(!NET.queue) {
    NET.queue = []
  }
  let fullAddress = 'ws' 
    + (window.location.protocol.length > 5 ? 's' : '')
    + '://' + peerAddress + ':' + peerPort
  if(!NET.socket1) {
    NET.socket1 = new WebSocket(fullAddress)
    NET.socket1.binaryType = 'arraybuffer';
    NET.socket1.addEventListener('open', socketOpen.bind(null, reconnect, NET.socket1, port), false)
    NET.socket1.addEventListener('message', socketMessage.bind(null, NET.socket1, port), false)
    NET.socket1.addEventListener('error', socketError.bind(null, NET.socket2, port), false)
  }
  if(!NET.socket2) {
    NET.socket2 = new WebSocket(fullAddress)
    NET.socket2.binaryType = 'arraybuffer';
    NET.socket2.addEventListener('open', socketOpen.bind(null, reconnect, NET.socket2, port), false)
    NET.socket2.addEventListener('message', socketMessage.bind(null, NET.socket2, port), false)
    NET.socket2.addEventListener('error', socketError.bind(null, NET.socket2, port), false)
  }
}

function NET_Close() {
  if(NET.heartbeat) {
    clearInterval(NET.heartbeat)
  }
  if(NET.heartbeatTimeout) {
    clearTimeout(NET.heartbeatTimeout)
  }
  if(NET.socket1) {
    NET.socket1.removeEventListener('open', null)
    NET.socket1.removeEventListener('message', null)
    NET.socket1.removeEventListener('close', null)
    NET.socket1.close()
    NET.socket1 = null
  }
  if(NET.socket2) {
    NET.socket2.removeEventListener('open', null)
    NET.socket2.removeEventListener('message', null)
    NET.socket2.removeEventListener('close', null)
    NET.socket2.close()
    NET.socket2 = null
  }
}

function Sys_IsLANAddress() {
  
}

function Sys_Offline() {

}

function Sys_NET_MulticastLocal (net, length, data) {
  debugger
  // all this does is use a dedicated server in a service worker
  window.serverWorker.postMessage([
    'net', net, Uint8Array.from(HEAP8.slice(data, data+length))])
}

function Com_DL_HeaderCallback(localName, response) {
  //let type = response.headers.get('Content-Type')
  if (!response || !(response.status >= 200 && response.status < 300 || response.status === 304)) {
    Sys_FileReady(stringToAddress(localName), null) // failed state, not to retry
    //throw new Error('Couldn\'t load ' + response.url + '. Status: ' + (response || {}).statusCode)
    response.body.getReader().cancel()
    // TODO: check for too big files!
    //if(controller)
      //controller.abort()
    return Promise.resolve()
  }
  return Promise.resolve(response.arrayBuffer())
}

function Com_DL_Begin(localName, remoteURL) {
  if(AbortController && !NET.controller) {
    NET.controller = new AbortController()
  }
  remoteURL += (remoteURL.includes('?') ? '&' : '?') + 'time=' + Q3e.cacheBuster
  return fetch(remoteURL, {
    mode: 'cors',
    responseType: 'arraybuffer',
    credentials: 'omit',
    signal: NET.controller ? NET.controller.signal : null
  })
  // why so many? this catches connection errors
  .catch(function (error) {
    return
  })
  .then(function (request) {
    return Com_DL_HeaderCallback(localName, request)
  })
  // this catches streaming errors, does everybody do this?
  .catch(function (error) {
    return
  })
}

function Com_DL_Perform(nameStr, localName, responseData) {
  NET.downloadCount--
  if(!responseData) {
    // already responded with null data
    return
  }
  // TODO: intercept this information here so we can invalidate the IDBFS storage
  if(localName.includes('version.json')) {
    Q3e.cacheBuster = Date.parse(JSON.parse(Array.from(new Uint8Array(responseData))
      .map(c => String.fromCharCode(c)).join(''))[0])
  }

  // don't store any index files, redownload every start
  if(nameStr.endsWith('/')) {
    let tempName = nameStr + '.' // yes this is where it always looks for temp files
      + Math.round(Math.random() * 0xFFFFFFFF).toString(16) + '.tmp'
    FS_CreatePath(stringToAddress(nameStr))
    FS.virtual[tempName] = {
      timestamp: new Date(),
      mode: 33206,
      contents: new Uint8Array(responseData)
    }
    Sys_FileReady(stringToAddress(localName), stringToAddress(tempName));
  } else {
    // TODO: JSON.parse
    // save the file in memory for now
    FS_CreatePath(stringToAddress(nameStr))
    FS.virtual[nameStr] = {
      timestamp: new Date(),
      mode: 33206,
      contents: new Uint8Array(responseData)
    }
    // async to filesystem
    // does it REALLY matter if it makes it? wont it just redownload?
    writeStore(FS.virtual[nameStr], nameStr)
    Sys_FileReady(stringToAddress(localName), stringToAddress(nameStr));
  }

}

function CL_Download(cmd, name, auto) {
  if(!FS.database) {
    openDatabase()
  }
  if(NET.downloadCount > 5) {
    return false // delay like cl_curl does
  }

  // TODO: make a utility for Cvar stuff?
  let dlURL = addressToString(Cvar_VariableString(stringToAddress("cl_dlURL")))
  let gamedir = addressToString(FS_GetCurrentGameDir())
  let nameStr = addressToString(name)
  let localName = nameStr
  if(localName[0] == '/')
    localName = localName.substring(1)
  if(localName.startsWith(gamedir))
    localName = localName.substring(gamedir.length)
  if(localName[0] == '/')
    localName = localName.substring(1)

  let remoteURL
  if(dlURL.includes('%1')) {
    remoteURL = dlURL.replace('%1', localName.replace(/\//ig, '%2F'))
  } else {
    remoteURL = dlURL + '/' + localName
  }
  if(remoteURL.includes('.googleapis.com')) {
    if(nameStr.endsWith('/')) {
      remoteURL = 'https://www.googleapis.com/storage/v1/b/'
        + remoteURL.match(/\/b\/(.*?)\/o\//)[1]
        + '/o/?includeTrailingDelimiter=true&maxResults=100&delimiter=%2f&prefix='
        + remoteURL.match(/\/o\/(.*)/)[1]
    } else if (!remoteURL.includes('?')) {
      remoteURL += '?alt=media'
    }
  }
  try {
    NET.downloadCount++

    readStore(nameStr)
      .then(function (result) {
        // always redownload indexes in case the content has changed?
        if(!result || result.mode == 16895) {
          return Com_DL_Begin(localName, remoteURL)
        // bust the caches!
        } else if(result.timestamp.getTime() < Q3e.cacheBuster) {
          return Com_DL_Begin(localName, remoteURL)
        // valid from disk
        } else {
          return Promise.resolve(result.contents)
        }
      })
      .then(function (responseData) {
        Com_DL_Perform(nameStr, localName, responseData)
      })
  } catch (e) {
    
  }
  if(!nameStr.includes(gamedir)) {
    debugger
    throw new Error('something wrong')
  }

  return true
}

var NET = {
  lookup: {},
  lookupCount1: 1,
  lookupCount2: 127,
  downloadCount: 0,
  controller: null,
  NET_AdrToString: NET_AdrToString,
  Sys_Offline: Sys_Offline,
  Sys_SockaddrToString: Sys_SockaddrToString,
  Sys_StringToSockaddr: Sys_StringToSockaddr,
  NET_GetPacket: NET_GetPacket,
  NET_Sleep: NET_Sleep,
  NET_OpenIP: NET_OpenIP,
  NET_Close: NET_Close,
  Sys_StringToAdr: Sys_StringToAdr,
  Sys_SendPacket: Sys_SendPacket,
  Sys_IsLANAddress: Sys_IsLANAddress,
  Sys_NET_MulticastLocal: Sys_NET_MulticastLocal,
  CL_Download: CL_Download,

}