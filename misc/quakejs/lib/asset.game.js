var fs = require('fs')
var path = require('path')
var glob = require('glob')
var minimatch = require("minimatch")

var {graphQVM, getGameAssets} = require('../lib/asset.qvm.js')
var md3 = require('../lib/asset.md3.js')
var bsp = require('../lib/asset.bsp.js')
var shaderLoader = require('../lib/asset.shader.js')
var skinLoader = require('../lib/asset.skin.js')
var {
  findTypes, fileTypes, sourceTypes,
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

function loadGame(project) {
  if(!project) {
    project = PROJECT
  }
  var everything = glob.sync('**/*', {
    cwd: project,
    nodir: true
  }).map(f => path.join(project, f))
  
  var game = {
    maps: graphMaps(project),
    models: graphModels(project),
    shaders: graphShaders(project),
    skins: graphSkins(project),
    qvms: graphQVM(0, project),
    everything: everything,
  }
  // TODO: accept an entities definition to match with QVM
  // use some known things about QVMs to group files together first
  var cgame = Object.values(game.qvms).flat(1)
    .filter(k => k.match(/cgame\.dis/i))[0]
  var entities = getGameAssets(cgame)
  
  // add all vertices
  var entityRefs = Object.keys(game.maps)
    .reduce((obj, k) => {
      obj[k] = game.maps[k].entities
        .reduce((arr, e) => {
          arr.push.apply(arr, [
            e.noise,
            (e.music || '').replace(/(\.wav)\s+/ig, '$1{SPLIT}').split(/\{SPLIT\}/ig),
            e.model,
            e.model2
          ].flat(1))
          return arr
        }, [])
        .filter(e => e && e.charAt(0) != '*')
        .concat([k.replace('.bsp', '.aas')])
        .concat(game.maps[k].entities.map(e => e.classname))
        .filter((e, i, arr) => arr.indexOf(e) === i)
      obj[k].sort()
      return obj
    }, {})
  var mapShaders = Object.keys(game.maps)
    .reduce((obj, k) => {
      obj[k] = game.maps[k].shaders
        .map(s => s.shaderName)
      obj[k].sort()
      return obj
    }, {})
  var modelShaders = Object.keys(game.models)
    .reduce((obj, k) => {
      obj[k] = game.models[k].surfaces
        .map(s => s.shaders).flat(1)
        .filter(s => s)
      obj[k].sort()
      return obj
    }, {})
  var scriptShaders = Object.keys(game.shaders)
    .reduce((obj, k) => {
      obj[k] = Object.keys(game.shaders[k])
      obj[k].sort()
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
            obj[k].push.apply(obj[k], game.shaders[s][k].stages.map(stage => stage.maps).flat(1))
          }
          if(game.shaders[s][k].outerBox) {
            obj[k].push.apply(obj[k], game.shaders[s][k].outerBox)
          }
          if(game.shaders[s][k].innerBox) {
            obj[k].push.apply(obj[k], game.shaders[s][k].innerBox)
          }
          obj[k].sort()
        })
      return obj
    }, {})
  var skinShaders = Object.keys(game.skins)
    .reduce((obj, k) => {
      obj[k] = game.skins[k].surfaces
        .map(s => s.shaderName)
      obj[k].sort()
      return obj
    }, {})
  var qvmFiles = Object.keys(game.qvms)
    .reduce((obj, k) => {
      console.log(`Searching for QVM files ${path.basename(k)} from ${game.qvms[k].length} strings`)
      var wildcards = game.qvms[k].filter(s => s.includes('*'))
      obj[k] = wildcards
        .map(w => w.replace(/\\/ig, '/'))
        .map(w => everything.filter(minimatch.filter('**/' + w)))
        .flat(1)
        .concat(game.qvms[k])
        .filter((e, i, arr) => arr.indexOf(e) === i)
      obj[k].sort()
      return obj
    }, {})

  var gameState = {
    entities: entities,
    mapEntities: entityRefs,
    maps: mapShaders,
    models: modelShaders,
    scripts: scriptShaders,
    shaders: scriptTextures,
    skins: skinShaders,
    qvms: qvmFiles,
    everything: everything,
  }
  fs.writeFileSync(TEMP_NAME, JSON.stringify(gameState, null, 2))
  
  return Object.assign(game, gameState)
}

