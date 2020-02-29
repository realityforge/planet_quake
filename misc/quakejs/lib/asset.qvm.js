var path = require('path');
var fs = require('fs');
var glob = require('glob')
var minimatch = require('minimatch')
var {whitelist, findTypes} = require('../bin/repack-whitelist.js')

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-baseq3'

var MATCH_JMPLIST = /^((0x[0-9a-f]{8})\s+[0-9a-f]{2} [0-9a-f]{2} [0-9a-f]{2} [0-9a-f]{2}\s+(0x[0-9a-f]{1,8}))\s*$/i
var MATCH_STRINGS = /^((0x[0-9a-f]{8})\s+"(.*?)"\s*$)/i
var MATCH_ENTS = /(ammo_|item_|team_|weapon_|holdable_)/i
var SIZEOF_GITEM = 13*4

function loadQVMEntities() {
  // TODO: try to make sense of the jump list that is defined by bg_misc.c
  //   by tracing the addresses from the strings back to the entity array
}

function loadQVMStrings(buffer, topdirs) {
  var qvmstrings = buffer
    .toString('utf-8').split('\n')
    .map(line => (/['""'](.*?)['""']/ig).exec(line))
    .filter(string => string)
    // assuming a single character is the path seperator,
    //   TODO: could be a number or something, derive from QVM function call with ascii character nearby?
    //   TODO: might need something for %i matching lightmaps names or animation frames
    .map(f => [f[1].replace(/%[0-9\-\.]*[sdi]/ig, '*')
                   .replace(/%[c]/ig, '/'),
               f[1].replace(/%[0-9\-\.]*[sdic]/ig, '*')])
    .flat(1)

  // now for some filtering fun
  var filteredstrings = qvmstrings.filter((file, i, arr) =>
      // make sure there is only one occurrence
      arr.indexOf(file) === i
      // the shortest strings match from the file system is probably
      //   greater than 5 characters, because even vm is actually vm%c%s
      && file.length > 5
      // should include a slash or a %c
      //  TODO: doesn't work, not all shaders have slashes
      //&& (file.includes('%') || file.includes('/'))
      // colons aren't allowed in filenames
      && !file.match(/[:]|\\n|\.\.|^[\s\*]*$|EV_/ig)
    // it also has to include one of the top directories
    //   shaders in QUAKED skip the textures folder
    //   this will be faster than minimatch lookups
    //  TODO: doesn't work, not all shaders have slashes
    //  && topdirs.filter(dir => file.includes(dir)).length > 0
    )
  return filteredstrings
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
      .concat(['botfiles/**', '*.cfg', '*.shader', disassembly])
    result[qvms[i]] = qvmstrings
  }
  console.log(`Found ${qvms.length} QVMs and ${Object.values(result).reduce((t, o) => t += o.length, 0)} strings`)
  return result
}

function getGameAssets(disassembly) {
  console.log('Looking for game entities')
  var lines = fs.readFileSync(disassembly).toString('utf-8').split('\n')
  var entMatches = lines
    .map(l => MATCH_STRINGS.exec(l))
    .filter(l => l)
  // now map the entity strings use to read a .map to the jumplist for bg_misc.c bg_itemlist[]
  var entityJmplist = lines
    .map(l => MATCH_JMPLIST.exec(l))
    .filter(j => j)
  // get the earliest/latest parts of the list matching the entities above
  var bg_itemlist = entMatches
    .filter(l => l[3].match(MATCH_ENTS))
    .reduce((obj, l) => {
      var itemoffset = entityJmplist
        .filter(j => parseInt(j[3], 16) === parseInt(l[2], 16))[0]
      if(!itemoffset) return obj
      var itemstart = parseInt(itemoffset[2], 16)
      // map the jump list on to 13bytes of gitem_s
      var itemstrings = entityJmplist
        .filter(j => parseInt(j[2], 16) >= itemstart && parseInt(j[2], 16) < itemstart + SIZEOF_GITEM)
        .map(j => entMatches.filter(s => parseInt(s[2], 16) === parseInt(j[3], 16))[0])
        .filter(s => s && s[3].length > 0 && !s[3].match(/EV_/))
        .map(s => s[3].replace(/(\.wav)\s+/ig, '$1{SPLIT}').split(/\{SPLIT\}/ig))
        .flat(1)
      if(itemstrings.length > 0) {
        obj[l[3]] = itemstrings
      }
      return obj
    }, {})
  console.log(`Found ${Object.keys(bg_itemlist).length} game entities`)
  return bg_itemlist
}

module.exports = {
  getGameAssets: getGameAssets,
  loadQVMEntities: loadQVMEntities,
  loadQVMStrings: loadQVMStrings,
  graphQVM: graphQVM,
  MATCH_ENTS: MATCH_ENTS
}