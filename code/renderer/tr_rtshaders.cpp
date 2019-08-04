
#include "tr_rtshaders.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

extern "C"
{
#include "tr_local.h"
}

//#define DUMP_FUNCTIONS_TO_FILE
//#define USE_SINCOS // not supported on the Mac

float shaderTime = 0.0f; // referenced by LLVM

class DefaultShader : public RtShader
{
	textures::Texture _texture;

	static void SampleDefault(const RtShader *obj, rapido::ray **rays, int count, const float3 *normals, const float2 *tcs, const float3 *diffuses)
	{
		const textures::Texture &tex = static_cast<const DefaultShader *>(obj)->_texture;

		int r = 0;
		while(r + 4 <= count)
		{
			vector4 texture(tex.Sample(vector2(tcs[r], tcs[r + 1], tcs[r + 2], tcs[r + 3])));
			const vector3 diffuse(diffuses[r + 0], diffuses[r + 1], diffuses[r + 2], diffuses[r + 3]);
			texture.x = texture.x * diffuse.x; texture.y = texture.y * diffuse.y; texture.z = texture.z * diffuse.z;
			_MM_TRANSPOSE4_PS(texture.x.f4, texture.y.f4, texture.z.f4, texture.w.f4);
			for(int j = 0; j < 4; ++j, ++r)
				_mm_storeu_ps(reinterpret_cast<float *>(rays[r]->user.data), texture[j].f4);
		}

		for( ; r < count; ++r)
		{
			float4 texture(tex.Sample(tcs[r]));
			const float3 diffuse(diffuses[r]);
			texture.x = texture.x * diffuse.x; texture.y = texture.y * diffuse.y; texture.z = texture.z * diffuse.z;
			*reinterpret_cast<float4 *>(rays[r]->user.data) = texture;
		}
	}

public:
	inline DefaultShader(textures::TextureManager *textureManager, const char *texture) :
		RtShader(&SampleDefault, TextureCoordinates | Diffuses)
	{
		_texture = textureManager->CreateFrom(texture);
	}
};

extern "C" void ShadeBackground(float4 *dest, rapido::ray **rays, int count); // implemented in tr_raytracer.cpp

extern "C" void SampleTexture(float4 *dest, const RtShader *obj, int texId, const float2 *tc)
{
	*dest = obj->_textures[texId].Sample(*tc);
}

extern "C" void SampleAnimatedTexture(float4 *dest, const RtShader *obj, int texId, float animInterval, const float2 *tc)
{
	const vector<textures::Texture> &anims = obj->_animatedTextures[texId];
	const float idx = animInterval * shaderTime;
	const int iidx = static_cast<int>(idx), num = static_cast<int>(anims.size());
	const float4 texa(anims[iidx % num].Sample(*tc)), texb(anims[(iidx + 1) % num].Sample(*tc));
	*dest = lerp(texa, texb, idx - iidx);
}

extern "C" void SampleSkyBox(float4 *dest, const RtShader *obj, int texId, const float3 *direction)
{
	// Determine face and local u/v coordinates ...
	// source: http://developer.nvidia.com/object/cube_map_ogl_tutorial.html

	// major axis 
	// direction     target                              sc     tc    ma 
	// ----------    ---------------------------------   ---    ---   --- 
	//  +rx          GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT   -rz    -ry   rx 
	//  -rx          GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT   +rz    -ry   rx 
	//  +ry          GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT   +rx    +rz   ry 
	//  -ry          GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT   +rx    -rz   ry 
	//  +rz          GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT   +rx    -ry   rz 
	//  -rz          GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT   -rx    -ry   rz

	const float3 absCoords = abs(*direction);
	float2 st; float invMagnitude; int face;
	if(absCoords.x >= absCoords.y && absCoords.x >= absCoords.z)
	{
		invMagnitude = 1.0f / absCoords.x;
		if(direction->x >= 0.0f)
			{ face = 0; st = float2(direction->y, -direction->z); } // major axis direction: +rx
		else
			{ face = 1; st = float2(-direction->y, -direction->z); } // major axis direction: -rx
	}
	else if(absCoords.y >= absCoords.x && absCoords.y >= absCoords.z)
	{
		invMagnitude = 1.0f / absCoords.y;
		if(direction->y >= 0.0f)
			{ face = 5; st = float2(-direction->x, -direction->z); } // major axis direction: +rz
		else
			{ face = 4; st = float2(direction->x, -direction->z); } // major axis direction: -rz
	}
	else // if(absCoords.z >= absCoords.x && absCoords.z >= absCoords.y)
	{
		invMagnitude = 1.0f / absCoords.z;
		if(direction->z >= 0.0f)
			{ face = 2; st = float2(direction->x, -direction->y); } // major axis direction: -ry
		else
			{ face = 3; st = float2(direction->x, direction->y); } // major axis direction: +ry
	}
	st = st * (invMagnitude * 0.5f) + 0.5f; // s = (sc / |ma| + 1) / 2, t = (tc / |ma| + 1) / 2
	st = st * (253.0f / 255.0f) + (1.0f / 255.0f); // modify to avoid seams
	*dest = obj->_skybox[6 * texId + face].Sample(st);
}

static textures::TextureManager textureManager;
static vector<RtShader *> shaders;

struct CompiledShader
{
	RtShader::SampleFunction *sampleFunction;
	int requirementsMask;
	vector<string> textures;
	vector<string> skybox;
	vector<vector<string> > animatedTextures;

	int AddTexture(string &name)
	{
		vector<string>::iterator it = find(textures.begin(), textures.end(), name);
		if(it == textures.end())
		{
			textures.push_back(name);
			return textures.size() - 1;
		}
		else
			return it - textures.begin();
	}

	int AddAnimatedTexture(const vector<string> &names)
	{
		for(vector<vector<string> >::iterator it = animatedTextures.begin(); it != animatedTextures.begin(); ++it)
		{
			if(it->size() == names.size())
			{
				bool match = true;
				for(unsigned int i = 0, e = names.size(); i < e && match; ++i)
					match &= ((*it)[i] == names[i]);
				if(match) return it - animatedTextures.begin();
			}
		}
		animatedTextures.push_back(names);
		return animatedTextures.size() - 1;
	}

	int AddSkyBox(string &name)
	{
		vector<string>::iterator it = find(skybox.begin(), skybox.end(), name);
		if(it == skybox.end())
		{
			skybox.push_back(name);
			return skybox.size() - 1;
		}
		else
			return it - skybox.begin();
	}
};

static string GetShaderIdent(shader_t *shader)
{
	char ident[512];
	sprintf(ident, "%s_%i_%i", shader->name, shader->lightmapIndex, shader->index);
	return ident;
}

static CompiledShader BuildShader(const string &ident, const shader_t *shader);
static map<string, CompiledShader> compiledShaders;

extern "C" void RT_LoadShader(shader_t *shader)
{
	RtShader *obj = 0;
	if(shader->explicitlyDefined)
	{
		string ident = GetShaderIdent(shader);
		map<string, CompiledShader>::iterator it = compiledShaders.find(ident);
		if(it == compiledShaders.end())
		{
			CompiledShader newcomp = BuildShader(ident, shader);
			if(newcomp.sampleFunction != 0)
			{
				compiledShaders[ident] = newcomp; // store in the shader map for later reuse
				obj = new RtShader(newcomp.sampleFunction, newcomp.requirementsMask,
					&textureManager, newcomp.textures, newcomp.animatedTextures, newcomp.skybox);
			}
			else
				Com_Printf("failed to generate code for shader %s\n", shader->name);
		}
		else
		{
			obj = new RtShader(it->second.sampleFunction, it->second.requirementsMask,
				&textureManager, it->second.textures, it->second.animatedTextures, it->second.skybox);
		}
	}
	else
		obj = new DefaultShader(&textureManager, shader->name);

	if(obj != 0)
		shaders.push_back(obj);
	shader->shaderClass = obj;
}

