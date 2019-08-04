
#include "tr_rttextures.h"
#include <assert.h>
#include <vector>

extern "C"
{
#include "tr_local.h"
}

extern "C" void R_LoadImage(const char *name, byte **pic, int *width, int *height);

namespace textures
{
	inline static vector4 ConvertRawTexel4(const __m128i &c4)
	{
		const __m128i Zero4i = _mm_set1_epi32(0);
		// preshuffle
		__m128i sf1 = _mm_shufflehi_epi16(c4, 141);
		__m128i sf2 = _mm_shufflelo_epi16(sf1, 141); // 14 15 10 11 12 13 08 09 | 06 07 02 03 04 05 00 01
		// expand to 2 epi16 registers
		__m128i v1 = _mm_unpackhi_epi8(sf2, Zero4i); // -- 14 -- 15 -- 10 -- 11 | -- 12 -- 13 -- 08 -- 09
		__m128i v2 = _mm_unpacklo_epi8(sf2, Zero4i); // -- 06 -- 07 -- 02 -- 03 | -- 04 -- 05 -- 00 -- 01
		// expand to 4 epi32 registers
		__m128i v3 = _mm_unpackhi_epi16(v1, Zero4i); // -- -- -- 14 -- -- -- 15 | -- -- -- 10 -- -- -- 11
		__m128i v4 = _mm_unpackhi_epi16(v2, Zero4i); // -- -- -- 06 -- -- -- 07 | -- -- -- 02 -- -- -- 03
		__m128i v5 = _mm_unpacklo_epi16(v1, Zero4i); // -- -- -- 0c -- -- -- 0d | -- -- -- 08 -- -- -- 09
		__m128i v6 = _mm_unpacklo_epi16(v2, Zero4i); // -- -- -- 04 -- -- -- 05 | -- -- -- 00 -- -- -- 01
		// convert to 4 ps registers
		__m128 f1 = _mm_cvtepi32_ps(v3); // -- -- -- 14 -- -- -- 15 | -- -- -- 10 -- -- -- 11
		__m128 f2 = _mm_cvtepi32_ps(v4); // -- -- -- 06 -- -- -- 07 | -- -- -- 02 -- -- -- 03
		__m128 f3 = _mm_cvtepi32_ps(v5); // -- -- -- 12 -- -- -- 13 | -- -- -- 08 -- -- -- 09
		__m128 f4 = _mm_cvtepi32_ps(v6); // -- -- -- 04 -- -- -- 05 | -- -- -- 00 -- -- -- 01
		// shuffle to rrrr, gggg, bbbb
		return vector4(_mm_shuffle_ps(f1, f2, 34), // {  3,  7, 11, 15 }
			_mm_shuffle_ps(f1, f2, 119),       // {  2,  6, 10, 14 }
			_mm_shuffle_ps(f3, f4, 34),        // {  1,  5,  9, 13 }
			_mm_shuffle_ps(f3, f4, 119));      // {  0,  4,  8, 12 }
	}

	inline static quad qand(quad q, int m) { return _mm_and_ps(q.f4, reinterpret_cast<__m128>(_mm_set1_epi32(m))); }

	float4 Texture::Sample(const float2 &coords) const
	{
		float u1 = (coords.x - floor(coords.x)) * _img.width, v1 = (coords.y - floor(coords.y)) * _img.height;
		int x1 = static_cast<int>(u1), y1 = static_cast<int>(v1);
		int x2 = (x1 + 1) & _img.widthMask, y2 = (y1 + 1) & _img.heightMask;
		float dx = u1 - x1, dy = v1 - y1;

		x1 &= _img.widthMask; x2 &= _img.heightMask;

		const unsigned char *rows[2] = { reinterpret_cast<const unsigned char *>(_img.bits) + y1 * _img.width * 4,
			reinterpret_cast<const unsigned char *>(_img.bits) + y2 * _img.width * 4 };
		const unsigned char *texels[4] = { rows[0] + x1 * 4, rows[0] + x2 * 4, rows[1] + x1 * 4, rows[1] + x2 * 4 };
		return lerp(
			lerp(float4(texels[0][0], texels[0][1], texels[0][2], texels[0][3]), float4(texels[1][0], texels[1][1], texels[1][2], texels[1][3]), dx),
			lerp(float4(texels[2][0], texels[2][1], texels[2][2], texels[2][3]), float4(texels[3][0], texels[3][1], texels[3][2], texels[3][3]), dx),
			dy) * (1.0f / 255.0f);
	}

