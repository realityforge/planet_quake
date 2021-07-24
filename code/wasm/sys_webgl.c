
/* generated with

find:
\s*GLE\(\s*(.*?)\s*,\s*(.*?)\s*,\s*(.*?)\s*\).*

replace:
\nEM_JS($1, SYS_$2, ( $3 ), 
{ return gl$2($3) });
$1 qgl$2( $3 )
{ return SYS_$2( $3 ); }
\n


find:
return (gl|SYS_)(.*?)\(([^\)]*?)\s*((const\s*)*(GLclampd|GLubyte|gldouble|glvoid|GLfloat|glint|glsizei|glfloat|glsizeiptr|gluint|glintptr|glenum|glboolean|void|GLclampf|GLbitfield|glchar)(\s*\*)*)\s*([^\)]*?)\)

replace:
return $1$2($3$8)

*/
#include <GL/gl.h>
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"


EM_JS(void, SYS_AlphaFunc, ( GLenum func, GLclampf ref ), 
{ return glAlphaFunc(func,ref) });
void qglAlphaFunc( GLenum func, GLclampf ref )
{ return SYS_AlphaFunc(func,ref ); }


EM_JS(void, SYS_BindTexture, ( GLenum target, GLuint texture ), 
{ return glBindTexture(target,texture) });
void qglBindTexture( GLenum target, GLuint texture )
{ return SYS_BindTexture(target,texture ); }


EM_JS(void, SYS_BlendFunc, ( GLenum sfactor, GLenum dfactor ), 
{ return glBlendFunc(sfactor,dfactor) });
void qglBlendFunc( GLenum sfactor, GLenum dfactor )
{ return SYS_BlendFunc(sfactor,dfactor ); }


EM_JS(void, SYS_ClearColor, ( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ), 
{ return glClearColor(red,green,blue,alpha) });
void qglClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{ return SYS_ClearColor(red,green,blue,alpha ); }


EM_JS(void, SYS_Clear, ( GLbitfield mask ), 
{ return glClear(mask) });
void qglClear( GLbitfield mask )
{ return SYS_Clear(mask ); }


EM_JS(void, SYS_ClearStencil, ( GLint s ), 
{ return glClearStencil(s) });
void qglClearStencil( GLint s )
{ return SYS_ClearStencil(s ); }


EM_JS(void, SYS_Color4f, ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ), 
{ return glColor4f(red,green,blue,alpha) });
void qglColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{ return SYS_Color4f(red,green,blue,alpha ); }


EM_JS(void, SYS_ColorMask, ( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ), 
{ return glColorMask(red,green,blue,alpha) });
void qglColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{ return SYS_ColorMask(red,green,blue,alpha ); }


EM_JS(void, SYS_ColorPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glColorPointer(size,type,stride,ptr) });
void qglColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{ return SYS_ColorPointer(size,type,stride,ptr ); }


EM_JS(void, SYS_CopyTexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height) });
void qglCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{ return SYS_CopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height ); }


EM_JS(void, SYS_CullFace, ( GLenum mode ), 
{ return glCullFace(mode) });
void qglCullFace( GLenum mode )
{ return SYS_CullFace(mode ); }


EM_JS(void, SYS_DeleteTextures, ( GLsizei n, const GLuint *textures ), 
{ return glDeleteTextures(n,textures) });
void qglDeleteTextures( GLsizei n, const GLuint *textures )
{ return SYS_DeleteTextures(n,textures ); }


EM_JS(void, SYS_DepthFunc, ( GLenum func ), 
{ return glDepthFunc(func) });
void qglDepthFunc( GLenum func )
{ return SYS_DepthFunc(func ); }


EM_JS(void, SYS_DepthMask, ( GLboolean flag ), 
{ return glDepthMask(flag) });
void qglDepthMask( GLboolean flag )
{ return SYS_DepthMask(flag ); }


EM_JS(void, SYS_DisableClientState, ( GLenum cap ), 
{ return glDisableClientState(cap) });
void qglDisableClientState( GLenum cap )
{ return SYS_DisableClientState(cap ); }


EM_JS(void, SYS_Disable, ( GLenum cap ), 
{ return glDisable(cap) });
void qglDisable( GLenum cap )
{ return SYS_Disable(cap ); }


EM_JS(void, SYS_DrawArrays, ( GLenum mode, GLint first, GLsizei count ), 
{ return glDrawArrays(mode,first,count) });
void qglDrawArrays( GLenum mode, GLint first, GLsizei count )
{ return SYS_DrawArrays(mode,first,count ); }


EM_JS(void, SYS_DrawElements, ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ), 
{ return glDrawElements(mode,count,type,indices) });
void qglDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{ return SYS_DrawElements(mode,count,type,indices ); }


EM_JS(void, SYS_EnableClientState, ( GLenum cap ), 
{ return glEnableClientState(cap) });
void qglEnableClientState( GLenum cap )
{ return SYS_EnableClientState(cap ); }


EM_JS(void, SYS_Enable, ( GLenum cap ), 
{ return glEnable(cap) });
void qglEnable( GLenum cap )
{ return SYS_Enable(cap ); }


