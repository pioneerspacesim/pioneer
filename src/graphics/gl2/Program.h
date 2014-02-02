// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
			Uniform specular;
			Uniform shininess;
			Uniform texture0;
			Uniform texture1;
			Uniform texture2;
			Uniform texture3;
			Uniform texture4;
			Uniform heatGradient;
			Uniform heatingMatrix;
			Uniform heatingNormal;
			Uniform heatingAmount;

			Uniform sceneAmbient;

		protected:
			static GLuint s_curProgram;

			void LoadShaders(const std::string&, const std::string &defines);
			virtual void InitUniforms();
			std::string m_name;
			std::string m_defines;
			GLuint m_program;
		};

	}

}
#endif
