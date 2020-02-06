var crc32 = require('buffer-crc32')
var zlib = require('zlib')
var {ufs} = require('unionfs')
var archiver = require('archiver')
//var brot = require('brotli')

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
async function compressFile(stream, writeStream) {
  return new Promise((resolve, reject) => {
    var crc = crc32.unsigned('')
    var compressed = 0
    var size = 0

    // gzip the file contents to determine the compressed length
    // of the file so the client can present correct progress info
    var gzip = zlib.createGzip()

    stream.on('error', function (err) {
      reject(err)
    })
    stream.on('data', function (data) {
      crc = crc32.unsigned(data, crc)
      size += data.length
      gzip.write(data)
    })
    stream.on('end', function () {
      gzip.end()
    })

    gzip.on('data', function (data) {
      compressed += data.length
    })
    gzip.on('end', function () {
      resolve({
        compressed: compressed,
        checksum: crc,
        size: size
      })
    })
    
    stream.pipe(gzip).pipe(writeStream)
  })
}

function sendCompressed(file, res, compress) {
  // if compressed version already exists, send it directly
  if(ufs.existsSync(file + '.gz') && compress) {
    res.append('content-encoding', 'gzip')
    file += '.gz'
  }
  if(file.includes('.gz') || !compress) {
    var readStream = ufs.createReadStream(file)
    res.append('content-length', ufs.statSync(file).size);
    readStream.on('open', function () {
      readStream.pipe(res)
    })
    readStream.on('error', function(err) {
      res.end(err)
    })
    return
  }
  // return file from baseq3 or index.json
  var readStream = ufs.createReadStream(file)
  var gzip = zlib.createGzip()
  res.append('content-encoding', 'gzip')
  // TODO: res.append('Content-Length', file);
  readStream.on('open', function () {
    readStream.pipe(gzip).pipe(res)
  })
  readStream.on('error', function(err) {
    res.end(err)
  })
}

module.exports = {
  compressFile,
  sendCompressed,
  compressDirectory
}
