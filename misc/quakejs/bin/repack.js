var fs = require('fs')
var path = require('path')
var glob = require('glob')
var {graphQVM, loadQVM, loadQVMData, getGameAssets} = require('../lib/asset.qvm.js')
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
  
  console.log(`Not found: ${game.notfound.length}`, game.notfound.slice(0, 10))
  console.log(`Files in baseq3: ${game.baseq3.length}`, game.baseq3.slice(0, 10))
  
  // largest matches, more than 5 edges?
  var vertices = game.graph.getVertices()
  vertices.sort((a, b) => b.inEdges.length - a.inEdges.length)
  console.log('Most used assets:', vertices.slice(0, 10).map(v => v.inEdges.length + ' - ' + v.id))
  
  // how many packs to create?
  var filesOverLimit = vertices
    .filter(v => v.inEdges.length > edges)
    .map(v => [v.id].concat(v.outEdges.map(e => e.inVertex.id)))
    .flat(1)
    .filter(f => game.everything.includes(f))
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
  
  
  //'player/*/**' && (model||sound)
  
  
  return
  // subtract those pregrouped files from what is left
  // make a map of maps to entities needed to save bandwidth
  
  
  // for each md3, bsp, group by parent directory
  var roots = vertices.filter(v => v.id.match(/(\.bsp|\.md3|\.qvm)/i))
  var grouped = {'menu': [], 'menu/game': []}
  for(var i = 0; i < roots.length; i++) {
    var parent = roots[i].id.includes('.bsp')
      ? roots[i].id.replace('.bsp', '')
      : path.dirname(roots[i].id)
    
    if(typeof grouped[parent] == 'undefined') {
      grouped[parent] = []
    }
    grouped[parent].push(roots[i].id)
    grouped[parent].push.apply(grouped[parent], roots[i].outEdges.map(e => e.inVertex.id))
    var textures = roots[i].outEdges.map(e => e.inVertex.outEdges.map(e => e.inVertex.id))
    grouped[parent].push.apply(grouped[parent], [].concat.apply([], textures))
  }
  
  var groupedFlat = Object.values(grouped).flat(1)
  var linked = game.everything.filter(e => groupedFlat.includes(e))
  percent('Linked assets', linked.length, game.everything.length)
    
  // make sure we have everything
  for(var i = 0; i < game.everything.length; i++) {
    var parent = path.dirname(game.everything[i])
    if(parent.includes('/menu')
      || path.resolve(parent).toLowerCase() == path.resolve(project).toLowerCase()
      || uiUnique.includes(game.everything[i])
      || game.everything[i].includes('ui.qvm')) {
      parent = 'menu'
    } else if (cgameUnique.filter(f => !f.includes('player')).includes(game.everything[i])
      || game.everything[i].includes('cgame.qvm')) {
      parent = 'menu/game'
    } else if(linked.includes(game.everything[i])
      // put shared assets in a directory of their own
      && !filesOverLimit.includes(game.everything[i])) {
      continue
    }
    if(typeof grouped[parent] == 'undefined') {
      grouped[parent] = []
    }
    grouped[parent].push(game.everything[i])
    var dependencies = vertices
      .filter(v => v.id == game.everything[i])
      .map(v => v.outEdges.map(e => e.inVertex.id)
        .concat(v.outEdges.map(e => e.inVertex.outEdges.map(e2 => e2.inVertex.id))
          .flat(1)))
      .flat(1)
    grouped[parent].push.apply(grouped[parent], dependencies)
  }
  // TODO: add weapon/powerup sounds using lists of data from bg_misc, but some mods
  //   like weapons factory load this config from a file we can use instead
  
  // in order of confidence, most to least
  var numericMap = [
    ['menu', 1], // 1 - ui.qvm, menu system to get the game running, all scripts
                 // 11 - cgame.qvm, qagame.qvm, in game feedback sounds not in menu
                 // 12-19 - menu pk3s, hud, sprites
    ['maps', 9], // 90-99 - map textures
    ['weapon', 5], // 50-59 - weapons 1-9
    ['mapobject', 7],
    ['powerup', 3], // 30-39 - powerups
    ['player', 2], // 20-29 - player models
    ['item', 4],
    ['model', 6],
    ['other', /.*/, 8], // 80-89 - map models
  ]
  var groupedKeys = ['menu', 'menu/game'].concat(Object.keys(grouped))
  var condensed = {}
  for(var i = 0; i < groupedKeys.length; i++) {
    var parent = groupedKeys[i]
    // combine with parent basenames that match
    var newParent = null
    for(var m = 0; m < numericMap.length; m++) {
      if(newParent) break
      for(var j = 0; j < numericMap[m].length - 1; j++) {
        if(parent.match(new RegExp(numericMap[m][j]))) {
          newParent = numericMap[m][0] + path.basename(parent)
          break
        }
      }
    }
    if(typeof condensed[newParent] == 'undefined') {
      condensed[newParent] = []
    }
    condensed[newParent].push.apply(condensed[newParent], grouped[groupedKeys[i]])
  }
  
  var totalAssets = []
  var assetCount = {}
  var condensedKeys = ['menumenu', 'menugame'].concat(Object.keys(condensed))
  for(var i = 0; i < condensedKeys.length; i++) {
    var parent = condensedKeys[i]
    var pakName = null
    for(var m = 0; m < numericMap.length; m++) {
      var type = numericMap[m][0]
      var numeral = numericMap[m][numericMap[m].length - 1]
      if(parent.includes(type)) {
        if(typeof assetCount[type] == 'undefined') {
          assetCount[type] = 1
        } else {
          assetCount[type]++
        }
        // replace names with numeric counts
        pakName = 'pak' + parent
          .replace(type, getSequenceNumeral(numeral, assetCount[type]))
        break
      }
    }
    if(!pakName) {
      throw new Error('Something went wrong with naming paks')
    }
    var uniqueAssets = []
    if(typeof condensed[parent] == 'undefined') continue
    for(var j = 0; j < condensed[parent].length; j++) {
      var f = condensed[parent][j]
      if(game.everything.includes(f) && !totalAssets.includes(f)) {
          uniqueAssets.push(f)
          totalAssets.push(f)
        }
    }
    if(uniqueAssets.length > edges) {
      condensed[pakName] = uniqueAssets
    } else {
      condensed['pak12game'].push.apply(condensed['pak12game'], uniqueAssets)
    }
    delete condensed[parent]
  }
  
  condensedKeys = Object.keys(condensed)
  condensedKeys.sort()
  console.log('Proposed layout:', condensedKeys.map(k => k + ' - ' + condensed[k].length))
  console.log(`Assets accounted for: ${totalAssets.length}/${game.everything.length
    } - ${percent(totalAssets.length, game.everything.length)}%`)
  game.condensed = condensed
  fs.writeFileSync(TEMP_NAME, JSON.stringify(condensed, null, 2))
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

