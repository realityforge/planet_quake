
#include "tr_rtgroupedshader.h"
#include "tr_rtshaders.h"
#include "threadpool/threadpool.hpp"
#include <boost/bind.hpp>
#include <interlocked.h>
#include <threadinghelper.h>
#include <rapido.h>
#include <vector>
using namespace std;
using namespace rapido;

extern "C"
{
#include "tr_local.h"
}

#define TILE_SIZE 8

static StaticKdTree staticScene;
static DynamicBvh::Geometry dynamicGeometry;
static SceneGroup<StaticKdTree, DynamicBvh::Geometry> sceneGroup(&staticScene, &dynamicGeometry);
static Tracer<SceneGroup<StaticKdTree, DynamicBvh::Geometry> > tracer(&sceneGroup);

static bool initialized = false, floatTexturesSupported;
static GLuint rtTex, rtPbo;
static float4 *rtImage;
static char *rtImageMem;
static int rtWidth, rtHeight;

static int rtNumberOfThreads;
static boost::threadpool::pool rtThreadPool;

extern "C" void RT_InitRaytracer(void)
{
	// query support for pixel buffer objects
	if(strstr(glConfig.extensions_string, "GL_ARB_pixel_buffer_object") == 0)
	{
		Com_Error(ERR_DROP, "GL_ARB_pixel_buffer_object is not supported!");
		return;
	}

	// query support for floating point texture support
	// TODO: floating point texture cause a slowdown and no increase in speed!
	floatTexturesSupported = false; // strstr(glConfig.extensions_string, "GL_ARB_texture_float") != 0;

	// create the thread pool
	rtNumberOfThreads = GetNumberOfProcessors();
	rtThreadPool = boost::threadpool::pool(rtNumberOfThreads);

	// round width and height up so that they're a multiple of TILE_SIZE
	int width = glConfig.vidWidth, height = glConfig.vidHeight;
	int rx = (width % TILE_SIZE), ry = (height % TILE_SIZE);
	rtWidth = width + ((rx == 0) ? 0 : (TILE_SIZE - rx));
	rtHeight = height + ((ry == 0) ? 0 : (TILE_SIZE - ry));

	// create a pbo for fast uploads of image data to the gpu and a texture to hold the data
	qglGenTextures(1, &rtTex);
	qglGenBuffersARB(1, &rtPbo);
	if(!floatTexturesSupported)
	{
		// make sure rtImage is aligned on 16-byte border for sse operations!
		rtImageMem = reinterpret_cast<char *>(ri.Malloc(sizeof(float4) * rtWidth * rtHeight + 16));
		rtImage = reinterpret_cast<float4 *>(rtImageMem + (16 - (reinterpret_cast<long>(rtImageMem) & 15)));
	}

	qglBindTexture(GL_TEXTURE_2D, rtTex);
	qglTexImage2D(GL_TEXTURE_2D, 0, floatTexturesSupported ? GL_RGBA32F_ARB : GL_RGBA, rtWidth, rtHeight, 0, GL_RGBA, floatTexturesSupported ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);
	qglBindTexture(GL_TEXTURE_2D, 0);

	qglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, rtPbo);
	qglBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, rtWidth * rtHeight * (floatTexturesSupported ? (sizeof(float) * 4) : sizeof(int)), 0, GL_DYNAMIC_DRAW_ARB);
	qglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	initialized = true;
}

extern "C" void RT_ShutdownRaytracer(void)
{
	RT_DestroyAllShaders();

	if(initialized)
	{
		if(!floatTexturesSupported)
			ri.Free(rtImageMem);
		qglDeleteBuffersARB(1, &rtPbo);
		qglDeleteTextures(1, &rtTex);
		initialized = false;
	}
}

inline static unsigned int min(unsigned int a, unsigned int b) { return (a <= b) ? a : b; }
inline static unsigned int max(unsigned int a, unsigned int b) { return (a >= b) ? a : b; }

