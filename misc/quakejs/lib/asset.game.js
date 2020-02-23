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
  console.log(`Found ${result.length} maps`)
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
  var withSkins = result.filter(m => m.skins.length > 0)
  console.log(`Found ${result.length} models, ${withSkins.length} with skins`)
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
  console.log(`Found ${result.length} shaders`)
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
  console.log(`Found ${result.length} skins`)
  return result
}

function graphGames(project) {
  if(!project) {
    project = PROJECT
  }
  var everything = glob.sync('**/*', {cwd: project, nodir: true})
    .map(f => path.join(project, f).toLowerCase())
  var known = glob.sync(`**/+(${knownDirs.join('|')})/**`, {cwd: project})
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
    .reduce((obj, k) => game.maps[k].entities
    .reduce((arr, e) => arr.push.apply(arr, e.noise, e.music, e.model2), [])
    .filter(e => e && e.charAt(0) != '*')
    .concat([k.replace('.bsp', '.aas')]), {})
  var mapShaders = Object.keys(game.maps)
    .reduce((obj, k) => game.maps[k].shaders
    .map(s => s.shaderName), {})
  var modelShaders = Object.keys(game.models)
    .reduce((obj, k) => game.models[k].surfaces
    .reduce((arr, s) => arr.push.apply(arr, s.shaders), []), {})
  var scriptTextures = Object.keys(game.shaders)
    .reduce((obj, k) => game.shaders[k].stages
    .reduce((arr, s) => arr.push.apply(arr, s.maps), [])
    .concat(game.shaders[k].outerBox)
    .concat(game.shaders[k].innerBox), {})
  var skinShaders = Object.keys(game.skins)
    .reduce((obj, k) => game.skins[k].surfaces
    .map(s => s.shaderName), {})
  
  
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
