var crc32 = require('buffer-crc32')
var zlib = require('zlib')
var {ufs} = require('unionfs')
var archiver = require('archiver')

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

async function compressDirectory(fullpath, outputStream, absolute) {
  var archive = archiver('zip', {
    zlib: { level: 9 } // Sets the compression level.
  })
  archive.pipe(outputStream);
  if(!Array.isArray(fullpath)) fullpath = [fullpath];
  for(var i = 0; i < fullpath.length; i++) {
    if(ufs.statSync(fullpath[i]).isDirectory()) continue
    archive.append(ufs.createReadStream(fullpath[i]), {
      name: fullpath[i].replace(absolute, '')
    });
  }
  await archive.finalize();
}

// stream each file in, generating a hash for it's original
// contents, and gzip'ing the buffer to determine the compressed
// length for the client so it can present accurate progress info
async function compressFile(stream, brWrite, gzWrite, dfWrite) {
  return new Promise((resolve, reject) => {
    var crc = crc32.unsigned('')
    var compressed = 0
    var brCompressed = 0
    var dfCompressed = 0
    var size = 0

    brZip = zlib.createBrotliCompress()
    gZip = zlib.createGzip()
    dfZip = zlib.createDeflate()

    stream.on('error', function (err) {
      reject(err)
    })
    stream.on('data', function (data) {
      crc = crc32.unsigned(data, crc)
      size += data.length
    })
    brZip.on('data', function (data) {
      brCompressed += data.length
    })
    gZip.on('data', function (data) {
      compressed += data.length
    })
    dfZip.on('data', function (data) {
      dfCompressed += data.length
    })
    stream.on('end', function () {
      resolve({
        compressed: compressed,
        brCompressed: brCompressed,
        dfCompressed: dfCompressed,
        checksum: crc,
        size: size
      })
    })
    
    stream.pipe(brZip).pipe(brWrite)
    stream.pipe(gZip).pipe(gzWrite)
    stream.pipe(dfZip).pipe(dfWrite)
  })
}

function sendCompressed(file, res, acceptEncoding) {
  var readStream = ufs.createReadStream(file)
  var compressionExists = false
  // if compressed version already exists, send it directly
  if(acceptEncoding.includes('br')) {
    res.append('content-encoding', 'br')
    if(ufs.existsSync(file + '.br')) {
      res.append('content-length', ufs.statSync(file + '.br').size);
      readStream = ufs.createReadStream(file + '.br')
    } else {
      readStream = readStream.pipe(zlib.createBrotliCompress())
    }
  } else if(acceptEncoding.includes('gzip')) {
    res.append('content-encoding', 'gzip')
    if(ufs.existsSync(file + '.gz')) {
      res.append('content-length', ufs.statSync(file + '.gz').size);
      readStream = ufs.createReadStream(file + '.gz')
    } else {
      readStream = readStream.pipe(zlib.createGzip())
    }
  } else if(acceptEncoding.includes('deflate')) {
    res.append('content-encoding', 'deflate')
    if(ufs.existsSync(file + '.df')) {
      res.append('content-length', ufs.statSync(file + '.df').size);
      readStream = ufs.createReadStream(file + '.df')
    } else {
      readStream = readStream.pipe(zlib.createDeflate())
    }
  } else {
    res.append('content-length', ufs.statSync(file).size);
  }
  
  res.append('vary', 'accept-encoding')
  readStream.pipe(res)
}

module.exports = {
  compressFile,
  sendCompressed,
  compressDirectory
}