inline static unsigned int ConvertPixel(const float4 &col)
{
	return max(0, min((int)(col.x * 255.0f), 255)) |
		max(0, min((int)(col.y * 255.0f), 255)) << 8 |
		max(0, min((int)(col.z * 255.0f), 255)) << 16 |
		max(0, min((int)(col.w * 255.0f), 255)) << 24;
}

inline static void ConvertImage(unsigned int *dest, const float *src, unsigned int count)
{	
	unsigned int pcnt = count & 0x2;
	if(pcnt > 0)
	{
		for(unsigned int i = 0; i < pcnt; ++i, ++dest, src += 4)
			*dest = ConvertPixel(*reinterpret_cast<const float4 *>(src));
	}
	
	// converts 4 pixels at a time using sse instructions
	__m128 scale4 = _mm_set1_ps(255.0f);
	unsigned int *end = dest + count;
	while(dest < end)
	{
		__m128i color0 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src + 0), scale4));
		__m128i color1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src + 4), scale4));
		__m128i color2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src + 8), scale4));
		__m128i color3 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src + 12), scale4));
				
		__m128i final = _mm_packus_epi16(_mm_packs_epi32(color0, color1), _mm_packs_epi32(color2, color3));
		_mm_stream_ps(reinterpret_cast<float *>(dest), (__m128)final);
		
		src += 16; dest += 4;
	}
}

static void TraceView(viewParms_t *parms);

extern "C" void RT_RenderScene(viewParms_t *parms)
{
	qglBindTexture(GL_TEXTURE_2D, rtTex);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	qglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, rtPbo);

	if(floatTexturesSupported)
		rtImage = reinterpret_cast<float4 *>(qglMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB));

	TraceView(parms);

	if(!floatTexturesSupported)
	{
		unsigned int *dest = reinterpret_cast<unsigned int *>(qglMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB));
		ConvertImage(dest, reinterpret_cast<float *>(rtImage), rtWidth * rtHeight);
	}

	qglUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rtWidth, rtHeight, GL_RGBA, floatTexturesSupported ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);
	qglBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	// present the rendered image on the screen
	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	{
		qglLoadIdentity();

		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		{
			qglLoadIdentity();
			qglOrtho(0.0, glConfig.vidWidth, glConfig.vidHeight, 0.0, -1.0, 1.0);

			int oldState = glState.glStateBits;
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHTEST_DISABLE);

			int oldEnv = glState.texEnv[glState.currenttmu];
			GL_TexEnv(GL_REPLACE);

			const float maxt = rt_pixeldoubling->value ? 0.5f : 1.0f;

			qglBegin(GL_QUADS);
			qglTexCoord2f(0.0f, 0.0f);
			qglVertex2f(0.0f, 0.0f);

			qglTexCoord2f(maxt, 0.0f);
			qglVertex2f(glConfig.vidWidth, 0.0f);

			qglTexCoord2f(maxt, maxt);
			qglVertex2f(glConfig.vidWidth, glConfig.vidHeight);

			qglTexCoord2f(0.0f, maxt);
			qglVertex2f(0.0f, glConfig.vidHeight);
			qglEnd();

			GL_TexEnv(oldEnv);
			GL_State(oldState);
		}
		qglPopMatrix();
	}
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	qglBindTexture(GL_TEXTURE_2D, 0);
}

// Geometry processing --------------------------------------------------------

static float3 *staticNormals = 0;
static float2 *staticTexCoords = 0;
static shader_t **staticShaders = 0;

static vector<float3> dynamicNormals;
static vector<float2> dynamicTexCoords;
static vector<shader_t *> dynamicShaders; // pointer to shader per triangle

