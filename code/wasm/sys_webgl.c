
/* generated with

find:
\s*GLE\(\s*(.*?)\s*,\s*(.*?)\s*,\s*(.*?)\s*\).*

replace:
\nEM_EXPORT($1, qgl$2, ( $3 ), 
{ return gl$2($3) });
$1 qgl$2( $3 )
{ return qgl$2( $3 ); }
\n


find:
return (gl|qgl)(.*?)\(([^\)]*?)\s*((const\s*)*(GLclampd|GLubyte|gldouble|glvoid|GLfloat|glint|glsizei|glfloat|glsizeiptr|gluint|glintptr|glenum|glboolean|void|GLclampf|GLbitfield|glchar)(\s*\*)*)\s*([^\)]*?)\)

replace:
return $1$2($3$8)

*/
#include <GL/gl.h>
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"


EM_EXPORT(void, qglAlphaFunc, ( GLenum func, GLclampf ref ), 
{ return glAlphaFunc(func,ref) });

EM_EXPORT(void, qglBindTexture, ( GLenum target, GLuint texture ), 
{ return glBindTexture(target,texture) });

EM_EXPORT(void, qglBlendFunc, ( GLenum sfactor, GLenum dfactor ), 
{ return glBlendFunc(sfactor,dfactor) });

EM_EXPORT(void, qglClearColor, ( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ), 
{ return glClearColor(red,green,blue,alpha) });

EM_EXPORT(void, qglClear, ( GLbitfield mask ), 
{ return glClear(mask) });

EM_EXPORT(void, qglClearStencil, ( GLint s ), 
{ return glClearStencil(s) });

EM_EXPORT(void, qglColor4f, ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ), 
{ return glColor4f(red,green,blue,alpha) });

EM_EXPORT(void, qglColorMask, ( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ), 
{ return glColorMask(red,green,blue,alpha) });

EM_EXPORT(void, qglColorPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glColorPointer(size,type,stride,ptr) });

EM_EXPORT(void, qglCopyTexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height) });

EM_EXPORT(void, qglCullFace, ( GLenum mode ), 
{ return glCullFace(mode) });

EM_EXPORT(void, qglDeleteTextures, ( GLsizei n, const GLuint *textures ), 
{ return glDeleteTextures(n,textures) });

EM_EXPORT(void, qglDepthFunc, ( GLenum func ), 
{ return glDepthFunc(func) });

EM_EXPORT(void, qglDepthMask, ( GLboolean flag ), 
{ return glDepthMask(flag) });

EM_EXPORT(void, qglDisableClientState, ( GLenum cap ), 
{ return glDisableClientState(cap) });

EM_EXPORT(void, qglDisable, ( GLenum cap ), 
{ return glDisable(cap) });

EM_EXPORT(void, qglDrawArrays, ( GLenum mode, GLint first, GLsizei count ), 
{ return glDrawArrays(mode,first,count) });

EM_EXPORT(void, qglDrawElements, ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ), 
{ return glDrawElements(mode,count,type,indices) });

EM_EXPORT(void, qglEnableClientState, ( GLenum cap ), 
{ return glEnableClientState(cap) });

EM_EXPORT(void, qglEnable, ( GLenum cap ), 
{ return glEnable(cap) });

EM_EXPORT(void, qglFinish, ( void ), 
{ return glFinish() });

EM_EXPORT(void, qglFlush, ( void ), 
{ return glFlush() });

EM_EXPORT(void, qglGenTextures, ( GLsizei n, GLuint *textures ), 
{ return glGenTextures(n,textures) });

EM_EXPORT(void, qglGetBooleanv, ( GLenum pname, GLboolean *params ), 
{ return glGetBooleanv(pname,params) });

EM_EXPORT(GLenum, qglGetError, ( void ), 
{ return glGetError() });

EM_EXPORT(void, qglGetIntegerv, ( GLenum pname, GLint *params ), 
{ return glGetIntegerv(pname,params) });

EM_EXPORT(const GLubyte *, qglGetString, ( GLenum name ), 
{ return glGetString(name) });

EM_EXPORT(void, qglLineWidth, ( GLfloat width ), 
{ return glLineWidth(width) });

EM_EXPORT(void, qglLoadIdentity, ( void ), 
{ return glLoadIdentity() });