EM_JS(void, SYS_Finish, ( void ), 
{ return glFinish() });
void qglFinish( void )
{ return SYS_Finish(); }


EM_JS(void, SYS_Flush, ( void ), 
{ return glFlush() });
void qglFlush( void )
{ return SYS_Flush(); }


EM_JS(void, SYS_GenTextures, ( GLsizei n, GLuint *textures ), 
{ return glGenTextures(n,textures) });
void qglGenTextures( GLsizei n, GLuint *textures )
{ return SYS_GenTextures(n,textures ); }


EM_JS(void, SYS_GetBooleanv, ( GLenum pname, GLboolean *params ), 
{ return glGetBooleanv(pname,params) });
void qglGetBooleanv( GLenum pname, GLboolean *params )
{ return SYS_GetBooleanv(pname,params ); }


EM_JS(GLenum, SYS_GetError, ( void ), 
{ return glGetError() });
GLenum qglGetError( void )
{ return SYS_GetError(); }


EM_JS(void, SYS_GetIntegerv, ( GLenum pname, GLint *params ), 
{ return glGetIntegerv(pname,params) });
void qglGetIntegerv( GLenum pname, GLint *params )
{ return SYS_GetIntegerv(pname,params ); }


EM_JS(const GLubyte *, SYS_GetString, ( GLenum name ), 
{ return glGetString(name) });
const GLubyte * qglGetString( GLenum name )
{ return SYS_GetString(name ); }


EM_JS(void, SYS_LineWidth, ( GLfloat width ), 
{ return glLineWidth(width) });
void qglLineWidth( GLfloat width )
{ return SYS_LineWidth(width ); }


EM_JS(void, SYS_LoadIdentity, ( void ), 
{ return glLoadIdentity() });
void qglLoadIdentity( void )
{ return SYS_LoadIdentity(); }


EM_JS(void, SYS_LoadMatrixf, ( const GLfloat *m ), 
{ return glLoadMatrixf(m) });
void qglLoadMatrixf( const GLfloat *m )
{ return SYS_LoadMatrixf(m ); }


EM_JS(void, SYS_MatrixMode, ( GLenum mode ), 
{ return glMatrixMode(mode) });
void qglMatrixMode( GLenum mode )
{ return SYS_MatrixMode(mode ); }


EM_JS(void, SYS_PolygonOffset, ( GLfloat factor, GLfloat units ), 
{ return glPolygonOffset(factor,units) });
void qglPolygonOffset( GLfloat factor, GLfloat units )
{ return SYS_PolygonOffset(factor,units ); }


EM_JS(void, SYS_PopMatrix, ( void ), 
{ return glPopMatrix() });
void qglPopMatrix( void )
{ return SYS_PopMatrix(); }


EM_JS(void, SYS_PushMatrix, ( void ), 
{ return glPushMatrix() });
void qglPushMatrix( void )
{ return SYS_PushMatrix(); }


EM_JS(void, SYS_ReadPixels, ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ), 
{ return glReadPixels(x,y,width,height,format,type,pixels) });
void qglReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{ return SYS_ReadPixels(x,y,width,height,format,type,pixels ); }


EM_JS(void, SYS_Scissor, ( GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glScissor(x,y,width,height) });
void qglScissor( GLint x, GLint y, GLsizei width, GLsizei height )
{ return SYS_Scissor(x,y,width,height ); }


EM_JS(void, SYS_ShadeModel, ( GLenum mode ), 
{ return glShadeModel(mode) });
void qglShadeModel( GLenum mode )
{ return SYS_ShadeModel(mode ); }


EM_JS(void, SYS_StencilFunc, ( GLenum func, GLint ref, GLuint mask ), 
{ return glStencilFunc(func,ref,mask) });
void qglStencilFunc( GLenum func, GLint ref, GLuint mask )
{ return SYS_StencilFunc(func,ref,mask ); }


EM_JS(void, SYS_StencilMask, ( GLuint mask ), 
{ return glStencilMask(mask) });
void qglStencilMask( GLuint mask )
{ return SYS_StencilMask(mask ); }


EM_JS(void, SYS_StencilOp, ( GLenum fail, GLenum zfail, GLenum zpass ), 
{ return glStencilOp(fail,zfail,zpass) });
void qglStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{ return SYS_StencilOp(fail,zfail,zpass ); }


EM_JS(void, SYS_TexCoordPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glTexCoordPointer(size,type,stride,ptr) });
void qglTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{ return SYS_TexCoordPointer(size,type,stride,ptr ); }


EM_JS(void, SYS_TexEnvf, ( GLenum target, GLenum pname, GLfloat param ), 
{ return glTexEnvf(target,pname,param) });
void qglTexEnvf( GLenum target, GLenum pname, GLfloat param )
{ return SYS_TexEnvf(target,pname,param ); }


EM_JS(void, SYS_TexImage2D, ( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTexImage2D(target,level,internalFormat,width,height,border,format,type,pixels) });
void qglTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{ return SYS_TexImage2D(target,level,internalFormat,width,height,border,format,type,pixels ); }


EM_JS(void, SYS_TexParameterf, ( GLenum target, GLenum pname, GLfloat param ), 
{ return glTexParameterf(target,pname,param) });
void qglTexParameterf( GLenum target, GLenum pname, GLfloat param )
{ return SYS_TexParameterf(target,pname,param ); }


