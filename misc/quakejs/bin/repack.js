var fs = require('fs')
var path = require('path')
var {graphGames, graphModels, graphMaps} = gameLoader = require('../lib/asset.game.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/baseq3-combined-converted'

var edges = 3
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
var mountPoints = []

function percent(a, b) {
  return Math.round(a/b*1000) / 10
}

function gameInfo(project) {
  var game = graphGames(project)[0]
  // how many files are matched versus unknown?
  console.log(`Known files: ${game.files.length}/${game.everything.length
    } - ${percent(game.files.length, game.everything.length)}%`)
  console.log(`Known directories: ${game.directories.length}/${game.everything.length
    } - ${percent(game.directories.length, game.everything.length)}%`)
  
  // how many files a part of menu system?
  console.log(`Menu files: ${game.menu.length}/${game.everything.length
    } - ${game.menu.map(f => fs.statSync(f).size).reduce((sum, i) => sum + i, 0)} bytes`)
  
  // how many files are graphed versus unmatched or unknown?
  console.log(`Cgame files: ${game.uiqvm.length}/${game.everything.length
    } - ${percent(game.uiqvm.length, game.everything.length)}%`)
  
  console.log(`Not found: ${game.notfound.length}`)
  
  console.log(`Files in baseq3: ${game.baseq3.length}`)
  
  // largest matches, more than 5 edges?
  var vertices = game.graph.getVertices()
  vertices.sort((a, b) => b.inEdges.length - a.inEdges.length)
  console.log('Most used assets:', vertices.slice(0, 100).map(v => v.inEdges.length + ' - ' + v.id))
  
  vertices.sort((a, b) => a.inEdges.length - b.inEdges.length)
  console.log('Least used assets:', vertices
    .filter(v => v.inEdges.length > 0) // && v.inEdges[0].inVertex.id != v.id)
    .slice(0, 100)
    .map(v => v.inEdges.length + ' - ' + v.id + ' - ' + v.inEdges.map(e => e.outVertex.id).join(', ')))

  var unused = vertices
    .filter(v => v.inEdges.length == 0 && !v.id.includes('.bsp') && !v.id.includes('.md3'))
    
  console.log('Unused assets:', unused.length, unused.slice(0, 100).map(v => v.id))
  
  // how many packs to create?
  var filesOverLimit = vertices
    .filter(v => v.inEdges.length > edges)
    .map(v => v.id)
  
  // for each md3, bsp, group by parent directory
  var roots = vertices.filter(v => v.id.includes('.md3') || v.id.includes('.bsp'))
  var grouped = {}
  for(var i = 0; i < roots.length; i++) {
    var parent = roots[i].id.includes('.bsp')
      ? roots[i].id
      : path.dirname(roots[i].id)
    if(typeof grouped[parent] == 'undefined') {
      grouped[parent] = []
    }
    grouped[parent].push(roots[i].id)
    grouped[parent].push.apply(grouped[parent], roots[i].outEdges.map(e => e.inVertex.id))
    var textures = roots[i].outEdges.map(e => e.inVertex.outEdges.map(e => e.inVertex.id))
    grouped[parent].push.apply(grouped[parent], [].concat.apply([], textures))
  }
  
  // TODO: make sure we have everything
  
  
  var playerCount = 0, powerupCount = 0, modelCount = 0, 
    weaponCount = 0, mapModelCount = 0, mapCount = 0,
    itemCount = 0, otherCount = 0
  var paks = Object.keys(grouped).map(parent => {
    // 1 - ui.qvm, menu system to get the game running, all scripts
    // 11 - cgame.qvm, qagame.qvm, in game feedback sounds not in menu
    // 12-19 - menu pk3s, hud, sprites
    // 20-29 - player models
    // 30-39 - powerups
    // 50-59 - weapons 1-9
    // 80-89 - map models
    // 90-99 - map textures
    var pre = '0'
    if(parent.includes('maps')) {
      mapCount++
      pre = getSequenceNumeral(9, mapCount)
    } else if(parent.includes('player')) {
      playerCount++
      pre = getSequenceNumeral(2, playerCount)
    } else if(parent.includes('powerup')) {
      powerupCount++
      pre = getSequenceNumeral(3, powerupCount)
    } else if(parent.includes('weapon')) {
      weaponCount++
      pre = getSequenceNumeral(5, weaponCount)
    } else if(parent.includes('mapobject')) {
      mapModelCount++
      pre = getSequenceNumeral(8, mapModelCount)
    } else if(parent.includes('item')) {
      itemCount++
      pre = getSequenceNumeral(4, itemCount)
    } else if(parent.includes('model')) {
      modelCount++
      pre = getSequenceNumeral(6, modelCount)
    } else {
      otherCount++
      pre = getSequenceNumeral(7, otherCount)
    }
    return 'pak' + pre + path.basename(parent).replace(/\..*/, '')
  })
  
  paks.sort()

  console.log('Proposed layout:', paks)
  
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

//graphModels('/Users/briancullinan/planet_quake_data/baseq3-combined-converted')
//graphMaps(PROJECT)
gameInfo(PROJECT)