EM_EXPORT(void, qglLoadMatrixf, ( const GLfloat *m ), 
{ return glLoadMatrixf(m) });

EM_EXPORT(void, qglMatrixMode, ( GLenum mode ), 
{ return glMatrixMode(mode) });

EM_EXPORT(void, qglPolygonOffset, ( GLfloat factor, GLfloat units ), 
{ return glPolygonOffset(factor,units) });

EM_EXPORT(void, qglPopMatrix, ( void ), 
{ return glPopMatrix() });

EM_EXPORT(void, qglPushMatrix, ( void ), 
{ return glPushMatrix() });

EM_EXPORT(void, qglReadPixels, ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ), 
{ return glReadPixels(x,y,width,height,format,type,pixels) });

EM_EXPORT(void, qglScissor, ( GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glScissor(x,y,width,height) });

EM_EXPORT(void, qglShadeModel, ( GLenum mode ), 
{ return glShadeModel(mode) });

EM_EXPORT(void, qglStencilFunc, ( GLenum func, GLint ref, GLuint mask ), 
{ return glStencilFunc(func,ref,mask) });

EM_EXPORT(void, qglStencilMask, ( GLuint mask ), 
{ return glStencilMask(mask) });

EM_EXPORT(void, qglStencilOp, ( GLenum fail, GLenum zfail, GLenum zpass ), 
{ return glStencilOp(fail,zfail,zpass) });

EM_EXPORT(void, qglTexCoordPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glTexCoordPointer(size,type,stride,ptr) });

EM_EXPORT(void, qglTexEnvf, ( GLenum target, GLenum pname, GLfloat param ), 
{ return glTexEnvf(target,pname,param) });

EM_EXPORT(void, qglTexImage2D, ( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTexImage2D(target,level,internalFormat,width,height,border,format,type,pixels) });

EM_EXPORT(void, qglTexParameterf, ( GLenum target, GLenum pname, GLfloat param ), 
{ return glTexParameterf(target,pname,param) });

EM_EXPORT(void, qglTexParameteri, ( GLenum target, GLenum pname, GLint param ), 
{ return glTexParameteri(target,pname,param) });

EM_EXPORT(void, qglTexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels) });

EM_EXPORT(void, qglTranslatef, ( GLfloat x, GLfloat y, GLfloat z ), 
{ return glTranslatef(x,y,z) });

EM_EXPORT(void, qglVertexPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glVertexPointer(size,type,stride,ptr) });

EM_EXPORT(void, qglViewport, ( GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glViewport(x,y,width,height) });

EM_EXPORT(void, qglArrayElement, ( GLint i ), 
{ return glArrayElement(i) });

EM_EXPORT(void, qglBegin, ( GLenum mode ), 
{ return glBegin(mode) });

EM_EXPORT(void, qglClearDepth, ( GLclampd depth ), 
{ return glClearDepth(depth) });

EM_EXPORT(void, qglClipPlane, ( GLenum plane, const GLdouble *equation ), 
{ return glClipPlane(plane,equation) });

EM_EXPORT(void, qglColor3f, ( GLfloat red, GLfloat green, GLfloat blue ), 
{ return glColor3f(red,green,blue) });

EM_EXPORT(void, qglColor4ubv, ( const GLubyte *v ), 
{ return glColor4ubv(v) });

EM_EXPORT(void, qglDepthRange, ( GLclampd near_val, GLclampd far_val ), 
{ return glDepthRange(near_val,far_val) });

EM_EXPORT(void, qglDrawBuffer, ( GLenum mode ), 
{ return glDrawBuffer(mode) });

EM_EXPORT(void, qglEnd, ( void ), 
{ return glEnd() });

EM_EXPORT(void, qglFrustum, ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ), 
{ return glFrustum(left,right,bottom,top,near_val,far_val) });

EM_EXPORT(void, qglOrtho, ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ), 
{ return glOrtho(left,right,bottom,top,near_val,far_val) });

EM_EXPORT(void, qglPolygonMode, ( GLenum face, GLenum mode ), 
{ return glPolygonMode(face,mode) });

EM_EXPORT(void, qglTexCoord2f, ( GLfloat s, GLfloat t ), 
{ return glTexCoord2f(s,t) });

