var glob = require('glob')
var path = require('path')

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

var sourceTypes = [
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
  'scripts',
  'botfiles',
  'fonts',
  'gfx',
  'hud',
  'icons',
  'include',
  'menu',
  'models',
  'music',
  'powerups',  // powerup shaders
  'sprites',
  'sound',
  'ui',
  'maps',
  'textures',
]

function findTypes(types, project) {
  if(Array.isArray(types)) types = `**/*+(${types.join('|')})`
  return glob.sync(types, {cwd: project})
    .map(f => path.join(project, f).toLowerCase())
}

module.exports = {
  knownDirs,
  fileTypes,
  sourceTypes,
  audioTypes,
  imageTypes,
  findTypes,
}
