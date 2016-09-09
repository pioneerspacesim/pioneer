// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREGL_H
#define _TEXTUREGL_H

#include "../Texture.h"
#include "OpenGLLibs.h"

namespace Graphics {

namespace GL2 {

class GL2Texture : public Texture {
public:
	virtual void Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips);
	virtual void Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips);

	virtual ~GL2Texture();

	void Bind();
	void Unbind();

	virtual void SetSampleMode(TextureSampleMode);
	GLuint GetTexture() const { return m_texture; }

	void BuildMipmaps();

private:
	friend class RendererGL2;
	GL2Texture(const TextureDescriptor &descriptor, const bool useCompressed);

	GLenum m_target;
	GLuint m_texture;
};

}

}

#endif
