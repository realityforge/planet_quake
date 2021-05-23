var express = require('express')
var path = require('path')
var {ufs} = require('unionfs')
var express = require('express')
var {pathToAbsolute, makeIndexJson, repackPk3Dir} = require('./content.js')
var {sendCompressed} = require('./compress.js')
var serveStatic = require('../lib/serve-static.js')
var {Server} = require('../lib/socks.server.js')
var WebSocketServer = require('ws').Server
var http = require('http')
var {createServer} = require('net')

var help = `
npm run proxy [options]
--help -h - print this help message and exit
--proxy-ip - (x-forwarded-for header) redirect websocket requests to a specific IP address, 
  good for telling master server how to reach game server
--max {number} - Max number of connections for this process
--master - Run a Q3 master server in addition to web service
TODO:
--master - Designated master server, force redirection after connection
--slave - is implied from lack of --master, don't need this option
If max is set, the following number is the number of connections,
If max is not set or any number following max is assumed to be port numbers
all numbers and addresses are used as slave addresses
e.g. 127.0.0.1:1080 1081 1082 192.168.0.120 1080-1082 or CIDR notations
All of these numbers and addresses will be fully enumerated as a part of health check
--log-71 - show log data for protocol 71
--log-http - show log data for http traffic
--no-ws - Do NOT automatically create a websocket server for port bindings, 
  useful if you game already handles TCP connections
Add a command to check the number of connections so it can be used as health check
Either using the socket or event using process signals to communicate across process threads
Add websocket piping back in for quakejs servers
e.g. npm run proxy -- --max 10
`

var ports = [1081, 80, 8080]
var masterServer = false
var specifyPorts = false
var forwardIP = ''
var slaves = []
for(var i = 0; i < process.argv.length; i++) {
  var a = process.argv[i]
  if(a.match(/\/node$/ig)) continue
  if(a.match(/\/web\.js$/ig)) continue

  if(a == '--help' || a == '-h') {
		console.log(help)
    process.exit(0)
  } else if (parseInt(a) + '' === a) {
    if(!specifyPorts) {
      ports = []
      specifyPorts = true
    }
    ports.push(parseInt(a))
	} else if (a == '--master') {
    console.log('Starting master server.')
    masterServer = true
  } else if (a == '--proxy-ip') {
    console.log('Forwarding ip address: ', process.argv[i+1])
    forwardIP = process.argv[i+1]
    i++
  } else {
    try {
      var ipv6 = ip6addr.parse(a)
      slaves.push(a)
    } catch (e) {
      console.log(`ERROR: Unrecognized option "${a}"`, e)
    }
	}
}

var socks = new Server({slaves, proxy: forwardIP}) // TODO: add password authentication

var app = express()
app.enable('etag')
app.set('etag', 'strong')
express.static.mime.types['wasm'] = 'application/wasm'
express.static.mime.types['pk3'] = 'application/octet-stream'
express.static.mime.types['bsp'] = 'application/octet-stream'

function pathToDirectoryIndex(url) {
  const parsed = new URL(`https://local${url}`)
  var absolute = pathToAbsolute(decodeURIComponent(parsed.pathname))
  // return index.json for directories or return a file out of baseq3
  var filename
  if(absolute > 0
    && ufs.existsSync(absolute)
    && ufs.statSync(absolute).isDirectory()) {
    filename = path.join(parsed.pathname, 'index.json')
    absolute = path.join(absolute, 'index.json')
  } else if (absolute > 0
    && ufs.existsSync(path.dirname(absolute))
    && ufs.statSync(path.dirname(absolute)).isDirectory()
    && parsed.pathname.match(/\/index\.json$/ig)) {
    filename = decodeURIComponent(parsed.pathname)
  }
  return {filename, absolute}
}

async function serveUnionFs(req, res, next) {
  var {filename, absolute} = pathToDirectoryIndex(req.url)
  if(filename) {
    await makeIndexJson(filename, absolute)
  }
  if ((absolute + 'dir').includes('.pk3dir')
    && ufs.existsSync(absolute + 'dir')) {
    await repackPk3Dir(absolute + 'dir')
  }
  if (absolute && ufs.existsSync(absolute)) {
    sendCompressed(absolute, res, req.headers['accept-encoding'])
  } else if (req.url.includes('version.json')) {
    var mtime
    if(ufs.existsSync(path.join(__dirname, '../../../build/release-js-js/quake3e.js')))
      mtime = ufs.statSync(path.join(__dirname, '../../../build/release-js-js/quake3e.js')).mtime
    if(ufs.existsSync(path.join(__dirname, '../../../build/debug-js-js/quake3e.js')))
      mtime = ufs.statSync(path.join(__dirname, '../../../build/debug-js-js/quake3e.js')).mtime
    // only return version file if there is an index present, otherwise the client won't look on quake.games
    if(mtime && absolute !== -1) {
      var versionString = JSON.stringify([mtime, mtime])
      res.append('content-length', versionString.length)
      res.send(versionString)
    } else {
      console.log(`Couldn't find version (no indexes, no host).`)
  		next()
    }
  } else if (absolute === -1) {
    console.log(`No hosting, no repacked files found.`)
		next()
  } else {
    console.log(`Couldn't find file "${req.url}" "${absolute}".`)
		next()
	}
}

if(ufs.existsSync(path.join(__dirname, '../../../build/release-js-js/quake3e.js')))
  app.use(serveStatic(path.join(__dirname, '../../../build/release-js-js'), {
    setHeaders: (res, path) => {
      res.setHeader('Access-Control-Allow-Origin', '*')
    }
  }))
if(ufs.existsSync(path.join(__dirname, '../../../build/debug-js-js/quake3e.js')))
  app.use(serveStatic(path.join(__dirname, '../../../build/debug-js-js'), {
    setHeaders: (res, path) => {
      res.setHeader('Access-Control-Allow-Origin', '*')
    }
  }))
app.use(serveStatic(path.join(__dirname), {
  setHeaders: (res, path) => {
    res.setHeader('Access-Control-Allow-Origin', '*')
  }
}))
app.use(serveUnionFs)

if(ports.includes(1080)) {
  // redirect http attempts to loading page
  const server = createServer(function(socket) {
  	try {
  		socks._onConnection(socket)
  	} catch (e) {
  		console.log(e)
  	}
  })
  server.listen(1080, () => console.log(`Server running at http://0.0.0.0:1080`))
}

ports.forEach((p, i, ports) => {
  if(ports[i] === 1080) return
  var httpServer = http.createServer(app)
  var wss = new WebSocketServer({server: httpServer})
  wss.on('connection', function(ws) {
    try {
      socks._onConnection(ws)
    } catch (e) {
      console.log(e)
    }
  })
  httpServer.listen(ports[i], () => console.log(`Http running at http://0.0.0.0:${ports[i]}`))
})

if(masterServer) {
  var startMasterServer = require('./master.js')
  startMasterServer(27950)
}
