var path = require('path');
var fs = require('fs');
var glob = require('glob')
var exec = require('child_process').execSync;

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'
var disassembler = path.resolve(path.join(__dirname, '../lib/qvmdisas/'))
var MATCH_CONST = /CONST.*String:\s+"(.*?)"/

function loadQVM(qvm, project) {
  if(!project) {
    project = PROJECT
  }
  var cgame = path.join(project, glob.sync(qvm, {cwd: project})[0])
  var result
  try {
    exec(`echo "header" | ./QVMDisas "${cgame}"`, {cwd: disassembler})
  } catch (e) {
    result = e.stdout.toString()
  }
  var start = parseInt((/Data Offset: (0x.*)/i).exec(result)[1])
  var length = parseInt((/Data Length: (0x.*)/i).exec(result)[1])
  var buffer = fs.readFileSync(cgame)
  var gameStrings = buffer.slice(start + length)
    .toString('utf-8').split('\0')
  return gameStrings
}

function loadQVMFunction(name, qvm, project) {
  if(!project) {
    project = PROJECT
  }
  var cgame = path.join(project, glob.sync(qvm, {cwd: project})[0])
  var topdirs = glob.sync('*/', {cwd: path.dirname(path.dirname(cgame))})
  var result
  try {
    exec(`echo "dis ${name}" | ./QVMDisas "${cgame}"`, {cwd: disassembler})
  } catch (e) {
    result = e.stdout.toString()
  }
  var constStrings = result.split('\n')
    .filter(line => line.match(MATCH_CONST))
    .map(line => MATCH_CONST.exec(line)[1])
  return constStrings
}

function graphQVM(qvm, project) {
  if(!project) {
    project = PROJECT
  }
  console.log('Looking for QVM strings')
  var cgame = path.join(project, glob.sync(qvm, {cwd: project})[0])
  var topdirs = glob.sync('*/', {cwd: path.dirname(path.dirname(cgame))})
  var resultStrings = []
  var subroutines = []
  for(var i = 0; i < topdirs.length; i++) {
    var result
    try {
      exec(`echo "sref ${topdirs[i]}" | ./QVMDisas "${cgame}"`, {cwd: disassembler})
    } catch (e) {
      result = e.stdout.toString()
    }
    // search for all references to assets
    subroutines.push.apply(subroutines, result.split('\n')
      .filter(line => line.match(/^sub_/)))
  }
  subroutines = subroutines.filter((a, i, arr) => arr.indexOf(a) === i)
  for(var j = 0; j < subroutines.length; j++) {
    var funcStrings = loadQVMFunction(subroutines[j].trim(), qvm, project)
    resultStrings.push.apply(resultStrings, funcStrings)
  }
  resultStrings = resultStrings
    .filter(s => topdirs.filter(d => s.includes(d)).length > 0)
  console.log(`Found ${resultStrings.length} QVM strings`)
  return resultStrings
}

module.exports = {
  loadQVM: loadQVM,
  graphQVM: graphQVM,
}
