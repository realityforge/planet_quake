var fs = require('fs')
var path = require('path')
var {mkdirpSync} = require('./compress.js')
var {execSync} = require('child_process');

function chext(file, ext) {
  return file.replace(new RegExp('\\' + path.extname(file) + '$'), ext)
}

function chroot(file, root, output) {
  // TODO: example of defensive programming
  if(file.substr(0, root.length).localeCompare(root) !== 0)
    throw new Error(`File not in root ${file}`)
  return path.join(output, file.substr(root.length))
}

async function convertNonAlpha(inFile, project, output) {
  var outFile
  var alphaCmd;
  try {
    alphaCmd = execSync(`identify -format '%[opaque]' '${inFile}'`)
  } catch (e) {
    console.log(e)
  }
  // if it is alpha
  if(alphaCmd.localeCompare('False') === 0) {
    // convert everything else to png to support transparency
    outFile = chroot(chext(inFile, '.png'), project, output)
  } else {
    // if a jpg already exists, use that file for convert
    if(fs.existsSync(chext(inFile, '.jpg'))) {
        inFile = chext(inFile, '.jpg')
    }
    // transfer low quality jpeg instead
    outFile = chroot(chext(inFile, '.jpg'), project, output)
  }
  mkdirpSync(path.dirname(outFile))
  // convert, baseq3 already includes jpg
  execSync(`convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 50% "${inFile}" "${outFile}"`)
}

async function convertAudio(inFile, project, output) {
  var outFile, inFile = files[i]
  outFile = chroot(chext(inFile, '.opus'), project, output)
  mkdirpSync(path.dirname(outFile))
  execSync(`opusenc --quiet --bitrate 24 --vbr "${inFile}" "${outFile}"`)
}

async function convertGameFiles(project, outConverted, progress) {
  await progress([[1, 0, 3, `Converting images`]])
  var orderedEverything = Object.values(gs.ordered)
    .filter((f, i, arr) => arr.indexOf(f) === i)
  var images = findTypes(imageTypes, orderedEverything)
  for(var j = 0; j < images.length; j++) {
    await progress([[2, j, images.length, `Converting image ${images[j]}`]])
    await convertNonAlpha(images[j], project, outConverted)
  }
  await progress([[1, 1, 3, `Converting audio`]])
  var audio = findTypes(audioTypes, orderedEverything)
  for(var j = 0; j < audio.length; j++) {
    await progress([[2, j, audio.length, `Converting audio ${audio[j]}`]])
    await convertAudio(audio[j], project, outConverted)
  }
  await progress([[1, 1, 3, `Copying known files`]])
  var known = orderedEverything
    .filter(f => !images.includes(f) && !audio.includes(f))
  for(var j = 0; j < known.length; j++) {
    await progress([[2, j, known.length, `Copying files ${known[j]}`]])
    ufs.copyFileSync(chroot(known[j], project, outConverted))
  }
  await progress([[2, false]])
}

module.exports = {
  convertNonAlpha,
  convertAudio,
  convertGameFiles
}