EM_JS(void, SYS_TexParameteri, ( GLenum target, GLenum pname, GLint param ), 
{ return glTexParameteri(target,pname,param) });
void qglTexParameteri( GLenum target, GLenum pname, GLint param )
{ return SYS_TexParameteri(target,pname,param ); }


EM_JS(void, SYS_TexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels) });
void qglTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{ return SYS_TexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels ); }


EM_JS(void, SYS_Translatef, ( GLfloat x, GLfloat y, GLfloat z ), 
{ return glTranslatef(x,y,z) });
void qglTranslatef( GLfloat x, GLfloat y, GLfloat z )
{ return SYS_Translatef(x,y,z ); }


EM_JS(void, SYS_VertexPointer, ( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ), 
{ return glVertexPointer(size,type,stride,ptr) });
void qglVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{ return SYS_VertexPointer(size,type,stride,ptr ); }


EM_JS(void, SYS_Viewport, ( GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glViewport(x,y,width,height) });
void qglViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{ return SYS_Viewport(x,y,width,height ); }


EM_JS(void, SYS_ArrayElement, ( GLint i ), 
{ return glArrayElement(i) });
void qglArrayElement( GLint i )
{ return SYS_ArrayElement(i ); }


EM_JS(void, SYS_Begin, ( GLenum mode ), 
{ return glBegin(mode) });
void qglBegin( GLenum mode )
{ return SYS_Begin(mode ); }


EM_JS(void, SYS_ClearDepth, ( GLclampd depth ), 
{ return glClearDepth(depth) });
void qglClearDepth( GLclampd depth )
{ return SYS_ClearDepth(depth ); }


EM_JS(void, SYS_ClipPlane, ( GLenum plane, const GLdouble *equation ), 
{ return glClipPlane(plane,equation) });
void qglClipPlane( GLenum plane, const GLdouble *equation )
{ return SYS_ClipPlane(plane,equation ); }


EM_JS(void, SYS_Color3f, ( GLfloat red, GLfloat green, GLfloat blue ), 
{ return glColor3f(red,green,blue) });
void qglColor3f( GLfloat red, GLfloat green, GLfloat blue )
{ return SYS_Color3f(red,green,blue ); }


EM_JS(void, SYS_Color4ubv, ( const GLubyte *v ), 
{ return glColor4ubv(v) });
void qglColor4ubv( const GLubyte *v )
{ return SYS_Color4ubv(v ); }


EM_JS(void, SYS_DepthRange, ( GLclampd near_val, GLclampd far_val ), 
{ return glDepthRange(near_val,far_val) });
void qglDepthRange( GLclampd near_val, GLclampd far_val )
{ return SYS_DepthRange(near_val,far_val ); }


EM_JS(void, SYS_DrawBuffer, ( GLenum mode ), 
{ return glDrawBuffer(mode) });
void qglDrawBuffer( GLenum mode )
{ return SYS_DrawBuffer(mode ); }


EM_JS(void, SYS_End, ( void ), 
{ return glEnd() });
void qglEnd( void )
{ return SYS_End(); }


EM_JS(void, SYS_Frustum, ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ), 
{ return glFrustum(left,right,bottom,top,near_val,far_val) });
void qglFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val )
{ return SYS_Frustum(left,right,bottom,top,near_val,far_val ); }


EM_JS(void, SYS_Ortho, ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ), 
{ return glOrtho(left,right,bottom,top,near_val,far_val) });
void qglOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val )
{ return SYS_Ortho(left,right,bottom,top,near_val,far_val ); }


EM_JS(void, SYS_PolygonMode, ( GLenum face, GLenum mode ), 
{ return glPolygonMode(face,mode) });
void qglPolygonMode( GLenum face, GLenum mode )
{ return SYS_PolygonMode(face,mode ); }


EM_JS(void, SYS_TexCoord2f, ( GLfloat s, GLfloat t ), 
{ return glTexCoord2f(s,t) });
void qglTexCoord2f( GLfloat s, GLfloat t )
{ return SYS_TexCoord2f(s,t ); }


EM_JS(void, SYS_TexCoord2fv, ( const GLfloat *v ), 
{ return glTexCoord2fv(v) });
void qglTexCoord2fv( const GLfloat *v )
{ return SYS_TexCoord2fv(v ); }


EM_JS(void, SYS_Vertex2f, ( GLfloat x, GLfloat y ), 
{ return glVertex2f(x,y) });
void qglVertex2f( GLfloat x, GLfloat y )
{ return SYS_Vertex2f(x,y ); }


EM_JS(void, SYS_Vertex3f, ( GLfloat x, GLfloat y, GLfloat z ), 
{ return glVertex3f(x,y,z) });
void qglVertex3f( GLfloat x, GLfloat y, GLfloat z )
{ return SYS_Vertex3f(x,y,z ); }


EM_JS(void, SYS_Vertex3fv, ( const GLfloat *v ), 
{ return glVertex3fv(v) });
void qglVertex3fv( const GLfloat *v )
{ return SYS_Vertex3fv(v ); }


