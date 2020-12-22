var path = require('path')
var crc32 = require('buffer-crc32')
var zlib = require('zlib')
var {ufs} = require('unionfs')
var archiver = require('archiver')
var glob = require('glob')
var mime = require('mime')
var StreamZip = require('node-stream-zip')

/*
// because cloud storage doesn't necissarily support it?
function createVirtualGZip(mountPoint) {
  // create a virtual gzip file to test for gzip encoding support
  const readable = Readable.from(["window.gzipSupported = true"])
  compressFile(
    ufs.createReadStream(mountPoint),
    vol.createWriteStream(fullpath + '.gz'),
    resolve, reject)
}
*/

function mkdirpSync(p) {
  try {
    ufs.mkdirSync(p)
  } catch (e) {
    if(e.code == 'EEXIST') {
      return
    }
    if(e.code == 'ENOENT') {
      var parent = path.dirname(p)
      if(parent == p) throw e
      mkdirpSync(parent)
      ufs.mkdirSync(p)
    } else {
      throw e
    }
  }
}

async function readPak(zipFile, progress, outDirectory, noOverwrite) {
  var skipped = 0
  const zip = new StreamZip({
      file: zipFile,
      storeEntries: true
  })
  var index = await new Promise(resolve => {
    zip.on('ready', () => {
      console.log('Entries read: ' + zip.entriesCount + ' ' + path.basename(zipFile))
      resolve(Object.values(zip.entries()))
    })
    zip.on('error', (err) => {
      console.warn(err, zipFile)
      resolve([])
    })
  })
  
  if(!outDirectory) {
    return index
  }

  for(var i = 0; i < index.length; i++) {
    var entry = index[i]
    if(entry.isDirectory || entry.size === 0) continue
    if(!entry.name) debugger
    var levelPath = path.join(outDirectory, entry.name)
    mkdirpSync(path.dirname(levelPath))
    if(progress)
      await progress([[2, i, index.length, entry.name]])
    if(noOverwrite && ufs.existsSync(levelPath)) {
      // make an exception if the file was already written once,
      //   write it again so subsequent paks can update files
      if(!Array.isArray(noOverwrite) || !noOverwrite.includes(levelPath)) {
        skipped++
        continue
      }
    } else if (Array.isArray(noOverwrite)) {
      noOverwrite.push(levelPath)
    }
    await new Promise(resolve => {
      zip.extract(entry.name, levelPath, err => {
        if(err) console.log('Extract error ' + err)
        resolve()
      })
    })
  }

  return skipped
}

async function unpackPk3s(project, outCombined, progress, noOverwrite, pk3dir) {
  // pk3s are list mini filesystems overlayed on top of each other.
  //   The first time repack runs, it must overlap files, otherwise
  //   you will end up with the first qvm in pak0.pk3 instead of the
  //   updated one because of no overlapping. So written files are stored
  //   here so that if it writes the file the first time, then it will make
  //   an exception and write the files again.
  var overwriteFirstTime = Array.isArray(noOverwrite) ? noOverwrite : []
  // TODO: copy non-pk3 files first, in case of unpure modes
  var notpk3s = glob.sync('**/*', {nodir: true, cwd: project, ignore: '*+(.pk3|.zip)', nocase: true})
  var skipped = 0
  for(var j = 0; j < notpk3s.length; j++) {
    await progress([[1, j, notpk3s.length, `${notpk3s[j]}`]])
    if(notpk3s[j].match(/\.pk3$/i)) continue
    var newFile = path.join(outCombined, notpk3s[j])
    mkdirpSync(path.dirname(newFile))
    if(!noOverwrite || !ufs.existsSync(newFile)
      || overwriteFirstTime.includes(newFile)) {
      overwriteFirstTime.push(newFile)
      ufs.copyFileSync(path.join(project, notpk3s[j]), newFile)
    } else {
      skipped++
    }
  }

  var zips = glob.sync('**/*.zip', {nodir: true, cwd: project, nocase: true})
  zips.sort((a, b) => a[0].localeCompare(b[0], 'en', { sensitivity: 'base' }))
  for(var j = 0; j < zips.length; j++) {
    await progress([[1, j, zips.length, `${zips[j]}`]])
    var pk3sinzip = (await readPak(path.join(project, zips[j])))
      .filter(f => f.name.match(/\.pk3$/i))
    if(pk3sinzip.length > 0) {
      skipped += await readPak(path.join(project, zips[j]), progress, outCombined,
        noOverwrite ? overwriteFirstTime : false)
    }
  }

  var pk3s = glob.sync('**/*.pk3', {nodir: true, cwd: project, nocase: true})
    .map(f => path.join(project, f))
    .concat(glob.sync('**/*.pk3', {nodir: true, cwd: outCombined, nocase: true})
      .map(f => path.join(outCombined, f)))
  pk3s.sort((a, b) => a[0].localeCompare(b[0], 'en', { sensitivity: 'base' }))
  for(var j = 0; j < pk3s.length; j++) {
    await progress([[1, j, pk3s.length, `${pk3s[j]}`]])
    var outDirectory = pk3dir 
      ? path.join(outCombined, path.basename(pk3s[j]) + 'dir') 
      : outCombined
    skipped += await readPak(pk3s[j], progress, outDirectory,
      noOverwrite ? overwriteFirstTime : false)
    await progress([[2, false]])
  }

  if(noOverwrite) {
    console.log(`Skipped ${skipped}`)
  }
}

