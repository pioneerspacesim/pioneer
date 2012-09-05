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
	class Material;

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
		static std::vector<Light> m_lights;

	public:
		static float invLogZfarPlus1; // for LMR, updated by rendererGL2
		static void SetLights(int n, const Light *lights);
		static int GetNumLights() { return m_lights.size(); } // for LMR
		static std::vector<Light> GetLights() { return m_lights; }
	};

	extern bool shadersAvailable;
	extern bool shadersEnabled;
	extern Material *vtxColorMaterial;

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