/* Expected output
Known files: 1041/3777 - 27.6%
Known directories: 3666/3777 - 97.1%
Menu files: 877/3777 - 5810202 bytes
Cgame files: 1142/3777 - 30.2%
Not found: 623
Files in baseq3: 1
Most used assets: [
  '209 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/sound/world/fire1.opus',
  '75 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/sound/world/firesoft.opus',
  '43 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/sound/world/wind1.opus',
  '42 - textures/common/nodrawnonsolid',
  '36 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/textures/base_light/ceil1_39.blend.jpg',
  '35 - textures/common/clip',
  '35 - textures/common/caulk',
  '34 - textures/gothic_floor/metalbridge06brokeb',
  '33 - flareShader',
  '32 - textures/common/trigger'
]
Shared files: 654/3777 - 17.3%
Least used assets: [
  '1 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/models/flags/b_flag.md3 - models/flags/b_flag',
  '1 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/models/flags/r_flag.md3 - models/flags/r_flag',
  '1 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/models/mapobjects/bitch/fembotbig.md3 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/q3dm0.bsp',
  '1 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/models/weapons2/bfg/bfg_1.md3 - models/weapons2/bfg/bfg',
  '1 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/models/weapons2/grapple/grapple_hand.md3 - models/weapons2/grapple/grapple_h',
  '1 - textures/base_button/shootme2 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/q3dm11.bsp',
  '1 - textures/base_floor/cybergrate2 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/q3dm12.bsp',
  '1 - textures/base_floor/diamond2cspot - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/q3dm0.bsp',
  '1 - textures/base_floor/clangspot2 - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/q3dm12.bsp',
  '1 - textures/base_floor/clangdarkspot - /users/briancullinan/planet_quake_data/baseq3-combined-converted/maps/pro-q3dm6.bsp'
]
Unused assets: 2440 [
  'textures/base_light/btactmach0',
  'textures/base_floor/techfloor',
  'textures/base_floor/clang_floor_ow',
  'textures/base_floor/cybergrate',
  'textures/base_floor/pool_side2',
  'textures/base_floor/pool_side3',
  'textures/base_floor/pool_floor2',
  'textures/base_floor/pool_floor3',
  'textures/base_floor/clangspot',
  'textures/base_floor/tilefloor5'
]
Linked assets: 1600/3777 - 42.4%
Proposed layout: [
  'pak11menu - 169',
  'pak12game - 74',
  'pak20grunt - 47',
  'pak21anarki - 45',
  'pak220razor - 55',
  'pak221hunter - 47',
  'pak2220players - 31',
  'pak2221sarge - 56',
  'pak22223player - 37',
  'pak22224announce - 65',
  'pak22225footsteps - 28',
  'pak2222slash - 55',
  'pak2223sorlag - 41',
  'pak2224tankjr - 38',
  'pak2225tim - 18',
  'pak2226uriel - 50',
  'pak2227visor - 43',
  'pak2228xaero - 43',
  'pak2229xian - 13',
  'pak222keel - 41',
  'pak223klesk - 48',
  'pak224lucy - 47',
  'pak225major - 47',
  'pak226mynx - 41',
  'pak227orbb - 45',
  'pak228paulj - 12',
  'pak229ranger - 46',
  'pak22biker - 65',
  'pak23bitterman - 40',
  'pak24bones - 40',
  'pak25brandon - 17',
  'pak26carmack - 15',
  'pak27cash - 12',
  'pak28crash - 43',
  'pak29doom - 47',
  'pak31ammo - 37',
  'pak32armor - 14',
  'pak33health - 20',
  'pak34holdable - 8',
  'pak35instant - 26',
  'pak36powerups - 5',
  'pak41items - 21',
  'pak50shells - 5',
  'pak51bfg - 12',
  'pak52gauntlet - 8',
  'pak53grapple - 15',
  'pak54grenadel - 7',
  'pak551shotgun - 9',
  'pak552weapons2 - 11',
  'pak553weapons - 11',
  'pak55lightning - 15',
  'pak56machinegun - 18',
  'pak57plasma - 11',
  'pak58railgun - 11',
  'pak59rocketl - 8',
  'pak60rlboom - 8',
  'pak61ammo - 5',
  'pak62rocket - 6',
  'pak63gibs - 11',
  'pak65weaphits - 27',
  'pak66flags - 8',
  'pak67models - 9',
  'pak72podium - 7',
  'pak77mapobjects - 30',
  'pak80levelshots - 39',
  'pak81botfiles - 20',
  'pak82bots - 174',
  'pak83gfx - 4',
  'pak842d - 18',
  'pak85numbers - 11',
  'pak87damage - 8',
  'pak881music - 9',
  'pak882scripts - 32',
  'pak883sound - 8',
  'pak884feedback - 32',
  'pak8882sprites - 6',
  'pak8883explode1 - 23',
  'pak8884textures - 29',
  'pak8886base_door - 5',
  'pak8887base_floor - 14',
  'pak88880gothic_floor - 9',
  'pak88881base_trim - 12',
  'pak88882base_wall - 22',
  'pak88884ctf - 5',
  'pak88885effects - 5',
  'pak88886gothic_block - 9',
  'pak88887gothic_cath - 11',
  'pak888881gothic_light - 4',
  'pak888882gothic_trim - 16',
  'pak888883gothic_wall - 8',
  'pak888886sfx - 17',
  'pak8888base_light - 23',
  'pak888teamplay - 26',
  'pak889world - 6',
  'pak88misc - 24',
  'pak89icons - 42',
  'pak90q3dm1 - 53',
  'pak91pro-q3dm13 - 97',
  'pak92pro-q3dm6 - 68',
  'pak93pro-q3tourney2 - 60',
  ... 27 more items
]
Assets accounted for: 3777/3777 - 100%

*/

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
//gameInfo(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')), PROJECT)
gameInfo(0, PROJECT)
//groupAssets(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')), PROJECT)
//graphShaders(PROJECT)
//graphGame(0, PROJECT)
//var game = graphGame(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')))
//repack(JSON.parse(fs.readFileSync(TEMP_NAME).toString('utf-8')))
//repack()
//graphQVM('**/pak8.pk3dir/**/*.qvm')
//var strings = loadQVM('**/cgame.qvm', PROJECT)
