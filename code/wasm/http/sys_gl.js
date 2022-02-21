
var GLImmediate


var GLEmulation = {
  fogStart: 0,
  fogEnd: 1,
  fogDensity: 1.0,
  fogColor: null,
  fogMode: 0x800, // GL_EXP
  fogEnabled: false,

  // GL_CLIP_PLANE support
  MAX_CLIP_PLANES: 6,
  clipPlaneEnabled: [false, false, false, false, false, false],
  clipPlaneEquation: [],

  // GL_LIGHTING support
  lightingEnabled: false,

  lightModelAmbient: null,
  lightModelLocalViewer: false,
  lightModelTwoSide: false,

  materialAmbient: null,
  materialDiffuse: null,
  materialSpecular: null,
  materialShininess: null,
  materialEmission: null,

  MAX_LIGHTS: 8,
  lightEnabled: [false, false, false, false, false, false, false, false],
  lightAmbient: [],
  lightDiffuse: [],
  lightSpecular: [],
  lightPosition: [],
  // TODO attenuation modes of lights

  // GL_POINTS support.
  pointSize: 1.0,

  // VAO support
  vaos: [],
  currentVao: null,
  enabledVertexAttribArrays: {}, // helps with vao cleanups

  hasRunInit: false,
  counter: 1, // 0 is reserved as 'null' in gl
  buffers: [],
//#if FULL_ES3
  mappedBuffers: {},
//#endif
  programs: [],
  framebuffers: [],
  renderbuffers: [],
  shaders: [],
  vaos: [],
//#if USE_PTHREADS // with pthreads a context is a location in memory with some synchronized data between threads
  contexts: {},
//#else            // without pthreads, it's just an integer ID
//  contexts: [],
//#endif
  offscreenCanvases: {}, // DOM ID -> OffscreenCanvas mappings of <canvas> elements that have their rendering control transferred to offscreen.
  queries: [], // on WebGL1 stores WebGLTimerQueryEXT, on WebGL2 WebGLQuery
//#if MAX_WEBGL_VERSION >= 2
  samplers: [],
  transformFeedbacks: [],
  syncs: [],
//#endif

//#if FULL_ES2 || LEGACY_GL_EMULATION
  numTempVertexBuffersPerSize: 64, // (const)
  byteSizeByTypeRoot: 0x1400, // GL_BYTE
  byteSizeByType: [
    1, // GL_BYTE
    1, // GL_UNSIGNED_BYTE
    2, // GL_SHORT
    2, // GL_UNSIGNED_SHORT
    4, // GL_INT
    4, // GL_UNSIGNED_INT
    4, // GL_FLOAT
    2, // GL_2_BYTES
    3, // GL_3_BYTES
    4, // GL_4_BYTES
    8  // GL_DOUBLE
  ],
//#endif

  stringCache: {},
//#if MAX_WEBGL_VERSION >= 2
  stringiCache: {},
//#endif

  unpackAlignment: 4,
  textures: [],
  colorChannels: [
    // 0x1902 /* GL_DEPTH_COMPONENT */: 1,
    // 0x1906 /* GL_ALPHA */: 1,
    /* 0x1907 /* GL_RGB */  3,
    /* 0x1908 /* GL_RGBA */  4,
    // 0x1909 /* GL_LUMINANCE */: 1,
    /* 0x190A /*GL_LUMINANCE_ALPHA*/  2,
    /* 0x8C40 /*(GL_SRGB_EXT)*/  3,
    /* 0x8C42 /*(GL_SRGB_ALPHA_EXT*/  4,
// webgl2
    // 0x1903 /* GL_RED */: 1,
    /* 0x8227 /*GL_RG*/  2,
    /* 0x8228 /*GL_RG_INTEGER*/  2,
    // 0x8D94 /* GL_RED_INTEGER */: 1,
    /* 0x8D98 /*GL_RGB_INTEGER*/  3,
    /* 0x8D99 /*GL_RGBA_INTEGER*/  4
  ],
  MAX_TEMP_BUFFER_SIZE: 2097152,
  // in non-dll mode, this just returns the exported function returned from importing
  GL_GetProcAddress: function (fn) {
    if (!GLEmulation.restrideBuffer) GLEmulation.restrideBuffer = malloc(GLEmulation.MAX_TEMP_BUFFER_SIZE);
    // TODO: renderer1/renderer2
  },
  glDisable: function () {},
  glEnable: function () {},
  glBindProgramARB: function () { },
  glProgramLocalParameter4fARB: function () {},
  glProgramLocalParameter4fvARB: function () {},
  glPolygonOffset: function () {},
  glTexCoordPointer: function (size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture, size, type, stride, pointer);
    if (Q3e.webgl.currentArrayBufferBinding) {
      var loc = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture;
      Q3e.webgl.vertexAttribPointer(loc, size, type, false, stride, pointer);
    }
  },
  glNormalPointer: function (type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.NORMAL, 3, type, stride, pointer);
    if (Q3e.webgl.currentArrayBufferBinding) {
      Q3e.webgl.vertexAttribPointer(GLImmediate.NORMAL, 3, type, true, stride, pointer);
    }
  },
  glVertexPointer: function (size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.VERTEX, size, type, stride, pointer);
    if (Q3e.webgl.currentArrayBufferBinding) {
      Q3e.webgl.vertexAttribPointer(GLImmediate.VERTEX, size, type, false, stride, pointer);
    }
  },
  glColorPointer: function (size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.COLOR, size, type, stride, pointer);
    if (Q3e.webgl.currentArrayBufferBinding) {
      Q3e.webgl.vertexAttribPointer(GLImmediate.COLOR, size, type, true, stride, pointer);
    }
  },

  glLockArraysEXT: function () {},
  glUnlockArraysEXT: function () {},
  glProgramStringARB: function () {},
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
        // TODO: make something up?
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
  glGenProgramsARB: function () {},
  glBindRenderbuffer: function () { debugger },
  glDeleteRenderbuffers: function () {},
  glFramebufferTexture2D: function () {},
  glDeleteTextures: function () {},
  glDeleteFramebuffers: function () {},
  glBindFramebuffer: function () {},
  glBlitFramebuffer: function () {},
  glViewport: function () {},
  glScissor: function () {},
  glMatrixMode: function () {},
  glLoadMatrixf: function () {},
  glLoadIdentity: function () {},
  glColor4f: function () {},
  glDrawArrays: function () {
    debugger
  },
  glDrawBuffer: function () { /* do nothing */ },
  glClearColor: function () { debugger },
  glClear: function () {},
  glColorMask: function () {},
  glGenFramebuffers: function () { debugger },
  glGenRenderbuffers: function () {},
  glRenderbufferStorageMultisample: function () {},
  glFramebufferRenderbuffer: function () {},
  glCheckFramebufferStatus: function () {},
  glGenTextures: function (n, textures) {
    GLEmulation.textures.push(Q3e.webgl.createTexture())
    Q3e.paged32[textures >> 2] = GLEmulation.textures.length
  },
  glTexParameteri: function (target, pname, param) { 
    if(param == 0x2900) {
      param = 0x812F /*GL_CLAMP_TO_EDGE*/
    }
    Q3e.webgl.texParameteri(target, pname, param) 
  },
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
  glBindTexture: function (target, id) { Q3e.webgl.bindTexture(target, GLEmulation.textures[id - 1]) },
  glActiveTextureARB: function (unit) { Q3e.webgl.activeTexture(unit) },
  glCullFace: function (side) { Q3e.webgl.cullFace(side) },
  glTexEnvi: function () {},
  glDepthFunc: function (depth) { Q3e.webgl.depthFunc(depth) },
  glBlendFunc: function () {},
  glDepthMask: function (mask) { Q3e.webgl.depthMask(mask) },
  glPolygonMode: function () {},
  glAlphaFunc: function () {},

  getAttributeFromCapability: function(cap) {
    var attrib = null;
    switch (cap) {
      case 0xDE1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
        // Fall through:
      case 0x8078: // GL_TEXTURE_COORD_ARRAY
        attrib = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture; break;
      case 0x8074: // GL_VERTEX_ARRAY
        attrib = GLImmediate.VERTEX; break;
      case 0x8075: // GL_NORMAL_ARRAY
        attrib = GLImmediate.NORMAL; break;
      case 0x8076: // GL_COLOR_ARRAY
        attrib = GLImmediate.COLOR; break;
    }
    return attrib;
  },

  enabledClientAttribIndices: [],
  disableVertexAttribArray: function disableVertexAttribArray(index) {
    if (GLEmulation.enabledClientAttribIndices[index]) {
      GLEmulation.enabledClientAttribIndices[index] = false;
      Q3e.webgl.disableVertexAttribArray(index);
    }
  },
  enableVertexAttribArray: function enableVertexAttribArray(index) {
    if (!GLEmulation.enabledClientAttribIndices[index]) {
      GLEmulation.enabledClientAttribIndices[index] = true;
      Q3e.webgl.enableVertexAttribArray(index);
    }
  },

  glEnableClientState: function (cap) { 
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
      return;
    }
    if (!GLImmediate.enabledClientAttributes[attrib]) {
      GLImmediate.enabledClientAttributes[attrib] = true;
      GLImmediate.totalEnabledClientAttributes++;
      GLImmediate.currentRenderer = null; // Will need to change current renderer, since the set of active vertex pointers changed.
//#if GL_FFP_ONLY
      // In GL_FFP_ONLY mode, attributes are bound to the same index in each FFP emulation shader, so we can immediately apply the change here.
      GLEmulation.enableVertexAttribArray(attrib);
//#endif
      if (GLEmulation.currentVao) GLEmulation.currentVao.enabledClientStates[cap] = 1;
      GLImmediate.modifiedClientAttributes = true;
    }
  },
  glDisableClientState: function(cap) {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
      return;
    }
    if (GLImmediate.enabledClientAttributes[attrib]) {
      GLImmediate.enabledClientAttributes[attrib] = false;
      GLImmediate.totalEnabledClientAttributes--;
      GLImmediate.currentRenderer = null; // Will need to change current renderer, since the set of active vertex pointers changed.
  //#if GL_FFP_ONLY
      // In GL_FFP_ONLY mode, attributes are bound to the same index in each FFP emulation shader, so we can immediately apply the change here.
  //    GL.disableVertexAttribArray(attrib);
  //#endif
      if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledClientStates[cap];
      GLImmediate.modifiedClientAttributes = true;
    }
  },
  glClientActiveTextureARB: function () {},
  glTexSubImage2D: function (a1, a2, a3, a4, a5, a6, a7) { 
    Q3e.webgl.texSubImage2D(a1, a2, a3, a4, a5, a6, a7) 
  },
  glFinish: function () {
    debugger
  },
  glDepthRange: function (range) { Q3e.webgl.depthRange(range) },
  glPushMatrix: function () {},
  glPopMatrix: function () {},
  glReadPixels: function () {},
  glClearDepth: function () {},
  glShadeModel: function () {},

  log2ceilLookup: function(i) {
    return 32 - Math.clz32(i === 0 ? 0 : i - 1)
  },
  tempIndexBuffers: [],

  getTempVertexBuffer: function getTempVertexBuffer(sizeBytes) {
    var idx = GLEmulation.log2ceilLookup(sizeBytes);
    var ringbuffer = Q3e.webgl.tempVertexBuffers1[idx];
    var nextFreeBufferIndex = Q3e.webgl.tempVertexBufferCounters1[idx];
    Q3e.webgl.tempVertexBufferCounters1[idx] = (Q3e.webgl.tempVertexBufferCounters1[idx]+1) & (GLEmulation.numTempVertexBuffersPerSize-1);
    var vbo = ringbuffer[nextFreeBufferIndex];
    if (vbo) {
      return vbo;
    }
    var prevVBO = Q3e.webgl.getParameter(0x8894 /*GL_ARRAY_BUFFER_BINDING*/);
    ringbuffer[nextFreeBufferIndex] = Q3e.webgl.createBuffer();
    Q3e.webgl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, ringbuffer[nextFreeBufferIndex]);
    Q3e.webgl.bufferData(0x8892 /*GL_ARRAY_BUFFER*/, 1 << idx, 0x88E8 /*GL_DYNAMIC_DRAW*/);
    Q3e.webgl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, prevVBO);
    return ringbuffer[nextFreeBufferIndex];
  },

  getTempIndexBuffer: function getTempIndexBuffer(sizeBytes) {
    var idx = GLEmulation.log2ceilLookup(sizeBytes)
    var ibo = Q3e.webgl.tempIndexBuffers[idx]
    if (ibo) {
      return ibo;
    }
    var prevIBO = Q3e.webgl.getParameter(0x8895 /*ELEMENT_ARRAY_BUFFER_BINDING*/)
    Q3e.webgl.tempIndexBuffers[idx] = Q3e.webgl.createBuffer()
    Q3e.webgl.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, Q3e.webgl.tempIndexBuffers[idx])
    Q3e.webgl.bufferData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, 1 << idx, 0x88E8 /*GL_DYNAMIC_DRAW*/);
    Q3e.webgl.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, prevIBO)
    return Q3e.webgl.tempIndexBuffers[idx];
  },

  // Returns a random integer from 0 to range - 1.
  randomInt: function randomInt(range) {
    return Math.floor(Math.random() * range);
  },
  
  // Fill the buffer with the values that define a rectangle.
  setRectangle: function (gl, x, y, width, height) {
    var x1 = x;
    var x2 = x + width;
    var y1 = y;
    var y2 = y + height;
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
       x1, y1,
       x2, y1,
       x1, y2,
       x2, y2,
    ]), gl.STATIC_DRAW);
  },

  glDrawElements: function (mode, count, type, indices, start, end) {
    //Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, arrayBuffer);
    //let vertexData = Q3e.paged.subarray(GLEmulation.vertexPointer >> 2, end ? (GLEmulation.vertexPointer + (end+1)*GLImmediate.stride) >> 2 : void 0)
    //let indexBuffer = GLEmulation.getTempIndexBuffer(count << 1)
    //let vertexData = Q3e.paged32.slice(start >> 2, end >> 2)
    //Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, indexBuffer)
    //Q3e.webgl.bufferSubData(Q3e.webgl.ELEMENT_ARRAY_BUFFER, start, vertexData)
    //Q3e.webgl.drawElements(mode, count, Q3e.webgl.UNSIGNED_SHORT, indices)
    if (GLImmediate.totalEnabledClientAttributes == 0 && mode <= 6 && Q3e.webgl.currentElementArrayBufferBinding) {
      Q3e.webgl.drawElements(mode, count, type, indices);
      return;
    }

    GLImmediate.prepareClientAttributes(count, false);
    GLImmediate.mode = mode;
    if (!Q3e.webgl.currentArrayBufferBinding) {
      GLImmediate.firstVertex = end ? start : Q3e.paged.length; // if we don't know the start, set an invalid value and we will calculate it later from the indices
      GLImmediate.lastVertex = end ? end+1 : 0;
      GLImmediate.vertexData = Q3e.paged32f.subarray(GLImmediate.vertexPointer >> 2, end ? (GLImmediate.vertexPointer + (end+1)*GLImmediate.stride) >> 2 : undefined); // XXX assuming float
    }
    GLImmediate.flush(count, 0, indices);
    GLImmediate.mode = -1;

/*

    // setup GLSL program
    var program = webglUtils.createProgramFromScripts(gl, ["vertex-shader-2d", "fragment-shader-2d"]);
  
    // look up where the vertex data needs to go.
    var positionAttributeLocation = gl.getAttribLocation(program, "a_position");
  
    // look up uniform locations
    var resolutionUniformLocation = gl.getUniformLocation(program, "u_resolution");
    var colorUniformLocation = gl.getUniformLocation(program, "u_color");
  
    // Create a buffer to put three 2d clip space points in
    var positionBuffer = gl.createBuffer();
  
    // Bind it to ARRAY_BUFFER (think of it as ARRAY_BUFFER = positionBuffer)
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
  
    //webglUtils.resizeCanvasToDisplaySize(gl.canvas);
  
    // Tell WebGL how to convert from clip space to pixels
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
  
    // Clear the canvas
    gl.clearColor(0, 0, 0, 0);
    gl.clear(gl.COLOR_BUFFER_BIT);
  
    // Tell it to use our program (pair of shaders)
    gl.useProgram(program);
  
    // Bind the position buffer.
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
  
    // create the buffer
    const indexBuffer = gl.createBuffer();
  
    // make this buffer the current 'ELEMENT_ARRAY_BUFFER'
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
  
    // Fill the current element array buffer with data
    const indices = [
      0, 1, 2,   // first triangle
      2, 1, 3,   // second triangle
    ];
    gl.bufferData(
        gl.ELEMENT_ARRAY_BUFFER,
        new Uint16Array(indices),
        gl.STATIC_DRAW
    );
  
    // code above this line is initialization code
    // --------------------------------
    // code below this line is rendering code
  
    // Turn on the attribute
    gl.enableVertexAttribArray(positionAttributeLocation);
  
    // Tell the attribute how to get data out of positionBuffer (ARRAY_BUFFER)
    var size = 2;          // 2 components per iteration
    var type = gl.FLOAT;   // the data is 32bit floats
    var normalize = false; // don't normalize the data
    var stride = 0;        // 0 = move forward size * sizeof(type) each iteration to get the next position
    var offset = 0;        // start at the beginning of the buffer
    gl.vertexAttribPointer(
        positionAttributeLocation, size, type, normalize, stride, offset);
  
    // bind the buffer containing the indices
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
  
    // set the resolution
    gl.uniform2f(resolutionUniformLocation, gl.canvas.width, gl.canvas.height);
  
    // draw 50 random rectangles in random colors
    for (var ii = 0; ii < 50; ++ii) {
      // Setup a random rectangle
      // This will write to positionBuffer because
      // its the last thing we bound on the ARRAY_BUFFER
      // bind point
      setRectangle(
          gl, randomInt(300), randomInt(300), randomInt(300), randomInt(300));
  
      // Set a random color.
      gl.uniform4f(colorUniformLocation, Math.random(), Math.random(), Math.random(), 1);
  
      // Draw the rectangle.
      var primitiveType = gl.TRIANGLES;
      var offset = 0;
      var count = 6;
      var indexType = gl.UNSIGNED_SHORT;
      gl.drawElements(primitiveType, count, indexType, offset);
    }

    */

    //if(mode <= 6 && Q3e.webgl.currentElementArrayBufferBinding) {
    //  Q3e.webgl.drawElements(mode, count, type, indices);
    //} else {
  },
  glGetBooleanv: function () {},
  glLineWidth: function () {},
  glStencilFunc: function () {},
  glStencilOp: function () {},
  glMultiTexCoord2fARB: function () {},
  glGenBuffersARB: function () { debugger },
  glDeleteBuffersARB: function () {},
  glBindBufferARB: function (target, buffer) { 
    debugger
    if (target == 0x8892 /*GL_ARRAY_BUFFER*/) {
      Q3e.webgl.currentArrayBufferBinding = buffer;
//#if LEGACY_GL_EMULATION
      GLImmediate.lastArrayBuffer = buffer;
//#endif
    } else if (target == 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/) {
      Q3e.webgl.currentElementArrayBufferBinding = buffer;
    }

  },
  glBufferDataARB: function () {},
  glRenderbufferStorage: function () {},
  glGetRenderbufferParameteriv: function () {},
  glIsFramebuffer: function () {},
  glGetFramebufferAttachmentParameteriv: function () {},

  generateTempBuffers: function(quads, context) {
    var largestIndex = GLEmulation.log2ceilLookup(GLEmulation.MAX_TEMP_BUFFER_SIZE);
    context.tempVertexBufferCounters1 = [];
    context.tempVertexBufferCounters2 = [];
    context.tempVertexBufferCounters1.length = context.tempVertexBufferCounters2.length = largestIndex+1;
    context.tempVertexBuffers1 = [];
    context.tempVertexBuffers2 = [];
    context.tempVertexBuffers1.length = context.tempVertexBuffers2.length = largestIndex+1;
    context.tempIndexBuffers = [];
    context.tempIndexBuffers.length = largestIndex+1;
    for (var i = 0; i <= largestIndex; ++i) {
      context.tempIndexBuffers[i] = null; // Created on-demand
      context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
      var ringbufferLength = GLEmulation.numTempVertexBuffersPerSize;
      context.tempVertexBuffers1[i] = [];
      context.tempVertexBuffers2[i] = [];
      var ringbuffer1 = context.tempVertexBuffers1[i];
      var ringbuffer2 = context.tempVertexBuffers2[i];
      ringbuffer1.length = ringbuffer2.length = ringbufferLength;
      for (var j = 0; j < ringbufferLength; ++j) {
        ringbuffer1[j] = ringbuffer2[j] = null; // Created on-demand
      }
    }

    if (quads) {
      // GL_QUAD indexes can be precalculated
      context.tempQuadIndexBuffer = Q3e.webgl.createBuffer();
      Q3e.webgl.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, context.tempQuadIndexBuffer);
      var numIndexes = GLEmulation.MAX_TEMP_BUFFER_SIZE >> 1;
      var quadIndexes = new Uint16Array(numIndexes);
      var i = 0, v = 0;
      while (1) {
        quadIndexes[i++] = v;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v+1;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v+2;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v+2;
        if (i >= numIndexes) break;
        quadIndexes[i++] = v+3;
        if (i >= numIndexes) break;
        v += 4;
      }
      Q3e.webgl.bufferData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, quadIndexes, 0x88E4 /*GL_STATIC_DRAW*/);
      Q3e.webgl.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
    }
  },

}

