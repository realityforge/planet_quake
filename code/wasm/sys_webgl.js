var LibraryWebGL3 = {
  $GL3__postset: 'var GLctx;',
  $GL3__deps: [
    '_webgl_enable_ANGLE_instanced_arrays',
    '_webgl_enable_OES_vertex_array_object',
    '_webgl_enable_WEBGL_draw_buffers',
    '_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
    '_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance',
    '_webgl_enable_WEBGL_multi_draw',
    ],
  $GL3: {
#if GL_DEBUG
    debug: true,
#endif
    recordError: function recordError(errorCode) {
    #if GL_TRACK_ERRORS
      if (!GL3.lastError) {
        GL3.lastError = errorCode;
      }
    #endif
    },
    createContext: function(canvas, webGLContextAttributes) {
      webGLContextAttributes.failIfMajorPerformanceCaveat = true
      var ctx = (webGLContextAttributes.majorVersion > 1)
        ? canvas.getContext("webgl2", webGLContextAttributes)
        : (canvas.getContext("webgl", webGLContextAttributes)
          || canvas.getContext('experimental-webgl'))
      if (!ctx) return 0
      var handle = GL3.registerContext(ctx, webGLContextAttributes)
      return handle
    },
  },

  // TODO: list every GL function with a special prefix for renderers to switch between modes
  //   sick of trying to figure out what compile options makes this work, brute force is necessary
  //   Quake 3 seems to stack compatibility on top of each other with SDL and qgl.h
  //   Emscripten doesn't seem to have a concept of this stacking and it's one or the other
  $emscriptenWebGLGetBufferBinding: function(target) {
    switch (target) {
      case 0x8892 /*GL_ARRAY_BUFFER*/: target = 0x8894 /*GL_ARRAY_BUFFER_BINDING*/; break;
      case 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/: target = 0x8895 /*GL_ELEMENT_ARRAY_BUFFER_BINDING*/; break;
      case 0x88EB /*GL_PIXEL_PACK_BUFFER*/: target = 0x88ED /*GL_PIXEL_PACK_BUFFER_BINDING*/; break;
      case 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/: target = 0x88EF /*GL_PIXEL_UNPACK_BUFFER_BINDING*/; break;
      case 0x8C8E /*GL_TRANSFORM_FEEDBACK_BUFFER*/: target = 0x8C8F /*GL_TRANSFORM_FEEDBACK_BUFFER_BINDING*/; break;
      case 0x8F36 /*GL_COPY_READ_BUFFER*/: target = 0x8F36 /*GL_COPY_READ_BUFFER_BINDING*/; break;
      case 0x8F37 /*GL_COPY_WRITE_BUFFER*/: target = 0x8F37 /*GL_COPY_WRITE_BUFFER_BINDING*/; break;
      case 0x8A11 /*GL_UNIFORM_BUFFER*/: target = 0x8A28 /*GL_UNIFORM_BUFFER_BINDING*/; break;
      // In default case, fall through and assume passed one of the _BINDING enums directly.
    }
    var buffer = GLctx.getParameter(target);
    if (buffer) return buffer.name|0;
    else return 0;
  },

  $emscriptenWebGLValidateMapBufferTarget: function(target) {
    switch (target) {
      case 0x8892: // GL_ARRAY_BUFFER
      case 0x8893: // GL_ELEMENT_ARRAY_BUFFER
      case 0x8F36: // GL_COPY_READ_BUFFER
      case 0x8F37: // GL_COPY_WRITE_BUFFER
      case 0x88EB: // GL_PIXEL_PACK_BUFFER
      case 0x88EC: // GL_PIXEL_UNPACK_BUFFER
      case 0x8C2A: // GL_TEXTURE_BUFFER
      case 0x8C8E: // GL_TRANSFORM_FEEDBACK_BUFFER
      case 0x8A11: // GL_UNIFORM_BUFFER
        return true;
      default:
        return false;
    }
  },

  glMapBufferRange__sig: 'iiiii',
  glMapBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glMapBufferRange: function(target, offset, length, access) {
    if (access != 0x1A && access != 0xA) {
      err("glMapBufferRange is only supported when access is MAP_WRITE|INVALIDATE_BUFFER");
      return 0;
    }

    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL3.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glMapBufferRange');
      return 0;
    }

    var mem = _malloc(length);
    if (!mem) return 0;

    GL3.mappedBuffers[emscriptenWebGLGetBufferBinding(target)] = {
      offset: offset,
      length: length,
      mem: mem,
      access: access,
    };
    return mem;
  },

  glGetBufferPointerv__sig: 'viii',
  glGetBufferPointerv__deps: ['$emscriptenWebGLGetBufferBinding'],
  glGetBufferPointerv: function(target, pname, params) {
    if (pname == 0x88BD/*GL_BUFFER_MAP_POINTER*/) {
      var ptr = 0;
      var mappedBuffer = GL3.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
      if (mappedBuffer) {
        ptr = mappedBuffer.mem;
      }
      {{{ makeSetValue('params', '0', 'ptr', 'i32') }}};
    } else {
      GL3.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glGetBufferPointerv');
    }
  },

  glFlushMappedBufferRange__sig: 'viii',
  glFlushMappedBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glFlushMappedBufferRange: function(target, offset, length) {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL3.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glFlushMappedBufferRange');
      return;
    }

    var mapping = GL3.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
    if (!mapping) {
      GL3.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glFlushMappedBufferRange');
      return;
    }

    if (!(mapping.access & 0x10)) {
      GL3.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange');
      return;
    }
    if (offset < 0 || length < 0 || offset + length > mapping.length) {
      GL3.recordError(0x501 /* GL_INVALID_VALUE */);
      err('invalid range in glFlushMappedBufferRange');
      return;
    }

    GLctx.bufferSubData(
      target,
      mapping.offset,
      HEAPU8.subarray(mapping.mem + offset, mapping.mem + offset + length));
  },

  glUnmapBuffer__sig: 'ii',
  glUnmapBuffer__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glUnmapBuffer: function(target) {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL3.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var buffer = emscriptenWebGLGetBufferBinding(target);
    var mapping = GL3.mappedBuffers[buffer];
    if (!mapping) {
      GL3.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glUnmapBuffer');
      return 0;
    }
    GL3.mappedBuffers[buffer] = null;

    if (!(mapping.access & 0x10)) /* GL_MAP_FLUSH_EXPLICIT_BIT */
      if (GL3.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL3. Use those always when possible.
        GLctx.bufferSubData(target, mapping.offset, HEAPU8, mapping.mem, mapping.length);
      } else {
        GLctx.bufferSubData(target, mapping.offset, HEAPU8.subarray(mapping.mem, mapping.mem+mapping.length));
      }
    _free(mapping.mem);
    return 1;
  },
  
  glReadBuffer__sig: 'vi',
  glReadBuffer: function (buf) {},

  eglGetProcAddress__sig: 'ii',
  eglGetProcAddress: function (){
    // not going to use but something is referencing it even with dlopen
    throw new Error('not using this, fixed only')
  },
  glBegin__sig: 'vi',
  glBegin: function(mode){}, // TODO
  glClearDepth__sig: 'vf',
  glClearDepth: function (depth) {},
  
  glPolygonMode__sig: 'vii',
  glPolygonMode: function (face, mode) {
    // TODO
  },
  glDrawBuffer__sig: 'vi',
  glDrawBuffer: function (mode) {
    // TODO
  },
  glBindTexture__sig: 'vii',
  glBindTexture: function(target, texture) {
#if GL_ASSERTIONS
    GL3.validateGLObjectID(GL3.textures, texture, 'glBindTexture', 'texture');
#endif
    GLctx.bindTexture(target, GL3.textures[texture]);
  },
  glGetStringi__deps: ['$stringToNewUTF8'],
  glGetStringi__sig: 'iii',
  glGetStringi: function(name, index) {
    if (GL3.currentContext.version < 2) {
      GL3.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
      return 0;
    }
    var stringiCache = GL3.stringiCache[name];
    if (stringiCache) {
      if (index < 0 || index >= stringiCache.length) {
        GL3.recordError(0x501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
        err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ')!');
#endif
        return 0;
      }
      return stringiCache[index];
    }
    switch (name) {
      case 0x1F03 /* GL_EXTENSIONS */:
        var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
        exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
#endif
        exts = exts.map(function(e) { return stringToNewUTF8(e); });

        stringiCache = GL3.stringiCache[name] = exts;
        if (index < 0 || index >= stringiCache.length) {
          GL3.recordError(0x501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
          err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ') in a call to GL_EXTENSIONS!');
#endif
          return 0;
        }
        return stringiCache[index];
      default:
        GL3.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetStringi: Unknown parameter ' + name + '!');
#endif
        return 0;
    }
  },
  glBlitFramebuffer__sig: 'viiiiiiiiii',
  glBlitFramebuffer: function () {},
  glRenderbufferStorageMultisample__sig: 'viiiii',
  glRenderbufferStorageMultisample: function () {},

  glDrawElements__sig: 'viiii',
  glDrawElements: function(mode, count, type, indices) {
    var buf;
    if (!GL3.currElementArrayBuffer) {
      var size = GL3.calcBufLength(1, type, 0, count);
      buf = GL3.getTempIndexBuffer(size);
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, buf);
      GLctx.bufferSubData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/,
                               0,
                               HEAPU8.subarray(indices, indices + size));
      // the index is now 0
      indices = 0;
    }

    // bind any client-side buffers
    GL3.preDrawHandleClientVertexAttribBindings(count);

    GLctx.drawElements(mode, count, type, indices);

    GL3.postDrawHandleClientVertexAttribBindings(count);

    if (!GL3.currElementArrayBuffer) {
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
    }
  },

  /*
  glPolygonMode: function(){}, // TODO
  glDrawBuffer: function(){},
  glActiveTextureARB: function(){ return _glActiveTexture.apply(null, Array.from(arguments)) },
  glBindBufferARB: function(){ return _glBindBuffer.apply(null, Array.from(arguments)) },
  glDeleteProgramsARB: function(){}, // TODO
  SDL_uclibc_exp: function(){},
  SDL_uclibc_fmod: function(){},
  SDL_uclibc_log10: function(){},
  __cxa_find_matching_catch_3: function (){},
  __cxa_find_matching_catch_2: function (){},
  */

};

autoAddDeps(LibrarySDL, '$GL3');
autoAddDeps(LibraryWebGL3, 'glBindTexture')
mergeInto(LibraryManager.library, LibraryWebGL3);