extern "C" void RT_LoadWorldMap(void)
{
	if(tr.world == 0 || tr.world->bmodels == 0)
		return;

	const bmodel_t &worldModel = tr.world->bmodels[0];

	// TODO: filter geometry with shaders that deform vertices and handle it as dynamic

	// 1. count the number of triangles
	int numberOfTriangles = 0;
	for(msurface_t *surf = worldModel.firstSurface, *end = surf + worldModel.numSurfaces; surf < end; ++surf)
	{
		surfaceType_t *data = surf->data;
		switch(*data)
		{
		case SF_FACE: numberOfTriangles += reinterpret_cast<srfSurfaceFace_t *>(data)->numIndices / 3; break;
		case SF_GRID: numberOfTriangles += reinterpret_cast<srfGridMesh_t *>(data)->width * reinterpret_cast<srfGridMesh_t *>(data)->height * 2; break;
		case SF_TRIANGLES: numberOfTriangles += reinterpret_cast<srfTriangles_t *>(data)->numIndexes / 3; break;
		default: break;
		}
	}

	Com_Printf("Building ray tracing acceleration structure for map %s. Totalling %i triangles ...\n", tr.world->baseName, numberOfTriangles);

	if(numberOfTriangles > 0)
	{
		// 2. allocate memory and generate triangles
		float3 *vertices = reinterpret_cast<float3 *>(ri.Malloc(sizeof(float3) * numberOfTriangles * 3));
		staticNormals = reinterpret_cast<float3 *>(ri.Hunk_Alloc(sizeof(float3) * numberOfTriangles * 3, h_low));
		staticTexCoords = reinterpret_cast<float2 *>(ri.Hunk_Alloc(sizeof(float2) * numberOfTriangles * 3, h_low));
		staticShaders = reinterpret_cast<shader_t **>(ri.Hunk_Alloc(sizeof(shader_t *) * numberOfTriangles, h_low));

		shader_t **sdest = staticShaders;
		float2 *tcdest = staticTexCoords;
		float3 *dest = vertices, *ndest = staticNormals;

		for(msurface_t *surf = worldModel.firstSurface, *end = surf + worldModel.numSurfaces; surf < end; ++surf)
		{
			shader_t *shader = surf->shader;
			surfaceType_t *data = surf->data;
			switch(*data)
			{
			case SF_FACE:
				{
					srfSurfaceFace_t *face = reinterpret_cast<srfSurfaceFace_t *>(data);
					unsigned int *indices = reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(face) + face->ofsIndices);
					for(int i = 0; i < face->numIndices; ++i)
					{
						float *vert = face->points[0] + indices[i] * VERTEXSIZE;
						*dest++ = *reinterpret_cast<float3 *>(vert);
						*tcdest++ = *reinterpret_cast<float2 *>(vert + 3);
						*ndest++ = *reinterpret_cast<float3 *>(face->plane.normal);
					}
					for(int i = 0; i < face->numIndices; i += 3)
						*sdest++ = shader;
				}
				break;

			case SF_GRID:
				{
					srfGridMesh_t *grid = reinterpret_cast<srfGridMesh_t *>(data);
					for(int y = 1; y < grid->height; ++y)
					{
						for(int x = 1; x < grid->width; ++x)
						{
							int brIdx = y * grid->width + x;
							drawVert_t &a = grid->verts[brIdx - grid->width - 1];
							drawVert_t &b = grid->verts[brIdx - grid->width]; 
							drawVert_t &c = grid->verts[brIdx - 1];
							drawVert_t &d = grid->verts[brIdx]; 
							*dest++ = *(float3 *)a.xyz; *dest++ = *(float3 *)b.xyz; *dest++ = *(float3 *)c.xyz;
							*dest++ = *(float3 *)c.xyz; *dest++ = *(float3 *)b.xyz; *dest++ = *(float3 *)d.xyz;
							*ndest++ = *(float3 *)a.normal; *ndest++ = *(float3 *)b.normal; *ndest++ = *(float3 *)c.normal;
							*ndest++ = *(float3 *)c.normal; *ndest++ = *(float3 *)b.normal; *ndest++ = *(float3 *)d.normal;
							*tcdest++ = *(float2 *)a.st; *tcdest++ = *(float2 *)b.st; *tcdest++ = *(float2 *)c.st;
							*tcdest++ = *(float2 *)c.st; *tcdest++ = *(float2 *)b.st; *tcdest++ = *(float2 *)d.st;
							*sdest++ = shader; *sdest++ = shader;
						}
					}
				}
				break;

			case SF_TRIANGLES:
				{
					srfTriangles_t *tris = reinterpret_cast<srfTriangles_t *>(data);
					for(int i = 0; i < tris->numIndexes; ++i)
					{
						drawVert_t &vert = tris->verts[tris->indexes[i]];
						*dest++ = *reinterpret_cast<float3 *>(vert.xyz);
						*ndest++ = *reinterpret_cast<float3 *>(vert.normal);
						*tcdest++ = *reinterpret_cast<float2 *>(vert.st);
					}
					for(int i = 0; i < tris->numIndexes; i += 3)
						*sdest++ = shader;
				}
				break;

			default: break;
			}
		}

		staticScene = StaticKdTreeBuilder::BuildFrom(vertices, numberOfTriangles);

		ri.Free(vertices);

		// TODO: offset decals based on classification from available shaders
	}
}