function $GLImmediate() {
  setTimeout(this.init, 10)
}
$GLImmediate.prototype.constructor = $GLImmediate
Object.assign($GLImmediate.prototype, {
  MapTreeLib: null,
  spawnMapTreeLib: function() {
    /* A naive implementation of a map backed by an array, and accessed by
     * naive iteration along the array. (hashmap with only one bucket)
     */
    function CNaiveListMap() {
      var list = [];

      this.insert = function CNaiveListMap_insert(key, val) {
        if (this.contains(key|0)) return false;
        list.push([key, val]);
        return true;
      };

      var __contains_i;
      this.contains = function CNaiveListMap_contains(key) {
        for (__contains_i = 0; __contains_i < list.length; ++__contains_i) {
          if (list[__contains_i][0] === key) return true;
        }
        return false;
      };

      var __get_i;
      this.get = function CNaiveListMap_get(key) {
        for (__get_i = 0; __get_i < list.length; ++__get_i) {
          if (list[__get_i][0] === key) return list[__get_i][1];
        }
        return undefined;
      };
    };

    /* A tree of map nodes.
      Uses `KeyView`s to allow descending the tree without garbage.
      Example: {
        // Create our map object.
        var map = new ObjTreeMap();

        // Grab the static keyView for the map.
        var keyView = map.GetStaticKeyView();

        // Let's make a map for:
        // root: <undefined>
        //   1: <undefined>
        //     2: <undefined>
        //       5: "Three, sir!"
        //       3: "Three!"

        // Note how we can chain together `Reset` and `Next` to
        // easily descend based on multiple key fragments.
        keyView.Reset().Next(1).Next(2).Next(5).Set("Three, sir!");
        keyView.Reset().Next(1).Next(2).Next(3).Set("Three!");
      }
    */
    function CMapTree() {
      function CNLNode() {
        var map = new CNaiveListMap();

        this.child = function CNLNode_child(keyFrag) {
          if (!map.contains(keyFrag|0)) {
            map.insert(keyFrag|0, new CNLNode());
          }
          return map.get(keyFrag|0);
        };

        this.value = undefined;
        this.get = function CNLNode_get() {
          return this.value;
        };

        this.set = function CNLNode_set(val) {
          this.value = val;
        };
      }

      function CKeyView(root) {
        var cur;

        this.reset = function CKeyView_reset() {
          cur = root;
          return this;
        };
        this.reset();

        this.next = function CKeyView_next(keyFrag) {
          cur = cur.child(keyFrag);
          return this;
        };

        this.get = function CKeyView_get() {
          return cur.get();
        };

        this.set = function CKeyView_set(val) {
          cur.set(val);
        };
      };

      var root;
      var staticKeyView;

      this.createKeyView = function CNLNode_createKeyView() {
        return new CKeyView(root);
      }

      this.clear = function CNLNode_clear() {
        root = new CNLNode();
        staticKeyView = this.createKeyView();
      };
      this.clear();

      this.getStaticKeyView = function CNLNode_getStaticKeyView() {
        staticKeyView.reset();
        return staticKeyView;
      };
    };

    // Exports:
    return {
      create: function() {
        return new CMapTree();
      },
    };
  },

  TexEnvJIT: null,
  spawnTexEnvJIT: function() {
    // GL defs:
    var GL_TEXTURE0 = 0x84C0;
    var GL_TEXTURE_1D = 0xDE0;
    var GL_TEXTURE_2D = 0xDE1;
    var GL_TEXTURE_3D = 0x806f;
    var GL_TEXTURE_CUBE_MAP = 0x8513;
    var GL_TEXTURE_ENV = 0x2300;
    var GL_TEXTURE_ENV_MODE = 0x2200;
    var GL_TEXTURE_ENV_COLOR = 0x2201;
    var GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
    var GL_TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
    var GL_TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
    var GL_TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
    var GL_TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
    var GL_TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;

    var GL_SRC0_RGB = 0x8580;
    var GL_SRC1_RGB = 0x8581;
    var GL_SRC2_RGB = 0x8582;

    var GL_SRC0_ALPHA = 0x8588;
    var GL_SRC1_ALPHA = 0x8589;
    var GL_SRC2_ALPHA = 0x858A;

    var GL_OPERAND0_RGB = 0x8590;
    var GL_OPERAND1_RGB = 0x8591;
    var GL_OPERAND2_RGB = 0x8592;

    var GL_OPERAND0_ALPHA = 0x8598;
    var GL_OPERAND1_ALPHA = 0x8599;
    var GL_OPERAND2_ALPHA = 0x859A;

    var GL_COMBINE_RGB = 0x8571;
    var GL_COMBINE_ALPHA = 0x8572;

    var GL_RGB_SCALE = 0x8573;
    var GL_ALPHA_SCALE = 0xD1C;

    // env.mode
    var GL_ADD      = 0x104;
    var GL_BLEND    = 0xBE2;
    var GL_REPLACE  = 0x1E01;
    var GL_MODULATE = 0x2100;
    var GL_DECAL    = 0x2101;
    var GL_COMBINE  = 0x8570;

    // env.color/alphaCombiner
    //var GL_ADD         = 0x104;
    //var GL_REPLACE     = 0x1E01;
    //var GL_MODULATE    = 0x2100;
    var GL_SUBTRACT    = 0x84E7;
    var GL_INTERPOLATE = 0x8575;

    // env.color/alphaSrc
    var GL_TEXTURE       = 0x1702;
    var GL_CONSTANT      = 0x8576;
    var GL_PRIMARY_COLOR = 0x8577;
    var GL_PREVIOUS      = 0x8578;

    // env.color/alphaOp
    var GL_SRC_COLOR           = 0x300;
    var GL_ONE_MINUS_SRC_COLOR = 0x301;
    var GL_SRC_ALPHA           = 0x302;
    var GL_ONE_MINUS_SRC_ALPHA = 0x303;

    var GL_RGB  = 0x1907;
    var GL_RGBA = 0x1908;

    // Our defs:
    var TEXENVJIT_NAMESPACE_PREFIX = "tej_";
    // Not actually constant, as they can be changed between JIT passes:
    var TEX_UNIT_UNIFORM_PREFIX = "uTexUnit";
    var TEX_COORD_VARYING_PREFIX = "vTexCoord";
    var PRIM_COLOR_VARYING = "vPrimColor";
    var TEX_MATRIX_UNIFORM_PREFIX = "uTexMatrix";

    // Static vars:
    var s_texUnits = null; //[];
    var s_activeTexture = 0;

    var s_requiredTexUnitsForPass = [];

    // Static funcs:
    function abort(info) {
      assert(false, "[TexEnvJIT] ABORT: " + info);
    }

    function abort_noSupport(info) {
      abort("No support: " + info);
    }

    function abort_sanity(info) {
      abort("Sanity failure: " + info);
    }

    function genTexUnitSampleExpr(texUnitID) {
      var texUnit = s_texUnits[texUnitID];
      var texType = texUnit.getTexType();

      var func = null;
      switch (texType) {
        case GL_TEXTURE_1D:
          func = "texture2D";
          break;
        case GL_TEXTURE_2D:
          func = "texture2D";
          break;
        case GL_TEXTURE_3D:
          return abort_noSupport("No support for 3D textures.");
        case GL_TEXTURE_CUBE_MAP:
          func = "textureCube";
          break;
        default:
          return abort_sanity("Unknown texType: 0x" + texType.toString(16));
      }

      var texCoordExpr = TEX_COORD_VARYING_PREFIX + texUnitID;
      if (TEX_MATRIX_UNIFORM_PREFIX != null) {
        texCoordExpr = "(" + TEX_MATRIX_UNIFORM_PREFIX + texUnitID + " * " + texCoordExpr + ")";
      }
      return func + "(" + TEX_UNIT_UNIFORM_PREFIX + texUnitID + ", " + texCoordExpr + ".xy)";
    }

    function getTypeFromCombineOp(op) {
      switch (op) {
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
          return "vec3";
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
          return "float";
      }

      return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
    }

    function getCurTexUnit() {
      return s_texUnits[s_activeTexture];
    }

    function genCombinerSourceExpr(texUnitID, constantExpr, previousVar,
                                   src, op)
    {
      var srcExpr = null;
      switch (src) {
        case GL_TEXTURE:
          srcExpr = genTexUnitSampleExpr(texUnitID);
          break;
        case GL_CONSTANT:
          srcExpr = constantExpr;
          break;
        case GL_PRIMARY_COLOR:
          srcExpr = PRIM_COLOR_VARYING;
          break;
        case GL_PREVIOUS:
          srcExpr = previousVar;
          break;
        default:
            return abort_noSupport("Unsupported combiner src: 0x" + src.toString(16));
      }

      var expr = null;
      switch (op) {
        case GL_SRC_COLOR:
          expr = srcExpr + ".rgb";
          break;
        case GL_ONE_MINUS_SRC_COLOR:
          expr = "(vec3(1.0) - " + srcExpr + ".rgb)";
          break;
        case GL_SRC_ALPHA:
          expr = srcExpr + ".a";
          break;
        case GL_ONE_MINUS_SRC_ALPHA:
          expr = "(1.0 - " + srcExpr + ".a)";
          break;
        default:
          return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
      }

      return expr;
    }

    function valToFloatLiteral(val) {
      if (val == Math.round(val)) return val + '.0';
      return val;
    }


    // Classes:
    function CTexEnv() {
      this.mode = GL_MODULATE;
      this.colorCombiner = GL_MODULATE;
      this.alphaCombiner = GL_MODULATE;
      this.colorScale = 1;
      this.alphaScale = 1;
      this.envColor = [0, 0, 0, 0];

      this.colorSrc = [
        GL_TEXTURE,
        GL_PREVIOUS,
        GL_CONSTANT
      ];
      this.alphaSrc = [
        GL_TEXTURE,
        GL_PREVIOUS,
        GL_CONSTANT
      ];
      this.colorOp = [
        GL_SRC_COLOR,
        GL_SRC_COLOR,
        GL_SRC_ALPHA
      ];
      this.alphaOp = [
        GL_SRC_ALPHA,
        GL_SRC_ALPHA,
        GL_SRC_ALPHA
      ];

      // Map GLenums to small values to efficiently pack the enums to bits for tighter access.
      this.traverseKey = {
        // mode
        0x1E01 /* GL_REPLACE */: 0,
        0x2100 /* GL_MODULATE */: 1,
        0x104 /* GL_ADD */: 2,
        0xBE2 /* GL_BLEND */: 3,
        0x2101 /* GL_DECAL */: 4,
        0x8570 /* GL_COMBINE */: 5,

        // additional color and alpha combiners
        0x84E7 /* GL_SUBTRACT */: 3,
        0x8575 /* GL_INTERPOLATE */: 4,

        // color and alpha src
        0x1702 /* GL_TEXTURE */: 0,
        0x8576 /* GL_CONSTANT */: 1,
        0x8577 /* GL_PRIMARY_COLOR */: 2,
        0x8578 /* GL_PREVIOUS */: 3,

        // color and alpha op
        0x300 /* GL_SRC_COLOR */: 0,
        0x301 /* GL_ONE_MINUS_SRC_COLOR */: 1,
        0x302 /* GL_SRC_ALPHA */: 2,
        0x303 /* GL_ONE_MINUS_SRC_ALPHA */: 3
      };

      // The tuple (key0,key1,key2) uniquely identifies the state of the variables in CTexEnv.
      // -1 on key0 denotes 'the whole cached key is dirty'
      this.key0 = -1;
      this.key1 = 0;
      this.key2 = 0;

      this.computeKey0 = function() {
        var k = this.traverseKey;
        var key = k[this.mode] * 1638400; // 6 distinct values.
        key += k[this.colorCombiner] * 327680; // 5 distinct values.
        key += k[this.alphaCombiner] * 65536; // 5 distinct values.
        // The above three fields have 6*5*5=150 distinct values -> 8 bits.
        key += (this.colorScale-1) * 16384; // 10 bits used.
        key += (this.alphaScale-1) * 4096; // 12 bits used.
        key += k[this.colorSrc[0]] * 1024; // 14
        key += k[this.colorSrc[1]] * 256; // 16
        key += k[this.colorSrc[2]] * 64; // 18
        key += k[this.alphaSrc[0]] * 16; // 20
        key += k[this.alphaSrc[1]] * 4; // 22
        key += k[this.alphaSrc[2]]; // 24 bits used total.
        return key;
      }
      this.computeKey1 = function() {
        var k = this.traverseKey;
        key = k[this.colorOp[0]] * 4096;
        key += k[this.colorOp[1]] * 1024;
        key += k[this.colorOp[2]] * 256;
        key += k[this.alphaOp[0]] * 16;
        key += k[this.alphaOp[1]] * 4;
        key += k[this.alphaOp[2]];
        return key;
      }
      // TODO: remove this. The color should not be part of the key!
      this.computeKey2 = function() {
        return this.envColor[0] * 16777216 + this.envColor[1] * 65536 + this.envColor[2] * 256 + 1 + this.envColor[3];
      }
      this.recomputeKey = function() {
        this.key0 = this.computeKey0();
        this.key1 = this.computeKey1();
        this.key2 = this.computeKey2();
      }
      this.invalidateKey = function() {
        this.key0 = -1; // The key of this texture unit must be recomputed when rendering the next time.
        GLImmediate.currentRenderer = null; // The currently used renderer must be re-evaluated at next render.
      }
    }

    function CTexUnit() {
      this.env = new CTexEnv();
      this.enabled_tex1D   = false;
      this.enabled_tex2D   = false;
      this.enabled_tex3D   = false;
      this.enabled_texCube = false;
      this.texTypesEnabled = 0; // A bitfield combination of the four flags above, used for fast access to operations.

      this.traverseState = function CTexUnit_traverseState(keyView) {
        if (this.texTypesEnabled) {
          if (this.env.key0 == -1) {
            this.env.recomputeKey();
          }
          keyView.next(this.texTypesEnabled | (this.env.key0 << 4));
          keyView.next(this.env.key1);
          keyView.next(this.env.key2);
        } else {
          // For correctness, must traverse a zero value, theoretically a subsequent integer key could collide with this value otherwise.
          keyView.next(0);
        }
      };
    };

    // Class impls:
    CTexUnit.prototype.enabled = function CTexUnit_enabled() {
      return this.texTypesEnabled;
    }

    CTexUnit.prototype.genPassLines = function CTexUnit_genPassLines(passOutputVar, passInputVar, texUnitID) {
      if (!this.enabled()) {
        return ["vec4 " + passOutputVar + " = " + passInputVar + ";"];
      }
      var lines = this.env.genPassLines(passOutputVar, passInputVar, texUnitID).join('\n');

      var texLoadLines = '';
      var texLoadRegex = /(texture.*?\(.*?\))/g;
      var loadCounter = 0;
      var load;

      // As an optimization, merge duplicate identical texture loads to one var.
      while (load = texLoadRegex.exec(lines)) {
        var texLoadExpr = load[1];
        var secondOccurrence = lines.slice(load.index+1).indexOf(texLoadExpr);
        if (secondOccurrence != -1) { // And also has a second occurrence of same load expression..
          // Create new var to store the common load.
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var texLoadVar = prefix + 'texload' + loadCounter++;
          var texLoadLine = 'vec4 ' + texLoadVar + ' = ' + texLoadExpr + ';\n';
          texLoadLines += texLoadLine + '\n'; // Store the generated texture load statements in a temp string to not confuse regex search in progress.
          lines = lines.split(texLoadExpr).join(texLoadVar);
          // Reset regex search, since we modified the string.
          texLoadRegex = /(texture.*\(.*\))/g;
        }
      }
      return [texLoadLines + lines];
    }

    CTexUnit.prototype.getTexType = function CTexUnit_getTexType() {
      if (this.enabled_texCube) {
        return GL_TEXTURE_CUBE_MAP;
      } else if (this.enabled_tex3D) {
        return GL_TEXTURE_3D;
      } else if (this.enabled_tex2D) {
        return GL_TEXTURE_2D;
      } else if (this.enabled_tex1D) {
        return GL_TEXTURE_1D;
      }
      return 0;
    }

    CTexEnv.prototype.genPassLines = function CTexEnv_genPassLines(passOutputVar, passInputVar, texUnitID) {
      switch (this.mode) {
        case GL_REPLACE: {
          /* RGB:
           * Cv = Cs
           * Av = Ap // Note how this is different, and that we'll
           *            need to track the bound texture internalFormat
           *            to get this right.
           *
           * RGBA:
           * Cv = Cs
           * Av = As
           */
          return [
            "vec4 " + passOutputVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
          ];
        }
        case GL_ADD: {
          /* RGBA:
           * Cv = Cp + Cs
           * Av = ApAs
           */
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var texVar = prefix + "tex";
          var colorVar = prefix + "color";
          var alphaVar = prefix + "alpha";

          return [
            "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
            "vec3 " + colorVar + " = " + passInputVar + ".rgb + " + texVar + ".rgb;",
            "float " + alphaVar + " = " + passInputVar + ".a * " + texVar + ".a;",
            "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
          ];
        }
        case GL_MODULATE: {
          /* RGBA:
           * Cv = CpCs
           * Av = ApAs
           */
          var line = [
            "vec4 " + passOutputVar,
            " = ",
              passInputVar,
              " * ",
              genTexUnitSampleExpr(texUnitID),
            ";",
          ];
          return [line.join("")];
        }
        case GL_DECAL: {
          /* RGBA:
           * Cv = Cp(1 - As) + CsAs
           * Av = Ap
           */
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var texVar = prefix + "tex";
          var colorVar = prefix + "color";
          var alphaVar = prefix + "alpha";

          return [
            "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
            [
              "vec3 " + colorVar + " = ",
                passInputVar + ".rgb * (1.0 - " + texVar + ".a)",
                  " + ",
                texVar + ".rgb * " + texVar + ".a",
              ";"
            ].join(""),
            "float " + alphaVar + " = " + passInputVar + ".a;",
            "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
          ];
        }
        case GL_BLEND: {
          /* RGBA:
           * Cv = Cp(1 - Cs) + CcCs
           * Av = As
           */
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var texVar = prefix + "tex";
          var colorVar = prefix + "color";
          var alphaVar = prefix + "alpha";

          return [
            "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
            [
              "vec3 " + colorVar + " = ",
                passInputVar + ".rgb * (1.0 - " + texVar + ".rgb)",
                  " + ",
                PRIM_COLOR_VARYING + ".rgb * " + texVar + ".rgb",
              ";"
            ].join(""),
            "float " + alphaVar + " = " + texVar + ".a;",
            "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
          ];
        }
        case GL_COMBINE: {
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var colorVar = prefix + "color";
          var alphaVar = prefix + "alpha";
          var colorLines = this.genCombinerLines(true, colorVar,
                                                 passInputVar, texUnitID,
                                                 this.colorCombiner, this.colorSrc, this.colorOp);
          var alphaLines = this.genCombinerLines(false, alphaVar,
                                                 passInputVar, texUnitID,
                                                 this.alphaCombiner, this.alphaSrc, this.alphaOp);

          // Generate scale, but avoid generating an identity op that multiplies by one.
          var scaledColor = (this.colorScale == 1) ? colorVar : (colorVar + " * " + valToFloatLiteral(this.colorScale));
          var scaledAlpha = (this.alphaScale == 1) ? alphaVar : (alphaVar + " * " + valToFloatLiteral(this.alphaScale));

          var line = [
            "vec4 " + passOutputVar,
            " = ",
              "vec4(",
                  scaledColor,
                  ", ",
                  scaledAlpha,
              ")",
            ";",
          ].join("");
          return [].concat(colorLines, alphaLines, [line]);
        }
      }

      return abort_noSupport("Unsupported TexEnv mode: 0x" + this.mode.toString(16));
    }

    CTexEnv.prototype.genCombinerLines = function CTexEnv_getCombinerLines(isColor, outputVar,
                                                                           passInputVar, texUnitID,
                                                                           combiner, srcArr, opArr)
    {
      var argsNeeded = null;
      switch (combiner) {
        case GL_REPLACE:
          argsNeeded = 1;
          break;

        case GL_MODULATE:
        case GL_ADD:
        case GL_SUBTRACT:
          argsNeeded = 2;
          break;

        case GL_INTERPOLATE:
          argsNeeded = 3;
          break;

        default:
          return abort_noSupport("Unsupported combiner: 0x" + combiner.toString(16));
      }

      var constantExpr = [
        "vec4(",
          valToFloatLiteral(this.envColor[0]),
          ", ",
          valToFloatLiteral(this.envColor[1]),
          ", ",
          valToFloatLiteral(this.envColor[2]),
          ", ",
          valToFloatLiteral(this.envColor[3]),
        ")",
      ].join("");
      var src0Expr = (argsNeeded >= 1) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[0], opArr[0])
                                       : null;
      var src1Expr = (argsNeeded >= 2) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[1], opArr[1])
                                       : null;
      var src2Expr = (argsNeeded >= 3) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[2], opArr[2])
                                       : null;

      var outputType = isColor ? "vec3" : "float";
      var lines = null;
      switch (combiner) {
        case GL_REPLACE: {
          var line = [
            outputType + " " + outputVar,
            " = ",
              src0Expr,
            ";",
          ];
          lines = [line.join("")];
          break;
        }
        case GL_MODULATE: {
          var line = [
            outputType + " " + outputVar + " = ",
              src0Expr + " * " + src1Expr,
            ";",
          ];
          lines = [line.join("")];
          break;
        }
        case GL_ADD: {
          var line = [
            outputType + " " + outputVar + " = ",
              src0Expr + " + " + src1Expr,
            ";",
          ];
          lines = [line.join("")];
          break;
        }
        case GL_SUBTRACT: {
          var line = [
            outputType + " " + outputVar + " = ",
              src0Expr + " - " + src1Expr,
            ";",
          ];
          lines = [line.join("")];
          break;
        }
        case GL_INTERPOLATE: {
          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
          var arg2Var = prefix + "colorSrc2";
          var arg2Line = getTypeFromCombineOp(this.colorOp[2]) + " " + arg2Var + " = " + src2Expr + ";";

          var line = [
            outputType + " " + outputVar,
            " = ",
              src0Expr + " * " + arg2Var,
              " + ",
              src1Expr + " * (1.0 - " + arg2Var + ")",
            ";",
          ];
          lines = [
            arg2Line,
            line.join(""),
          ];
          break;
        }

        default:
          return abort_sanity("Unmatched TexEnv.colorCombiner?");
      }

      return lines;
    }

    return {
      // Exports:
      init: function(gl, specifiedMaxTextureImageUnits) {
        var maxTexUnits = 0;
        if (specifiedMaxTextureImageUnits) {
          maxTexUnits = specifiedMaxTextureImageUnits;
        } else if (gl) {
          maxTexUnits = gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS);
        }
        s_texUnits = [];
        for (var i = 0; i < maxTexUnits; i++) {
          s_texUnits.push(new CTexUnit());
        }
      },

      setGLSLVars: function(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix) {
        TEX_UNIT_UNIFORM_PREFIX   = uTexUnitPrefix;
        TEX_COORD_VARYING_PREFIX  = vTexCoordPrefix;
        PRIM_COLOR_VARYING        = vPrimColor;
        TEX_MATRIX_UNIFORM_PREFIX = uTexMatrixPrefix;
      },

      genAllPassLines: function(resultDest, indentSize) {
        indentSize = indentSize || 0;

        s_requiredTexUnitsForPass.length = 0; // Clear the list.
        var lines = [];
        var lastPassVar = PRIM_COLOR_VARYING;
        for (var i = 0; i < s_texUnits.length; i++) {
          if (!s_texUnits[i].enabled()) continue;

          s_requiredTexUnitsForPass.push(i);

          var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + i + "_";
          var passOutputVar = prefix + "result";

          var newLines = s_texUnits[i].genPassLines(passOutputVar, lastPassVar, i);
          lines = lines.concat(newLines, [""]);

          lastPassVar = passOutputVar;
        }
        lines.push(resultDest + " = " + lastPassVar + ";");

        var indent = "";
        for (var i = 0; i < indentSize; i++) indent += " ";

        var output = indent + lines.join("\n" + indent);

        return output;
      },

      getUsedTexUnitList: function() {
        return s_requiredTexUnitsForPass;
      },

      getActiveTexture: function () {
        return s_activeTexture;
      },

      traverseState: function(keyView) {
        for (var i = 0; i < s_texUnits.length; i++) {
          s_texUnits[i].traverseState(keyView);
        }
      },

      getTexUnitType: function(texUnitID) {
        return s_texUnits[texUnitID].getTexType();
      },

      // Hooks:
      hook_activeTexture: function(texture) {
        s_activeTexture = texture - GL_TEXTURE0;
        // Check if the current matrix mode is GL_TEXTURE.
        if (GLImmediate.currentMatrix >= 2) {
          // Switch to the corresponding texture matrix stack.
          GLImmediate.currentMatrix = 2 + s_activeTexture;
        }
      },

      hook_enable: function(cap) {
        var cur = getCurTexUnit();
        switch (cap) {
          case GL_TEXTURE_1D:
            if (!cur.enabled_tex1D) {
              GLImmediate.currentRenderer = null; // Renderer state changed, and must be recreated or looked up again.
              cur.enabled_tex1D = true;
              cur.texTypesEnabled |= 1;
            }
            break;
          case GL_TEXTURE_2D:
            if (!cur.enabled_tex2D) {
              GLImmediate.currentRenderer = null;
              cur.enabled_tex2D = true;
              cur.texTypesEnabled |= 2;
            }
            break;
          case GL_TEXTURE_3D:
            if (!cur.enabled_tex3D) {
              GLImmediate.currentRenderer = null;
              cur.enabled_tex3D = true;
              cur.texTypesEnabled |= 4;
            }
            break;
          case GL_TEXTURE_CUBE_MAP:
            if (!cur.enabled_texCube) {
              GLImmediate.currentRenderer = null;
              cur.enabled_texCube = true;
              cur.texTypesEnabled |= 8;
            }
            break;
        }
      },

      hook_disable: function(cap) {
        var cur = getCurTexUnit();
        switch (cap) {
          case GL_TEXTURE_1D:
            if (cur.enabled_tex1D) {
              GLImmediate.currentRenderer = null; // Renderer state changed, and must be recreated or looked up again.
              cur.enabled_tex1D = false;
              cur.texTypesEnabled &= ~1;
            }
            break;
          case GL_TEXTURE_2D:
            if (cur.enabled_tex2D) {
              GLImmediate.currentRenderer = null;
              cur.enabled_tex2D = false;
              cur.texTypesEnabled &= ~2;
            }
            break;
          case GL_TEXTURE_3D:
            if (cur.enabled_tex3D) {
              GLImmediate.currentRenderer = null;
              cur.enabled_tex3D = false;
              cur.texTypesEnabled &= ~4;
            }
            break;
          case GL_TEXTURE_CUBE_MAP:
            if (cur.enabled_texCube) {
              GLImmediate.currentRenderer = null;
              cur.enabled_texCube = false;
              cur.texTypesEnabled &= ~8;
            }
            break;
        }
      },

      hook_texEnvf: function(target, pname, param) {
        if (target != GL_TEXTURE_ENV)
          return;

        var env = getCurTexUnit().env;
        switch (pname) {
          case GL_RGB_SCALE:
            if (env.colorScale != param) {
              env.invalidateKey(); // We changed FFP emulation renderer state.
              env.colorScale = param;
            }
            break;
          case GL_ALPHA_SCALE:
            if (env.alphaScale != param) {
              env.invalidateKey();
              env.alphaScale = param;
            }
            break;

          default:
            err('WARNING: Unhandled `pname` in call to `glTexEnvf`.');
        }
      },

      hook_texEnvi: function(target, pname, param) {
        if (target != GL_TEXTURE_ENV)
          return;

        var env = getCurTexUnit().env;
        switch (pname) {
          case GL_TEXTURE_ENV_MODE:
            if (env.mode != param) {
              env.invalidateKey(); // We changed FFP emulation renderer state.
              env.mode = param;
            }
            break;

          case GL_COMBINE_RGB:
            if (env.colorCombiner != param) {
              env.invalidateKey();
              env.colorCombiner = param;
            }
            break;
          case GL_COMBINE_ALPHA:
            if (env.alphaCombiner != param) {
              env.invalidateKey();
              env.alphaCombiner = param;
            }
            break;

          case GL_SRC0_RGB:
            if (env.colorSrc[0] != param) {
              env.invalidateKey();
              env.colorSrc[0] = param;
            }
            break;
          case GL_SRC1_RGB:
            if (env.colorSrc[1] != param) {
              env.invalidateKey();
              env.colorSrc[1] = param;
            }
            break;
          case GL_SRC2_RGB:
            if (env.colorSrc[2] != param) {
              env.invalidateKey();
              env.colorSrc[2] = param;
            }
            break;

          case GL_SRC0_ALPHA:
            if (env.alphaSrc[0] != param) {
              env.invalidateKey();
              env.alphaSrc[0] = param;
            }
            break;
          case GL_SRC1_ALPHA:
            if (env.alphaSrc[1] != param) {
              env.invalidateKey();
              env.alphaSrc[1] = param;
            }
            break;
          case GL_SRC2_ALPHA:
            if (env.alphaSrc[2] != param) {
              env.invalidateKey();
              env.alphaSrc[2] = param;
            }
            break;

          case GL_OPERAND0_RGB:
            if (env.colorOp[0] != param) {
              env.invalidateKey();
              env.colorOp[0] = param;
            }
            break;
          case GL_OPERAND1_RGB:
            if (env.colorOp[1] != param) {
              env.invalidateKey();
              env.colorOp[1] = param;
            }
            break;
          case GL_OPERAND2_RGB:
            if (env.colorOp[2] != param) {
              env.invalidateKey();
              env.colorOp[2] = param;
            }
            break;

          case GL_OPERAND0_ALPHA:
            if (env.alphaOp[0] != param) {
              env.invalidateKey();
              env.alphaOp[0] = param;
            }
            break;
          case GL_OPERAND1_ALPHA:
            if (env.alphaOp[1] != param) {
              env.invalidateKey();
              env.alphaOp[1] = param;
            }
            break;
          case GL_OPERAND2_ALPHA:
            if (env.alphaOp[2] != param) {
              env.invalidateKey();
              env.alphaOp[2] = param;
            }
            break;

          case GL_RGB_SCALE:
            if (env.colorScale != param) {
              env.invalidateKey();
              env.colorScale = param;
            }
            break;
          case GL_ALPHA_SCALE:
            if (env.alphaScale != param) {
              env.invalidateKey();
              env.alphaScale = param;
            }
            break;

          default:
            err('WARNING: Unhandled `pname` in call to `glTexEnvi`.');
        }
      },

      hook_texEnvfv: function(target, pname, params) {
        if (target != GL_TEXTURE_ENV) return;

        var env = getCurTexUnit().env;
        switch (pname) {
          case GL_TEXTURE_ENV_COLOR: {
            for (var i = 0; i < 4; i++) {
              var param = Q3e.paged32f[(params+i*4)>>2];
              if (env.envColor[i] != param) {
                env.invalidateKey(); // We changed FFP emulation renderer state.
                env.envColor[i] = param;
              }
            }
            break
          }
          default:
            err('WARNING: Unhandled `pname` in call to `glTexEnvfv`.');
        }
      },

      hook_getTexEnviv: function(target, pname, param) {
        if (target != GL_TEXTURE_ENV)
          return;

        var env = getCurTexUnit().env;
        switch (pname) {
          case GL_TEXTURE_ENV_MODE:
            Q3e.paged32[param>>2] = env.mode
            return;

          case GL_TEXTURE_ENV_COLOR:
            Q3e.paged32[param>>2] = Math.max(Math.min(env.envColor[0]*255, 255, -255))
            // TODO: are the addresses right here?
            Q3e.paged32[(param+4)>>2] = Math.max(Math.min(env.envColor[0]*255, 255, -255))
            Q3e.paged32[(param+8)>>2] = Math.max(Math.min(env.envColor[1]*255, 255, -255))
            Q3e.paged32[(param+16)>>2] = Math.max(Math.min(env.envColor[2]*255, 255, -255))
            Q3e.paged32[(param+24)>>2] = Math.max(Math.min(env.envColor[3]*255, 255, -255))
            //{{{ makeSetValue('param', '0', '', 'i32') }}};
            //{{{ makeSetValue('param', '1', 'Math.max(Math.min(env.envColor[1]*255, 255, -255))', 'i32') }}};
            // ...
            return;

          case GL_COMBINE_RGB:
            Q3e.paged32[param>>2] = env.colorCombiner
            return;

          case GL_COMBINE_ALPHA:
            Q3e.paged32[param>>2] = env.alphaCombiner
            return;

          case GL_SRC0_RGB:
            Q3e.paged32[param>>2] = env.colorSrc[0]
            return;

          case GL_SRC1_RGB:
            Q3e.paged32[param>>2] = env.colorSrc[1]
            return;

          case GL_SRC2_RGB:
            Q3e.paged32[param>>2] = env.colorSrc[2]
            return;

          case GL_SRC0_ALPHA:
            Q3e.paged32[param>>2] = env.alphaSrc[0]
            return;

          case GL_SRC1_ALPHA:
            Q3e.paged32[param>>2] = env.alphaSrc[1]
            return;

          case GL_SRC2_ALPHA:
            Q3e.paged32[param>>2] = env.alphaSrc[2]
            return;

          case GL_OPERAND0_RGB:
            Q3e.paged32[param>>2] = env.colorOp[0]
            return;

          case GL_OPERAND1_RGB:
            Q3e.paged32[param>>2] = env.colorOp[1]
            return;

          case GL_OPERAND2_RGB:
            Q3e.paged32[param>>2] = env.colorOp[2]
            return;

          case GL_OPERAND0_ALPHA:
            Q3e.paged32[param>>2] = env.alphaOp[0]
            return;

          case GL_OPERAND1_ALPHA:
            Q3e.paged32[param>>2] = env.alphaOp[1]
            return;

          case GL_OPERAND2_ALPHA:
            Q3e.paged32[param>>2] = env.alphaOp[2]
            return;

          case GL_RGB_SCALE:
            Q3e.paged32[param>>2] = env.colorScale
            return;

          case GL_ALPHA_SCALE:
            Q3e.paged32[param>>2] = env.alphaScale
            return;

          default:
            err('WARNING: Unhandled `pname` in call to `glGetTexEnvi`.');
        }
      },

      hook_getTexEnvfv: function(target, pname, param) {
        if (target != GL_TEXTURE_ENV)
          return;

        var env = getCurTexUnit().env;
        switch (pname) {
          case GL_TEXTURE_ENV_COLOR:
            Q3e.paged32f[(param+0)>>2] = env.envColor[0]
            Q3e.paged32f[(param+4)>>2] = env.envColor[1]
            Q3e.paged32f[(param+8)>>2] = env.envColor[2]
            Q3e.paged32f[(param+12)>>2] = env.envColor[3]
            return;
        }
      }
    };
  },

  // Vertex and index data
  vertexData: null, // current vertex data. either tempData (glBegin etc.) or a view into the heap (gl*Pointer). Default view is F32
  vertexDataU8: null, // U8 view
  tempData: null,
  indexData: null,
  vertexCounter: 0,
  mode: -1,

  rendererCache: null,
  rendererComponents: [], // small cache for calls inside glBegin/end. counts how many times the element was seen
  rendererComponentPointer: 0, // next place to start a glBegin/end component
  lastRenderer: null, // used to avoid cleaning up and re-preparing the same renderer
  lastArrayBuffer: null, // used in conjunction with lastRenderer
  lastProgram: null, // ""
  lastStride: -1, // ""

  // The following data structures are used for OpenGL Immediate Mode matrix routines.
  matrix: [],
  matrixStack: [],
  currentMatrix: 0, // 0: modelview, 1: projection, 2+i, texture matrix i.
  tempMatrix: null,
  matricesModified: false,
  useTextureMatrix: false,

  // Clientside attributes
  VERTEX: 0,
  NORMAL: 1,
  COLOR: 2,
  TEXTURE0: 3,
  NUM_ATTRIBUTES: -1, // Initialized in GL emulation init().
  MAX_TEXTURES: -1,   // Initialized in GL emulation init().

  totalEnabledClientAttributes: 0,
  enabledClientAttributes: [0, 0],
  clientAttributes: [], // raw data, including possible unneeded ones
  liveClientAttributes: [], // the ones actually alive in the current computation, sorted
  currentRenderer: null, // Caches the currently active FFP emulation renderer, so that it does not have to be re-looked up unless relevant state changes.
  modifiedClientAttributes: false,
  clientActiveTexture: 0,
  clientColor: null,
  usedTexUnitList: [],
  fixedFunctionProgram: null,

  setClientAttribute: function setClientAttribute(name, size, type, stride, pointer) {
    var attrib = GLImmediate.clientAttributes[name];
    if (!attrib) {
      for (var i = 0; i <= name; i++) { // keep flat
        if (!GLImmediate.clientAttributes[i]) {
          GLImmediate.clientAttributes[i] = {
            name: name,
            size: size,
            type: type,
            stride: stride,
            pointer: pointer,
            offset: 0
          };
        }
      }
    } else {
      attrib.name = name;
      attrib.size = size;
      attrib.type = type;
      attrib.stride = stride;
      attrib.pointer = pointer;
      attrib.offset = 0;
    }
    GLImmediate.modifiedClientAttributes = true;
  },

  // Renderers
  addRendererComponent: function addRendererComponent(name, size, type) {
    if (!GLImmediate.rendererComponents[name]) {
      GLImmediate.rendererComponents[name] = 1;
      GLImmediate.enabledClientAttributes[name] = true;
      GLImmediate.setClientAttribute(name, size, type, 0, GLImmediate.rendererComponentPointer);
      GLImmediate.rendererComponentPointer += size * GLEmulation.byteSizeByType[type - GLEmulation.byteSizeByTypeRoot];
//#if GL_FFP_ONLY
      // We can enable the correct attribute stream index immediately here, since the same attribute in each shader
      // will be bound to this same index.
      GLEmulation.enableVertexAttribArray(name);
//#endif
    } else {
      GLImmediate.rendererComponents[name]++;
    }
  },

  disableBeginEndClientAttributes: function disableBeginEndClientAttributes() {
    for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
      if (GLImmediate.rendererComponents[i]) GLImmediate.enabledClientAttributes[i] = false;
    }
  },

  getRenderer: function getRenderer() {
    // If no FFP state has changed that would have forced to re-evaluate which FFP emulation shader to use,
    // we have the currently used renderer in cache, and can immediately return that.
    if (GLImmediate.currentRenderer) {
      return GLImmediate.currentRenderer;
    }
    // return a renderer object given the liveClientAttributes
    // we maintain a cache of renderers, optimized to not generate garbage
    var attributes = GLImmediate.liveClientAttributes;
    var cacheMap = GLImmediate.rendererCache;
    var keyView = cacheMap.getStaticKeyView().reset();

    // By attrib state:
    var enabledAttributesKey = 0;
    for (var i = 0; i < attributes.length; i++) {
      enabledAttributesKey |= 1 << attributes[i].name;
    }

    // By fog state:
    var fogParam = 0;
    if (GLEmulation.fogEnabled) {
      switch (GLEmulation.fogMode) {
        case 0x801: // GL_EXP2
          fogParam = 1;
          break;
        case 0x2601: // GL_LINEAR
          fogParam = 2;
          break;
        default: // default to GL_EXP
          fogParam = 3;
          break;
      }
    }
    enabledAttributesKey = (enabledAttributesKey << 2) | fogParam;

    // By clip plane mode
    for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
      enabledAttributesKey = (enabledAttributesKey << 1) | GLEmulation.clipPlaneEnabled[clipPlaneId];
    }

    // By lighting mode and enabled lights
    enabledAttributesKey = (enabledAttributesKey << 1) | GLEmulation.lightingEnabled;
    for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
      enabledAttributesKey = (enabledAttributesKey << 1) | (GLEmulation.lightingEnabled ? GLEmulation.lightEnabled[lightId] : 0);
    }

    // By drawing mode:
    enabledAttributesKey = (enabledAttributesKey << 1) | (GLImmediate.mode == Q3e.webgl.POINTS ? 1 : 0);

    keyView.next(enabledAttributesKey);

    /*
#if !GL_FFP_ONLY
    // By cur program:
    keyView.next(GLEmulation.currProgram);
    if (!GLEmulation.currProgram) {
#endif
*/
      GLImmediate.TexEnvJIT.traverseState(keyView);

