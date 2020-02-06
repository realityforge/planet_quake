var fs = require('fs')
var path = require('path')
var {URL} = require('url')
var {Volume} = require('memfs')
var {ufs} = require('unionfs')
var {serveCompressed, compressFile} = require('./compress.js')

var recursive = false
var writeOut = false
var repackFiles = false
var pk3dir = false
// TODO: Add some command line options here
// --recursive -R, adds all directory files below current directory
// --pk3dir -pk, create virtual pk3dir out of pk3 and exclude pk3 files
//   opposite of repack
// --write -w, write all JSON files in every directory for CDN use
// --repack -rp, repack on the fly as pk3/media/images/sound files are accessed
//   opposit of pk3dir

// check the process args for a directory to serve as the baseq3 folders
var vol = Volume.fromJSON({})
ufs.use(fs).use(vol)
var mountPoint = '/assets'
var mountPoints = []
for(var i = 0; i < process.argv.length; i++) {
  var a = process.argv[i]
  if(fs.existsSync(a)) {
		if(a.match(/\/node$/ig)) continue
		if(a.match(/\/web\.js$/ig)) continue
		console.log(`linking ${mountPoint} to ${a}`)
    // create a link for user specified directory that doesn't exist
    mountPoints.push([mountPoint, a])
  // use an absolute path as a mount point if it doesn't exist
  } else if(a == '--recursive' || a == '-R') {
    console.log('Recursive')
    recursive = true
  } else if(a == '--pk3dir' || a == '-pk') {
    console.log('Virtual pk3dirs')
    pk3dir = true
  } else if(a == '--write' || a == '-w') {
    console.log('Writing manifest.json')
    writeOut = true
  } else if(a == '--repack' || a == '-rp') {
    console.log('Live repacking')
    repackFiles = true
  } else if (a.match(/^\//i)) {
		console.log('mounting ' + a)
    mountPoint = a
  } else {
    console.log(`ERROR: Unrecognized option "${a}"`)
  }
}
if(mountPoints.length === 0) {
  console.log('ERROR: No mount points, e.g. run `npm run start /Applications/ioquake3`')
}
mountPoints.sort((a, b) => a[0].localeCompare(b[0], 'en', { sensitivity: 'base' }))

function sendFile(file, res) {
  // return file from baseq3 or index.json
  var readStream = ufs.createReadStream(file)
// TODO: res.append('Content-Length', file);
  readStream.on('open', function () {
    readStream.pipe(res)
  })
  readStream.on('error', function(err) {
    res.end(err)
  })
}

function pathToAbsolute(virtualPath) {
  var result
	for(var i = 0; i < mountPoints.length; i++) {
		if(virtualPath.includes(mountPoints[i][0])) {
			result = path.join(mountPoints[i][1], virtualPath.replace(mountPoints[i][0], ''))
		}
	}
  return result
}

function readMultiDir(fullpath) {
	var dir = []
	for(var i = 0; i < mountPoints.length; i++) {
		const realpath = fullpath.replace(mountPoints[i][0], mountPoints[i][1])
		if(ufs.existsSync(realpath)) {
			var files = ufs.readdirSync(realpath).map(f => path.join(realpath, f))
			dir.push.apply(dir, files)
      if(recursive) {
        for(var j = 0; j < files.length; j++) {
          if(ufs.statSync(files[j]).isDirectory()) {
            var moreFiles = readMultiDir(files[j])
            dir.push.apply(dir, moreFiles)
          }
        }
      }
		}
	}
	return dir
}

async function serveIndexJson(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
	var absolute = pathToAbsolute(parsed.pathname)
  // return index.json for directories or return a file out of baseq3
	var filename
  if(absolute
	  && ufs.existsSync(absolute)
	  && ufs.statSync(absolute).isDirectory()) {
		filename = path.join(parsed.pathname, 'index.json')
    absolute = pathToAbsolute(filename)
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
      
			manifest[fullpath] = Object.assign({
        name: fullpath.replace(path.dirname(absolute), '')
      }, file)
		}
		vol.mkdirpSync(path.dirname(filename))
    vol.writeFileSync(filename, JSON.stringify(manifest, null, 2))    
  }
  sendFile(filename, res)
}

function serveBaseQ3(req, res, next) {
  const parsed = new URL(`https://local${req.url}`)
	const absolute = pathToAbsolute(parsed.pathname)
  if (absolute && fs.existsSync(absolute)) {
    sendFile(absolute, res)
  } else {
    console.log(`Couldn't find file "${parsed.pathname}" "${absolute}".`)
		next()
	}
}

module.exports = {
	serveBaseQ3,
	serveIndexJson,
	sendFile
}
