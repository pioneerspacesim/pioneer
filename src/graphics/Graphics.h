#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "libs.h"
#include "Color.h"
#include "Light.h"

/*
 * bunch of reused 3d drawy routines.
 * XXX most of this is to be removed
 */
namespace Graphics {

	class Renderer;
	class Shader;

	// requested video settings
	struct Settings {
		bool fullscreen;
		bool shaders;
		int vsync;
		int requestedSamples;
		int height;
		int width;
	};

	/* static */ class State {
	private:
		static float m_znear, m_zfar;
		static std::vector<Light> m_lights;
		static Color m_globalAmbientColor;
	public:
		static float m_invLogZfarPlus1; // for z-hack
		static void SetLights(int n, const Light *lights);
		static void SetGlobalSceneAmbientColor(Color camb) { m_globalAmbientColor = camb; }
		static void SetZnearZfar(float znear, float zfar) { m_znear = znear; m_zfar = zfar;
			m_invLogZfarPlus1 = 1.0f / (log(m_zfar+1.0f)/log(2.0f));
		}
		static int GetNumLights() { return m_lights.size(); }
		static std::vector<Light> GetLights() { return m_lights; }
		static Color GetGlobalSceneAmbientColor() { return m_globalAmbientColor; }
	};

	extern Shader *simpleShader;
	// one for each number of lights (stars in system)
	extern Shader *planetRingsShader[4];

	// does SDL video init, constructs appropriate Renderer
	Renderer* Init(const Settings&);
	void Uninit();
	bool AreShadersEnabled();

	void UnbindAllBuffers();
	void BindArrayBuffer(GLuint bo);
	void BindElementArrayBuffer(GLuint bo);
	bool IsArrayBufferBound(GLuint bo);
	bool IsElementArrayBufferBound(GLuint bo);
	//XXX keeping this because gui uses it...
	void SwapBuffers();
}

#endif /* _RENDER_H */
