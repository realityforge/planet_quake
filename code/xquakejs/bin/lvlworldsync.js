var https = require('https')
var path = require('path')
var Client = require('ftp');
var fs = require('fs');

function downloadMaps(newMaps) {
  if(newMaps.length === 0) return
  var nextMap = newMaps.pop()
  if(!nextMap.levelId) {
    downloadMaps(newMaps)
    return
  }
  var ftpUrl = '/' + [/[a-f]/, 'a-f',
    /[g-l]/i, 'g-l',
    /[m-r]/i, 'm-r',
    /[s-z]/i, 's-z',
    /[0-9]/i, 'other'
  ].filter((a, i, arr) => {
    return i % 2 === 1 ? (nextMap.zip[0].match(arr[i-1])?1:0) : false
  })[0] + '/' + nextMap.zip + '.zip'
  console.log('Downloading ' + nextMap.includes[0].title + ' from ' + ftpUrl)
  var newFile = path.join('/Applications/ioquake3/bestmaps',
    nextMap.levelId + '-' + nextMap.zip + '.zip')
  if(fs.existsSync(newFile)) {
    downloadMaps(newMaps)
    return
  }
  var c = new Client()
  c.on('ready', function() {
    c.get(ftpUrl, function(err, stream) {
      if (err) throw err
      var fws = fs.createWriteStream(newFile)
      stream.once('close', function() { c.end() })
      stream.pipe(fws)
      fws.on('finish', function () {
        downloadMaps(newMaps)
      })
    })
  })
  // connect to localhost:21 as anonymous
  c.connect({
    host:'lvlmirror.mhgaming.com'
  })
  /*
  https.get('https://lvlworld.com/download/id:' + nextMap.levelId, 
    function (res) {
      res.pipe(fws)
    })
  */
}

function downloadNewMaps(maps, existing) {
  var bsp = Object.keys(existing)
    .filter(k => k.includes('.bsp'))
    .map(k => path.basename(k).replace('.bsp', '').toLowerCase())
  var newMaps = Object.keys(maps)
    .filter(k => !bsp.includes(maps[k].includes[0].bsp.toLowerCase()))
    .map(k => maps[k])
  downloadMaps(newMaps)
}

function downloadExistingIndex(maps) {
  
  https.get('https://quake.games/assets/baseq3-cc/index.json?' + Date.now(),
    function(res) {
      var body = ''
      res.on('data', function(chunk){
          body += chunk
      })
      res.on('end', function(){
          var existing = JSON.parse(body)
          downloadNewMaps(maps, existing)
      })
    })

}


function downloadMapIndex() {
  
  https.get('https://lvlworld.com/metadata/from:'
    + (new Date()).getFullYear() + '-01-01/limit:200',
    function(res) {
      var body = ''
      res.on('data', function(chunk){
          body += chunk
      })
      res.on('end', function(){
          var maps = JSON.parse(body)
          downloadExistingIndex(maps)
      })
    })

}

downloadMapIndex()