extern "C" void RT_UpdateShaders()
{
	// set the time in the shader module
	shaderTime = tr.refdef.time * (1.0f / 1000.0f);
}

extern "C" void RT_DestroyAllShaders()
{
	for(vector<RtShader *>::iterator it = shaders.begin(); it != shaders.end(); ++it)
		delete *it;
	shaders.clear();
}

// Begin of LLVM Code Generation ----------------------------------------------

#include <boost/shared_ptr.hpp>
using namespace boost;

#define __STDC_LIMIT_MACROS
#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/PassManager.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/LLVMBuilder.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
using namespace llvm;

static ExistingModuleProvider moduleProvider(new Module("shaders"));
static Module &module = *moduleProvider.getModule();
static ExecutionEngine *jitter;
static FunctionPassManager passManager(&moduleProvider);

namespace ShaderOptimization
{
	namespace internal
	{
		inline static FunctionPassManager *CreatePassManager()
		{
			// add optimizations, see http://llvm.cs.uiuc.edu/docs/Passes.html
			passManager.add(new TargetData(*jitter->getTargetData())); // optimize for the jitter
			for(int i = 0; i < 2; ++i)
			{
				passManager.add(createCFGSimplificationPass()); // simplify the control flow graph (deleting unreachable blocks, etc)
				passManager.add(createPromoteMemoryToRegisterPass());// Kill useless allocas
				passManager.add(createGVNPass()); // eliminate common subexpressions
				passManager.add(createGCSEPass()); // Remove common subexprs
				passManager.add(createSCCPPass()); // Constant prop with SCCP
				passManager.add(createInstructionCombiningPass()); // "peephole" optimizations
				passManager.add(createReassociatePass()); // reassociate expressions
				passManager.add(createScalarReplAggregatesPass()); // Break up aggregate allocas
				passManager.add(createCondPropagationPass()); // Propagate conditionals
				passManager.add(createInstructionCombiningPass()); // Clean up after LICM/reassoc
				passManager.add(createDeadStoreEliminationPass()); // Delete dead stores
				passManager.add(createAggressiveDCEPass()); // SSA based 'Aggressive DCE'
			}
			// TODO: add more or reorder?
			return &passManager;
		}
	}

	static FunctionPassManager *PassManager;
}

// Type Definitions for Code Generation ---------------------------------------

// these variables are initialized by BuildShader and are valid only during building
static LLVMFoldingBuilder *builder;
static Function *currentFunction;
static CompiledShader *currentShader;

inline static LLVMFoldingBuilder *Builder() { return builder; }
#include "codegen/floats.h"

namespace ShaderType
{
	namespace internal
	{
		inline static StructType *CreateFloat2Ty()
		{
			vector<const Type *> float2Fields(2, Type::FloatTy);
			StructType *float2Ty = StructType::get(float2Fields, true);
			module.addTypeName("float2", float2Ty);
			return float2Ty;
		}

		inline static StructType *CreateFloat3Ty()
		{
			vector<const Type *> float3Fields(3, Type::FloatTy);
			StructType *float3Ty = StructType::get(float3Fields, true);
			module.addTypeName("float3", float3Ty);
			return float3Ty;
		}

		inline static StructType *CreateFloat4Ty()
		{
			vector<const Type *> float4Fields(4, Type::FloatTy);
			StructType *float4Ty = StructType::get(float4Fields, true);
			module.addTypeName("float4", float4Ty);
			return float4Ty;
		}

		inline static StructType *CreateHitTy()
		{
			vector<const Type *> hitFields;
			hitFields.push_back(Type::Int32Ty); // hit.triId
			hitFields.push_back(Type::FloatTy); // hit.distance
			hitFields.push_back(Type::FloatTy); // hit.beta
			hitFields.push_back(Type::FloatTy); // hit.gamma
			StructType *hitTy = StructType::get(hitFields, true);
			module.addTypeName("hit", hitTy);
			return hitTy;
		}

		inline static StructType *CreateStatsTy()
		{
			vector<const Type *> statsFields;
			statsFields.push_back(Type::Int32Ty); // visitedInnerNodes
			statsFields.push_back(Type::Int32Ty); // visitedLeaves
			statsFields.push_back(Type::Int32Ty); // intersectedTriangles
			statsFields.push_back(Type::Int32Ty); // splitNodes
			StructType *statsTy = StructType::get(statsFields, true);
			module.addTypeName("stats", statsTy);
			return statsTy;
		}

		inline static StructType *CreateRayTy(Type *float3Ty, Type *hitTy, Type *statsTy, Type *ptrToFloat4Ty)
		{
			// TODO: account for alignment when shuffling of ray data is enabled
			vector<const Type *> rayFields;
			rayFields.push_back(hitTy); 		// hit
			rayFields.push_back(float3Ty); 		// origin
			rayFields.push_back(float3Ty); 		// direction
			rayFields.push_back(statsTy); 		// staticStats
			rayFields.push_back(statsTy); 		// dynamicStats
			rayFields.push_back(ptrToFloat4Ty); 	// user.data as float4 *
			StructType *rayTy = StructType::get(rayFields, true);
			module.addTypeName("ray", rayTy);
			return rayTy;
		}

		inline static FunctionType *CreateSampleShaderFTy(Type *ptrToShaderObjTy, Type *ptrToPtrToRayTy, Type *ptrToFloat3Ty, Type *ptrToFloat2Ty)
		{
			vector<const Type *> sampleShaderArgs;
			sampleShaderArgs.push_back(ptrToShaderObjTy); 	// const RtShader *obj
			sampleShaderArgs.push_back(ptrToPtrToRayTy); 	// rapido::ray **rays
			sampleShaderArgs.push_back(Type::Int32Ty);	// int count
			sampleShaderArgs.push_back(ptrToFloat3Ty); 	// const float3 *normals
			sampleShaderArgs.push_back(ptrToFloat2Ty); 	// const float2 *tcs
			sampleShaderArgs.push_back(ptrToFloat3Ty); 	// const float3 *diffuses
			return FunctionType::get(Type::VoidTy, sampleShaderArgs, false);
		}
	}

	static StructType *Float2Ty;
	static StructType *Float3Ty;
	static StructType *Float4Ty;
	static PointerType *PtrToFloat2Ty;
	static PointerType *PtrToFloat3Ty;
	static PointerType *PtrToFloat4Ty;
	static StructType *HitTy;
	static StructType *StatsTy;
	static StructType *RayTy;
	static PointerType *PtrToRayTy;
	static PointerType *PtrToPtrToRayTy;
	static PointerType *PtrToShaderObjTy;

	static FunctionType *SampleShaderFTy;
}

namespace ShaderGlobals
{
	namespace internal
	{
		inline static Function *CreateSampleTexture()
		{
			vector<const Type *> sampleTextureArgs;
			sampleTextureArgs.push_back(ShaderType::PtrToFloat4Ty); 	// float4 *dest
			sampleTextureArgs.push_back(ShaderType::PtrToShaderObjTy); 	// const RtShader *obj
			sampleTextureArgs.push_back(Type::Int32Ty); 			// int texId
			sampleTextureArgs.push_back(ShaderType::PtrToFloat2Ty); 	// const float2 *tcs
			FunctionType *sampleTextureTy = FunctionType::get(Type::VoidTy, sampleTextureArgs, false);
			return new Function(sampleTextureTy, Function::ExternalLinkage, "SampleTexture", &module);
		}

