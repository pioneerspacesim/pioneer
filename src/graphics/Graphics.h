// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "libs.h"
#include "RenderTarget.h"

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
		bool useTextureCompression;
		bool enableDebugMessages;
		int vsync;
		int requestedSamples;
		int height;
		int width;
		const char *iconFile;
		const char *title;
	};

	//for querying available modes
	struct VideoMode {
		VideoMode(int w, int h)
		: width(w), height(h) { }

		int width;
		int height;
	};

	extern Material *vtxColorMaterial;

	int GetScreenWidth();
	int GetScreenHeight();

	float GetFov();
	void SetFov(float);
	float GetFovFactor(); //cached 2*tan(fov/2) for LOD

	// does SDL video init, constructs appropriate Renderer
	Renderer* Init(Settings);
	void Uninit();
	std::vector<VideoMode> GetAvailableVideoModes();
}

#endif /* _RENDER_H */
