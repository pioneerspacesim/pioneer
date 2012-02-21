#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "libs.h"

/*
 * bunch of reused 3d drawy routines.
 */
namespace Graphics {

	class Renderer;
	class Shader;

	/* static */ class State {
	private:
		static int m_numLights;
		static float m_znear, m_zfar;
	public:
		static float m_invLogZfarPlus1; // for z-hack
		static void SetNumLights(int n) { m_numLights = n; }
		static void SetZnearZfar(float znear, float zfar) { m_znear = znear; m_zfar = zfar;
			m_invLogZfarPlus1 = 1.0f / (log(m_zfar+1.0f)/log(2.0f));
		}
		static int GetNumLights() { return m_numLights; }
	};

	extern Shader *simpleShader;
	// one for each number of lights (stars in system)
	extern Shader *planetRingsShader[4];

	// constructs renderer
	Renderer* Init(int screen_width, int screen_height, bool wantShaders);
	void Uninit();
	bool AreShadersEnabled();

	void UnbindAllBuffers();
	void BindArrayBuffer(GLuint bo);
	void BindElementArrayBuffer(GLuint bo);
	bool IsArrayBufferBound(GLuint bo);
	bool IsElementArrayBufferBound(GLuint bo);
	//XXX keeping this because gui uses it...
	void SwapBuffers();

	void PrintGLInfo();
}

#endif /* _RENDER_H */
