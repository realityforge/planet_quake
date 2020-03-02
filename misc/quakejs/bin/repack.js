var fs = require('fs')
var os = require('os')
var path = require('path')
var glob = require('glob')
var assert = require('assert')
var minimatch = require('minimatch')
var {ufs} = require('unionfs')
ufs.use(fs)

var PROJECT = '/Applications/ioquake3/baseq3'
var PAK_NAME = path.join(__dirname, 'previous-pak.json')
var INFO_NAME = path.join(__dirname, 'previous-info.json')
var STEPS = {
  'source': 'Load Pk3 sources',
  'graph': 'Create a graph',
  'info': 'Print game info',
  'convert': 'Convert web format',
  'repack': 'Zip new paks',
  'clean': 'Cleaning up'
}

var help = `
npm run repack [options] [mod directory]
--edges {n} - (default is 3) number of connected edges to deserve it's own pk3
--roots - insert yourself anywhere in the graph, show top connections from that asset
--info -i - only print info, don't actually do any converting
--convert {args} - options to pass to image magick, make sure to put these last
--transcode {args} - options to pass to opus/ogg vorbis, make sure to put these last
--entities {ent.def/file} - entities definition to group models and sounds
--no-progress - turn off the progress bars for some sort of admining
--previous {optional file or previous-graph.js} -p - try to load information
  from the previous graph so we don't have to do step 1
--temp {directory}
--verbose -v - print all percentage status updates in case there is an error
--help -h - print this help message and exit
e.g. npm run repack -- /Applications/ioquake3/baseq3
npm run repack -- --info
TODO:
--collisions - skip unzipping and repacking, just list files that interfere with each other
`

var edges = 3
var noProgress = false
var convert = ''
var transcode = ''
var entities = ''
var mountPoints = []
var usePrevious = false
var TEMP_DIR = os.tmpdir()
var verbose = false

var isConvertParams = false
var isTranscodeParams = false
for(var i = 0; i < process.argv.length; i++) {
  var a = process.argv[i]
  if(a.match(/\/node$/ig)) continue
  if(a.match(/\/repack\.js$/ig)) continue
  if(ufs.existsSync(a) && ufs.statSync(a).isDirectory(a)) {
    mountPoints.push(a)
  }
  if(a == '--edges') {
    edges = parseInt(process.argv[i+1])
    i++
  } else if (a == '--info' || a == '-i') {
    delete STEPS['convert']
    delete STEPS['repack']
  } else if (a == '--no-progress') {
    noProgress = true
  } else if (a == '--convert') {
    isConvertParams = true
  } else if (a == '--entities') {
    if(ufs.existsSync(process.argv[i+1])) {
      entities = ufs.readFileSync(process.argv[i+1]).toString('utf-8')
      // TODO: need some basic parsing to get the part before _ of every entity name
      i++
    } else {
      console.error(`ERROR: entities def ${process.argv[i+1]} not found`)
    }
  } else if (a == '--previous' || a == '-p') {
    delete STEPS['source']
    if(ufs.existsSync(process.argv[i+1])) {
      usePrevious = process.argv[i+1]
      i++
    } else {
      usePrevious = true
    }
  } else if (a == '--temp') {
    if(ufs.existsSync(process.argv[i+1]) && ufs.statSync(process.argv[i+1].isDirectory())) {
      TEMP_DIR = process.argv[i+1]
      i++
    } else {
      throw new Error(`Temp directory ${process.argv[i+1]} not found or not a directory`)
    }
  } else if (a == '--verbose' || a == '-v') {
    verbose = true
  } else if (a == '--help' || a == '-h') {
    console.log(help)
    process.exit(0)
  } else if(isConvertParams) {
    convert += ' ' + a
    continue
  } else if(isTranscodeParams) {
    transcode += ' ' + a
    continue
  } else {
    console.error(`ERROR: Unrecognized option "${a}"`)
  }
  isConvertParams = false
  isTranscodeParams = false
}
if(typeof STEPS['convert'] != 'undefined') {
  delete STEPS['info']
}
if(mountPoints.length == 0) {
  console.error('ERROR: No mount points, e.g. run `npm run repack /Applications/ioquake3/baseq3`')
  if(ufs.existsSync(PROJECT))
    mountPoints.push(PROJECT)
} else {
  mountPoints.sort((a, b) => a[0].localeCompare(b[0], 'en', { sensitivity: 'base' }))
}
for(var i = 0; i < mountPoints.length; i++) {
  var name = path.basename(mountPoints[i])
  console.log(`Repacking directory ${mountPoints[i]} -> ${path.join(TEMP_DIR, name + '-combined-converted-repacked')}`)
}
try {
  require.resolve('cli-progress');
} catch (err) {
  noProgress = true
}
if(!noProgress) {
  var cliProgress = require('cli-progress')
  var multibar = new cliProgress.MultiBar({
      fps: 120,
      clearOnComplete: true,
      hideCursor: false,
      format: `[\u001b[34m{bar}\u001b[0m] {percentage}% | {value}/{total} | {message}`,
      barCompleteChar: '\u2588',
      barIncompleteChar: '\u2588',
      barGlue: '\u001b[33m',
      forceRedraw: true,
      linewrap: null,
      barsize: 30,
  })
  var oldConsole = console
  function resetRedraw(out) {
    var args = Array.from(arguments).slice(1)
    multibar.terminal.cursorRelativeReset()
    multibar.terminal.clearBottom()
    oldConsole[out].apply(oldConsole, args)
    multibar.update()
  }
  console = {
    log: resetRedraw.bind(null, 'log'),
    error: resetRedraw.bind(null, 'error'),
    info: resetRedraw.bind(null, 'info'),
  }
}

