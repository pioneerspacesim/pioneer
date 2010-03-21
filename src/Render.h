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

	class State {
	private:
		int m_numLights;
		float m_znear, m_zfar;
		float m_invLogZfarPlus1; // for z-hack
	public:
		State() {
			SetNumLights(1);
			SetZnearZfar(10.0f, 1000000.0f);
		}
		GLuint UseProgram(const Shader *shader);
		void SetNumLights(int n) { m_numLights = n; }
		void SetZnearZfar(float znear, float zfar) { m_znear = znear; m_zfar = zfar;
			m_invLogZfarPlus1 = 1.0f / (log(m_zfar+1.0f)/log(2.0f));
		}
		int GetNumLights() { return m_numLights; }
		void GetZnearZfar(float &outZnear, float &outZfar) {outZnear = m_znear; outZfar = m_zfar; }
	};

	extern Shader *simpleShader;
	extern Shader *planetRingsShader;

	void Init();
	void SetCurrentState(State *state);
	State *GetCurrentState();
	GLuint UseProgram(const Shader *shader);
	bool AreShadersEnabled();
	void ToggleShaders();
	static inline bool CanDoShaders() { return GLEW_VERSION_2_0; }

	void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex, int stride = sizeof(vector3f));
};

#endif /* _RENDER_H */
