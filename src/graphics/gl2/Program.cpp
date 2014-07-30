// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Program.h"
#include "FileSystem.h"
#include "StringRange.h"
#include "StringF.h"
#include "OS.h"
#include "graphics/Graphics.h"

namespace Graphics {

namespace GL2 {

static const char *s_glslVersion = "#version 110\n";
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
	Shader(GLenum type, const std::string &filename, const std::string &defines) {
		RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(filename);

		if (!code)
			Error("Could not load %s", filename.c_str());

		// Load some common code
		RefCountedPtr<FileSystem::FileData> logzCode = FileSystem::gameDataFiles.ReadFile("shaders/gl2/logz.glsl");
		assert(logzCode);
		RefCountedPtr<FileSystem::FileData> libsCode = FileSystem::gameDataFiles.ReadFile("shaders/gl2/lib.glsl");
		assert(libsCode);

		AppendSource(s_glslVersion);
		AppendSource(defines.c_str());
		if (type == GL_VERTEX_SHADER)
			AppendSource("#define VERTEX_SHADER\n");
		else
			AppendSource("#define FRAGMENT_SHADER\n");
		AppendSource(logzCode->AsStringRange().StripUTF8BOM());
		AppendSource(libsCode->AsStringRange().StripUTF8BOM());
		AppendSource(code->AsStringRange().StripUTF8BOM());
#if 0
		static bool s_bDumpShaderSource = true;
		if (s_bDumpShaderSource) {
			const char SHADER_OUT_DIR_NAME[] = "shaders";
			const char SHADER_GL2_OUT_DIR_NAME[] = "shaders/gl2";
			FileSystem::userFiles.MakeDirectory(SHADER_OUT_DIR_NAME);
			FileSystem::userFiles.MakeDirectory(SHADER_GL2_OUT_DIR_NAME);
			const std::string outFilename(FileSystem::GetUserDir() + "/" + filename);
			FILE *tmp = fopen(outFilename.c_str(), "w+");
			Output("%s", filename);
			for( Uint32 i=0; i<blocks.size(); i++ ) {
				fprintf(tmp, "%.*s", block_sizes[i], blocks[i]);
			}
			fclose(tmp);
		}
#endif
		shader = glCreateShader(type);
		Compile(shader);

		// CheckGLSL may use OS::Warning instead of Error so the game may still (attempt to) run
		if (!check_glsl_errors(filename.c_str(), shader))
			throw ShaderException();
	};

	~Shader() {
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

	std::vector<const char*> blocks;
	std::vector<GLint> block_sizes;
};

Program::Program()
: m_name("")
, m_defines("")
, m_program(0)
{
}

Program::Program(const std::string &name, const std::string &defines)
: m_name(name)
, m_defines(defines)
, m_program(0)
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
	const std::string filename = std::string("shaders/gl2/") + name;

	//load, create and compile shaders
	Shader vs(GL_VERTEX_SHADER, filename + ".vert", defines);
	Shader fs(GL_FRAGMENT_SHADER, filename + ".frag", defines);

	//create program, attach shaders and link
	m_program = glCreateProgram();
	glAttachShader(m_program, vs.shader);
	glAttachShader(m_program, fs.shader);
	glLinkProgram(m_program);

	check_glsl_errors(name.c_str(), m_program);

	//shaders may now be deleted by Shader destructor
}

void Program::InitUniforms()
{
	//Init generic uniforms, like matrices
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
	heatGradient.Init("heatGradient", m_program);
	heatingMatrix.Init("heatingMatrix", m_program);
	heatingNormal.Init("heatingNormal", m_program);
	heatingAmount.Init("heatingAmount", m_program);
	sceneAmbient.Init("scene.ambient", m_program);
}

} // GL2

} // Graphics
