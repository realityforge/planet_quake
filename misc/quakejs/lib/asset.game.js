var fs = require('fs')
var path = require('path')
var glob = require('glob')
var exec = require('child_process').execSync;
var minimatch = require("minimatch")

var md3 = require('../lib/asset.md3.js')
var bsp = require('../lib/asset.bsp.js')
var shaderLoader = require('../lib/asset.shader.js')
var skinLoader = require('../lib/asset.skin.js')
var whitelist = require('../bin/repack-whitelist.js')
var DirectedGraph = require('../lib/asset.graph.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'

function graphMaps(project) {
  console.log('Looking for maps')
  if(!project) {
    project = PROJECT
  }
  var result = []
  var maps = findTypes(['.bsp'], project)
  for(var i = 0; i < maps.length; i++) {
    var buffer = fs.readFileSync(maps[i])
    var map = bsp.load(buffer, { lumps: [bsp.LUMP.ENTITIES, bsp.LUMP.SHADERS] })
    var weapons = []
    var items = []
    var ents = []
    for(var e = 0; e < map.entities.length; e++) {
      if(map.entities[e].classname.includes('item_'))
        items.push(map.entities[e].classname)
      if(map.entities[e].classname.includes('weapon_'))
        weapons.push(map.entities[e].classname)
      if(map.entities[e].music)
        ents.push(map.entities[e].music.toLowerCase())
      if(map.entities[e].noise && map.entities[e].noise.charAt(0) != '*')
        ents.push(map.entities[e].noise.toLowerCase())
      if(map.entities[e].model2 && map.entities[e].model2.charAt(0) != '*')
        ents.push(map.entities[e].model2.toLowerCase())
    }
    var shaders = []
    for(var e = 0; e < map.shaders.length; e++) {
      if(map.shaders[e].shaderName && map.shaders[e].shaderName.charAt(0) != '*'
        && map.shaders[e].shaderName != 'noshader')
        shaders.push(map.shaders[e].shaderName)
    }
    result.push({
      name: maps[i],
      items: items,
      weapons: weapons,
      ents: ents,
      shaders: shaders
    })
  }
  console.log(`Found ${result.length} maps`)
  return result
}

function graphModels(project) {
  console.log('Looking for models')
  if(!project) {
    project = PROJECT
  }
  var result = []
  var models = findTypes(['.md5', '.md3'], project)
  for(var i = 0; i < models.length; i++) {
    var buffer = fs.readFileSync(models[i])
    var model = md3.load(buffer)
    var shaders = []
    for (var s = 0; s < model.surfaces.length; s++) {
  		for (var sh = 0; sh < model.surfaces[s].shaders.length; sh++) {
        if(model.surfaces[s].shaders[sh])
          shaders.push(model.surfaces[s].shaders[sh])
      }
    }
    result.push({
      name: models[i],
      skins: model.skins,
      shaders: shaders
    })
  }
  var withSkins = result.filter(m => m.skins.length > 0)
  console.log(`Found ${result.length} models, ${withSkins.length} with skins`)
  return result
}

function graphShaders(project) {
  console.log('Looking for shaders')
  if(!project) {
    project = PROJECT
  }
  var result = []
  var shaders = findTypes(['.shader'], project)
  for(var i = 0; i < shaders.length; i++) {
    var buffer = fs.readFileSync(shaders[i])
    var script = shaderLoader.load(buffer)
    var keys = Object.keys(script)
    for(var j = 0; j < keys.length; j++) {
      var shader = script[keys[j]]
      var textures = []
      for (var s = 0; s < shader.stages.length; s++) {
        for (var sh = 0; sh < shader.stages[s].maps.length; sh++) {
          textures.push(shader.stages[s].maps[sh])
        }
      }
      for (var s = 0; s < shader.innerBox.length; s++) {
        textures.push(shader.innerBox[s])
      }
      for (var s = 0; s < shader.outerBox.length; s++) {
        textures.push(shader.outerBox[s])
      }
      result.push({
        name: shader.name,
        textures: textures
      })
    }
  }
  console.log(`Found ${result.length} shaders`)
  return result
}

function graphSkins(project) {
  console.log('Looking for skins')
  if(!project) {
    project = PROJECT
  }
  var result = []
  var skins = findTypes(['.skin'], project)
  for(var i = 0; i < skins.length; i++) {
    var buffer = fs.readFileSync(skins[i]).toString('utf-8')
    var skin = skinLoader.load(buffer)
    var shaders = []
    for (var s = 0; s < skin.surfaces.length; s++) {
      shaders.push(skin.surfaces[s].shaderName)
    }
    result.push({
      name: skins[i],
      shaders: shaders
    })
  }
  console.log(`Found ${result.length} skins`)
  return result
}

function findTypes(types, project) {
  return glob.sync(`**/*+(${types.join('|')})`, {cwd: project})
    .map(f => path.join(project, f).toLowerCase())
}

function loadQVM(qvm, project) {
  console.log('Looking for QVM strings')
  var cgame = path.join(project, glob.sync(qvm, {cwd: project})[0])
  var disassembler = path.resolve(path.join(__dirname, '../lib/qvmdisas/'))
  var result
  try {
    exec(`echo "header" | ./QVMDisas "${cgame}"`, {cwd: disassembler})
  } catch (e) {
    result = e.stdout.toString()
  }
  var start = parseInt((/Data Offset: (0x.*)/i).exec(result)[1])
  var length = parseInt((/Data Length: (0x.*)/i).exec(result)[1])
  var buffer = fs.readFileSync(cgame)
  var gameStrings = buffer.slice(start + length)
    .toString('utf-8').split('\0')
  console.log(`Found ${gameStrings.length} QVM strings`)
  return gameStrings
}

function minimatchWild(everything, qvmstrings) {
  console.log('Looking for matching files from QVM strings')
  var wildcards = qvmstrings
    .filter(f => f.includes('/') && f.length > 5)
    .map(f => f.replace(/%[0-9sdci\-\.]+/ig, '*'))
  var result = everything.reduce((arr, f) => {
    if(!arr.includes(f)) {
      if(wildcards.filter(w => minimatch(f, '**/' + w + '*')).length > 0)
        arr.push(f)
    }
    return arr
  }, [])
  console.log(`Found ${result.length} matching files from QVM strings`)
  return result
}

function graphGames(project) {
  if(!project) {
    project = PROJECT
  }
  var everything = glob.sync('**/*', {cwd: project})
    .map(f => path.join(project, f).toLowerCase())
  var menu = glob.sync('**/+(scripts|menu|gfx|sound|levelshots)/**', {cwd: project})
    .map(f => path.join(project, f).toLowerCase())
  var uiwildcards = loadQVM('**/ui.qvm', project)
  var uiqvm = minimatchWild(everything, uiwildcards)
  var game = {
    maps: graphMaps(project),
    models: graphModels(project),
    shaders: graphShaders(project),
    skins: graphSkins(project),
    images: findTypes(whitelist.imageTypes, project),
    audio: findTypes(whitelist.audioTypes, project),
    sources: findTypes(whitelist.sourceTypes, project),
    files: findTypes(whitelist.fileTypes, project),
    directories: glob.sync(`**/+(${whitelist.knownDirs.join('|')})/**`, {cwd: project}),
    everything: everything,
    menu: menu,
    uiqvm: uiqvm,
  }
  
  // add all vertices
  var graph = new DirectedGraph()
  for(var i = 0; i < game.maps.length; i++) {
    graph.addVertex(game.maps[i].name, game.maps[i])
  }
  for(var i = 0; i < game.models.length; i++) {
    graph.addVertex(game.models[i].name, game.models[i])
  }
  for(var i = 0; i < game.shaders.length; i++) {
    graph.addVertex(game.shaders[i].name, game.shaders[i])
  }
  for(var i = 0; i < game.skins.length; i++) {
    graph.addVertex(game.skins[i].name, {name: game.skins[i]})
  }
  for(var i = 0; i < game.images.length; i++) {
    graph.addVertex(game.images[i], {name: game.images[i]})
  }
  for(var i = 0; i < game.audio.length; i++) {
    graph.addVertex(game.audio[i], {name: game.audio[i]})
  }
  for(var i = 0; i < game.files.length; i++) {
    graph.addVertex(game.files[i], {name: game.files[i]})
  }
  
  // add all edges to the graph
  var notfound = []
  var shadersPlusEverything = game.shaders.map(s => s.name).concat(everything)
  
  for(var i = 0; i < game.maps.length; i++) {
    var v = graph.getVertex(game.maps[i].name)
    for(var j = 0; j < game.maps[i].ents.length; j++) {
      var s = addEdgeMinimatch(v, game.maps[i].ents[j], everything, graph)
      if(s) graph.addEdge(v, s)
      else notfound.push(game.maps[i].ents[j])
    }
    for(var j = 0; j < game.maps[i].shaders.length; j++) {
      var s = addEdgeMinimatch(v, game.maps[i].shaders[j], shadersPlusEverything, graph)
      if(s) graph.addEdge(v, s)
      else notfound.push(game.maps[i].shaders[j])
    }
  }
  
  for(var i = 0; i < game.models.length; i++) {
    var v = graph.getVertex(game.models[i].name)
    for(var j = 0; j < game.models[i].shaders.length; j++) {
      var s = addEdgeMinimatch(v, game.models[i].shaders[j], shadersPlusEverything, graph)
      if(s) graph.addEdge(v, s)
      else notfound.push(game.models[i].shaders[j])
    }
  }
  
  for(var i = 0; i < game.skins.length; i++) {
    var v = graph.getVertex(game.skins[i].name)
    for(var j = 0; j < game.skins[i].shaders.length; j++) {
      var s = addEdgeMinimatch(v, game.skins[i].shaders[j], shadersPlusEverything, graph)
      if(s) graph.addEdge(v, s)
      else notfound.push(game.skins[i].shaders[j])
    }
  }
  
  for(var i = 0; i < game.shaders.length; i++) {
    var v = graph.getVertex(game.shaders[i].name)
    for(var j = 0; j < game.shaders[i].textures.length; j++) {
      var s = addEdgeMinimatch(v, game.shaders[i].textures[j], shadersPlusEverything, graph)
      if(s) graph.addEdge(v, s)
      else notfound.push(game.shaders[i].textures[j])
    }
  }
  
  // TODO: group by parent directories
  game.graph = graph
  game.notfound = notfound
  
  return [game]
}

function addEdgeMinimatch(fromVertex, search, everything, graph) {
  var notfound = []
  var name = everything.filter(minimatch.filter('**/' + search + '*'))[0]
  if(!name) {
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