function graphGame(gs, project) {
  if(!project) {
    project = PROJECT
  }
  if(!gs) {
    gs = loadGame(project)
  }
  var graph = new DirectedGraph()
  
  // add all edges to the graph
  var notfound = []
  var inbaseq3 = []
  var everything = gs.everything.map(f => f.toLowerCase())
  var allTypes = [imageTypes, audioTypes, sourceTypes, fileTypes].flat(1)
  var unknownTypes = gs.everything.map(f => path.extname(f).toLowerCase())
    .filter((t, i, arr) => arr.indexOf(t) === i)
    .filter(t => !allTypes.includes(t))

  // add all the vertices which are the keys of the variables above
  var vertices = []
    .concat(Object.values(gs.mapEntities).flat(1))
    .concat(Object.keys(gs.qvms))
    .concat(Object.keys(gs.maps))
    .concat(Object.keys(gs.scripts))
    .concat(Object.keys(gs.models))
    .concat(Object.keys(gs.skins))
    .concat(Object.values(gs.shaders).flat(1))
    .concat(Object.keys(gs.qvms))
    .concat(Object.values(gs.qvms).flat(1)) // can be filename or shaders
    .filter((v, i, arr) => v && arr.indexOf(v) == i)
    
  console.log(`Graphing ${vertices.length} vertices`)
  
  var fileLookups = {}
  for(var i = 0; i < vertices.length; i++) {
    // everything in vertices should match a file
    if(!fs.existsSync(vertices[i])) {
      var index = searchMinimatch(vertices[i], everything)
      if(index == -1) inbaseq3.push(vertices[i])
      else if (index !== null) {
        fileLookups[vertices[i]] = graph.getVertex(gs.everything[index])
          || graph.addVertex(gs.everything[index], {
          name: gs.everything[index]
        })
      }
      else notfound.push(vertices[i])
    } else {
      fileLookups[vertices[i]] = graph.getVertex(vertices[i])
        || graph.addVertex(vertices[i], {
        name: vertices[i]
      })
    }
  }

  // lookup all shaders
  var everyShaderName = Object.values(gs.scripts)
    .flat(1)
    .filter((s, i, arr) => arr.indexOf(s) === i)
  var allShaders = []
    .concat(Object.values(gs.entities).flat(1)) // match with shaders or files so icons match up
    .concat(Object.values(gs.maps).flat(1))
    .concat(Object.values(gs.models).flat(1))
    .concat(Object.values(gs.scripts).flat(1)) // obviously all these should match the list above
    .concat(Object.values(gs.skins).flat(1))
    .concat(Object.values(gs.qvms).flat(1)) // can be filename or shaders
    .filter((v, i, arr) => v && arr.indexOf(v) == i)
    
  console.log(`Graphing ${allShaders.length} shaders`)
    
  var shaderLookups = {}
  for(var i = 0; i < allShaders.length; i++) {
    // matches without extension
    //   which is what we want because mods override shaders
    var index = everyShaderName.indexOf(allShaders[i])
    if(index > -1) {
      shaderLookups[allShaders[i]] = graph.getVertex(everyShaderName[index])
        || graph.addVertex(everyShaderName[index], {
          name: everyShaderName[index]
        })
    } else {
      // try to match a filename directly
      index = searchMinimatch(allShaders[i], everything)
      if(index == -1) inbaseq3.push(allShaders[i])
      else if(index !== null) {
        shaderLookups[allShaders[i]] = graph.getVertex(gs.everything[index])
          || graph.addVertex(gs.everything[index], {
            name: gs.everything[index]
          })
      }
      else notfound.push(allShaders[i])
    }
  }
  
  // link all the vertices and follow all shaders through to their files
  Object.keys(gs.entities).forEach(k => {
    var entityEdges = gs.entities[k]
      .filter(e => typeof fileLookups[e] != 'undefined'
        || typeof shaderLookups[e] != 'undefined')
    if(entityEdges.length > 0) {
      fileLookups[k] = graph.addVertex(k, {name: k})
      entityEdges.forEach(e => graph.addEdge(fileLookups[k], fileLookups[e] || shaderLookups[e]))
    }
  })
  Object.keys(gs.mapEntities).forEach(k => {
    gs.mapEntities[k].forEach(e => {
      if(typeof fileLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), fileLookups[e])
    })
  })
  Object.keys(gs.shaders).forEach(k => {
    gs.shaders[k].forEach(e => {
      if(typeof fileLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), fileLookups[e])
    })
  })
  Object.keys(gs.maps).forEach(k => {
    gs.maps[k].forEach(e => {
      if(typeof shaderLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), shaderLookups[e])
    })
  })
  Object.keys(gs.models).forEach(k => {
    gs.models[k].forEach(e => {
      if(typeof shaderLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), shaderLookups[e])
    })
  })
  Object.keys(gs.skins).forEach(k => {
    gs.skins[k].forEach(e => {
      if(typeof shaderLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), shaderLookups[e])
    })
  })
  Object.keys(gs.qvms).forEach(k => {
    gs.qvms[k].forEach(e => {
      if(typeof fileLookups[e] == 'undefined'
        && typeof shaderLookups[e] == 'undefined') return
      graph.addEdge(graph.getVertex(k), fileLookups[e] || shaderLookups[e])
    })
  })
  
  // TODO: add arenas, configs, bot scripts, defi
  
  gs.graph = graph
  gs.notfound = notfound
  gs.baseq3 = inbaseq3
  
  return gs
}

function searchMinimatch(search, everything) {
  var lookup = search
    .replace(/\/\//ig, '/')
    .replace(/\\/g, '/')
    .replace(/\.[^\.]*$/, '') // remove extension
    .toLowerCase()
  var name = everything.filter(f => f.includes(lookup)) //minimatch.filter('**/' + search + '*'))[0]
  if(!name[0]) {
    if(baseq3.filter(f => f.includes(lookup))[0]) { //minimatch.filter('**/' + search + '*'))[0]) {
      return -1
    }
    return null
  } else if (name.length > 1) {
    var type = [imageTypes, audioTypes, sourceTypes, fileTypes]
      .filter(type => type.includes(path.extname(search).toLowerCase()))[0]
    if(path.extname(search) && !type) throw new Error('File type not found '  + search)
    else if (!type) type = imageTypes // assuming its a shading looking for an image
    name = everything.filter(f => type.filter(t => f.includes(lookup + t)).length > 0)
    if(name.length == 0 || name.length > 1) {
      return null
    }
  }
  return everything.indexOf(name[0])
}

module.exports = {
  graphMaps,
  graphModels,
  graphShaders,
  graphSkins,
  graphGame,
  load: graphGame
}
