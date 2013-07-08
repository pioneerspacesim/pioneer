// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "vector2.h"
#include "RefCounted.h"

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

class TextureDescriptor {
public:
	TextureDescriptor() :
		format(TEXTURE_RGBA_8888), dataSize(1.0f), texSize(1.0f), sampleMode(LINEAR_CLAMP), generateMipmaps(false), allowCompression(true), numberOfMipMaps(0)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false, bool _allowCompression = true, unsigned int _numberOfMipMaps = 0) :
		format(_format), dataSize(_dataSize), texSize(1.0f), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps), allowCompression(_allowCompression), numberOfMipMaps(_numberOfMipMaps)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, const vector2f &_texSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false, bool _allowCompression = true, unsigned int _numberOfMipMaps = 0) :
		format(_format), dataSize(_dataSize), texSize(_texSize), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps), allowCompression(_allowCompression), numberOfMipMaps(_numberOfMipMaps)
	{}

	const TextureFormat format;
	const vector2f dataSize;
	const vector2f texSize;
	const TextureSampleMode sampleMode;
	const bool generateMipmaps;
	const bool allowCompression;
	const unsigned int numberOfMipMaps;

	void operator=(const TextureDescriptor &o) {
		const_cast<TextureFormat&>(format) = o.format;
		const_cast<vector2f&>(dataSize) = o.dataSize;
		const_cast<vector2f&>(texSize) = o.texSize;
		const_cast<TextureSampleMode&>(sampleMode) = o.sampleMode;
		const_cast<bool&>(generateMipmaps) = o.generateMipmaps;
		const_cast<bool&>(allowCompression) = o.allowCompression;
		const_cast<unsigned int&>(numberOfMipMaps) = o.numberOfMipMaps;
	}
};

class Texture : public RefCounted {
public:
	const TextureDescriptor &GetDescriptor() const { return m_descriptor; }

	// XXX include position
	virtual void Update(const void *data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips = 0) = 0;
	virtual void SetSampleMode(TextureSampleMode) = 0;

	virtual ~Texture() {}

protected:
	Texture(const TextureDescriptor &descriptor) : m_descriptor(descriptor) {}

private:
	TextureDescriptor m_descriptor;
};

}

#endif