		inline static Function *CreateSampleAnimatedTexture()
		{
			vector<const Type *> sampleAnimatedTextureArgs;
			sampleAnimatedTextureArgs.push_back(ShaderType::PtrToFloat4Ty); 	// float4 *dest
			sampleAnimatedTextureArgs.push_back(ShaderType::PtrToShaderObjTy); 	// const RtShader *obj
			sampleAnimatedTextureArgs.push_back(Type::Int32Ty); 			// int texId
			sampleAnimatedTextureArgs.push_back(Type::FloatTy); 			// float animInterval
			sampleAnimatedTextureArgs.push_back(ShaderType::PtrToFloat2Ty); 	// const float2 *tcs
			FunctionType *sampleAnimatedTextureTy = FunctionType::get(Type::VoidTy, sampleAnimatedTextureArgs, false);
			return new Function(sampleAnimatedTextureTy, Function::ExternalLinkage, "SampleAnimatedTexture", &module);
		}

		inline static Function *CreateSampleSkyBox()
		{
			vector<const Type *> sampleSkyBoxArgs;
			sampleSkyBoxArgs.push_back(ShaderType::PtrToFloat4Ty); 		// float4 *dest
			sampleSkyBoxArgs.push_back(ShaderType::PtrToShaderObjTy); 	// const RtShader *obj
			sampleSkyBoxArgs.push_back(Type::Int32Ty); 			// int texId
			sampleSkyBoxArgs.push_back(ShaderType::PtrToFloat3Ty); 		// const float3 *direction
			FunctionType *sampleSkyBoxTy = FunctionType::get(Type::VoidTy, sampleSkyBoxArgs, false);
			return new Function(sampleSkyBoxTy, Function::ExternalLinkage, "SampleSkyBox", &module);
		}

		inline static Function *CreateSine()
		{
			vector<const Type *> sinfArgs;
			sinfArgs.push_back(Type::FloatTy);
			FunctionType *sinfTy = FunctionType::get(Type::FloatTy, sinfArgs, false);
			return new Function(sinfTy, Function::ExternalLinkage, "sinf", &module);
		}

		inline static Function *CreateCosine()
		{
			vector<const Type *> cosfArgs;
			cosfArgs.push_back(Type::FloatTy);
			FunctionType *cosfTy = FunctionType::get(Type::FloatTy, cosfArgs, false);
			return new Function(cosfTy, Function::ExternalLinkage, "cosf", &module);
		}
#ifdef USE_SINCOS
		inline static Function *CreateSineCosine()
		{
			vector<const Type *> sincosfArgs;
			sincosfArgs.push_back(Type::FloatTy);
			sincosfArgs.push_back(PointerType::getUnqual(Type::FloatTy));
			sincosfArgs.push_back(PointerType::getUnqual(Type::FloatTy));
			FunctionType *sincosfTy = FunctionType::get(Type::VoidTy, sincosfArgs, false);
			return new Function(sincosfTy, Function::ExternalLinkage, "sincosf", &module);
		}
#endif
		inline static Function *CreateGenerateFunction()
		{
			vector<const Type *> generateFunctionArgs;
			generateFunctionArgs.push_back(Type::Int32Ty); // int func
			generateFunctionArgs.push_back(Type::FloatTy); // float t
			FunctionType *generateFunctionTy = FunctionType::get(Type::FloatTy, generateFunctionArgs, false);
			return new Function(generateFunctionTy, Function::ExternalLinkage, "GenerateFunction", &module);
		}

		inline static Function *CreateShadeBackground()
		{
			vector<const Type *> shadeBackgroundArgs;
			shadeBackgroundArgs.push_back(ShaderType::PtrToFloat4Ty); 	// float4 *dest
			shadeBackgroundArgs.push_back(ShaderType::PtrToPtrToRayTy); 	// ray **rays
			shadeBackgroundArgs.push_back(Type::Int32Ty); 			// int count
			FunctionType *shadeBackgroundTy = FunctionType::get(Type::VoidTy, shadeBackgroundArgs, false);
			return new Function(shadeBackgroundTy, Function::ExternalLinkage, "ShadeBackground", &module);
		}
	}

	// references global variable shaderTime in this cpp file
	static Value *Time;

	static Function *SampleTexture;
	static Function *SampleAnimatedTexture;
	static Function *SampleSkyBox;
	static Function *sin;
	static Function *cos;
#ifdef USE_SINCOS
	static Function *sincos;
#endif
	static Function *GenerateFunction;
	static Function *ShadeBackground;
}

static void InitializeCodeGen()
{
	static bool initialized = false;
	if(initialized) return;
	initialized = true;
	
	jitter = ExecutionEngine::create(&module);
	
	// ShaderOptimization
	ShaderOptimization::PassManager = ShaderOptimization::internal::CreatePassManager();
	
	// ShaderType
	ShaderType::Float2Ty = ShaderType::internal::CreateFloat2Ty();
	ShaderType::Float3Ty = ShaderType::internal::CreateFloat3Ty();
	ShaderType::Float4Ty = ShaderType::internal::CreateFloat4Ty();;
	ShaderType::PtrToFloat2Ty = PointerType::getUnqual(ShaderType::Float2Ty);
	ShaderType::PtrToFloat3Ty = PointerType::getUnqual(ShaderType::Float3Ty);
	ShaderType::PtrToFloat4Ty = PointerType::getUnqual(ShaderType::Float4Ty);
	ShaderType::HitTy = ShaderType::internal::CreateHitTy();
	ShaderType::StatsTy = ShaderType::internal::CreateStatsTy();
	ShaderType::RayTy = ShaderType::internal::CreateRayTy(ShaderType::Float3Ty, ShaderType::HitTy, ShaderType::StatsTy, ShaderType::PtrToFloat4Ty);
	ShaderType::PtrToRayTy = PointerType::getUnqual(ShaderType::RayTy);
	ShaderType::PtrToPtrToRayTy = PointerType::getUnqual(ShaderType::PtrToRayTy);
	ShaderType::PtrToShaderObjTy = PointerType::getUnqual(Type::Int32Ty);
	ShaderType::SampleShaderFTy = ShaderType::internal::CreateSampleShaderFTy(ShaderType::PtrToShaderObjTy,
		ShaderType::PtrToPtrToRayTy, ShaderType::PtrToFloat3Ty, ShaderType::PtrToFloat2Ty);
	
	// ShaderGlobals
	ShaderGlobals::Time = new GlobalVariable(Type::FloatTy, false, GlobalValue::ExternalLinkage, 0, "shaderTime", &module);
	ShaderGlobals::SampleTexture = ShaderGlobals::internal::CreateSampleTexture();
	ShaderGlobals::SampleAnimatedTexture = ShaderGlobals::internal::CreateSampleAnimatedTexture();
	ShaderGlobals::SampleSkyBox = ShaderGlobals::internal::CreateSampleSkyBox();
	ShaderGlobals::sin = ShaderGlobals::internal::CreateSine();
	ShaderGlobals::cos = ShaderGlobals::internal::CreateCosine();
#ifdef USE_SINCOS
	ShaderGlobals::sincos = ShaderGlobals::internal::CreateSineCosine();
#endif
	ShaderGlobals::GenerateFunction = ShaderGlobals::internal::CreateGenerateFunction();
	ShaderGlobals::ShadeBackground = ShaderGlobals::internal::CreateShadeBackground();
}

