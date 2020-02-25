var path = require('path');
var fs = require('fs');
var glob = require('glob')
var exec = require('child_process').execSync;
var {whitelist, findTypes} = require('../bin/repack-whitelist.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-baseq3'

/*
GOAL: analize a ui.qvm and try to end up with something like this list

"  68:   64x  64 RGB       18kb gfx/misc/flare.tga\n",
  "  69:   64x  64 RGB       18kb gfx/misc/sun.tga\n",
  "  70:  256x 256 RGBA     256kb gfx/2d/bigchars.tga\n",
  "  71:  128x 128 RGBA      64kb gfx/misc/console01.tga\n",
  "  72:  128x 128 RGB       48kb gfx/misc/console02.tga\n",
  "  73:  256x 256 RGBA     256kb menu/art/font1_prop.tga\n",
  "  74:  256x 256 RGBA     256kb menu/art/font1_prop_glo.tga\n",
  "  75:  256x 256 RGBA     256kb menu/art/font2_prop.tga\n",
  "  76:   32x  32 RGBA       4kb menu/art/3_cursor2\n",
  "  77:   16x  16 RGBA    1024b  menu/art/switch_on\n",
  "  78:   16x  16 RGBA    1024b  menu/art/switch_off\n",
  "  79:  512x 512 RGB      768kb textures/sfx/logo512.tga\n",
  "  80:    8x   8 RGB      192b  gfx/colors/black.tga\n",
  "  81:  128x  16 RGBA       8kb menu/art/slider2\n",
  "  82:   16x  32 RGBA       2kb menu/art/sliderbutt_0\n",
  "  83:   16x  32 RGBA       2kb menu/art/sliderbutt_1\n",
  "  84:  256x 128 RGBA     128kb models/mapobjects/banner/q3banner02.tga\n",
  "  85:   64x 128 RGBA      32kb models/mapobjects/banner/q3banner02x.tga\n",
  "  86:  128x 128 RGB       48kb textures/sfx/firegorre2.tga\n",
  "  87:  128x 128 RGB       48kb textures/sfx/bolts.tga\n",
  "  88:  256x 256 RGBA     256kb menu/art/maps_select\n",
  "  89:  128x 129 RGBA      64kb menu/art/maps_selected\n",
  "  90:   16x 128 RGBA       8kb menu/art/narrow_0\n",
  "  91:   16x 128 RGBA       8kb menu/art/narrow_1\n",
  "  92:   64x  64 RGB       12kb menu/art/unknownmap\n",
  "  93:  128x 128 RGBA      64kb menu/art/level_complete1\n",
  "  94:  128x 128 RGBA      64kb menu/art/level_complete2\n",
  "  95:  128x 128 RGBA      64kb menu/art/level_complete3\n",
  "  96:  128x 128 RGBA      64kb menu/art/level_complete4\n",
  "  97:  128x 128 RGBA      64kb menu/art/level_complete5\n",
  "  98:  128x  64 RGBA      32kb menu/art/back_0\n",
  "  99:  128x  64 RGBA      32kb menu/art/back_1\n",
  " 100:  128x  64 RGBA      32kb menu/art/fight_0\n",
  " 101:  128x  64 RGBA      32kb menu/art/fight_1\n",
  " 102:  128x  64 RGBA      32kb menu/art/reset_0\n",
  " 103:  128x  64 RGBA      32kb menu/art/reset_1\n",
  " 104:  128x  64 RGBA      32kb menu/art/skirmish_0\n",
  " 105:  128x  64 RGBA      32kb menu/art/skirmish_1\n",
  " 106:   64x  64 RGBA      16kb menu/medals/medal_accuracy\n",
  " 107:   64x  64 RGBA      16kb menu/medals/medal_impressive\n",
  " 108:   64x  64 RGBA      16kb menu/medals/medal_excellent\n",
  " 109:   64x  64 RGBA      16kb menu/medals/medal_gauntlet\n",
  " 110:   64x  64 RGBA      16kb menu/medals/medal_frags\n",
  " 111:   64x  64 RGBA      16kb menu/medals/medal_victory\n",
  " 112:   64x  64 RGBA      16kb models/players/sarge/icon_default.tga\n",
  " 113:  143x 128 RGB       53kb levelshots/q3dm0.tga\n",
  " 114:   64x  64 RGBA      16kb models/players/crash/icon_default.tga\n",
  "`\n",
  
  
  "var shaderlist = `\n",
  "1     gen : gfx/2d/sunflare\n",
  "1   E gen : gfx/2d/bigchars\n",
  "1   E gen : white\n",
  "2   E gen : console\n",
  "1     gen : menu/art/font1_prop\n",
  "1     gen : menu/art/font1_prop_glo\n",
  "1     gen : menu/art/font2_prop\n",
  "1     gen : menu/art/3_cursor2\n",
  "1     gen : menu/art/switch_on\n",
  "1     gen : menu/art/switch_off\n",
  "1   E gen : menuback\n",
  "1   E gen : menubacknologo\n",
  "1     gen : menu/art/slider2\n",
  "1     gen : menu/art/sliderbutt_0\n",
  "1     gen : menu/art/sliderbutt_1\n",
  "3   E gen : models/mapobjects/banner/q3banner02\n",
  "3   E gen : models/mapobjects/banner/q3banner04\n",
  "1     gen : menu/art/maps_select\n",
  "1     gen : menu/art/maps_selected\n",
  "1     gen : menu/art/narrow_0\n",
  "1     gen : menu/art/narrow_1\n",
  "1     gen : menu/art/unknownmap\n",
  "1     gen : menu/art/level_complete1\n",
  "1     gen : menu/art/level_complete2\n",
  "1     gen : menu/art/level_complete3\n",
  "1     gen : menu/art/level_complete4\n",
  "1     gen : menu/art/level_complete5\n",
  "1     gen : menu/art/back_0\n",
  "1     gen : menu/art/back_1\n",
  "1     gen : menu/art/fight_0\n",
  "1     gen : menu/art/fight_1\n",
  "1     gen : menu/art/reset_0\n",
  "1     gen : menu/art/reset_1\n",
  "1     gen : menu/art/skirmish_0\n",
  "1     gen : menu/art/skirmish_1\n",
  "1     gen : menu/medals/medal_accuracy\n",
  "1     gen : menu/medals/medal_impressive\n",
  "1     gen : menu/medals/medal_excellent\n",
  "1     gen : menu/medals/medal_gauntlet\n",
  "1     gen : menu/medals/medal_frags\n",
  "1     gen : menu/medals/medal_victory\n",
  "1     gen : models/players/sarge/icon_default\n",
  "1     gen : levelshots/q3dm0\n",
  "1     gen : models/players/crash/icon_default\n",
  "`\n",
  "\n",


*/

function loadQVMEntities() {
  // TODO: try to make sense of the jump list that is defined by bg_misc.c
  //   by tracing the addresses from the strings back to the entity array
}

function loadQVMStrings(buffer, topdirs) {
  var qvmstrings = buffer
    .toString('utf-8').split('\n')
    .map(line => (/['""'](.*?)['""']/ig).exec(line))
    .filter(string => string)
    .map(match => match[1])
    // now for some filtering fun
    .filter(file => 
      // the shortest strings match from the file system is probably
      //   greater than 5 characters, because even vm is actually vm%c%s
      file.length > 5
      // should include a slash or a %c
      && (file.includes('%') || file.includes('/'))
      // colons aren't allowed in filenames
      && !file.includes(':')
    )
    // assuming a single character is the path seperator,
    //   TODO: could be a number or something, derive from QVM function call with ascii character nearby?
    //   TODO: might need something for %i matching lightmaps names or animation frames
    .map(f => f.replace(/%[0-9\-\.]*[sdi]/ig, '*')
               .replace(/%[c]/ig, '/'))
    // make sure there is only one occurrence
    .filter((a, i, arr) => arr.indexOf(a) === i)
    // it also has to include one of the top directories
    //   shaders in QUAKED skip the textures folder
    //   this will be faster than minimatch lookups
    .filter(file => topdirs.filter(dir => file.includes(dir)).length > 0)
  return qvmstrings
}

function graphQVM(qvm, project) {
  console.log('Looking for QVMs')
  var result = {}
  var qvms = findTypes(qvm || ['.qvm'], project || PROJECT)
  var topdirs = glob.sync('**/', {cwd: project || PROJECT})
    .map(dir => path.basename(dir))
  for(var i = 0; i < qvms.length; i++) {
    var disassembly = qvms[i].replace(/\.qvm/i, '.dis')
    if(!fs.existsSync(disassembly)) continue
    var buffer = fs.readFileSync(disassembly)
    var qvmstrings = loadQVMStrings(buffer, topdirs)
      .concat(['botfiles/**', '*.cfg', disassembly])
    result[qvms[i]] = qvmstrings
  }
  console.log(`Found ${qvms.length} QVMs and ${Object.values(result).reduce((t, o) => t += o.length, 0)} strings`)
  return result
}

module.exports = {
  loadQVMEntities: loadQVMEntities,
  loadQVMStrings: loadQVMStrings,
  graphQVM: graphQVM,
}
