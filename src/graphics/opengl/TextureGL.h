// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREGL_H
#define _TEXTUREGL_H

#include "graphics/Texture.h"

namespace Graphics {
namespace OGL {

class TextureGL : public Texture {
public:
	virtual void Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips);
	virtual void Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips);
	
	TextureGL(const TextureDescriptor &descriptor, const bool useCompressed, const bool useAnisoFiltering);
	virtual ~TextureGL();

	void Bind();
	void Unbind();

	virtual void SetSampleMode(TextureSampleMode);
	virtual void BuildMipmaps();
	GLuint GetTexture() const { return m_texture; }

private:
	GLenum m_target;
	GLuint m_texture;
	const bool m_useAnisoFiltering;
};

}
}

#endif
