const dgram = require('dgram')
var {createServer, Socket, isIP} = require('net')
var dns = require('dns')
var util = require('util')
var Parser = require('./socks.parser')
var ip6addr = require('ip6addr')
var WebSocket = require('ws')
var WebSocketServer = require('ws').Server;
const http = require('http');

var UDP_TIMEOUT = 330 * 1000 // clear stale listeners so we don't run out of ports,
  // must be longer than any typical client timeout, maybe the map takes too long to load?
  // longer than server HEARTBEAT_MSEC
var ATYP = {
  IPv4: 0x01,
  NAME: 0x03,
  IPv6: 0x04
}
var REP = {
  SUCCESS: 0x00,
  GENFAIL: 0x01,
  DISALLOW: 0x02,
  NETUNREACH: 0x03,
  HOSTUNREACH: 0x04,
  CONNREFUSED: 0x05,
  TTLEXPIRED: 0x06,
  CMDUNSUPP: 0x07,
  ATYPUNSUPP: 0x08
}

var BUF_AUTH_NO_ACCEPT = Buffer.from([0x05, 0xFF]),
    BUF_REP_INTR_SUCCESS = Buffer.from([0x05,
                                       REP.SUCCESS,
                                       0x00,
                                       0x01,
                                       0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00]),
    BUF_REP_DISALLOW = Buffer.from([0x05, REP.DISALLOW]),
    BUF_REP_CMDUNSUPP = Buffer.from([0x05, REP.CMDUNSUPP])

function Server(opts) {
  if (!(this instanceof Server))
    return new Server()

  var self = this
  this._slaves = (opts || {}).slaves || []
  this._listeners = {}
  this._receivers = {}
  this._directConnects = {}
  this._timeouts = {}
  this._dnsLookup = {}
  this._debug = true
  this._auths = []
  this._connections = 0
  this.maxConnections = Infinity
  setInterval(() => {
    Object.keys(this._timeouts).forEach(k => {
      if(this._timeouts[k] < Date.now() - UDP_TIMEOUT)
        self._timeoutUDP(k)
    })
  }, 100)
}

Server.prototype._onClose = function (socket, onData) {
  console.error('Closing ', socket._socket.remoteAddress)
  socket.off('data', onData)
  socket.off('message', onData)
  /*
  if (socket.dstSock) {
    if(typeof socket.dstSock.end == 'function')
      socket.dstSock.end()
    else if(typeof socket.dstSock.close == 'function')
      socket.dstSock.close()
  }
  */
  socket.dstSock = undefined
  if(socket._socket.writable) {
    socket.on('data', onData)
    socket.on('message', onData)
  }
}

Server.prototype._onParseError = function(socket, onData, err) {
  console.log('Parse error ', err)
  socket.off('data', onData)
  socket.off('message', onData)
  socket.close()
}

Server.prototype._onMethods = function(parser, socket, onData, methods) {
  var auths = this._auths
  parser.authed = true
  socket.off('data', onData)
  socket.off('message', onData)
  socket.send(Buffer.from([0x05, 0x00]))
  socket.on('data', onData)
  socket.on('message', onData)
  //socket.send(BUF_AUTH_NO_ACCEPT)
}

Server.prototype._onRequest = async function(socket, onData, reqInfo) {
  reqInfo.srcAddr = socket._socket.remoteAddress
  reqInfo.srcPort = socket._socket.remotePort
  var intercept = false // TODO: use this for something cool
  if (intercept && !reqInfo.dstAddr.includes('0.0.0.0')) {
    socket.send(BUF_REP_INTR_SUCCESS);
    socket.removeListener('error', this._onErrorNoop);
    process.nextTick(function() {
      var body = 'Hello ' + reqInfo.srcAddr + '!\n\nToday is: ' + (new Date());
      socket.send([
        'HTTP/1.1 200 OK',
        'Connection: close',
        'Content-Type: text/plain',
        'Content-Length: ' + Buffer.byteLength(body),
        '',
        body
      ].join('\r\n'));
    });
    return socket;
  } else {
    console.log('Requesting', reqInfo.cmd, reqInfo.dstAddr, ':', reqInfo.dstPort)
    await this.proxySocket.apply(this, [socket, reqInfo])
  }
}

