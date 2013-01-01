// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREGL_H
#define _TEXTUREGL_H

#include "Texture.h"
#include <GL/glew.h>

namespace Graphics {

class TextureGL : public Texture {
public:
	virtual void Update(const void *data, const vector2f &dataSize, ImageFormat format, ImageType type);

	virtual ~TextureGL();

	void Bind();
	void Unbind();

	// XXX for LMR, which can't use the normal Bind methods. remove once all
	// its drawing goes through the renderer
	GLuint GetTextureNum() const { return m_texture; }
	virtual void SetSampleMode(TextureSampleMode);

private:
	friend class RendererLegacy;
	friend class RendererGL2;
	TextureGL(const TextureDescriptor &descriptor, const bool useCompressed);

	GLenum m_target;
	GLuint m_texture;
};

}

#endif
