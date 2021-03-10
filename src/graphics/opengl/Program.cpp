// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Program.h"
#include "FileSystem.h"
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
					throw ShaderException();

				Compile(shader);

				if (!check_glsl_errors(filename.c_str(), shader))
					throw ShaderException();
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
			Unuse();
			glDeleteProgram(m_program);
			LoadShaders(def);
			InitUniforms(shader);
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
			Use();

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

			Unuse();
		}

		// ====================================================================
		// Shader Setup and Creation
		//

		Shader::Shader() :
			m_name(""),
			m_constantStorageSize(0)
		{}

		Shader::Shader(const std::string &name, const MaterialDescriptor &desc) :
			m_name(name),
			m_constantStorageSize(0)
		{
			InitUniforms();
		}

		Shader::~Shader()
		{
			while (!m_variants.empty()) {
				delete m_variants.back().second;
				m_variants.pop_back();
			}
		}

		void Shader::InitUniforms()
		{
			PROFILE_SCOPED()

			// The LightData and DrawData bindings will always be in slot 0 and 1
			AddBufferBinding("LightData");
			AddBufferBinding("DrawData");
		}

		Program *Shader::GetProgramForDesc(const MaterialDescriptor &desc)
		{
			for (auto &pair : m_variants)
				if (pair.first == desc)
					return pair.second;

			Program *program = LoadProgram(desc);
			if (program->Loaded())
				m_variants.push_back({ desc, LoadProgram(desc) });

			return program;
		}

		void Shader::Reload()
		{
			// FIXME: reload the shader definition file and regenerate
			// binding points. This is probably not doable without some
			// way to also track and reload all materials using this shader
			// (as they will need to have their data store reallocated as well)

			// For right now, simply reload the underlying shader program
			// source from disk and recompile; changes to the data interface
			// will require a program restart
			for (auto &variant : m_variants) {
				// TODO: load info from shaderdef file
				std::string filename = std::string("shaders/opengl/") + m_name;
				ProgramDef def{
					m_name,
					filename + ".vert",
					filename + ".frag",
					GetProgramDefines(variant.first)
				};

				variant.second->Reload(this, def);
			}
		}

		Program *Shader::LoadProgram(const MaterialDescriptor &desc)
		{
			// TODO: load info from shaderdef file
			std::string filename = std::string("shaders/opengl/") + m_name;
			ProgramDef def{
				m_name,
				filename + ".vert",
				filename + ".frag",
				GetProgramDefines(desc)
			};

			return new Program(this, def);
		}

		std::string Shader::GetProgramDefines(const MaterialDescriptor &desc)
		{
			//build some defines
			std::stringstream ss;

			ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
			ss << stringf("#define NUM_SHADOWS %0{u}\n", desc.numShadows);
			if (desc.lighting && desc.dirLights > 0)
				ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", 1.0f / float(desc.dirLights));

			if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA)
				ss << "#define TERRAIN_WITH_LAVA\n";
			if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER)
				ss << "#define TERRAIN_WITH_WATER\n";
			if (desc.effect == EFFECT_BILLBOARD_ATLAS)
				ss << "#define USE_SPRITE_ATLAS\n";

			if (desc.textures > 0)
				ss << "#define TEXTURE0\n";
			if (desc.vertexColors)
				ss << "#define VERTEXCOLOR\n";
			if (desc.alphaTest)
				ss << "#define ALPHA_TEST\n";

			if (desc.normalMap && desc.lighting && desc.dirLights > 0)
				ss << "#define MAP_NORMAL\n";
			if (desc.specularMap)
				ss << "#define MAP_SPECULAR\n";
			if (desc.glowMap)
				ss << "#define MAP_EMISSIVE\n";
			if (desc.ambientMap)
				ss << "#define MAP_AMBIENT\n";
			if (desc.usePatterns)
				ss << "#define MAP_COLOR\n";

			if (desc.quality & HAS_ATMOSPHERE)
				ss << "#define ATMOSPHERE\n";
			if (desc.quality & HAS_HEAT_GRADIENT)
				ss << "#define HEAT_COLOURING\n";
			if (desc.quality & HAS_ECLIPSES)
				ss << "#define ECLIPSE\n";

			if (desc.instanced)
				ss << "#define USE_INSTANCING\n";

			return ss.str();
		}

		// ====================================================================
		// Shader Reflection Information
		//

		uint32_t CalcSize(ConstantDataFormat format)
		{
			using CD = ConstantDataFormat;
			switch (format) {
			case CD::DATA_FORMAT_INT:
			case CD::DATA_FORMAT_FLOAT:
				return 4;
			case CD::DATA_FORMAT_FLOAT3:
				return 4 * 3;
			case CD::DATA_FORMAT_FLOAT4:
				return 4 * 4;
			case CD::DATA_FORMAT_MAT3:
				return 4 * 12;
			case CD::DATA_FORMAT_MAT4:
				return 4 * 16;
			default:
				assert(false);
				return 0;
			}
		}

		uint32_t CalcOffset(uint32_t last, ConstantDataFormat lastFormat, ConstantDataFormat thisFormat)
		{
			using CD = ConstantDataFormat;
			if (thisFormat == CD::DATA_FORMAT_INT || thisFormat == CD::DATA_FORMAT_FLOAT)
				// float and integer types align to 1n, where n is the size of the underlying type.
				// Because we're only dealing with increments of 1n, they're already aligned
				return last + CalcSize(lastFormat);
			else {
				// OpenGL requires uniform buffers to align vec and mat elements to increments of 4n,
				// where n is the size of the underlying type.
				// To preserve forwards compatibility, follow these alignment rules.
				constexpr uint32_t ALIGNMENT = 16 - 1;
				uint32_t offset = last + CalcSize(lastFormat);
				offset = (offset + ALIGNMENT) & ~ALIGNMENT;
				return offset;
			}
		}

		size_t Shader::AddBufferBinding(const std::string &name)
		{
			size_t hash = Renderer::GetName(name);
			GLuint binding = m_bufferBindingInfo.size();
			m_bufferBindingInfo.push_back({ hash, binding, 0 });
			m_nameMap[hash] = name;
			return hash;
		}

		size_t Shader::AddTextureBinding(const std::string &name, TextureType type)
		{
			size_t hash = Renderer::GetName(name);
			GLuint binding = m_textureBindingInfo.size();
			m_textureBindingInfo.push_back({ hash, binding, type });
			m_nameMap[hash] = name;
			return hash;
		}

		size_t Shader::AddConstantBinding(const std::string &name, ConstantDataFormat format)
		{
			size_t hash = Renderer::GetName(name);
			GLuint binding = m_pushConstantInfo.size();
			GLuint offset = 0;
			if (m_pushConstantInfo.size()) {
				auto &lastConstant = m_pushConstantInfo.back();
				offset = CalcOffset(lastConstant.offset, lastConstant.format, format);
			}
			m_pushConstantInfo.push_back({ hash, binding, offset, format });
			m_constantStorageSize = offset + CalcSize(format);
			m_nameMap[hash] = name;
			return hash;
		}

		// simple linear search significantly outperforms std::map for n <= 32
		// Luckily, we don't expect more than 32 elems in any of these maps
		template <typename T>
		const T *vector_search(const std::vector<T> &vec, size_t name)
		{
			for (const T &data : vec)
				if (data.name == name)
					return &data;

			return nullptr;
		}

		TextureBindingData Shader::GetTextureBindingInfo(size_t name) const
		{
			auto *data = vector_search(m_textureBindingInfo, name);
			if (data != nullptr) return *data;

			return { 0, InvalidBinding, TextureType::TEXTURE_2D };
		}

		PushConstantData Shader::GetPushConstantInfo(size_t name) const
		{
			auto *data = vector_search(m_pushConstantInfo, name);
			if (data != nullptr) return *data;

			return { 0, InvalidBinding, 0, ConstantDataFormat::DATA_FORMAT_NONE };
		}

		BufferBindingData Shader::GetBufferBindingInfo(size_t name) const
		{
			auto *data = vector_search(m_bufferBindingInfo, name);
			if (data != nullptr) return *data;

			return { 0, InvalidBinding, 0 };
		}

	} // namespace OGL

} // namespace Graphics