// load these modules down here to the console is overridden
var {loadQVM, loadQVMData, getGameAssets, MATCH_ENTS} = require('../lib/asset.qvm.js')
var {graphGame, graphModels, graphMaps, graphShaders, TEMP_NAME} = require('../lib/asset.game.js')
var {compressDirectory, unpackPk3s} = require('../bin/compress.js')
var {
  findTypes, fileTypes, sourceTypes,
  audioTypes, imageTypes, findTypes,
  allTypes
} = require('../bin/repack-whitelist.js')
var {convertGameFiles, convertNonAlpha, convertAudio} = require('../bin/convert.js')
var globalBars = []

function getPercent(l, a, b) {
  return `${l}: ${a}/${b} - ${Math.round(a/b*1000) / 10}%`
}

function percent(l, a, b) {
  console.log(getPercent(l, a, b))
}

async function progress(bars, forceVerbose) {
  if(bars === false) {
    for(var i = 0; i < globalBars.length; i++) {
      if(!globalBars[i]) continue
      globalBars[i].stop()
    }
    multibar.stop()
    await new Promise(resolve => setTimeout(resolve, 10))
  }
  //e.g. [[1, 0, 10, 'Removing temporary files']]
  for(var i = 0; i < bars.length; i++) {
    if(!multibar) {
      if(bars[i][1] === false) continue
      percent(bars[i][3], bars[i][1], bars[i][2])
      continue
    }
    if(bars[i][1] === false) {
      if(typeof globalBars[bars[i][0]] != 'undefined') {
        globalBars[bars[i][0]].stop()
        multibar.remove(globalBars[bars[i][0]])
      }
      globalBars[bars[i][0]] = void 0
      await new Promise(resolve => setTimeout(resolve, 10))
      continue
    }
    if(verbose || forceVerbose) {
      // print it out so we can see a record too
      percent(bars[i][3], bars[i][1], bars[i][2])
    }
    var info = {
      percentage: Math.round(bars[i][1]/bars[i][2]*1000) / 10,
      value: bars[i][1],
      total: bars[i][2],
      message: bars[i][3]
    }
    if(typeof globalBars[bars[i][0]] == 'undefined') {
      globalBars[bars[i][0]] = multibar.create()
      globalBars[bars[i][0]].start(bars[i][2], bars[i][1], info)
      await new Promise(resolve => setTimeout(resolve, 10))
    } else {
      globalBars[bars[i][0]].setTotal(bars[i][2])
      globalBars[bars[i][0]].update(bars[i][1], info)
    }
    await new Promise(resolve => setTimeout(resolve, 1))
  }
}

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

function getLeaves(v, depth) {
  var result = Array.isArray(v) ? v : [v]
  do {
    level = result
      .map(v => v.outEdges.map(e => e.inVertex))
      .flat(1)
      .filter((v) => !result.includes(v))
    result.push.apply(result, level)
  } while(!depth && depth !== 0 && level.length > 0 || --depth > 0)
  return result.map(v => v.id)
}

