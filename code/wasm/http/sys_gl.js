
var GLEmulation



GLEmulation = {
  textures: [],
  // in non-dll mode, this just returns the exported function returned from importing
  GL_GetProcAddress: function (fn) {
    // TODO: renderer1/renderer2
  },
  glDisable: function () {},
  glEnable: function () {},
  glBindProgramARB: function () { },
  glProgramLocalParameter4fARB: function () {},
  glProgramLocalParameter4fvARB: function () {},
  glPolygonOffset: function () {},
  glTexCoordPointer: function (size, type, stride, pointer) {
  },
  glNormalPointer: function (type, stride, pointer) {
  },
  glVertexPointer: function (size, type, stride, pointer) {
  },
  glColorPointer: function (size, type, stride, pointer) {
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
  glViewport: function (x, y, w, h) {
    Q3e.webgl.viewport(x, y, w, h);
  },
  glScissor: function () {},
  glMatrixMode: function () {},
  glLoadMatrixf: function () {},
  glLoadIdentity: function () {},
  glColor4f: function () {},
  glDrawArrays: function () {
    //debugger
  },
  glDrawBuffer: function () { /* do nothing */ },
  glClearColor: function (r, g, b, a) {
    Q3e.webgl.clearColor(r, g, b, a);
  },
  glClear: function () {},
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
    //Q3e.webgl.texImage2D(target, level, internalFormat, width, height, border, format, type, null) 
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
    
  },
  glDisableClientState: function(cap) {
    
  },
  glClientActiveTextureARB: function () {},
  glTexSubImage2D: function (a1, a2, a3, a4, a5, a6, a7) { 
    //Q3e.webgl.texSubImage2D(a1, a2, a3, a4, a5, a6, a7) 
  },
  glFinish: function () {
    debugger
  },
  glDepthRange: function (range) { 
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
    Q3e.webgl.bufferData(Q3e.webgl.ARRAY_BUFFER, new Float32Array([
       x1, y1,
       x2, y1,
       x1, y2,
       x2, y2,
    ]), gl.STATIC_DRAW);
  },

  glDrawElements: function (mode, count, type, indices2, start, end) {
    // setup GLSL program
    if(typeof GLEmulation.shaderProgram == 'undefined') {
      buildShaderProgram()
    }

    // look up where the vertex data needs to go.
    var positionAttributeLocation = Q3e.webgl.getAttribLocation(GLEmulation.shaderProgram, "a_position");
  
    // look up uniform locations
    var resolutionUniformLocation = Q3e.webgl.getUniformLocation(GLEmulation.shaderProgram, "u_resolution");
    var colorUniformLocation = Q3e.webgl.getUniformLocation(GLEmulation.shaderProgram, "u_color");
  
    // Create a buffer to put three 2d clip space points in
    var positionBuffer = Q3e.webgl.createBuffer();
  
    // Bind it to ARRAY_BUFFER (think of it as ARRAY_BUFFER = positionBuffer)
    Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, positionBuffer);
  
    //webglUtils.resizeCanvasToDisplaySize(Q3e.webgl.canvas);
  
    // Tell WebGL how to convert from clip space to pixels
  
    // Clear the canvas
    Q3e.webgl.clearColor(0, 0, 0, 0);
    Q3e.webgl.clear(Q3e.webgl.COLOR_BUFFER_BIT);
  
    // Tell it to use our program (pair of shaders)
    Q3e.webgl.useProgram(GLEmulation.shaderProgram);
  
    // Bind the position buffer.
    Q3e.webgl.bindBuffer(Q3e.webgl.ARRAY_BUFFER, positionBuffer);
  
    // create the buffer
    const indexBuffer = Q3e.webgl.createBuffer();
  
    // make this buffer the current 'ELEMENT_ARRAY_BUFFER'
    Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, indexBuffer);
  
    // Fill the current element array buffer with data
    const indices = [
      0, 1, 2,   // first triangle
      2, 1, 3,   // second triangle
    ];
    Q3e.webgl.bufferData(
      Q3e.webgl.ELEMENT_ARRAY_BUFFER,
      new Uint16Array(indices),
      Q3e.webgl.STATIC_DRAW
    );
  
    // code above this line is initialization code
    // --------------------------------
    // code below this line is rendering code
  
    // Turn on the attribute
    Q3e.webgl.enableVertexAttribArray(positionAttributeLocation);
  
    // Tell the attribute how to get data out of positionBuffer (ARRAY_BUFFER)
    var size = 2;          // 2 components per iteration
    var type = Q3e.webgl.FLOAT;   // the data is 32bit floats
    var normalize = false; // don't normalize the data
    var stride = 0;        // 0 = move forward size * sizeof(type) each iteration to get the next position
    var offset = 0;        // start at the beginning of the buffer
    Q3e.webgl.vertexAttribPointer(
        positionAttributeLocation, size, type, normalize, stride, offset);
  
    // bind the buffer containing the indices
    Q3e.webgl.bindBuffer(Q3e.webgl.ELEMENT_ARRAY_BUFFER, indexBuffer);
  
    // set the resolution
    Q3e.webgl.uniform2f(resolutionUniformLocation, Q3e.canvas.width, Q3e.canvas.height);
  
    // draw 50 random rectangles in random colors
    for (var ii = 0; ii < 50; ++ii) {
      // Setup a random rectangle
      // This will write to positionBuffer because
      // its the last thing we bound on the ARRAY_BUFFER
      // bind point
      GLEmulation.setRectangle(
        Q3e.webgl, GLEmulation.randomInt(300), GLEmulation.randomInt(300), GLEmulation.randomInt(300), GLEmulation.randomInt(300));
  
      // Set a random color.
      Q3e.webgl.uniform4f(colorUniformLocation, Math.random(), Math.random(), Math.random(), 1);
  
      // Draw the rectangle.
      var primitiveType = Q3e.webgl.TRIANGLES;
      var offset = 0;
      var count = 6;
      var indexType = Q3e.webgl.UNSIGNED_SHORT;
      Q3e.webgl.drawElements(primitiveType, count, indexType, offset);
    }

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
    } else if (target == 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/) {
      Q3e.webgl.currentElementArrayBufferBinding = buffer;
    }

  },
  glBufferDataARB: function () {},
  glRenderbufferStorage: function () {},
  glGetRenderbufferParameteriv: function () {},
  glIsFramebuffer: function () {},
  glGetFramebufferAttachmentParameteriv: function () {},

}


