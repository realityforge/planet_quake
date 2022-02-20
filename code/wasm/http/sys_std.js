
function addressToString(addr, length) {
  let newString = ''
  if(!length) length = 1024
  for(let i = 0; i < length; i++) {
    if(Q3e.paged[addr + i] == '0') {
      break;
    }
    newString += String.fromCharCode(Q3e.paged[addr + i])
  }
  return newString
}

function stringToAddress(str, addr) {
  let start = Q3e.sharedMemory + Q3e.sharedCounter
  if(addr) start = addr
  for(let j = 0; j < str.length; j++) {
    Q3e.paged[start+j] = str.charCodeAt(j)
  }
  Q3e.paged[start+str.length] = 0
  if(!addr) {
    Q3e.sharedCounter += str.length + 1
    Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
    if(Q3e.sharedCounter > 1024 * 512) {
      Q3e.sharedCounter = 0
    }
  }
  return start
}


// here's the thing, I know for a fact that all the callers copy this stuff
//   so I don't need to increase my temporary storage because by the time it's
//   overwritten the data won't be needed, should only keep shared storage around
//   for events and stuff that might take more than 1 frame
function stringsToMemory(list, length) {
  // add list length so we can return addresses like char **
  let start = Q3e.sharedMemory + Q3e.sharedCounter
  let posInSeries = start + list.length * 4
  for (let i = 0; i < list.length; i++) {
    Q3e.paged32[(start+i*4)>>2] = posInSeries // save the starting address in the list
    stringToAddress(list[i], posInSeries)
    posInSeries += list[i].length + 1
  }
  if(length) Q3e.paged32[length >> 2] = posInSeries - start
  Q3e.sharedCounter = posInSeries - Q3e.sharedMemory
  Q3e.sharedCounter += 4 - (Q3e.sharedCounter % 4)
  if(Q3e.sharedCounter > 1024 * 512) {
    Q3e.sharedCounter = 0
  }
  return start
}

var DATE = {
  asctime: function () {
    // Don't really care what time it is because this is what the engine does
    //   right above this call
    return stringToAddress(new Date().toLocaleString())
  },
  time: function () {
    // The pointer returned by localtime (and some other functions) are actually pointers to statically allocated memory.
    // perfect.

  },
  localtime: function (t) {
    // TODO: only uses this for like file names, so doesn't have to be fast
    debugger
    let s = Q3e.sharedMemory + Q3e.sharedCounter
    Q3e.paged32[(s + 4 * 1) >> 2] = floor(t / 60)
    Q3e.paged32[(s + 4 * 1) >> 2] = floor(t / 60 / 60)
    Q3e.paged32[(s + 4 * 1) >> 2] = floor(t / 60 / 60)
    /*
typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59]
	int tm_min;     /* minutes after the hour - [0,59]
	int tm_hour;    /* hours since midnight - [0,23]
	int tm_mday;    /* day of the month - [1,31]
	int tm_mon;     /* months since January - [0,11]
	int tm_year;    /* years since 1900
	int tm_wday;    /* days since Sunday - [0,6]
	int tm_yday;    /* days since January 1 - [0,365]
	int tm_isdst;   /* daylight savings time flag 
} qtime_t;
*/

  },
  ctime: function (t) {
    return stringToAddress(new Date(t).toString())
  },
  Com_RealTime: function (outAddress) {
    localtime(Date.now())
  }
}


var STD = {
  assert: console.assert, // TODO: convert to variadic fmt for help messages
  memset: function (addr, val, count) { debugger },
  longjmp: function (id, code) { throw new Error('longjmp', id, code) },
  setjmp: function (id) { try {  } catch (e) { } },
  fprintf: function (f, err, args) {
    // TODO: rewrite va_args in JS for convenience?
    console.log(addressToString(err), addressToString(Q3e.paged32[(args) >> 2]));
  },
  tolower: function tolower(c) { return String.fromCharCode(c).toLowerCase().charCodeAt(0) },
  srand: function srand() {}, // TODO: highly under-appreciated game dynamic
  atoi: function (i) { return parseInt(addressToString(i)) },
  atof: function (f) { return parseFloat(addressToString(f)) },
  strtof: function (f, n) { 
    // TODO: convert this to some sort of template?
    let str = addressToString(f)
    let result = parseFloat(str)
    if(isNaN(result)) {
      if(n) Q3e.paged32[(n) >> 2] = f
      return 0
    } else {
      if(n) Q3e.paged32[(n) >> 2] = f + str.length
      return result
    }
  },
  strlen: function (addr) { Q3e.paged.subarray(addr).indexOf(0) },
  memcpy: function (dest, source) {},
  strncpy: function () { debugger },
  strcmp: function () { debugger },
  strcat: function () { debugger },
  strchr: function () { debugger },
  memmove: function () { debugger },
  strrchr: function () { debugger },
  strcpy: function () { debugger },
  strncmp: function () { debugger },
  strpbrk: function () { debugger },
  strstr: function () { debugger },
  memcmp: function () { debugger },
  qsort: function () { debugger },
  strncat: function () { debugger },

}



var MATHS = {
  htons: function (c) { return c ? (n<<8 | n>>8) : n },
  ntohs: function (c) { return c ? (n<<8 | n>>8) : n },

}
// These can be assigned automatically? but only because they deal only with numbers and not strings
//   TODO: What about converting between float, endian, and shorts?
let maths = Object.getOwnPropertyNames(Math)
for(let j = 0; j < maths.length; j++) {
  MATHS[maths[j] + 'f'] = Math[maths[j]]
  MATHS[maths[j]] = Math[maths[j]]
}


