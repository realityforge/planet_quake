var glob = require('glob')
var path = require('path')
var fs = require('fs')
var minimatch = require("minimatch")

var imageTypes = [
  '.png',
  '.jpg',
  '.jpeg',
  '.tga',
  '.gif',
  '.pcx',
  '.webp',
  '.hdr',
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
]

var fileTypes = [
  '.roq',
  '.cfg',
  '.qvm',
  '.bot',
  '.txt',
  '.bsp',
  '.aas',
  '.md3',
  '.md5',
  '.iqm',
  '.mdr',
  '.log',
  '.shader',
  '.crosshair',
  '.skin',
  '.pk3',
  '.font',
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
  'videos',
  'textures',
]

// minimatch/globs
var whitelist = {
  'baseq3': [
    '**/+(sarge|major)/**',
    '**/player/*',
    '**/player/footsteps/*',
    '**/weapons2/+(machinegun|gauntlet)/**',
    '**/weaphits/**',
    '**/scripts/*.shader',
  ],
  'missionpack': [
    '**/+(james|janet|sarge)/**',
    '**/player/*',
    '**/player/footsteps/*',
    '**/weapons2/+(machinegun|gauntlet)/**',
    '**/weaphits/**',
    '**/scripts/*.shader',
    '**/ui/assets/**',
  ],
  'baseoa': [
    '**/+(sarge|major)/**',
    '**/player/*',
    '**/player/footsteps/*',
    '**/weapons2/+(machinegun|gauntlet)/**',
    '**/weaphits/**',
    '**/scripts/*.shader',
  ],
  'baseq3r': [
    '**/+(player|players)/sidepipe/**',
    '**/+(player|players)/heads/doom*',
    '**/+(player|players)/plates/**',
    '**/+(player|players)/wheels/*cobra*',
    '**/player/*',
    '**/player/footsteps/*',
    '**/weaphits/**',
    '**/scripts/*.shader',
  ],
  'q3ut4': [
    '**/+(athena)/**',
    '**/player/*',
    '**/player/footsteps/*',
    '**/weapons2/+(handskins)/**',
    '**/weaphits/**',
    '**/scripts/*.shader',
  ],
  'threewave': []
}

function findTypes(types, project) {
  if(Array.isArray(types)) types = `*+(${types.join('|')})`
  if(fs.existsSync(project)) {
    return glob.sync(types, {nocase: true, cwd: project, matchBase: true})
      .map(f => path.join(project, f))
  } else if(Array.isArray(project)) {
    return project.filter(minimatch.filter(types, {nocase: true, matchBase: true}))
  } else {
    throw new Error(`Don't know what to do with ${project}`)
  }
}

module.exports = {
  knownDirs,
  fileTypes,
  sourceTypes,
  audioTypes,
  imageTypes,
  findTypes,
  whitelist,
  allTypes: [imageTypes, audioTypes, sourceTypes, fileTypes].flat(1)
}
