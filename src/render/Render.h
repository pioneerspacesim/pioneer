#ifndef _RENDER_H
#define _RENDER_H

#include "libs.h"
#include "RenderShader.h"

/*
 * bunch of reused 3d drawy routines.
 */
namespace Render {

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
	void Uninit();
	bool AreShadersEnabled();
	void ToggleShaders();

	void GetNearFarClipPlane(float &znear, float &zfar);

	void UnbindAllBuffers();
	void BindArrayBuffer(GLuint bo);
	void BindElementArrayBuffer(GLuint bo);
	bool IsArrayBufferBound(GLuint bo);
	bool IsElementArrayBufferBound(GLuint bo);
	void PrepareFrame();
	void PostProcess();
	void SwapBuffers();

	void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], int stride = sizeof(vector3f));

	void PrintGLInfo();
}

#endif /* _RENDER_H */