namespace Ast
{
	enum GeneratorFunction { Sine, Triangle, Square, Sawtooth, InvSawtooth, Noise };
	enum BlendMode { Zero, One, SrcColor = 10, InvSrcColor, SrcAlpha, InvSrcAlpha, DestColor = 20, InvDestColor, DestAlpha, InvDestAlpha };
	enum AlphaFunc { Any, Gt0, Lt128, Ge128 };

	// Alpha Generators -----------------------------------------------------------

	struct IAlphaGen
	{
		virtual ~IAlphaGen() { }
		virtual Float GenerateAlpha(const Float &distance) = 0;
	};

	class AlphaGenWave : public IAlphaGen
	{
		GeneratorFunction _func;
		float _base, _amplitude, _phase, _frequency;

	public:
		inline AlphaGenWave(GeneratorFunction func, float base, float amplitude, float phase, float frequency) :
			_func(func), _base(base), _amplitude(amplitude), _phase(phase), _frequency(frequency) { }
		
		Float GenerateAlpha(const Float &distance)
		{
			Float t = Float(ShaderGlobals::Time, true) * _frequency + Float(_phase);
			vector<Value *> args; args.push_back(ConstantInt::get(Type::Int32Ty, _func)); args.push_back(t.value());
			Float gen(builder->CreateCall(ShaderGlobals::GenerateFunction, args.begin(), args.end()), false);
			return saturate(gen * _amplitude + Float(_base));
		}
	};

	class AlphaGenConst : public IAlphaGen
	{
		float _alpha;

	public:
		inline AlphaGenConst(float alpha) : _alpha(alpha) { }
		Float GenerateAlpha(const Float &distance) { return _alpha; }
	};

	class AlphaGenPortal : public IAlphaGen
	{
		float _range;

	public:
		inline AlphaGenPortal(float range) : _range(range) { }
		Float GenerateAlpha(const Float &distance) { return min(1.0f, distance * (1.0f / _range)); }
	};

	// RGB Generators -------------------------------------------------------------

	struct IRgbGen
	{
		virtual ~IRgbGen() { }
		virtual Float3 GenerateColor(Float3 &diffuse) = 0;
	};

	class RgbGenDiffuse : public IRgbGen
	{
		Float3 GenerateColor(Float3 &diffuse) { return diffuse; }
	};

	class RgbGenWave : public IRgbGen
	{
		GeneratorFunction _func;
		float _base, _amplitude, _phase, _frequency;

	public:
		inline RgbGenWave(GeneratorFunction func, float base, float amplitude, float phase, float frequency) :
			_func(func), _base(base), _amplitude(amplitude), _phase(phase), _frequency(frequency) { }

		Float3 GenerateColor(Float3 &diffuse)
		{
			AlphaGenWave gen(_func, _base, _amplitude, _phase, _frequency);
			return Float3(gen.GenerateAlpha(0.0f));
		}
	};

	class RgbGenConst : public IRgbGen
	{
		float3 _color;

	public:
		inline RgbGenConst(const float3 &color) : _color(color) { }
		Float3 GenerateColor(Float3 &diffuse) { return Float3(_color.x, _color.y, _color.z); }
	};

	// Texture Coordinate Generators ----------------------------------------------

	struct ITcGen
	{
		virtual ~ITcGen() { }
		virtual Float2 ComputeCoords(Float3 &normal, Float3 &direction, Float3 &point) = 0;
	};

	class TcGenEnvironment : public ITcGen
	{
		Float2 ComputeCoords(Float3 &normal, Float3 &direction, Float3 &point)
		{
			Float d2 = dot(normal, direction) * -2.0f;
			Float ry = normal.y * d2 - direction.y, rz = normal.z * d2 - direction.z;
			return Float2((Float(0.5f) + ry) * 0.5f, (Float(0.5f) - rz) * 0.5f);
		}
	};

	class TcGenVector : public ITcGen
	{
		float3 _u, _v;

	public:
		inline TcGenVector(const float3 &u, const float3 &v) : _u(u), _v(v) { }

		Float2 ComputeCoords(Float3 &normal, Float3 &direction, Float3 &point)
		{
			return Float2(dot(point, Float3(_u.x, _u.y, _u.z)), dot(point, Float3(_v.x, _v.y, _v.z)));
		}
	};

	// Texture Coordinate Modifiers -----------------------------------------------

	struct ITcMod
	{
		virtual ~ITcMod() { }
		virtual Float2 ComputeCoords(Float2 &tc, Float3 &point) = 0;
	};

	class TcModRotate : public ITcMod
	{
		float _rate;

	public:
		inline TcModRotate(float rate) : _rate(rate) { }

		Float2 ComputeCoords(Float2 &tc, Float3 &point)
		{
			Float t = Float(ShaderGlobals::Time, true) * -_rate;
#ifdef USE_SINCOS
			Float cost, sint;
			vector<Value *> args;
			args.push_back(t.value());
			args.push_back(cost.address(false));
			args.push_back(sint.address(false));
			builder->CreateCall(ShaderGlobals::sincos, args.begin(), args.end());
#else
			Float cost(builder->CreateCall(ShaderGlobals::cos, t.value()), false);
			Float sint(builder->CreateCall(ShaderGlobals::sin, t.value()), false);
#endif
			return Float2(cost * tc.x - sint * tc.y + (Float(1) - cost + sint) + 0.5f, sint * tc.x + cost * tc.y + (Float(1) - sint - cost) * 0.5f);
		}
	};

	class TcModScale : public ITcMod
	{
		float2 _scale;

	public:
		inline TcModScale(const float2 &scale) : _scale(scale) { }
		Float2 ComputeCoords(Float2 &tc, Float3 &point) { return Float2(tc.x * _scale.x, tc.y * _scale.y); }
	};

	class TcModScroll : public ITcMod
	{
		float2 _rate;

	public:
		TcModScroll(const float2 &rate) : _rate(rate) { }

		Float2 ComputeCoords(Float2 &tc, Float3 &point)
		{
			Float time(ShaderGlobals::Time, true);
			return Float2(tc.x + time * _rate.x, tc.y + time * _rate.y);
		}
	};

	class TcModStretch : public ITcMod
	{
		GeneratorFunction _func;
		float _base, _amplitude, _phase, _frequency;

	public:
		inline TcModStretch(GeneratorFunction func, float base, float amplitude, float phase, float frequency) :
			_func(func), _base(base), _amplitude(amplitude), _phase(phase), _frequency(frequency) { }

		Float2 ComputeCoords(Float2 &tc, Float3 &point)
		{
			Float t = Float(ShaderGlobals::Time, true) * _frequency + Float(_phase);
			vector<Value *> args; args.push_back(ConstantInt::get(Type::Int32Ty, _func)); args.push_back(t.value());
			Float gen(builder->CreateCall(ShaderGlobals::GenerateFunction, args.begin(), args.end()), false);
			Float p = Float(1.0f) / (gen * _amplitude + Float(_base));
			Float a = Float(0.5f) - Float(0.5f) * p;
			return Float2(tc.x * p + a, tc.y * p + a);
		}
	};

	class TcModTransform : public ITcMod
	{
		float2 _u, _v, _o;

	public:
		inline TcModTransform(const float2 &u, const float2 &v, const float2 &o) : _u(u), _v(v), _o(o) { }