async function compressDirectory(fullpath, outputStream, absolute) {
  var archive = archiver('zip', {
    zlib: { level: 9 } // Sets the compression level.
  })
  var dirs = []
  archive.pipe(outputStream)
  if(!Array.isArray(fullpath)) fullpath = [fullpath]
  for(var i = 0; i < fullpath.length; i++) {
    if(ufs.statSync(fullpath[i]).isDirectory()) {
      var newName = fullpath[i].replace(absolute, '') + '/'
      if(newName.length <= 1) continue
      archive.append(null, {name: newName})
      dirs.push(fullpath[i])
      continue
    }
    if(!dirs.includes(path.dirname(fullpath[i]))) {
      var newName = path.dirname(fullpath[i]).replace(absolute, '') + '/'
      if(newName.length > 1) {
        archive.append(null, {name: newName})
        dirs.push(path.dirname(fullpath[i]))
      }
    }
    archive.append(ufs.createReadStream(fullpath[i]), {
      name: fullpath[i].replace(absolute, '')
    })
  }
  await archive.finalize()
}

// stream each file in, generating a hash for it's original
// contents, and gzip'ing the buffer to determine the compressed
// length for the client so it can present accurate progress info
async function compressFile(fullpath, vol) {
  var crc = crc32.unsigned('')
  var stream = ufs.createReadStream(fullpath)
  stream.on('error', function (err) {
    throw err
  })
  stream.on('data', function (data) {
    crc = crc32.unsigned(data, crc)
  })
  await Promise.all([
    stream.pipe(zlib.createBrotliCompress())
      .pipe(vol.createWriteStream(fullpath + '.br')),
    stream.pipe(zlib.createGzip())
      .pipe(vol.createWriteStream(fullpath + '.gz')),
    stream.pipe(zlib.createDeflate())
      .pipe(vol.createWriteStream(fullpath + '.df'))
  ].map(stream => new Promise((resolve, reject) => {
    stream.on('finish', resolve).on('error', reject)
  })))
  return {
    compressed: vol.statSync(fullpath + '.gz').size,
    brCompressed: vol.statSync(fullpath + '.br').size,
    dfCompressed: vol.statSync(fullpath + '.df').size,
    checksum: crc,
    size: ufs.statSync(fullpath).size
  }
}

function sendCompressed(file, res, acceptEncoding) {
  var readStream = ufs.createReadStream(file)
  var compressionExists = false
  res.set('cache-control', 'public, max-age=31557600')
  res.set('content-type', mime.getType(file))
  // if compressed version already exists, send it directly
  if(acceptEncoding.includes('br')) {
    res.append('content-encoding', 'br')
    if(ufs.existsSync(file + '.br')) {
      res.append('content-length', ufs.statSync(file + '.br').size)
      readStream = ufs.createReadStream(file + '.br')
    } else {
      readStream = readStream.pipe(zlib.createBrotliCompress())
    }
  } else if(acceptEncoding.includes('gzip')) {
    res.append('content-encoding', 'gzip')
    if(ufs.existsSync(file + '.gz')) {
      res.append('content-length', ufs.statSync(file + '.gz').size)
      readStream = ufs.createReadStream(file + '.gz')
    } else {
      readStream = readStream.pipe(zlib.createGzip())
    }
  } else if(acceptEncoding.includes('deflate')) {
    res.append('content-encoding', 'deflate')
    if(ufs.existsSync(file + '.df')) {
      res.append('content-length', ufs.statSync(file + '.df').size)
      readStream = ufs.createReadStream(file + '.df')
    } else {
      readStream = readStream.pipe(zlib.createDeflate())
    }
  } else {
    res.append('content-length', ufs.statSync(file).size)
  }
  
  readStream.pipe(res)
}

module.exports = {
  compressFile,
  sendCompressed,
  compressDirectory,
  readPak,
  mkdirpSync,
  unpackPk3s,
}