extern "C" void RT_ClearScene(void)
{
	// reset dynamic geometry buffer, it will grow again as we add surfaces in R_AddDrawSurfToDynamicScene
	R_SyncRenderThread();
	dynamicGeometry.SetNumberOfTriangles(0);
	dynamicGeometry.SetNumberOfVertices(0);

	dynamicNormals.clear();
	dynamicTexCoords.clear();
	dynamicShaders.clear();
}

extern "C" void RT_SurfaceReady(void)
{
	int numTris = tess.numIndexes / 3;
	int baseVert = dynamicGeometry.GetNumberOfVertices();
	int baseTri = dynamicGeometry.GetNumberOfTriangles();
	dynamicGeometry.SetNumberOfVertices(baseVert + tess.numVertexes);
	dynamicGeometry.SetNumberOfTriangles(baseTri + numTris);

	const float3 &origin = *reinterpret_cast<float3 *>(backEnd.or.origin);
	const float3x3 mat = transpose(*reinterpret_cast<float3x3 *>(backEnd.or.axis));

	float3 *vertices = dynamicGeometry.GetVertices() + baseVert;
	for(int i = 0; i < tess.numVertexes; ++i)
		vertices[i] = mat * *reinterpret_cast<float3 *>(tess.xyz[i]) + origin;

	int *list = dynamicGeometry.GetTriangleList() + baseTri * 3;
	for(int i = 0; i < tess.numIndexes; ++i)
	{
		int idx = tess.indexes[i];
		list[i] = baseVert + idx;
		dynamicNormals.push_back(mat * *reinterpret_cast<float3 *>(tess.normal[idx]));
		dynamicTexCoords.push_back(*reinterpret_cast<float2 *>(tess.texCoords[idx][0]));
	}

	dynamicShaders.insert(dynamicShaders.end(), numTris, tess.shader);
}

// Rendering ------------------------------------------------------------------

class PerspectiveCamera
{
	float3 _origin, _b, _u, _v;

public:
	PerspectiveCamera(const float4x4 &world, const float4x4 &proj, int width, int height)
	{
		float4x4 mat = inverse(world) * inverse(proj);

		float4 eye = mat * float4(0, 0, 0, 1);
		float4 ct = mat * float4(0, 0, 1, 1);
		float4 tl = mat * float4(-1, 1, 1, 1);
		float4 tr = mat * float4(1, 1, 1, 1);
		float4 bl = mat * float4(-1, -1, 1, 1);

		eye = eye * (1.0f / eye.w);
		ct = ct * (1.0f / ct.w);
		_origin = float3(eye) - normalize(float3(ct) - float3(eye)) * 8.0f; // move the camera back a bit

		_b = tl * (1.0f / tl.w) - _origin;
		_u = ((tr * (1.0f / tr.w) - _origin) - _b) * (1.0f / (width - 1));
		_v = ((bl * (1.0f / bl.w) - _origin) - _b) * (1.0f / (height - 1));
	}

