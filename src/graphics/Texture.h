#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "vector2.h"
#include "RefCounted.h"

namespace Graphics {

enum TextureFormat {
	TEXTURE_RGBA,
	TEXTURE_RGB,
};

enum ImageFormat {
	IMAGE_RGBA,
	IMAGE_RGB,
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
		format(TEXTURE_RGBA), dataSize(1.0f), texSize(1.0f), sampleMode(LINEAR_CLAMP), generateMipmaps(false)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false) :
		format(_format), dataSize(_dataSize), texSize(1.0f), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps)
	{}

	TextureDescriptor(TextureFormat _format, const vector2f &_dataSize, const vector2f &_texSize, TextureSampleMode _sampleMode = LINEAR_CLAMP, bool _generateMipmaps = false) :
		format(_format), dataSize(_dataSize), texSize(_texSize), sampleMode(_sampleMode), generateMipmaps(_generateMipmaps)
	{}

	const TextureFormat format;
	const vector2f dataSize;
	const vector2f texSize;
	const TextureSampleMode sampleMode;
	const bool generateMipmaps;

	void operator=(const TextureDescriptor &o) {
		const_cast<TextureFormat&>(format) = o.format;
		const_cast<vector2f&>(dataSize) = o.dataSize;
		const_cast<vector2f&>(texSize) = o.texSize;
		const_cast<TextureSampleMode&>(sampleMode) = o.sampleMode;
		const_cast<bool&>(generateMipmaps) = o.generateMipmaps;
	}
};

class Texture : public RefCounted {
public:
	const TextureDescriptor &GetDescriptor() const { return m_descriptor; }

	// XXX include position
	virtual void Update(const void *data, const vector2f &dataSize, ImageFormat format, ImageType type) = 0;

	virtual ~Texture() {}

protected:
	Texture(const TextureDescriptor &descriptor) : m_descriptor(descriptor) {}

private:
	TextureDescriptor m_descriptor;
};

}

#endif