/*
#if !GL_FFP_ONLY
    }
#endif
*/

    // If we don't already have it, create it.
    var renderer = keyView.get();
    if (!renderer) {
      renderer = GLImmediate.createRenderer();
      GLImmediate.currentRenderer = renderer;
      keyView.set(renderer);
      return renderer;
    }
    GLImmediate.currentRenderer = renderer; // Cache the currently used renderer, so later lookups without state changes can get this fast.
    return renderer;
  },

  createRenderer: function createRenderer(renderer) {
    var useCurrProgram = !!GLEmulation.currProgram;
    var hasTextures = false;
    for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
      var texAttribName = GLImmediate.TEXTURE0 + i;
      if (!GLImmediate.enabledClientAttributes[texAttribName])
        continue;

      hasTextures = true;
    }

    var ret = {
      init: function init() {
        // For fixed-function shader generation.
        var uTexUnitPrefix = 'u_texUnit';
        var aTexCoordPrefix = 'a_texCoord';
        var vTexCoordPrefix = 'v_texCoord';
        var vPrimColor = 'v_color';
        var uTexMatrixPrefix = GLImmediate.useTextureMatrix ? 'u_textureMatrix' : null;

        if (useCurrProgram) {
          if (GLEmulation.shaderInfos[GLEmulation.programShaders[GLEmulation.currProgram][0]].type == Q3e.webgl.VERTEX_SHADER) {
            this.vertexShader = GLEmulation.shaders[GLEmulation.programShaders[GLEmulation.currProgram][0]];
            this.fragmentShader = GLEmulation.shaders[GLEmulation.programShaders[GLEmulation.currProgram][1]];
          } else {
            this.vertexShader = GLEmulation.shaders[GLEmulation.programShaders[GLEmulation.currProgram][1]];
            this.fragmentShader = GLEmulation.shaders[GLEmulation.programShaders[GLEmulation.currProgram][0]];
          }
          this.program = GLEmulation.programs[GLEmulation.currProgram];
          this.usedTexUnitList = [];
        } else {
          // IMPORTANT NOTE: If you parameterize the shader source based on any runtime values
          // in order to create the least expensive shader possible based on the features being
          // used, you should also update the code in the beginning of getRenderer to make sure
          // that you cache the renderer based on the said parameters.
          if (GLEmulation.fogEnabled) {
            switch (GLEmulation.fogMode) {
              case 0x801: // GL_EXP2
                // fog = exp(-(gl_Fog.density * gl_FogFragCoord)^2)
                var fogFormula = '  float fog = exp(-u_fogDensity * u_fogDensity * ecDistance * ecDistance); \n';
                break;
              case 0x2601: // GL_LINEAR
                // fog = (gl_Fog.end - gl_FogFragCoord) * gl_fog.scale
                var fogFormula = '  float fog = (u_fogEnd - ecDistance) * u_fogScale; \n';
                break;
              default: // default to GL_EXP
                // fog = exp(-gl_Fog.density * gl_FogFragCoord)
                var fogFormula = '  float fog = exp(-u_fogDensity * ecDistance); \n';
                break;
            }
          }

          GLImmediate.TexEnvJIT.setGLSLVars(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix);
          var fsTexEnvPass = GLImmediate.TexEnvJIT.genAllPassLines('gl_FragColor', 2);

          var texUnitAttribList = '';
          var texUnitVaryingList = '';
          var texUnitUniformList = '';
          var vsTexCoordInits = '';
          this.usedTexUnitList = GLImmediate.TexEnvJIT.getUsedTexUnitList();
          for (var i = 0; i < this.usedTexUnitList.length; i++) {
            var texUnit = this.usedTexUnitList[i];
            texUnitAttribList += 'attribute vec4 ' + aTexCoordPrefix + texUnit + ';\n';
            texUnitVaryingList += 'varying vec4 ' + vTexCoordPrefix + texUnit + ';\n';
            texUnitUniformList += 'uniform sampler2D ' + uTexUnitPrefix + texUnit + ';\n';
            vsTexCoordInits += '  ' + vTexCoordPrefix + texUnit + ' = ' + aTexCoordPrefix + texUnit + ';\n';

            if (GLImmediate.useTextureMatrix) {
              texUnitUniformList += 'uniform mat4 ' + uTexMatrixPrefix + texUnit + ';\n';
            }
          }

          var vsFogVaryingInit = null;
          if (GLEmulation.fogEnabled) {
            vsFogVaryingInit = '  v_fogFragCoord = abs(ecPosition.z);\n';
          }

          var vsPointSizeDefs = null;
          var vsPointSizeInit = null;
          if (GLImmediate.mode == Q3e.webgl.POINTS) {
            vsPointSizeDefs = 'uniform float u_pointSize;\n';
            vsPointSizeInit = '  gl_PointSize = u_pointSize;\n';
          }

          var vsClipPlaneDefs = '';
          var vsClipPlaneInit = '';
          var fsClipPlaneDefs = '';
          var fsClipPlanePass = '';
          for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
            if (GLEmulation.clipPlaneEnabled[clipPlaneId]) {
              vsClipPlaneDefs += 'uniform vec4 u_clipPlaneEquation' + clipPlaneId + ';';
              vsClipPlaneDefs += 'varying float v_clipDistance' + clipPlaneId + ';';
              vsClipPlaneInit += '  v_clipDistance' + clipPlaneId + ' = dot(ecPosition, u_clipPlaneEquation' + clipPlaneId + ');';
              fsClipPlaneDefs += 'varying float v_clipDistance' + clipPlaneId + ';';
              fsClipPlanePass += '  if(v_clipDistance' + clipPlaneId + ' < 0.0) discard;';
            }
          }

          var vsLightingDefs = '';
          var vsLightingPass = '';
          if (GLEmulation.lightingEnabled) {
            vsLightingDefs += 'attribute vec3 a_normal;';
            vsLightingDefs += 'uniform mat3 u_normalMatrix;';
            vsLightingDefs += 'uniform vec4 u_lightModelAmbient;';
            vsLightingDefs += 'uniform vec4 u_materialAmbient;';
            vsLightingDefs += 'uniform vec4 u_materialDiffuse;';
            vsLightingDefs += 'uniform vec4 u_materialSpecular;';
            vsLightingDefs += 'uniform float u_materialShininess;';
            vsLightingDefs += 'uniform vec4 u_materialEmission;';

            vsLightingPass += '  vec3 ecNormal = normalize(u_normalMatrix * a_normal);';
            vsLightingPass += '  v_color.w = u_materialDiffuse.w;';
            vsLightingPass += '  v_color.xyz = u_materialEmission.xyz;';
            vsLightingPass += '  v_color.xyz += u_lightModelAmbient.xyz * u_materialAmbient.xyz;';

            for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
              if (GLEmulation.lightEnabled[lightId]) {
                vsLightingDefs += 'uniform vec4 u_lightAmbient' + lightId + ';';
                vsLightingDefs += 'uniform vec4 u_lightDiffuse' + lightId + ';';
                vsLightingDefs += 'uniform vec4 u_lightSpecular' + lightId + ';';
                vsLightingDefs += 'uniform vec4 u_lightPosition' + lightId + ';';

                vsLightingPass += '  {';
                vsLightingPass += '    vec3 lightDirection = normalize(u_lightPosition' + lightId + ').xyz;';
                vsLightingPass += '    vec3 halfVector = normalize(lightDirection + vec3(0,0,1));';
                vsLightingPass += '    vec3 ambient = u_lightAmbient' + lightId + '.xyz * u_materialAmbient.xyz;';
                vsLightingPass += '    float diffuseI = max(dot(ecNormal, lightDirection), 0.0);';
                vsLightingPass += '    float specularI = max(dot(ecNormal, halfVector), 0.0);';
                vsLightingPass += '    vec3 diffuse = diffuseI * u_lightDiffuse' + lightId + '.xyz * u_materialDiffuse.xyz;';
                vsLightingPass += '    specularI = (diffuseI > 0.0 && specularI > 0.0) ? exp(u_materialShininess * log(specularI)) : 0.0;';
                vsLightingPass += '    vec3 specular = specularI * u_lightSpecular' + lightId + '.xyz * u_materialSpecular.xyz;';
                vsLightingPass += '    v_color.xyz += ambient + diffuse + specular;';
                vsLightingPass += '  }';
              }
            }
            vsLightingPass += '  v_color = clamp(v_color, 0.0, 1.0);';
          }

          var vsSource = [
            'attribute vec4 a_position;',
            'attribute vec4 a_color;',
            'varying vec4 v_color;',
            texUnitAttribList,
            texUnitVaryingList,
            (GLEmulation.fogEnabled ? 'varying float v_fogFragCoord;' : null),
            'uniform mat4 u_modelView;',
            'uniform mat4 u_projection;',
            vsPointSizeDefs,
            vsClipPlaneDefs,
            vsLightingDefs,
            'void main()',
            '{',
            '  vec4 ecPosition = u_modelView * a_position;', // eye-coordinate position
            '  gl_Position = u_projection * ecPosition;',
            '  v_color = a_color;',
            vsTexCoordInits,
            vsFogVaryingInit,
            vsPointSizeInit,
            vsClipPlaneInit,
            vsLightingPass,
            '}',
            ''
          ].join('\n').replace(/\n\n+/g, '\n');

          this.vertexShader = Q3e.webgl.createShader(Q3e.webgl.VERTEX_SHADER);
          Q3e.webgl.shaderSource(this.vertexShader, vsSource);
          Q3e.webgl.compileShader(this.vertexShader);

          var fogHeaderIfNeeded = null;
          if (GLEmulation.fogEnabled) {
            fogHeaderIfNeeded = [
              '',
              'varying float v_fogFragCoord; ',
              'uniform vec4 u_fogColor;      ',
              'uniform float u_fogEnd;       ',
              'uniform float u_fogScale;     ',
              'uniform float u_fogDensity;   ',
              'float ffog(in float ecDistance) { ',
              fogFormula,
              '  fog = clamp(fog, 0.0, 1.0); ',
              '  return fog;                 ',
              '}',
              '',
            ].join("\n");
          }

          var fogPass = null;
          if (GLEmulation.fogEnabled) {
            fogPass = 'gl_FragColor = vec4(mix(u_fogColor.rgb, gl_FragColor.rgb, ffog(v_fogFragCoord)), gl_FragColor.a);\n';
          }

          var fsSource = [
            'precision mediump float;',
            texUnitVaryingList,
            texUnitUniformList,
            'varying vec4 v_color;',
            fogHeaderIfNeeded,
            fsClipPlaneDefs,
            'void main()',
            '{',
            fsClipPlanePass,
            fsTexEnvPass,
            fogPass,
            '}',
            ''
          ].join("\n").replace(/\n\n+/g, '\n');

          this.fragmentShader = Q3e.webgl.createShader(Q3e.webgl.FRAGMENT_SHADER);
          Q3e.webgl.shaderSource(this.fragmentShader, fsSource);
          Q3e.webgl.compileShader(this.fragmentShader);

          this.program = Q3e.webgl.createProgram();
          Q3e.webgl.attachShader(this.program, this.vertexShader);
          Q3e.webgl.attachShader(this.program, this.fragmentShader);

          // As optimization, bind all attributes to prespecified locations, so that the FFP emulation
          // code can submit attributes to any generated FFP shader without having to examine each shader in turn.
          // These prespecified locations are only assumed if GL_FFP_ONLY is specified, since user could also create their
          // own shaders that didn't have attributes in the same locations.
          Q3e.webgl.bindAttribLocation(this.program, GLImmediate.VERTEX, 'a_position');
          Q3e.webgl.bindAttribLocation(this.program, GLImmediate.COLOR, 'a_color');
          Q3e.webgl.bindAttribLocation(this.program, GLImmediate.NORMAL, 'a_normal');
          var maxVertexAttribs = Q3e.webgl.getParameter(Q3e.webgl.MAX_VERTEX_ATTRIBS);
          for (var i = 0; i < GLImmediate.MAX_TEXTURES && GLImmediate.TEXTURE0 + i < maxVertexAttribs; i++) {
            Q3e.webgl.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, 'a_texCoord'+i);
            Q3e.webgl.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, aTexCoordPrefix+i);
          }
          Q3e.webgl.linkProgram(this.program);
        }

        // Stores an array that remembers which matrix uniforms are up-to-date in this FFP renderer, so they don't need to be resubmitted
        // each time we render with this program.
        this.textureMatrixVersion = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];

        this.positionLocation = Q3e.webgl.getAttribLocation(this.program, 'a_position');

        this.texCoordLocations = [];

        for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
          if (!GLImmediate.enabledClientAttributes[GLImmediate.TEXTURE0 + i]) {
            this.texCoordLocations[i] = -1;
            continue;
          }

          if (useCurrProgram) {
            this.texCoordLocations[i] = Q3e.webgl.getAttribLocation(this.program, 'a_texCoord' + i);
          } else {
            this.texCoordLocations[i] = Q3e.webgl.getAttribLocation(this.program, aTexCoordPrefix + i);
          }
        }
        this.colorLocation = Q3e.webgl.getAttribLocation(this.program, 'a_color');
        if (!useCurrProgram) {
          // Temporarily switch to the program so we can set our sampler uniforms early.
          var prevBoundProg = Q3e.webgl.getParameter(Q3e.webgl.CURRENT_PROGRAM);
          Q3e.webgl.useProgram(this.program);
          {
            for (var i = 0; i < this.usedTexUnitList.length; i++) {
              var texUnitID = this.usedTexUnitList[i];
              var texSamplerLoc = Q3e.webgl.getUniformLocation(this.program, uTexUnitPrefix + texUnitID);
              Q3e.webgl.uniform1i(texSamplerLoc, texUnitID);
            }
          }
          // The default color attribute value is not the same as the default for all other attribute streams (0,0,0,1) but (1,1,1,1),
          // so explicitly set it right at start.
          Q3e.webgl.vertexAttrib4fv(this.colorLocation, [1,1,1,1]);
          Q3e.webgl.useProgram(prevBoundProg);
        }

        this.textureMatrixLocations = [];
        for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
          this.textureMatrixLocations[i] = Q3e.webgl.getUniformLocation(this.program, 'u_textureMatrix' + i);
        }
        this.normalLocation = Q3e.webgl.getAttribLocation(this.program, 'a_normal');

        this.modelViewLocation = Q3e.webgl.getUniformLocation(this.program, 'u_modelView');
        this.projectionLocation = Q3e.webgl.getUniformLocation(this.program, 'u_projection');
        this.normalMatrixLocation = Q3e.webgl.getUniformLocation(this.program, 'u_normalMatrix');

        this.hasTextures = hasTextures;
        this.hasNormal = GLImmediate.enabledClientAttributes[GLImmediate.NORMAL] &&
                         GLImmediate.clientAttributes[GLImmediate.NORMAL].size > 0 &&
                         this.normalLocation >= 0;
        this.hasColor = (this.colorLocation === 0) || this.colorLocation > 0;

        this.floatType = Q3e.webgl.FLOAT; // minor optimization

        this.fogColorLocation = Q3e.webgl.getUniformLocation(this.program, 'u_fogColor');
        this.fogEndLocation = Q3e.webgl.getUniformLocation(this.program, 'u_fogEnd');
        this.fogScaleLocation = Q3e.webgl.getUniformLocation(this.program, 'u_fogScale');
        this.fogDensityLocation = Q3e.webgl.getUniformLocation(this.program, 'u_fogDensity');
        this.hasFog = !!(this.fogColorLocation || this.fogEndLocation ||
                         this.fogScaleLocation || this.fogDensityLocation);

        this.pointSizeLocation = Q3e.webgl.getUniformLocation(this.program, 'u_pointSize');

        this.hasClipPlane = false;
        this.clipPlaneEquationLocation = [];
        for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
          this.clipPlaneEquationLocation[clipPlaneId] = Q3e.webgl.getUniformLocation(this.program, 'u_clipPlaneEquation' + clipPlaneId);
          this.hasClipPlane = (this.hasClipPlane || this.clipPlaneEquationLocation[clipPlaneId]);
        }

        this.hasLighting = GLEmulation.lightingEnabled;
        this.lightModelAmbientLocation = Q3e.webgl.getUniformLocation(this.program, 'u_lightModelAmbient');
        this.materialAmbientLocation = Q3e.webgl.getUniformLocation(this.program, 'u_materialAmbient');
        this.materialDiffuseLocation = Q3e.webgl.getUniformLocation(this.program, 'u_materialDiffuse');
        this.materialSpecularLocation = Q3e.webgl.getUniformLocation(this.program, 'u_materialSpecular');
        this.materialShininessLocation = Q3e.webgl.getUniformLocation(this.program, 'u_materialShininess');
        this.materialEmissionLocation = Q3e.webgl.getUniformLocation(this.program, 'u_materialEmission');
        this.lightAmbientLocation = []
        this.lightDiffuseLocation = []
        this.lightSpecularLocation = []
        this.lightPositionLocation = []
        for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
          this.lightAmbientLocation[lightId] = Q3e.webgl.getUniformLocation(this.program, 'u_lightAmbient' + lightId);
          this.lightDiffuseLocation[lightId] = Q3e.webgl.getUniformLocation(this.program, 'u_lightDiffuse' + lightId);
          this.lightSpecularLocation[lightId] = Q3e.webgl.getUniformLocation(this.program, 'u_lightSpecular' + lightId);
          this.lightPositionLocation[lightId] = Q3e.webgl.getUniformLocation(this.program, 'u_lightPosition' + lightId);
        }

      },

      prepare: function prepare() {
        // Calculate the array buffer
        var arrayBuffer;
        if (!Q3e.webgl.currentArrayBufferBinding) {
          var start = GLImmediate.firstVertex*GLImmediate.stride;
          var end = GLImmediate.lastVertex*GLImmediate.stride;
          arrayBuffer = GLEmulation.getTempVertexBuffer(end);
          // TODO: consider using the last buffer we bound, if it was larger. downside is larger buffer, but we might avoid rebinding and preparing
        } else {
          arrayBuffer = Q3e.webgl.currentArrayBufferBinding;
        }

/*
#if GL_UNSAFE_OPTS
        // If the array buffer is unchanged and the renderer as well, then we can avoid all the work here
        // XXX We use some heuristics here, and this may not work in all cases. Try disabling GL_UNSAFE_OPTS if you
        // have odd glitches
        var lastRenderer = GLImmediate.lastRenderer;
        var canSkip = this == lastRenderer &&
                      arrayBuffer == GLImmediate.lastArrayBuffer &&
                      (GLEmulation.currProgram || this.program) == GLImmediate.lastProgram &&
                      GLImmediate.stride == GLImmediate.lastStride &&
                      !GLImmediate.matricesModified;
        if (!canSkip && lastRenderer) lastRenderer.cleanup();
#endif
*/
        if (!Q3e.webgl.currentArrayBufferBinding) {
          // Bind the array buffer and upload data after cleaning up the previous renderer

          if (arrayBuffer != GLImmediate.lastArrayBuffer) {
            Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, arrayBuffer);
            GLImmediate.lastArrayBuffer = arrayBuffer;
          }

          Q3e.webgl.bufferSubData(Q3e.webgl.ARRAY_BUFFER, start, GLImmediate.vertexData.subarray(start >> 2, end >> 2));
        }
