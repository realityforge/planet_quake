var fs = require('fs')
var path = require('path')
var glob = require('glob')

var md3 = require('../lib/asset.md3.js')
var bsp = require('../lib/asset.bsp.js')
var shaderLoader = require('../lib/asset.shader.js')
var skinLoader = require('../lib/asset.skin.js')
/*
Planned options:
--edges - number of connected edges to deserve it's own pk3, default is 3
--roots - insert yourself anywhere in the graph, show top connections from that asset
--info -i - only print info, don't actually do any converting
--convert - options to pass to image magick, make sure to put these last
--transcode - options to pass to opus/ogg vorbis, make sure to put these last

Basic steps:

Unpack
Graph
Convert
Repack

*/
var PROJECT = '/Users/briancullinan/planet_quake_data/baseq3-combined-converted'
var mountPoints = []

function graphMaps(project) { 
  if(!project) {
    project = PROJECT
  }
  var result = []
  var maps = glob.sync('**/*.bsp', {cwd: project})
    .map(f => path.join(project, f))
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
  return result
}

function graphModels(project) {
  if(!project) {
    project = PROJECT
  }
  var result = []
  var models = glob.sync('**/*+(.md5|.md3)', {cwd: project})
    .map(f => path.join(project, f))
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
  return result
}

function graphShaders(project) {
  if(!project) {
    project = PROJECT
  }
  var result = []
  var shaders = glob.sync('**/*.shader', {cwd: project})
    .map(f => path.join(project, f))
  for(var i = 0; i < shaders.length; i++) {
    var buffer = fs.readFileSync(shaders[i])
    var shader = shaderLoader.load(buffer)
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
      name: shaders[i],
      textures: textures
    })
  }
  return result
}

function graphSkins(project) {
  if(!project) {
    project = PROJECT
  }
  var result = []
  var skins = glob.sync('**/*.skin', {cwd: project})
    .map(f => path.join(project, f))
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
  return result
}

console.log(graphSkins())
