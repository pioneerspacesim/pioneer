#include "Shader.h"
#include "Graphics.h"
#include "FileSystem.h"
#include "StringRange.h"
#include <cstring>

namespace Graphics {

bool shadersEnabled;
bool shadersAvailable;
Shader *m_currentShader = 0;

void Shader::PrintGLSLCompileError(const char *filename, GLuint obj)
{
	int infologLength = 0;
	char infoLog[1024];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

	if (infologLength > 0) {
		fprintf(stderr, "Error compiling shader: %s:\n%sOpenGL vendor: %s\nOpenGL renderer string: %s\nPioneer will run with shaders disabled\n",
				filename, infoLog, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		shadersAvailable = false;
		shadersEnabled = false;
	}
}

static RefCountedPtr<FileSystem::FileData> s_lib_fs;
static RefCountedPtr<FileSystem::FileData> s_lib_vs;
static RefCountedPtr<FileSystem::FileData> s_lib_all;

// Render::FreeLibs
void FreeLibs()
{
	s_lib_fs.Reset();
	s_lib_vs.Reset();
	s_lib_all.Reset();
}

class ShaderSource {
public:
	void Clear()
	{
		blocks.clear();
		block_sizes.clear();
	}

	void Append(const char *str)
	{
		blocks.push_back(str);
		block_sizes.push_back(std::strlen(str));
	}

	void Append(StringRange str)
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

private:
	std::vector<const char*> blocks;
	std::vector<GLint> block_sizes;
};

bool Shader::Compile(const char *shader_name, const char *additional_defines)
{
	GLuint vs, ps = 0;
	ShaderSource shader_src;

	if (!shadersAvailable) {
		m_program = 0;
		return false;
	}

	if (!s_lib_fs) s_lib_fs = FileSystem::gameDataFiles.ReadFile("shaders/_library.frag.glsl");
	if (!s_lib_vs) s_lib_vs = FileSystem::gameDataFiles.ReadFile("shaders/_library.vert.glsl");
	if (!s_lib_all) s_lib_all = FileSystem::gameDataFiles.ReadFile("shaders/_library.all.glsl");

	const std::string name = std::string("shaders/") + shader_name;
	RefCountedPtr<FileSystem::FileData> vscode = FileSystem::gameDataFiles.ReadFile(name + ".vert.glsl");
	RefCountedPtr<FileSystem::FileData> pscode = FileSystem::gameDataFiles.ReadFile(name + ".frag.glsl");
	RefCountedPtr<FileSystem::FileData> allcode = FileSystem::gameDataFiles.ReadFile(name + ".all.glsl");

	if (!vscode) {
		fprintf(stderr, "Could not find shader %s\n", (name + ".vert.glsl").c_str());
		m_program = 0;
		return false;
	}

	if (additional_defines) shader_src.Append(additional_defines);
	shader_src.Append("#define ZHACK 1\n");
	shader_src.Append(s_lib_all->AsStringRange());
	shader_src.Append(s_lib_vs->AsStringRange());

	if (allcode) { shader_src.Append(allcode->AsStringRange()); }
	shader_src.Append(vscode->AsStringRange());

	vs = glCreateShader(GL_VERTEX_SHADER);
	shader_src.Compile(vs);
	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError((name + ".vert.glsl").c_str(), vs);
		m_program = 0;
		return false;
	}

	if (pscode) {
		shader_src.Clear();

		if (additional_defines) { shader_src.Append(additional_defines); }
		shader_src.Append("#define ZHACK 1\n");
		shader_src.Append(s_lib_all->AsStringRange());
		shader_src.Append(s_lib_fs->AsStringRange());
		if (allcode) { shader_src.Append(allcode->AsStringRange()); }
		shader_src.Append(pscode->AsStringRange());

		ps = glCreateShader(GL_FRAGMENT_SHADER);
		shader_src.Compile(ps);
		glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
		if (!status) {
			PrintGLSLCompileError((name + ".frag.glsl").c_str(), ps);
			m_program = 0;
			return false;
		}
	}

	m_program = glCreateProgram();
	glAttachShader(m_program, vs);
	if (pscode) glAttachShader(m_program, ps);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError(name.c_str(), m_program);
		m_program = 0;
		return false;
	}

	return true;
}

bool Shader::Use()
{
	if (!shadersEnabled || m_currentShader == this) return false;

	glUseProgram(m_program);
	set_invLogZfarPlus1(State::m_invLogZfarPlus1);
	m_currentShader = this;
	return true;
}

void Shader::Unuse()
{
	if (shadersEnabled)
		glUseProgram(0);
	m_currentShader = 0;
}

}
