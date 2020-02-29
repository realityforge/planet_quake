var fs = require('fs')
var path = require('path')
var glob = require('glob')
var assert = require('assert')
var minimatch = require('minimatch')
var {graphQVM, loadQVM, loadQVMData, getGameAssets, MATCH_ENTS} = require('../lib/asset.qvm.js')
var {graphGame, graphModels, graphMaps, graphShaders} = require('../lib/asset.game.js')
var {compressDirectory} = require('../bin/compress.js')
var {
  findTypes, fileTypes, sourceTypes,
  audioTypes, imageTypes, findTypes,
} = require('../bin/repack-whitelist.js')
var {ufs} = require('unionfs')
ufs.use(fs)

var PROJECT = '/Users/briancullinan/planet_quake_data/baseq3-combined-converted'
var TEMP_NAME = path.join(__dirname, 'previous-graph.json')
var PAK_NAME = path.join(__dirname, 'previous-pak.json')

// in order of confidence, most to least
var numericMap = [
  ['menu', 1], // 1 - ui.qvm, menu system to get the game running, all scripts
               // 12-19 - menu pk3s, hud, sprites
  ['game', 2], // 2 - cgame.qvm, qagame.qvm, in game feedback sounds not in menu
  ['maps', 9], // 90-99 - map textures
  ['weapon', 5], // 50-59 - weapons 1-9
  ['mapobject', 7], // misc_models
  ['powerup', 3], // 30-39 - powerups
  ['player', 4], // 40-49 - player models
  ['model', 6],
  ['other', /.*/, 8], // 80-89 - map models
]

var edges = 3
/*
Planned options:
--edges - number of connected edges to deserve it's own pk3, default is 3
--roots - insert yourself anywhere in the graph, show top connections from that asset
--info -i - only print info, don't actually do any converting
--convert - options to pass to image magick, make sure to put these last
--transcode - options to pass to opus/ogg vorbis, make sure to put these last
--entities - entities definition to group models and sounds

Basic steps:

Unpack
Graph
Convert
Repack

*/
var mountPoints = []

function percent(l, a, b) {
  console.log(`${l}: ${a}/${b} - ${Math.round(a/b*1000) / 10}%`)
}