		Float2 ComputeCoords(Float2 &tc, Float3 &point)
		{
			return Float2(dot(tc, Float2(_u.x, _u.y)) + _o.x, dot(tc, Float2(_v.x, _v.y)) + _o.y);
		}
	};

	class TcModTurbulence : public ITcMod
	{
		float _base, _amplitude, _phase, _frequency;

	public:
		inline TcModTurbulence(float base, float amplitude, float phase, float frequency) :
			_base(base), _amplitude(amplitude), _phase(phase), _frequency(frequency) { }

		Float2 ComputeCoords(Float2 &tc, Float3 &point)
		{
			Float3 p = point * ((1.0f / 128.0f) * 0.125f);
			Float t = Float(ShaderGlobals::Time, true) * _frequency + Float(_phase);
			Float sinxz(builder->CreateCall(ShaderGlobals::sin, (t + p.x + p.z).value()), false);
			Float siny(builder->CreateCall(ShaderGlobals::sin, (t + p.y).value()), false);
			return Float2(tc.x + sinxz * _amplitude, tc.y + siny * _amplitude);
		}
	};

	// ----------------------------------------------------------------------------

	class Stage
	{
		std::vector<std::string> _maps;
		float _animInterval;
		bool _clamp;
		BlendMode _source, _dest;

		shared_ptr<IRgbGen> _rgbGen;
		shared_ptr<IAlphaGen> _alphaGen;
		shared_ptr<ITcGen> _tcGen;
		std::vector<shared_ptr<ITcMod> > _tcMods;
		AlphaFunc _func;

		inline Float4 ComputeTextureColor(Value *obj, Float2 &tc)
		{
			Float4 tex;
			int texId = currentShader->AddTexture(_maps[0]);
			vector<Value *> args;
			args.push_back(tex.address(false));
			args.push_back(obj);
			args.push_back(ConstantInt::get(Type::Int32Ty, texId));
			args.push_back(tc.address());
			builder->CreateCall(ShaderGlobals::SampleTexture, args.begin(), args.end());
			return tex;
		}

		inline Float4 ComputeAnimatedTextureColor(Value *obj, Float2 &tc)
		{
			Float4 tex;
			int texId = currentShader->AddAnimatedTexture(_maps);
			vector<Value *> args;
			args.push_back(tex.address(false));
			args.push_back(obj);
			args.push_back(ConstantInt::get(Type::Int32Ty, texId));
			args.push_back(ConstantFP::get(Type::FloatTy, APFloat(_animInterval)));
			args.push_back(tc.address());
			builder->CreateCall(ShaderGlobals::SampleAnimatedTexture, args.begin(), args.end());
			return tex;
		}

		inline Float4 ComputeBlendedColor(BlendMode mode, Float4 &src, Float4 &dest)
		{
			switch(mode)
			{
			default: case One: return src;
			case Zero: return Float4(0.0f);
			case SrcColor: return src * src;
			case InvSrcColor: return src * (1.0f - src);
			case SrcAlpha: return src * src.w;
			case InvSrcAlpha: return src * (Float(1.0f) - src.w);
			case DestColor: return src * dest;
			case InvDestColor: return src * (1.0f - dest);
			case DestAlpha: return src * dest.w;
			case InvDestAlpha: return src * (Float(1.0f) - dest.w);
			}
		}

	public:
		inline Stage() : _source(One), _dest(Zero), _func(Any) { }
		inline void SetMap(const std::string &name, bool clamp) { _maps.clear(); _maps.push_back(name); _clamp = clamp; }
		inline void SetMaps(const std::vector<std::string> &maps, float interval) { _maps = maps; _clamp = false; _animInterval = interval; }
		inline void SetBlending(BlendMode source, BlendMode dest) { _source = source; _dest = dest; }
		inline void SetRgbGen(IRgbGen *gen) { _rgbGen = shared_ptr<IRgbGen>(gen); }
		inline void SetAlphaGen(IAlphaGen *gen) { _alphaGen = shared_ptr<IAlphaGen>(gen); }
		inline void SetTcGen(ITcGen *gen) { _tcGen = shared_ptr<ITcGen>(gen); }
		inline void AddTcMod(ITcMod *mod) { if(mod != 0) _tcMods.push_back(shared_ptr<ITcMod>(mod)); }
		inline void SetAlphaFunc(AlphaFunc func) { _func = func; }

		Float4 BuildColor(Value *obj, Float3 &normal, Float2 &tc, Float3 &diffuse,
			Float3 &origin, Float3 &direction, Float &distance, Float3 &point, Float4 &background)
		{
			if(_source == Zero && _dest == Zero)
				return Float4(0.0f);

			Float4 color = background;
			if(_maps.size() > 0)
			{
				if(_maps[0] == "$whiteimage")
					color = Float4(1.0f);
				else if(_maps[0] == "$lightmap")
					color = Float4(diffuse, 1.0f);
				else
				{
					// generate texture coordinates
					Float2 texc((_tcGen != 0) ? _tcGen->ComputeCoords(normal, direction, point) : tc);
					for(vector<shared_ptr<ITcMod> >::const_iterator it = _tcMods.begin(); it != _tcMods.end(); ++it)
						texc = (*it)->ComputeCoords(texc, point);
					if(_clamp) texc = saturate(texc);
					// sample the texture(s)
					if(_maps.size() > 1) color = ComputeAnimatedTextureColor(obj, texc);
					else color = ComputeTextureColor(obj, texc);
				}
			}

			// modulate the color
			if(_rgbGen != 0 || _alphaGen != 0)
			{
				color = Float4((_rgbGen != 0) ? (_rgbGen->GenerateColor(diffuse) * Float3(color)) : Float3(color),
					(_alphaGen != 0) ? (_alphaGen->GenerateAlpha(distance) * color.w) : color.w);
			}

			// blend color with background

			// transform SrcColor -> DestColor and vice versa
			// SrcColor = 10, InvSrcColor, SrcAlpha, InvSrcAlpha, DestColor = 20, InvDestColor, DestAlpha, InvDestAlpha 
			BlendMode moddest = _dest;
			if(moddest >= 20) moddest = static_cast<BlendMode>(static_cast<int>(moddest) - 10);
			else if(moddest >= 10) moddest = static_cast<BlendMode>(static_cast<int>(moddest) + 10);

			if(_source != Zero)
			{
				Float4 s = ComputeBlendedColor(_source, color, background);
				if(_dest != Zero)
					color = s + ComputeBlendedColor(moddest, background, color);
				else
					color = s;
			}
			else
				color = ComputeBlendedColor(moddest, background, color);

			// alpha testing
			if(_func != Any)
			{
				Value *zero = ConstantFP::get(Type::FloatTy, APFloat(0.0f));
				Value *half = ConstantFP::get(Type::FloatTy, APFloat(0.5f));

				Value *condition;
				switch(_func)
				{
				case Gt0: condition = builder->CreateFCmpOGT(color.w.value(), zero); break;
				case Lt128: condition = builder->CreateFCmpOLT(color.w.value(), half); break;
				case Ge128: condition = builder->CreateFCmpOGE(color.w.value(), half); break;
				default: case Any: condition = 0; break; // shut up the compiler
				}

				return Float4(builder->CreateSelect(condition, color.address(), background.address()));
			}
			else
				return color;
		}
	};

	// ----------------------------------------------------------------------------

	class Shader
	{
		struct { bool valid; float cloudHeight; std::string box; } _sky;
		struct { bool valid; float3 color; float opaqueDist; } _fog;
		std::vector<Stage> _stages;

