#ifndef _GRAPHICS_GL2PROGRAM_H
#define _GRAPHICS_GL2PROGRAM_H
/*
 * The new 'Shader' class
 * This is a base class without specific uniforms
 */
#include "libs.h"
#include "Uniform.h"

namespace Graphics {

	namespace GL2 {

		struct ShaderException { };

		class Program {
		public:
			Program();
			Program(const std::string &name, const std::string &defines);
			virtual ~Program();
			void Reload();
			virtual void Use();
			virtual void Unuse();

			// Some generic uniforms.
			// to be added: matrices etc.
			Uniform invLogZfarPlus1;
			Uniform diffuse;
			Uniform emission;
			Uniform texture0;
			Uniform texture1;

			Uniform sceneAmbient;

		protected:
			void LoadShaders(const std::string&, const std::string &defines);
			virtual void InitUniforms();
			std::string m_name;
			std::string m_defines;
			GLuint m_program;
		};

	}

}
#endif
