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
  '.ogg',
]

var sourceTypes = [
  '.c', // these can be compiled in game to run bot AI
  '.h',
  '.map',
  '.scc',
  '.dis',
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
  '.config',
  '.menu',
  '.defi', // CPMA game mode definition
  '.arena', // map based game mode definition
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
    .map(f => path.join(project, f))
}

module.exports = {
  knownDirs,
  fileTypes,
  sourceTypes,
  audioTypes,
  imageTypes,
  findTypes,
}