		inline Float4 ComputeSkyBox(Value *obj, Float3 &direction)
		{
			Float4 sky;
			int texId = currentShader->AddSkyBox(_sky.box);
			vector<Value *> args;
			args.push_back(sky.address(false));
			args.push_back(obj);
			args.push_back(ConstantInt::get(Type::Int32Ty, texId));
			args.push_back(direction.address());
			builder->CreateCall(ShaderGlobals::SampleSkyBox, args.begin(), args.end());
			return sky;
		}

		inline Float2 ComputeSkyTexCoords(Float3 &direction)
		{
			const float baseRadius = 8192.0f;
			const float t = (float)-sqrt(4.0f * baseRadius * baseRadius) * 0.5f / (4.0f * baseRadius);
			float s = t * _sky.cloudHeight / baseRadius;

			// create a skydome
			// cloudscale = t + (s - t) * direction.z
			Value *cloudscale = builder->CreateAdd(ConstantFP::get(Type::FloatTy, APFloat(t)),
				builder->CreateMul(ConstantFP::get(Type::FloatTy, APFloat(s - t)), direction.z.value()),
				"cloudscale");

			return Float2(direction.x * Float(cloudscale, false), direction.y * Float(cloudscale, false));
		}

	public:
		inline Shader() { _sky.valid = false; _fog.valid = false; }
		inline void SetSky(float cloudHeight, const std::string &box) { _sky.valid = true; _sky.cloudHeight = cloudHeight; _sky.box = box; }
		inline void SetFog(const float3 &color, float opaqueDist) { _fog.valid = true; _fog.color = color; _fog.opaqueDist = opaqueDist; }
		inline void AddStage(const Stage &stage) { _stages.push_back(stage); }

		Float4 ComputeRayColor(Value *obj, Float3 &normal, Float2 &tc, Float3 &diffuse,
			Float3 &origin, Float3 &direction, Float &distance, Float3 &point, Float4 &background)
		{
			if(_fog.valid)
			{
				BasicBlock *foggybb = new BasicBlock("foggy", currentFunction);
				BasicBlock *fogsolidbb = new BasicBlock("fogsolid", currentFunction);
				BasicBlock *fogendbb = new BasicBlock("fogend", currentFunction);

				PHINode *select = builder->CreatePHI(ShaderType::Float4Ty);

				// if(fogOpacity >= 1.0f) skip color computation and return fogcolor;
				Float fogOpacity = distance * (1.0f / _fog.opaqueDist);
				Value *condition = builder->CreateFCmpOLT(fogOpacity.value(), Float(1.0f).value());
				builder->CreateCondBr(condition, foggybb, fogsolidbb); 

				// compute color and lerp with fogcolor depending on the fog's opacity
				builder->SetInsertPoint(foggybb);				
				Float4 color((_sky.valid && _sky.box.size() > 0) ? ComputeSkyBox(obj, direction) : background);
				Float2 texc(_sky.valid ? ComputeSkyTexCoords(direction) : tc);
				for(vector<Ast::Stage>::iterator it = _stages.begin(); it != _stages.end(); ++it)
					color = it->BuildColor(obj, normal, texc, diffuse, origin, direction, distance, point, color);
				color = lerp(color, Float4(_fog.color.x, _fog.color.y, _fog.color.z, 1.0f), fogOpacity);
				select->addIncoming(color.address(), foggybb);
				builder->CreateBr(fogendbb);

				// fog is solid
				builder->SetInsertPoint(fogsolidbb);				
				Float4 fogcolor(_fog.color.x, _fog.color.y, _fog.color.z, 1.0f);
				select->addIncoming(fogcolor.address(), fogsolidbb);
				builder->CreateBr(fogendbb);
				
				// return color depending on path taken
				builder->SetInsertPoint(fogendbb);
				return Float4(select, true);
			}
			else
			{
				Float4 color((_sky.valid && _sky.box.size() > 0) ? ComputeSkyBox(obj, direction) : background);
				Float2 texc(_sky.valid ? ComputeSkyTexCoords(direction) : tc);
				for(vector<Ast::Stage>::iterator it = _stages.begin(); it != _stages.end(); ++it)
					color = it->BuildColor(obj, normal, texc, diffuse, origin, direction, distance, point, color);
				return color;
			}
		}
	};

	// Derivatin of the Abstract Syntax Tree from a shader_t Structure ----

	static void GetBlendingModesFromStateBits(BlendMode &src, BlendMode &dest, unsigned int stateBits)
	{
		switch(stateBits & GLS_SRCBLEND_BITS)
		{
		case GLS_SRCBLEND_ZERO: src = Zero; break;
		default: case GLS_SRCBLEND_ONE: src = One; break;
		case GLS_SRCBLEND_DST_COLOR: src = DestColor; break;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR: src = InvDestColor; break;
		case GLS_SRCBLEND_SRC_ALPHA: src = SrcAlpha; break;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA: src = InvSrcAlpha; break;
		case GLS_SRCBLEND_DST_ALPHA: src = DestAlpha; break;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA: src = InvDestAlpha; break;
		}

		switch(stateBits & GLS_DSTBLEND_BITS)
		{
		default: case GLS_DSTBLEND_ZERO: dest = Zero; break;
		case GLS_DSTBLEND_ONE: dest = One; break;
		case GLS_DSTBLEND_SRC_COLOR: dest = SrcColor; break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR: dest = InvSrcColor; break;
		case GLS_DSTBLEND_SRC_ALPHA: dest = SrcAlpha; break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA: dest = InvSrcAlpha; break;
		case GLS_DSTBLEND_DST_ALPHA: dest = DestAlpha; break;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA: dest = InvDestAlpha; break;
		}
	}

	static AlphaFunc GetAlphaFuncFromStateBits(unsigned int stateBits)
	{
		switch(stateBits & GLS_ATEST_BITS)
		{
		case GLS_ATEST_GT_0: return Gt0;
		case GLS_ATEST_LT_80: return Lt128;
		case GLS_ATEST_GE_80: return Ge128;
		default: return Any;
		}
	}

	static GeneratorFunction GetGeneratorFunction(genFunc_t func)
	{
		switch(func)
		{
		default: case GF_NONE: return Noise;
		case GF_SIN: return Sine;
		case GF_SQUARE: return Square;
		case GF_TRIANGLE: return Triangle;
		case GF_SAWTOOTH: return Sawtooth;
		case GF_INVERSE_SAWTOOTH: return InvSawtooth;
		case GF_NOISE: return Noise;
		}
	}

	static IRgbGen *CreateRgbGen(colorGen_t rgbGen, const waveForm_t &rgbWave, const byte *constantColor)
	{
		switch(rgbGen)
		{
		default: case CGEN_BAD: return 0;
		case CGEN_IDENTITY_LIGHTING: return 0; // not supported
		case CGEN_IDENTITY: return 0;
		case CGEN_ENTITY: return 0; // not supported
		case CGEN_ONE_MINUS_ENTITY: return 0; // not supported

		// vertex colors are not yet supported, but they're used for diffuse lighting anyways
		case CGEN_EXACT_VERTEX: return new RgbGenDiffuse();
		case CGEN_VERTEX: return new RgbGenDiffuse();
		case CGEN_ONE_MINUS_VERTEX: return 0; // not supported

		case CGEN_WAVEFORM: return new RgbGenWave(GetGeneratorFunction(rgbWave.func), rgbWave.base, rgbWave.amplitude, rgbWave.phase, rgbWave.frequency);
		case CGEN_LIGHTING_DIFFUSE: return new RgbGenDiffuse();
		case CGEN_FOG: return 0; // not supported
		case CGEN_CONST: return new RgbGenConst(float3(constantColor[0], constantColor[1], constantColor[2]) * (1.0f / 255.f));
		}
	}

