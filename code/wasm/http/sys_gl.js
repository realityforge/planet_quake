
var GLEmulation

const MAX_TEMP_BUFFER_SIZE = 1024

GLEmulation = {
  textures: [],
  // in non-dll mode, this just returns the exported function returned from importing
  GL_GetProcAddress: function (fn) {
    // TODO: renderer1/renderer2
  },
  glDisable: function () {},
  glEnable: function () {},
  glProgramLocalParameter4fARB: function () {},
  glProgramLocalParameter4fvARB: function () {},
  glPolygonOffset: function () {},
  glLockArraysEXT: function () {},
  glUnlockArraysEXT: function () {},
  glGetIntegerv: function (pname, param) {
    switch (pname) {
      case 0x8872 /* GL_MAX_TEXTURE_IMAGE_UNITS */: 
        Q3e.paged32[(param) >> 2] = Q3e.webgl.MAX_TEXTURE_IMAGE_UNITS
        break
      case 0x0D33 /* GL_MAX_TEXTURE_SIZE */:
        Q3e.paged32[(param) >> 2] = Q3e.webgl.MAX_TEXTURE_SIZE
        break
      case 0x8B4D /* GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS */:
        Q3e.paged32[(param) >> 2] = Q3e.webgl.MAX_COMBINED_TEXTURE_IMAGE_UNITS
        break
      case 0x864B /* GL_PROGRAM_ERROR_POSITION_ARB */:
        // TODO: copy from renderer2 requirements
        break
      default:
        debugger
    }
  },
  glGetError: function () {},
  glGetString: function (id) {
    switch(id) {
      case 0x1F03 /* GL_EXTENSIONS */:
        return stringToAddress(Q3e.webgl.getSupportedExtensions().join(' '))
      case 0x1F00 /* GL_VENDOR */:
      case 0x1F01 /* GL_RENDERER */:
      case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
      case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
      case 0x1F02 /* GL_VERSION */:
      case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
        return stringToAddress('' + Q3e.webgl.getParameter(id))
      case 0x8874 /* GL_PROGRAM_ERROR_STRING_ARB */ :
        break
      default:
        return 0

    }
  },
  glDeleteProgramsARB: function () {},
  glBindRenderbuffer: function () { debugger },
  glDeleteRenderbuffers: function () {},
  glFramebufferTexture2D: function () {},
  glDeleteTextures: function () {},
  glDeleteFramebuffers: function () {},
  glBindFramebuffer: function () {},
  glBlitFramebuffer: function () {},
  glViewport: function (x, y, w, h) {
    Q3e.webgl.viewport(x, y, w, h);
  },
  glScissor: function () {},
  glMatrixMode: function () {},
  glLoadMatrixf: function () {},
  glLoadIdentity: function () {},
  glColor4f: function (r, g, b, a) {
    Q3e.webgl.colorMask(r, g, b, a)
  },
  glDrawArrays: function (mode, start, end) {
    //Q3e.webgl.drawArrays(mode, start, end);
  },
  glDrawBuffer: function () { /* do nothing */ },
  glClearColor: function (r, g, b, a) {
    Q3e.webgl.clearColor(r, g, b, a);
  },
  glClear: function (bits) {
    Q3e.webgl.clear(bits);
  },
  glColorMask: function () {},
  glGenFramebuffers: function () { debugger },
  glGenRenderbuffers: function () {},
  glRenderbufferStorageMultisample: function () {},
  glFramebufferRenderbuffer: function () {},
  glCheckFramebufferStatus: function () {},
  glGenTextures: function (n, textures) {
    //GLEmulation.textures.push(Q3e.webgl.createTexture())
    Q3e.paged32[textures >> 2] = GLEmulation.textures.length
  },
  glTexParameteri: function (target, pname, param) { 
    if(param == 0x2900) {
      param = 0x812F /*GL_CLAMP_TO_EDGE*/
    }
    //Q3e.webgl.texParameteri(target, pname, param) 
  },
  colorChannels: [
    /* 0x1902 /* GL_DEPTH_COMPONENT */  1,
    /* 0x1906 /* GL_ALPHA */            1,
    /* 0x1907 /* GL_RGB */              3,
    /* 0x1908 /* GL_RGBA */             4,
    /* 0x1909 /* GL_LUMINANCE */        1,
    /* 0x190A /*GL_LUMINANCE_ALPHA*/    2,
    /* 0x8C40 /*(GL_SRGB_EXT)*/         3,
    /* 0x8C42 /*(GL_SRGB_ALPHA_EXT*/    4,
// webgl2
    /* 0x1903 /* GL_RED */              1,
    /* 0x8227 /*GL_RG*/                 2,
    /* 0x8228 /*GL_RG_INTEGER*/         2,
    /* 0x8D94 /* GL_RED_INTEGER */      1,
    /* 0x8D98 /*GL_RGB_INTEGER*/        3,
    /* 0x8D99 /*GL_RGBA_INTEGER*/       4
  ],
  glTexImage2D: function (target, level, internalFormat, width, height, border, format, type, pixels) {
    if(target != 0x0DE1) {
      debugger
    }
    if(type == 0x1401
      && format == 0x1907 && internalFormat == 0x1908) {
        internalFormat = 0x881B/*GL_RGB16F*/;
        //format = 0x1908;
        //internalFormat = 0x1908;
        //type =
      }
    if (format == 0x1902/*GL_DEPTH_COMPONENT*/ && internalFormat == 0x1902/*GL_DEPTH_COMPONENT*/ && type == 0x1405/*GL_UNSIGNED_INT*/) {
      internalFormat = 0x81A6 /*GL_DEPTH_COMPONENT24*/;
    }
    if (type == 0x8d61/*GL_HALF_FLOAT_OES*/) {
      type = 0x140B /*GL_HALF_FLOAT*/;
      if (format == 0x1908/*GL_RGBA*/ && internalFormat == 0x1908/*GL_RGBA*/) {
        internalFormat = 0x881A/*GL_RGBA16F*/;
      }
    }
    if (internalFormat == 0x84f9 /*GL_DEPTH_STENCIL*/) {
      internalFormat = 0x88F0 /*GL_DEPTH24_STENCIL8*/;
    }

    let computedSize = width * height * GLEmulation.colorChannels[format - 0x1902]
    //let imageView = Q3e.paged.subarray(pixels, pixels + computedSize)
    console.log('format:', internalFormat, format, type)
    Q3e.webgl.texImage2D(target, level, internalFormat, width, height, border, format, type, null) 
  },
  glGetInternalformativ: function () {},
  glBindTexture: function (target, id) { 
    //Q3e.webgl.bindTexture(target, GLEmulation.textures[id - 1]) 
  },
  glActiveTextureARB: function (unit) { 
    //Q3e.webgl.activeTexture(unit) 
  },
  glCullFace: function (side) { 
    //Q3e.webgl.cullFace(side) 
  },
  glTexEnvi: function () {},
  glDepthFunc: function (depth) { 
    //Q3e.webgl.depthFunc(depth) 
  },
  glBlendFunc: function () {},
  glDepthMask: function (mask) { 
    //Q3e.webgl.depthMask(mask) 
  },
  glPolygonMode: function () {},
  glAlphaFunc: function () {},

  getAttributeFromCapability: function(cap) {
    var attrib = null;
    switch (cap) {
      case 0xDE1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
        // Fall through:
      case 0x8078: // GL_TEXTURE_COORD_ARRAY
        attrib = TEXTURE0 + GLEmulation.clientActiveTexture; break;
      case 0x8074: // GL_VERTEX_ARRAY
        attrib = VERTEX; break;
      case 0x8075: // GL_NORMAL_ARRAY
        attrib = NORMAL; break;
      case 0x8076: // GL_COLOR_ARRAY
        attrib = COLOR; break;
    }
    return attrib;
  },

  glEnableClientState: function (cap) { 
    if(typeof GLEmulation.indexBuffers[1] == 'undefined') {
      GLEmulation.indexBuffers[1] = Q3e.webgl.createBuffer()
    }
    if(cap == /* GL_VERTEX_ARRAY */ 0x8074) {
      // setup GLSL program
      if(typeof GLEmulation.positionBuffer == 'undefined'
        && GLEmulation.attribPointers.attr_Position > -1) {
          // set up some buffers
        if(typeof GLEmulation.texcoordBuffer == 'undefined') {
          GLEmulation.positionBuffer = Q3e.webgl.createBuffer()
        }
        Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, GLEmulation.positionBuffer)
        Q3e.webgl.enableVertexAttribArray(GLEmulation.attribPointers.attr_Position);
        Q3e.webgl.useProgram(GLEmulation.programPointers[0])

        // must have initial for 2D drawing?
        //GLEmulation.glBufferDataARB(Q3e.webgl.ELEMENT_ARRAY_BUFFER, 6, 0, Q3e.webgl.STATIC_DRAW)
        // set the resolution
        Q3e.webgl.uniform2f(GLEmulation.uniforms.u_resolution, Q3e.canvas.width, Q3e.canvas.height);
      }
    } else if (cap == /* GL_TEXTURE_COORD_ARRAY */ 0x8078
        && GLEmulation.attribPointers.attr_TexCoord0 > -1) {
        if(typeof GLEmulation.texcoordBuffer == 'undefined') {
          GLEmulation.texcoordBuffer = Q3e.webgl.createBuffer()
        }
        Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, GLEmulation.texcoordBuffer)
        Q3e.webgl.enableVertexAttribArray(GLEmulation.attribPointers.attr_TexCoord0);
        Q3e.webgl.useProgram(GLEmulation.programPointers[0])
    }
  },
  glDisableClientState: function(cap) {
    if(cap == /* GL_VERTEX_ARRAY */	0x8074) {
      Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, GLEmulation.positionBuffer)
      Q3e.webgl.disableVertexAttribArray(GLEmulation.attribPointers.attr_Position);
    }
  },
  glClientActiveTextureARB: function () {},
  glTexSubImage2D: function (a1, a2, a3, a4, a5, a6, a7) { 
    //Q3e.webgl.texSubImage2D(a1, a2, a3, a4, a5, a6, a7) 
  },
  glFinish: function () {
    debugger
  },
  glDepthRange: function (range) { 
    // TODO: will this file always load after the context? How to reload if its in a var?
    //WebGL2RenderingContext.glDepthRange.bind(Q3e.webgl)
    //Q3e.webgl.depthRange(range) 
  },
  glPushMatrix: function () {},
  glPopMatrix: function () {},
  glReadPixels: function () {},
  glClearDepth: function () {},
  glShadeModel: function () {},

  log2ceilLookup: function(i) {
    return 32 - Math.clz32(i === 0 ? 0 : i - 1)
  },

  // Returns a random integer from 0 to range - 1.
  randomInt: function randomInt(range) {
    return Math.floor(Math.random() * range);
  },

  // code above this line is initialization code
  // --------------------------------
  // code below this line is rendering code
  texcoordStride: 0,
  texcoordSize: 2,
  texcoordPointer: null,
  glTexCoordPointer: function (size, type, stride, pointer) {
    GLEmulation.texcoordStride = stride
    GLEmulation.texcoordSize = size
    GLEmulation.texcoordPointer = pointer
  },
  glNormalPointer: function (type, stride, pointer) {
    //debugger
  },
  glColorPointer: function (size, type, stride, pointer) {
    //debugger
  },

  vertexStride: 0,
  vertexSize: 2,
  vertexPointer: null,
  glVertexPointer: function (size, type, stride, pointer) {
    GLEmulation.vertexStride = stride
    GLEmulation.vertexSize = size
    GLEmulation.vertexPointer = pointer

    // wtf... can't buffer the data here because we need to know the count from the index call
    //   when drawelements or drawarrays is called
    //Q3e.webgl.vertexAttribPointer(
    //  GLEmulation.attribPointers.attr_Position, size, type, false, stride, 
    //  Q3e.paged.subarray(pointer, pointer + size));
  },

  glDrawElements: function (mode, count, type, indices, start, end) {
     

    Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, GLEmulation.indexBuffers[1])
    Q3e.webgl.bufferData(
      Q3e.webgl.ELEMENT_ARRAY_BUFFER, 
      Q3e.paged32.subarray(
        indices >> 2,                   // start 
        (indices >> 2) + count), // end
      Q3e.webgl.STATIC_DRAW)

    Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, GLEmulation.texcoordBuffer)
    Q3e.webgl.bufferData(
      Q3e.webgl.ARRAY_BUFFER,
      Q3e.paged32f.subarray(
        GLEmulation.texcoordPointer >> 2,  // start
        (GLEmulation.texcoordPointer >> 2) // end
          + (count * (GLEmulation.texcoordSize + GLEmulation.texcoordStride))),
      Q3e.webgl.STATIC_DRAW)
    Q3e.webgl.vertexAttribPointer(
      GLEmulation.attribPointers.attr_TexCoord0, GLEmulation.texcoordSize, Q3e.webgl.FLOAT, false, GLEmulation.texcoordStride, 0);
  
    Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, GLEmulation.positionBuffer)
    Q3e.webgl.bufferData(
      Q3e.webgl.ARRAY_BUFFER,
      Q3e.paged32f.subarray(
        GLEmulation.vertexPointer >> 2,  // start
        (GLEmulation.vertexPointer >> 2) // end
          + (count * (GLEmulation.vertexSize + GLEmulation.vertexStride))),
      Q3e.webgl.STATIC_DRAW)
    Q3e.webgl.vertexAttribPointer(
      GLEmulation.attribPointers.attr_Position, GLEmulation.vertexSize, Q3e.webgl.FLOAT, false, GLEmulation.vertexStride, 0);

  
    // Set a random color.
    Q3e.webgl.uniform4f(GLEmulation.uniforms.u_BaseColor, 
      Math.random(), Math.random(), Math.random(), 1);
    if(mode != Q3e.webgl.TRIANGLES) {
      debugger
    }
    if(!Q3e.initialized) {
      Q3e.initialized = true
      document.body.className += ' done-loading '
    }

    Q3e.webgl.drawElements(
      Q3e.webgl.TRIANGLES, count, 
      Q3e.webgl.UNSIGNED_INT, 0)
    // TODO: there is a right time to reset all this?
    //Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, null);

  },
  glGetBooleanv: function () {},
  glLineWidth: function () { },
  glStencilFunc: function () {},
  glStencilOp: function () {},
  glMultiTexCoord2fARB: function () {},
  glGenBuffersARB: function () { debugger },
  glDeleteBuffersARB: function () {},
  glBindBufferARB: function (target, buffer) { 
    debugger
    if (target == 0x8892 /*GL_ARRAY_BUFFER*/) {
      Q3e.webgl.currentArrayBufferBinding = buffer;
    } else if (target == 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/) {
      Q3e.webgl.currentElementArrayBufferBinding = buffer;
    }

  },
  numIndexBuffers: 0,
  indexBuffers: {},
  glBufferDataARB: function (type, count) {
    debugger
    if(type != Q3e.webgl.ELEMENT_ARRAY_BUFFER)
      throw new Error('Don\'t know what to do!')

    if(count == 0) {
      count = MAX_TEMP_BUFFER_SIZE
    }
    let numIndexes = MAX_TEMP_BUFFER_SIZE
    let quadIndexes = new Uint16Array(MAX_TEMP_BUFFER_SIZE)

    let i = 0, v = 0;
    while (1) {
      quadIndexes[i++] = v;
      if (i >= numIndexes) break;
      quadIndexes[i++] = v+1;
      if (i >= numIndexes) break;
      quadIndexes[i++] = v+2;
      if (i >= numIndexes) break;
      quadIndexes[i++] = v+2;
      if (i >= numIndexes) break;
      quadIndexes[i++] = v+1;
      if (i >= numIndexes) break;
      quadIndexes[i++] = v+3;
      if (i >= numIndexes) break;
      v += 4;
    }

    Q3e.webgl.bufferData(
      Q3e.webgl.ELEMENT_ARRAY_BUFFER,
      quadIndexes,
      //new Uint16Array([
      //  0, 1, 2,   // first triangle
      //  2, 1, 3,   // second triangle
      //]),
      Q3e.webgl.STATIC_DRAW
    )

    Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, null)

    return numIndexes
  },
  glRenderbufferStorage: function () {},
  glGetRenderbufferParameteriv: function () {},
  glIsFramebuffer: function () {},
  glGetFramebufferAttachmentParameteriv: function () {},


  /* --------------  ARB */
  // replace ARB with realy glsl shaders?

  programPointers: [],
  numProgramPointers: 0,
  currentProgram: 0,
  glProgramStringARB: function (kind, encoding, length, text) {
    if(GLEmulation.currentProgram < 2) {
      let newShader
      if(kind == 0x8620 /* GL_VERTEX_PROGRAM_ARB */) {
        newShader = Q3e.webgl.createShader(Q3e.webgl.VERTEX_SHADER);
      } else {
        newShader = Q3e.webgl.createShader(Q3e.webgl.FRAGMENT_SHADER);
      }
      Q3e.webgl.shaderSource(newShader, SHADERS[GLEmulation.currentProgram]);
      Q3e.webgl.compileShader(newShader);
      if (!Q3e.webgl.getShaderParameter(newShader, Q3e.webgl.COMPILE_STATUS)) {
        console.log(Q3e.webgl.getShaderInfoLog(newShader));
        throw new Error(`Error compiling ${kind == 0x8620?'vertex':'fragment'} shader:`, Q3e.webgl.getShaderInfoLog(newShader));

      } else {
        Q3e.webgl.attachShader(GLEmulation.programPointers[0], newShader);
      }

    }
  },

  glBindProgramARB: function (kind, pointer) {
    GLEmulation.currentProgram = pointer

    if(pointer > 1 
      && typeof GLEmulation.programPointers[pointer] != 'undefined') {
      //GLEmulation.currentProgram = 1
    }

  },

  glGenProgramsARB: function (size, programs) {
    if(GLEmulation.numProgramPointers == 0) {
      let program = GLEmulation.programPointers[0] = Q3e.webgl.createProgram()
      GLEmulation.glBindProgramARB(0x8620 /* GL_VERTEX_PROGRAM_ARB */, 0)
      GLEmulation.glProgramStringARB(0x8620)
      GLEmulation.glBindProgramARB(0x8804 /* GL_FRAGMENT_PROGRAM_ARB */, 1)
      GLEmulation.glProgramStringARB(0x8804)

      //Q3e.webgl.bindAttribLocation(program, Q3e.webgl.ATTR_INDEX_POSITION, 'attr_Position')
      //Q3e.webgl.bindAttribLocation(program, Q3e.webgl.ATTR_INDEX_TEXCOORD, 'attr_TexCoord0')
      //Q3e.webgl.bindAttribLocation(program, Q3e.webgl.ATTR_INDEX_NORMAL, 'attr_Normal')
      //Q3e.webgl.bindAttribLocation(program, Q3e.webgl.ATTR_INDEX_COLOR, 'attr_Color')

      Q3e.webgl.linkProgram(program)

      // TODO: move this to bind program?
      if(GLEmulation.currentProgram > 1) {
        Q3e.webgl.useProgram(GLEmulation.programPointers[0])
      }

      GLEmulation.attribPointers = {
        attr_Position: Q3e.webgl.getAttribLocation(program, "attr_Position"),
        attr_TexCoord0: Q3e.webgl.getAttribLocation(program, "attr_TexCoord0"),
      }

      GLEmulation.uniforms = {
        u_resolution: Q3e.webgl.getUniformLocation(program, "u_resolution"),
        u_BaseColor: Q3e.webgl.getUniformLocation(program, "u_BaseColor"),
      }

    }
    for(let i = 0; i < size; i++) {
      GLEmulation.programPointers[++GLEmulation.numProgramPointers] = 
        Q3e.webgl.createProgram()
      Q3e.paged32[(programs >> 2) + i] = GLEmulation.numProgramPointers
    }
  },

}


