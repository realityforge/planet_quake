const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const { send } = require('process');

const GAME_DIRECTORY = 'multigame'
const WEB_DIRECTORY = __dirname + '/../http'
const ASSETS_DIRECTORY = __dirname + '/../../../games/multigame/assets/'
const BUILD_DIRECTORY = __dirname + '/../../../build/'
const ALLOWED_DIRECTORIES = [
  WEB_DIRECTORY,
  ASSETS_DIRECTORY,
  BUILD_DIRECTORY].map(dir => path.resolve(dir));

const BUILD_ORDER = [
  'release-wasm-js',
  'debug-wasm-js',
  //'release-js-js',
  //'debug-js-js',
  'release-darwin-x86_64',
  'debug-darwin-x86_64'
]

// TODO: if trying to load menu path, return the index page
const MENU_PATHS = [
  'MAINMENU',
  'SETUP',
  'MULTIPLAYER',
  'CHOOSELEVEL',
  'ARENASERVERS',
  'DIFFICULTY',
  'PLAYERMODEL',
  'PLAYERSETTINGS',
]

var fileTimeout
var latestMtime = new Date()

function findFile(filename) {
  // layer the file system, so no matter what we're building, the browser loads something
  for(let i = 0; i < BUILD_ORDER.length; i++) {
    let newPath = path.join(BUILD_DIRECTORY, BUILD_ORDER[i], filename)
    if(fs.existsSync(path.resolve(newPath))) {
      return newPath
    }
  }

  if(filename.startsWith(GAME_DIRECTORY)) {
    let newPath = path.join(ASSETS_DIRECTORY, filename.substring(GAME_DIRECTORY.length))
    if(fs.existsSync(path.resolve(newPath))) {
      return newPath
    }
  }

  let newPath = path.join(WEB_DIRECTORY, filename)
  if(fs.existsSync(path.resolve(newPath))) {
    return newPath
  }

  // TODO: more alternatives?
}