async function gameInfo(gs, project) {
  if(!project) {
    project = PROJECT
  }
  var game
  if(!gs.graph) {
    game = await graphGame(gs, project, progress)
  } else {
    game = gs
  }

  // how many files are matched versus unknown?
  game.files = game.files || findTypes(fileTypes, game.everything)
  game.images = game.images || findTypes(imageTypes, game.everything)
  game.audio = game.audio || findTypes(audioTypes, game.everything)
  game.sources = game.sources || findTypes(sourceTypes, game.everything)
  game.known = game.known || findTypes(allTypes, game.everything)
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
    .map(v => getLeaves(v))
    .flat(1)
    .filter((f, i, arr) => arr.indexOf(f) === i && game.everything.includes(f))
  percent('Shared files', filesOverLimit.length, game.everything.length)
  
  // how many files are graphed versus unmatched or unknown?
  vertices.sort((a, b) => a.inEdges.length - b.inEdges.length)
  var leastUsed = vertices
    .filter(v => v.inEdges.length > 0 && !v.id.match(/(\.bsp|\.md3|\.qvm|\.aas)/i))
  console.log('Least used assets:', leastUsed.slice(0, 10)
    .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')))
  
  var leastUsedExcept = vertices
    .filter(v => v.inEdges.length > 0 && v.id.match(/(\.md3)/i) && !v.id.match(/(players)/i))
  console.log('Least used models:', leastUsedExcept.slice(0, 10)
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
  
  console.log(`Info report written to "${INFO_NAME}"`)
  ufs.writeFileSync(INFO_NAME, JSON.stringify({
    summary: {
      images: getPercent('Image files', game.images.length, game.everything.length),
      audio: getPercent('Audio files', game.audio.length, game.everything.length),
      sources: getPercent('Source files', game.sources.length, game.everything.length),
      files: getPercent('Known files', game.files.length, game.everything.length),
      known: getPercent('Recognized files', game.known.length, game.everything.length),
      unrecognized: getPercent('Unrecognized files', unrecognized.length, game.everything.length),
      uiqvm: getPercent('UI files', uiqvm.length, game.everything.length),
      cgame: getPercent('CGame files', cgame.length, game.everything.length),
      qagame: getPercent('QAGame files', qagame.length, game.everything.length),
      qvmFiles: getPercent('Total QVM files', qvmFiles.length, game.everything.length),
      notfound: `Missing/not found: ${game.notfound.length}`,
      excludingMap: `Missing/not found excluding map/shaders: ${exludingMap.length}`,
      baseq3: `Files in baseq3: ${game.baseq3.length}`,
      vertices: 'Most used assets: ' + vertices.slice(0, 10).map(v => v.inEdges.length + ' - ' + v.id),
      filesOverLimit: getPercent('Shared files', filesOverLimit.length, game.everything.length),
      leastUsed: 'Least used assets: ' + leastUsed.slice(0, 10)
        .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', '))
        .join('\n'),
      leastUsedExcept: 'Least used models: ' + leastUsedExcept.slice(0, 10)
        .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', '))
        .join('\n'),
      unused: 'Unused assets:' + unused.length + unused.slice(0, 10).map(v => v.id),
      graphed: getPercent('All graphed', graphed.length, game.everything.length),
      ungraphed: getPercent('Ungraphed', ungraphed.length, game.everything.length) + ungraphed.slice(0, 10)
    },
    images: game.images,
    audio: game.audio,
    sources: game.sources,
    files: game.files,
    known: game.known,
    unrecognized: unrecognized,
    uiqvm: uiqvm,
    cgame: cgame,
    qagame: qagame,
    qvmFiles: qvmFiles,
    notfound: game.notfound,
    exludingMap: exludingMap,
    baseq3: game.baseq3,
    vertices: vertices.map(v => v.inEdges.length + ' - ' + v.id),
    filesOverLimit: filesOverLimit,
    leastUsed: leastUsed
      .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')),
    leastUsedExcept: leastUsedExcept
      .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')),
    unused: unused,
    graphed: graphed,
    ungraphed: ungraphed,
  }, null, 2))
  
  return gs
}

