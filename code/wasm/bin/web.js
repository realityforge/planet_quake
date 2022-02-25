const express = require('express');
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
app.use('/', express.static(__dirname + '/../http/'));


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