/*
#if GL_UNSAFE_OPTS
        if (canSkip) return;
        GLImmediate.lastRenderer = this;
        GLImmediate.lastProgram = GLEmulation.currProgram || this.program;
        GLImmediate.lastStride == GLImmediate.stride;
        GLImmediate.matricesModified = false;
#endif
*/

        if (!GLEmulation.currProgram) {
          if (GLImmediate.fixedFunctionProgram != this.program) {
            Q3e.webgl.useProgram(this.program);
            GLImmediate.fixedFunctionProgram = this.program;
          }
        }

        if (this.modelViewLocation && this.modelViewMatrixVersion != GLImmediate.matrixVersion[0/*m*/]) {
          this.modelViewMatrixVersion = GLImmediate.matrixVersion[0/*m*/];
          Q3e.webgl.uniformMatrix4fv(this.modelViewLocation, false, GLImmediate.matrix[0/*m*/]);

          // set normal matrix to the upper 3x3 of the inverse transposed current modelview matrix
          if (GLEmulation.lightEnabled) {
            var tmpMVinv = GLImmediate.matrixLib.mat4.create(GLImmediate.matrix[0]);
            GLImmediate.matrixLib.mat4.inverse(tmpMVinv);
            GLImmediate.matrixLib.mat4.transpose(tmpMVinv);
            Q3e.webgl.uniformMatrix3fv(this.normalMatrixLocation, false, GLImmediate.matrixLib.mat4.toMat3(tmpMVinv));
          }
        }
        if (this.projectionLocation && this.projectionMatrixVersion != GLImmediate.matrixVersion[1/*p*/]) {
          this.projectionMatrixVersion = GLImmediate.matrixVersion[1/*p*/];
          Q3e.webgl.uniformMatrix4fv(this.projectionLocation, false, GLImmediate.matrix[1/*p*/]);
        }

        var clientAttributes = GLImmediate.clientAttributes;
        var posAttr = clientAttributes[GLImmediate.VERTEX];


