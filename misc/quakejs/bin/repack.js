var fs = require('fs')
var {graphGames, graphModels} = gameLoader = require('../lib/asset.game.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'

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
  var game = graphGames()[0]
  // how many files are matched versus unknown?
  console.log(`Known files: ${game.directories.length}/${game.everything.length
    } - ${percent(game.directories.length, game.everything.length)}%`)
  
  // how many files a part of menu system?
  console.log(`Menu files: ${game.menu.length}/${game.everything.length
    } - ${game.menu.map(f => fs.statSync(f).size).reduce((sum, i) => sum + i, 0)} bytes`)
  
  // how many files are graphed versus unmatched or unknown?
  console.log(`Cgame files: ${game.uiqvm.length}/${game.everything.length
    } - ${percent(game.uiqvm.length, game.everything.length)}%`)
  
  // largest matches, more than 5 edges?
  
  
  // how many packs to create?
  
  // 1 - ui.qvm, menu system to get the game running, all scripts
  // 11 - cgame.qvm, qagame.qvm, in game feedback sounds not in menu
  // 12-19 - menu pk3s, hud, sprites
  // 20-29 - player models
  // 30-39 - powerups
  // 50-59 - weapons 1-9
  // 80-89 - map models
  // 90-99 - map textures
  return game
}

console.log(graphModels('/Users/briancullinan/planet_quake_data/baseq3-combined-converted'))
//gameInfo()
