#ifndef _GRAPHICS_GL2PROGRAM_H
#define _GRAPHICS_GL2PROGRAM_H
/*
 * The new 'Shader' class
 */
#include "libs.h"

namespace Graphics {

	namespace GL2 {

		struct ShaderException { };

		class Program {
		public:
			Program(const std::string &name);
			virtual ~Program();
			void Reload();
			virtual void Use();
			virtual void Unuse();

		protected:
			void LoadShaders(const std::string&);
			virtual void InitUniforms();

		private:
			std::string m_name;
			std::string m_defines;
			GLuint m_program;
		};

	}

}
#endif