	inline ray GetRayForPixel(float x, float y) const { return ray(_origin, normalize(_b + _u * x + _v * y)); }
};

static void ShadeNull(ray **rays, int count, int depth)
{ }

static void ShadeWhite(ray **rays, int count, int depth)
{
	for(int i = 0; i < count; ++i)
		*reinterpret_cast<float4 *>(rays[i]->user.data) = 1.0f;
}

static void ShadeTextured(ray **rays, int count, int depth)
{
	bool dynamic = rays[0]->HitDynamicTriangle();
	int triId = rays[0]->GetDynamicTriangleId(), vertId = 3 * triId;
	shader_t *shader = dynamic ? dynamicShaders[triId] : staticShaders[triId];
	if(shader->shaderClass == 0)
	{
		for(int i = 0; i < count; ++i)
			*reinterpret_cast<float4 *>(rays[i]->user.data) = float4(1.0f, 0.0f, 1.0f, 1.0f);
		return;
	}

	const RtShader *sobj = reinterpret_cast<const RtShader *>(shader->shaderClass);
	int requirements = sobj->GetRequirementsMask();

	float3 *normals = 0; float2 *tcs = 0;
	if(requirements & (RtShader::Normals | RtShader::Diffuses/* | RtShader::Reflections*/))
	{
		if(requirements & RtShader::TextureCoordinates)
		{
			normals = reinterpret_cast<float3 *>(alloca(sizeof(float3) * count));
			tcs = reinterpret_cast<float2 *>(alloca(sizeof(float2) * count));

			const float3 *norms = dynamic ? &dynamicNormals[vertId] : &staticNormals[vertId];			
			const float2 *texcs = dynamic ? &dynamicTexCoords[vertId] : &staticTexCoords[vertId];
			float3 an(norms[0]), bn(norms[1] - norms[0]), cn(norms[2] - norms[0]);
			float2 atc(texcs[0]), btc(texcs[1] - texcs[0]), ctc(texcs[2] - texcs[0]);
			for(int i = 0; i < count; ++i)
			{
				normals[i] = an + bn * rays[i]->hit.beta + cn * rays[i]->hit.gamma;
				tcs[i] = atc + btc * rays[i]->hit.beta + ctc * rays[i]->hit.gamma;
			}
		}
		else
		{
			normals = reinterpret_cast<float3 *>(alloca(sizeof(float3) * count));

			const float3 *norms = dynamic ? &dynamicNormals[vertId] : &staticNormals[vertId];			
			float3 a(norms[0]), b(norms[1] - norms[0]), c(norms[2] - norms[0]);
			for(int i = 0; i < count; ++i)
				normals[i] = a + b * rays[i]->hit.beta + c * rays[i]->hit.gamma;
		}
	}
	else if(requirements & RtShader::TextureCoordinates)
	{	
		tcs = reinterpret_cast<float2 *>(alloca(sizeof(float2) * count));

		const float2 *texcs = dynamic ? &dynamicTexCoords[vertId] : &staticTexCoords[vertId];
		float2 a(texcs[0]), b(texcs[1] - texcs[0]), c(texcs[2] - texcs[0]);
		for(int i = 0; i < count; ++i)
			tcs[i] = a + b * rays[i]->hit.beta + c * rays[i]->hit.gamma;
	}

	/*float4 *reflections = 0;
	if(requirements & RtShader::Reflections)
	{
		reflections = reinterpret_cast<float4 *>(alloca(sizeof(float4) * count));
		if(depth < 1)
		{
			ray *reflectionRays = reinterpret_cast<ray *>(sse_alloca(sizeof(ray) * count));
			for(int i = 0; i < count; ++i)
			{
				const float3 ndir = normalize(rays[i]->direction);
				const float3 reflectedDir = ndir - (2.0f * dot(normals[i], ndir)) * normals[i];
				reflectionRays[i] = rays[i]->SpawnSecondary(reflectedDir, 1.0f);
				reflectionRays[i].user.data = &reflections[i];
			}

			tracer.Trace(reflectionRays, count, 0);
			GroupedShader::Shade(reflectionRays, count, depth + 1, &ShadeTextured, &ShadeBlack);
		}
		else
			memset(reflections, 0, sizeof(float4) * count);
	}*/

	float3 *diffuses = 0;
	if(requirements & RtShader::Diffuses)
	{
		diffuses = reinterpret_cast<float3 *>(alloca(sizeof(float3) * count));
#if 1
		// perform dynamic lighting
		for(int i = 0; i < count; ++i)
		{
			diffuses[i] = float3(fabs(dot(normals[i], normalize(rays[i]->direction))));

			ray lightRays[MAX_DLIGHTS];
			float3 lightColor[MAX_DLIGHTS];
			int lightCount = 0;

			const float3 point = rays[i]->GetHitPoint();
			for(dlight_t *dl = tr.refdef.dlights, *end = dl + tr.refdef.num_dlights; dl < end; ++dl)
			{
				const float3 &origin = *reinterpret_cast<const float3 *>(dl->origin);
				lightRays[lightCount].direction = point - origin;
				const float diffuse = dot(normals[i], lightRays[lightCount].direction);
				if(diffuse < 0.0f)
				{
					const float dirLengthSq = lengthsq(lightRays[lightCount].direction);
					const float radiusSq = dl->radius * dl->radius;
					if(dirLengthSq < radiusSq)
					{
						lightRays[lightCount].origin = origin;
						const float invDistSq = 1.0f - dirLengthSq / radiusSq;
						lightColor[lightCount++] = *reinterpret_cast<float3 *>(dl->color) * (invDistSq * -diffuse / sqrtf(dirLengthSq));
					}
				}
			}

			if(lightCount > 0)
			{
#if 0
				// check for shadows
				tracer.Trace(lightRays, lightCount, ShadowRays);
				for(int l = 0; l < lightCount; ++l)
				{
					if(lightRays[l].hit.distance >= 0.999f)
						diffuses[i] = diffuses[i] + lightColor[l];
				}
#else
				for(int l = 0; l < lightCount; ++l)
					diffuses[i] = diffuses[i] + lightColor[l];
#endif
			}
		}
#else
		for(int i = 0; i < count; ++i)
			diffuses[i] = float3(fabs(dot(normals[i], normalize(rays[i]->direction))));
#endif
	}

	sobj->Sample(rays, count, normals, tcs, diffuses);
}