EM_EXPORT(void, qglTexCoord2fv, ( const GLfloat *v ), 
{ return glTexCoord2fv(v) });

EM_EXPORT(void, qglVertex2f, ( GLfloat x, GLfloat y ), 
{ return glVertex2f(x,y) });

EM_EXPORT(void, qglVertex3f, ( GLfloat x, GLfloat y, GLfloat z ), 
{ return glVertex3f(x,y,z) });

EM_EXPORT(void, qglVertex3fv, ( const GLfloat *v ), 
{ return glVertex3fv(v) });

EM_EXPORT(void, qglClearDepthf, ( GLclampf depth ), 
{ return glClearDepthf(depth) });

EM_EXPORT(void, qglClipPlanef, ( GLenum plane, const GLfloat *equation ), 
{ return glClipPlanef(plane,equation) });

EM_EXPORT(void, qglDepthRangef, ( GLclampf near_val, GLclampf far_val ), 
{ return glDepthRangef(near_val,far_val) });

EM_EXPORT(void, qglFrustumf, ( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val ), 
{ return glFrustumf(left,right,bottom,top,near_val,far_val) });

EM_EXPORT(void, qglOrthof, ( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val ), 
{ return glOrthof(left,right,bottom,top,near_val,far_val) });

EM_EXPORT(void, qglActiveTexture, ( GLenum texture ), 
{ return glActiveTexture(texture) });

EM_EXPORT(void, qglCompressedTexImage2D, ( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data ), 
{ return glCompressedTexImage2D(target,level,internalformat,width,height,border,imageSize,data) });

EM_EXPORT(void, qglCompressedTexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data ), 
{ return glCompressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data) });

EM_EXPORT(void, qglGenQueries, ( GLsizei n, GLuint *ids ), 
{ return glGenQueries(n,ids) });

EM_EXPORT(void, qglDeleteQueries, ( GLsizei n, const GLuint *ids ), 
{ return glDeleteQueries(n,ids) });

EM_EXPORT(void, qglBeginQuery, ( GLenum target, GLuint id ), 
{ return glBeginQuery(target,id) });

EM_EXPORT(void, qglEndQuery, ( GLenum target ), 
{ return glEndQuery(target) });

EM_EXPORT(void, qglGetQueryObjectiv, ( GLuint id, GLenum pname, GLint *params ), 
{ return glGetQueryObjectiv(id,pname,params) });

EM_EXPORT(void, qglGetQueryObjectuiv, ( GLuint id, GLenum pname, GLuint *params ), 
{ return glGetQueryObjectuiv(id,pname,params) });

EM_EXPORT(void, qglBindBuffer, ( GLenum target, GLuint buffer ), 
{ return glBindBuffer(target,buffer) });

EM_EXPORT(void, qglDeleteBuffers, ( GLsizei n, const GLuint *buffers ), 
{ return glDeleteBuffers(n,buffers) });

EM_EXPORT(void, qglGenBuffers, ( GLsizei n, GLuint *buffers ), 
{ return glGenBuffers(n,buffers) });

EM_EXPORT(void, qglBufferData, ( GLenum target, GLsizeiptr size, const void *data, GLenum usage ), 
{ return glBufferData(target,size,data,usage) });

EM_EXPORT(void, qglBufferSubData, ( GLenum target, GLintptr offset, GLsizeiptr size, const void *data ), 
{ return glBufferSubData(target,offset,size,data) });

EM_EXPORT(GLboolean, qglUnmapBuffer, ( GLenum target ), 
{ return glUnmapBuffer(target) });

EM_EXPORT(void *, qglMapBufferRange, ( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ), 
{ return glMapBufferRange(target,offset,length,access) });

EM_EXPORT(void, qglReadBuffer, ( GLenum mode ), 
{ return glReadBuffer(mode) });

EM_EXPORT(void, qglDrawBuffers, ( GLsizei n, const GLenum *bufs ), 
{ return glDrawBuffers(n,bufs) });

EM_EXPORT(void, qglAttachShader, ( GLuint program, GLuint shader ), 
{ return glAttachShader(program,shader) });

EM_EXPORT(void, qglBindAttribLocation, ( GLuint program, GLuint index, const GLchar *name ), 
{ return glBindAttribLocation(program,index,name) });

EM_EXPORT(void, qglCompileShader, ( GLuint shader ), 
{ return glCompileShader(shader) });

