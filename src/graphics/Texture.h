// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "RefCounted.h"
#include "vector2.h"
#include "vector3.h"
#include <vector>

namespace Graphics {

	enum TextureFormat {
		TEXTURE_NONE,

		TEXTURE_RGBA_8888,
		TEXTURE_RGB_888,
		TEXTURE_RG_88,
		TEXTURE_R8,

		TEXTURE_DXT1, // data is expected to be pre-compressed
		TEXTURE_DXT5,

		TEXTURE_DEPTH //precision chosen by renderer
	};

	enum TextureSampleMode {
		LINEAR_CLAMP,
		NEAREST_CLAMP,
		LINEAR_REPEAT,
		NEAREST_REPEAT
	};

	enum TextureType {
		TEXTURE_2D,
		TEXTURE_CUBE_MAP,
		TEXTURE_2D_ARRAY
	};

	struct TextureCubeData {
		void *posX;
		void *negX;
		void *posY;
		void *negY;
		void *posZ;
		void *negZ;
	};

	// WARNING: TextureDescriptor is intended to be immutable. Internal values should not be changed!
	class TextureDescriptor {
	public:
		TextureDescriptor() :
			format(TEXTURE_RGBA_8888),
			dataSize(1.0f),
			texSize(1.0f),
			sampleMode(LINEAR_CLAMP),
			generateMipmaps(false),
			allowCompression(true),
			useAnisotropicFiltering(true),
			numberOfMipMaps(0),
			type(TEXTURE_2D)
		{}

		TextureDescriptor(TextureFormat _format, const vector3f &_dataSize, TextureSampleMode _sampleMode, bool _generateMipmaps, bool _allowCompression, bool _useAnisotropicFiltering, unsigned int _numberOfMipMaps, TextureType _textureType) :
			format(_format),
			dataSize(_dataSize),
			texSize(1.0f),
			sampleMode(_sampleMode),
			generateMipmaps(_generateMipmaps),
			allowCompression(_allowCompression),
			useAnisotropicFiltering(_useAnisotropicFiltering),
			numberOfMipMaps(_numberOfMipMaps),
			type(_textureType)
		{}

		TextureDescriptor(TextureFormat _format, const vector3f &_dataSize, const vector2f &_texSize, TextureSampleMode _sampleMode, bool _generateMipmaps, bool _allowCompression, bool _useAnisotropicFiltering, unsigned int _numberOfMipMaps, TextureType _textureType) :
			format(_format),
			dataSize(_dataSize),
			texSize(_texSize),
			sampleMode(_sampleMode),
			generateMipmaps(_generateMipmaps),
			allowCompression(_allowCompression),
			useAnisotropicFiltering(_useAnisotropicFiltering),
			numberOfMipMaps(_numberOfMipMaps),
			type(_textureType)
		{}

		vector2f GetOriginalSize() const { return vector2f(dataSize.x * texSize.x, dataSize.y * texSize.y); }

		// WARNING: these values shall not be changed
		TextureFormat format;
		vector3f dataSize;
		vector2f texSize;
		TextureSampleMode sampleMode;
		bool generateMipmaps;
		bool allowCompression;
		bool useAnisotropicFiltering;
		unsigned int numberOfMipMaps;
		TextureType type;
	};

	class Texture : public RefCounted {
	public:
		const TextureDescriptor &GetDescriptor() const { return m_descriptor; }

		virtual void Update(const void *data, const vector2f &pos, const vector3f &dataSize, TextureFormat format, const unsigned int numMips = 0) = 0;
		virtual void Update(const void *data, const vector3f &dataSize, TextureFormat format, const unsigned int numMips = 0)
		{
			Update(data, vector2f(0, 0), dataSize, format, numMips);
		}
		virtual void Update(const TextureCubeData &data, const vector3f &dataSize, TextureFormat format, const unsigned int numMips = 0) = 0;
		typedef std::vector<void *> vecDataPtr;
		virtual void Update(const vecDataPtr &data, const vector3f &dataSize, const TextureFormat format, const unsigned int numMips = 0) = 0;
		virtual void SetSampleMode(TextureSampleMode) = 0;
		// Call this function to update the texture's mipmaps.
		// validMips is the number of mipmaps which already have valid data uploaded, and is mostly for internal use.
		virtual void BuildMipmaps(const uint32_t validMips = 1) = 0;
		virtual uint32_t GetTextureID() const = 0;
		virtual uint32_t GetTextureMemSize() const = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual ~Texture() {}

	protected:
		Texture(const TextureDescriptor &descriptor) :
			m_descriptor(descriptor) {}

	private:
		TextureDescriptor m_descriptor;
	};

} // namespace Graphics

#endif