static void ShadeBlack(ray **rays, int count, int depth)
{
	for(int i = 0; i < count; ++i)
		*reinterpret_cast<float4 *>(rays[i]->user.data) = 0.0f;
}

extern "C" void ShadeBackground(float4 *dest, rapido::ray **rays, int count)
{
	const int depth = 0; // TODO: depth?
	ray *backcolorRays = reinterpret_cast<ray *>(sse_alloca(sizeof(ray) * count));
	for(int i = 0; i < count; ++i)
	{
		backcolorRays[i] = rays[i]->SpawnSecondary(rays[i]->direction, 0.001f);
		backcolorRays[i].user.data = &dest[i];
	}

	tracer.Trace(backcolorRays, count, 0);
	for(int i = 0; i < count; ++i)
		backcolorRays[i].hit.triId = (backcolorRays[i].hit.triId == rays[i]->hit.triId) ? ray::NoHit : backcolorRays[i].hit.triId;
	GroupedShader::Shade(backcolorRays, count, depth + 1, &ShadeTextured, &ShadeBlack);
}

static void ShadeTriIds(ray **rays, int count, int depth)
{
	int triId = rays[0]->GetDynamicTriangleId();
	float4 color = float4(triId & 0xf, (triId >> 4) & 0xf, (triId >> 8) & 0xf, 1.0f) * (1.0f / 15.0f);
	for(int i = 0; i < count; ++i)
		*reinterpret_cast<float4 *>(rays[i]->user.data) = color;
}

