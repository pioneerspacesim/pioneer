#ifndef _RENDER_H
#define _RENDER_H

#include "libs.h"

/*
 * bunch of reused 3d drawy routines.
 */
namespace Render {
	class Shader {
	private:
		// for 1-4 lightsources
		static GLuint CompileProgram(const char *shader_name, int num_lights);
	public:
		GLuint m_program[4];
		Shader(const char *name);
	};

	/* static */ class State {
	private:
		static int m_numLights;
		static float m_znear, m_zfar;
		static float m_invLogZfarPlus1; // for z-hack
		static GLuint m_currentProgram;
	public:
		/** setting numLights to 1..4 overrides m_numLights */
		static GLuint UseProgram(const Shader *shader, int numLights=-1);
		static void SetNumLights(int n) { m_numLights = n; }
		static void SetZnearZfar(float znear, float zfar) { m_znear = znear; m_zfar = zfar;
			m_invLogZfarPlus1 = 1.0f / (log(m_zfar+1.0f)/log(2.0f));
		}
		static int GetNumLights() { return m_numLights; }
		static void GetZnearZfar(float &outZnear, float &outZfar) {outZnear = m_znear; outZfar = m_zfar; }
	};

	extern Shader *simpleShader;
	extern Shader *planetRingsShader;

	void Init();
	bool AreShadersEnabled();
	void ToggleShaders();

	void UnbindAllBuffers();
	void BindArrayBuffer(GLuint bo);
	void BindElementArrayBuffer(GLuint bo);
	bool IsArrayBufferBound(GLuint bo);
	bool IsElementArrayBufferBound(GLuint bo);

	void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex, int stride = sizeof(vector3f));
};

#endif /* _RENDER_H */
