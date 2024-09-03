// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Uniform.h"
#include "TextureGL.h"

namespace Graphics {
	namespace OGL {

		Uniform::Uniform() :
			m_location(-1)
		{
		}

		void Uniform::Init(const char *name, GLuint program)
		{
			m_location = glGetUniformLocation(program, name);
		}

		void Uniform::InitBlock(const char *name, GLuint program, GLint binding)
		{
			m_location = glGetUniformBlockIndex(program, name);
			if (IsValid() && GLuint(m_location) != GL_INVALID_INDEX)
				glUniformBlockBinding(program, m_location, binding);
		}

		void Uniform::Set(int i)
		{
			if (m_location != -1)
				glUniform1i(m_location, i);
		}

		void Uniform::Set(float f)
		{
			if (m_location != -1)
				glUniform1f(m_location, f);
		}

		void Uniform::Set(const vector3f &v)
		{
			if (m_location != -1)
				glUniform3f(m_location, v.x, v.y, v.z);
		}

		void Uniform::Set(const vector3d &v)
		{
			if (m_location != -1)
				glUniform3f(m_location, static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z)); //yes, 3f
		}

		void Uniform::Set(const Color &c)
		{
			Color4f c4f = c.ToColor4f();
			if (m_location != -1)
				glUniform4f(m_location, c4f.r, c4f.g, c4f.b, c4f.a);
		}

		void Uniform::Set(const Color4f &c)
		{
			if (m_location != -1)
				glUniform4f(m_location, c.r, c.g, c.b, c.a);
		}

		/*
		void Uniform::Set(const int v[3])
		{
			if (m_location != -1)
				glUniform3i(m_location, v[0], v[1], v[2]);
		}

		void Uniform::Set(const float x, const float y, const float z, const float w)
		{
			if (m_location != -1)
				glUniform4f(m_location, x, y, z, w);
		}

		void Uniform::Set(const float m[9])
		{
			if (m_location != -1)
				glUniformMatrix3fv(m_location, 1, GL_FALSE, m);
		}
		*/

		void Uniform::Set(const matrix3x3f &m)
		{
			if (m_location != -1)
				glUniformMatrix3fv(m_location, 1, GL_FALSE, &m[0]);
		}

		void Uniform::Set(const matrix4x4f &m)
		{
			// Note for those who might be confused about the GL_FALSE:
			// a row-major matrix stored in row-major order and a column-major
			// matrix stored in column-major order have exactly the same memory
			// layout. X = 0..3, Y = 4..7, Z = 8..11, T = 12..15. Thus, our
			// row-major matricies are perfectly interpreted as OpenGL
			// column-major matricies without any problems.
			if (m_location != -1)
				glUniformMatrix4fv(m_location, 1, GL_FALSE, &m[0]);
		}

		void Uniform::Set(Texture *tex, unsigned int unit)
		{
			if (m_location != -1 && tex) {
				glActiveTexture(GL_TEXTURE0 + unit);
				static_cast<TextureGL *>(tex)->Bind();
				glUniform1i(m_location, unit);
			}
		}

	} // namespace OGL
} // namespace Graphics
