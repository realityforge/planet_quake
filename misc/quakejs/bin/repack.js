/*
Planned options:
--edges - number of connected edges to deserve it's own pk3, default is 2
--roots - insert yourself anywhere in the graph, show top connections from that asset
--info -i - only print info, don't actually do any converting


Basic steps:

Unpack
Graph
Convert
Repack


*/
var fs = require('fs')
var path = require('path')
var glob = require('glob')

var md3 = require('../lib/asset.md3.js')
var bsp = require('../lib/asset.bsp.js')

function graphMaps(project) { 
  if(!project) {
    project = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'
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
    project = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'
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
  name = sanitize(name)

	var self = this
	var shaderV = this._addAsset(name, game, ASSET.SHADER)
	var shader = qkfiles.shader.loadShader(buffer)

	shader.stages.forEach(function (stage) {
		stage.maps.forEach(function (map) {
			// ignore special textures (e.g. *white)
			if (map.charAt(0) === '*') {
				return
			}

			var stageV = self._getOrAddAsset(map, game, ASSET.TEXTURE)
			self._addReference(shaderV, stageV)
		})
	})

	// add inner / outer box maps for sky shaders
	shader.innerBox.forEach(function (map) {
		var mapV = self._getOrAddAsset(map, game, ASSET.TEXTURE)
		self._addReference(shaderV, mapV)
	})

	shader.outerBox.forEach(function (map) {
		var mapV = self._getOrAddAsset(map, game, ASSET.TEXTURE)
		self._addReference(shaderV, mapV)
	})
}

console.log(graphModels())
