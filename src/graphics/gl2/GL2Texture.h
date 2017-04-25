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
	virtual void Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) override final;
	virtual void Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) override final;

	GL2Texture(const TextureDescriptor &descriptor, const bool useCompressed);
	virtual ~GL2Texture();

	virtual void Bind() override final;
	virtual void Unbind() override final;

	virtual void SetSampleMode(TextureSampleMode) override final;
	virtual uint32_t GetTextureID() const override final { assert(sizeof(uint32_t)==sizeof(GLuint)); return m_texture; }

	void BuildMipmaps() override;

private:
	GLenum m_target;
	GLuint m_texture;
};

}

}

#endif
