var https = require('https')
var path = require('path')
var Client = require('ftp')
var fs = require('fs')
var glob = require('glob')
var {makeMapIndex, makeIndexJson} = require('./content.js')
var {spawn} = require('child_process')
var {unpackPk3s} = require('./compress.js')
var {convertGameFiles} = require('./convert.js');
var {loadDefaultDirectories} = require('../lib/asset.game.js')

var UPDATE_DIRNAME = 'bestmaps-' + (new Date()).getFullYear()
//  + '-' + (new Date()).getMonth() + '-' + (new Date()).getDate()
var UPDATE_DIRECTORY = '/Applications/ioquake3/' + UPDATE_DIRNAME
var TEMP_DIR = path.join(process.env.HOME || process.env.HOMEPATH 
  || process.env.USERPROFILE || os.tmpdir(), '/.quake3')
var DIRMAP = [
  /[a-f]/i, 'a-f',
  /[g-l]/i, 'g-l',
  /[m-r]/i, 'm-r',
  /[s-z]/i, 's-z',
  /[0-9]/i, 'other'
]

async function downloadMaps(newMaps) {
  if(newMaps.length === 0) {
    return
  }
  if(!fs.existsSync(UPDATE_DIRECTORY)) {
    fs.mkdirSync(UPDATE_DIRECTORY)
  }
  while(1) {
    var nextMap = newMaps.pop()
    if(!nextMap) {
      break
    }
    if(!nextMap.levelId) {
      continue
    }
    var ftpUrl = '/' + DIRMAP.filter((a, i, arr) => {
      return i % 2 === 1 ? (nextMap.zip[0].match(arr[i-1])?1:0) : false
    })[0] + '/' + nextMap.zip + '.zip'
    console.log('Downloading ' + nextMap.includes[0].title + ' from ' + ftpUrl)
    var newFile = path.join(UPDATE_DIRECTORY, nextMap.levelId + '-'
      + nextMap.zip + '.zip')
    if(fs.existsSync(newFile)) {
      continue
    }
    var c = new Client()
    await new Promise(resolve => {
      c.on('ready', function() {
        c.get(ftpUrl, function(err, stream) {
          if (err) throw err
          var fws = fs.createWriteStream(newFile)
          stream.once('close', function() { c.end() })
          stream.pipe(fws)
          fws.on('finish', function () {
            resolve()
          })
        })
      })
      // connect to localhost:21 as anonymous
      c.connect({
        host:'lvlmirror.mhgaming.com'
      })
    })
  }
}

async function downloadNewMaps(maps, existing) {
  var bsp = Object.keys(existing)
    .filter(k => k.includes('.bsp'))
    .map(k => path.basename(k).replace('.bsp', '').toLowerCase())
  var newMaps = Object.keys(maps)
    .filter(k => !bsp.includes(maps[k].includes[0].bsp.toLowerCase()))
    .map(k => maps[k])
  await downloadMaps(newMaps)
}

async function downloadExistingIndex(maps) {
  var existing
  await new Promise(resolve => {
    https.get('https://quake.games/assets/baseq3-cc/index.json?' + Date.now(),
      function(res) {
        var body = ''
        res.on('data', function(chunk) {
            body += chunk
        })
        res.on('end', function() {
            existing = JSON.parse(body)
            resolve()
        })
      })
  })
  await downloadNewMaps(maps, existing)
  return existing
}


async function downloadMapIndex() {
  var maps
  await new Promise(resolve => {
    https.get('https://lvlworld.com/metadata/from:'
      + (new Date()).getFullYear() + '-01-01/limit:200',
      function(res) {
        var body = ''
        res.on('data', function(chunk) {
            body += chunk
        })
        res.on('end', function() {
            maps = JSON.parse(body)
            resolve()
        })
      })
  })
  return await downloadExistingIndex(maps)
}

async function doUpdate() {
  var remoteIndex = await downloadMapIndex()
  var outCombined = path.join(TEMP_DIR, UPDATE_DIRNAME + '-c')
  var outConverted = path.join(TEMP_DIR, UPDATE_DIRNAME + '-cc')
  var outRepacked = path.join(TEMP_DIR, UPDATE_DIRNAME + '-ccr')

  try {
    await unpackPk3s(UPDATE_DIRECTORY, outCombined, console.log, true, true)
    await loadDefaultDirectories()
    var everything = glob.sync('**/*', { cwd: outCombined, nodir: true })
      .map(f => path.join(outCombined, f))
    var gs = {
      ordered: everything.reduce((obj, f) => {
        if(f.match(/\.pk3dir/i)) {
          var pk3Path = path.basename(f.substr(0, f.match(/\.pk3dir/i).index))
          if(typeof obj[pk3Path] == 'undefined') {
            obj[pk3Path] = []
          }
          obj[pk3Path].push(f)
        }
        return obj
      }, {}),
      everything: everything
    }
    await convertGameFiles(gs, outCombined, outConverted, true, console.log)
    // save the remote index instead of the useless local index
    fs.writeFileSync(path.join(outConverted, 'index.json'), JSON.stringify(remoteIndex, null, 2))
    await makeMapIndex( outCombined, outConverted, outRepacked, false, console.log, false)
  } catch (up) {
    if(up.code != 'ENOENT') throw up
  }

  glob.sync('*.json', {
    cwd: outConverted
  }).forEach(f => fs.writeFileSync(
    path.join(outConverted, f),
    fs.readFileSync(path.join(outConverted, f))
      .toString('utf-8').replace(/bestmaps.*?\//ig, 'baseq3-cc/')))

  console.log('Uploading to GCP')
  await new Promise(resolve => {
    var gsutil = spawn('gsutil', [
      'rsync', '-R', '-x', 'index\.json$',
      path.join(TEMP_DIR, UPDATE_DIRNAME + '-cc'),
      'gs://quake.games/assets/baseq3-cc'
    ])
    gsutil.stdout.on('data', function (data) {
      console.log('stdout: ' + data.toString());
    })
    gsutil.stderr.on('data', function (data) {
      console.log('stderr: ' + data.toString());
    })
    gsutil.on('exit', function (code) {
      resolve()
    })
  })
}

doUpdate()
