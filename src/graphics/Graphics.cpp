// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Graphics.h"
#include "FileSystem.h"
#include "Material.h"
#include "RendererGL2.h"
#include "RendererLegacy.h"
#include "OS.h"

namespace Graphics {

static bool initted = false;
bool shadersAvailable = false;
bool shadersEnabled = false;
Material *vtxColorMaterial;
Settings settings;
static float g_fov = 85.f;
static float g_fovFactor = 1.f;

int GetScreenWidth()
{
	return settings.width;
}

int GetScreenHeight()
{
	return settings.height;
}

float GetFov()
{
	return g_fov;
}

void SetFov(float fov)
{
	g_fov = fov;
	g_fovFactor = 2 * tan(DEG2RAD(g_fov) / 2.f);
}

float GetFovFactor()
{
	return g_fovFactor;
}

Renderer* Init(Settings vs)
{
	assert(!initted);
	if (initted) return 0;

    // XXX SDL2 not sure what do to with modes yet
    assert(vs.width > 0 && vs.height > 0);
#if 0
	// no mode set, find an ok one
	if ((vs.width <= 0) || (vs.height <= 0)) {
		const std::vector<VideoMode> modes = GetAvailableVideoModes();
		assert(!modes.empty());

		vs.width = modes.front().width;
		vs.height = modes.front().height;
	}
#endif

	WindowSDL *window = new WindowSDL(vs, "Pioneer");

	glewInit();

	if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
		OS::Error("OpenGL extension ARB_vertex_buffer_object not supported. Pioneer can not run on your graphics card.");

	Renderer *renderer = 0;

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = vs.shaders && shadersAvailable;

	if (shadersEnabled)
		renderer = new RendererGL2(window, vs);
	else
		renderer = new RendererLegacy(window, vs);

	printf("Initialized %s\n", renderer->GetName());

	initted = true;

	MaterialDescriptor desc;
	desc.vertexColors = true;
	vtxColorMaterial = renderer->CreateMaterial(desc);
	vtxColorMaterial->IncRefCount();

	Graphics::settings = vs;

	return renderer;
}

void Uninit()
{
	delete vtxColorMaterial;
}

bool AreShadersEnabled()
{
	return shadersEnabled;
}

std::vector<VideoMode> GetAvailableVideoModes()
{
	std::vector<VideoMode> modes;

	// XXX SDL2 modes stuff
#if 0
	//querying modes using the current pixel format
	//note - this has always been sdl_fullscreen, hopefully it does not matter
	SDL_Rect **sdlmodes = SDL_ListModes(0, SDL_HWSURFACE | SDL_FULLSCREEN);

	if (sdlmodes == 0)
		OS::Error("Failed to query video modes");

	if (sdlmodes == reinterpret_cast<SDL_Rect **>(-1)) {
		// Modes restricted. Fall back to 800x600
		modes.push_back(VideoMode(800, 600));
	} else {
		for (int i=0; sdlmodes[i]; ++i) {
			modes.push_back(VideoMode(sdlmodes[i]->w, sdlmodes[i]->h));
		}
	}
#endif
	return modes;
}

}
