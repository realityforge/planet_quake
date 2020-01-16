var fs = require('fs')
var path = require('path')
var {URL} = require('url')
var {Volume} = require('memfs')
var {ufs} = require('unionfs')
var {serveCompressed, compressFile} = require('./compress.js')

// TODO: Add some command line options here
// --recursive -R, adds all directory files below current directory
// --pk3dir -pk, create virtual pk3dir out of pk3 and exclude pk3 files
// --write -w, write all JSON files in every directory for CDN use
// --repack -rp, repack on the fly as pk3/media/images/sound files are accessed

// check the process args for a directory to serve as the baseq3 folders
var vol = Volume.fromJSON({})
ufs.use(fs).use(vol)
var mountPoint
var mountPoints = []
Array.from(process.argv).forEach((a) => {
  if(fs.existsSync(a)) {
		if(a.match(/\/node$/ig)) return true
		if(a.match(/\/web\.js$/ig)) return true
		console.log('linking ' + a)
		mountPoints.push(['/base', a])
    // create a link for user specified directory that doesn't exist
    if(mountPoint) {
			mountPoints.push([mountPoint, a])
    } else {
			mountPoints.push(['/assets', a])
    }
  // use an absolute path as a mount point if it doesn't exist
  } else if (a.match(/^\//i)) {
		console.log('mounting ' + a)
    mountPoint = a
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

function pathToAbsolute(path) {
	for(var i = 0; i < mountPoints.length; i++) {
		var fullpath = path.replace(mountPoints[i][0], mountPoints[i][1])
		if(fullpath.localeCompare(path) !== 0) {
			return fullpath
		}
	}
}

function readMultiDir(fullpath) {
	var dir = []
	for(var i = 0; i < mountPoints.length; i++) {
		const realpath = fullpath.replace(mountPoints[i][0], mountPoints[i][1])
		if(ufs.existsSync(realpath)) {
			var files = ufs.readdirSync(realpath).map(f => path.join(realpath, f))
			dir.push.apply(dir, files)
		}
	}
	return dir
}

async function serveIndexJson(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
	const absolute = pathToAbsolute(parsed.pathname)
  // return index.json for directories or return a file out of baseq3
	var filename
  if(absolute
	  && ufs.existsSync(absolute)
	  && ufs.statSync(absolute).isDirectory()) {
		filename = path.join(parsed.pathname, 'index.json')
	} else if (absolute
	  && ufs.existsSync(path.dirname(absolute))
		&& ufs.statSync(path.dirname(absolute)).isDirectory()
		&& parsed.pathname.match(/\/index\.json$/ig)) {
		filename = parsed.pathname
	} else {
		return next()
	}
	
  // if there is no index.json, generate one
  if(!ufs.existsSync(filename)) {
		var files = readMultiDir(path.dirname(filename))
		var manifest = {}
		for(var i = 0; i < files.length; i++) {
			var fullpath = files[i]
			if(!ufs.existsSync(fullpath)) return true
			var file = {}
			if(ufs.statSync(fullpath).isFile()) {
				file = await new Promise((resolve, reject) => {
					compressFile(ufs.createReadStream(fullpath), resolve, reject)
				})
			}
			manifest[fullpath] = Object.assign({name: path.basename(fullpath)}, file)
		}
		vol.mkdirpSync(path.dirname(filename))
    vol.writeFileSync(filename, JSON.stringify(manifest, null, 2))    
  }
  sendFile(filename, res)
}

function serveBaseQ3(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
	const absolute = pathToAbsolute(parsed.pathname)
  if (absolute) {
    sendFile(absolute, res)
  } else {
		next()
	}
}

module.exports = {
	serveBaseQ3,
	serveIndexJson,
	sendFile
}