Server.prototype.lookupDNS = async function (address) {
  var self = this
  if(typeof this._dnsLookup[address] != 'undefined')
    return this._dnsLookup[address]
  return new Promise((resolve, reject) => dns.lookup(address, function(err, dstIP) {
    if(err) {
      return reject(err)
    }
    if(address.localeCompare(dstIP, 'en', { sensitivity: 'base' }) > 0) {
      console.log('DNS found ' + address + ' -> ' + dstIP)
      self._dnsLookup[address] = dstIP
    }
    return resolve(dstIP)
  }))
}

Server.prototype._onUDPMessage = function (udpLookupPort, message, rinfo) {
  var self = this
  var socket = self._receivers[udpLookupPort]
  // is this valid SOCKS5 for UDP?
  var returnIP = false
  var ipv6 = ip6addr.parse(rinfo.address)
  var localbytes = ipv6.toBuffer()
  if(ipv6.kind() == 'ipv4') {
    localbytes = localbytes.slice(12)
  }
  var domain = Object.keys(this._dnsLookup)
    .filter(n => this._dnsLookup[n] == rinfo.address)[0]
  var bufrep = returnIP || !domain
    ? Buffer.alloc(4 + localbytes.length + 2 /* port */)
    : Buffer.alloc(4 + 1 /* for strlen */ + domain.length + 1 /* \0 null */ + 2)
  bufrep[0] = 0x00
  bufrep[1] = 0x00
  bufrep[2] = 0x00
  if(returnIP || !domain) {
    bufrep[3] = 0x01
    for (var i = 0, p = 4; i < localbytes.length; ++i, ++p) {
      bufrep[p] = localbytes[i]
    }
    bufrep.writeUInt16BE(rinfo.port, 8, true)
  } else {
    var domainBuf = Buffer.from(domain)
    bufrep[3] = 0x03
    bufrep[4] = domain.length+1
    domainBuf.copy(bufrep, 5)
    bufrep.writeUInt16BE(rinfo.port, 5 + bufrep[4], true)
  }
  console.log('UDP message from', rinfo.address, ' -> ', udpLookupPort)
  socket.send(Buffer.concat([bufrep, message]))
}

Server.prototype._timeoutUDP = function(udpLookupPort) {
  var self = this
  if(typeof self._listeners[udpLookupPort] !== 'undefined')
    self._listeners[udpLookupPort].close()
  delete self._listeners[udpLookupPort]
}

Server.prototype._onConnection = function(socket) {
  ++this._connections
  var parser = new Parser(socket)
      onData = parser._onData.bind(parser),
      onError = this._onParseError.bind(this, socket, onData), // data for unbinding, err passed in
      onMethods = this._onMethods.bind(this, parser, socket, onData),
      onRequest = this._onRequest.bind(this, socket, onData), // reqInfo passed in
      onClose = this._onClose.bind(this, socket, onData)
      
  if(socket instanceof WebSocket) {
    console.log(`Websocket connection ${socket._socket.remoteAddress}:${socket._socket.remotePort}....`)
    socket.on('message', onData)
  } else if (socket instanceof Socket) {
    console.log(`Net socket connection ${socket.remoteAddress}:${socket.remotePort}....`)
    socket.on('data', onData)
    socket.send = socket.write
    socket._socket = socket
    socket.close = socket.end
  } else {
    console.log('Socket type unknown!')
    socket.close()
    return
  }
  
  parser
    .on('error', onError)
    .on('methods', onMethods)
    .on('request', onRequest)

  socket.parser = parser
  socket.on('error', this._onErrorNoop)
        .on('close', onClose)
}

Server.prototype.useAuth = function(auth) {
  if (typeof auth !== 'object'
      || typeof auth.server !== 'function'
      || auth.server.length !== 2)
    throw new Error('Invalid authentication handler')
  else if (this._auths.length >= 255)
    throw new Error('Too many authentication handlers (limited to 255).')

  this._auths.push(auth)

  return this
}

Server.prototype._onErrorNoop = function(err) {
  console.log(err)
}

Server.prototype._onSocketConnect = function(udpLookupPort, reqInfo) {
  var self = this
  var socket = self._receivers[udpLookupPort]
  if(!socket._socket.writable) return
  var ipv6 = ip6addr.parse(this._slaves[0] || socket._socket.localAddress)
  var localbytes = ipv6.toBuffer()
  if(ipv6.kind() == 'ipv4') {
    localbytes = localbytes.slice(12)
  }
  var bufrep = Buffer.alloc(6 + localbytes.length)
  bufrep[0] = 0x05
  bufrep[1] = REP.SUCCESS
  bufrep[2] = 0x00
  bufrep[3] = (ipv6.kind() == 'ipv4' ? ATYP.IPv4 : ATYP.IPv6)
  for (var i = 0, p = 4; i < localbytes.length; ++i, ++p)
    bufrep[p] = localbytes[i]
  bufrep.writeUInt16BE(socket._socket.localPort, p, true)
  socket.send(bufrep)

  // do some new piping for the socket
  if(typeof socket.dstSock == 'function') {
    console.log('Starting pipe')
    socket._socket.pipe(socket.dstSock)
    socket.dstSock.pipe(socket._socket)
  } else {
    console.log('Starting messages ' + ipv6.kind())
    socket.send(bufrep)
  }
}