EM_JS(void, SYS_ClearDepthf, ( GLclampf depth ), 
{ return glClearDepthf(depth) });
void qglClearDepthf( GLclampf depth )
{ return SYS_ClearDepthf(depth ); }


EM_JS(void, SYS_ClipPlanef, ( GLenum plane, const GLfloat *equation ), 
{ return glClipPlanef(plane,equation) });
void qglClipPlanef( GLenum plane, const GLfloat *equation )
{ return SYS_ClipPlanef(plane,equation ); }


EM_JS(void, SYS_DepthRangef, ( GLclampf near_val, GLclampf far_val ), 
{ return glDepthRangef(near_val,far_val) });
void qglDepthRangef( GLclampf near_val, GLclampf far_val )
{ return SYS_DepthRangef(near_val,far_val ); }


EM_JS(void, SYS_Frustumf, ( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val ), 
{ return glFrustumf(left,right,bottom,top,near_val,far_val) });
void qglFrustumf( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val )
{ return SYS_Frustumf(left,right,bottom,top,near_val,far_val ); }


EM_JS(void, SYS_Orthof, ( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val ), 
{ return glOrthof(left,right,bottom,top,near_val,far_val) });
void qglOrthof( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val )
{ return SYS_Orthof(left,right,bottom,top,near_val,far_val ); }


EM_JS(void, SYS_ActiveTexture, ( GLenum texture ), 
{ return glActiveTexture(texture) });
void qglActiveTexture( GLenum texture )
{ return SYS_ActiveTexture(texture ); }


EM_JS(void, SYS_CompressedTexImage2D, ( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data ), 
{ return glCompressedTexImage2D(target,level,internalformat,width,height,border,imageSize,data) });
void qglCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data )
{ return SYS_CompressedTexImage2D(target,level,internalformat,width,height,border,imageSize,data ); }


EM_JS(void, SYS_CompressedTexSubImage2D, ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data ), 
{ return glCompressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data) });
void qglCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data )
{ return SYS_CompressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data ); }


EM_JS(void, SYS_GenQueries, ( GLsizei n, GLuint *ids ), 
{ return glGenQueries(n,ids) });
void qglGenQueries( GLsizei n, GLuint *ids )
{ return SYS_GenQueries(n,ids ); }


EM_JS(void, SYS_DeleteQueries, ( GLsizei n, const GLuint *ids ), 
{ return glDeleteQueries(n,ids) });
void qglDeleteQueries( GLsizei n, const GLuint *ids )
{ return SYS_DeleteQueries(n,ids ); }


EM_JS(void, SYS_BeginQuery, ( GLenum target, GLuint id ), 
{ return glBeginQuery(target,id) });
void qglBeginQuery( GLenum target, GLuint id )
{ return SYS_BeginQuery(target,id ); }


EM_JS(void, SYS_EndQuery, ( GLenum target ), 
{ return glEndQuery(target) });
void qglEndQuery( GLenum target )
{ return SYS_EndQuery(target ); }


EM_JS(void, SYS_GetQueryObjectiv, ( GLuint id, GLenum pname, GLint *params ), 
{ return glGetQueryObjectiv(id,pname,params) });
void qglGetQueryObjectiv( GLuint id, GLenum pname, GLint *params )
{ return SYS_GetQueryObjectiv(id,pname,params ); }


EM_JS(void, SYS_GetQueryObjectuiv, ( GLuint id, GLenum pname, GLuint *params ), 
{ return glGetQueryObjectuiv(id,pname,params) });
void qglGetQueryObjectuiv( GLuint id, GLenum pname, GLuint *params )
{ return SYS_GetQueryObjectuiv(id,pname,params ); }


EM_JS(void, SYS_BindBuffer, ( GLenum target, GLuint buffer ), 
{ return glBindBuffer(target,buffer) });
void qglBindBuffer( GLenum target, GLuint buffer )
{ return SYS_BindBuffer(target,buffer ); }


EM_JS(void, SYS_DeleteBuffers, ( GLsizei n, const GLuint *buffers ), 
{ return glDeleteBuffers(n,buffers) });
void qglDeleteBuffers( GLsizei n, const GLuint *buffers )
{ return SYS_DeleteBuffers(n,buffers ); }


EM_JS(void, SYS_GenBuffers, ( GLsizei n, GLuint *buffers ), 
{ return glGenBuffers(n,buffers) });
void qglGenBuffers( GLsizei n, GLuint *buffers )
{ return SYS_GenBuffers(n,buffers ); }


EM_JS(void, SYS_BufferData, ( GLenum target, GLsizeiptr size, const void *data, GLenum usage ), 
{ return glBufferData(target,size,data,usage) });
void qglBufferData( GLenum target, GLsizeiptr size, const void *data, GLenum usage )
{ return SYS_BufferData(target,size,data,usage ); }


EM_JS(void, SYS_BufferSubData, ( GLenum target, GLintptr offset, GLsizeiptr size, const void *data ), 
{ return glBufferSubData(target,offset,size,data) });
void qglBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void *data )
{ return SYS_BufferSubData(target,offset,size,data ); }