static void ShadeNormals(ray **rays, int count, int depth)
{
	bool dynamic = rays[0]->HitDynamicTriangle();
	int vertId = 3 * rays[0]->GetDynamicTriangleId();
	const float3 *verts = dynamic ? &dynamicNormals[vertId] : &staticNormals[vertId];
	for(int i = 0; i < count; ++i)
	{
		float alpha = 1.0f - rays[i]->hit.beta - rays[i]->hit.gamma;
		float3 normal = verts[0] * alpha + verts[1] * rays[i]->hit.beta + verts[2] * rays[i]->hit.gamma;
		*reinterpret_cast<float4 *>(rays[i]->user.data) = float4(normal * 0.5f + 0.5f, 1.0f);
	}
}

static void ShadeTextureCoordinates(ray **rays, int count, int depth)
{
	bool dynamic = rays[0]->HitDynamicTriangle();
	int vertId = 3 * rays[0]->GetDynamicTriangleId();
	const float2 *verts = dynamic ? &dynamicTexCoords[vertId] : &staticTexCoords[vertId];
	for(int i = 0; i < count; ++i)
	{
		float alpha = 1.0f - rays[i]->hit.beta - rays[i]->hit.gamma;
		float2 tc = verts[0] * alpha + verts[1] * rays[i]->hit.beta + verts[2] * rays[i]->hit.gamma;
		*reinterpret_cast<float4 *>(rays[i]->user.data) = float4(tc, 0.0f, 1.0f);
	}
}

static void ShadeShaderIds(ray **rays, int count, int depth)
{
	bool dynamic = rays[0]->HitDynamicTriangle();
	int triId = rays[0]->GetDynamicTriangleId();
	shader_t *shader = dynamic ? dynamicShaders[triId] : staticShaders[triId];
	float4 color = float4(shader->index & 0xf, (shader->index >> 4) & 0xf, (shader->index >> 8) & 0xf, 1.0f) * (1.0f / 15.0f);
	for(int i = 0; i < count; ++i)
		*reinterpret_cast<float4 *>(rays[i]->user.data) = color;
}

inline static void TracePrimaryRays(ray rays[], const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	for(int j = 0, idx = 0; j < TILE_SIZE; j += 2, dest += 2 * stride)
	{
		for(int i = 0; i < TILE_SIZE; i += 2)
		{
			rays[idx] = camera.GetRayForPixel(x + i, y + j);
			rays[idx++].user.data = dest + i;
			rays[idx] = camera.GetRayForPixel(x + i + 1, y + j);
			rays[idx++].user.data = dest + i + 1;
			rays[idx] = camera.GetRayForPixel(x + i, y + j + 1);
			rays[idx++].user.data = dest + i + stride;
			rays[idx] = camera.GetRayForPixel(x + i + 1, y + j + 1);
			rays[idx++].user.data = dest + i + stride + 1;
		}
	}
	tracer.Trace(rays, TILE_SIZE * TILE_SIZE, CommonOrigin);
}

static void TraceTriangleIds(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	GroupedShader::Shade(rays, TILE_SIZE * TILE_SIZE, 0, &ShadeTriIds, &ShadeWhite);
}

static void TraceTotalVisitedNodes(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	for(int i = 0; i < TILE_SIZE * TILE_SIZE; ++i)
	{
		*reinterpret_cast<float4 *>(rays[i].user.data) = float4(
			(rays[i].staticStats.visitedInnerNodes + rays[i].staticStats.visitedLeaves) * (1.0f / 128.0f),
			(rays[i].dynamicStats.visitedInnerNodes + rays[i].dynamicStats.visitedLeaves) * (1.0f / 128.0f), 0.0f, 1.0f);
	}
}