Server.prototype._onProxyError = function(udpLookupPort, err) {
  var socket = this._receivers[udpLookupPort]
  console.log(err)
  if(!socket._socket.writable) return
  var errbuf = Buffer.from([0x05, REP.GENFAIL])
  if (err.code) {
    switch (err.code) {
      case 'ENOENT':
      case 'ENOTFOUND':
      case 'ETIMEDOUT':
      case 'EHOSTUNREACH':
        errbuf[1] = REP.HOSTUNREACH
      break
      case 'ENETUNREACH':
        errbuf[1] = REP.NETUNREACH
      break
      case 'ECONNREFUSED':
        errbuf[1] = REP.CONNREFUSED
      break
    }
  }
  socket.send(errbuf)
}

Server.prototype.tryBindPort = async function(reqInfo) {
  var self = this
  var onUDPMessage = this._onUDPMessage.bind(self, reqInfo.dstPort)
  for(var i = 0; i < 10; i++) {
    try {
      var fail = false
      var portLeft = Math.round(Math.random() * 50) * 1000 + 5000
      var portRight = reqInfo.dstPort & 0xfff
      const listener = dgram.createSocket('udp4')
      console.log('Starting listener ', reqInfo.dstAddr, portLeft + portRight, ' -> ', reqInfo.dstPort)
      await new Promise((resolve, reject) => listener
        .on('listening', resolve)
        .on('close', () => {
          delete this._listeners[reqInfo.dstPort]
        })
        .on('error', reject)
        .on('message', onUDPMessage)
        .bind(portLeft + portRight, reqInfo.dstAddr || '0.0.0.0'))
      // TODO: fix this, port will be the same for every client
      //   client needs to request the random port we assign
      self._listeners[reqInfo.dstPort] = listener
      self._timeouts[reqInfo.dstPort] = Date.now()
      return listener
    } catch(e) {
      if(!e.code.includes('EADDRINUSE')) throw e
    }
  }
  throw new Error('Failed to start UDP listener.')
}

Server.prototype.websockify = async function (reqInfo) {
  var self = this
  var onUDPMessage = self._onUDPMessage.bind(self, reqInfo.dstPort)
  var onError = this._onProxyError.bind(this, reqInfo.dstPort)
  var httpServer = http.createServer()
  var wss = new WebSocketServer({server: httpServer})
  var port = self._listeners[reqInfo.dstPort].address().port
  wss.on('connection', function(ws, req) {
    var remoteAddr = req.socket.remoteAddress+':'+req.socket.remotePort
    console.log('Direct connect from ' + remoteAddr)
    ws.on('message', (msg) => onUDPMessage(Buffer.from(msg), {
      address: req.socket.remoteAddress, port: req.socket.remotePort
    }))
      .on('error', this._onErrorNoop)
      .on('close', () => delete self._directConnects[remoteAddr])
    self._directConnects[remoteAddr] = ws;
  })
  self._listeners[reqInfo.dstPort].on('close', () => {
    wss.close()
  })
  await new Promise(resolve => httpServer.listen(port, reqInfo.dstAddr, resolve))
}

