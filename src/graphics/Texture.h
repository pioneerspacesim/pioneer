// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "vector2.h"
#include "RefCounted.h"

namespace Graphics {

enum TextureFormat {
	TEXTURE_RGBA,
	TEXTURE_RGB,
	TEXTURE_LUMINANCE_ALPHA, // luminance value put into R,G,B components; separate alpha value
	TEXTURE_INTENSITY,
	TEXTURE_ALPHA
};

enum ImageFormat {
	IMAGE_RGBA,
	IMAGE_RGB,
	IMAGE_LUMINANCE_ALPHA,
	IMAGE_INTENSITY,
	IMAGE_ALPHA
};

enum ImageType {
	IMAGE_UNSIGNED_BYTE,
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
		format(TEXTURE_RGBA), dataSize(1.0f), texSize(1.0f), sampleMode(LINEAR_CLAMP), generateMipmaps(false), allowCompression(true)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false, bool _allowCompression = true) :
		format(_format), dataSize(_dataSize), texSize(1.0f), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps), allowCompression(_allowCompression)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, const vector2f &_texSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false, bool _allowCompression = true) :
		format(_format), dataSize(_dataSize), texSize(_texSize), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps), allowCompression(_allowCompression)
	{}

	const TextureFormat format;
	const vector2f dataSize;
	const vector2f texSize;
	const TextureSampleMode sampleMode;
	const bool generateMipmaps;
	const bool allowCompression;

	void operator=(const TextureDescriptor &o) {
		const_cast<TextureFormat&>(format) = o.format;
		const_cast<vector2f&>(dataSize) = o.dataSize;
		const_cast<vector2f&>(texSize) = o.texSize;
		const_cast<TextureSampleMode&>(sampleMode) = o.sampleMode;
		const_cast<bool&>(generateMipmaps) = o.generateMipmaps;
		const_cast<bool&>(allowCompression) = o.allowCompression;
	}
};

class Texture : public RefCounted {
public:
	const TextureDescriptor &GetDescriptor() const { return m_descriptor; }

	// XXX include position
	virtual void Update(const void *data, const vector2f &dataSize, ImageFormat format, ImageType type) = 0;
	virtual void SetSampleMode(TextureSampleMode) = 0;

	virtual ~Texture() {}

protected:
	Texture(const TextureDescriptor &descriptor) : m_descriptor(descriptor) {}

private:
	TextureDescriptor m_descriptor;
};

}

#endif
