// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREDUMMY_H
#define _TEXTUREDUMMY_H

#include "graphics/Texture.h"

namespace Graphics {

class TextureDummy : public Texture {
public:
	virtual void Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) {}
	virtual void Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) {}

	void Bind() {}
	void Unbind() {}

	virtual void SetSampleMode(TextureSampleMode) {}
	GLuint GetTexture() const { return 0; }

private:
	friend class RendererDummy;
	TextureDummy(const TextureDescriptor &descriptor) : Texture(descriptor) {}
};

}

#endif
