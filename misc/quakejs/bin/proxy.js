var Buffer = require('buffer').Buffer
var dgram = require('dgram')
var WebSocketServer = require('ws').Server
var Huffman = require('../lib/huffman.js')
var {Server} = require('../lib/socks.server.js')
var {createServer} = require('net')

var socks = new Server() // TODO: add password authentication

// redirect http attempts to loading page
const server = createServer(function(socket) {
	try {
		socks._onConnection(socket)
	} catch (e) {
		console.log(e)
	}
})

const http = require('http')
const httpServer = http.createServer(function(req, res) {
	res.writeHead(200, {'Location': 'https://quake.games' + req.url})
	res.write('It works!')
	res.end()
})

var wss = new WebSocketServer({server: httpServer})

wss.on('connection', function(ws) {
	try {
		socks._onConnection(ws)
	} catch (e) {
		console.log(e)
	}
})

httpServer.listen(1081,  () => console.log(`Http running at http://0.0.0.0:1081`))
server.listen(1080, () => console.log(`Server running at http://0.0.0.0:1080`))