//#if GL_FFP_ONLY
/*
        if (!Q3e.webgl.currentArrayBufferBinding) {
          Q3e.webgl.vertexAttribPointer(GLImmediate.VERTEX, posAttr.size, posAttr.type, false, GLImmediate.stride, posAttr.offset);
          if (this.hasNormal) {
            var normalAttr = clientAttributes[GLImmediate.NORMAL];
            Q3e.webgl.vertexAttribPointer(GLImmediate.NORMAL, normalAttr.size, normalAttr.type, true, GLImmediate.stride, normalAttr.offset);
          }
        }
*/

        Q3e.webgl.vertexAttribPointer(this.positionLocation, posAttr.size, posAttr.type, false, GLImmediate.stride, posAttr.offset);
        Q3e.webgl.enableVertexAttribArray(this.positionLocation);
        if (this.hasNormal) {
          var normalAttr = clientAttributes[GLImmediate.NORMAL];
          Q3e.webgl.vertexAttribPointer(this.normalLocation, normalAttr.size, normalAttr.type, true, GLImmediate.stride, normalAttr.offset);
          Q3e.webgl.enableVertexAttribArray(this.normalLocation);
        }

        if (this.hasTextures) {
          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
//#if GL_FFP_ONLY
/*
            if (!Q3e.webgl.currentArrayBufferBinding) {
              var attribLoc = GLImmediate.TEXTURE0+i;
              var texAttr = clientAttributes[attribLoc];
              if (texAttr.size) {
                Q3e.webgl.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
              } else {
                // These two might be dangerous, but let's try them.
                Q3e.webgl.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
              }
            }
*/
            var attribLoc = this.texCoordLocations[i];
            if (attribLoc === undefined || attribLoc < 0) continue;
            var texAttr = clientAttributes[GLImmediate.TEXTURE0+i];

            if (texAttr.size) {
              Q3e.webgl.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
              Q3e.webgl.enableVertexAttribArray(attribLoc);
            } else {
              // These two might be dangerous, but let's try them.
              Q3e.webgl.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
              Q3e.webgl.disableVertexAttribArray(attribLoc);
            }

            var t = 2/*t*/+i;
            if (this.textureMatrixLocations[i] && this.textureMatrixVersion[t] != GLImmediate.matrixVersion[t]) { // XXX might we need this even without the condition we are currently in?
              this.textureMatrixVersion[t] = GLImmediate.matrixVersion[t];
              Q3e.webgl.uniformMatrix4fv(this.textureMatrixLocations[i], false, GLImmediate.matrix[t]);
            }
          }
        }
        if (GLImmediate.enabledClientAttributes[GLImmediate.COLOR]) {
          var colorAttr = clientAttributes[GLImmediate.COLOR];
//#if GL_FFP_ONLY
/*
          if (!Q3e.webgl.currentArrayBufferBinding) {
            Q3e.webgl.vertexAttribPointer(GLImmediate.COLOR, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
          }
*/
          Q3e.webgl.vertexAttribPointer(this.colorLocation, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
          Q3e.webgl.enableVertexAttribArray(this.colorLocation);
        }
        else if (this.hasColor) {
          Q3e.webgl.disableVertexAttribArray(this.colorLocation);
          Q3e.webgl.vertexAttrib4fv(this.colorLocation, GLImmediate.clientColor);
        }
        if (this.hasFog) {
          if (this.fogColorLocation) Q3e.webgl.uniform4fv(this.fogColorLocation, GLEmulation.fogColor);
          if (this.fogEndLocation) Q3e.webgl.uniform1f(this.fogEndLocation, GLEmulation.fogEnd);
          if (this.fogScaleLocation) Q3e.webgl.uniform1f(this.fogScaleLocation, 1/(GLEmulation.fogEnd - GLEmulation.fogStart));
          if (this.fogDensityLocation) Q3e.webgl.uniform1f(this.fogDensityLocation, GLEmulation.fogDensity);
        }

        if (this.hasClipPlane) {
          for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
            if (this.clipPlaneEquationLocation[clipPlaneId]) Q3e.webgl.uniform4fv(this.clipPlaneEquationLocation[clipPlaneId], GLEmulation.clipPlaneEquation[clipPlaneId]);
          }
        }

        if (this.hasLighting) {
          if (this.lightModelAmbientLocation) Q3e.webgl.uniform4fv(this.lightModelAmbientLocation, GLEmulation.lightModelAmbient);
          if (this.materialAmbientLocation) Q3e.webgl.uniform4fv(this.materialAmbientLocation, GLEmulation.materialAmbient);
          if (this.materialDiffuseLocation) Q3e.webgl.uniform4fv(this.materialDiffuseLocation, GLEmulation.materialDiffuse);
          if (this.materialSpecularLocation) Q3e.webgl.uniform4fv(this.materialSpecularLocation, GLEmulation.materialSpecular);
          if (this.materialShininessLocation) Q3e.webgl.uniform1f(this.materialShininessLocation, GLEmulation.materialShininess[0]);
          if (this.materialEmissionLocation) Q3e.webgl.uniform4fv(this.materialEmissionLocation, GLEmulation.materialEmission);
          for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
            if (this.lightAmbientLocation[lightId]) Q3e.webgl.uniform4fv(this.lightAmbientLocation[lightId], GLEmulation.lightAmbient[lightId]);
            if (this.lightDiffuseLocation[lightId]) Q3e.webgl.uniform4fv(this.lightDiffuseLocation[lightId], GLEmulation.lightDiffuse[lightId]);
            if (this.lightSpecularLocation[lightId]) Q3e.webgl.uniform4fv(this.lightSpecularLocation[lightId], GLEmulation.lightSpecular[lightId]);
            if (this.lightPositionLocation[lightId]) Q3e.webgl.uniform4fv(this.lightPositionLocation[lightId], GLEmulation.lightPosition[lightId]);
          }
        }

        if (GLImmediate.mode == Q3e.webgl.POINTS) {
          if (this.pointSizeLocation) {
            Q3e.webgl.uniform1f(this.pointSizeLocation, GLEmulation.pointSize);
          }
        }
      },

      cleanup: function cleanup() {

        Q3e.webgl.disableVertexAttribArray(this.positionLocation);
        if (this.hasTextures) {
          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            if (GLImmediate.enabledClientAttributes[GLImmediate.TEXTURE0+i] && this.texCoordLocations[i] >= 0) {
              Q3e.webgl.disableVertexAttribArray(this.texCoordLocations[i]);
            }
          }
        }
        if (this.hasColor) {
          Q3e.webgl.disableVertexAttribArray(this.colorLocation);
        }
        if (this.hasNormal) {
          Q3e.webgl.disableVertexAttribArray(this.normalLocation);
        }
        if (!GLEmulation.currProgram) {
          Q3e.webgl.useProgram(null);
          GLImmediate.fixedFunctionProgram = 0;
        }
        if (!Q3e.webgl.currentArrayBufferBinding) {
          Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, null);
          GLImmediate.lastArrayBuffer = null;
        }

/*
#if GL_UNSAFE_OPTS
        GLImmediate.lastRenderer = null;
        GLImmediate.lastProgram = null;
#endif
*/
        GLImmediate.matricesModified = true;
      }
    };
    ret.init();
    return ret;
  },

  setupFuncs: function() {
    // TexEnv stuff needs to be prepared early, so do it here.
    // init() is too late for -O2, since it freezes the GL functions
    // by that point.
    GLImmediate.MapTreeLib = GLImmediate.spawnMapTreeLib();
    GLImmediate.spawnMapTreeLib = null;

    GLImmediate.TexEnvJIT = GLImmediate.spawnTexEnvJIT();
    GLImmediate.spawnTexEnvJIT = null;
  },

  // Main functions
  initted: false,
  init: function() {
    GLImmediate.setupFuncs()
    GLImmediate.initted = true;

    //if (!Module.useWebGL) return; // a 2D canvas may be currently used TODO: make sure we are actually called in that case

    // User can override the maximum number of texture units that we emulate. Using fewer texture units increases runtime performance
    // slightly, so it is advantageous to choose as small value as needed.
    GLImmediate.MAX_TEXTURES = Q3e.webgl.getParameter(Q3e.webgl.MAX_TEXTURE_IMAGE_UNITS);

    GLImmediate.TexEnvJIT.init(Q3e.webgl, GLImmediate.MAX_TEXTURES);

    GLImmediate.NUM_ATTRIBUTES = 3 /*pos+normal+color attributes*/ + GLImmediate.MAX_TEXTURES;
    GLImmediate.clientAttributes = [];
    GLEmulation.enabledClientAttribIndices = [];
    for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
      GLImmediate.clientAttributes.push({});
      GLEmulation.enabledClientAttribIndices.push(false);
    }

    // Initialize matrix library
    // When user sets a matrix, increment a 'version number' on the new data, and when rendering, submit
    // the matrices to the shader program only if they have an old version of the data.
    GLImmediate.matrix = [];
    GLImmediate.matrixStack = [];
    GLImmediate.matrixVersion = [];
    for (var i = 0; i < 2 + GLImmediate.MAX_TEXTURES; i++) { // Modelview, Projection, plus one matrix for each texture coordinate.
      GLImmediate.matrixStack.push([]);
      GLImmediate.matrixVersion.push(0);
      GLImmediate.matrix.push(GLImmediate.matrixLib.mat4.create());
      GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[i]);
    }

    // Renderer cache
    GLImmediate.rendererCache = GLImmediate.MapTreeLib.create();

    // Buffers for data
    GLImmediate.tempData = new Float32Array(GLEmulation.MAX_TEMP_BUFFER_SIZE >> 2);
    GLImmediate.indexData = new Uint16Array(GLEmulation.MAX_TEMP_BUFFER_SIZE >> 1);

    GLImmediate.vertexDataU8 = new Uint8Array(GLImmediate.tempData.buffer);

    GLEmulation.generateTempBuffers(true, Q3e.webgl);

    GLImmediate.clientColor = new Float32Array([1, 1, 1, 1]);
  },

  // Prepares and analyzes client attributes.
  // Modifies liveClientAttributes, stride, vertexPointer, vertexCounter
  //   count: number of elements we will draw
  //   beginEnd: whether we are drawing the results of a begin/end block
  prepareClientAttributes: function prepareClientAttributes(count, beginEnd) {
    // If no client attributes were modified since we were last called, do nothing. Note that this
    // does not work for glBegin/End, where we generate renderer components dynamically and then
    // disable them ourselves, but it does help with glDrawElements/Arrays.
    if (!GLImmediate.modifiedClientAttributes) {
      GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
      return;
    }
    GLImmediate.modifiedClientAttributes = false;

    // The role of prepareClientAttributes is to examine the set of client-side vertex attribute buffers
    // that user code has submitted, and to prepare them to be uploaded to a VBO in GPU memory
    // (since WebGL does not support client-side rendering, i.e. rendering from vertex data in CPU memory)
    // User can submit vertex data generally in three different configurations:
    // 1. Fully planar: all attributes are in their own separate tightly-packed arrays in CPU memory.
    // 2. Fully interleaved: all attributes share a single array where data is interleaved something like (pos,uv,normal), (pos,uv,normal), ...
    // 3. Complex hybrid: Multiple separate arrays that either are sparsely strided, and/or partially interleave vertex attributes.

    // For simplicity, we support the case (2) as the fast case. For (1) and (3), we do a memory copy of the
    // vertex data here to prepare a relayouted buffer that is of the structure in case (2). The reason
    // for this is that it allows the emulation code to get away with using just one VBO buffer for rendering,
    // and not have to maintain multiple ones. Therefore cases (1) and (3) will be very slow, and case (2) is fast.

    // Detect which case we are in by using a quick heuristic by examining the strides of the buffers. If all the buffers have identical
    // stride, we assume we have case (2), otherwise we have something more complex.
    var clientStartPointer = 0x7FFFFFFF;
    var bytes = 0; // Total number of bytes taken up by a single vertex.
    var minStride = 0x7FFFFFFF;
    var maxStride = 0;
    var attributes = GLImmediate.liveClientAttributes;
    attributes.length = 0;
    for (var i = 0; i < 3+GLImmediate.MAX_TEXTURES; i++) {
      if (GLImmediate.enabledClientAttributes[i]) {
        var attr = GLImmediate.clientAttributes[i];
        attributes.push(attr);
        clientStartPointer = Math.min(clientStartPointer, attr.pointer);
        attr.sizeBytes = attr.size * GLEmulation.byteSizeByType[attr.type - GLEmulation.byteSizeByTypeRoot];
        bytes += attr.sizeBytes;
        minStride = Math.min(minStride, attr.stride);
        maxStride = Math.max(maxStride, attr.stride);
      }
    }

    if ((minStride != maxStride || maxStride < bytes) && !beginEnd) {
      // We are in cases (1) or (3): slow path, shuffle the data around into a single interleaved vertex buffer.
      // The immediate-mode glBegin()/glEnd() vertex submission gets automatically generated in appropriate layout,
      // so never need to come down this path if that was used.
      if (!GLImmediate.restrideBuffer) GLImmediate.restrideBuffer = malloc(GLEmulation.MAX_TEMP_BUFFER_SIZE);
      var start = GLImmediate.restrideBuffer;
      bytes = 0;
      // calculate restrided offsets and total size
      for (var i = 0; i < attributes.length; i++) {
        var attr = attributes[i];
        var size = attr.sizeBytes;
        if (size % 4 != 0) size += 4 - (size % 4); // align everything
        attr.offset = bytes;
        bytes += size;
      }
      // copy out the data (we need to know the stride for that, and define attr.pointer)
      for (var i = 0; i < attributes.length; i++) {
        var attr = attributes[i];
        var srcStride = Math.max(attr.sizeBytes, attr.stride);
        if ((srcStride & 3) == 0 && (attr.sizeBytes & 3) == 0) {
          var size4 = attr.sizeBytes>>2;
          var srcStride4 = Math.max(attr.sizeBytes, attr.stride)>>2;
          for (var j = 0; j < count; j++) {
            for (var k = 0; k < size4; k++) { // copy in chunks of 4 bytes, our alignment makes this possible
              Q3e.paged32[((start + attr.offset + bytes*j)>>2) + k] = Q3e.paged32[(attr.pointer>>2) + j*srcStride4 + k];
            }
          }
        } else {
          for (var j = 0; j < count; j++) {
            for (var k = 0; k < attr.sizeBytes; k++) { // source data was not aligned to multiples of 4, must copy byte by byte.
              Q3e.paged[start + attr.offset + bytes*j + k] = Q3e.paged[attr.pointer + j*srcStride + k];
            }
          }
        }
        attr.pointer = start + attr.offset;
      }
      GLImmediate.stride = bytes;
      GLImmediate.vertexPointer = start;
    } else {
      // case (2): fast path, all data is interleaved to a single vertex array so we can get away with a single VBO upload.
      if (Q3e.webgl.currentArrayBufferBinding) {
        GLImmediate.vertexPointer = 0;
      } else {
        GLImmediate.vertexPointer = clientStartPointer;
      }
      for (var i = 0; i < attributes.length; i++) {
        var attr = attributes[i];
        attr.offset = attr.pointer - GLImmediate.vertexPointer; // Compute what will be the offset of this attribute in the VBO after we upload.
      }
      GLImmediate.stride = Math.max(maxStride, bytes);
    }
    if (!beginEnd) {
      GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
    }
  },

  flush: function flush(numProvidedIndexes, startIndex, ptr) {
    startIndex = startIndex || 0;
    ptr = ptr || 0;

    var renderer = GLImmediate.getRenderer();

    // Generate index data in a format suitable for GLES 2.0/WebGL
    var numVertexes = 4 * GLImmediate.vertexCounter / GLImmediate.stride;
    if (!numVertexes) return;
    var emulatedElementArrayBuffer = false;
    var numIndexes = 0;
    if (numProvidedIndexes) {
      numIndexes = numProvidedIndexes;
      if (!Q3e.webgl.currentArrayBufferBinding && GLImmediate.firstVertex > GLImmediate.lastVertex) {
        // Figure out the first and last vertex from the index data
        for (var i = 0; i < numProvidedIndexes; i++) {
          var currIndex = Q3e.paged16[(ptr+i*2)] = null;
          GLImmediate.firstVertex = Math.min(GLImmediate.firstVertex, currIndex);
          GLImmediate.lastVertex = Math.max(GLImmediate.lastVertex, currIndex+1);
        }
      }
      if (!Q3e.webgl.currentElementArrayBufferBinding) {
        // If no element array buffer is bound, then indices is a literal pointer to clientside data
        var indexBuffer = GLEmulation.getTempIndexBuffer(numProvidedIndexes << 1);
        Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, indexBuffer);
        Q3e.webgl.bufferSubData(Q3e.webgl.ELEMENT_ARRAY_BUFFER, 0, Uint16Array.from(Q3e.paged16.subarray(ptr >> 1, (ptr + numProvidedIndexes) >> 1)));
        ptr = 0;
        emulatedElementArrayBuffer = true;
      }
    } else if (GLImmediate.mode > 6) { // above GL_TRIANGLE_FAN are the non-GL ES modes
      if (GLImmediate.mode != 7) throw 'unsupported immediate mode ' + GLImmediate.mode; // GL_QUADS
      // GLImmediate.firstVertex is the first vertex we want. Quad indexes are in the pattern
      // 0 1 2, 0 2 3, 4 5 6, 4 6 7, so we need to look at index firstVertex * 1.5 to see it.
      // Then since indexes are 2 bytes each, that means 3
      ptr = GLImmediate.firstVertex*3;
      var numQuads = numVertexes / 4;
      numIndexes = numQuads * 6; // 0 1 2, 0 2 3 pattern
      Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, Q3e.webgl.tempQuadIndexBuffer);
      emulatedElementArrayBuffer = true;
      GLImmediate.mode = Q3e.webgl.TRIANGLES;
    }

    renderer.prepare();

    if (numIndexes) {
      Q3e.webgl.drawElements(GLImmediate.mode, numIndexes, Q3e.webgl.UNSIGNED_SHORT, ptr);
    } else {
      Q3e.webgl.drawArrays(GLImmediate.mode, startIndex, numVertexes);
    }

    if (emulatedElementArrayBuffer) {
      Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, GLEmulation.buffers[Q3e.webgl.currentElementArrayBufferBinding] || null);
    }

    renderer.cleanup();
  }
})