	static IAlphaGen *CreateAlphaGen(alphaGen_t alphaGen, const waveForm_t &alphaWave, byte constantAlpha, float portalRange)
	{
		switch(alphaGen)
		{
		default: case AGEN_SKIP: return 0;
		case AGEN_IDENTITY: return 0;
		case AGEN_ENTITY: return 0; // not supported
		case AGEN_ONE_MINUS_ENTITY: return 0; // not supported
		case AGEN_VERTEX: return 0; // not supported
		case AGEN_ONE_MINUS_VERTEX: return 0; // not supported
		case AGEN_LIGHTING_SPECULAR: return 0; // not supported
		case AGEN_WAVEFORM: return new AlphaGenWave(GetGeneratorFunction(alphaWave.func), alphaWave.base, alphaWave.amplitude, alphaWave.phase, alphaWave.frequency);
		case AGEN_PORTAL: return new AlphaGenPortal(portalRange);
		case AGEN_CONST: return new AlphaGenConst(constantAlpha * (1.0f / 255.0f));
		}
	}

	static ITcGen *CreateTcGen(texCoordGen_t tcGen, const vec3_t *tcGenVectors)
	{
		switch(tcGen)
		{
		default: case TCGEN_BAD: return 0;
		case TCGEN_IDENTITY: return 0; // not supported
		case TCGEN_LIGHTMAP: return 0; // not supported
		case TCGEN_TEXTURE: return 0; // not supported
		case TCGEN_ENVIRONMENT_MAPPED: return new TcGenEnvironment;
		case TCGEN_FOG: return 0; // not supported
		case TCGEN_VECTOR: return new TcGenVector(*reinterpret_cast<const float3 *>(tcGenVectors[0]), *reinterpret_cast<const float3 *>(tcGenVectors[1]));
		}
	}

	static ITcMod *CreateTcMod(const texModInfo_t &tcMod)
	{
		switch(tcMod.type)
		{
		default: case TMOD_NONE: return 0;
		case TMOD_TRANSFORM: return new TcModTransform(float2(tcMod.matrix[0][0], tcMod.matrix[0][1]), float2(tcMod.matrix[1][0], tcMod.matrix[1][1]), float2(tcMod.translate[0], tcMod.translate[1]));
		case TMOD_TURBULENT: return new TcModTurbulence(tcMod.wave.base, tcMod.wave.amplitude, tcMod.wave.phase, tcMod.wave.frequency);
		case TMOD_SCROLL: return new TcModScroll(float2(tcMod.scroll[0], tcMod.scroll[1]));
		case TMOD_SCALE: return new TcModScale(float2(tcMod.scale[0], tcMod.scale[1]));
		case TMOD_STRETCH: return new TcModStretch(GetGeneratorFunction(tcMod.wave.func), tcMod.wave.base, tcMod.wave.amplitude, tcMod.wave.phase, tcMod.wave.frequency);
		case TMOD_ROTATE: return new TcModRotate(tcMod.rotateSpeed);
		case TMOD_ENTITY_TRANSLATE: return 0; // not supported
		}
	}

	Shader Create(const shader_t *shader)
	{
		Shader s;

		if(shader->isSky)
		{
			if(shader->sky.outerbox[0] != 0)
			{

				char skytex[MAX_QPATH];
				strncpy(skytex, shader->sky.outerbox[0]->imgName, sizeof(skytex));
				char *ext = strrchr(skytex, '_'); if(ext != 0) *ext = 0; // strip the extension
				s.SetSky(shader->sky.cloudHeight, skytex);
			}
			else
				s.SetSky(shader->sky.cloudHeight, "");
		}

		if(shader->contentFlags & CONTENTS_FOG)
			s.SetFog(*reinterpret_cast<const float3 *>(shader->fogParms.color), shader->fogParms.depthForOpaque);

		for(int i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			const shaderStage_t *stage = shader->stages[i];
			if(stage == 0) break;
			if(!stage->active) continue;

			for(int b = 0; b < NUM_TEXTURE_BUNDLES; ++b)
			{
				const textureBundle_t &bundle = stage->bundle[b];
				if(bundle.image[0] == 0) break;

				Stage st;

				// texture maps
				if(bundle.numImageAnimations > 0)
				{
					vector<string> maps;
					for(int j = 0; j < bundle.numImageAnimations; ++j)
						maps.push_back(bundle.image[j]->imgName);
					st.SetMaps(maps, bundle.imageAnimationSpeed);
				}
				else
				{
					if(!strcmp(bundle.image[0]->imgName, "*white"))
						st.SetMap("$whiteimage", bundle.image[0]->wrapClampMode == GL_CLAMP);
					else if(strstr(bundle.image[0]->imgName, "*lightmap") != 0)
						st.SetMap("$lightmap", bundle.image[0]->wrapClampMode == GL_CLAMP);
					else
						st.SetMap(bundle.image[0]->imgName, bundle.image[0]->wrapClampMode == GL_CLAMP);
				}

				// texture coordinate generation
				ITcGen *tcGen = CreateTcGen(bundle.tcGen, bundle.tcGenVectors);
				if(tcGen != 0) st.SetTcGen(tcGen);

				// texture coordinate modification
				for(int j = 0; j < bundle.numTexMods; ++j)
				{
					ITcMod *tcMod = CreateTcMod(bundle.texMods[j]);
					if(tcMod != 0) st.AddTcMod(tcMod);
				}

				if(b == 1)
				{
					BlendMode src, dest;
					switch(shader->multitextureEnv)
					{
					default: case GL_REPLACE: src = One; dest = Zero; break;
					case GL_MODULATE: src = DestColor; dest = Zero; break;
					case GL_ADD: src = One; dest = One; break;
					}
					st.SetBlending(src, dest);
				}
				else
				{
					// alpha testing
					st.SetAlphaFunc(GetAlphaFuncFromStateBits(stage->stateBits));

					// blending mode
					BlendMode src, dest;
					GetBlendingModesFromStateBits(src, dest, stage->stateBits);
					st.SetBlending(src, dest);

					// color generation
					IRgbGen *rgbGen = CreateRgbGen(stage->rgbGen, stage->rgbWave, stage->constantColor);
					if(rgbGen != 0) st.SetRgbGen(rgbGen);

					// alpha generation
					IAlphaGen *alphaGen = CreateAlphaGen(stage->alphaGen, stage->alphaWave, stage->constantColor[3], shader->portalRange);
					if(alphaGen != 0) st.SetAlphaGen(alphaGen);
				}

				s.AddStage(st);
			}
		}
		
		return s;
	}
}

static class PerlinNoise
{
	float _noiseTable[256];
	unsigned char _noisePerm[256];

	inline int Permutate(int x) const { return _noisePerm[x & 255]; }
	inline float GetNoiseValue(int x, int y, int z, int t) const { return _noiseTable[Permutate(x + Permutate(y + Permutate(z + Permutate(t))))]; }

public:
	PerlinNoise()
	{
		srand(1809);

		const double InvRandMax = 1.0f / RAND_MAX;
		for(int i = 0; i < 256; ++i)
		{
			_noiseTable[i] = static_cast<float>(rand() * InvRandMax * 2.0 - 1.0);
			_noisePerm[i] = static_cast<unsigned char>(rand() * InvRandMax * 255.0);
		}
	}