Server.prototype.websocketRequest = async function (onError, onUDPMessage, reqInfo, dstIP) {
  var self = this
  //var onConnect = this._onSocketConnect.bind(this, reqInfo.dstPort, reqInfo)
  var remoteAddr = dstIP+':'+reqInfo.dstPort
  if(typeof self._directConnects[remoteAddr] == 'undefined'
    || self._directConnects[remoteAddr].readyState > 1) {
    console.log('Websocket request ' + remoteAddr)
    self._directConnects[remoteAddr] = new WebSocket(`ws://${remoteAddr}`)
    self._directConnects[remoteAddr]
      .on('message', (msg) => self._directConnects[remoteAddr]
        ._message(Buffer.from(msg), {
          address: dstIP, port: reqInfo.dstPort
        }))
      .on('error', (err) => self._directConnects[remoteAddr]._error(err))
      .on('open', () => {
        //onConnect()
        self._directConnects[remoteAddr]._pending.forEach(d => {
          self._directConnects[remoteAddr].send(d)
        })
      })
      .on('close', () => delete self._directConnects[remoteAddr])
    self._directConnects[remoteAddr]._pending = [
      reqInfo.data
    ]
  } else if (self._directConnects[remoteAddr].readyState !== 1) {
    self._directConnects[remoteAddr]._pending.push(reqInfo.data)
  } else {
    self._directConnects[remoteAddr].send(reqInfo.data)
  }
  self._directConnects[remoteAddr]._message = onUDPMessage
  self._directConnects[remoteAddr]._error = onError
}

Server.prototype.proxySocket = async function(socket, reqInfo) {
  var self = this
  var onConnect = this._onSocketConnect.bind(this, reqInfo.dstPort, reqInfo)
  var onError = this._onProxyError.bind(this, reqInfo.dstPort) // err is passed in
  var dstIP
  try {
    dstIP = await this.lookupDNS(reqInfo.dstAddr || '0.0.0.0')
  } catch (e) {
    console.log('DNS error:', e)
    return
  }
  try {
    var remoteAddr = dstIP+':'+reqInfo.dstPort
    // socket was connected from outside websocket connection
    /*if((reqInfo.cmd == 'connect' || reqInfo.cmd == 'ws')
      && typeof self._directConnects[remoteAddr] != 'undefined') {
      if (self._directConnects[remoteAddr].readyState !== 1) {
        self._directConnects[remoteAddr]._pending.push(reqInfo.data)
      } else {
        self._directConnects[remoteAddr].send(reqInfo.data)
      }
    // web-socket is bound to a specific socket already
    } else if(reqInfo.cmd == 'connect' && socket.dstSock) {
      self._timeouts[socket.dstSock.address().port] = Date.now()
      socket.dstSock.send(reqInfo.data, 0, reqInfo.data.length, reqInfo.dstPort, dstIP)
    // continue with normal commands
  } else*/ if (reqInfo.cmd == 'udp') {
      socket.parser.authed = true
      self._receivers[reqInfo.dstPort] = socket
      if(typeof this._listeners[reqInfo.dstPort] == 'undefined'
        || this._listeners[reqInfo.dstPort].readyState > 1) {
        await self.tryBindPort(reqInfo)
        // TODO: make command line option --no-ws to turn this off
        await self.websockify(reqInfo)
        onConnect()
      } else if (reqInfo.dstAddr) {
        onConnect()
      } else {
      }
      console.log('Switching to UDP listener', reqInfo.dstPort)
      socket.dstSock = this._listeners[reqInfo.dstPort]
    } else if(reqInfo.cmd == 'bind') {
      socket.parser.authed = true
      self._receivers[reqInfo.dstPort] = socket
      const listener = createServer()
      socket.dstSock = listener
      listener.on('connection', () => {})
              .on('error', onError)
              .listen(reqInfo.dstPort, reqInfo.dstAddr, onConnect)
    } else if(reqInfo.cmd == 'connect') {
      if(socket.dstSock) {
        var port = Object.keys(self._listeners)
          .filter(k => self._listeners[k] === socket.dstSock)[0]
          || reqInfo.srcPort
        self._timeouts[port] = Date.now()
        socket.dstSock.send(reqInfo.data, 0, reqInfo.data.length, reqInfo.dstPort, dstIP)
      } else {
        console.log('SHOULD NEVER HIT HERE', reqInfo)
        var dstSock = new Socket()
        socket.dstSock = dstSock
        dstSock.setKeepAlive(false)
        dstSock.on('error', this._onErrorNoop)
               .on('connect', onConnect)
               .connect(reqInfo.dstPort, dstIP)
      }
    // special websocket piping for quakejs servers
    } else if(reqInfo.cmd == 'ws') {
      var port = Object.keys(self._receivers)
        .filter(k => self._receivers[k] === socket)[0]
        || reqInfo.srcPort
      self._receivers[port] = socket
      await self.websocketRequest(
        self._onProxyError.bind(self, port),
        self._onUDPMessage.bind(self, port),
        reqInfo, dstIP)
    } else {
      socket.send(BUF_REP_CMDUNSUPP)
      socket.close()
    }
  } catch (err) {
    console.log('Request error:', err)
  }
}

exports.Server = Server
