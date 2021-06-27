// TRIM this way down BECAUSE! Q3 has it's own internal event queue, so by 
//   Using SDLs event queue, we are double queuing. Instead, we just inject
//   events directly into the games event stack, since JS must have some sort
//   of memory semaphore built in, we don't need to worry about similar pthread(mutexes)
//   DROP SDL input support all together and use sys_input.js, this stuff is for n00bz
var global = window || global
global['SDL'] = {}

function SDL_SetVideoMode (width, height, depth, flags) {
  // (0,0) means 'use fullscreen' in native; in Emscripten, use the current canvas size.
  if (width == 0 && height == 0) {
    width = SYSI.canvas.width;
    height = SYSI.canvas.height;
  }

  SDL.settingVideoMode = true; // SetVideoMode itself should not trigger resize events
  SYSI.canvas.width = width
  SYSI.canvas.height = height
  SDL.settingVideoMode = false;

  // Free the old surface first if there is one
  if (SDL.screen) {
    SDL.freeSurface(SDL.screen);
    assert(!SDL.screen);
  }

  if (SDL.GL) flags = flags | 0x04000000; // SDL_OPENGL - if we are using GL, then later calls to SetVideoMode may not mention GL, but we do need it. Once in GL mode, we never leave it.

  SDL.screen = SDL.makeSurface(width, height, flags, true, 'screen');

  return SDL.screen;
}

function SDL_DestroyWindow (window) {
  // TODO: delete window?
}

function SDL_QuitSubSystem (flags) {}

function SDL_MinimizeWindow (window) {
  window.fullscreen = false
}

function SDL_GL_SetAttribute (attr, value) {
  if (!(attr in SDL.glAttributes)) {
    abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
  }

  SDL.glAttributes[attr] = value;
}

function SDL_GL_GetAttribute (attr, value) {
  if (!(attr in SDL.glAttributes)) {
    abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
  }

  if (value) HEAP32[(value+0)>>2] = SDL.glAttributes[attr];

  return 0;
}

function SDL_ShowCursor (toggle) {
  // TODO: wait for first click because browsers do this really annoying
  //   thing where it can request the pointer lock from outside the window
  //   causing the pointer to be stuck and not receive mouse move events
  //   this is apparent when hiding the Q3 console with a ~ key press when
  //   the mouse is still outside
  switch (toggle) {
    case 0: // SDL_DISABLE
      //if (Browser.isFullscreen) { // only try to lock the pointer when in full screen mode
      //} else { // else return SDL_ENABLE to indicate the failure
      //  return 1;
      //}
      SYSI.canvas.requestPointerLock();
      return 0;
      break;
    case 1: // SDL_ENABLE
      SYSI.canvas.exitPointerLock();
      return 1;
      break;
    case -1: // SDL_QUERY
      return !Browser.pointerLock;
      break;
    default:
      console.log( "SDL_ShowCursor called with unknown toggle parameter value: " + toggle + "." );
      break;
  }
}

function SDL_GetWindowFlags (x, y) {
  if (Browser.isFullscreen) {
     return 1;
  }

  return 0;
}

function SDL_SetWindowGrab (window, grabbed) {
  // set the window to do the grabbing, when ungrabbing this doesn't really matter
  SDL_ShowCursor(grabbed)
}

function SDL_GetError () {
  if (!SDL.errorMessage) {
    SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), ALLOC_NORMAL);
  }
  return SDL.errorMessage;
}

function SDL_SetError (err) {}

function Sys_GetDisplayMode (index, mode) {
  SDL.mode = allocate(new Int32Array(6>>2), ALLOC_NORMAL)
  SDL.mode[1] = window.innerWidth
  SDL.mode[2] = window.innerHeight
  return 0
}

function SDL_SetWindowTitle (window, title) {
  document.title = UTF8ToString(title)
}

function SDL_GL_GetDrawableSize (window, w, h) {
  HEAP32[w] = SYSI.canvas.width
  HEAP32[h] = SYSI.canvas.height
}

function SDL_CreateWindow (title, x, y, w, h, flags) {
  // TODO: this will be really cool with multiworld and Movie Maker Edition
  //   Make the whole engine capable of rendering multiple windows so 2 people
  //   Can use the same machine to play on 2 displays, true PC multiplayer
  var win = _malloc(8)
  SDL.windows[handle] = {
    x: x,
    y: y,
    w: w,
    h: h,
    title: title,
    flags: flags,
  }
  SDL_SetWindowTitle(win, title)
  return win;
}

function SDL_SetDisplayModeForDisplay () {
  throw new Error('TODO: just keep track of this or something')
}