	float GetNoise(const float4 &v) const
	{
		int ix = static_cast<int>(floor(v.x));
		float fx = v.x - ix;
		int iy = static_cast<int>(floor(v.y));
		float fy = v.y - iy;
		int iz = static_cast<int>(floor(v.z));
		float fz = v.z - iz;
		int it = static_cast<int>(floor(v.w));
		float ft = v.w - it;

		float value[2];
		for(int i = 0; i < 2; ++i)
		{
			const float front[] = { GetNoiseValue(ix, iy, iz, it + i), GetNoiseValue(ix + 1, iy, iz, it + i),
				GetNoiseValue(ix, iy + 1, iz, it + i), GetNoiseValue(ix + 1, iy + 1, iz, it + i) };
			const float back[] = { GetNoiseValue(ix, iy, iz + 1, it + i), GetNoiseValue(ix + 1, iy, iz + 1, it + i),
				GetNoiseValue(ix, iy + 1, iz + 1, it + i), GetNoiseValue(ix + 1, iy + 1, iz + 1, it + i) };
			const float fvalue = lerp(lerp(front[0], front[1], fx), lerp(front[2], front[3], fx), fy);
			const float bvalue = lerp(lerp(back[0], back[1], fx), lerp(back[2], back[3], fx), fy);
			value[i] = lerp(fvalue, bvalue, fz);
		}
		return lerp(value[0], value[1], ft);
	}

	inline float GetNoise(float f) const
	{
		const int it = static_cast<int>(floor(f)); const float ft = f - it;
		return lerp(GetNoiseValue(0, 0, 0, it), GetNoiseValue(0, 0, 0, it + 1), ft);
	}

} PerlinNoiseMaker;

extern "C" float GenerateFunction(int func, float t)
{
	switch(func)
	{
	case Ast::Sine: return sinf(t);
	case Ast::Triangle: return (1.0f - fabs(t - floorf(t) - 0.5f));
	case Ast::Square: return sign(sinf(t));
	case Ast::Sawtooth: return (t - floor(t));
	case Ast::InvSawtooth: return (1.0f - (t - floorf(t)));
	default: case Ast::Noise: return PerlinNoiseMaker.GetNoise(t);
	}
}

static CompiledShader BuildShader(const string &ident, const shader_t *shader)
{
	InitializeCodeGen();
	
	Function *sample = new Function(ShaderType::SampleShaderFTy, Function::ExternalLinkage, ident.c_str(), &module);
	Function::arg_iterator args = sample->arg_begin();
	Value *obj(args++), *rays = args++, *count = args++;
	obj->setName("obj"); rays->setName("rays"); count->setName("count");
	Value *normals = args++, *tcs = args++, *diffuses = args++;
	normals->setName("normals"); tcs->setName("tcs"); diffuses->setName("diffuses");

	BasicBlock *entrybb = new BasicBlock("entry", sample);
	LLVMFoldingBuilder builderinst(entrybb);

	CompiledShader newcomp;
	newcomp.requirementsMask = 0;

	// initialize globals, this way we don't have to pass the referenced objects to each and every item
	builder = &builderinst;
	currentFunction = sample;
	currentShader = &newcomp;

	// initialize the index and jump to the loop head
	Value *idxPtr = builder->CreateAlloca(Type::Int32Ty);
	builder->CreateStore(ConstantInt::get(APInt(32, 0)), idxPtr);
	BasicBlock *backgroundbb = new BasicBlock("tracebackground", sample);
	builder->CreateBr(backgroundbb);

	// determine the background color, this block will be erased automatically during optimization if the background is not required
	builder->SetInsertPoint(backgroundbb);
	Value *backgrounds = builder->CreateAlloca(ShaderType::Float4Ty, count);
	backgrounds->setName("backgrounds");

	// invoke the raytracer to determine the background color for all rays at once
	vector<Value *> bgargs;
	bgargs.push_back(backgrounds);
	bgargs.push_back(rays);
	bgargs.push_back(count);
	builder->CreateCall(ShaderGlobals::ShadeBackground, bgargs.begin(), bgargs.end());

	BasicBlock *headbb = new BasicBlock("head", sample);
	builder->CreateBr(headbb);

	// if idx < count goto body else goto tail
	builder->SetInsertPoint(headbb);
	BasicBlock *bodybb = new BasicBlock("body", sample), *tailbb = new BasicBlock("tail", sample);
	Value *idx = builder->CreateLoad(idxPtr, "idx");
	builder->CreateCondBr(builder->CreateICmpSLT(idx, count), bodybb, tailbb);

	// we're in the loop's body now
	builder->SetInsertPoint(bodybb);
	{
		Value *rayAdr = builder->CreateGEP(rays, idx);
		Value *rayPtr = builder->CreateLoad(rayAdr, "ray");

		Float3 normal(builder->CreateGEP(normals, idx, "normal"));
		Float2 tc(builder->CreateGEP(tcs, idx, "tc"));
		Float3 diffuse(builder->CreateGEP(diffuses, idx, "diffuse"));
		Float3 origin(builder->CreateStructGEP(rayPtr, 1, "origin"));
		Float3 direction(builder->CreateStructGEP(rayPtr, 2, "direction"));
		Float distance(builder->CreateLoad(builder->CreateStructGEP(builder->CreateStructGEP(rayPtr, 1), 2), "distance"), false);
		Float3 point = origin + direction * distance;
		Float4 background(builder->CreateGEP(backgrounds, idx, "background"));

		Ast::Shader s = Ast::Create(shader);
		Float4 color = s.ComputeRayColor(obj, normal, tc, diffuse, origin, direction, distance, point, background);

		// write the computed color to the ray's associated color
		Value *dest = builder->CreateLoad(builder->CreateStructGEP(rayPtr, 5), "user.data");
		builder->CreateStore(color.x.value(), builder->CreateStructGEP(dest, 0));
		builder->CreateStore(color.y.value(), builder->CreateStructGEP(dest, 1));
		builder->CreateStore(color.z.value(), builder->CreateStructGEP(dest, 2));
		builder->CreateStore(color.w.value(), builder->CreateStructGEP(dest, 3));

		newcomp.requirementsMask = 0; // compute the requirements mask
		newcomp.requirementsMask |= normal.accessed() ? RtShader::Normals : 0;
		newcomp.requirementsMask |= tc.accessed() ? RtShader::TextureCoordinates : 0;
		newcomp.requirementsMask |= diffuse.accessed() ? RtShader::Diffuses : 0;
	}

	// increment the index and return to the loop head
	builder->CreateStore(builder->CreateAdd(idx, ConstantInt::get(APInt(32, 1))), idxPtr);
	builder->CreateBr(headbb);

	// tail of the loop and of the function: return
	builder->SetInsertPoint(tailbb);
	builder->CreateRetVoid();
#ifdef DUMP_FUNCTIONS_TO_FILE
	ofstream f("functions.txt", ios::app);
	f << "Before optimization:" << endl;
	sample->print(f);
	ShaderOptimization::PassManager->run(*sample); // optimize
	f << endl << "After optimization:" << endl;
	sample->print(f);
	f << endl << endl;
#else
	ShaderOptimization::PassManager->run(*sample); // optimize
#endif
	// execute the jitter and retrieve a function pointer
	newcomp.sampleFunction = reinterpret_cast<RtShader::SampleFunction *>(jitter->getPointerToFunction(sample));
	return newcomp;
}

