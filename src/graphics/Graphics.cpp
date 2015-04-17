// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Graphics.h"
#include "FileSystem.h"
#include "Material.h"
#include "opengl/RendererGL.h"
#include "dummy/RendererDummy.h"
#include "OS.h"
#include "StringF.h"
#include <sstream>
#include <iterator>

namespace Graphics {

static RendererCreateFunc rendererCreateFunc[MAX_RENDERER_TYPE] = {};

void RegisterRenderer(RendererType type, RendererCreateFunc fn) {
	assert(type < MAX_RENDERER_TYPE);
	assert(fn);
	rendererCreateFunc[type] = fn;
}

static bool initted = false;
Material *vtxColorMaterial;
static int width, height;
static float g_fov = 85.f;
static float g_fovFactor = 1.f;

int GetScreenWidth()
{
	return width;
}

int GetScreenHeight()
{
	return height;
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

	// no mode set, find an ok one
	if ((vs.width <= 0) || (vs.height <= 0)) {
		const std::vector<VideoMode> modes = GetAvailableVideoModes();
		assert(!modes.empty());

		vs.width = modes.front().width;
		vs.height = modes.front().height;
	}

	WindowSDL *window = new WindowSDL(vs, "Pioneer");
	if (vs.rendererType == Graphics::RENDERER_DUMMY) {
		width = vs.width;
		height = vs.height;
	} else {
		width = window->GetWidth();
		height = window->GetHeight();
	}

	// We deliberately ignore the value from GL_NUM_COMPRESSED_TEXTURE_FORMATS, because some drivers
	// choose not to list any formats (despite supporting texture compression). See issue #3132.
	// This is (probably) allowed by the spec, which states that only formats which are "suitable
	// for general-purpose usage" should be enumerated.

	assert(vs.rendererType < MAX_RENDERER_TYPE);
	assert(rendererCreateFunc[vs.rendererType]);
	Renderer *renderer = rendererCreateFunc[vs.rendererType](window, vs);

	Output("Initialized %s\n", renderer->GetName());

	{
		std::ostringstream buf;
		renderer->WriteRendererInfo(buf);

		FILE *f = FileSystem::userFiles.OpenWriteStream("opengl.txt", FileSystem::FileSourceFS::WRITE_TEXT);
		if (!f)
			Output("Could not open 'opengl.txt'\n");
		const std::string &s = buf.str();
		fwrite(s.c_str(), 1, s.size(), f);
		fclose(f);
	}

	initted = true;

	MaterialDescriptor desc;
	desc.effect = EFFECT_VTXCOLOR;
	desc.vertexColors = true;
	vtxColorMaterial = renderer->CreateMaterial(desc);
	vtxColorMaterial->IncRefCount();

	return renderer;
}

void Uninit()
{
	delete vtxColorMaterial;
}

static bool operator==(const VideoMode &a, const VideoMode &b) {
	return a.width==b.width && a.height==b.height;
}

std::vector<VideoMode> GetAvailableVideoModes()
{
	std::vector<VideoMode> modes;

	const int num_displays = SDL_GetNumVideoDisplays();
	for(int display_index = 0; display_index < num_displays; display_index++)
	{
		const int num_modes = SDL_GetNumDisplayModes(display_index);

		SDL_Rect display_bounds;
		SDL_GetDisplayBounds(display_index, &display_bounds);

		for (int display_mode = 0; display_mode < num_modes; display_mode++)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(display_index, display_mode, &mode);
			// insert only if unique resolution
			if( modes.end()==std::find(modes.begin(), modes.end(), VideoMode(mode.w, mode.h)) ) {
				modes.push_back(VideoMode(mode.w, mode.h));
			}
		}
	}
	if( num_displays==0 ) {
		modes.push_back(VideoMode(800, 600));
	}
	return modes;
}

}
