var express = require('express')
var path = require('path')
var fs = require('fs')
var liveServer = require("live-server")
var {vol} = require('memfs')
var {ufs} = require('unionfs')
var {link} = require('linkfs')
var {URL} = require('url')

express.static.mime.types['wasm'] = 'application/wasm'

// check the process args for a directory to serve as the baseq3 folders
ufs.use(vol)
var mountPoint
Array.from(process.argv).forEach((a) => {
  if(fs.existsSync(a)) {
    const lfs = link(fs, ['/base', a])
    ufs.use(lfs)
    // create a link for user specified directory that doesn't exist
    if(mountPoint) {
      const lfs = link(fs, [mountPoint, a])
      ufs.use(lfs)
    } else {
      // create a link for assets directory just incase we can't reorganize
      const lfs = link(fs, ['/assets', a])
      ufs.use(lfs)
    }
  // use an absolute path as a mount point if it doesn't exist
  } else if (a.match(/^\//i)) {
    mountPoint = a
  }
})

function serveBaseQ3(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
  const dir = path.dirname(parsed.pathname)
  // TODO: copy necessary code from content.js
  var file
  // return index.json for directories or return a file out of baseq3
  if(ufs.statSync(parsed.pathname).isDirectory
    || parsed.pathname.match(/\/index\.json/ig)) {
    file = path.join(path.dirname(parsed.pathname), 'index.json')
    // if there is no index.json, generate one
    if(!ufs.existsSync(file)) {
      vol.writeFileSync('/base/index.json', JSON.stringify(, null, 2))    
    }
  } else if (ufs.existsSync(parsed.pathname)) {
    file = parsed.pathname
  }
  
  if(file)
    // return file from baseq3 or index.json
    var readStream = ufs.createReadStream(file)
    readStream.on('open', function () {
      readStream.pipe(res)
    })
    readStream.on('error', function(err) {
      res.end(err)
    })
  }
  
  next()
}

var params = {
    port: 8080, // Set the server port. Defaults to 8080.
    host: "0.0.0.0", // Set the address to bind to. Defaults to 0.0.0.0 or process.env.IP.
    root: "./", // Set root directory that's being served. Defaults to cwd.
    open: false, // When false, it won't load your browser by default.
    ignore: 'scss,my/templates', // comma-separated string for paths to ignore
    file: "./misc/quakejs/bin/index.html", // When set, serve this file (server root relative) for every 404 (useful for single-page applications)
    wait: 1000, // Waits for all changes, before reloading. Defaults to 0 sec.
//    mount: [['/components', './node_modules']], // Mount a directory to a route.
    logLevel: 2, // 0 = errors only, 1 = some, 2 = lots
    middleware: [
			express.static(path.join(__dirname, './index.html'), { extensions: ['html'] }),
			express.static(path.join(__dirname, '../../../build/release-js-js'), { extensions: ['wasm'] }),
      serveBaseQ3
		] // Takes an array of Connect-compatible middleware that are injected into the server middleware stack
}

liveServer.start(params)
