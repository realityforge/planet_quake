
#ifndef TR_RTTEXTURES_H
#define TR_RTTEXTURES_H

#include <floats.h>
#include <map>
#include <string>
#include <vectors.h>

namespace textures
{
	class Texture
	{
		class ITextureManager *_textureManager;
		std::string _name;
		struct image
		{
			const unsigned int *bits;
			unsigned int width, height, widthMask, heightMask;
		} _img;

		friend class TextureManager;
		Texture(class ITextureManager *textureManager, const std::string &name, const image &img);

	public:
		Texture();
		Texture(const Texture &other);
		Texture &operator=(const Texture &other);
		~Texture();

		const std::string GetName() const { return _name; }

		float4 Sample(const float2 &coords) const;
		vector4 Sample(const vector2 &coords) const;
		float SampleAlpha(const float2 &coords) const;
	};

	class ITextureManager
	{
		friend class Texture;
		virtual void AddRef(const std::string &name) = 0;
		virtual void Release(const std::string &name) = 0;

	public:
		virtual ~ITextureManager() { }

		virtual Texture CreateFrom(const char *name) = 0;
		
		virtual void BeginTransition() = 0;
		virtual void EndTransition() = 0;
	};

	class TextureManager : public ITextureManager
	{
		std::map<std::string, std::pair<int, Texture::image> > _data; // name -> refCount/image
		bool _immediateDeletion;

		friend class Texture;
		void AddRef(const std::string &name);
		void Release(const std::string &name);

		static void ScaleUp(Texture::image &img);

		// copying and assignment are not supported
		TextureManager(const TextureManager &other);
		TextureManager &operator=(const TextureManager &other);

	public:
		TextureManager();
		~TextureManager();

		Texture CreateFrom(const char *name);

		void BeginTransition();
		void EndTransition();
	};
}

#endif // TR_RTTEXTURES_H