EM_EXPORT(GLuint, qglCreateProgram, ( void ), 
{ return glCreateProgram() });

EM_EXPORT(GLuint, qglCreateShader, ( GLenum type ), 
{ return glCreateShader(type) });

EM_EXPORT(void, qglDeleteProgram, ( GLuint program ), 
{ return glDeleteProgram(program) });

EM_EXPORT(void, qglDeleteShader, ( GLuint shader ), 
{ return glDeleteShader(shader) });

EM_EXPORT(void, qglDetachShader, ( GLuint program, GLuint shader ), 
{ return glDetachShader(program,shader) });

EM_EXPORT(void, qglDisableVertexAttribArray, ( GLuint index ), 
{ return glDisableVertexAttribArray(index) });

EM_EXPORT(void, qglEnableVertexAttribArray, ( GLuint index ), 
{ return glEnableVertexAttribArray(index) });

EM_EXPORT(void, qglGetActiveUniform, ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ), 
{ return glGetActiveUniform(program,index,bufSize,length,size,type,name) });

EM_EXPORT(void, qglGetProgramiv, ( GLuint program, GLenum pname, GLint *params ), 
{ return glGetProgramiv(program,pname,params) });

EM_EXPORT(void, qglGetProgramInfoLog, ( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog ), 
{ return glGetProgramInfoLog(program,bufSize,length,infoLog) });

EM_EXPORT(void, qglGetShaderiv, ( GLuint shader, GLenum pname, GLint *params ), 
{ return glGetShaderiv(shader,pname,params) });

EM_EXPORT(void, qglGetShaderInfoLog, ( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog ), 
{ return glGetShaderInfoLog(shader,bufSize,length,infoLog) });

EM_EXPORT(void, qglGetShaderSource, ( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source ), 
{ return glGetShaderSource(shader,bufSize,length,source) });

EM_EXPORT(GLint, qglGetUniformLocation, ( GLuint program, const GLchar *name ), 
{ return glGetUniformLocation(program,name) });

EM_EXPORT(void, qglLinkProgram, ( GLuint program ), 
{ return glLinkProgram(program) });

EM_EXPORT(void, qglShaderSource, ( GLuint shader, GLsizei count, const GLchar* *string, const GLint *length ), 
{ return glShaderSource(shader,count,string,length) });

EM_EXPORT(void, qglUseProgram, ( GLuint program ), 
{ return glUseProgram(program) });

EM_EXPORT(void, qglUniform1f, ( GLint location, GLfloat v0 ), 
{ return glUniform1f(location,v0) });

EM_EXPORT(void, qglUniform2f, ( GLint location, GLfloat v0, GLfloat v1 ), 
{ return glUniform2f(location,v0,v1) });

EM_EXPORT(void, qglUniform3f, ( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ), 
{ return glUniform3f(location,v0,v1,v2) });

EM_EXPORT(void, qglUniform4f, ( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ), 
{ return glUniform4f(location,v0,v1,v2,v3) });

EM_EXPORT(void, qglUniform1i, ( GLint location, GLint v0 ), 
{ return glUniform1i(location,v0) });

EM_EXPORT(void, qglUniform1fv, ( GLint location, GLsizei count, const GLfloat *value ), 
{ return glUniform1fv(location,count,value) });

EM_EXPORT(void, qglUniformMatrix4fv, ( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ), 
{ return glUniformMatrix4fv(location,count,transpose,value) });

EM_EXPORT(void, qglValidateProgram, ( GLuint program ), 
{ return glValidateProgram(program) });

EM_EXPORT(void, qglVertexAttribPointer, ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer ), 
{ return glVertexAttribPointer(index,size,type,normalized,stride,pointer) });

EM_EXPORT(const GLubyte *, qglGetStringi, ( GLenum name, GLuint index ), 
{ return glGetStringi(name,index) });

EM_EXPORT(void, qglBindRenderbuffer, ( GLenum target, GLuint renderbuffer ), 
{ return glBindRenderbuffer(target,renderbuffer) });

EM_EXPORT(void, qglDeleteRenderbuffers, ( GLsizei n, const GLuint *renderbuffers ), 
{ return glDeleteRenderbuffers(n,renderbuffers) });

