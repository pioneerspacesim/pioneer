// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_UNIFORM_H
#define _OGL_UNIFORM_H
/*
 * Shader uniform
 */
#include "libs.h"
namespace Graphics {

	class Texture;

	namespace OGL {
		class Uniform {
		public:
			Uniform();
			void Init(const char *name, GLuint program);
			void Set(int);
			void Set(float);
			void Set(const vector3f&);
			void Set(const vector3d&);
			void Set(const Color&);
			void Set(const int v[3]);
			void Set(const float x, const float y, const float z, const float w);
			void Set(const float m[9]);
			void Set(const matrix3x3f&);
			void Set(const matrix4x4f&);
			void Set(Texture *t, unsigned int unit);

		//private:
			GLint m_location;
		};
	}
}

#endif
