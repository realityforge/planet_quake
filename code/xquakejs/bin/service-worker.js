'use strict'
var PROGRAM_FILES = 'quake-games-cache-v1';

var precacheConfig = [
  'index.html',
  'nipplejs.js',
  'quakejs-border.png',
  'quakejs-noborder-transparent.png',
  'quake3e.js',
  'quake3e.wasm',
  'assets/baseq3-cc/index.json',
]

var caseInsensitiveCompare = (str, cmp) => str.localeCompare(cmp, 'en', {sensitivity: 'base'}) === 0
var ignoreUrlParametersMatching = [/^utm_/]
var addDirectoryIndex = function (pathname, index) {
    if (pathname.slice(-1) === '/') {
      pathname += index
    }
    return pathname
  }
var cleanResponse = function (originalResponse) {
    // If this is not a redirected response, then we don't have to do anything.
    if (!originalResponse.redirected) {
      return Promise.resolve(originalResponse)
    }

    // Firefox 50 and below doesn't support the Response.body stream, so we may
    // need to read the entire body to memory as a Blob.
    var bodyPromise = 'body' in originalResponse ?
      Promise.resolve(originalResponse.body) :
      originalResponse.blob()
    return bodyPromise.then(function(body) {
      // new Response() is happy when passed either a stream or a Blob.
      return new Response(body, {
        headers: originalResponse.headers,
        status: originalResponse.status,
        statusText: originalResponse.statusText
      })
    })
  }
var isPathWhitelisted = function (whitelist, absoluteUrlString) {
    // If the whitelist is empty, then consider all URLs to be whitelisted.
    if (whitelist.length === 0) {
      return true
    }

    // Otherwise compare each path regex to the path of the URL passed in.
    var path = (new URL(absoluteUrlString)).pathname
    return whitelist.some(function(whitelistedPathRegex) {
      return path.match(whitelistedPathRegex)
    })
  }