static void TraceIntersectedTriangles(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	for(int i = 0; i < TILE_SIZE * TILE_SIZE; ++i)
	{
		*reinterpret_cast<float4 *>(rays[i].user.data) = float4(
			rays[i].staticStats.intersectedTriangles * (1.0f / 16.0f),
			rays[i].dynamicStats.intersectedTriangles * (1.0f / 16.0f), 0.0f, 1.0f);
	}
}

static void TraceNormals(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	GroupedShader::Shade(rays, TILE_SIZE * TILE_SIZE, 0, &ShadeNormals, &ShadeWhite);
}

static void TraceTextureCoordinates(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	GroupedShader::Shade(rays, TILE_SIZE * TILE_SIZE, 0, &ShadeTextureCoordinates, &ShadeWhite);
}

static void TraceShaderIds(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	GroupedShader::Shade(rays, TILE_SIZE * TILE_SIZE, 0, &ShadeShaderIds, &ShadeWhite);
}

static void TraceTextured(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride)
{
	ray rays[TILE_SIZE * TILE_SIZE];
	TracePrimaryRays(rays, camera, x, y, dest, stride);
	GroupedShader::Shade(rays, TILE_SIZE * TILE_SIZE, 0, &ShadeTextured, &ShadeNull);
}

typedef void TraceFunc(const PerspectiveCamera &camera, int x, int y, float4 *dest, int stride);

static void Trace(TraceFunc *func, const PerspectiveCamera &camera, volatile long *currentTile, int horizontalTiles, long numberOfTiles)
{
	while(true)
	{
		long tile = Interlocked::ExchangeAdd(currentTile, 1);
		if(tile >= numberOfTiles)
			break;

		int x = TILE_SIZE * (tile % horizontalTiles);
		int y = TILE_SIZE * (tile / horizontalTiles);
		func(camera, x, y, rtImage + y * rtWidth + x, rtWidth);
	}	
}

static void TraceView(viewParms_t *parms)
{
	if(parms->viewportWidth <= 0 || parms->viewportHeight <= 0)
		return;

	// select the tracing function
	TraceFunc *tracingFunc;
	switch(rt_mode->integer)
	{
	case 0: tracingFunc = &TraceTriangleIds; break;
	case 1: tracingFunc = &TraceTotalVisitedNodes; break;
	case 2: tracingFunc = &TraceIntersectedTriangles; break;
	case 3: tracingFunc = &TraceNormals; break;
	case 4: tracingFunc = &TraceTextureCoordinates; break;
	case 5: tracingFunc = &TraceShaderIds; break;
	case 6: tracingFunc = &TraceTextured; break;
	default: return;
	}

	// collect dynamic geometry
	R_RenderView(parms);
	R_SyncRenderThread();

	// rebuild acceleration structure
	dynamicGeometry.Rebuild();

	// make sure, all shaders are ready and set level time
	RT_UpdateShaders();

	// initialize the camera
	int width = glConfig.vidWidth, height = glConfig.vidHeight;
	if(rt_pixeldoubling->value) { width /= 2; height /= 2; }
	const float4x4 world = transpose(*reinterpret_cast<float4x4 *>(tr.viewParms.world.modelMatrix));
	const float4x4 proj = transpose(*reinterpret_cast<float4x4 *>(tr.viewParms.projectionMatrix));
	PerspectiveCamera camera = PerspectiveCamera(world, proj, width, height);

	// start the worker threads
	volatile long currentTile = 0;
	int horizontalTiles = width / TILE_SIZE;
	long numberOfTiles = horizontalTiles * (height / TILE_SIZE);
	for(int i = 0; i < rtNumberOfThreads; ++i)
		rtThreadPool.schedule(boost::bind(&Trace, tracingFunc, camera, &currentTile, horizontalTiles, numberOfTiles));
	rtThreadPool.wait();
}

