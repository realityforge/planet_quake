
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
