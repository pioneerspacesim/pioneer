#ifndef _RENDER_H
#define _RENDER_H

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
		if ((val_##name[0] != a) || (val_##name[1] != b) || (val_##name[2] != c) || (val_##name[3] != d)) { glUniform4f(loc_##name, a,b,c,d); val_##name[0]=a; val_##name[1]=b;val_##name[2]=c;val_##name[3]=d; } \
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
		if ((val_##name[0] != a) || (val_##name[1] != b) || (val_##name[2] != c)) { glUniform3f(loc_##name, a,b,c); val_##name[0]=a; val_##name[1]=b;val_##name[2]=c; } \
	}
#define SHADER_UNIFORM_FLOAT(name) \
	private: \
	GLuint loc_##name; float val_##name; \
	public: \
	void set_##name(float v) { \
		if (!loc_##name) { loc_##name = glGetUniformLocation(m_program, #name); } \
		if (val_##name != v) { glUniform1f(loc_##name, v); val_##name = v; } \
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

/*
 * bunch of reused 3d drawy routines.
 */
namespace Render {
	class Shader {
		SHADER_UNIFORM_FLOAT(invLogZfarPlus1);
	protected:
		GLuint m_program;
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
	};

	/* static */ class State {
	private:
		static int m_numLights;
		static float m_znear, m_zfar;
		static float m_invLogZfarPlus1; // for z-hack
		static Shader *m_currentShader;
	public:
		/** Returns true if the shader was changed, or false if shader == m_currentShader */
		static bool UseProgram(Shader *shader);
		static void SetNumLights(int n) { m_numLights = n; }
		static void SetZnearZfar(float znear, float zfar) { m_znear = znear; m_zfar = zfar;
			m_invLogZfarPlus1 = 1.0f / (log(m_zfar+1.0f)/log(2.0f));
		}
		static int GetNumLights() { return m_numLights; }
		static void GetZnearZfar(float &outZnear, float &outZfar) {outZnear = m_znear; outZfar = m_zfar; }
	};

	extern Shader *simpleShader;
	// one for each number of lights (stars in system)
	extern Shader *planetRingsShader[4];

	void Init(int screen_width, int screen_height);
	bool AreShadersEnabled();
	void ToggleShaders();
	void ToggleHDR();

	void UnbindAllBuffers();
	void BindArrayBuffer(GLuint bo);
	void BindElementArrayBuffer(GLuint bo);
	bool IsArrayBufferBound(GLuint bo);
	bool IsElementArrayBufferBound(GLuint bo);
	bool IsHDREnabled();
	bool IsHDRAvailable();
	void PrepareFrame();
	void PostProcess();
	void SwapBuffers();

	void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex, int stride = sizeof(vector3f));
}

#endif /* _RENDER_H */
