// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Graphics.h"
#include "FileSystem.h"
#include "Material.h"
#include "RendererGL2.h"
#include "OS.h"

namespace Graphics {

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
	width = window->GetWidth();
	height = window->GetHeight();

	glewInit();

	if (!glewIsSupported("GL_VERSION_2_0") )
		Error("OpenGL Version 2.0 is not supported. Pioneer cannot run on your graphics card.");

	if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
		Error("OpenGL extension ARB_vertex_buffer_object not supported. Pioneer can not run on your graphics card.");

	if (!glewIsSupported("GL_EXT_texture_compression_s3tc"))
		Error("OpenGL extension GL_EXT_texture_compression_s3tc not supported.\nPioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");

	GLint intv[4];
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &intv[0]);
	if( intv[0] == 0 )
		Error("GL_NUM_COMPRESSED_TEXTURE_FORMATS is zero.\nPioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");

	Renderer *renderer = new RendererGL2(window, vs);

	Output("Initialized %s\n", renderer->GetName());

	initted = true;

	MaterialDescriptor desc;
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