EM_EXPORT(void, qglGenRenderbuffers, ( GLsizei n, GLuint *renderbuffers ), 
{ return glGenRenderbuffers(n,renderbuffers) });

EM_EXPORT(void, qglRenderbufferStorage, ( GLenum target, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glRenderbufferStorage(target,internalformat,width,height) });

EM_EXPORT(void, qglBindFramebuffer, ( GLenum target, GLuint framebuffer ), 
{ return glBindFramebuffer(target,framebuffer) });

EM_EXPORT(void, qglDeleteFramebuffers, ( GLsizei n, const GLuint *framebuffers ), 
{ return glDeleteFramebuffers(n,framebuffers) });

EM_EXPORT(void, qglGenFramebuffers, ( GLsizei n, GLuint *framebuffers ), 
{ return glGenFramebuffers(n,framebuffers) });

EM_EXPORT(GLenum, qglCheckFramebufferStatus, ( GLenum target ), 
{ return glCheckFramebufferStatus(target) });

EM_EXPORT(void, qglFramebufferTexture2D, ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ), 
{ return glFramebufferTexture2D(target,attachment,textarget,texture,level) });

EM_EXPORT(void, qglFramebufferRenderbuffer, ( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ), 
{ return glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer) });

EM_EXPORT(void, qglGenerateMipmap, ( GLenum target ), 
{ return glGenerateMipmap(target) });

EM_EXPORT(void, qglBlitFramebuffer, ( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ), 
{ return glBlitFramebuffer(srcX0,srcY0,srcX1,srcY1,dstX0,dstY0,dstX1,dstY1,mask,filter) });

EM_EXPORT(void, qglRenderbufferStorageMultisample, ( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glRenderbufferStorageMultisample(target,samples,internalformat,width,height) });

EM_EXPORT(void, qglBindVertexArray, ( GLuint array ), 
{ return glBindVertexArray(array) });

EM_EXPORT(void, qglDeleteVertexArrays, ( GLsizei n, const GLuint *arrays ), 
{ return glDeleteVertexArrays(n,arrays) });

EM_EXPORT(void, qglGenVertexArrays, ( GLsizei n, GLuint *arrays ), 
{ return glGenVertexArrays(n,arrays) });

EM_EXPORT(GLvoid, qglBindMultiTextureEXT, ( GLenum texunit, GLenum target, GLuint texture ), 
{ return glBindMultiTextureEXT(texunit,target,texture) });

EM_EXPORT(GLvoid, qglTextureParameterfEXT, ( GLuint texture, GLenum target, GLenum pname, GLfloat param ), 
{ return glTextureParameterfEXT(texture,target,pname,param) });

EM_EXPORT(GLvoid, qglTextureParameteriEXT, ( GLuint texture, GLenum target, GLenum pname, GLint param ), 
{ return glTextureParameteriEXT(texture,target,pname,param) });

EM_EXPORT(GLvoid, qglTextureImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTextureImage2DEXT(texture,target,level,internalformat,width,height,border,format,type,pixels) });

EM_EXPORT(GLvoid, qglTextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,type,pixels) });

EM_EXPORT(GLvoid, qglCopyTextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glCopyTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,x,y,width,height) });

EM_EXPORT(GLvoid, qglCompressedTextureImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data ), 
{ return glCompressedTextureImage2DEXT(texture,target,level,internalformat,width,height,border,imageSize,data) });

EM_EXPORT(GLvoid, qglCompressedTextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data ), 
{ return glCompressedTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,imageSize,data) });

EM_EXPORT(GLvoid, qglGenerateTextureMipmapEXT, ( GLuint texture, GLenum target ), 
{ return glGenerateTextureMipmapEXT(texture,target) });

EM_EXPORT(GLvoid, qglProgramUniform1iEXT, ( GLuint program, GLint location, GLint v0 ), 
{ return glProgramUniform1iEXT(program,location,v0) });

EM_EXPORT(GLvoid, qglProgramUniform1fEXT, ( GLuint program, GLint location, GLfloat v0 ), 
{ return glProgramUniform1fEXT(program,location,v0) });

EM_EXPORT(GLvoid, qglProgramUniform2fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1 ), 
{ return glProgramUniform2fEXT(program,location,v0,v1) });

EM_EXPORT(GLvoid, qglProgramUniform3fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ), 
{ return glProgramUniform3fEXT(program,location,v0,v1,v2) });

