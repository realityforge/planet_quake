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
        ents.push(map.entities[e].music)
      if(map.entities[e].noise && map.entities[e].noise.charAt(0) != '*')
        ents.push(map.entities[e].noise)
      if(map.entities[e].model2 && map.entities[e].model2.charAt(0) != '*')
        ents.push(map.entities[e].model2)
    }
    var shaders = []
    for(var e = 0; e < map.shaders.length; e++) {
      if(map.shaders[e].shaderName && map.shaders[e].shaderName.charAt(0) != '*')
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
  console.log(`Found ${result.length} models`)
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
    .map(f => path.join(project, f))
}

function loadQVM(qvm, project) {
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
  return gameStrings
}

function minimatchWild(everything, qvmstrings) {
  var wildcards = qvmstrings
    .filter(f => f.includes('/'))
    .map(f => f.replace('%s', '*').replace('%c', '*'))
  return everything.reduce((arr, f) => {
    if(!arr.includes(f)) {
      if(wildcards.filter(w => minimatch(f, '**/*' + w + '*')).length > 0)
        arr.push(f)
    }
    return arr
  }, [])
}

function graphGames(project) {
  if(!project) {
    project = PROJECT
  }
  var everything = glob.sync('**/*', {cwd: project})
    .map(f => path.join(project, f))
  var menu = glob.sync('**/+(scripts|menu|gfx|sound|levelshots)/**', {cwd: project})
    .map(f => path.join(project, f))
  var uiwildcards = loadQVM('**/ui.qvm', project)
  var uiqvm = minimatchWild(everything, uiwildcards)
  var result = [{
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
  }]
  
  
  
  // TODO: group by parent directories
  
  
  return result
}

module.exports = {
  graphMaps,
  graphModels,
  graphShaders,
  graphSkins,
  graphGames,
  load: graphGames
}
