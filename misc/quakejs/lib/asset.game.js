var fs = require('fs')
var path = require('path')
var glob = require('glob')
var minimatch = require("minimatch")

var {graphQVM} = require('../lib/asset.qvm.js')
var md3 = require('../lib/asset.md3.js')
var bsp = require('../lib/asset.bsp.js')
var shaderLoader = require('../lib/asset.shader.js')
var skinLoader = require('../lib/asset.skin.js')
var {
  findTypes, knownDirs, fileTypes, sourceTypes,
  audioTypes, imageTypes, findTypes,
} = require('../bin/repack-whitelist.js')
var DirectedGraph = require('../lib/asset.graph.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'
var BASEQ3 = '/Users/briancullinan/planet_quake_data/quake3-baseq3'
var TEMP_NAME = path.join(__dirname, '../bin/previous-graph.json')

if(fs.existsSync(BASEQ3)) {
  var baseq3 = glob.sync('**/*', {cwd: BASEQ3})
    .map(f => path.join(BASEQ3, f).toLowerCase())
  fs.writeFileSync(path.join(__dirname, './baseq3-filelist.json'), JSON.stringify(baseq3, null, 2))
}

function graphMaps(project) {
  console.log('Looking for maps')
  var result = {}
  var maps = findTypes(['.bsp'], project || PROJECT)
  for(var i = 0; i < maps.length; i++) {
    var buffer = fs.readFileSync(maps[i])
    var map = bsp.load(buffer, { lumps: [bsp.LUMP.ENTITIES, bsp.LUMP.SHADERS] })
    result[maps[i]] = map
  }
  console.log(`Found ${Object.keys(result).length} maps`)
  return result
}

function graphModels(project) {
  console.log('Looking for models')
  var result = {}
  var models = findTypes(['.md5', '.md3'], project || PROJECT)
  for(var i = 0; i < models.length; i++) {
    var buffer = fs.readFileSync(models[i])
    var model = md3.load(buffer)
    result[models[i]] = model
  }
  var withSkins = Object.keys(result).filter(m => result[m].skins.length > 0)
  console.log(`Found ${Object.keys(result).length} models, ${withSkins.length} with skins`)
  return result
}

function graphShaders(project) {
  console.log('Looking for shaders')
  var result = {}
  var shaders = findTypes(['.shader'], project || PROJECT)
  for(var i = 0; i < shaders.length; i++) {
    var buffer = fs.readFileSync(shaders[i])
    var script = shaderLoader.load(buffer)
    result[shaders[i]] = script
  }
  console.log(`Found ${Object.keys(result).length} shaders`)
  return result
}

function graphSkins(project) {
  console.log('Looking for skins')
  var result = {}
  var skins = findTypes(['.skin'], project || PROJECT)
  for(var i = 0; i < skins.length; i++) {
    var buffer = fs.readFileSync(skins[i]).toString('utf-8')
    var skin = skinLoader.load(buffer)
    result[skins[i]] = skin
  }
  console.log(`Found ${Object.keys(result).length} skins`)
  return result
}

function graphGames(project) {
  if(!project) {
    project = PROJECT
  }
  var everything = glob.sync('**/*', {
    cwd: project,
    nodir: true
  }).map(f => path.join(project, f).toLowerCase())
  var known = glob.sync(`**/+(${knownDirs.join('|')})/**`, {
    cwd: project,
    nodir: true
  }).map(f => path.join(project, f).toLowerCase())
  
  var game = {
    maps: graphMaps(project),
    models: graphModels(project),
    shaders: graphShaders(project),
    skins: graphSkins(project),
    images: findTypes(imageTypes, project),
    audio: findTypes(audioTypes, project),
    sources: findTypes(sourceTypes, project),
    files: findTypes(fileTypes, project),
    qvms: graphQVM(0, project),
    directories: known,
    everything: everything,
  }
  
  // add all vertices
  var entityRefs = Object.keys(game.maps)
    .reduce((obj, k) => {
      obj[k] = game.maps[k].entities
        .reduce((arr, e) => {
          arr.push(e.noise, e.music, e.model2)
          return arr
        }, [])
        .filter(e => e && e.charAt(0) != '*')
        .concat([k.replace('.bsp', '.aas')])
      return obj
    }, {})
  var mapShaders = Object.keys(game.maps)
    .reduce((obj, k) => {
      obj[k] = game.maps[k].shaders
        .map(s => s.shaderName)
      return obj
    }, {})
  var modelShaders = Object.keys(game.models)
    .reduce((obj, k) => {
      obj[k] = game.models[k].surfaces
        .reduce((arr, s) => {
          arr.push(s.shaders)
          return arr
        }, [])
      return obj
    }, {})
  var scriptShaders = Object.keys(game.shaders)
    .reduce((obj, k) => {
      obj[k] = Object.keys(game.shaders[k])
      return obj
    }, {})
  var scriptTextures = Object.keys(game.shaders)
    .reduce((obj, s) => {
      var keys = Object.keys(game.shaders[s])
        .forEach(k => {
          if(typeof obj[k] === 'undefined') {
            obj[k] = []
          }
          if(game.shaders[s][k].stages) {
            obj[k].push(game.shaders[s][k].stages.map(stage => stage.maps).flat(1))
          }
          if(game.shaders[s][k].outerBox) {
            obj[k].push.apply(obj[k], game.shaders[s][k].outerBox)
          }
          if(game.shaders[s][k].innerBox) {
            obj[k].push.apply(obj[k], game.shaders[s][k].innerBox)
          }
        })
      return obj
    }, {})
  var skinShaders = Object.keys(game.skins)
    .reduce((obj, k) => {
      obj[k] = game.skins[k].surfaces
        .map(s => s.shaderName)
      return obj
    }, {})
  
  fs.writeFileSync(TEMP_NAME, JSON.stringify({
    entities: entityRefs,
    maps: mapShaders,
    models: modelShaders,
    scripts: scriptShaders,
    shaders: scriptTextures,
    skins: skinShaders,
    qvms: game.qvms,
    everything: everything,
  }, null, 2))
  /*
  var graph = new DirectedGraph()
  
  // add all edges to the graph
  var notfound = []
  var inbaseq3 = []
  var shadersPlusEverything = game.shaders.map(s => s.name)
    .concat(everything)
    .concat(baseq3)
  
  for(var i = 0; i < game.shaders.length; i++) {
    var v = graph.getVertex(game.shaders[i].name)
    for(var j = 0; j < game.shaders[i].textures.length; j++) {
      var s = addEdgeMinimatch(v, game.shaders[i].textures[j], everything, graph)
      if(s == -1) inbaseq3.push(game.shaders[i].textures[j])
      else if (s) graph.addEdge(v, s)
      else notfound.push(game.shaders[i].textures[j])
    }
  }
  
  // TODO: group by parent directories
  game.graph = graph
  game.notfound = notfound.filter((a, i, arr) => arr.indexOf(a) == i)
  game.baseq3 = inbaseq3.filter((a, i, arr) => arr.indexOf(a) == i)
  */
  
  return [game]
}

function addEdgeMinimatch(fromVertex, search, everything, graph) {
  search = search.replace(/\..*/, '') // remove extension
  var name = everything.filter(f => f.includes(search))[0] //minimatch.filter('**/' + search + '*'))[0]
  if(!name) {
    if(baseq3.filter(f => f.includes(search))[0]) { //minimatch.filter('**/' + search + '*'))[0]) {
      return -1
    }
    console.error('Resource not found ' + search)
    return null
  }
  return graph.getVertex(name)
}

module.exports = {
  graphMaps,
  graphModels,
  graphShaders,
  graphSkins,
  graphGames,
  load: graphGames
}