function gameInfo(gs, project) {
  if(!project) {
    project = PROJECT
  }
  var game = graphGame(gs, project)
  // how many files are matched versus unknown?
  game.files = game.files || findTypes(fileTypes, game.everything)
  game.images = game.images || findTypes(imageTypes, game.everything)
  game.audio = game.audio || findTypes(audioTypes, game.everything)
  game.sources = game.sources || findTypes(sourceTypes, game.everything)
  game.known = game.known || findTypes([imageTypes, audioTypes, sourceTypes, fileTypes].flat(1), game.everything)
  percent('Image files', game.images.length, game.everything.length)
  percent('Audio files', game.audio.length, game.everything.length)
  percent('Source files', game.sources.length, game.everything.length)
  percent('Known files', game.files.length, game.everything.length)
  percent('Recognized files', game.known.length, game.everything.length)
  var unrecognized = game.everything.filter(f => !game.known.includes(f))
  percent('Unrecognized files', unrecognized.length, game.everything.length)
  console.log(unrecognized.slice(0, 10))
  
  // how many files a part of menu system?  
  var uiqvm = game.graph.getVertices()
    .filter(v => v.id.match(/ui\.qvm/i))[0]
    .outEdges
    .map(e => e.inVertex.id)
    .filter(e => game.everything.includes(e))
  percent('UI files', uiqvm.length, game.everything.length)
  var cgame = game.graph.getVertices()
    .filter(v => v.id.match(/cgame\.qvm/i))[0]
    .outEdges
    .map(e => e.inVertex.id)
    .filter(e => game.everything.includes(e))
  percent('Cgame files', cgame.length, game.everything.length)
  var qagame = game.graph.getVertices()
    .filter(v => v.id.match(/qagame\.qvm/i))[0]
    .outEdges
    .map(e => e.inVertex.id)
    .filter(e => game.everything.includes(e))
  percent('QAgame files', qagame.length, game.everything.length)

  var qvmFiles = game.everything
    .filter(f => uiqvm.includes(f) || cgame.includes(f) || qagame.includes(f))
  percent('Total QVM files', qvmFiles.length, game.everything.length)
  
  console.log(`Missing/not found: ${game.notfound.length}`, game.notfound.slice(0, 10))
  var mapFiles = Object.values(game.maps).flat(1)
    .concat(Object.values(game.mapEntities).flat(1))
    .concat(Object.values(game.shaders).flat(1))
    .concat(Object.values(game.qvms).flat(1))
  var exludingMap = game.notfound.filter(n => !mapFiles.includes(n))
  console.log(`Missing/not found excluding map/shaders: ${exludingMap.length}`, exludingMap.slice(0, 10))
  console.log(`Files in baseq3: ${game.baseq3.length}`, game.baseq3.slice(0, 10))
  
  // largest matches, more than 5 edges?
  var vertices = game.graph.getVertices()
  vertices.sort((a, b) => b.inEdges.length - a.inEdges.length)
  console.log('Most used assets:', vertices.slice(0, 10).map(v => v.inEdges.length + ' - ' + v.id))
  
  // how many packs to create?
  var filesOverLimit = vertices
    .filter(v => v.inEdges.length > edges)
    .map(v => [v.id]
      .concat(v.outEdges.map(e => e.inVertex.id))
      .concat(v.outEdges.map(e => e.inVertex.outEdges.map(e2 => e2.inVertex.id))))
    .flat(2)
    .filter((f, i, arr) => arr.indexOf(f) === i && game.everything.includes(f))
  percent('Shared files', filesOverLimit.length, game.everything.length)
  
  // how many files are graphed versus unmatched or unknown?
  vertices.sort((a, b) => a.inEdges.length - b.inEdges.length)
  console.log('Least used assets:', vertices
    .filter(v => v.inEdges.length > 0 && !v.id.match(/(\.bsp|\.md3|\.qvm|\.aas)/i))
    .slice(0, 10)
    .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')))
  console.log('Least used models:', vertices
    .filter(v => v.inEdges.length > 0 && v.id.match(/(\.md3)/i) && !v.id.match(/(players)/i))
    .slice(0, 10)
    .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')))
  
  var allShaders = Object.values(game.scripts).flat(1)
  var unused = vertices
    .filter(v => v.inEdges.length == 0
      && !v.id.match(/(\.bsp|\.md3|\.qvm|\.shader)/i)
      && !allShaders.includes(v.id))
  console.log('Unused assets:', unused.length, unused.slice(0, 10).map(v => v.id))
  
  var allVertices = vertices.map(v => v.id)
  var graphed = game.everything.filter(e => allVertices.includes(e))
  percent('All graphed', graphed.length, game.everything.length)
  var ungraphed = game.everything.filter(e => !allVertices.includes(e))
  percent('Ungraphed', ungraphed.length, game.everything.length)
  console.log(ungraphed.slice(0, 10))
  
  return gs
}

function groupAssets(gs, project) {
  if(!project) {
    project = PROJECT
  }
  var game = graphGame(gs, project)
  var vertices = game.graph.getVertices()
  var grouped = {'menu/menu': [], 'game/game': []}
  
  // group all entities
  var entityDuplicates = Object.keys(game.entities)
    .map(ent => game.graph.getVertex(ent))
    .filter((v, i, arr) => v)
    .map(v => [v]
      .concat(v.outEdges.map(e => e.inVertex.id))
      .concat(v.outEdges.map(e => e.inVertex.outEdges.map(e2 => e2.inVertex.id)).flat(1))
      .filter((f, i, arr) => arr.indexOf(f) === i))
    .flat(1)
    .filter((f, i, arr) => arr.indexOf(f) !== i)
  Object.keys(game.entities).forEach(ent => {
    var v = game.graph.getVertex(ent)
    if(!v) return true
    var entFiles = v.outEdges.map(e => e.inVertex.id)
      .concat(v.outEdges.map(e => e.inVertex.outEdges.map(e2 => e2.inVertex.id)).flat(1))
      .filter((f, i, arr) => arr.indexOf(f) === i && !entityDuplicates.includes(f))
    var model = entFiles.filter(f => f.match(/.\.md3/i))[0]
      || entFiles[0]
    var pakClass = numericMap
      .filter(map => map.filter((m, i) => 
        i < map.length - 1 && model.match(new RegExp(m))).length > 0)[0][0]
    var pakKey = pakClass + '/' + ent.split('_')[1]
    if(typeof grouped[pakKey] == 'undefined') {
      grouped[pakKey] = []
    }
    grouped[pakKey].push.apply(grouped[pakKey], entFiles)
  })
  
  // group players with sounds
  // TODO: use graph for player models instead,
  //   incase they are not all in the correctly named folders
  game.everything.filter(minimatch.filter('**/+(player|players)/**'))
    .forEach(f => {
      var player = path.basename(path.dirname(f))
      //model and sound matched
      if(typeof grouped['player/' + player] === 'undefined') {
        grouped['player/' + player] = []
      }
      grouped['player/' + player].push(f)
    })

  // all current assets that shouldn't be included in menu/cgame by default
  var externalAssets = Object.values(grouped).flat(1)
  
  // group shared textures and sounds by folder name
  var filesOverLimit = vertices
    .filter(v => v.inEdges.length > edges)
    .map(v => [v.id]
      .concat(v.outEdges.map(e => e.inVertex.id))
      .concat(v.outEdges.map(e => e.inVertex.outEdges.map(e2 => e2.inVertex.id))))
    .flat(2)
    .concat(entityDuplicates)
    .filter((f, i, arr) => arr.indexOf(f) === i
      && game.everything.includes(f) && !externalAssets.includes(f))
  filesOverLimit.forEach(f => {
    var parent = path.basename(path.dirname(f))
    var pakClass = numericMap
      .filter(map => map.filter((m, i) => 
        i < map.length - 1 && f.match(new RegExp(m))).length > 0)[0][0]
    if(typeof grouped[pakClass + '/' + parent] == 'undefined') {
      grouped[pakClass + '/' + parent] = []
    }
    grouped[pakClass + '/' + parent].push(f)
  })

  var externalAndShared;
    
  // map menu and cgame files
  var qvms = Object.keys(game.qvms).filter(qvm => qvm.match(/ui.qvm/i))
    .concat(Object.keys(game.qvms).filter(qvm => !qvm.match(/ui.qvm/i)))
  qvms.forEach(qvm => {
    // update shared items so menu is downloaded followed by cgame
    externalAndShared = Object.values(grouped).flat(1)
    
    var className = qvm.match(/ui.qvm/i) ? 'menu' : 'game'
    // don't include disassembly in new pak
    // don't include maps obviously because they are listed below
    var gameVertices = game.graph.getVertex(qvm).outEdges.map(e => e.inVertex)
      .filter(v => !v.id.match(/\.dis|\.bsp/i))
    var gameAssets = [qvm]
      .concat(gameVertices.map(v => v.id))
      .concat(gameVertices
        .filter(v => !v.id.match(/\.shader/i))
        .map(v => v.outEdges.map(e2 => e2.inVertex.id)))
      .flat(2)
      .filter((f, i, arr) => arr.indexOf(f) === i
        && game.everything.includes(f)
        && !externalAndShared.includes(f))
    
    gameAssets.forEach(f => {
      var pakName = path.basename(path.dirname(f))
      if(pakName == path.basename(project)) {
        pakName = className
      }
      if(typeof grouped[className + '/' + pakName] == 'undefined') {
        grouped[className + '/' + pakName] = []
      }
      grouped[className + '/' + pakName].push(f)
    })
  })
  
  externalAndShared = Object.values(grouped).flat(1)
  
  // group all map models and textures by map name
  Object.keys(game.maps)
    .map(m => game.graph.getVertex(m))
    .forEach(v => {
      var map = path.basename(v.id).replace(/\.bsp/i, '')
      var mapAssets = v.outEdges.map(e => e.inVertex.id)
        .filter(f => !externalAndShared.includes(f))
        // map through shaders
        .concat(v.outEdges.map(e => e.inVertex.outEdges.map(v => v.inVertex.id)).flat(1))
        .filter((f, i, arr) => arr.indexOf(f) === i && game.everything.includes(f) && !externalAndShared.includes(f))
      if(mapAssets.length > 0) {
        grouped['maps/' + map] = mapAssets
      }
    })

  // make sure lots of items are linked
  var groupedFlat = Object.values(grouped).flat(1)
  var linked = game.everything.filter(e => groupedFlat.includes(e))
  var unlinked = game.everything.filter(e => !groupedFlat.includes(e))
  percent('Linked assets', linked.length, game.everything.length)
  percent('Unlinked assets', unlinked.length, game.everything.length)
  console.log(unlinked.slice(0, 10))
  
  // regroup groups with only a few files
  var condensed = Object.keys(grouped).reduce((obj, k) => {
    var newKey = k.split('/')[0]
    if(grouped[k].length <= edges || k.split('/')[1] == newKey) {
      if(typeof obj[newKey] == 'undefined') {
        obj[newKey] = []
      }
      obj[newKey].push.apply(obj[newKey], grouped[k])
    } else {
      obj[k] = grouped[k]
    }
    return obj
  }, {})
  
  // make sure condensing worked properly
  var condensedFlat = Object.values(condensed).flat(1)
  var condensedLinked = game.everything.filter(e => condensedFlat.includes(e))
  assert(condensedLinked.length === linked.length, 'Regrouped length doesn\'t match linked length')
  
  // regroup by numeric classification
  var numeralCounts = {}
  var renamed = Object.keys(condensed).reduce((obj, k) => {
    var pakClass = numericMap
      .filter(map => map.filter((m, i) => 
        i < map.length - 1 && k.match(new RegExp(m))).length > 0)[0]
    var numeral = pakClass[pakClass.length - 1]
    if(typeof numeralCounts[numeral] == 'undefined') numeralCounts[numeral] = 1
    else numeralCounts[numeral]++
    var pakNumeral = getSequenceNumeral(numeral, numeralCounts[numeral])
    var pakName = k.split('/').length === 1
      ? (pakNumeral + k)
      : (pakNumeral + k.split('/')[1])
    obj[ 'pak' + pakName] = condensed[k]
    return obj
  }, {})
  
  // make sure renaming worked properly
  var renamedFlat = Object.values(renamed).flat(1)
  var renamedLinked = game.everything.filter(e => renamedFlat.includes(e))
  assert(renamedLinked.length === linked.length, 'Renamed length doesn\'t match linked length')
  
  // make sure there are no duplicates
  var duplicates = Object.values(renamed).flat(1).filter((f, i, arr) => arr.indexOf(f) !== i)
  console.log('Duplicates found: ' + duplicates.length, duplicates.slice(0, 10))
  
  // sort the renamed keys for printing output
  var renamedKeys = Object.keys(renamed)
  renamedKeys.sort()
  var ordered = renamedKeys.reduce((obj, k) => {
    obj[k] = renamed[k]
    obj[k].sort()
    return obj
  }, {})
  console.log('Proposed layout:',
    renamedKeys.map(k => k + ' - ' + renamed[k].length),
    renamedKeys.map(k => k + ' - ' + renamed[k].length).slice(100))
  fs.writeFileSync(PAK_NAME, JSON.stringify(ordered, null, 2))
  
  // generate a index.json the server can use for pk3 sorting based on map/game type
  
  
  //assert(duplicates.length === 0, 'Duplicates found: ' + duplicates.length)
  //fs.writeFileSync(TEMP_NAME, JSON.stringify(condensed, null, 2))
  return game
}

function getSequenceNumeral(pre, count) {
  var digitCount = Math.ceil(count / 10) + 1
  var result = 0
  for(var i = 1; i < digitCount; i++) {
    result += pre * Math.pow(10, i)
  }
  return result + (count % 10)
}

async function repack(condensed, project) {
  if(!project) {
    project = PROJECT
  }
  if(!condensed) {
    condensed = gameInfo(PROJECT).condensed
  }
  var outputProject = path.join(path.dirname(project), path.basename(project) + '-repacked')
  if(!fs.existsSync(outputProject)) fs.mkdirSync(outputProject)
  var everything = glob.sync('**/*', {cwd: project})
    .map(f => path.join(project, f))
  var everythingLower = everything.map(f => f.toLowerCase())
  var condensedKeys = Object.keys(condensed)
  for(var i = 0; i < condensedKeys.length; i++) {
    var pak = condensed[condensedKeys[i]]
    console.log('Packing ' + condensedKeys[i])
    var real = pak
      .map(f => everything[everythingLower.indexOf(f)])
      .filter(f => fs.existsSync(f) && !fs.statSync(f).isDirectory())
    var output = fs.createWriteStream(path.join(outputProject, condensedKeys[i] + '.pk3'))
    await compressDirectory(real, output, project)
  }
}

//graphModels('/Users/briancullinan/planet_quake_data/baseq3-combined-converted')
//graphMaps(PROJECT)
gameInfo(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')), PROJECT)
//gameInfo(0, PROJECT)
//groupAssets(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')), PROJECT)
//graphShaders(PROJECT)
//graphGame(0, PROJECT)
//var game = graphGame(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')))
//repack(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')))
//repack()
//graphQVM('**/pak8.pk3dir/**/*.qvm')
//var strings = loadQVM('**/cgame.qvm', PROJECT)
