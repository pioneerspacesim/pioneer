#ifndef _RENDERTEXTURE_H
#define _RENDERTEXTURE_H

#include "libs.h"

namespace Render {

	/*
	 * Holds an OpenGL texture
	 */
	class Texture {
	public:
		Texture();
		Texture(int w, int h, GLint format, GLint internalFormat, GLenum type);
		~Texture();
		virtual void Bind();
		virtual void Unbind();
		GLuint GetGLTexture() const { return m_texture; }
		/*
		 * Draw the texture on screen, w/h in screen percent.
		 * This is only for testing.
		 */
		void Show(float x=0.f, float y=0.f, float w=100.f, float h=100.f);
	protected:
		int m_w;
		int m_h;
		GLuint m_texture;
	};
}

#endif
