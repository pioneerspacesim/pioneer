// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_OGLPROGRAM_H
#define _GRAPHICS_OGLPROGRAM_H
/*
 * The new 'Shader' class
 * This is a base class without specific uniforms
 */
#include "OpenGLLibs.h"
#include "Uniform.h"

#include <string>

namespace Graphics {

	namespace OGL {

		struct ShaderException {};

		struct ProgramException {};

		class Program {
		public:
			Program();
			Program(const std::string &name, const std::string &defines);
			virtual ~Program();
			void Reload();
			virtual void Use();
			virtual void Unuse();
			bool Loaded() const { return success; }

			// Uniforms.
			Uniform uProjectionMatrix;
			Uniform uViewMatrix;
			Uniform uViewMatrixInverse;
			Uniform uViewProjectionMatrix;
			Uniform uNormalMatrix;

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
			Uniform texture5;
			Uniform texture6;
			Uniform heatGradient;
			Uniform heatingMatrix;
			Uniform heatingNormal;
			Uniform heatingAmount;

			Uniform sceneAmbient;

			//Light uniform parameters
			struct UniformLight {
				Uniform diffuse;
				Uniform specular;
				Uniform position;
			};
			UniformLight lights[4];

		protected:
			static GLuint s_curProgram;

			void LoadShaders(const std::string &, const std::string &defines);
			virtual void InitUniforms();
			std::string m_name;
			std::string m_defines;
			GLuint m_program;
			bool success;
		};

	} // namespace OGL

} // namespace Graphics
#endif
