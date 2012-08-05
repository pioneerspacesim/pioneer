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
			void Init(const char *name, GLuint program);
			void Set(int);

		private:
			GLuint m_location;
		};
	}
}

#endif