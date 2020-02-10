var Buffer = require('buffer').Buffer
var dgram = require('dgram')
var WebSocketServer = require('ws').Server
var Huffman = require('../lib/huffman.js')
var {Server} = require('../lib/socks.server.js')

// redirect http attempts to loading page
const http = require('http')
const server = http.createServer(function(req, res) {
	res.writeHead(200, {'Location': 'https://quake.games' + req.url})
	res.write('It works!')
	res.end()
})

var wss = new WebSocketServer({server})
var socks = new Server() // TODO: add password authentication

wss.on('error', function(error) {
	console.log("error", error)
})
 
wss.on('connection', function(ws) {
	try {
		console.log('on connection....')
		socks._onConnection(ws._socket)

		/*
		var udpClient = dgram.createSocket('udp4')
		udpClient.on('message', function(msg, rinfo) {
			try {
				ws.send(msg)
			} catch(e) {
				console.log(`ws.send(${e})`)
			}
		})
		ws.on('message', function(message) {
			var msgBuff = Buffer.from(message)
			try {
				console.log(msgBuff.toString('utf-8'))
				// TODO: sniff websocket connection to figure out how to intercept the server address
				// emscripten qcommon/huffman.c and decode the message
				// add cl_main.c getchallenge/connect net_ip to control this proxy server
				// this will allow clients to control the proxy server to connect
				//   to any server they want from the browser
				//if(msgBuff.toString('utf-8').includes('connect ')) {
				//	debugger
				//}
				udpClient.send(msgBuff, 0, msgBuff.length, SERVER_PORT, SERVER_IP)
			} catch(e) {
				console.log(`udpClient.send(${e})`)
			}
		})
*/
	} catch (e) {
		console.log(e)
	}
})

server.listen(1080, () => console.log(`Server running at http://0.0.0.0:1080`))
