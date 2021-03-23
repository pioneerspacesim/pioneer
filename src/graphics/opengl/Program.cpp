// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Program.h"
#include "FileSystem.h"
#include "Shader.h"
#include "StringF.h"
#include "StringRange.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "utils.h"

#include <set>
#include <sstream>

namespace Graphics {

	namespace OGL {

		// #version 140 for OpenGL3.1
		// #version 150 for OpenGL3.2
		// #version 330 for OpenGL3.3
		static const char *s_glslVersion = "#version 140\n";

		// Check and warn about compile & link errors
		static bool check_glsl_errors(const char *filename, GLuint obj)
		{
			//check if shader or program
			bool isShader = (glIsShader(obj) == GL_TRUE);

			int infologLength = 0;
			char infoLog[1024];

			if (isShader)
				glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
			else
				glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

			GLint status;
			if (isShader)
				glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
			else
				glGetProgramiv(obj, GL_LINK_STATUS, &status);

			if (status == GL_FALSE) {
				Error("Error compiling shader: %s:\n%sOpenGL vendor: %s\nOpenGL renderer string: %s",
					filename, infoLog, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
				return false;
			}

#if 0
	if (!isShader) {
		// perform general validation that the program is usable
		glValidateProgram(obj);

		glGetProgramiv(obj, GL_VALIDATE_STATUS, &status);

		if (status == GL_FALSE) {
			Error("Error vaildating shader: %s:\n%sOpenGL vendor: %s\nOpenGL renderer string: %s",
				filename, infoLog, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
			return false;
		}
	}
#endif

			// TODO: this code is obsolete
			// Log warnings even if successfully compiled
			// Sometimes the log is full of junk "success" messages so
			// this is not a good use for OS::Warning
#ifndef NDEBUG
			if (infologLength > 0) {
				if (pi_strcasestr("infoLog", "warning"))
					Output("%s: %s", filename, infoLog);
			}
#endif

			return true;
		}

		// ShaderProgram is a helper class to wrap fragment/vertex shader
		// creation. The hierarchy is conceptually:
		//   Shader ->
		//     Program ->
		//       ShaderProgram (vertex)
		//       ShaderProgram (fragment)
		struct ShaderProgram {
			ShaderProgram(GLenum type, const std::string &filename, const std::string &defines)
			{
				RefCountedPtr<FileSystem::FileData> filecode = FileSystem::gameDataFiles.ReadFile(filename);

				if (!filecode.Valid())
					Error("Could not load %s", filename.c_str());

				std::string strCode(filecode->AsStringRange().ToString());
				size_t found = strCode.find("#include");
				while (found != std::string::npos) {
					// find the name of the file to include
					const size_t begFilename = strCode.find_first_of("\"", found + 8) + 1;
					const size_t endFilename = strCode.find_first_of("\"", begFilename + 1);

					const std::string incFilename = strCode.substr(begFilename, endFilename - begFilename);

					// check we haven't it already included it (avoids circular dependencies)
					const std::set<std::string>::const_iterator foundIt = previousIncludes.find(incFilename);
					if (foundIt != previousIncludes.end()) {
						Error("Circular, or multiple, include of %s\n", incFilename.c_str());
					} else {
						previousIncludes.insert(incFilename);
					}

					// build path for include
					const std::string incPathBuffer = stringf("shaders/opengl/%0", incFilename);

					// read included file
					RefCountedPtr<FileSystem::FileData> incCode = FileSystem::gameDataFiles.ReadFile(incPathBuffer);
					assert(incCode.Valid());

					if (incCode.Valid()) {
						// replace the #include and filename with the included files text
						strCode.replace(found, (endFilename + 1) - found, incCode->GetData(), incCode->GetSize());
						found = strCode.find("#include");
					} else {
						Error("Could not load shader #include %s for shader %s\n", incPathBuffer.c_str(), filename.c_str());
					}
				}
				// Store the modified text with the included files (if any)
				const StringRange code(strCode.c_str(), strCode.size());

				// Build the final shader text to be compiled
				AppendSource(s_glslVersion);
				AppendSource(defines.c_str());
				if (type == GL_VERTEX_SHADER) {
					AppendSource("#define VERTEX_SHADER\n");
				} else {
					AppendSource("#define FRAGMENT_SHADER\n");
				}
				AppendSource(code.StripUTF8BOM());
#if 0
		static bool s_bDumpShaderSource = true;
		if (s_bDumpShaderSource) {
			const char SHADER_OUT_DIR_NAME[] = "shaders";
			const char SHADER_OGL_OUT_DIR_NAME[] = "shaders/opengl";
			FileSystem::userFiles.MakeDirectory(SHADER_OUT_DIR_NAME);
			FileSystem::userFiles.MakeDirectory(SHADER_OGL_OUT_DIR_NAME);
			const std::string outFilename(FileSystem::GetUserDir() + "/" + filename);
			FILE *tmp = fopen(outFilename.c_str(), "wb");
			if(tmp) {
				Output("%s", filename);
				for( Uint32 i=0; i<blocks.size(); i++ ) {
					const char *block = blocks[i];
					const GLint sizes = block_sizes[i];
					if(block && sizes>0) {
						fprintf(tmp, "%.*s", sizes, block);
					}
				}
				fclose(tmp);
			} else {
				Output("Could not open file %s", outFilename.c_str());
			}
		}
#endif
				shader = glCreateShader(type);
				if (glIsShader(shader) != GL_TRUE)
					throw ShaderCompileException();

				Compile(shader);

				if (!check_glsl_errors(filename.c_str(), shader))
					throw ShaderCompileException();
			};

			~ShaderProgram()
			{
				glDeleteShader(shader);
			}

			GLuint shader;

		private:
			void AppendSource(const char *str)
			{
				blocks.push_back(str);
				block_sizes.push_back(std::strlen(str));
			}

			void AppendSource(StringRange str)
			{
				blocks.push_back(str.begin);
				block_sizes.push_back(str.Size());
			}

			void Compile(GLuint shader_id)
			{
				assert(blocks.size() == block_sizes.size());
				glShaderSource(shader_id, blocks.size(), &blocks[0], &block_sizes[0]);
				glCompileShader(shader_id);
			}

			std::vector<const char *> blocks;
			std::vector<GLint> block_sizes;
			std::set<std::string> previousIncludes;
		};

		// ====================================================================
		// Program Setup and Creation
		//

		Program::Program(Shader *shader, const ProgramDef &def) :
			m_program(0),
			success(false)
		{
			LoadShaders(def);
			InitUniforms(shader);
		}

		Program::~Program()
		{
			glDeleteProgram(m_program);
		}

		void Program::Reload(Shader *shader, const ProgramDef &def)
		{
			glDeleteProgram(m_program);
			LoadShaders(def);
			InitUniforms(shader);
		}

		//load, compile and link
		void Program::LoadShaders(const ProgramDef &def)
		{
			PROFILE_SCOPED()

			//load, create and compile shaders
			ShaderProgram vs(GL_VERTEX_SHADER, def.vertexShader, def.defines);
			ShaderProgram fs(GL_FRAGMENT_SHADER, def.fragmentShader, def.defines);

			//create program, attach shaders and link
			m_program = glCreateProgram();
			if (glIsProgram(m_program) != GL_TRUE)
				throw ProgramException();

			glAttachShader(m_program, vs.shader);

			glAttachShader(m_program, fs.shader);

			//extra attribs, if they exist
			glBindAttribLocation(m_program, 0, "a_vertex");
			glBindAttribLocation(m_program, 1, "a_normal");
			glBindAttribLocation(m_program, 2, "a_color");
			glBindAttribLocation(m_program, 3, "a_uv0");
			glBindAttribLocation(m_program, 4, "a_uv1");
			glBindAttribLocation(m_program, 5, "a_tangent");
			glBindAttribLocation(m_program, 6, "a_transform");
			// a_transform @ 6 shadows (uses) 7, 8, and 9
			// next available is layout (location = 10)

			// TODO: setup fragment output locations from shader attributes
			glBindFragDataLocation(m_program, 0, "frag_color");

			glLinkProgram(m_program);

			success = check_glsl_errors(def.name.c_str(), m_program);

			//shaders may now be deleted by Shader destructor
		}

		void Program::InitUniforms(Shader *shader)
		{
			glUseProgram(m_program);

			// Bind texture sampler locations to texture units
			for (auto &texInfo : shader->GetTextureBindings()) {
				GLuint location = glGetUniformLocation(m_program, shader->GetString(texInfo.name).c_str());
				if (location == GL_INVALID_INDEX)
					continue;

				glUniform1i(location, texInfo.binding);
			}

			// Bind uniform buffer blocks to buffer binding points
			for (auto &bufInfo : shader->GetBufferBindings()) {
				GLuint location = glGetUniformBlockIndex(m_program, shader->GetString(bufInfo.name).c_str());
				if (location == GL_INVALID_INDEX)
					continue;

				glUniformBlockBinding(m_program, location, bufInfo.binding);
			}

			// Build the table of active push constants and their uniform locations
			m_constants.resize(shader->GetNumPushConstants());
			for (auto &conInfo : shader->GetPushConstantBindings()) {
				GLuint location = glGetUniformLocation(m_program, shader->GetString(conInfo.name).c_str());
				m_constants[conInfo.binding] = location;
			}

			glUseProgram(0);
		}

	} // namespace OGL

} // namespace Graphics