const SHADERS = [
  `
  precision mediump float;

  attribute vec3 attr_Position;
  attribute vec3 attr_Normal;
  attribute vec4 attr_Color;
  attribute vec4 attr_TexCoord0;

  uniform mat4   u_ModelViewProjectionMatrix;
  uniform vec4   u_BaseColor;
  uniform vec4   u_VertColor;

  // TODO: remove
  uniform vec2   u_resolution;
 
  varying vec2   var_DiffuseTex;
  varying vec4   var_Color;

  void main() {
    vec2 position = vec2(attr_Position.x, attr_Position.y);
    vec2 tex = attr_TexCoord0.st;
    //var_Color = vec4(1) * attr_Color + u_BaseColor;
    var_Color = u_BaseColor;
    var_DiffuseTex = tex;

    /*
    vec3 normal    = attr_Normal;
    gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);
    vec2 tex = attr_TexCoord0.st;
    */

    vec2 zeroToOne = position / u_resolution;
    vec2 zeroToTwo = zeroToOne * 2.0;
    vec2 clipSpace = zeroToTwo - 1.0;
    gl_Position = vec4(clipSpace.x, -clipSpace.y, 0, 1);

  }`,
  `
  precision mediump float;
  uniform sampler2D u_DiffuseMap;
  uniform int       u_AlphaTest;
  varying vec4      var_Color;
  varying vec2      var_DiffuseTex;

  void main()
  {
    vec4 color  = texture2D(u_DiffuseMap, var_DiffuseTex);

    gl_FragColor.rgb = color.rgb * var_Color.rgb;
    gl_FragColor.a = 1.0;
    //gl_FragColor = vec4(1, 0, 0.5, 1); 
  }`,

]

