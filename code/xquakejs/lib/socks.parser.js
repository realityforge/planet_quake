var Buffer = require('buffer').Buffer
var inherits = require('util').inherits
var EventEmitter = require('events').EventEmitter

var CMD = {
  CONNECT: 0x01,
  BIND: 0x02,
  UDP: 0x03,
  WS: 0x04
}
var ATYP = {
  IPv4: 0x01,
  NAME: 0x03,
  IPv6: 0x04
}

var STATE_VERSION = 0,
    STATE_NMETHODS = 1,
    STATE_METHODS = 2,
    STATE_REQ_CMD = 3,
    STATE_REQ_RSV = 4,
    STATE_REQ_ATYP = 5,
    STATE_REQ_DSTADDR = 6,
    STATE_REQ_DSTADDR_VARLEN = 7,
    STATE_REQ_DSTPORT = 8

function Parser(ws) {
  this._ws = ws
  this._listening = false
  
  this._buffer = Buffer.alloc(1400*3, 0)
  this._state = STATE_VERSION
  this._methods = undefined
  this._methodsp = 0
  this._cmd = 0
  this._atyp = 0
  this._dstaddr = undefined
  this._dstaddrp = 0
  this._dstport = undefined

  this.authed = false
}
inherits(Parser, EventEmitter)

