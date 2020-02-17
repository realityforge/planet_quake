var {graphShaders} = gameLoader = require('../lib/asset.game.js')

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

var imageTypes = [
  '.png',
  '.jpg',
  '.jpeg',
  '.tga',
  '.gif',
  '.pcx',
  '.webp',
]

var audioTypes = [
  '.opus',
  '.wav',
  '.mp3',
]

var sourceFiles = [
  '.map',
  '.scc',
]

var fileTypes = [
  '.cfg',
  '.qvm',
  '.bot',
  '.txt',
  '.bsp',
  '.aas',
  '.md3',
  '.md5',
  '.shader',
  '.skin',
  '.pk3',
  '.c', // these can be compiled in game to run bot AI
  '.h',
  '.config',
  '.menu'
]

var knownDirs = [
  'scripts/',
  'botfiles/',
  'fonts/',
  'gfx/',
  'hud/',
  'icons/',
  'include/',
  'menu/',
  'models/',
  'music/',
  'powerups/',  // powerup shaders
  'sprites/',
  'sound/',
  'ui/',
  'maps/',
  'textures/',
]

console.log(graphShaders())
