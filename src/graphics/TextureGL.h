#ifndef _TEXTUREGL_H
#define _TEXTUREGL_H

#include "Texture.h"
#include <GL/glew.h>

namespace Graphics {

class TextureGL : public Texture {
	virtual void Update(const void *data, const vector2f &dataSize, ImageFormat format, ImageType type);

	virtual ~TextureGL();

	void Bind();
	void Unbind();

private:
	friend class RendererLegacy;
	friend class RendererGL2;
	TextureGL(const TextureDescriptor &descriptor);

	GLenum m_target;
	GLuint m_texture;
};

}

#endif
