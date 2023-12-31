// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_UNIFORM_H
#define _OGL_UNIFORM_H
/*
 * Shader uniform
 */
#include "OpenGLLibs.h"

#include "Color.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace Graphics {

	class Texture;

	namespace OGL {
		class Uniform {
		public:
			Uniform();
			Uniform(GLint location) :
				m_location(location) {}
			void Init(const char *name, GLuint program);
			void InitBlock(const char *name, GLuint program, GLint binding);
			void Set(int);
			void Set(float);
			void Set(const vector3f &);
			void Set(const vector3d &);
			void Set(const Color &);
			void Set(const Color4f &);
			void Set(const matrix3x3f &);
			void Set(const matrix4x4f &);
			void Set(Texture *t, unsigned int unit);
			bool IsValid() const { return (m_location != -1); }
			GLint Location() const { return m_location; }

		private:
			GLint m_location;
		};
	} // namespace OGL
} // namespace Graphics

#endif