	vector4 Texture::Sample(const vector2 &coords) const
	{
		quad u1 = (coords.x - floorf(coords.x)) * static_cast<float>(_img.width);
		quad v1 = (coords.y - floorf(coords.y)) * static_cast<float>(_img.height);
		quad x1 = floor(u1), y1 = floor(v1);
		const __m128i One4i = _mm_set1_epi32(1);
		quad x2 = qand(_mm_add_epi32(x1.i4, One4i), _img.widthMask);
		quad y2 = qand(_mm_add_epi32(y1.i4, One4i), _img.heightMask);
		quad dx = u1 - _mm_cvtepi32_ps(x1.i4), dy = v1 - _mm_cvtepi32_ps(y1.i4);

		x1 = qand(x1.i4, _img.widthMask);
		y1 = qand(y1.i4, _img.heightMask);

		quad raw[4];
		for(int i = 0; i < 4; ++i)
		{
			const unsigned int *rows[2] = { _img.bits + y1.i[i] * _img.width, _img.bits + y2.i[i] * _img.width };
			raw[0].i[3-i] = rows[0][x1.i[i]];
			raw[1].i[3-i] = rows[0][x2.i[i]];
			raw[2].i[3-i] = rows[1][x1.i[i]];
			raw[3].i[3-i] = rows[1][x2.i[i]];
		}

		vector4 texels[4] = { ConvertRawTexel4(raw[0].i4), ConvertRawTexel4(raw[1].i4),
			ConvertRawTexel4(raw[2].i4), ConvertRawTexel4(raw[3].i4) };
		return lerp(lerp(texels[0], texels[1], dx), lerp(texels[2], texels[3], dx), dy) * (1.0f / 255.0f);
	}

	float Texture::SampleAlpha(const float2 &coords) const
	{
		float u1 = (coords.x - floor(coords.x)) * _img.width, v1 = (coords.y - floor(coords.y)) * _img.height;
		int x1 = static_cast<int>(u1), y1 = static_cast<int>(v1);
		int x2 = (x1 + 1) & _img.widthMask, y2 = (y1 + 1) & _img.heightMask;
		float dx = u1 - x1, dy = v1 - y1;

		x1 &= _img.widthMask; x2 &= _img.heightMask;

		const unsigned char *rows[2] = { reinterpret_cast<const unsigned char *>(_img.bits) + y1 * _img.width * 4,
			reinterpret_cast<const unsigned char *>(_img.bits) + y2 * _img.width * 4 };
		const unsigned char *texels[4] = { rows[0] + x1 * 4, rows[0] + x2 * 4, rows[1] + x1 * 4, rows[1] + x2 * 4 };
		return lerp(lerp(texels[0][3], texels[1][3], dx), lerp(texels[2][3], texels[3][3], dx), dy) * (1.0f / 255.0f);
	}

	Texture::Texture() : _textureManager(0), _name("dummy")
	{
		static unsigned int dummy = 0xffff00ff;
		_img.bits = &dummy;
		_img.width = 1; _img.height = 1;
		_img.widthMask = 0x0; _img.heightMask = 0x0;
	}

	Texture::Texture(ITextureManager *textureManager, const std::string &name, const image &img) :
		_textureManager(textureManager), _name(name), _img(img)
	{
		assert(textureManager != 0);
		assert(img.bits != 0);
		_textureManager->AddRef(_name);
	}

	Texture::Texture(const Texture &other):
		_textureManager(other._textureManager), _name(other._name), _img(other._img)
	{
		if(_textureManager) _textureManager->AddRef(_name);
	}

	Texture &Texture::operator=(const Texture &other)
	{
		if(this != &other)
		{
			// reference the new data first, then release the old one
			if(other._textureManager) other._textureManager->AddRef(other._name);
			try { if(_textureManager) _textureManager->Release(_name); }
			catch(...) { if(other._textureManager) other._textureManager->Release(other._name); throw; }
			_textureManager = other._textureManager; _name = other._name; _img = other._img;
		}
		return *this;
	}

	Texture::~Texture()
	{
		if(_textureManager) _textureManager->Release(_name);
	}

	// ----------------------------------------------------------------------------

	TextureManager::TextureManager() :
		_immediateDeletion(true)
	{ }

	TextureManager::~TextureManager()
	{
		assert(_data.size() == 0); // make sure that all textures have been freed, the texturemanager has to be the last to fall! :-)
	}

	// Image Upscaling to the next Power of 2 -------------------------------------
	// Functions are based on the GLFW image upsampling code

