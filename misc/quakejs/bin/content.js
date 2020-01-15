
// TODO: move this to content.js, add convert.js for pk3, and compress.js for compression
var fs = require('fs')
var path = require('path')
var {Volume} = require('memfs')
var {ufs} = require('unionfs')
var {link} = require('linkfs')
var {URL} = require('url')
var {serveCompressed, compressFile} = require('./compress.js')

// check the process args for a directory to serve as the baseq3 folders
var vol = Volume.fromJSON({'/': {}}, '/app')
ufs.use(vol)
try {
	vol.mkdirSync('/base')
	vol.mkdirSync('/assets')
} catch (e) {
	if(e.code !== 'EXIST') throw e
}
var mountPoint
Array.from(process.argv).forEach((a) => {
  if(fs.existsSync(a)) {
		if(a.match(/\/node$/ig)) return true
		if(a.match(/\/web\.js$/ig)) return true
		console.log('linking ' + a)
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
		console.log('mounting ' + a)
    mountPoint = a
		try {
			vol.mkdirSync(mountPoint)
		} catch (e) {
			if(e.code !== 'EXIST') throw e
		}
  }
})

function sendFile(file, res) {
  // return file from baseq3 or index.json
  var readStream = ufs.createReadStream(file)
  readStream.on('open', function () {
    readStream.pipe(res)
  })
  readStream.on('error', function(err) {
    res.end(err)
  })
}

async function serveIndexJson(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
  // return index.json for directories or return a file out of baseq3
  if(ufs.existsSync(parsed.pathname)
	  && ufs.statSync(parsed.pathname).isDirectory
    || parsed.pathname.match(/\/index\.json$/ig)) {
			
    var filename = path.join(path.dirname(parsed.pathname), 'index.json')
    // if there is no index.json, generate one
    if(!ufs.existsSync(filename)) {
			var files = ufs.readdirSync(path.dirname(filename))
			var manifest = {}
			await files.forEach(async function(f) {
				var file = await new Promise(resolve => resolve({
					name: f
				})) //compressFile(f, resolve))
				manifest[f] = file
			})
      vol.writeFileSync(filename, JSON.stringify(manifest, null, 2))    
    }
    sendFile(filename, res)
  }
  
  next()
}

function serveBaseQ3(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
  if (ufs.existsSync(parsed.pathname)) {
    sendFile(parsed.pathname, res)
  }

  next()
}

module.exports = {
	serveBaseQ3,
	serveIndexJson,
	sendFile
}