EM_JS(GLboolean, SYS_UnmapBuffer, ( GLenum target ), 
{ return glUnmapBuffer(target) });
GLboolean qglUnmapBuffer( GLenum target )
{ return SYS_UnmapBuffer(target ); }


EM_JS(void *, SYS_MapBufferRange, ( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ), 
{ return glMapBufferRange(target,offset,length,access) });
void * qglMapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access )
{ return SYS_MapBufferRange(target,offset,length,access ); }


EM_JS(void, SYS_ReadBuffer, ( GLenum mode ), 
{ return glReadBuffer(mode) });
void qglReadBuffer( GLenum mode )
{ return SYS_ReadBuffer(mode ); }


EM_JS(void, SYS_DrawBuffers, ( GLsizei n, const GLenum *bufs ), 
{ return glDrawBuffers(n,bufs) });
void qglDrawBuffers( GLsizei n, const GLenum *bufs )
{ return SYS_DrawBuffers(n,bufs ); }


EM_JS(void, SYS_AttachShader, ( GLuint program, GLuint shader ), 
{ return glAttachShader(program,shader) });
void qglAttachShader( GLuint program, GLuint shader )
{ return SYS_AttachShader(program,shader ); }


EM_JS(void, SYS_BindAttribLocation, ( GLuint program, GLuint index, const GLchar *name ), 
{ return glBindAttribLocation(program,index,name) });
void qglBindAttribLocation( GLuint program, GLuint index, const GLchar *name )
{ return SYS_BindAttribLocation(program,index,name ); }


EM_JS(void, SYS_CompileShader, ( GLuint shader ), 
{ return glCompileShader(shader) });
void qglCompileShader( GLuint shader )
{ return SYS_CompileShader(shader ); }


EM_JS(GLuint, SYS_CreateProgram, ( void ), 
{ return glCreateProgram() });
GLuint qglCreateProgram( void )
{ return SYS_CreateProgram(); }


EM_JS(GLuint, SYS_CreateShader, ( GLenum type ), 
{ return glCreateShader(type) });
GLuint qglCreateShader( GLenum type )
{ return SYS_CreateShader(type ); }


EM_JS(void, SYS_DeleteProgram, ( GLuint program ), 
{ return glDeleteProgram(program) });
void qglDeleteProgram( GLuint program )
{ return SYS_DeleteProgram(program ); }


EM_JS(void, SYS_DeleteShader, ( GLuint shader ), 
{ return glDeleteShader(shader) });
void qglDeleteShader( GLuint shader )
{ return SYS_DeleteShader(shader ); }


EM_JS(void, SYS_DetachShader, ( GLuint program, GLuint shader ), 
{ return glDetachShader(program,shader) });
void qglDetachShader( GLuint program, GLuint shader )
{ return SYS_DetachShader(program,shader ); }


EM_JS(void, SYS_DisableVertexAttribArray, ( GLuint index ), 
{ return glDisableVertexAttribArray(index) });
void qglDisableVertexAttribArray( GLuint index )
{ return SYS_DisableVertexAttribArray(index ); }


EM_JS(void, SYS_EnableVertexAttribArray, ( GLuint index ), 
{ return glEnableVertexAttribArray(index) });
void qglEnableVertexAttribArray( GLuint index )
{ return SYS_EnableVertexAttribArray(index ); }


EM_JS(void, SYS_GetActiveUniform, ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ), 
{ return glGetActiveUniform(program,index,bufSize,length,size,type,name) });
void qglGetActiveUniform( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )
{ return SYS_GetActiveUniform(program,index,bufSize,length,size,type,name ); }


EM_JS(void, SYS_GetProgramiv, ( GLuint program, GLenum pname, GLint *params ), 
{ return glGetProgramiv(program,pname,params) });
void qglGetProgramiv( GLuint program, GLenum pname, GLint *params )
{ return SYS_GetProgramiv(program,pname,params ); }


EM_JS(void, SYS_GetProgramInfoLog, ( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog ), 
{ return glGetProgramInfoLog(program,bufSize,length,infoLog) });
void qglGetProgramInfoLog( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog )
{ return SYS_GetProgramInfoLog(program,bufSize,length,infoLog ); }


EM_JS(void, SYS_GetShaderiv, ( GLuint shader, GLenum pname, GLint *params ), 
{ return glGetShaderiv(shader,pname,params) });
void qglGetShaderiv( GLuint shader, GLenum pname, GLint *params )
{ return SYS_GetShaderiv(shader,pname,params ); }


EM_JS(void, SYS_GetShaderInfoLog, ( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog ), 
{ return glGetShaderInfoLog(shader,bufSize,length,infoLog) });
void qglGetShaderInfoLog( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog )
{ return SYS_GetShaderInfoLog(shader,bufSize,length,infoLog ); }


EM_JS(void, SYS_GetShaderSource, ( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source ), 
{ return glGetShaderSource(shader,bufSize,length,source) });
void qglGetShaderSource( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source )
{ return SYS_GetShaderSource(shader,bufSize,length,source ); }


EM_JS(GLint, SYS_GetUniformLocation, ( GLuint program, const GLchar *name ), 
{ return glGetUniformLocation(program,name) });
GLint qglGetUniformLocation( GLuint program, const GLchar *name )
{ return SYS_GetUniformLocation(program,name ); }


