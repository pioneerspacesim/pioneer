// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Program.h"
#include "FileSystem.h"
#include "OS.h"
#include "StringF.h"
#include "StringRange.h"
#include "graphics/Graphics.h"

#include <set>

namespace Graphics {

	namespace OGL {

		// #version 140 for OpenGL3.1
		// #version 150 for OpenGL3.2
		// #version 330 for OpenGL3.3
		static const char *s_glslVersion = "#version 140\n";
		GLuint Program::s_curProgram = 0;

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

		struct Shader {
			Shader(GLenum type, const std::string &filename, const std::string &defines)
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
						Error("Could not load %s", incPathBuffer.c_str());
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
					throw ShaderException();

				Compile(shader);

				// CheckGLSL may use OS::Warning instead of Error so the game may still (attempt to) run
				if (!check_glsl_errors(filename.c_str(), shader))
					throw ShaderException();
			};

			~Shader()
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

		Program::Program() :
			m_name(""),
			m_defines(""),
			m_program(0),
			success(false)
		{
		}

		Program::Program(const std::string &name, const std::string &defines) :
			m_name(name),
			m_defines(defines),
			m_program(0),
			success(false)
		{
			LoadShaders(name, defines);
			InitUniforms();
		}

		Program::~Program()
		{
			glDeleteProgram(m_program);
		}

		void Program::Reload()
		{
			Unuse();
			glDeleteProgram(m_program);
			LoadShaders(m_name, m_defines);
			InitUniforms();
		}

		void Program::Use()
		{
			if (s_curProgram != m_program)
				glUseProgram(m_program);
			s_curProgram = m_program;
		}

		void Program::Unuse()
		{
			glUseProgram(0);
			s_curProgram = 0;
		}

		//load, compile and link
		void Program::LoadShaders(const std::string &name, const std::string &defines)
		{
			PROFILE_SCOPED()
			const std::string filename = std::string("shaders/opengl/") + name;

			//load, create and compile shaders
			Shader vs(GL_VERTEX_SHADER, filename + ".vert", defines);
			Shader fs(GL_FRAGMENT_SHADER, filename + ".frag", defines);

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

			glBindFragDataLocation(m_program, 0, "frag_color");

			glLinkProgram(m_program);

			success = check_glsl_errors(name.c_str(), m_program);

			//shaders may now be deleted by Shader destructor
		}

		void Program::InitUniforms()
		{
			PROFILE_SCOPED()
			//Init generic uniforms, like matrices
			uProjectionMatrix.Init("uProjectionMatrix", m_program);
			uViewMatrix.Init("uViewMatrix", m_program);
			uViewMatrixInverse.Init("uViewMatrixInverse", m_program);
			uViewProjectionMatrix.Init("uViewProjectionMatrix", m_program);
			uNormalMatrix.Init("uNormalMatrix", m_program);

			//Light uniform parameters
			char cLight[64];
			for (int i = 0; i < 4; i++) {
				snprintf(cLight, 64, "uLight[%d]", i);
				const std::string strLight(cLight);
				lights[i].diffuse.Init((strLight + ".diffuse").c_str(), m_program);
				lights[i].specular.Init((strLight + ".specular").c_str(), m_program);
				lights[i].position.Init((strLight + ".position").c_str(), m_program);
			}

			invLogZfarPlus1.Init("invLogZfarPlus1", m_program);
			diffuse.Init("material.diffuse", m_program);
			emission.Init("material.emission", m_program);
			specular.Init("material.specular", m_program);
			shininess.Init("material.shininess", m_program);
			texture0.Init("texture0", m_program);
			texture1.Init("texture1", m_program);
			texture2.Init("texture2", m_program);
			texture3.Init("texture3", m_program);
			texture4.Init("texture4", m_program);
			texture5.Init("texture5", m_program);
			texture6.Init("texture6", m_program);
			heatGradient.Init("heatGradient", m_program);
			heatingMatrix.Init("heatingMatrix", m_program);
			heatingNormal.Init("heatingNormal", m_program);
			heatingAmount.Init("heatingAmount", m_program);
			sceneAmbient.Init("scene.ambient", m_program);
		}

	} // namespace OGL

} // namespace Graphics
