'use strict'
console.log('Service Worker registered')

var DB_STORE_NAME = 'FILE_DATA';
var PROGRAM_FILES = 'quake-games-cache-v1';
var open

var precacheConfig = [
  'index.html',
  'nipplejs.js',
  'quakejs-border.png',
  'quakejs-noborder-transparent.png',
  'quake3e.js',
  'quake3e.wasm',
  'assets/baseq3-cc/index.json',
  'server-worker.js',
]

var caseInsensitiveCompare = function (str, cmp) { return str.localeCompare(cmp, 'en', {sensitivity: 'base'}) === 0 }
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

async function readFile(key) {
  var db = await openDatabase()
  var transaction = db.transaction([DB_STORE_NAME], 'readwrite');
  var objStore = transaction.objectStore(DB_STORE_NAME);
  return await new Promise(function (resolve) {
    let tranCursor = objStore.get(key)
    let result = []
    tranCursor.onsuccess = function () {
      resolve(tranCursor.result)
    }
    tranCursor.onerror = function (error) {
      console.error(error)
      resolve(error)
    }
    transaction.commit()
  })
}

async function openDatabase() {
  if(open && open.readyState != 'done') {
    var checkInterval, count = 0
    await new Promise(resolve => {
      checkInterval = setInterval(() => {
        if(open.readyState == 'done' || count === 1000) {
          clearInterval(checkInterval)
          resolve()
        }
        else
          count++
      }, 20)
    })
  }
  if(open && open.readyState == 'done') {
    return Promise.resolve(open.result)
  }
  return await new Promise(function (resolve) {
    open = indexedDB.open('/base', 21)
    open.onsuccess = function () {
      resolve(open.result)
    }
    open.onupgradeneeded = function (evt) {
      var fileStore = open.result.createObjectStore(DB_STORE_NAME)
      if (!fileStore.indexNames.contains('timestamp')) {
        fileStore.createIndex('timestamp', 'timestamp', { unique: false });
      }
    }
    open.onerror = function (error) {
      console.error(error)
      resolve(error)
    }
  })
}

async function writeStore(value, key) {
  var db = await openDatabase()
  var transaction = db.transaction([DB_STORE_NAME], 'readwrite');
  var objStore = transaction.objectStore(DB_STORE_NAME);
  return await new Promise(function (resolve) {
    let storeValue = objStore.put(value, key)
    storeValue.onsuccess = function () {}
    transaction.oncomplete = function () {
      //db.close()
      resolve()
    }
    storeValue.onerror = function (error) {
      console.error(error, value, key)
      resolve(error)
    }
    transaction.commit()
  })
}

async function mkdirp(path) {
  var segments = path.split(/\/|\\/gi)
  for(var i = 3; i <= segments.length; i++)
  {
    var dir = '/' + segments.slice(0, i).join('/')
    var obj = {
      timestamp: new Date(),
      mode: 16895
    }
    await writeStore(obj, dir)
  }
}

async function fetchAsset(url, key) {
  var response = await fetch(url, {credentials: 'omit'})
  if (!response.ok) {
    throw new Error('Request for ' + key + ' returned a ' +
      'response with status ' + response.status)
  }
  var content = await response.clone().arrayBuffer()
  if(key.match(/index\.json/i)) {
    var moreIndex = (JSON.parse((new TextDecoder("utf-8")).decode(content)) || [])
    Object.keys(moreIndex).forEach(function (k) {
      precacheConfig.push(k.toLowerCase().replace(/^\/base\//ig, 'assets/'))
    })
  }
  key = key.replace(/^\//ig, '').replace(/-cc?r?\//ig, '\/')
  await mkdirp('base/' + key.replace(/^\/?base\/|^\/?assets\/|^\/|\/[^\/]*$/ig, ''))
  var obj = {
    timestamp: new Date(),
    mode: 33206,
    contents: new Uint8Array(content)
  }
  await writeStore(obj, '/base/' + key.replace(/^\/?base\/|^\/?assets\/|^\//ig, ''))
  return response
}

self.addEventListener('install', function(event) {
  event.waitUntil(
    Promise.all(precacheConfig.map(function (requiredFile) {
      var localName = '/base/' + requiredFile.replace(/^\/?assets\//ig, '')
      return readFile(localName)
        .then(function (files) {
          if(files && files.contents) {
            // already saved
          } else {
            return fetchAsset(requiredFile, localName)
          }
        })
    })).then(function () {
      //caches.open(PROGRAM_FILES)
      //  .then(function (cache) { return cache.addAll(precacheConfig })
      return self.skipWaiting()
    })
  )
})
self.addEventListener('activate', function(event) {
  // make sure database is created so emscripten picks up the same instance
  event.waitUntil(openDatabase()
    .then(db => {
      open = null
      db.close()
      return self.clients.claim()
    }))
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
      var localName = '/base/' + url.replace(/^\/?assets\/|^\//ig, '')
      var init = { 
        status : 200,
        url: event.request.url,
        headers: {
          // these are needed to start up, the engine only cares about bits
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
      event.respondWith(readFile(localName)
        .then(function (files) {
          if(files && files.contents) {
            return new Response(files.contents, init)
          } else {
            return fetchAsset(event.request.url, url)
          }
        }))
    }
  }
})
