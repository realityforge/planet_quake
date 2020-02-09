var fs = require('fs')
var path = require('path')
var {URL} = require('url')
var {Volume} = require('memfs')
var {ufs} = require('unionfs')
var { Readable } = require('stream')
var {compressFile, compressDirectory} = require('./compress.js')

var recursive = false
var writeOut = false
var repackFiles = false
var pk3dir = false
var runContentGeneration = false
var includeHidden = false
var watchChanges = false
// TODO: Add some command line options here
// --recursive -R, adds all directory files below current directory
// --pk3dir -pk, create virtual pk3dir out of pk3 and exclude pk3 files
//   opposite of repack
// --write -wr, write all JSON files in every directory for CDN use
// --repack -rp, repack on the fly as pk3/media/images/sound files are accessed
//   opposit of pk3dir
// --hidden -h, include hidden files (uncommon)
// --watch, watch files for changes

// check the process args for a directory to serve as the baseq3 folders
var vol = Volume.fromJSON({})
ufs.use(fs).use(vol)
var mountPoint = '/assets/baseq3'
var mountPoints = []
for(var i = 0; i < process.argv.length; i++) {
  var a = process.argv[i]
  if(fs.existsSync(a)) {
		if(a.match(/\/node$/ig)) continue
		if(a.match(/\/web\.js$/ig)) continue
    // if running content script directly, automatically call each mount point 
    //   so the json files and zipped files can be generated
    if(a.match(/\/content\.js$/ig)) {
      runContentGeneration = true
      continue
    }
		console.log(`Linking ${mountPoint} -> ${a}`)
    // create a link for user specified directory that doesn't exist
    mountPoints.push([mountPoint, a])
  // use an absolute path as a mount point if it doesn't exist
  } else if(a == '--recursive' || a == '-R') {
    console.log('Recursive')
    recursive = true
  } else if(a == '--hidden' || a == '-h') {
    console.log('Hidden files')
    includeHidden = true
  } else if(a == '--pk3dir' || a == '-pk') {
    console.log('Virtual pk3dirs')
    pk3dir = true
  } else if(a == '--write' || a == '-wr') {
    console.log('Writing manifest.json, not watching')
    writeOut = true
    watchChanges = false
  } else if(a == '--watch') {
    console.log('Watching for changes, not writing')
    watchChanges = true
    writeOut = false
  } else if(a == '--repack' || a == '-rp') {
    console.log('Live repacking')
    repackFiles = true
  } else if (a.match(/^\//i)) {
		console.log('Using mount point ' + a)
    mountPoint = a
  } else {
    console.log(`ERROR: Unrecognized option "${a}"`)
  }
}
if(mountPoints.length === 0) {
  console.log('ERROR: No mount points, e.g. run `npm run start /Applications/ioquake3`')
}
mountPoints.sort((a, b) => a[0].localeCompare(b[0], 'en', { sensitivity: 'base' }))

function watchForChanges() {
  var chokidar = require('chokidar');
  var watcher = chokidar.watch(mountPoints.map(m => m[1] + '/**'), {
    interval: 1000,
    atomic: 1000,
    awaitWriteFinish: true
  })
  var doing = false
  watcher.on('change', function(changePath) {
    if(doing) return
    doing = true
    // remove all cache files from the directory tree
    var keys = Object.keys(vol.toJSON())
    for(var i = 0; i < mountPoints.length; i++) {
      if(changePath.includes(mountPoints[i][1])) {
        // remove all files in the affected mount point
        console.log(`Changes detected in ${mountPoints[i][1]}, unlinking...`)
        for(var j = 0; j < keys.length; j++) {
          if(keys[j].includes(mountPoints[i][1])) {
            try {
              // remove memfs cache of files
              vol.unlinkSync(keys[j])
            } catch (e) {
              // already removed?
              if(!e.code == 'ENOENT') throw e
            }
          }
        }
      }
    }
    doing = false
  })
}
if(watchChanges) {
  watchForChanges()
}

function pathToAbsolute(virtualPath) {
  var result
	for(var i = 0; i < mountPoints.length; i++) {
		if(virtualPath.includes(mountPoints[i][0])) {
      result = path.join(mountPoints[i][1],
        virtualPath.replace(mountPoints[i][0], ''))
      if(ufs.existsSync(result)) {
        return result
      }
		}
	}
  return result
}

function readMultiDir(fullpath, forceRecursive) {
	var dir = []
  // skip pk3dirs in repack mode because they will be zipped by indexer
  if(repackFiles && !forceRecursive
    && fullpath.includes('.pk3dir')
    && ufs.statSync(fullpath).isDirectory()) {
    return dir
  }
  if(ufs.existsSync(fullpath)) {
    var files = ufs.readdirSync(fullpath)
      .map(f => path.join(fullpath, f))
      .filter(f => includeHidden || path.basename(f)[0] != '.')
    dir.push.apply(dir, files)
    if(recursive || forceRecursive) {
      for(var j = 0; j < files.length; j++) {
        if(ufs.statSync(files[j]).isDirectory()) {
          var moreFiles = readMultiDir(files[j], forceRecursive)
          dir.push.apply(dir, moreFiles)
        }
      }
    }
  } else {
    throw new Error(`Cannot find directory ${fullpath}`)
  }
	return dir
}

async function repackPk3Dir(fullpath) {
  if(!repackFiles) {
    return
  }
  if(!ufs.statSync(fullpath).isDirectory()) {
    throw new Error(`Provided path ${fullpath} is not a directory.`)
  }
  var newPk3 = fullpath.replace('.pk3dir', '.pk3')
  vol.mkdirpSync(path.dirname(fullpath))
  if(!ufs.existsSync(fullpath.replace('.pk3dir', '.pk3'))
    || writeOut) {
    console.log(`archiving ${newPk3}`)
    await compressDirectory(
      readMultiDir(fullpath, true),
      vol.createWriteStream(newPk3),
      fullpath
    )
  }
  return await compressFile(newPk3, vol)
}

async function cacheFile(fullpath) {
  vol.mkdirpSync(path.dirname(fullpath))
  return await compressFile(fullpath, vol)
}

async function makeIndexJson(filename, absolute) {
  // if there is no index.json, generate one
  if(filename && !ufs.existsSync(absolute)) {
		var files = readMultiDir(path.dirname(absolute))
		var manifest = {}
		for(var i = 0; i < files.length; i++) {
			var fullpath = files[i]
			if(!ufs.existsSync(fullpath)) continue
			var file = {}
			if(ufs.statSync(fullpath).isFile()) {
				file = await cacheFile(fullpath)
			} else if(repackFiles
        && fullpath.includes('.pk3dir')
        && ufs.statSync(fullpath).isDirectory()) {
        // only make the pk3 if we are intentionally writing or it doesn't already exist
        file = await repackPk3Dir(fullpath)
        fullpath = fullpath.replace('.pk3dir', '.pk3')
      }

			manifest[fullpath] = Object.assign(file, {
        name: fullpath.replace(path.dirname(absolute), '')
      })
		}
    console.log(`writing directory index ${absolute}`)
		vol.mkdirpSync(path.dirname(absolute))
    vol.writeFileSync(absolute, JSON.stringify(manifest, null, 2))    
  }
}

module.exports = {
	makeIndexJson,
	pathToAbsolute,
  repackPk3Dir,
}
