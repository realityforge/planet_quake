
#ifndef TR_RTSHADERS_H
#define TR_RTSHADERS_H

#include <floats.h>
#include <rapido.h>
#include "tr_rttextures.h"
#include <string>
#include <vector>

struct RtShader
{
	typedef void SampleFunction(const RtShader *obj, rapido::ray **rays, int count, const float3 *normals, const float2 *tcs, const float3 *diffuses);
	SampleFunction *_sampleFunction;
	int _requirementsMask;
	std::vector<textures::Texture> _textures, _skybox;
	std::vector<std::vector<textures::Texture> > _animatedTextures;

	enum Requirements { None = 0x0, Normals = 0x1, TextureCoordinates = 0x2, Diffuses = 0x4 };

	inline RtShader(SampleFunction *sampleFunction, int requirementsMask) :
		_sampleFunction(sampleFunction), _requirementsMask(requirementsMask)
	{ }

	inline RtShader(SampleFunction *sampleFunction, int requirementsMask, textures::TextureManager *textureManager,
		const std::vector<std::string> &textures, const std::vector<std::vector<std::string> > &animatedTextures, const std::vector<std::string> &skybox) :
		_sampleFunction(sampleFunction), _requirementsMask(requirementsMask)
	{
		// load the textures
		for(std::vector<std::string>::const_iterator it = textures.begin(); it != textures.end(); ++it)
			_textures.push_back(textureManager->CreateFrom(it->c_str()));

		// load the animated textures
		for(std::vector<std::vector<std::string> >::const_iterator it = animatedTextures.begin(); it != animatedTextures.end(); ++it)
		{
			std::vector<textures::Texture> anims;
			for(std::vector<std::string>::const_iterator it2 = it->begin(); it2 != it->end(); ++it2)
				anims.push_back(textureManager->CreateFrom(it2->c_str()));
			_animatedTextures.push_back(anims);
		}

		// load the skyboxes
		for(std::vector<std::string>::const_iterator it = skybox.begin(); it != skybox.end(); ++it)
		{
			static const char *exts[6] = { "ft", "bk", "up", "dn", "rt", "lf" };
			for(int i = 0; i < 6; ++i)
			{
				char name[512];
				snprintf(name, sizeof(name), "%s_%s.tga", it->c_str(), exts[i]);
				_skybox.push_back(textureManager->CreateFrom(name));
			}
		}
	}

	virtual ~RtShader() { }

	inline int GetRequirementsMask() const { return _requirementsMask; }

	inline void Sample(rapido::ray **rays, int count, const float3 *normals, const float2 *tcs, const float3 *diffuses) const
	{
		_sampleFunction(this, rays, count, normals, tcs, diffuses);
	}
};

#endif