EM_JS(void, SYS_LinkProgram, ( GLuint program ), 
{ return glLinkProgram(program) });
void qglLinkProgram( GLuint program )
{ return SYS_LinkProgram(program ); }


EM_JS(void, SYS_ShaderSource, ( GLuint shader, GLsizei count, const GLchar* *string, const GLint *length ), 
{ return glShaderSource(shader,count,string,length) });
void qglShaderSource( GLuint shader, GLsizei count, const GLchar* *string, const GLint *length )
{ return SYS_ShaderSource(shader,count,string,length ); }


EM_JS(void, SYS_UseProgram, ( GLuint program ), 
{ return glUseProgram(program) });
void qglUseProgram( GLuint program )
{ return SYS_UseProgram(program ); }


EM_JS(void, SYS_Uniform1f, ( GLint location, GLfloat v0 ), 
{ return glUniform1f(location,v0) });
void qglUniform1f( GLint location, GLfloat v0 )
{ return SYS_Uniform1f(location,v0 ); }


EM_JS(void, SYS_Uniform2f, ( GLint location, GLfloat v0, GLfloat v1 ), 
{ return glUniform2f(location,v0,v1) });
void qglUniform2f( GLint location, GLfloat v0, GLfloat v1 )
{ return SYS_Uniform2f(location,v0,v1 ); }


EM_JS(void, SYS_Uniform3f, ( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ), 
{ return glUniform3f(location,v0,v1,v2) });
void qglUniform3f( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 )
{ return SYS_Uniform3f(location,v0,v1,v2 ); }


EM_JS(void, SYS_Uniform4f, ( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ), 
{ return glUniform4f(location,v0,v1,v2,v3) });
void qglUniform4f( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{ return SYS_Uniform4f(location,v0,v1,v2,v3 ); }


EM_JS(void, SYS_Uniform1i, ( GLint location, GLint v0 ), 
{ return glUniform1i(location,v0) });
void qglUniform1i( GLint location, GLint v0 )
{ return SYS_Uniform1i(location,v0 ); }


EM_JS(void, SYS_Uniform1fv, ( GLint location, GLsizei count, const GLfloat *value ), 
{ return glUniform1fv(location,count,value) });
void qglUniform1fv( GLint location, GLsizei count, const GLfloat *value )
{ return SYS_Uniform1fv(location,count,value ); }


EM_JS(void, SYS_UniformMatrix4fv, ( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ), 
{ return glUniformMatrix4fv(location,count,transpose,value) });
void qglUniformMatrix4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value )
{ return SYS_UniformMatrix4fv(location,count,transpose,value ); }


EM_JS(void, SYS_ValidateProgram, ( GLuint program ), 
{ return glValidateProgram(program) });
void qglValidateProgram( GLuint program )
{ return SYS_ValidateProgram(program ); }


EM_JS(void, SYS_VertexAttribPointer, ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer ), 
{ return glVertexAttribPointer(index,size,type,normalized,stride,pointer) });
void qglVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer )
{ return SYS_VertexAttribPointer(index,size,type,normalized,stride,pointer ); }


EM_JS(const GLubyte *, SYS_GetStringi, ( GLenum name, GLuint index ), 
{ return glGetStringi(name,index) });
const GLubyte * qglGetStringi( GLenum name, GLuint index )
{ return SYS_GetStringi(name,index ); }


EM_JS(void, SYS_BindRenderbuffer, ( GLenum target, GLuint renderbuffer ), 
{ return glBindRenderbuffer(target,renderbuffer) });
void qglBindRenderbuffer( GLenum target, GLuint renderbuffer )
{ return SYS_BindRenderbuffer(target,renderbuffer ); }


EM_JS(void, SYS_DeleteRenderbuffers, ( GLsizei n, const GLuint *renderbuffers ), 
{ return glDeleteRenderbuffers(n,renderbuffers) });
void qglDeleteRenderbuffers( GLsizei n, const GLuint *renderbuffers )
{ return SYS_DeleteRenderbuffers(n,renderbuffers ); }


EM_JS(void, SYS_GenRenderbuffers, ( GLsizei n, GLuint *renderbuffers ), 
{ return glGenRenderbuffers(n,renderbuffers) });
void qglGenRenderbuffers( GLsizei n, GLuint *renderbuffers )
{ return SYS_GenRenderbuffers(n,renderbuffers ); }


EM_JS(void, SYS_RenderbufferStorage, ( GLenum target, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glRenderbufferStorage(target,internalformat,width,height) });
void qglRenderbufferStorage( GLenum target, GLenum internalformat, GLsizei width, GLsizei height )
{ return SYS_RenderbufferStorage(target,internalformat,width,height ); }


EM_JS(void, SYS_BindFramebuffer, ( GLenum target, GLuint framebuffer ), 
{ return glBindFramebuffer(target,framebuffer) });
void qglBindFramebuffer( GLenum target, GLuint framebuffer )
{ return SYS_BindFramebuffer(target,framebuffer ); }


