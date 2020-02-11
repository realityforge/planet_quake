const dgram = require('dgram')
var {createServer, Socket, isIP} = require('net')
var dns = require('dns')
var util = require('util')
var Parser = require('./socks.parser')
var ip6addr = require('ip6addr')
var WebSocket = require('ws')
var ipv6 = require('ipv6').v6
function ipbytes(str) {
  var type = isIP(str),
      nums,
      bytes,
      i;

  if (type === 4) {
    nums = str.split('.', 4);
    bytes = new Array(4);
    for (i = 0; i < 4; ++i) {
      if (isNaN(bytes[i] = +nums[i]))
        throw new Error('Error parsing IP: ' + str);
    }
  } else if (type === 6) {
    var addr = new ipv6.Address(str),
        b = 0,
        group;
    if (!addr.valid)
      throw new Error('Error parsing IP: ' + str);
    nums = addr.parsedAddress;
    bytes = new Array(16);
    for (i = 0; i < 8; ++i, b += 2) {
      group = parseInt(nums[i], 16);
      bytes[b] = group >>> 8;
      bytes[b + 1] = group & 0xFF;
    }
  }

  return bytes;
}

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

function Server() {
  if (!(this instanceof Server))
    return new Server()

  var self = this

  this._debug = true
  this._auths = []
  this._connections = 0
  this.maxConnections = Infinity
}

Server.prototype._onConnection = function(socket) {
  ++this._connections
  var self = this,
      parser = new Parser(socket)
      onData = parser._onData.bind(parser)
  
  if(socket instanceof WebSocket) {
    socket.on('message', onData)
  } else if (socket instanceof Socket) {
    socket.on('data', onData)
    socket.send = socket.write
    socket._socket = socket
    socket.close = socket.end
  }
  
  parser.on('error', function(err) {
    socket.off('data', onData)
    socket.close()
  }).on('methods', function(methods) {
    var auths = self._auths
    parser.authed = true
    socket.off('data', onData)
    socket._socket.pause()
    socket.send(Buffer.from([0x05, 0x00]))
    socket.on('data', onData)
    socket._socket.resume()
    //socket.send(BUF_AUTH_NO_ACCEPT)
  }).on('request', function(reqInfo) {
    console.log('Requesting', reqInfo.dstAddr)
    reqInfo.srcAddr = socket._socket.remoteAddress
    reqInfo.srcPort = socket._socket.remotePort
    socket.off('data', onData)
    var intercept = false // TODO: use this for something cool
    if (intercept) {
      socket.write(BUF_REP_INTR_SUCCESS);
      socket.removeListener('error', onErrorNoop);
      process.nextTick(function() {
        socket.resume();
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
      proxySocket(socket, reqInfo)
    }
  })

  function onClose() {
    socket.off('data', onData)
    if (socket.dstSock)
      socket.dstSock.end()
    socket.dstSock = undefined
  }

  socket.on('error', onErrorNoop)
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

exports.Server = Server

function onErrorNoop(err) {console.log(err)}

function proxySocket(socket, req) {
  dns.lookup(req.dstAddr, function(err, dstIP) {
    if (err) {
      handleProxyError(socket, err)
      return
    }

    function onError(err) {
      handleProxyError(socket, err)
    }
    
    function onConnect() {
      if(!socket._socket.writable) return
      var portLeft = Math.round(Math.random() * 50) * 1000 + 5000
      var port = socket.dstSock.localPort || socket._socket.localPort // portLeft + req.dstPort
      var localAddress = socket.dstSock.localAddress || socket._socket.localAddress
      var localbytes = ip6addr.parse(localAddress).toBuffer()
      var kind = ip6addr.parse(localAddress).kind()
      if(kind == 'ipv4') {
        localbytes = localbytes.slice(12)
      }
      var bufrep = Buffer.alloc(6 + localbytes.length)
      bufrep[0] = 0x05
      bufrep[1] = REP.SUCCESS
      bufrep[2] = 0x00
      bufrep[3] = (kind == 'ipv4' ? ATYP.IPv4 : ATYP.IPv6)
      for (var i = 0, p = 4; i < localbytes.length; ++i, ++p)
        bufrep[p] = localbytes[i]
      bufrep.writeUInt16BE(port, p, true)
      socket.send(bufrep)
      //socket._socket.on('data', socket.dstSock.write)
      //socket.dstSock.on('data', socket._socket.write)
      if(typeof socket.pipe == 'function') {
        socket.pipe(socket.dstSock).pipe(socket)
      } else {
        socket.dstSock.on('message', socket.send)
        socket.on('message', socket.dstSock.send)
      }
      //if(req.data.length) {
      //  socket.destSock.write(req.data)
      //}
    }
    
    if (req.cmd == 'udp') {
      const listener = dgram.createSocket('udp4')
      socket.dstSock = listener
      listener.on('listening', onConnect)
              .on('error', onError)
              .bind(req.dstPort, req.dstAddr)
    } else if(req.cmd == 'bind') {
      const listener = createServer()
      socket.dstSock = listener
      listener.on('connection', onConnect)
              .on('error', onError)
              .listen(req.dstPort, req.dstAddr)
    } else if(req.cmd == 'connect') {
      var dstSock = new Socket()
      socket.dstSock = dstSock
      dstSock.setKeepAlive(false)
      dstSock.on('error', onError)
             .on('connect', onConnect)
             .connect(req.dstPort, dstIP)
    } else {
      socket.send(BUF_REP_CMDUNSUPP)
      socket.close()
    }
  })
}

function handleProxyError(socket, err) {
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
  socket.close()
}
