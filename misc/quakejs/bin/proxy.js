var Buffer = require('buffer').Buffer;
var dgram = require('dgram');
var WebSocketServer = require('ws').Server;
const http = require('http');
const server = http.createServer(function(req, res) {
	res.writeHead(200, {'Location': 'https://quake.games' + req.url});
	res.write('It works!')
	res.end();
});

var wss = new WebSocketServer({server});

// TODO: rewrite this to use node dns
var SERVER_IP = '35.208.205.189'
var SERVER_PORT = 27960
 
wss.on('error', function(error) {
	console.log("error", error);
});
 
wss.on('connection', function(ws) {
	try {
	
		console.log("on connection....", ws);
		//Create a udp socket for this websocket connection
		var udpClient = dgram.createSocket('udp4');
		
		//ws.send("HAIIII");
		
		//When a message is received from udp server send it to the ws client
		udpClient.on('message', function(msg, rinfo) {
			//console.log("udp -> ws", msg);
			try {
				ws.send(msg);
			} catch(e) { console.log(`ws.send(${e})`) }
			//ws.send("test");
		});
	 
		//When a message is received from ws client send it to udp server.
		ws.on('message', function(message) {
			var msgBuff = new Buffer.from(message);
			try {
				udpClient.send(msgBuff, 0, msgBuff.length, SERVER_PORT, SERVER_IP);
			} catch(e) { console.log("udpClient.send") }
			//console.log("ws -> udp", message);
		});
	} catch (e) {
		console.log(e);
	}
});

server.listen(27960, () => console.log(`Server running at http://0.0.0.0:27960`));
