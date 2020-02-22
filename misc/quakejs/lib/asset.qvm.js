var path = require('path');
var fs = require('fs');
var glob = require('glob')
var exec = require('child_process').execSync;

var PROJECT = '/Users/briancullinan/planet_quake_data/quake3-defrag-combined'
var disassembler = path.resolve(path.join(__dirname, '../lib/qvmdisas/'))
var MATCH_CONST = /CONST.*String:\s+"(.*?)"/
var QVM_CONSCIOUS = {}

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

function loadQVMData(offset, qvm, project) {
  if(!project) {
    project = PROJECT
  }
  var cgame = path.join(project, glob.sync(qvm, {cwd: project})[0])
  var buffer = fs.readFileSync(cgame)
  var gameStrings = buffer.slice(buffer.length - offset)
    .toString('utf-8').split('\0')
  return gameStrings[0]
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
  if(typeof QVM_CONSCIOUS[cgame] == 'undefined') {
    QVM_CONSCIOUS[cgame] = {searches: {}, functions: {}, strings: []}
  }
  QVM_CONSCIOUS[cgame]['functions'][name] = result.replace('qvmd> EOF', '').replace('qvmd> ', '')
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
    var subs = result.split('\n').filter(line => line.match(/^sub_/))
    if(typeof QVM_CONSCIOUS[cgame] == 'undefined') {
      QVM_CONSCIOUS[cgame] = {searches: {}, functions: {}, strings: []}
    }
    QVM_CONSCIOUS[cgame]['searches'][topdirs[i]] = subs
    subroutines.push.apply(subroutines, subs)
  }
  subroutines = subroutines.filter((a, i, arr) => arr.indexOf(a) === i)
  for(var j = 0; j < subroutines.length; j++) {
    var funcStrings = loadQVMFunction(subroutines[j].trim(), qvm, project)
    resultStrings.push.apply(resultStrings, funcStrings)
  }
  if(typeof QVM_CONSCIOUS[cgame] == 'undefined') {
    QVM_CONSCIOUS[cgame] = {searches: {}, functions: {}, strings: []}
  }
  resultStrings = resultStrings
    .filter(s => topdirs.filter(d => s.includes(d)).length > 0)
  QVM_CONSCIOUS[cgame]['strings'] = resultStrings
  fs.writeFileSync(path.join(__dirname, 'previous_qvm.json'), JSON.stringify(QVM_CONSCIOUS, null, 2))
  console.log(`Found ${resultStrings.length} QVM strings`)
  return resultStrings
}

function minimatchWildCards(everything, qvmstrings) {
  console.log('Looking for matching files from QVM strings')
  var wildcards = qvmstrings
    .filter(f => f.includes('/') && f.length > 5)
    .map(f => f.replace(/%[0-9sdi\-\.]+/ig, '*')
               .replace(/%[c]/ig, '/')) // assuming a single character is the path seperator, TODO: could be a number or something, derive from QVM
  var result = everything.reduce((arr, f) => {
    if(!arr.includes(f)) {
      if(wildcards.filter(w => minimatch(f, '**/' + w + '*')).length > 0)
        arr.push(f)
    }
    return arr
  }, [])
  console.log(`Found ${result.length} matching files from QVM strings`)
  return result
}

module.exports = {
  loadQVMData: loadQVMData,
  loadQVMFunction: loadQVMFunction,
  loadQVM: loadQVM,
  graphQVM: graphQVM,
}
