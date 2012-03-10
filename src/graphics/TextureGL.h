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

private:
	friend class RendererLegacy;
	friend class RendererGL2;
	TextureGL(const TextureDescriptor &descriptor);

	GLenum m_target;
	GLuint m_texture;
};

}

#endif