async function groupAssets(gs, project) {
  if(!project) {
    project = PROJECT
  }
  var game
  if(!gs.graph) {
    game = await graphGame(gs, project, progress)
  } else {
    game = gs
  }
  var vertices = game.graph.getVertices()
  var grouped = {'menu/menu': [], 'game/game': []}
  
  // group all entities
  var entityDuplicates = Object.keys(game.entities)
    .map(ent => game.graph.getVertex(ent))
    .filter((v, i, arr) => v)
    .map(v => getLeaves(v))
    .flat(1)
    .filter((f, i, arr) => arr.indexOf(f) !== i)
  
  Object.keys(game.entities).forEach(ent => {
    var v = game.graph.getVertex(ent)
    if(!v) return true
    var entFiles = getLeaves(v).filter(f => !entityDuplicates.includes(f))
    var model = entFiles.filter(f => f.match(/.\.md3/i))[0] || entFiles[0]
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
  var filesOverLimit = getLeaves(vertices.filter(v => v.inEdges.length > edges))
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
  var qvms = Object.keys(game.qvms)
    .filter(qvm => qvm.match(/ui.qvm/i))
    .concat(Object.keys(game.qvms)
    .filter(qvm => !qvm.match(/ui.qvm/i)))
  qvms.forEach(qvm => {
    // update shared items so menu is downloaded followed by cgame
    externalAndShared = Object.values(grouped).flat(1)
    
    var className = qvm.match(/ui.qvm/i) ? 'menu' : 'game'
    // don't include disassembly in new pak
    // don't include maps obviously because they are listed below
    var gameVertices = game.graph.getVertex(qvm)
      .outEdges
      .map(e => e.inVertex)
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
      var mapAssets = getLeaves(v)
        .filter(f => game.everything.includes(f) && !externalAndShared.includes(f))
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
  console.log(`Pak layout written to "${PAK_NAME}"`)
  ufs.writeFileSync(PAK_NAME, JSON.stringify(ordered, null, 2))
  
  game.ordered = ordered
  game.renamed = renamed
  game.condensed = condensed
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

async function repack(gs, project, outputProject) {
  if(!project) {
    project = PROJECT
  }
  var game
  if(!gs.graph) {
    game = await graphGame(gs, project, progress)
  } else {
    game = gs
  }
  if(!game || !game.ordered) {
    game = await groupAssets(gs, project)
  }
  if(!ufs.existsSync(outputProject)) ufs.mkdirSync(outputProject)
  var orderedKeys = Object.keys(game.ordered)
  for(var i = 0; i < orderedKeys.length; i++) {
    await progress([[1, i, orderedKeys.length, 'Packing ' + orderedKeys[i]]])
    var pak = game.ordered[orderedKeys[i]]
    var real = pak.filter(f => ufs.existsSync(f) && !ufs.statSync(f).isDirectory())
    var output = ufs.createWriteStream(path.join(outputProject, orderedKeys[i] + '.pk3'))
    await compressDirectory(real, output, project)
  }
  
  // generate a index.json the server can use for pk3 sorting based on map/game type
  var indexJson = path.join(outputProject, './index.json')
  console.log(`Writing index.json "${indexJson}", you should run
npm run start -- /assets/${path.basename(outputProject)} ${outputProject}
and
open ./build/release-*/ioq3ded +set fs_game ${path.basename(outputProject)}`)
  var remapped = {}
  
  ufs.writeFileSync(indexJson, JSON.stringify(remapped, null, 2))
}

// do the actual work specified in arguments
async function repackGames() {
  for(var i = 0; i < mountPoints.length; i++) {
    try {
      var outCombined = path.join(TEMP_DIR, path.basename(mountPoints[i]) + '-combined')
      var outConverted = path.join(TEMP_DIR, path.basename(mountPoints[i]) + '-combined-converted')
      var outRepacked = path.join(TEMP_DIR, path.basename(mountPoints[i]) + '-combined-converted-repacked')
      var gs
      if(typeof STEPS['source'] != 'undefined') {
        await progress([[0, 0, Object.keys(STEPS).length, STEPS['source']]])
        await progress([[1, 0, 2, 'Sourcing files']])
        await unpackPk3s(mountPoints[i], outCombined, progress)
        await progress([[0, 1, Object.keys(STEPS).length, STEPS['graph']]])
        await new Promise(resolve => setTimeout(resolve, 10))
        gs = await graphGame(0, outCombined, progress)
      } else {
        await progress([[0, 0, Object.keys(STEPS).length, STEPS['graph']]])
        gs = await graphGame(JSON.parse(ufs.readFileSync(TEMP_NAME).toString('utf-8')),
          outCombined, progress)
      }
      if(typeof STEPS['info'] != 'undefined') {
        await progress([
          [1, false],
          [0, typeof STEPS['source'] != 'undefined' ? 2 : 1, Object.keys(STEPS).length, STEPS['info']],
        ])
        await gameInfo(gs, outCombined)
      }
      
      await groupAssets(gs, outCombined)
      
      // transcoding and graphics magick
      if(typeof STEPS['convert'] != 'undefined') {
        await progress([
          [1, false],
          [0, typeof STEPS['source'] != 'undefined' ? 2 : 1, Object.keys(STEPS).length, STEPS['convert']],
        ])
        await convertGameFiles(gs, outCombined, outConverted, progress)
        console.log(`Updating Pak layout written to "${PAK_NAME}"`)
        ufs.writeFileSync(PAK_NAME, JSON.stringify(gs.ordered, null, 2))
      }
      if(typeof STEPS['repack'] != 'undefined') {    
        // repacking
        await progress([
          [1, false],
          [0, typeof STEPS['source'] != 'undefined' ? 3 : 2, Object.keys(STEPS).length, STEPS['repack']],
        ])
        await repack(gs, mountPoints[i], outRepacked)
      }
    } catch (e) {
      console.log(e)
    }
  }
  await progress(false)
}

repackGames().then(() => progress(false))

module.exports = {
  repackGames,
  repack,
  gameInfo,
  PAK_NAME,
  INFO_NAME,
  STEPS,
}