EM_JS(void, SYS_DeleteFramebuffers, ( GLsizei n, const GLuint *framebuffers ), 
{ return glDeleteFramebuffers(n,framebuffers) });
void qglDeleteFramebuffers( GLsizei n, const GLuint *framebuffers )
{ return SYS_DeleteFramebuffers(n,framebuffers ); }


EM_JS(void, SYS_GenFramebuffers, ( GLsizei n, GLuint *framebuffers ), 
{ return glGenFramebuffers(n,framebuffers) });
void qglGenFramebuffers( GLsizei n, GLuint *framebuffers )
{ return SYS_GenFramebuffers(n,framebuffers ); }


EM_JS(GLenum, SYS_CheckFramebufferStatus, ( GLenum target ), 
{ return glCheckFramebufferStatus(target) });
GLenum qglCheckFramebufferStatus( GLenum target )
{ return SYS_CheckFramebufferStatus(target ); }


EM_JS(void, SYS_FramebufferTexture2D, ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ), 
{ return glFramebufferTexture2D(target,attachment,textarget,texture,level) });
void qglFramebufferTexture2D( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level )
{ return SYS_FramebufferTexture2D(target,attachment,textarget,texture,level ); }


EM_JS(void, SYS_FramebufferRenderbuffer, ( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ), 
{ return glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer) });
void qglFramebufferRenderbuffer( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer )
{ return SYS_FramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer ); }


EM_JS(void, SYS_GenerateMipmap, ( GLenum target ), 
{ return glGenerateMipmap(target) });
void qglGenerateMipmap( GLenum target )
{ return SYS_GenerateMipmap(target ); }


EM_JS(void, SYS_BlitFramebuffer, ( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ), 
{ return glBlitFramebuffer(srcX0,srcY0,srcX1,srcY1,dstX0,dstY0,dstX1,dstY1,mask,filter) });
void qglBlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter )
{ return SYS_BlitFramebuffer(srcX0,srcY0,srcX1,srcY1,dstX0,dstY0,dstX1,dstY1,mask,filter ); }


EM_JS(void, SYS_RenderbufferStorageMultisample, ( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glRenderbufferStorageMultisample(target,samples,internalformat,width,height) });
void qglRenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height )
{ return SYS_RenderbufferStorageMultisample(target,samples,internalformat,width,height ); }


EM_JS(void, SYS_BindVertexArray, ( GLuint array ), 
{ return glBindVertexArray(array) });
void qglBindVertexArray( GLuint array )
{ return SYS_BindVertexArray(array ); }


EM_JS(void, SYS_DeleteVertexArrays, ( GLsizei n, const GLuint *arrays ), 
{ return glDeleteVertexArrays(n,arrays) });
void qglDeleteVertexArrays( GLsizei n, const GLuint *arrays )
{ return SYS_DeleteVertexArrays(n,arrays ); }


EM_JS(void, SYS_GenVertexArrays, ( GLsizei n, GLuint *arrays ), 
{ return glGenVertexArrays(n,arrays) });
void qglGenVertexArrays( GLsizei n, GLuint *arrays )
{ return SYS_GenVertexArrays(n,arrays ); }


EM_JS(GLvoid, SYS_BindMultiTextureEXT, ( GLenum texunit, GLenum target, GLuint texture ), 
{ return glBindMultiTextureEXT(texunit,target,texture) });
GLvoid qglBindMultiTextureEXT( GLenum texunit, GLenum target, GLuint texture )
{ return SYS_BindMultiTextureEXT(texunit,target,texture ); }


EM_JS(GLvoid, SYS_TextureParameterfEXT, ( GLuint texture, GLenum target, GLenum pname, GLfloat param ), 
{ return glTextureParameterfEXT(texture,target,pname,param) });
GLvoid qglTextureParameterfEXT( GLuint texture, GLenum target, GLenum pname, GLfloat param )
{ return SYS_TextureParameterfEXT(texture,target,pname,param ); }


EM_JS(GLvoid, SYS_TextureParameteriEXT, ( GLuint texture, GLenum target, GLenum pname, GLint param ), 
{ return glTextureParameteriEXT(texture,target,pname,param) });
GLvoid qglTextureParameteriEXT( GLuint texture, GLenum target, GLenum pname, GLint param )
{ return SYS_TextureParameteriEXT(texture,target,pname,param ); }


EM_JS(GLvoid, SYS_TextureImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTextureImage2DEXT(texture,target,level,internalformat,width,height,border,format,type,pixels) });
GLvoid qglTextureImage2DEXT( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{ return SYS_TextureImage2DEXT(texture,target,level,internalformat,width,height,border,format,type,pixels ); }


EM_JS(GLvoid, SYS_TextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ), 
{ return glTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,type,pixels) });
GLvoid qglTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{ return SYS_TextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,type,pixels ); }


EM_JS(GLvoid, SYS_CopyTextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ), 
{ return glCopyTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,x,y,width,height) });
GLvoid qglCopyTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{ return SYS_CopyTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,x,y,width,height ); }


EM_JS(GLvoid, SYS_CompressedTextureImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data ), 
{ return glCompressedTextureImage2DEXT(texture,target,level,internalformat,width,height,border,imageSize,data) });
GLvoid qglCompressedTextureImage2DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )
{ return SYS_CompressedTextureImage2DEXT(texture,target,level,internalformat,width,height,border,imageSize,data ); }


