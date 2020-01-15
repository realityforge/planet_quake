var crc32 = require('buffer-crc32')
var zlib = require('zlib')
var brot = require('brotli')

function compressFile(file, cb) {
  var crc = crc32.unsigned('')
  var compressed = 0
  var size = 0

  // stream each file in, generating a hash for it's original
  // contents, and gzip'ing the buffer to determine the compressed
  // length for the client so it can present accurate progress info
  var stream = fs.createReadStream(file)

  // gzip the file contents to determine the compressed length
  // of the file so the client can present correct progress info
  var gzip = zlib.createGzip()

  stream.on('error', function (err) {
    callback(err)
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
    cb(null, {
      name: file,
      compressed: compressed,
      checksum: crc
    })
  })
}

function serveCompressed(req, res, next) {
  next()
}

module.exports = {
  compressFile,
  serveCompressed
}
