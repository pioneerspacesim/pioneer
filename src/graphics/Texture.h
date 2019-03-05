// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "RefCounted.h"
#include <vector>
#include "vector2.h"
#include "vector3.h"

namespace Graphics {

	enum TextureFormat {
		TEXTURE_NONE,

		TEXTURE_RGBA_8888,
		TEXTURE_RGB_888,

		//luminance/intensity formats are deprecated in opengl 3+
		//so we might remove them someday
		TEXTURE_LUMINANCE_ALPHA_88, //luminance value put into R,G,B components; separate alpha value
		TEXTURE_INTENSITY_8, //intensity value put into RGBA components

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

		const TextureFormat format;
		const vector3f dataSize;
		const vector2f texSize;
		const TextureSampleMode sampleMode;
		const bool generateMipmaps;
		const bool allowCompression;
		const bool useAnisotropicFiltering;
		const unsigned int numberOfMipMaps;
		const TextureType type;

		TextureDescriptor &operator=(const TextureDescriptor &o)
		{
			const_cast<TextureFormat &>(format) = o.format;
			const_cast<vector3f&>(dataSize) = o.dataSize;
			const_cast<vector2f &>(texSize) = o.texSize;
			const_cast<TextureSampleMode &>(sampleMode) = o.sampleMode;
			const_cast<bool &>(generateMipmaps) = o.generateMipmaps;
			const_cast<bool &>(allowCompression) = o.allowCompression;
			const_cast<bool &>(useAnisotropicFiltering) = o.useAnisotropicFiltering;
			const_cast<unsigned int &>(numberOfMipMaps) = o.numberOfMipMaps;
			const_cast<TextureType &>(type) = o.type;
			return *this;
		}
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
		virtual void BuildMipmaps() = 0;
		virtual uint32_t GetTextureID() const = 0;

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