function buildShaderProgram() {
  let program = Q3e.webgl.createProgram();

  let vertexShader = Q3e.webgl.createShader(Q3e.webgl.VERTEX_SHADER);
  Q3e.webgl.shaderSource(vertexShader, `
  attribute vec2 a_position;
 
  uniform vec2 u_resolution;
 
  void main() {

    vec2 zeroToOne = a_position / u_resolution;
    vec2 zeroToTwo = zeroToOne * 2.0;
    vec2 clipSpace = zeroToTwo - 1.0;
    gl_Position = vec4(clipSpace, 0, 1);

  }`);
  Q3e.webgl.compileShader(vertexShader);
  if (!Q3e.webgl.getShaderParameter(vertexShader, Q3e.webgl.COMPILE_STATUS)) {
    console.log('Error compiling fragment shader:');
    console.log(Q3e.webgl.getShaderInfoLog(vertexShader));
  } else {
    Q3e.webgl.attachShader(program, vertexShader);
  }

  let fragmentShader = Q3e.webgl.createShader(Q3e.webgl.FRAGMENT_SHADER);
  Q3e.webgl.shaderSource(fragmentShader, `
  precision mediump float;
 
  void main() {

    gl_FragColor = vec4(1, 0, 1, 1); // return reddish-purple

  }`);
  Q3e.webgl.compileShader(fragmentShader);
  if (!Q3e.webgl.getShaderParameter(fragmentShader, Q3e.webgl.COMPILE_STATUS)) {
    console.log('Error compiling fragment shader:');
    console.log(Q3e.webgl.getShaderInfoLog(fragmentShader));
  } else {
    Q3e.webgl.attachShader(program, fragmentShader);
  }

  Q3e.webgl.linkProgram(program)

  if (!Q3e.webgl.getProgramParameter(program, Q3e.webgl.LINK_STATUS)) {
    console.log("Error linking shader program:");
    console.log(Q3e.webgl.getProgramInfoLog(program));
  }

  GLEmulation.shaderProgram = program
  return program;
}