EM_JS(GLvoid, SYS_CompressedTextureSubImage2DEXT, ( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data ), 
{ return glCompressedTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,imageSize,data) });
GLvoid qglCompressedTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data )
{ return SYS_CompressedTextureSubImage2DEXT(texture,target,level,xoffset,yoffset,width,height,format,imageSize,data ); }


EM_JS(GLvoid, SYS_GenerateTextureMipmapEXT, ( GLuint texture, GLenum target ), 
{ return glGenerateTextureMipmapEXT(texture,target) });
GLvoid qglGenerateTextureMipmapEXT( GLuint texture, GLenum target )
{ return SYS_GenerateTextureMipmapEXT(texture,target ); }


EM_JS(GLvoid, SYS_ProgramUniform1iEXT, ( GLuint program, GLint location, GLint v0 ), 
{ return glProgramUniform1iEXT(program,location,v0) });
GLvoid qglProgramUniform1iEXT( GLuint program, GLint location, GLint v0 )
{ return SYS_ProgramUniform1iEXT(program,location,v0 ); }


EM_JS(GLvoid, SYS_ProgramUniform1fEXT, ( GLuint program, GLint location, GLfloat v0 ), 
{ return glProgramUniform1fEXT(program,location,v0) });
GLvoid qglProgramUniform1fEXT( GLuint program, GLint location, GLfloat v0 )
{ return SYS_ProgramUniform1fEXT(program,location,v0 ); }


EM_JS(GLvoid, SYS_ProgramUniform2fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1 ), 
{ return glProgramUniform2fEXT(program,location,v0,v1) });
GLvoid qglProgramUniform2fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1 )
{ return SYS_ProgramUniform2fEXT(program,location,v0,v1 ); }


EM_JS(GLvoid, SYS_ProgramUniform3fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ), 
{ return glProgramUniform3fEXT(program,location,v0,v1,v2) });
GLvoid qglProgramUniform3fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2 )
{ return SYS_ProgramUniform3fEXT(program,location,v0,v1,v2 ); }


EM_JS(GLvoid, SYS_ProgramUniform4fEXT, ( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ), 
{ return glProgramUniform4fEXT(program,location,v0,v1,v2,v3) });
GLvoid qglProgramUniform4fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{ return SYS_ProgramUniform4fEXT(program,location,v0,v1,v2,v3 ); }


EM_JS(GLvoid, SYS_ProgramUniform1fvEXT, ( GLuint program, GLint location, GLsizei count, const GLfloat *value ), 
{ return glProgramUniform1fvEXT(program,location,count,value) });
GLvoid qglProgramUniform1fvEXT( GLuint program, GLint location, GLsizei count, const GLfloat *value )
{ return SYS_ProgramUniform1fvEXT(program,location,count,value ); }


EM_JS(GLvoid, SYS_ProgramUniformMatrix4fvEXT, ( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ), 
{ return glProgramUniformMatrix4fvEXT(program,location,count,transpose,value) });
GLvoid qglProgramUniformMatrix4fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value )
{ return SYS_ProgramUniformMatrix4fvEXT(program,location,count,transpose,value ); }


EM_JS(GLvoid, SYS_NamedRenderbufferStorageEXT, ( GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glNamedRenderbufferStorageEXT(renderbuffer,internalformat,width,height) });
GLvoid qglNamedRenderbufferStorageEXT( GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height )
{ return SYS_NamedRenderbufferStorageEXT(renderbuffer,internalformat,width,height ); }


EM_JS(GLvoid, SYS_NamedRenderbufferStorageMultisampleEXT, ( GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ), 
{ return glNamedRenderbufferStorageMultisampleEXT(renderbuffer,samples,internalformat,width,height) });
GLvoid qglNamedRenderbufferStorageMultisampleEXT( GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height )
{ return SYS_NamedRenderbufferStorageMultisampleEXT(renderbuffer,samples,internalformat,width,height ); }


EM_JS(GLenum, SYS_CheckNamedFramebufferStatusEXT, ( GLuint framebuffer, GLenum target ), 
{ return glCheckNamedFramebufferStatusEXT(framebuffer,target) });
GLenum qglCheckNamedFramebufferStatusEXT( GLuint framebuffer, GLenum target )
{ return SYS_CheckNamedFramebufferStatusEXT(framebuffer,target ); }


EM_JS(GLvoid, SYS_NamedFramebufferTexture2DEXT, ( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level ), 
{ return glNamedFramebufferTexture2DEXT(framebuffer,attachment,textarget,texture,level) });
GLvoid qglNamedFramebufferTexture2DEXT( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level )
{ return SYS_NamedFramebufferTexture2DEXT(framebuffer,attachment,textarget,texture,level ); }


EM_JS(GLvoid, SYS_NamedFramebufferRenderbufferEXT, ( GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ), 
{ return glNamedFramebufferRenderbufferEXT(framebuffer,attachment,renderbuffertarget,renderbuffer) });
GLvoid qglNamedFramebufferRenderbufferEXT( GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer )
{ return SYS_NamedFramebufferRenderbufferEXT(framebuffer,attachment,renderbuffertarget,renderbuffer ); }