// okay this function is apparently a little idiotic and triggers on accesses
//   this is not how native notify works, but maybes it's the best they could make the same
function fileChanged(prefix, eventType, filename) {
  if(filename.includes('version.json')) {
    return // this would be redundant
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
  writeVersionFile(latestMtime)
}


function startFileWatcher() {
  // TODO: enable file watchers in live reload mode
  //for(let i = 0; i < directories.length; i++) {
  //  fs.watch(path.join(ASSETS_DIRECTORY, directories[i]), 
  //    fileChanged.bind(null, directories[i]))
  //}

}


function findAltFile(localName) {
  // what makes this clever is it only converts when requested
  let ext = path.extname(localName)
  let strippedName = localName
  if(ext) {
    strippedName = strippedName.substring(0, localName.length - ext.length)
  }
  let file
  if((file = findFile(strippedName + '.tga'))) {
    return file
  }
  if((file = findFile(strippedName + '.pcx'))) {
    return file
  }
}


function hasAlpha(otherFormatName) {
  try {
    let alphaCmd = execSync(`identify -format '%[opaque]' "${path.resolve(otherFormatName)}"`, 
      {stdio : 'pipe'}).toString('utf-8')
    // if it is alpha
    if(/* true || TODO: allAlpha? */ alphaCmd.match(/true/ig)) {
      return false
    }
  } catch (e) {
    console.error(e.message, (e.output || '').toString('utf-8').substr(0, 1000))
  }
  return true
}


function layeredDir(filename) {
  let list = []

  for(let i = 0; i < BUILD_ORDER.length; i++) {
    let newPath = path.join(BUILD_DIRECTORY, BUILD_ORDER[i], filename)
    if(fs.existsSync(path.resolve(newPath))
      && fs.statSync(path.resolve(newPath)).isDirectory()) {
      list.push.apply(list, fs.readdirSync(path.resolve(newPath))
        .filter(file => file == GAME_DIRECTORY || file == 'vm' 
          || path.extname(file) == '.wasm' || path.extname(file) == '.qvm'))
    }
  }

  if(filename.startsWith(GAME_DIRECTORY)) {
    let newPath = path.join(ASSETS_DIRECTORY, filename.substring(GAME_DIRECTORY.length))
    if(fs.existsSync(path.resolve(newPath))
      && fs.statSync(path.resolve(newPath)).isDirectory()) {
      list.push.apply(list, fs.readdirSync(path.resolve(newPath)))
    }
  }

  let newPath = path.join(WEB_DIRECTORY, filename)
  if(fs.existsSync(path.resolve(newPath))
    && fs.statSync(path.resolve(newPath)).isDirectory()) {
    list.push.apply(list, fs.readdirSync(path.resolve(newPath)))
  }

  if(layeredDir == GAME_DIRECTORY) {
    list.push('version.json')
  }

  return list.filter((p, i, l) => p[0] != '.' && l.indexOf(p) == i)
}


function makeDirectoryHtml(localName, list) {
  let filelist = list.map(node => 
    `<li><a href="${path.join(localName, node)}">${node}</a></li>`).join('\n')
  let title = localName.endsWith('/') ? localName.substring(0, localName.length - 1) : localName
  let breadcrumbs = ('/' + localName).split('/').filter((b, i) => i == 0 || b)
  let pathlinks = breadcrumbs.map((crumb, i) => 
    `<a href="${breadcrumbs.slice(0, i + 1).join('/')}">${i == 0 ? 'home' : crumb}</a>`).join(' / \n')
  return `
<!DOCTYPE html>
<html>
<head>
<title>${title}</title>
<style>ol{list-style:none;padding:0;}</style>
<base href="/" target="_self">
</head>
<body>
<h1>${pathlinks}</h1>
<ol>
${filelist}
</ol>
</body>
</html>
`
}


function writeVersionFile(time) {
  console.log('Updating working directory...')
  // debounce file changes for a second in case there is a copy process going on
  if(fileTimeout) {
    clearTimeout(fileTimeout)
  }
  fileTimeout = setTimeout(function () {
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
  }, 1000)

}


//var fullUrl = req.protocol + '://' + req.get('host') + req.originalUrl;

function respondRequest(request, response) {
  let localName = path.join(request.baseUrl, request.path)
  if(localName[0] == '/')
    localName = localName.substring(1)
  // remove MAINMENU from path
  let menuDir = localName.substring(localName.lastIndexOf('/'))
  if(MENU_PATHS.includes(menuDir.toUpperCase())) {
    localName = localName.substring(0, localName.length - menuDir.length)
  }
  if(localName.endsWith('/')) {
    localName = localName.substring(0, localName.length - 1)
  }
  //if(localName.startsWith(GAME_DIRECTORY))
  //  localName = localName.substring(GAME_DIRECTORY.length)
  //if(localName[0] == '/')
  //  localName = localName.substring(1)

  let file
  if((file = findFile(localName))) {
    // TODO: if loading a directory return a formatted file index HTML directory listing
    if(fs.statSync(file).isDirectory()) {
      if((file = findFile(path.join(localName, 'index.html')))) {
        return response.sendFile(path.resolve(file))
      } else {
        let list = layeredDir(localName)
        return response.send(makeDirectoryHtml(localName, list))
      }
    } else {
      return response.sendFile(path.resolve(file))
    }
  }


// TODO: convert paths like *.pk3dir and *.pk3 to their zip counterparts and stream
// TODO: if loading a path out of a .pk3 file, return it as a directory


  // always make a version file in live-reload mode
  if(localName.match('version.json')) {
    let newPath = path.join(ASSETS_DIRECTORY, 'version.json')
    if(!fs.existsSync(newPath)) {
      writeVersionFile(latestMtime)
    }
    response.sendFile(path.resolve(newPath));
  }

  // if loading an image in a different format convert it
  if((file = findAltFile(localName))) {
    let newPath = path.join(ASSETS_DIRECTORY, localName)
    let alpha = hasAlpha(file)
    if((!alpha && localName.includes('.jpeg'))
      || (alpha && localName.includes('.png'))) {
      execSync(`convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 20% -auto-orient "${file}" "${path.resolve(newPath)}"`, {stdio : 'pipe'})
    }
    if(fs.existsSync(newPath)) {
      return response.sendFile(path.resolve(newPath))
    }
  }

  if((file = findFile('index.html'))) {
    // if loading a missing path return the index page
    if(localName.length < 2) { // index page?
      return response.sendFile(path.resolve(file))
    } else {
      return response.status(404).send() //.sendFile(path.resolve(file))
    }
  }

}

let runServer = false
for(var i = 0; i < process.argv.length; i++) {
  var a = process.argv[i]
  if(a.match(__filename)) {
    runServer = true
  }
}

if(runServer) {
  const app = require('express')()

  app.use(respondRequest)
  
  app.listen(8080, ()=> {
    console.log(`Server is running on 8080` )
  })

  startFileWatcher()
}

module.exports = {
  writeVersionFile,
  respondRequest,

}