$GLImmediate.prototype.matrixLib = (function() {

  /**
   * @fileoverview gl-matrix - High performance matrix and vector operations for WebGL
   * @author Brandon Jones
   * @version 1.2.4
   */
  
  // Modifed for emscripten: Global scoping etc.
  
  /*
   * Copyright (c) 2011 Brandon Jones
   *
   * This software is provided 'as-is', without any express or implied
   * warranty. In no event will the authors be held liable for any damages
   * arising from the use of this software.
   *
   * Permission is granted to anyone to use this software for any purpose,
   * including commercial applications, and to alter it and redistribute it
   * freely, subject to the following restrictions:
   *
   *    1. The origin of this software must not be misrepresented; you must not
   *    claim that you wrote the original software. If you use this software
   *    in a product, an acknowledgment in the product documentation would be
   *    appreciated but is not required.
   *
   *    2. Altered source versions must be plainly marked as such, and must not
   *    be misrepresented as being the original software.
   *
   *    3. This notice may not be removed or altered from any source
   *    distribution.
   */
  
  
  /**
   * @class 3 Dimensional Vector
   * @name vec3
   */
  var vec3 = {};
  
  /**
   * @class 3x3 Matrix
   * @name mat3
   */
  var mat3 = {};
  
  /**
   * @class 4x4 Matrix
   * @name mat4
   */
  var mat4 = {};
  
  /**
   * @class Quaternion
   * @name quat4
   */
  var quat4 = {};
  
  var MatrixArray = Float32Array;
  
  /*
   * vec3
   */
  
  /**
   * Creates a new instance of a vec3 using the default array type
   * Any javascript array-like objects containing at least 3 numeric elements can serve as a vec3
   *
   * @param {vec3} [vec] vec3 containing values to initialize with
   *
   * @returns {vec3} New vec3
   */
  vec3.create = function (vec) {
      var dest = new MatrixArray(3);
  
      if (vec) {
          dest[0] = vec[0];
          dest[1] = vec[1];
          dest[2] = vec[2];
      } else {
          dest[0] = dest[1] = dest[2] = 0;
      }
  
      return dest;
  };
  
  /**
   * Copies the values of one vec3 to another
   *
   * @param {vec3} vec vec3 containing values to copy
   * @param {vec3} dest vec3 receiving copied values
   *
   * @returns {vec3} dest
   */
  vec3.set = function (vec, dest) {
      dest[0] = vec[0];
      dest[1] = vec[1];
      dest[2] = vec[2];
  
      return dest;
  };
  
  /**
   * Performs a vector addition
   *
   * @param {vec3} vec First operand
   * @param {vec3} vec2 Second operand
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.add = function (vec, vec2, dest) {
      if (!dest || vec === dest) {
          vec[0] += vec2[0];
          vec[1] += vec2[1];
          vec[2] += vec2[2];
          return vec;
      }
  
      dest[0] = vec[0] + vec2[0];
      dest[1] = vec[1] + vec2[1];
      dest[2] = vec[2] + vec2[2];
      return dest;
  };
  
  /**
   * Performs a vector subtraction
   *
   * @param {vec3} vec First operand
   * @param {vec3} vec2 Second operand
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.subtract = function (vec, vec2, dest) {
      if (!dest || vec === dest) {
          vec[0] -= vec2[0];
          vec[1] -= vec2[1];
          vec[2] -= vec2[2];
          return vec;
      }
  
      dest[0] = vec[0] - vec2[0];
      dest[1] = vec[1] - vec2[1];
      dest[2] = vec[2] - vec2[2];
      return dest;
  };
  
  /**
   * Performs a vector multiplication
   *
   * @param {vec3} vec First operand
   * @param {vec3} vec2 Second operand
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.multiply = function (vec, vec2, dest) {
      if (!dest || vec === dest) {
          vec[0] *= vec2[0];
          vec[1] *= vec2[1];
          vec[2] *= vec2[2];
          return vec;
      }
  
      dest[0] = vec[0] * vec2[0];
      dest[1] = vec[1] * vec2[1];
      dest[2] = vec[2] * vec2[2];
      return dest;
  };
  
  /**
   * Negates the components of a vec3
   *
   * @param {vec3} vec vec3 to negate
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.negate = function (vec, dest) {
      if (!dest) { dest = vec; }
  
      dest[0] = -vec[0];
      dest[1] = -vec[1];
      dest[2] = -vec[2];
      return dest;
  };
  
  /**
   * Multiplies the components of a vec3 by a scalar value
   *
   * @param {vec3} vec vec3 to scale
   * @param {number} val Value to scale by
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.scale = function (vec, val, dest) {
      if (!dest || vec === dest) {
          vec[0] *= val;
          vec[1] *= val;
          vec[2] *= val;
          return vec;
      }
  
      dest[0] = vec[0] * val;
      dest[1] = vec[1] * val;
      dest[2] = vec[2] * val;
      return dest;
  };
  
  /**
   * Generates a unit vector of the same direction as the provided vec3
   * If vector length is 0, returns [0, 0, 0]
   *
   * @param {vec3} vec vec3 to normalize
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.normalize = function (vec, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0], y = vec[1], z = vec[2],
          len = Math.sqrt(x * x + y * y + z * z);
  
      if (!len) {
          dest[0] = 0;
          dest[1] = 0;
          dest[2] = 0;
          return dest;
      } else if (len === 1) {
          dest[0] = x;
          dest[1] = y;
          dest[2] = z;
          return dest;
      }
  
      len = 1 / len;
      dest[0] = x * len;
      dest[1] = y * len;
      dest[2] = z * len;
      return dest;
  };
  
  /**
   * Generates the cross product of two vec3s
   *
   * @param {vec3} vec First operand
   * @param {vec3} vec2 Second operand
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.cross = function (vec, vec2, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0], y = vec[1], z = vec[2],
          x2 = vec2[0], y2 = vec2[1], z2 = vec2[2];
  
      dest[0] = y * z2 - z * y2;
      dest[1] = z * x2 - x * z2;
      dest[2] = x * y2 - y * x2;
      return dest;
  };
  
  /**
   * Caclulates the length of a vec3
   *
   * @param {vec3} vec vec3 to calculate length of
   *
   * @returns {number} Length of vec
   */
  vec3.length = function (vec) {
      var x = vec[0], y = vec[1], z = vec[2];
      return Math.sqrt(x * x + y * y + z * z);
  };
  
  /**
   * Caclulates the dot product of two vec3s
   *
   * @param {vec3} vec First operand
   * @param {vec3} vec2 Second operand
   *
   * @returns {number} Dot product of vec and vec2
   */
  vec3.dot = function (vec, vec2) {
      return vec[0] * vec2[0] + vec[1] * vec2[1] + vec[2] * vec2[2];
  };
  
  /**
   * Generates a unit vector pointing from one vector to another
   *
   * @param {vec3} vec Origin vec3
   * @param {vec3} vec2 vec3 to point to
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.direction = function (vec, vec2, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0] - vec2[0],
          y = vec[1] - vec2[1],
          z = vec[2] - vec2[2],
          len = Math.sqrt(x * x + y * y + z * z);
  
      if (!len) {
          dest[0] = 0;
          dest[1] = 0;
          dest[2] = 0;
          return dest;
      }
  
      len = 1 / len;
      dest[0] = x * len;
      dest[1] = y * len;
      dest[2] = z * len;
      return dest;
  };
  
  /**
   * Performs a linear interpolation between two vec3
   *
   * @param {vec3} vec First vector
   * @param {vec3} vec2 Second vector
   * @param {number} lerp Interpolation amount between the two inputs
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.lerp = function (vec, vec2, lerp, dest) {
      if (!dest) { dest = vec; }
  
      dest[0] = vec[0] + lerp * (vec2[0] - vec[0]);
      dest[1] = vec[1] + lerp * (vec2[1] - vec[1]);
      dest[2] = vec[2] + lerp * (vec2[2] - vec[2]);
  
      return dest;
  };
  
  /**
   * Calculates the euclidian distance between two vec3
   *
   * Params:
   * @param {vec3} vec First vector
   * @param {vec3} vec2 Second vector
   *
   * @returns {number} Distance between vec and vec2
   */
  vec3.dist = function (vec, vec2) {
      var x = vec2[0] - vec[0],
          y = vec2[1] - vec[1],
          z = vec2[2] - vec[2];
  
      return Math.sqrt(x*x + y*y + z*z);
  };
  
  /**
   * Projects the specified vec3 from screen space into object space
   * Based on the <a href="http://webcvs.freedesktop.org/mesa/Mesa/src/glu/mesa/project.c?revision=1.4&view=markup">Mesa gluUnProject implementation</a>
   *
   * @param {vec3} vec Screen-space vector to project
   * @param {mat4} view View matrix
   * @param {mat4} proj Projection matrix
   * @param {vec4} viewport Viewport as given to gl.viewport [x, y, width, height]
   * @param {vec3} [dest] vec3 receiving unprojected result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  vec3.unproject = function (vec, view, proj, viewport, dest) {
      if (!dest) { dest = vec; }
  
      var m = mat4.create();
      var v = new MatrixArray(4);
  
      v[0] = (vec[0] - viewport[0]) * 2.0 / viewport[2] - 1.0;
      v[1] = (vec[1] - viewport[1]) * 2.0 / viewport[3] - 1.0;
      v[2] = 2.0 * vec[2] - 1.0;
      v[3] = 1.0;
  
      mat4.multiply(proj, view, m);
      if(!mat4.inverse(m)) { return null; }
  
      mat4.multiplyVec4(m, v);
      if(v[3] === 0.0) { return null; }
  
      dest[0] = v[0] / v[3];
      dest[1] = v[1] / v[3];
      dest[2] = v[2] / v[3];
  
      return dest;
  };
  
  /**
   * Returns a string representation of a vector
   *
   * @param {vec3} vec Vector to represent as a string
   *
   * @returns {string} String representation of vec
   */
  vec3.str = function (vec) {
      return '[' + vec[0] + ', ' + vec[1] + ', ' + vec[2] + ']';
  };
  
  /*
   * mat3
   */
  
  /**
   * Creates a new instance of a mat3 using the default array type
   * Any javascript array-like object containing at least 9 numeric elements can serve as a mat3
   *
   * @param {mat3} [mat] mat3 containing values to initialize with
   *
   * @returns {mat3} New mat3
   */
  mat3.create = function (mat) {
      var dest = new MatrixArray(9);
  
      if (mat) {
          dest[0] = mat[0];
          dest[1] = mat[1];
          dest[2] = mat[2];
          dest[3] = mat[3];
          dest[4] = mat[4];
          dest[5] = mat[5];
          dest[6] = mat[6];
          dest[7] = mat[7];
          dest[8] = mat[8];
      }
  
      return dest;
  };
  
  /**
   * Copies the values of one mat3 to another
   *
   * @param {mat3} mat mat3 containing values to copy
   * @param {mat3} dest mat3 receiving copied values
   *
   * @returns {mat3} dest
   */
  mat3.set = function (mat, dest) {
      dest[0] = mat[0];
      dest[1] = mat[1];
      dest[2] = mat[2];
      dest[3] = mat[3];
      dest[4] = mat[4];
      dest[5] = mat[5];
      dest[6] = mat[6];
      dest[7] = mat[7];
      dest[8] = mat[8];
      return dest;
  };
  
  /**
   * Sets a mat3 to an identity matrix
   *
   * @param {mat3} dest mat3 to set
   *
   * @returns dest if specified, otherwise a new mat3
   */
  mat3.identity = function (dest) {
      if (!dest) { dest = mat3.create(); }
      dest[0] = 1;
      dest[1] = 0;
      dest[2] = 0;
      dest[3] = 0;
      dest[4] = 1;
      dest[5] = 0;
      dest[6] = 0;
      dest[7] = 0;
      dest[8] = 1;
      return dest;
  };
  
  /**
   * Transposes a mat3 (flips the values over the diagonal)
   *
   * Params:
   * @param {mat3} mat mat3 to transpose
   * @param {mat3} [dest] mat3 receiving transposed values. If not specified result is written to mat
   */
  mat3.transpose = function (mat, dest) {
      // If we are transposing ourselves we can skip a few steps but have to cache some values
      if (!dest || mat === dest) {
          var a01 = mat[1], a02 = mat[2],
              a12 = mat[5];
  
          mat[1] = mat[3];
          mat[2] = mat[6];
          mat[3] = a01;
          mat[5] = mat[7];
          mat[6] = a02;
          mat[7] = a12;
          return mat;
      }
  
      dest[0] = mat[0];
      dest[1] = mat[3];
      dest[2] = mat[6];
      dest[3] = mat[1];
      dest[4] = mat[4];
      dest[5] = mat[7];
      dest[6] = mat[2];
      dest[7] = mat[5];
      dest[8] = mat[8];
      return dest;
  };
  
  /**
   * Copies the elements of a mat3 into the upper 3x3 elements of a mat4
   *
   * @param {mat3} mat mat3 containing values to copy
   * @param {mat4} [dest] mat4 receiving copied values
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat3.toMat4 = function (mat, dest) {
      if (!dest) { dest = mat4.create(); }
  
      dest[15] = 1;
      dest[14] = 0;
      dest[13] = 0;
      dest[12] = 0;
  
      dest[11] = 0;
      dest[10] = mat[8];
      dest[9] = mat[7];
      dest[8] = mat[6];
  
      dest[7] = 0;
      dest[6] = mat[5];
      dest[5] = mat[4];
      dest[4] = mat[3];
  
      dest[3] = 0;
      dest[2] = mat[2];
      dest[1] = mat[1];
      dest[0] = mat[0];
  
      return dest;
  };
  
  /**
   * Returns a string representation of a mat3
   *
   * @param {mat3} mat mat3 to represent as a string
   *
   * @param {string} String representation of mat
   */
  mat3.str = function (mat) {
      return '[' + mat[0] + ', ' + mat[1] + ', ' + mat[2] +
          ', ' + mat[3] + ', ' + mat[4] + ', ' + mat[5] +
          ', ' + mat[6] + ', ' + mat[7] + ', ' + mat[8] + ']';
  };
  
  /*
   * mat4
   */
  
  /**
   * Creates a new instance of a mat4 using the default array type
   * Any javascript array-like object containing at least 16 numeric elements can serve as a mat4
   *
   * @param {mat4} [mat] mat4 containing values to initialize with
   *
   * @returns {mat4} New mat4
   */
  mat4.create = function (mat) {
      var dest = new MatrixArray(16);
  
      if (mat) {
          dest[0] = mat[0];
          dest[1] = mat[1];
          dest[2] = mat[2];
          dest[3] = mat[3];
          dest[4] = mat[4];
          dest[5] = mat[5];
          dest[6] = mat[6];
          dest[7] = mat[7];
          dest[8] = mat[8];
          dest[9] = mat[9];
          dest[10] = mat[10];
          dest[11] = mat[11];
          dest[12] = mat[12];
          dest[13] = mat[13];
          dest[14] = mat[14];
          dest[15] = mat[15];
      }
  
      return dest;
  };
  
  /**
   * Copies the values of one mat4 to another
   *
   * @param {mat4} mat mat4 containing values to copy
   * @param {mat4} dest mat4 receiving copied values
   *
   * @returns {mat4} dest
   */
  mat4.set = function (mat, dest) {
      dest[0] = mat[0];
      dest[1] = mat[1];
      dest[2] = mat[2];
      dest[3] = mat[3];
      dest[4] = mat[4];
      dest[5] = mat[5];
      dest[6] = mat[6];
      dest[7] = mat[7];
      dest[8] = mat[8];
      dest[9] = mat[9];
      dest[10] = mat[10];
      dest[11] = mat[11];
      dest[12] = mat[12];
      dest[13] = mat[13];
      dest[14] = mat[14];
      dest[15] = mat[15];
      return dest;
  };
  
  /**
   * Sets a mat4 to an identity matrix
   *
   * @param {mat4} dest mat4 to set
   *
   * @returns {mat4} dest
   */
  mat4.identity = function (dest) {
      if (!dest) { dest = mat4.create(); }
      dest[0] = 1;
      dest[1] = 0;
      dest[2] = 0;
      dest[3] = 0;
      dest[4] = 0;
      dest[5] = 1;
      dest[6] = 0;
      dest[7] = 0;
      dest[8] = 0;
      dest[9] = 0;
      dest[10] = 1;
      dest[11] = 0;
      dest[12] = 0;
      dest[13] = 0;
      dest[14] = 0;
      dest[15] = 1;
      return dest;
  };
  
  /**
   * Transposes a mat4 (flips the values over the diagonal)
   *
   * @param {mat4} mat mat4 to transpose
   * @param {mat4} [dest] mat4 receiving transposed values. If not specified result is written to mat
   */
  mat4.transpose = function (mat, dest) {
      // If we are transposing ourselves we can skip a few steps but have to cache some values
      if (!dest || mat === dest) {
          var a01 = mat[1], a02 = mat[2], a03 = mat[3],
              a12 = mat[6], a13 = mat[7],
              a23 = mat[11];
  
          mat[1] = mat[4];
          mat[2] = mat[8];
          mat[3] = mat[12];
          mat[4] = a01;
          mat[6] = mat[9];
          mat[7] = mat[13];
          mat[8] = a02;
          mat[9] = a12;
          mat[11] = mat[14];
          mat[12] = a03;
          mat[13] = a13;
          mat[14] = a23;
          return mat;
      }
  
      dest[0] = mat[0];
      dest[1] = mat[4];
      dest[2] = mat[8];
      dest[3] = mat[12];
      dest[4] = mat[1];
      dest[5] = mat[5];
      dest[6] = mat[9];
      dest[7] = mat[13];
      dest[8] = mat[2];
      dest[9] = mat[6];
      dest[10] = mat[10];
      dest[11] = mat[14];
      dest[12] = mat[3];
      dest[13] = mat[7];
      dest[14] = mat[11];
      dest[15] = mat[15];
      return dest;
  };
  
  /**
   * Calculates the determinant of a mat4
   *
   * @param {mat4} mat mat4 to calculate determinant of
   *
   * @returns {number} determinant of mat
   */
  mat4.determinant = function (mat) {
      // Cache the matrix values (makes for huge speed increases!)
      var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3],
          a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7],
          a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11],
          a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15];
  
      return (a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
              a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
              a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
              a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
              a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
              a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33);
  };
  
  /**
   * Calculates the inverse matrix of a mat4
   *
   * @param {mat4} mat mat4 to calculate inverse of
   * @param {mat4} [dest] mat4 receiving inverse matrix. If not specified result is written to mat, null if matrix cannot be inverted
   */
  mat4.inverse = function (mat, dest) {
      if (!dest) { dest = mat; }
  
      // Cache the matrix values (makes for huge speed increases!)
      var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3],
          a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7],
          a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11],
          a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15],
  
          b00 = a00 * a11 - a01 * a10,
          b01 = a00 * a12 - a02 * a10,
          b02 = a00 * a13 - a03 * a10,
          b03 = a01 * a12 - a02 * a11,
          b04 = a01 * a13 - a03 * a11,
          b05 = a02 * a13 - a03 * a12,
          b06 = a20 * a31 - a21 * a30,
          b07 = a20 * a32 - a22 * a30,
          b08 = a20 * a33 - a23 * a30,
          b09 = a21 * a32 - a22 * a31,
          b10 = a21 * a33 - a23 * a31,
          b11 = a22 * a33 - a23 * a32,
  
          d = (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06),
          invDet;
  
          // Calculate the determinant
          if (!d) { return null; }
          invDet = 1 / d;
  
      dest[0] = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
      dest[1] = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
      dest[2] = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
      dest[3] = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
      dest[4] = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
      dest[5] = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
      dest[6] = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
      dest[7] = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
      dest[8] = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
      dest[9] = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
      dest[10] = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
      dest[11] = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
      dest[12] = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
      dest[13] = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
      dest[14] = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
      dest[15] = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
  
      return dest;
  };
  
  /**
   * Copies the upper 3x3 elements of a mat4 into another mat4
   *
   * @param {mat4} mat mat4 containing values to copy
   * @param {mat4} [dest] mat4 receiving copied values
   *
   * @returns {mat4} dest is specified, a new mat4 otherwise
   */
  mat4.toRotationMat = function (mat, dest) {
      if (!dest) { dest = mat4.create(); }
  
      dest[0] = mat[0];
      dest[1] = mat[1];
      dest[2] = mat[2];
      dest[3] = mat[3];
      dest[4] = mat[4];
      dest[5] = mat[5];
      dest[6] = mat[6];
      dest[7] = mat[7];
      dest[8] = mat[8];
      dest[9] = mat[9];
      dest[10] = mat[10];
      dest[11] = mat[11];
      dest[12] = 0;
      dest[13] = 0;
      dest[14] = 0;
      dest[15] = 1;
  
      return dest;
  };
  
  /**
   * Copies the upper 3x3 elements of a mat4 into a mat3
   *
   * @param {mat4} mat mat4 containing values to copy
   * @param {mat3} [dest] mat3 receiving copied values
   *
   * @returns {mat3} dest is specified, a new mat3 otherwise
   */
  mat4.toMat3 = function (mat, dest) {
      if (!dest) { dest = mat3.create(); }
  
      dest[0] = mat[0];
      dest[1] = mat[1];
      dest[2] = mat[2];
      dest[3] = mat[4];
      dest[4] = mat[5];
      dest[5] = mat[6];
      dest[6] = mat[8];
      dest[7] = mat[9];
      dest[8] = mat[10];
  
      return dest;
  };
  
  /**
   * Calculates the inverse of the upper 3x3 elements of a mat4 and copies the result into a mat3
   * The resulting matrix is useful for calculating transformed normals
   *
   * Params:
   * @param {mat4} mat mat4 containing values to invert and copy
   * @param {mat3} [dest] mat3 receiving values
   *
   * @returns {mat3} dest is specified, a new mat3 otherwise, null if the matrix cannot be inverted
   */
  mat4.toInverseMat3 = function (mat, dest) {
      // Cache the matrix values (makes for huge speed increases!)
      var a00 = mat[0], a01 = mat[1], a02 = mat[2],
          a10 = mat[4], a11 = mat[5], a12 = mat[6],
          a20 = mat[8], a21 = mat[9], a22 = mat[10],
  
          b01 = a22 * a11 - a12 * a21,
          b11 = -a22 * a10 + a12 * a20,
          b21 = a21 * a10 - a11 * a20,
  
          d = a00 * b01 + a01 * b11 + a02 * b21,
          id;
  
      if (!d) { return null; }
      id = 1 / d;
  
      if (!dest) { dest = mat3.create(); }
  
      dest[0] = b01 * id;
      dest[1] = (-a22 * a01 + a02 * a21) * id;
      dest[2] = (a12 * a01 - a02 * a11) * id;
      dest[3] = b11 * id;
      dest[4] = (a22 * a00 - a02 * a20) * id;
      dest[5] = (-a12 * a00 + a02 * a10) * id;
      dest[6] = b21 * id;
      dest[7] = (-a21 * a00 + a01 * a20) * id;
      dest[8] = (a11 * a00 - a01 * a10) * id;
  
      return dest;
  };
  
  /**
   * Performs a matrix multiplication
   *
   * @param {mat4} mat First operand
   * @param {mat4} mat2 Second operand
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.multiply = function (mat, mat2, dest) {
      if (!dest) { dest = mat; }
  
      // Cache the matrix values (makes for huge speed increases!)
      var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3],
          a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7],
          a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11],
          a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15],
  
          b00 = mat2[0], b01 = mat2[1], b02 = mat2[2], b03 = mat2[3],
          b10 = mat2[4], b11 = mat2[5], b12 = mat2[6], b13 = mat2[7],
          b20 = mat2[8], b21 = mat2[9], b22 = mat2[10], b23 = mat2[11],
          b30 = mat2[12], b31 = mat2[13], b32 = mat2[14], b33 = mat2[15];
  
      dest[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
      dest[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
      dest[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
      dest[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;
      dest[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
      dest[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
      dest[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
      dest[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;
      dest[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
      dest[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
      dest[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
      dest[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;
      dest[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
      dest[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
      dest[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
      dest[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
  
      return dest;
  };
  
  /**
   * Transforms a vec3 with the given matrix
   * 4th vector component is implicitly '1'
   *
   * @param {mat4} mat mat4 to transform the vector with
   * @param {vec3} vec vec3 to transform
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec3} dest if specified, vec otherwise
   */
  mat4.multiplyVec3 = function (mat, vec, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0], y = vec[1], z = vec[2];
  
      dest[0] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12];
      dest[1] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13];
      dest[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
  
      return dest;
  };
  
  /**
   * Transforms a vec4 with the given matrix
   *
   * @param {mat4} mat mat4 to transform the vector with
   * @param {vec4} vec vec4 to transform
   * @param {vec4} [dest] vec4 receiving operation result. If not specified result is written to vec
   *
   * @returns {vec4} dest if specified, vec otherwise
   */
  mat4.multiplyVec4 = function (mat, vec, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0], y = vec[1], z = vec[2], w = vec[3];
  
      dest[0] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12] * w;
      dest[1] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13] * w;
      dest[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14] * w;
      dest[3] = mat[3] * x + mat[7] * y + mat[11] * z + mat[15] * w;
  
      return dest;
  };
  
  /**
   * Translates a matrix by the given vector
   *
   * @param {mat4} mat mat4 to translate
   * @param {vec3} vec vec3 specifying the translation
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.translate = function (mat, vec, dest) {
      var x = vec[0], y = vec[1], z = vec[2],
          a00, a01, a02, a03,
          a10, a11, a12, a13,
          a20, a21, a22, a23;
  
      if (!dest || mat === dest) {
          mat[12] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12];
          mat[13] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13];
          mat[14] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
          mat[15] = mat[3] * x + mat[7] * y + mat[11] * z + mat[15];
          return mat;
      }
  
      a00 = mat[0]; a01 = mat[1]; a02 = mat[2]; a03 = mat[3];
      a10 = mat[4]; a11 = mat[5]; a12 = mat[6]; a13 = mat[7];
      a20 = mat[8]; a21 = mat[9]; a22 = mat[10]; a23 = mat[11];
  
      dest[0] = a00; dest[1] = a01; dest[2] = a02; dest[3] = a03;
      dest[4] = a10; dest[5] = a11; dest[6] = a12; dest[7] = a13;
      dest[8] = a20; dest[9] = a21; dest[10] = a22; dest[11] = a23;
  
      dest[12] = a00 * x + a10 * y + a20 * z + mat[12];
      dest[13] = a01 * x + a11 * y + a21 * z + mat[13];
      dest[14] = a02 * x + a12 * y + a22 * z + mat[14];
      dest[15] = a03 * x + a13 * y + a23 * z + mat[15];
      return dest;
  };
  
  /**
   * Scales a matrix by the given vector
   *
   * @param {mat4} mat mat4 to scale
   * @param {vec3} vec vec3 specifying the scale for each axis
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.scale = function (mat, vec, dest) {
      var x = vec[0], y = vec[1], z = vec[2];
  
      if (!dest || mat === dest) {
          mat[0] *= x;
          mat[1] *= x;
          mat[2] *= x;
          mat[3] *= x;
          mat[4] *= y;
          mat[5] *= y;
          mat[6] *= y;
          mat[7] *= y;
          mat[8] *= z;
          mat[9] *= z;
          mat[10] *= z;
          mat[11] *= z;
          return mat;
      }
  
      dest[0] = mat[0] * x;
      dest[1] = mat[1] * x;
      dest[2] = mat[2] * x;
      dest[3] = mat[3] * x;
      dest[4] = mat[4] * y;
      dest[5] = mat[5] * y;
      dest[6] = mat[6] * y;
      dest[7] = mat[7] * y;
      dest[8] = mat[8] * z;
      dest[9] = mat[9] * z;
      dest[10] = mat[10] * z;
      dest[11] = mat[11] * z;
      dest[12] = mat[12];
      dest[13] = mat[13];
      dest[14] = mat[14];
      dest[15] = mat[15];
      return dest;
  };
  
  /**
   * Rotates a matrix by the given angle around the specified axis
   * If rotating around a primary axis (X,Y,Z) one of the specialized rotation functions should be used instead for performance
   *
   * @param {mat4} mat mat4 to rotate
   * @param {number} angle Angle (in radians) to rotate
   * @param {vec3} axis vec3 representing the axis to rotate around
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.rotate = function (mat, angle, axis, dest) {
      var x = axis[0], y = axis[1], z = axis[2],
          len = Math.sqrt(x * x + y * y + z * z),
          s, c, t,
          a00, a01, a02, a03,
          a10, a11, a12, a13,
          a20, a21, a22, a23,
          b00, b01, b02,
          b10, b11, b12,
          b20, b21, b22;
  
      if (!len) { return null; }
      if (len !== 1) {
          len = 1 / len;
          x *= len;
          y *= len;
          z *= len;
      }
  
      s = Math.sin(angle);
      c = Math.cos(angle);
      t = 1 - c;
  
      a00 = mat[0]; a01 = mat[1]; a02 = mat[2]; a03 = mat[3];
      a10 = mat[4]; a11 = mat[5]; a12 = mat[6]; a13 = mat[7];
      a20 = mat[8]; a21 = mat[9]; a22 = mat[10]; a23 = mat[11];
  
      // Construct the elements of the rotation matrix
      b00 = x * x * t + c; b01 = y * x * t + z * s; b02 = z * x * t - y * s;
      b10 = x * y * t - z * s; b11 = y * y * t + c; b12 = z * y * t + x * s;
      b20 = x * z * t + y * s; b21 = y * z * t - x * s; b22 = z * z * t + c;
  
      if (!dest) {
          dest = mat;
      } else if (mat !== dest) { // If the source and destination differ, copy the unchanged last row
          dest[12] = mat[12];
          dest[13] = mat[13];
          dest[14] = mat[14];
          dest[15] = mat[15];
      }
  
      // Perform rotation-specific matrix multiplication
      dest[0] = a00 * b00 + a10 * b01 + a20 * b02;
      dest[1] = a01 * b00 + a11 * b01 + a21 * b02;
      dest[2] = a02 * b00 + a12 * b01 + a22 * b02;
      dest[3] = a03 * b00 + a13 * b01 + a23 * b02;
  
      dest[4] = a00 * b10 + a10 * b11 + a20 * b12;
      dest[5] = a01 * b10 + a11 * b11 + a21 * b12;
      dest[6] = a02 * b10 + a12 * b11 + a22 * b12;
      dest[7] = a03 * b10 + a13 * b11 + a23 * b12;
  
      dest[8] = a00 * b20 + a10 * b21 + a20 * b22;
      dest[9] = a01 * b20 + a11 * b21 + a21 * b22;
      dest[10] = a02 * b20 + a12 * b21 + a22 * b22;
      dest[11] = a03 * b20 + a13 * b21 + a23 * b22;
      return dest;
  };
  
  /**
   * Rotates a matrix by the given angle around the X axis
   *
   * @param {mat4} mat mat4 to rotate
   * @param {number} angle Angle (in radians) to rotate
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.rotateX = function (mat, angle, dest) {
      var s = Math.sin(angle),
          c = Math.cos(angle),
          a10 = mat[4],
          a11 = mat[5],
          a12 = mat[6],
          a13 = mat[7],
          a20 = mat[8],
          a21 = mat[9],
          a22 = mat[10],
          a23 = mat[11];
  
      if (!dest) {
          dest = mat;
      } else if (mat !== dest) { // If the source and destination differ, copy the unchanged rows
          dest[0] = mat[0];
          dest[1] = mat[1];
          dest[2] = mat[2];
          dest[3] = mat[3];
  
          dest[12] = mat[12];
          dest[13] = mat[13];
          dest[14] = mat[14];
          dest[15] = mat[15];
      }
  
      // Perform axis-specific matrix multiplication
      dest[4] = a10 * c + a20 * s;
      dest[5] = a11 * c + a21 * s;
      dest[6] = a12 * c + a22 * s;
      dest[7] = a13 * c + a23 * s;
  
      dest[8] = a10 * -s + a20 * c;
      dest[9] = a11 * -s + a21 * c;
      dest[10] = a12 * -s + a22 * c;
      dest[11] = a13 * -s + a23 * c;
      return dest;
  };
  
  /**
   * Rotates a matrix by the given angle around the Y axis
   *
   * @param {mat4} mat mat4 to rotate
   * @param {number} angle Angle (in radians) to rotate
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.rotateY = function (mat, angle, dest) {
      var s = Math.sin(angle),
          c = Math.cos(angle),
          a00 = mat[0],
          a01 = mat[1],
          a02 = mat[2],
          a03 = mat[3],
          a20 = mat[8],
          a21 = mat[9],
          a22 = mat[10],
          a23 = mat[11];
  
      if (!dest) {
          dest = mat;
      } else if (mat !== dest) { // If the source and destination differ, copy the unchanged rows
          dest[4] = mat[4];
          dest[5] = mat[5];
          dest[6] = mat[6];
          dest[7] = mat[7];
  
          dest[12] = mat[12];
          dest[13] = mat[13];
          dest[14] = mat[14];
          dest[15] = mat[15];
      }
  
      // Perform axis-specific matrix multiplication
      dest[0] = a00 * c + a20 * -s;
      dest[1] = a01 * c + a21 * -s;
      dest[2] = a02 * c + a22 * -s;
      dest[3] = a03 * c + a23 * -s;
  
      dest[8] = a00 * s + a20 * c;
      dest[9] = a01 * s + a21 * c;
      dest[10] = a02 * s + a22 * c;
      dest[11] = a03 * s + a23 * c;
      return dest;
  };
  
  /**
   * Rotates a matrix by the given angle around the Z axis
   *
   * @param {mat4} mat mat4 to rotate
   * @param {number} angle Angle (in radians) to rotate
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
   */
  mat4.rotateZ = function (mat, angle, dest) {
      var s = Math.sin(angle),
          c = Math.cos(angle),
          a00 = mat[0],
          a01 = mat[1],
          a02 = mat[2],
          a03 = mat[3],
          a10 = mat[4],
          a11 = mat[5],
          a12 = mat[6],
          a13 = mat[7];
  
      if (!dest) {
          dest = mat;
      } else if (mat !== dest) { // If the source and destination differ, copy the unchanged last row
          dest[8] = mat[8];
          dest[9] = mat[9];
          dest[10] = mat[10];
          dest[11] = mat[11];
  
          dest[12] = mat[12];
          dest[13] = mat[13];
          dest[14] = mat[14];
          dest[15] = mat[15];
      }
  
      // Perform axis-specific matrix multiplication
      dest[0] = a00 * c + a10 * s;
      dest[1] = a01 * c + a11 * s;
      dest[2] = a02 * c + a12 * s;
      dest[3] = a03 * c + a13 * s;
  
      dest[4] = a00 * -s + a10 * c;
      dest[5] = a01 * -s + a11 * c;
      dest[6] = a02 * -s + a12 * c;
      dest[7] = a03 * -s + a13 * c;
  
      return dest;
  };
  
  /**
   * Generates a frustum matrix with the given bounds
   *
   * @param {number} left Left bound of the frustum
   * @param {number} right Right bound of the frustum
   * @param {number} bottom Bottom bound of the frustum
   * @param {number} top Top bound of the frustum
   * @param {number} near Near bound of the frustum
   * @param {number} far Far bound of the frustum
   * @param {mat4} [dest] mat4 frustum matrix will be written into
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat4.frustum = function (left, right, bottom, top, near, far, dest) {
      if (!dest) { dest = mat4.create(); }
      var rl = (right - left),
          tb = (top - bottom),
          fn = (far - near);
      dest[0] = (near * 2) / rl;
      dest[1] = 0;
      dest[2] = 0;
      dest[3] = 0;
      dest[4] = 0;
      dest[5] = (near * 2) / tb;
      dest[6] = 0;
      dest[7] = 0;
      dest[8] = (right + left) / rl;
      dest[9] = (top + bottom) / tb;
      dest[10] = -(far + near) / fn;
      dest[11] = -1;
      dest[12] = 0;
      dest[13] = 0;
      dest[14] = -(far * near * 2) / fn;
      dest[15] = 0;
      return dest;
  };
  
  /**
   * Generates a perspective projection matrix with the given bounds
   *
   * @param {number} fovy Vertical field of view
   * @param {number} aspect Aspect ratio. typically viewport width/height
   * @param {number} near Near bound of the frustum
   * @param {number} far Far bound of the frustum
   * @param {mat4} [dest] mat4 frustum matrix will be written into
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat4.perspective = function (fovy, aspect, near, far, dest) {
      var top = near * Math.tan(fovy * Math.PI / 360.0),
          right = top * aspect;
      return mat4.frustum(-right, right, -top, top, near, far, dest);
  };
  
  /**
   * Generates a orthogonal projection matrix with the given bounds
   *
   * @param {number} left Left bound of the frustum
   * @param {number} right Right bound of the frustum
   * @param {number} bottom Bottom bound of the frustum
   * @param {number} top Top bound of the frustum
   * @param {number} near Near bound of the frustum
   * @param {number} far Far bound of the frustum
   * @param {mat4} [dest] mat4 frustum matrix will be written into
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat4.ortho = function (left, right, bottom, top, near, far, dest) {
      if (!dest) { dest = mat4.create(); }
      var rl = (right - left),
          tb = (top - bottom),
          fn = (far - near);
      dest[0] = 2 / rl;
      dest[1] = 0;
      dest[2] = 0;
      dest[3] = 0;
      dest[4] = 0;
      dest[5] = 2 / tb;
      dest[6] = 0;
      dest[7] = 0;
      dest[8] = 0;
      dest[9] = 0;
      dest[10] = -2 / fn;
      dest[11] = 0;
      dest[12] = -(left + right) / rl;
      dest[13] = -(top + bottom) / tb;
      dest[14] = -(far + near) / fn;
      dest[15] = 1;
      return dest;
  };
  
  /**
   * Generates a look-at matrix with the given eye position, focal point, and up axis
   *
   * @param {vec3} eye Position of the viewer
   * @param {vec3} center Point the viewer is looking at
   * @param {vec3} up vec3 pointing "up"
   * @param {mat4} [dest] mat4 frustum matrix will be written into
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat4.lookAt = function (eye, center, up, dest) {
      if (!dest) { dest = mat4.create(); }
  
      var x0, x1, x2, y0, y1, y2, z0, z1, z2, len,
          eyex = eye[0],
          eyey = eye[1],
          eyez = eye[2],
          upx = up[0],
          upy = up[1],
          upz = up[2],
          centerx = center[0],
          centery = center[1],
          centerz = center[2];
  
      if (eyex === centerx && eyey === centery && eyez === centerz) {
          return mat4.identity(dest);
      }
  
      //vec3.direction(eye, center, z);
      z0 = eyex - centerx;
      z1 = eyey - centery;
      z2 = eyez - centerz;
  
      // normalize (no check needed for 0 because of early return)
      len = 1 / Math.sqrt(z0 * z0 + z1 * z1 + z2 * z2);
      z0 *= len;
      z1 *= len;
      z2 *= len;
  
      //vec3.normalize(vec3.cross(up, z, x));
      x0 = upy * z2 - upz * z1;
      x1 = upz * z0 - upx * z2;
      x2 = upx * z1 - upy * z0;
      len = Math.sqrt(x0 * x0 + x1 * x1 + x2 * x2);
      if (!len) {
          x0 = 0;
          x1 = 0;
          x2 = 0;
      } else {
          len = 1 / len;
          x0 *= len;
          x1 *= len;
          x2 *= len;
      }
  
      //vec3.normalize(vec3.cross(z, x, y));
      y0 = z1 * x2 - z2 * x1;
      y1 = z2 * x0 - z0 * x2;
      y2 = z0 * x1 - z1 * x0;
  
      len = Math.sqrt(y0 * y0 + y1 * y1 + y2 * y2);
      if (!len) {
          y0 = 0;
          y1 = 0;
          y2 = 0;
      } else {
          len = 1 / len;
          y0 *= len;
          y1 *= len;
          y2 *= len;
      }
  
      dest[0] = x0;
      dest[1] = y0;
      dest[2] = z0;
      dest[3] = 0;
      dest[4] = x1;
      dest[5] = y1;
      dest[6] = z1;
      dest[7] = 0;
      dest[8] = x2;
      dest[9] = y2;
      dest[10] = z2;
      dest[11] = 0;
      dest[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
      dest[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
      dest[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
      dest[15] = 1;
  
      return dest;
  };
  
  /**
   * Creates a matrix from a quaternion rotation and vector translation
   * This is equivalent to (but much faster than):
   *
   *     mat4.identity(dest);
   *     mat4.translate(dest, vec);
   *     var quatMat = mat4.create();
   *     quat4.toMat4(quat, quatMat);
   *     mat4.multiply(dest, quatMat);
   *
   * @param {quat4} quat Rotation quaternion
   * @param {vec3} vec Translation vector
   * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to a new mat4
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  mat4.fromRotationTranslation = function (quat, vec, dest) {
      if (!dest) { dest = mat4.create(); }
  
      // Quaternion math
      var x = quat[0], y = quat[1], z = quat[2], w = quat[3],
          x2 = x + x,
          y2 = y + y,
          z2 = z + z,
  
          xx = x * x2,
          xy = x * y2,
          xz = x * z2,
          yy = y * y2,
          yz = y * z2,
          zz = z * z2,
          wx = w * x2,
          wy = w * y2,
          wz = w * z2;
  
      dest[0] = 1 - (yy + zz);
      dest[1] = xy + wz;
      dest[2] = xz - wy;
      dest[3] = 0;
      dest[4] = xy - wz;
      dest[5] = 1 - (xx + zz);
      dest[6] = yz + wx;
      dest[7] = 0;
      dest[8] = xz + wy;
      dest[9] = yz - wx;
      dest[10] = 1 - (xx + yy);
      dest[11] = 0;
      dest[12] = vec[0];
      dest[13] = vec[1];
      dest[14] = vec[2];
      dest[15] = 1;
  
      return dest;
  };
  
  /**
   * Returns a string representation of a mat4
   *
   * @param {mat4} mat mat4 to represent as a string
   *
   * @returns {string} String representation of mat
   */
  mat4.str = function (mat) {
      return '[' + mat[0] + ', ' + mat[1] + ', ' + mat[2] + ', ' + mat[3] +
          ', ' + mat[4] + ', ' + mat[5] + ', ' + mat[6] + ', ' + mat[7] +
          ', ' + mat[8] + ', ' + mat[9] + ', ' + mat[10] + ', ' + mat[11] +
          ', ' + mat[12] + ', ' + mat[13] + ', ' + mat[14] + ', ' + mat[15] + ']';
  };
  
  /*
   * quat4
   */
  
  /**
   * Creates a new instance of a quat4 using the default array type
   * Any javascript array containing at least 4 numeric elements can serve as a quat4
   *
   * @param {quat4} [quat] quat4 containing values to initialize with
   *
   * @returns {quat4} New quat4
   */
  quat4.create = function (quat) {
      var dest = new MatrixArray(4);
  
      if (quat) {
          dest[0] = quat[0];
          dest[1] = quat[1];
          dest[2] = quat[2];
          dest[3] = quat[3];
      }
  
      return dest;
  };
  
  /**
   * Copies the values of one quat4 to another
   *
   * @param {quat4} quat quat4 containing values to copy
   * @param {quat4} dest quat4 receiving copied values
   *
   * @returns {quat4} dest
   */
  quat4.set = function (quat, dest) {
      dest[0] = quat[0];
      dest[1] = quat[1];
      dest[2] = quat[2];
      dest[3] = quat[3];
  
      return dest;
  };
  
  /**
   * Calculates the W component of a quat4 from the X, Y, and Z components.
   * Assumes that quaternion is 1 unit in length.
   * Any existing W component will be ignored.
   *
   * @param {quat4} quat quat4 to calculate W component of
   * @param {quat4} [dest] quat4 receiving calculated values. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.calculateW = function (quat, dest) {
      var x = quat[0], y = quat[1], z = quat[2];
  
      if (!dest || quat === dest) {
          quat[3] = -Math.sqrt(Math.abs(1.0 - x * x - y * y - z * z));
          return quat;
      }
      dest[0] = x;
      dest[1] = y;
      dest[2] = z;
      dest[3] = -Math.sqrt(Math.abs(1.0 - x * x - y * y - z * z));
      return dest;
  };
  
  /**
   * Calculates the dot product of two quaternions
   *
   * @param {quat4} quat First operand
   * @param {quat4} quat2 Second operand
   *
   * @return {number} Dot product of quat and quat2
   */
  quat4.dot = function(quat, quat2){
      return quat[0]*quat2[0] + quat[1]*quat2[1] + quat[2]*quat2[2] + quat[3]*quat2[3];
  };
  
  /**
   * Calculates the inverse of a quat4
   *
   * @param {quat4} quat quat4 to calculate inverse of
   * @param {quat4} [dest] quat4 receiving inverse values. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.inverse = function(quat, dest) {
      var q0 = quat[0], q1 = quat[1], q2 = quat[2], q3 = quat[3],
          dot = q0*q0 + q1*q1 + q2*q2 + q3*q3,
          invDot = dot ? 1.0/dot : 0;
  
      // TODO: Would be faster to return [0,0,0,0] immediately if dot == 0
  
      if(!dest || quat === dest) {
          quat[0] *= -invDot;
          quat[1] *= -invDot;
          quat[2] *= -invDot;
          quat[3] *= invDot;
          return quat;
      }
      dest[0] = -quat[0]*invDot;
      dest[1] = -quat[1]*invDot;
      dest[2] = -quat[2]*invDot;
      dest[3] = quat[3]*invDot;
      return dest;
  };
  
  
  /**
   * Calculates the conjugate of a quat4
   * If the quaternion is normalized, this function is faster than quat4.inverse and produces the same result.
   *
   * @param {quat4} quat quat4 to calculate conjugate of
   * @param {quat4} [dest] quat4 receiving conjugate values. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.conjugate = function (quat, dest) {
      if (!dest || quat === dest) {
          quat[0] *= -1;
          quat[1] *= -1;
          quat[2] *= -1;
          return quat;
      }
      dest[0] = -quat[0];
      dest[1] = -quat[1];
      dest[2] = -quat[2];
      dest[3] = quat[3];
      return dest;
  };
  
  /**
   * Calculates the length of a quat4
   *
   * Params:
   * @param {quat4} quat quat4 to calculate length of
   *
   * @returns Length of quat
   */
  quat4.length = function (quat) {
      var x = quat[0], y = quat[1], z = quat[2], w = quat[3];
      return Math.sqrt(x * x + y * y + z * z + w * w);
  };
  
  /**
   * Generates a unit quaternion of the same direction as the provided quat4
   * If quaternion length is 0, returns [0, 0, 0, 0]
   *
   * @param {quat4} quat quat4 to normalize
   * @param {quat4} [dest] quat4 receiving operation result. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.normalize = function (quat, dest) {
      if (!dest) { dest = quat; }
  
      var x = quat[0], y = quat[1], z = quat[2], w = quat[3],
          len = Math.sqrt(x * x + y * y + z * z + w * w);
      if (len === 0) {
          dest[0] = 0;
          dest[1] = 0;
          dest[2] = 0;
          dest[3] = 0;
          return dest;
      }
      len = 1 / len;
      dest[0] = x * len;
      dest[1] = y * len;
      dest[2] = z * len;
      dest[3] = w * len;
  
      return dest;
  };
  
  /**
   * Performs quaternion addition
   *
   * @param {quat4} quat First operand
   * @param {quat4} quat2 Second operand
   * @param {quat4} [dest] quat4 receiving operation result. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.add = function (quat, quat2, dest) {
      if(!dest || quat === dest) {
          quat[0] += quat2[0];
          quat[1] += quat2[1];
          quat[2] += quat2[2];
          quat[3] += quat2[3];
          return quat;
      }
      dest[0] = quat[0]+quat2[0];
      dest[1] = quat[1]+quat2[1];
      dest[2] = quat[2]+quat2[2];
      dest[3] = quat[3]+quat2[3];
      return dest;
  };
  
  /**
   * Performs a quaternion multiplication
   *
   * @param {quat4} quat First operand
   * @param {quat4} quat2 Second operand
   * @param {quat4} [dest] quat4 receiving operation result. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.multiply = function (quat, quat2, dest) {
      if (!dest) { dest = quat; }
  
      var qax = quat[0], qay = quat[1], qaz = quat[2], qaw = quat[3],
          qbx = quat2[0], qby = quat2[1], qbz = quat2[2], qbw = quat2[3];
  
      dest[0] = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
      dest[1] = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
      dest[2] = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
      dest[3] = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;
  
      return dest;
  };
  
  /**
   * Transforms a vec3 with the given quaternion
   *
   * @param {quat4} quat quat4 to transform the vector with
   * @param {vec3} vec vec3 to transform
   * @param {vec3} [dest] vec3 receiving operation result. If not specified result is written to vec
   *
   * @returns dest if specified, vec otherwise
   */
  quat4.multiplyVec3 = function (quat, vec, dest) {
      if (!dest) { dest = vec; }
  
      var x = vec[0], y = vec[1], z = vec[2],
          qx = quat[0], qy = quat[1], qz = quat[2], qw = quat[3],
  
          // calculate quat * vec
          ix = qw * x + qy * z - qz * y,
          iy = qw * y + qz * x - qx * z,
          iz = qw * z + qx * y - qy * x,
          iw = -qx * x - qy * y - qz * z;
  
      // calculate result * inverse quat
      dest[0] = ix * qw + iw * -qx + iy * -qz - iz * -qy;
      dest[1] = iy * qw + iw * -qy + iz * -qx - ix * -qz;
      dest[2] = iz * qw + iw * -qz + ix * -qy - iy * -qx;
  
      return dest;
  };
  
  /**
   * Multiplies the components of a quaternion by a scalar value
   *
   * @param {quat4} quat to scale
   * @param {number} val Value to scale by
   * @param {quat4} [dest] quat4 receiving operation result. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.scale = function (quat, val, dest) {
      if(!dest || quat === dest) {
          quat[0] *= val;
          quat[1] *= val;
          quat[2] *= val;
          quat[3] *= val;
          return quat;
      }
      dest[0] = quat[0]*val;
      dest[1] = quat[1]*val;
      dest[2] = quat[2]*val;
      dest[3] = quat[3]*val;
      return dest;
  };
  
  /**
   * Calculates a 3x3 matrix from the given quat4
   *
   * @param {quat4} quat quat4 to create matrix from
   * @param {mat3} [dest] mat3 receiving operation result
   *
   * @returns {mat3} dest if specified, a new mat3 otherwise
   */
  quat4.toMat3 = function (quat, dest) {
      if (!dest) { dest = mat3.create(); }
  
      var x = quat[0], y = quat[1], z = quat[2], w = quat[3],
          x2 = x + x,
          y2 = y + y,
          z2 = z + z,
  
          xx = x * x2,
          xy = x * y2,
          xz = x * z2,
          yy = y * y2,
          yz = y * z2,
          zz = z * z2,
          wx = w * x2,
          wy = w * y2,
          wz = w * z2;
  
      dest[0] = 1 - (yy + zz);
      dest[1] = xy + wz;
      dest[2] = xz - wy;
  
      dest[3] = xy - wz;
      dest[4] = 1 - (xx + zz);
      dest[5] = yz + wx;
  
      dest[6] = xz + wy;
      dest[7] = yz - wx;
      dest[8] = 1 - (xx + yy);
  
      return dest;
  };
  
  /**
   * Calculates a 4x4 matrix from the given quat4
   *
   * @param {quat4} quat quat4 to create matrix from
   * @param {mat4} [dest] mat4 receiving operation result
   *
   * @returns {mat4} dest if specified, a new mat4 otherwise
   */
  quat4.toMat4 = function (quat, dest) {
      if (!dest) { dest = mat4.create(); }
  
      var x = quat[0], y = quat[1], z = quat[2], w = quat[3],
          x2 = x + x,
          y2 = y + y,
          z2 = z + z,
  
          xx = x * x2,
          xy = x * y2,
          xz = x * z2,
          yy = y * y2,
          yz = y * z2,
          zz = z * z2,
          wx = w * x2,
          wy = w * y2,
          wz = w * z2;
  
      dest[0] = 1 - (yy + zz);
      dest[1] = xy + wz;
      dest[2] = xz - wy;
      dest[3] = 0;
  
      dest[4] = xy - wz;
      dest[5] = 1 - (xx + zz);
      dest[6] = yz + wx;
      dest[7] = 0;
  
      dest[8] = xz + wy;
      dest[9] = yz - wx;
      dest[10] = 1 - (xx + yy);
      dest[11] = 0;
  
      dest[12] = 0;
      dest[13] = 0;
      dest[14] = 0;
      dest[15] = 1;
  
      return dest;
  };
  
  /**
   * Performs a spherical linear interpolation between two quat4
   *
   * @param {quat4} quat First quaternion
   * @param {quat4} quat2 Second quaternion
   * @param {number} slerp Interpolation amount between the two inputs
   * @param {quat4} [dest] quat4 receiving operation result. If not specified result is written to quat
   *
   * @returns {quat4} dest if specified, quat otherwise
   */
  quat4.slerp = function (quat, quat2, slerp, dest) {
      if (!dest) { dest = quat; }
  
      var cosHalfTheta = quat[0] * quat2[0] + quat[1] * quat2[1] + quat[2] * quat2[2] + quat[3] * quat2[3],
          halfTheta,
          sinHalfTheta,
          ratioA,
          ratioB;
  
      if (Math.abs(cosHalfTheta) >= 1.0) {
          if (dest !== quat) {
              dest[0] = quat[0];
              dest[1] = quat[1];
              dest[2] = quat[2];
              dest[3] = quat[3];
          }
          return dest;
      }
  
      halfTheta = Math.acos(cosHalfTheta);
      sinHalfTheta = Math.sqrt(1.0 - cosHalfTheta * cosHalfTheta);
  
      if (Math.abs(sinHalfTheta) < 0.001) {
          dest[0] = (quat[0] * 0.5 + quat2[0] * 0.5);
          dest[1] = (quat[1] * 0.5 + quat2[1] * 0.5);
          dest[2] = (quat[2] * 0.5 + quat2[2] * 0.5);
          dest[3] = (quat[3] * 0.5 + quat2[3] * 0.5);
          return dest;
      }
  
      ratioA = Math.sin((1 - slerp) * halfTheta) / sinHalfTheta;
      ratioB = Math.sin(slerp * halfTheta) / sinHalfTheta;
  
      dest[0] = (quat[0] * ratioA + quat2[0] * ratioB);
      dest[1] = (quat[1] * ratioA + quat2[1] * ratioB);
      dest[2] = (quat[2] * ratioA + quat2[2] * ratioB);
      dest[3] = (quat[3] * ratioA + quat2[3] * ratioB);
  
      return dest;
  };
  
  /**
   * Returns a string representation of a quaternion
   *
   * @param {quat4} quat quat4 to represent as a string
   *
   * @returns {string} String representation of quat
   */
  quat4.str = function (quat) {
      return '[' + quat[0] + ', ' + quat[1] + ', ' + quat[2] + ', ' + quat[3] + ']';
  };
  
  
  return {
    vec3: vec3,
    mat3: mat3,
    mat4: mat4,
    quat4: quat4
  };
  
})();
  
  