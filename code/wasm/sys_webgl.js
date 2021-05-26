var LibraryWebGL3 = {

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
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glMapBufferRange');
      return 0;
    }

    var mem = _malloc(length);
    if (!mem) return 0;

    GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)] = {
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
      var mappedBuffer = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
      if (mappedBuffer) {
        ptr = mappedBuffer.mem;
      }
      {{{ makeSetValue('params', '0', 'ptr', 'i32') }}};
    } else {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glGetBufferPointerv');
    }
  },

  glFlushMappedBufferRange__sig: 'viii',
  glFlushMappedBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glFlushMappedBufferRange: function(target, offset, length) {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glFlushMappedBufferRange');
      return;
    }

    var mapping = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
    if (!mapping) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glFlushMappedBufferRange');
      return;
    }

    if (!(mapping.access & 0x10)) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange');
      return;
    }
    if (offset < 0 || length < 0 || offset + length > mapping.length) {
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
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
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var buffer = emscriptenWebGLGetBufferBinding(target);
    var mapping = GL.mappedBuffers[buffer];
    if (!mapping) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glUnmapBuffer');
      return 0;
    }
    GL.mappedBuffers[buffer] = null;

    if (!(mapping.access & 0x10)) /* GL_MAP_FLUSH_EXPLICIT_BIT */
      if ({{{ isCurrentContextWebGL2() }}}) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
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
    GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
#endif
    GLctx.bindTexture(target, GL.textures[texture]);
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


autoAddDeps(LibraryWebGL3, 'glBindTexture')
mergeInto(LibraryManager.library, LibraryWebGL3);