	static void UpsampleImage(unsigned char *dest, unsigned int dwidth, unsigned int dheight,
		const unsigned char *src, unsigned int swidth, unsigned int sheight)
	{
		// calculate scaling factor
		float xstep = (swidth - 1) / static_cast<float>(dwidth - 1);
		float ystep = (sheight - 1) / static_cast<float>(dheight - 1);

		// copy source data to destination data with bilinear interpolation
		float dy = 0.0f;
		for(unsigned int n = 0, y = 0; n < dheight; ++n)
		{
			float dx = 0.0f;
			const unsigned char *src1 = src + y * swidth * 4, *src2 = src1 + 4;
			const unsigned char *src3 = (y < (sheight - 1)) ? src1 + swidth * 4 : src1, *src4 = src3 + 4;
			for(unsigned int m = 0, x = 0; m < dwidth; ++m)
			{
				for(int k = 0; k < 4; ++k)
				{
					float col = lerp(lerp(src1[k], src2[k], dx), lerp(src3[k], src4[k], dx), dy);
					*dest++ = static_cast<unsigned char>(min(255, static_cast<unsigned int>(col + 0.5)));
				}

				dx += xstep;
				if(dx >= 1.0f)
				{
					++x;
					dx -= 1.0f;
					if(x >= swidth - 1)
					{
						// clamp to the right border
						src2 = src1;
						src4 = src3;
					}

					// move on pixel to the right
					src1 += 4; src2 += 4; src3 += 4; src4 += 4;
				}
			}

			dy += ystep;
			if(dy >= 1.0f)
			{
				++y;
				dy -= 1.0f;
			}
		}
	}

	inline static unsigned int log2(unsigned int i) { unsigned int l = 0; while(i > 1) { i >>= 1; ++l; } return l; }
	inline static bool ispow2(unsigned int i) { return ((i & (i - 1)) == 0); }

	void TextureManager::ScaleUp(Texture::image &img)
	{
		// calculate next larger power-of-2 dimensions
		unsigned int width = 1 << log2(img.width);
		if(width < img.width) width <<= 1;
		unsigned int height = 1 << log2(img.height);
		if(height < img.height) height <<= 1;
		if(width == img.width && height == img.height)
			return;

		// copy old image data to new image data with interpolation
		unsigned int *newbits = reinterpret_cast<unsigned int *>(malloc(sizeof(unsigned int) * width * height));
		UpsampleImage(reinterpret_cast<unsigned char *>(newbits), width, height,
			reinterpret_cast<const unsigned char *>(img.bits), img.width, img.height);

		free(const_cast<unsigned int *>(img.bits));
		img.bits = newbits;
		img.width = width; img.height = height;
		img.widthMask = img.width - 1; img.heightMask = img.height - 1;
	}

	Texture TextureManager::CreateFrom(const char *name)
	{
		// look for an existing instance
		std::map<std::string, std::pair<int, Texture::image> >::iterator it = _data.find(name);
		if(it != _data.end())
			return Texture(this, it->first, it->second.second);

		// try creating a new texture
		try
		{
			byte *image; int width, height;
			R_LoadImage(name, &image, &width, &height);
			if(image != 0)
			{
				try
				{
					if(width <= 0 || height <= 0)
						throw "Invalid image dimensions!";

					Texture::image img;
					img.width = width; img.height = height;
					img.widthMask = width - 1; img.heightMask = height - 1;
					size_t length = 4 * img.width * img.height;
					unsigned int *bits = reinterpret_cast<unsigned int *>(malloc(length));
					img.bits = bits;
					try
					{
						memcpy(bits, image, length);
						ri.Free(image); image = 0;

						if(!ispow2(img.width) || !ispow2(img.height))
							ScaleUp(img);

						// insert into the map and return a texture
						_data[name] = std::pair<int, Texture::image>(0, img);
						try { return Texture(this, name, img); }
						catch(...) { _data.erase(name); throw; }						
					}
					catch(...) { free(bits); throw; }
				}
				catch(...) {if(image != 0) ri.Free(image); throw; }
			}
		}
		catch(char *ex) { Com_Printf("Couldn't load texture %s due to exception: %s!\n", name, ex); }
		catch(...) { Com_Printf("Couldn't load texture %s!\n", name); }

		// return a dummy texture
		return Texture();
	}

	void TextureManager::BeginTransition()
	{
		_immediateDeletion = false;
	}

	void TextureManager::EndTransition()
	{
		std::vector<std::string> dead; // delete data for images with zero reference count
		for(std::map<std::string, std::pair<int, Texture::image> >::iterator it = _data.begin(); it != _data.end(); ++it)
		{
			if(it->second.first == 0)
			{
				free(const_cast<unsigned int *>(it->second.second.bits));
				dead.push_back(it->first);
			}
		}

		for(std::vector<std::string>::iterator it = dead.begin(); it != dead.end(); ++it)
			_data.erase(*it);

		_immediateDeletion = true;
	}

	void TextureManager::AddRef(const std::string &name)
	{
		std::map<std::string, std::pair<int, Texture::image> >::iterator it = _data.find(name);
		assert(it != _data.end());
		++it->second.first;
	}

	void TextureManager::Release(const std::string &name)
	{
		std::map<std::string, std::pair<int, Texture::image> >::iterator it = _data.find(name);
		assert(it != _data.end());
		--it->second.first;
		if(_immediateDeletion && it->second.first == 0)
		{
			free(const_cast<unsigned int *>(it->second.second.bits));
			_data.erase(it);
		}
	}
}

