#ifndef _GL2_UNIFORM_H
#define _GL2_UNIFORM_H
/*
 * Shader uniform
 */
#include "libs.h"
namespace Graphics {
	namespace GL2 {
		class Uniform {
		public:
			Uniform();
			void Init(const char *name, GLuint program);
			void Set(int);
			void Set(float);

		private:
			GLint m_location;
		};
	}
}

#endif