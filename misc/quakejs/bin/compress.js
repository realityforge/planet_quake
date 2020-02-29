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
  res.set('cache-control', 'public, max-age=31557600');
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
  
  readStream.pipe(res)
}

module.exports = {
  compressFile,
  sendCompressed,
  compressDirectory
}
