var fs = require('fs')
var path = require('path')
var {mkdirpSync} = require('./compress.js')

function chext(file, ext) {
  return file.replace(new RegExp('\\' + path.extname(file) + '$'), ext)
}

function chroot(file, root, output) {
  // TODO: example of defensive programming
  if(file.substr(0, root.length).localeCompare(root) !== 0)
    throw new Error(`File not in root ${file}`)
  return path.join(output, file.substr(root.length))
}

async function convertNonAlpha(inFile, project) {
  var outFile
  var alphaCmd = await execCmd(`identify -format '%[opaque]' '${inFile}'`, {quiet: true})
  // if it is alpha
  if(alphaCmd[0].localeCompare('False') === 0) {
    // convert everything else to png to support transparency
    outFile = chroot(chext(inFile, '.png'), root, output)
  } else {
    // if a jpg already exists, use that file for convert
    if(fs.existsSync(chext(inFile, '.jpg'))) {
        inFile = chext(inFile, '.jpg')
    }
    // transfer low quality jpeg instead
    outFile = chroot(chext(inFile, '.jpg'), root, output)
  }
  mkdirpSync(path.dirname(outFile))
  // convert, baseq3 already includes jpg
  await execCmd(`convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 50% "${inFile}" "${outFile}"`, {quiet: true})
}