var stripIgnoredUrlParameters = function (originalUrl,
    ignoreUrlParametersMatching) {
    var url = new URL(originalUrl)
    // Remove the hash; see https://github.com/GoogleChrome/sw-precache/issues/290
    url.hash = ''
    url.search = url.search.slice(1) // Exclude initial '?'
      .split('&') // Split into an array of 'key=value' strings
      .map(function(kv) {
        return kv.split('='); // Split each 'key=value' string into a [key, value] array
      })
      .filter(function(kv) {
        return ignoreUrlParametersMatching.every(function(ignoredRegex) {
          return !ignoredRegex.test(kv[0]); // Return true iff the key doesn't match any of the regexes.
        })
      })
      .map(function(kv) {
        return kv.join('='); // Join each [key, value] array into a 'key=value' string
      })
      .join('&'); // Join the array of 'key=value' strings into a string with '&' in between each

    return url.pathname.replace(/^\//ig, '') + (url.search ? ('?' + url.search) : '')
  }

function openFile(predicate, result, resolve, evt) {
  let limit = 1
  let cursor = evt.target.result
  if (!cursor) return resolve(result)
  let item = cursor.value
  if (predicate(item, cursor.key)) {
    result.push(item)
    if (limit && result.length == limit) {
      return resolve(result)
    }
  }
  cursor.continue()
}

async function readStore(db, store, predicate) {
  if(!db) {
    db = await openDatabase()
  }
  return new Promise(resolve => {
    let tran = db.transaction(store)
    let objStore = tran.objectStore(store)
    let tranCursor = objStore.openCursor()
    let result = []
    tranCursor.onsuccess = openFile.bind(null, predicate, result, resolve)
    tranCursor.onerror = error => {
      console.error(error)
      resolve(error)
    }
  })
}

async function readFile(db, store, key) {
  if(!db) {
    db = await openDatabase()
  }
  return new Promise(resolve => {
    let tran = db.transaction(store)
    let objStore = tran.objectStore(store)
    let tranCursor = objStore.get(key)
    let result = []
    tranCursor.onsuccess = () => {
      resolve(tranCursor.result)
    }
    tranCursor.onerror = error => {
      console.error(error)
      resolve(error)
    }
  })
}
async function openDatabase() {
  return new Promise(resolve => {
    let open = indexedDB.open('/base', 21)
    open.onsuccess = () => {
      var transaction = open.result.transaction(['FILE_DATA'], 'readwrite');
      var files = transaction.objectStore('FILE_DATA');
      resolve(open.result)
    }
    open.onupgradeneeded = evt => {
      var fileStore = open.result.createObjectStore('FILE_DATA')
      if (!fileStore.indexNames.contains('timestamp')) {
        fileStore.createIndex('timestamp', 'timestamp', { unique: false });
      }
    }
    open.onerror = error => {
      console.error(error)
      resolve(error)
    }
  })
}

async function writeStore(value, key) {
  var db = await openDatabase()
  return new Promise(resolve => {
    let tran = db.transaction('FILE_DATA', 'readwrite')
    let objStore = tran.objectStore('FILE_DATA')
    let storeValue = objStore.add(value, key)
    storeValue.onsuccess = resolve
    storeValue.onerror = error => {
      console.error(error)
      resolve(error)
    }
  })
}

async function mkdirp(path) {
  var segments = path.split(/\/|\\/gi)
  for(var i = 3; i < segments.length; i++)
  {
    var dir = segments.slice(0, i).join('/')
    var obj = {
      timestamp: new Date(),
      mode: 16895
    }
    try {
      await writeStore(obj, dir)
    } catch (e) {
    }
  }
}

async function fetchAsset(key) {
  var response = await fetch(key, {credentials: 'omit'})
  if (!response.ok) {
    throw new Error('Request for ' + key + ' returned a ' +
      'response with status ' + response.status)
  }
  var content = await response.clone().arrayBuffer()
  if(key.match(/index\.json/i)) {
    var moreIndex = (JSON.parse((new TextDecoder("utf-8")).decode(content)) || [])
    Object.keys(moreIndex).forEach(k => {
      precacheConfig.push(k.toLowerCase().replace(/^\/base\//ig, 'assets/'))
    })
  }
  await mkdirp('/base/' + key.replace(/^\/?assets\/|^\/|\/[^\/]*$/i, ''))
  var obj = {
    timestamp: new Date(),
    mode: 33206,
    contents: new Uint8Array(content)
  }
  try {
    await writeStore(obj, '/base/' + key.replace(/^\/?assets\/|^\//i, ''))
  } catch (e) {
  }
  return obj
}

self.addEventListener('install', function(event) {
  var db
  event.waitUntil(
    Promise.all(precacheConfig.map(requiredFile => {
      return new Promise(async resolve => {
        var files = await readFile(db, 'FILE_DATA', '/base/' + requiredFile.replace(/^\/?assets\//ig, ''))
        if(files && files.contents) {
          // already saved
        } else {
          await fetchAsset(requiredFile)
        }
        resolve()
      })
    })).then(() => {
      //caches.open(PROGRAM_FILES)
      //  .then(cache => cache.addAll(precacheConfig))
      return self.skipWaiting()
    })
  )
})
self.addEventListener('activate', function(event) {
  
})
self.addEventListener('fetch', function(event) {
  if (event.request.method === 'GET') {
    // Should we call event.respondWith() inside this fetch event handler?
    // This needs to be determined synchronously, which will give other fetch
    // handlers a chance to handle the request if need be.
    var shouldRespond
    // First, remove all the ignored parameters and hash fragment, and see if we
    // have that URL in our cache. If so, great! shouldRespond will be true.
    var url = stripIgnoredUrlParameters(event.request.url, ignoreUrlParametersMatching)
    shouldRespond = precacheConfig.includes(url)
    
    if(!shouldRespond) {
      shouldRespond = precacheConfig.includes(url.toLowerCase())
    }
    // If shouldRespond is false, check again, this time with 'index.html'
    // (or whatever the directoryIndex option is set to) at the end.
    var directoryIndex = 'index.html'
    if (!shouldRespond && directoryIndex) {
      url = addDirectoryIndex(url, directoryIndex)
      shouldRespond = precacheConfig.includes(url)
    }

    // If shouldRespond is still false, check to see if this is a navigation
    // request, and if so, whether the URL matches navigateFallbackWhitelist.
    var navigateFallback = '/index.html'
    if (!shouldRespond &&
        navigateFallback &&
        (event.request.mode === 'navigate') &&
        isPathWhitelisted([/index\.json/i], event.request.url)) {
      url = new URL(navigateFallback, self.location).toString()
      shouldRespond = precacheConfig.includes(url)
    }

    // If shouldRespond was set to true at any point, then call
    // event.respondWith(), using the appropriate cache key.
    if (shouldRespond) {
      event.respondWith(
        new Promise(async resolve => {
          var localName = '/base/' + url.replace(/^\/?assets\/|^\//ig, '')
          var files = await readFile(null, 'FILE_DATA', localName)
          var response
          var init = { 
            status : 200,
            url: event.request.url,
            headers: {
              'content-type': event.request.url.includes('.json')
                ? 'application/json'
                : event.request.url.includes('.html')
                  ? 'text/html'
                  : event.request.url.includes('.png')
                    ? 'image/png'
                    : event.request.url.includes('.jpg')
                      ? 'image/jpg'
                      : event.request.url.includes('.js')
                        ? 'application/javascript'
                        : event.request.url.includes('.wasm')
                          ? 'application/wasm'
                          : 'application/octet-stream'
            }
          }
          if(files && files.contents) {
            
          } else {
            files = await fetchAsset(event.request.url)
          }
          response = new Response(files.contents, init)
          return resolve(response)
        })
      )
    }
  }
})
