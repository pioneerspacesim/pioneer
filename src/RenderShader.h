#ifndef _SHADER_H
#define _SHADER_H

#include "libs.h"

#define SHADER_UNIFORM_VEC4(name) \
	private: \
	GLuint loc_##name; float val_##name[4]; \
	public: \
	void set_##name(float v[4]) { \
		set_##name(v[0],v[1],v[2],v[3]); \
	} \
	void set_##name(float a, float b, float c, float d) { \
		if (!loc_##name) { loc_##name = glGetUniformLocation(m_program, #name); } \
		if (!float_equal_exact(val_##name[0], a) || !float_equal_exact(val_##name[1], b) || \
			!float_equal_exact(val_##name[2], c) || !float_equal_exact(val_##name[3], d)) \
		{ glUniform4f(loc_##name, a,b,c,d); val_##name[0]=a; val_##name[1]=b;val_##name[2]=c;val_##name[3]=d; } \
	}
#define SHADER_UNIFORM_VEC3(name) \
	private: \
	GLuint loc_##name; float val_##name[3]; \
	public: \
	void set_##name(float v[3]) { \
		set_##name(v[0],v[1],v[2]); \
	} \
	void set_##name(float a, float b, float c) { \
		if (!loc_##name) { loc_##name = glGetUniformLocation(m_program, #name); } \
		if (!float_equal_exact(val_##name[0], a) || !float_equal_exact(val_##name[1], b) || \
			!float_equal_exact(val_##name[2], c)) \
		{ glUniform3f(loc_##name, a,b,c); val_##name[0]=a; val_##name[1]=b;val_##name[2]=c; } \
	}
#define SHADER_UNIFORM_FLOAT(name) \
	private: \
	GLuint loc_##name; float val_##name; \
	public: \
	void set_##name(float v) { \
		if (!loc_##name) { loc_##name = glGetUniformLocation(m_program, #name); } \
		if (!float_equal_exact(val_##name, v)) { glUniform1f(loc_##name, v); val_##name = v; } \
	}
#define SHADER_UNIFORM_INT(name) \
	private: \
	GLuint loc_##name; int val_##name; \
	public: \
	void set_##name(int v) { \
		if (!loc_##name) { loc_##name = glGetUniformLocation(m_program, #name); } \
		if (val_##name != v) { glUniform1i(loc_##name, v); val_##name = v; } \
	}
#define SHADER_UNIFORM_SAMPLER(name) \
	SHADER_UNIFORM_INT(name)

#define SHADER_CLASS_BEGIN(name) \
	class name: public Render::Shader { \
		public: \
		name(const char *shaderFilename, const char *additional_defines): Render::Shader() { \
			memset(this, 0, sizeof(name)); \
			Compile(shaderFilename, additional_defines); \
		} \
		name(const char *shaderFilename): Render::Shader() { \
			memset(this, 0, sizeof(name)); \
			Compile(shaderFilename, 0); \
		}
#define SHADER_CLASS_END()	};

namespace Render {

	extern bool shadersAvailable;
	extern bool shadersEnabled;

	class Shader {
	public:
		GLuint GetProgram() const { return m_program; }
		Shader() { m_program = 0; }
		Shader(const char *shaderFilename) {
			Compile(shaderFilename, 0);
		}
		Shader(const char *shaderFilename, const char *additional_defines) {
			Compile(shaderFilename, additional_defines);
		}
		bool Compile(const char *shader_name, const char *additional_defines = 0);
		SHADER_UNIFORM_FLOAT(invLogZfarPlus1);
	protected:
		GLuint m_program;
	private:
		void PrintGLSLCompileError(const char* filename, GLuint obj);
	};

}

#endif
