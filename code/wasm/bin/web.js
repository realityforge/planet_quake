const { execSync } = require('child_process');
const express = require('express');
const { sendFile } = require('express/lib/response');
const fs = require('fs');
const path = require('path')
const serveIndex = require('serve-index');

const app = express()

const ASSETS_DIRECTORY = __dirname + '/../../../games/multigame/assets/'
const BUILD_DIRECTORY = __dirname + '/../../../build/'

app.use('/', express.static(BUILD_DIRECTORY + 'release-js-js/'));
app.use('/', express.static(BUILD_DIRECTORY + 'debug-js-js/'));


// layer alternatives because WASM loads QVM
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'release-js-js/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'debug-js-js/multigame/vm/'));

app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'release-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'debug-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'release-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'debug-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'release-win-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(BUILD_DIRECTORY + 'debug-win-x86_64/multigame/vm/'));


app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'release-js-js/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'debug-js-js/multigame/vm/'));

app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'release-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'debug-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'release-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'debug-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'release-win-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(BUILD_DIRECTORY + 'debug-win-x86_64/multigame/vm/'));


app.use('/multigame/', serveIndex(__dirname + '/../../../games/multigame/assets/'));
app.use('/multigame/', express.static(__dirname + '/../../../games/multigame/assets/'));
app.use('/multigame/', function (request, response, next) {
  // what makes this clever is it only converts when requested
  let localName = request.originalUrl
  // TODO: convert paths like *.pk3dir and *.pk3 to their zip counterparts and stream
  if(localName[0] == '/')
    localName = localName.substring(1)
  if(localName.startsWith('multigame'))
    localName = localName.substring('multigame'.length)
  if(localName[0] == '/')
  localName = localName.substring(1)
  let ext = path.extname(localName)
  let strippedName = localName
  if(ext) {
    strippedName = strippedName.substring(0, localName.length - ext.length)
  }
  let realBasePath = path.resolve(path.join(__dirname, '/../../../games/multigame/assets/'))
  let parentDirectory = path.dirname(path.join(realBasePath, localName))
  if(fs.existsSync(parentDirectory)) {
    let otherFormatName
    if(fs.existsSync(path.join(realBasePath, strippedName + '.tga'))) {
      otherFormatName = strippedName + '.tga'
    } else if (fs.existsSync(path.join(realBasePath, strippedName + '.pcx'))) {
      otherFormatName = strippedName + '.pcx'
    }
    
    let hasAlpha = true
    if(otherFormatName) {
      let alphaCmd
      try {
        alphaCmd = execSync(`identify -format '%[opaque]' "${path.join(realBasePath, otherFormatName)}"`, {stdio : 'pipe'}).toString('utf-8')
      } catch (e) {
        console.error(e.message, (e.output || '').toString('utf-8').substr(0, 1000))
      }
      // if it is alpha
      if(/* true || TODO: allAlpha? */ alphaCmd.match(/true/ig)) {
        hasAlpha = false
      }
    } else {
      return next()
    }

    if((!hasAlpha && localName.includes('.jpeg'))
      || (hasAlpha && localName.includes('.png'))) {
      // try to convert the file and return it
      try {
        // TODO: allAlpha? execSync(`convert "${inFile}" -alpha set -background none -channel A -evaluate multiply 0.5 +channel -auto-orient "${outFile}"`, {stdio : 'pipe'})
        execSync(`convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 20% -auto-orient "${path.join(realBasePath, otherFormatName)}" "${path.join(realBasePath, localName)}"`, {stdio : 'pipe'})
      } catch (e) {
        console.error(e.message, (e.output || '').toString('utf-8').substr(0, 1000))
      }
    }
  }
  if(fs.existsSync(path.join(realBasePath, localName))) {
    return response.sendFile(path.join(realBasePath, localName))
  }
  return next()
});


function sendIndex(request, response){
  response.sendFile(path.resolve(__dirname + '/../http/index.html'));
}

app.use('/', express.static(__dirname + '/../http/'));
// show index file for all request paths, in case UI is loading fancy breadcrumb menu
app.get('/MAINMENU', sendIndex);
app.get('/SETUP', sendIndex);
app.get('/MULTIPLAYER', sendIndex);
app.get('/CHOOSELEVEL', sendIndex);
app.get('/ARENASERVERS', sendIndex);
app.get('/DIFFICULTY', sendIndex);
app.get('/PLAYERMODEL', sendIndex);
app.get('/PLAYERSETTINGS', sendIndex);


var fileTimeout
var latestMtime = new Date()

function writeVersionFile(time) {
  console.log('Updating working directory...')
  try {
    if(!time) time = new Date()
    // refresh any connected clients
    require('fs').writeFileSync(
      path.join(ASSETS_DIRECTORY, 'version.json'), 
      JSON.stringify([time, time]))
    //fs.watchFile(file, function(curr, prev) {
    //});
  } catch(e) {
    console.log(e)
  }
}

// okay this function is apparently a little idiotic and triggers on accesses
//   this is not how native notify works, but maybes it's the best they could make the same
function startFileWatcher(prefix, eventType, filename) {
  if(filename.includes('version.json')) {
    return // this would be redundant
  }
  if(fileTimeout) {
    clearTimeout(fileTimeout)
    fileTimeout = null
  }
  if(!fs.existsSync(path.join(ASSETS_DIRECTORY, prefix, filename))) {
    // must have been deleted
    latestMtime = new Date()
  } else {
    let newMtime = fs.statSync(path.join(ASSETS_DIRECTORY, prefix, filename)).mtime
    if(newMtime > latestMtime) {
      latestMtime = newMtime
    }
  }
  // debounce file changes for a second in case there is a copy process going on
  fileTimeout = setTimeout(function () {
    writeVersionFile(latestMtime)
  }, 1000)
}

if(fs.existsSync(ASSETS_DIRECTORY)) {
  fs.watch(ASSETS_DIRECTORY, startFileWatcher.bind(null, ''))
  let buildDirs = fs.readdirSync(BUILD_DIRECTORY)
    .filter(node => node[0] != '.' && fs.statSync(path.join(BUILD_DIRECTORY, node)).isDirectory())
  for(let i = 0; i < buildDirs.length; i++) {
    fs.watch(path.join(BUILD_DIRECTORY, buildDirs[i]), function () {
      writeVersionFile()
    })
  }
  // watch at least the top level directories for convenience
  let directories = fs.readdirSync(ASSETS_DIRECTORY)
    .filter(node => node[0] != '.' && fs.statSync(path.join(ASSETS_DIRECTORY, node)).isDirectory())
  for(let i = 0; i < directories.length; i++) {
    fs.watch(path.join(ASSETS_DIRECTORY, directories[i]), 
      startFileWatcher.bind(null, directories[i]))
  }

  // always make a version file in live-reload mode
  app.use('/multigame/version.json', function (request, response) {
    if(!fs.existsSync(path.join(ASSETS_DIRECTORY, 'version.json'))) {
      writeVersionFile(latestMtime)
    }
    response.sendFile(path.join(ASSETS_DIRECTORY, 'version.json'));
  });
}


app.listen(8080, ()=> {
  console.log(`Server is running on 8080` )
})
