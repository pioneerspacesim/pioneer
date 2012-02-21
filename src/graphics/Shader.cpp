#include "Shader.h"
#include "Graphics.h"

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
		Warning("Error compiling shader: %s: %s\nOpenGL vendor: %s\nOpenGL renderer string: %s\n\nPioneer will run with shaders disabled.",
				filename, infoLog, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		shadersAvailable = false;
		shadersEnabled = false;
	}
}

static __attribute((malloc)) char *load_file(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f) {
		//printf("Could not open %s.\n", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = static_cast<char*>(malloc(sizeof(char) * (len+1)));
	size_t len_read = fread(buf, 1, len, f);
	if (len_read != len) {
		fclose(f);
		free(buf);
		return 0;
	}
	fclose(f);
	buf[len] = 0;
	return buf;
}

static char *s_lib_fs = 0;
static char *s_lib_vs = 0;
static char *s_lib_all = 0;

// Render::FreeLibs
void FreeLibs()
{
	if(s_lib_fs) { free(s_lib_fs); s_lib_fs = 0; }
	if(s_lib_vs) { free(s_lib_vs); s_lib_vs = 0; }
	if(s_lib_all) { free(s_lib_all); s_lib_all = 0; }
}

bool Shader::Compile(const char *shader_name, const char *additional_defines)
{
	GLuint vs, ps = 0;
	std::vector<const char*> shader_src;

	if (!shadersAvailable) {
		m_program = 0;
		return false;
	}
	if (!s_lib_fs) s_lib_fs = load_file(PIONEER_DATA_DIR"/shaders/_library.frag.glsl");
	if (!s_lib_vs) s_lib_vs = load_file(PIONEER_DATA_DIR"/shaders/_library.vert.glsl");
	if (!s_lib_all) s_lib_all = load_file(PIONEER_DATA_DIR"/shaders/_library.all.glsl");

	const std::string name = std::string(PIONEER_DATA_DIR"/shaders/") + shader_name;
	char *vscode = load_file((name + ".vert.glsl").c_str());
	char *pscode = load_file((name + ".frag.glsl").c_str());
	char *allcode = load_file((name + ".all.glsl").c_str());

	if (vscode == 0) {
		Warning("Could not find shader %s.", (name + ".vert.glsl").c_str());
		goto fail;
	}

	vs = glCreateShader(GL_VERTEX_SHADER);

	if (additional_defines) shader_src.push_back(additional_defines);
	shader_src.push_back("#define ZHACK 1\n");
	shader_src.push_back(s_lib_all);
	shader_src.push_back(s_lib_vs);
	if (allcode) shader_src.push_back(allcode);
	shader_src.push_back(vscode);

	glShaderSource(vs, shader_src.size(), &shader_src[0], 0);
	glCompileShader(vs);
	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError((name + ".vert.glsl").c_str(), vs);
		goto fail;
	}

	if (pscode) {
		shader_src.clear();
		if (additional_defines) shader_src.push_back(additional_defines);
		shader_src.push_back("#define ZHACK 1\n");
		shader_src.push_back(s_lib_all);
		shader_src.push_back(s_lib_fs);
		if (allcode) shader_src.push_back(allcode);
		shader_src.push_back(pscode);

		ps = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(ps, shader_src.size(), &shader_src[0], 0);
		glCompileShader(ps);
		glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
		if (!status) {
			PrintGLSLCompileError((name + ".frag.glsl").c_str(), ps);
			goto fail;
		}
	}

	m_program = glCreateProgram();
	glAttachShader(m_program, vs);
	if (pscode) glAttachShader(m_program, ps);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError(name.c_str(), m_program);
		goto fail;
	}

	free(vscode);
	free(pscode);
	free(allcode);

	return true;

fail:
	free(vscode);
	free(pscode);
	free(allcode);

	m_program = 0;
	return false;
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
	glUseProgram(0);
	m_currentShader = 0;
}

} /* namespace Render */