EM_EXPORT(GLvoid, qglProgramUniform4fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ), 
{ return glProgramUniform4fEXT(program,location,v0,v1,v2,v3) });

EM_EXPORT(GLvoid, qglProgramUniform1fvEXT, ( GLuint program, GLint location, GLsizei count, const GLfloat *value ), 
{ return glProgramUniform1fvEXT(program,location,count,value) });

EM_EXPORT(GLvoid, qglProgramUniformMatrix4fvEXT, ( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ), 
{ return glProgramUniformMatrix4fvEXT(program,location,count,transpose,value) });

EM_EXPORT(GLvoid, qglNamedRenderbufferStorageEXT, ( GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glNamedRenderbufferStorageEXT(renderbuffer,internalformat,width,height) });

EM_EXPORT(GLvoid, qglNamedRenderbufferStorageMultisampleEXT, ( GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glNamedRenderbufferStorageMultisampleEXT(renderbuffer,samples,internalformat,width,height) });

EM_EXPORT(GLenum, qglCheckNamedFramebufferStatusEXT, ( GLuint framebuffer, GLenum target ), 
{ return glCheckNamedFramebufferStatusEXT(framebuffer,target) });

EM_EXPORT(GLvoid, qglNamedFramebufferTexture2DEXT, ( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level ), 
{ return glNamedFramebufferTexture2DEXT(framebuffer,attachment,textarget,texture,level) });

EM_EXPORT(GLvoid, qglNamedFramebufferRenderbufferEXT, ( GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ), 
{ return glNamedFramebufferRenderbufferEXT(framebuffer,attachment,renderbuffertarget,renderbuffer) });

EM_EXPORT(GLvoid, qglBindProgramARB, ( GLenum target, GLuint program ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglProgramLocalParameter4fARB, ( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglProgramLocalParameter4fvARB, ( GLenum target, GLuint index, const GLfloat *params ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglNormalPointer, ( GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglLockArraysEXT, ( GLint first, GLsizei count ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglUnlockArraysEXT, ( void ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglProgramStringARB, ( GLenum target, GLenum format, GLsizei len, const void *string ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglDeleteProgramsARB, ( GLsizei n, const GLuint *programs ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglGenProgramsARB, ( GLsizei n, GLuint *programs ), 
{ return assert(program == 0) });

EM_EXPORT(GLvoid, qglGetInternalformativ, ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params ), 
{ return assert(program == 0) });

GLvoid qglBindBufferARB( GLenum target, GLuint buffer )
{ return qglBindBuffer(target, buffer); }

GLvoid qglDeleteBuffersARB( GLsizei n, const GLuint *buffers )
{ return qglDeleteBuffers(n, buffers); }

GLvoid qglGenBuffersARB( GLsizei n, GLuint *buffers )
{ return qglGenBuffers(n, buffers); }

GLvoid qglActiveTextureARB( GLenum texture )
{ return qglActiveTexture(texture ); }

EM_EXPORT(GLvoid, qglTexEnvi, ( GLenum target, GLenum pname, GLint param ), 
{ return glTexEnvi(target,pname,param) });

EM_EXPORT(GLvoid, qglClientActiveTextureARB, ( GLenum texture ), 
{ return glClientActiveTexture(texture) });

EM_EXPORT(GLvoid, qglMultiTexCoord2fARB, ( GLenum target, const GLfloat *v ), 
{ return glMultiTexCoord2f(target,v) });

EM_EXPORT(GLvoid, qglBufferDataARB, ( GLenum target, GLsizeiptrARB size, const void *data, GLenum usage  ), 
{ return glBufferDataARB(target, pname, params) });

EM_EXPORT(GLvoid, qglGetRenderbufferParameteriv, ( GLenum target, GLenum pname, GLint *params ), 
{ return glGetRenderbufferParameter(target, pname, params) });

EM_EXPORT(GLvoid, qglIsFramebuffer, ( GLuint framebuffer ), 
{ return glIsFramebuffer(framebuffer) });

EM_EXPORT(GLvoid, qglGetFramebufferAttachmentParameteriv, ( GLenum target, GLenum attachment, GLenum pname, GLint *params ), 
{ return glGetFramebufferAttachmentParameter(target, attachment, pname, params) });