Parser.CMD = CMD
Parser.prototype._onData = function(message) {
  if(message === null) {
    this._buffer.fill(0)
    return
  }
  if(typeof message == 'string') {
    this._buffer.write(message)
  } else {
    message.copy(this._buffer, 0, 0, message.length)
  }
  var state = this._state,
      i = 0,
      len = message.length,
      left,
      chunkLeft,
      minLen
  // connection and version number are implied from now on
  if(this.authed && Object.values(ATYP).includes(this._buffer[3])) {
    this._buffer[0] = 0x05
    this._buffer[1] = this._buffer[1] || 0x01 // could be 1 or 4 from client
    this._buffer[2] = 0x00
  }
  //console.log(chunk)
  while (i < len) {
    // emscripten overdoing it a little
    if(this._buffer[i] === 0xFF && this._buffer[i+1] === 0xFF
      && this._buffer[i+2] === 0xFF && this._buffer[i+3] === 0xFF
      && this._buffer[i+4] === 'p'.charCodeAt(0)
      && this._buffer[i+5] === 'o'.charCodeAt(0)
      && this._buffer[i+6] === 'r'.charCodeAt(0)
      && this._buffer[i+7] === 't'.charCodeAt(0)) {
      // then we know we are in UDP mode, thanks a lot
      this._dstport = this._buffer[i+8]
      this._dstport <<= 8
      this._dstport += this._buffer[i+9]
      this.authed = true
      this.emit('request', {
        cmd: CMD.UDP,
        dstPort: this._dstport
      })
      return
    }
    switch (state) {
      /*
        +----+----------+----------+
        |VER | NMETHODS | METHODS  |
        +----+----------+----------+
        | 1  |    1     | 1 to 255 |
        +----+----------+----------+
      */
      case STATE_VERSION:
        if(this._buffer[i] !== 0x05)
          if (this.authed && this._buffer[i] === 0x00) {
          } else {
            if(this._buffer[i] !== 0x00)
              this.emit('error', new Error('Incompatible SOCKS protocol version: '
                                  + this._buffer[i]))
            return
          }
        ++i
        if (this.authed)
          state = STATE_REQ_CMD
        else
          ++state
      break
      case STATE_NMETHODS:
        var nmethods = this._buffer[i]
        if (nmethods === 0) {
          this.emit('error', new Error('Unexpected empty methods list'))
          return
        }
        ++i
        ++state
        this._methods = Buffer.alloc(nmethods)
        this._methodsp = 0
      break
      case STATE_METHODS:
        left = this._methods.length - this._methodsp
        chunkLeft = len - i
        minLen = (left < chunkLeft ? left : chunkLeft)
        this._buffer.copy(this._methods,
                   this._methodsp,
                   i,
                   i + minLen)
        this._methodsp += minLen
        i += minLen
        if (this._methodsp === this._methods.length) {
          this._state = STATE_VERSION
          var methods = this._methods
          this._methods = undefined
          if(this._ws._socket.pause)
            this._ws._socket.pause()
          this.emit('methods', methods)
          return
        }
      break
      // =======================================================================
      /*
        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

        Where:

              o  VER    protocol version: X'05'
              o  CMD
                 o  CONNECT X'01'
                 o  BIND X'02'
                 o  UDP ASSOCIATE X'03'
              o  RSV    RESERVED
              o  ATYP   address type of following address
                 o  IP V4 address: X'01'
                 o  DOMAINNAME: X'03'
                 o  IP V6 address: X'04'
              o  DST.ADDR       desired destination address
              o  DST.PORT desired destination port in network octet
                 order
      */
      case STATE_REQ_CMD:
        var cmd = this._buffer[i]
        this._cmd = cmd || CMD.CONNECT
        if(cmd < 0 || cmd > 0x04) {
          this.emit('error', new Error('Invalid request command: ' + cmd))
          return
        }
        this._dstport = void 0
        ++i
        ++state
      break
      case STATE_REQ_RSV:
        ++i
        ++state
      break
      case STATE_REQ_ATYP:
        var atyp = this._buffer[i]
        state = STATE_REQ_DSTADDR
        if (atyp === ATYP.IPv4)
          this._dstaddr = Buffer.alloc(4)
        else if (atyp === ATYP.IPv6)
          this._dstaddr = Buffer.alloc(16)
        else if (atyp === ATYP.NAME)
          state = STATE_REQ_DSTADDR_VARLEN
        else if (atyp === 0) {
          this.emit('ping')
          return
        } else {
          this.emit('error', new Error('Invalid request address type: ' + atyp))
          return
        }
        this._atyp = atyp
        ++i
      break
      case STATE_REQ_DSTADDR:
        left = this._dstaddr.length - this._dstaddrp
        chunkLeft = len - i
        minLen = (left < chunkLeft ? left : chunkLeft)
        this._buffer.copy(this._dstaddr, this._dstaddrp, i, i + minLen)
        this._dstaddrp += minLen
        i += minLen
        if (this._dstaddrp === this._dstaddr.length)
          state = STATE_REQ_DSTPORT
      break
      case STATE_REQ_DSTADDR_VARLEN:
        this._dstaddr = Buffer.alloc(this._buffer[i])
        this._dstaddrp = 0
        state = STATE_REQ_DSTADDR
        ++i
      break
      case STATE_REQ_DSTPORT:
        if (this._dstport === undefined)
          this._dstport = this._buffer[i]
        else {
          this._dstport <<= 8
          this._dstport += this._buffer[i]
          ++i

          if (this._atyp === ATYP.IPv4)
            this._dstaddr = Array.prototype.join.call(this._dstaddr, '.')
          else if (this._atyp === ATYP.IPv6) {
            var ipv6str = '',
                addr = this._dstaddr
            for (var b = 0; b < 16; ++b) {
              if (b % 2 === 0 && b > 0)
                ipv6str += ':'
              ipv6str += addr[b].toString(16)
            }
            this._dstaddr = ipv6str
          } else
            this._dstaddr = this._dstaddr.toString()

          if(this._ws._socket.pause)
            this._ws._socket.pause()
          this.emit('request', {
            cmd: this._cmd,
            srcAddr: undefined,
            srcPort: undefined,
            dstAddr: this._dstaddr.replace('\0', ''),
            dstPort: this._dstport,
            data: Buffer.from(this._buffer.subarray(i, len))
          })
          return
        }
        ++i
      break
      // ===================================================================
    }
  }

  this._state = state
}

module.exports = Parser