function SDL_GL_SetSwapInterval (interval) {
  throw new Error('TODO: GL_SetSwapInterval')
}

function SDL_GL_GetDrawableSize () {
  throw new Error('TODO: GL_GetDrawableSize')
}

function SDL_WarpMouseInWindow () {
  // doesn't work in javascript, which causes a stupid error where if the
  //   console is up when GRABMOUSE() is called it will get stuck outside
  throw new Error('TODO: SDL_WarpMouseInWindow')
}

function SDL_WasInit () {
  if (SDL.startTime === null) {
    Sys_GLimpInit();
  }
  return 1;
}

function SDL_GetCurrentVideoDriver () {
  throw new Error('SDL_GetCurrentVideoDriver: emscripten')
}

function SDL_GL_SwapWindow (window) {
  throw new Error('TODO: GL_SwapWindow')
}

function SDL_StartTextInput () {
  SDL.textInput = true;
}

function SDL_StopTextInput () {
  SDL.textInput = false;
}

function SDL_OpenAudio (desired, obtained) {
  try {
    SDL.audio = {
      freq: HEAP32[(desired+C_STRUCTS.SDL_AudioSpec.freq)>>2],
      format: HEAP16[(desired+C_STRUCTS.SDL_AudioSpec.format)>>1],
      channels: HEAP8[desired+C_STRUCTS.SDL_AudioSpec.channels],
      samples: HEAP16[(desired+C_STRUCTS.SDL_AudioSpec.samples)>>1], // Samples in the CB buffer per single sound channel.
      callback: HEAP32[(desired+C_STRUCTS.SDL_AudioSpec.callback)>>2],
      userdata: HEAP32[(desired+C_STRUCTS.SDL_AudioSpec.userdata)>>2],
      paused: true,
      timer: null
    };
    // The .silence field tells the constant sample value that corresponds to the safe un-skewed silence value for the wave data.
    if (SDL.audio.format == 0x0008 /*AUDIO_U8*/) {
      SDL.audio.silence = 128; // Audio ranges in [0, 255], so silence is half-way in between.
    } else if (SDL.audio.format == 0x8010 /*AUDIO_S16LSB*/) {
      SDL.audio.silence = 0; // Signed data in range [-32768, 32767], silence is 0.
    } else if (SDL.audio.format == 0x8120 /*AUDIO_F32*/) {
      SDL.audio.silence = 0.0; // Float data in range [-1.0, 1.0], silence is 0.0
    } else {
      throw 'Invalid SDL audio format ' + SDL.audio.format + '!';
    }
    // Round the desired audio frequency up to the next 'common' frequency value.
    // Web Audio API spec states 'An implementation must support sample-rates in at least the range 22050 to 96000.'
    if (SDL.audio.freq <= 0) {
      throw 'Unsupported sound frequency ' + SDL.audio.freq + '!';
    } else if (SDL.audio.freq <= 22050) {
      SDL.audio.freq = 22050; // Take it safe and clamp everything lower than 22kHz to that.
    } else if (SDL.audio.freq <= 32000) {
      SDL.audio.freq = 32000;
    } else if (SDL.audio.freq <= 44100) {
      SDL.audio.freq = 44100;
    } else if (SDL.audio.freq <= 48000) {
      SDL.audio.freq = 48000;
    } else if (SDL.audio.freq <= 96000) {
      SDL.audio.freq = 96000;
    } else {
      throw 'Unsupported sound frequency ' + SDL.audio.freq + '!';
    }
    if (SDL.audio.channels == 0) {
      SDL.audio.channels = 1; // In SDL both 0 and 1 mean mono.
    } else if (SDL.audio.channels < 0 || SDL.audio.channels > 32) {
      throw 'Unsupported number of audio channels for SDL audio: ' + SDL.audio.channels + '!';
    } else if (SDL.audio.channels != 1 && SDL.audio.channels != 2) { // Unsure what SDL audio spec supports. Web Audio spec supports up to 32 channels.
      console.log('Warning: Using untested number of audio channels ' + SDL.audio.channels);
    }
    if (SDL.audio.samples < 128 || SDL.audio.samples > 524288 /* arbitrary cap */) {
      throw 'Unsupported audio callback buffer size ' + SDL.audio.samples + '!';
    } else if ((SDL.audio.samples & (SDL.audio.samples-1)) != 0) {
      throw 'Audio callback buffer size ' + SDL.audio.samples + ' must be a power-of-two!';
    }

    var totalSamples = SDL.audio.samples*SDL.audio.channels;
    if (SDL.audio.format == 0x0008 /*AUDIO_U8*/) {
      SDL.audio.bytesPerSample = 1;
    } else if (SDL.audio.format == 0x8010 /*AUDIO_S16LSB*/) {
      SDL.audio.bytesPerSample = 2;
    } else if (SDL.audio.format == 0x8120 /*AUDIO_F32*/) {
      SDL.audio.bytesPerSample = 4;
    } else {
      throw 'Invalid SDL audio format ' + SDL.audio.format + '!';
    }
    SDL.audio.bufferSize = totalSamples*SDL.audio.bytesPerSample;
    SDL.audio.bufferDurationSecs = SDL.audio.bufferSize / SDL.audio.bytesPerSample / SDL.audio.channels / SDL.audio.freq; // Duration of a single queued buffer in seconds.
    SDL.audio.bufferingDelay = 50 / 1000; // Audio samples are played with a constant delay of this many seconds to account for browser and jitter.
    SDL.audio.buffer = _malloc(SDL.audio.bufferSize);

    // To account for jittering in frametimes, always have multiple audio buffers queued up for the audio output device.
    // This helps that we won't starve that easily if a frame takes long to complete.
    SDL.audio.numSimultaneouslyQueuedBuffers = Module['SDL_numSimultaneouslyQueuedBuffers'] || 5;

    // Pulls and queues new audio data if appropriate. This function gets "over-called" in both requestAnimationFrames and
    // setTimeouts to ensure that we get the finest granularity possible and as many chances from the browser to fill
    // new audio data. This is because setTimeouts alone have very poor granularity for audio streaming purposes, but also
    // the application might not be using emscripten_set_main_loop to drive the main loop, so we cannot rely on that alone.
    SDL.audio.queueNewAudioData = function SDL_queueNewAudioData() {
      if (!SDL.audio) return;

      for (var i = 0; i < SDL.audio.numSimultaneouslyQueuedBuffers; ++i) {
        // Only queue new data if we don't have enough audio data already in queue. Otherwise skip this time slot
        // and wait to queue more in the next time the callback is run.
        var secsUntilNextPlayStart = SDL.audio.nextPlayTime - SDL.audioContext['currentTime'];
        if (secsUntilNextPlayStart >= SDL.audio.bufferingDelay + SDL.audio.bufferDurationSecs*SDL.audio.numSimultaneouslyQueuedBuffers) return;

        // Ask SDL audio data from the user code.
        SDL.audio.callback(SDL.audio.userdata, SDL.audio.buffer, SDL.audio.bufferSize);
        // And queue it to be played after the currently playing audio stream.
        SDL.audio.pushAudio(SDL.audio.buffer, SDL.audio.bufferSize);
      }
    }

    // Create a callback function that will be routinely called to ask more audio data from the user application.
    SDL.audio.caller = function SDL_audioCaller() {
      if (!SDL.audio) return;

      --SDL.audio.numAudioTimersPending;

      SDL.audio.queueNewAudioData();

      // Queue this callback function to be called again later to pull more audio data.
      var secsUntilNextPlayStart = SDL.audio.nextPlayTime - SDL.audioContext['currentTime'];

      // Queue the next audio frame push to be performed half-way when the previously queued buffer has finished playing.
      var preemptBufferFeedSecs = SDL.audio.bufferDurationSecs/2.0;

      if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
        ++SDL.audio.numAudioTimersPending;
        SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, Math.max(0.0, 1000.0*(secsUntilNextPlayStart-preemptBufferFeedSecs)));

        // If we are risking starving, immediately queue an extra buffer.
        if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
          ++SDL.audio.numAudioTimersPending;
          Browser.safeSetTimeout(SDL.audio.caller, 1.0);
        }
      }
    };

    SDL.audio.audioOutput = new Audio();

    // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
    // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
    SDL.openAudioContext();
    if (!SDL.audioContext) throw 'Web Audio API is not available!';
    autoResumeAudioContext(SDL.audioContext);
    SDL.audio.nextPlayTime = 0; // Time in seconds when the next audio block is due to start.

    // The pushAudio function with a new audio buffer whenever there is new audio data to schedule to be played back on the device.
    SDL.audio.pushAudio=function(ptr,sizeBytes) {
      try {
        if (SDL.audio.paused) return;

        var sizeSamples = sizeBytes / SDL.audio.bytesPerSample; // How many samples fit in the callback buffer?
        var sizeSamplesPerChannel = sizeSamples / SDL.audio.channels; // How many samples per a single channel fit in the cb buffer?
        if (sizeSamplesPerChannel != SDL.audio.samples) {
          throw 'Received mismatching audio buffer size!';
        }
        // Allocate new sound buffer to be played.
        var source = SDL.audioContext['createBufferSource']();
        var soundBuffer = SDL.audioContext['createBuffer'](SDL.audio.channels,sizeSamplesPerChannel,SDL.audio.freq);
        source['connect'](SDL.audioContext['destination']);

        SDL.fillWebAudioBufferFromHeap(ptr, sizeSamplesPerChannel, soundBuffer);
        // Workaround https://bugzilla.mozilla.org/show_bug.cgi?id=883675 by setting the buffer only after filling. The order is important here!
        source['buffer'] = soundBuffer;

        // Schedule the generated sample buffer to be played out at the correct time right after the previously scheduled
        // sample buffer has finished.
        var curtime = SDL.audioContext['currentTime'];
        // Don't ever start buffer playbacks earlier from current time than a given constant 'SDL.audio.bufferingDelay', since a browser
        // may not be able to mix that audio clip in immediately, and there may be subsequent jitter that might cause the stream to starve.
        var playtime = Math.max(curtime + SDL.audio.bufferingDelay, SDL.audio.nextPlayTime);
        if (typeof source['start'] !== 'undefined') {
          source['start'](playtime); // New Web Audio API: sound sources are started with a .start() call.
        } else if (typeof source['noteOn'] !== 'undefined') {
          source['noteOn'](playtime); // Support old Web Audio API specification which had the .noteOn() API.
        }
        /*
        // Uncomment to debug SDL buffer feed starves.
        if (SDL.audio.curBufferEnd) {
          var thisBufferStart = Math.round(playtime * SDL.audio.freq);
          if (thisBufferStart != SDL.audio.curBufferEnd) console.log('SDL starved ' + (thisBufferStart - SDL.audio.curBufferEnd) + ' samples!');
        }
        SDL.audio.curBufferEnd = Math.round(playtime * SDL.audio.freq + sizeSamplesPerChannel);
        */

        SDL.audio.nextPlayTime = playtime + SDL.audio.bufferDurationSecs;
      } catch(e) {
        console.log('Web Audio API error playing back audio: ' + e.toString());
      }
    }

    if (obtained) {
      // Report back the initialized audio parameters.
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.freq)>>2] = SDL.audio.freq
      HEAP16[(obtained+C_STRUCTS.SDL_AudioSpec.format)>>1] = SDL.audio.format;
      HEAP8[obtained+C_STRUCTS.SDL_AudioSpec.channels] = SDL.audio.channels;
      HEAP8[obtained+C_STRUCTS.SDL_AudioSpec.silence] = SDL.audio.silence;
      HEAP16[(obtained+C_STRUCTS.SDL_AudioSpec.samples)>>1] = SDL.audio.samples;
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.callback)>>2] = SDL.audio.callback;
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.userdata)>>2] = SDL.audio.userdata;
    }
    SDL.allocateChannels(32);

  } catch(e) {
    console.log('Initializing SDL audio threw an exception: "' + e.toString() + '"! Continuing without audio.');
    SDL.audio = null;
    SDL.allocateChannels(0);
    if (obtained) {
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.freq)>>2] = 0;
      HEAP16[(obtained+C_STRUCTS.SDL_AudioSpec.format)>>1] = 0;
      HEAP8[obtained+C_STRUCTS.SDL_AudioSpec.channels] = 0;
      HEAP8[obtained+C_STRUCTS.SDL_AudioSpec.silence] = 0;
      HEAP16[(obtained+C_STRUCTS.SDL_AudioSpec.samples)>>1] = 0;
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.callback)>>2] = 0;
      HEAP32[(obtained+C_STRUCTS.SDL_AudioSpec.userdata)>>2] = 0;
    }
  }
  if (!SDL.audio) {
    return -1;
  }
  return 0;
}

function SDL_PauseAudio (pauseOn) {
  if (!SDL.audio) {
    return;
  }
  if (pauseOn) {
    if (SDL.audio.timer !== undefined) {
      clearTimeout(SDL.audio.timer);
      SDL.audio.numAudioTimersPending = 0;
      SDL.audio.timer = undefined;
    }
  } else if (!SDL.audio.timer) {
    // Start the audio playback timer callback loop.
    SDL.audio.numAudioTimersPending = 1;
    SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, 1);
  }
  SDL.audio.paused = pauseOn;
}

function SDL_CloseAudio() {
  if (SDL.audio) {
    if (SDL.audio.callbackRemover) {
      SDL.audio.callbackRemover();
      SDL.audio.callbackRemover = null;
    }
    _SDL_PauseAudio(1);
    _free(SDL.audio.buffer);
    SDL.audio = null;
    SDL.allocateChannels(0);
  }
}
