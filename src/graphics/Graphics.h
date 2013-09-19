// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "libs.h"

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
		bool useTextureCompression;
		bool enableDebugMessages;
		int vsync;
		int requestedSamples;
		int height;
		int width;
	};

	//for querying available modes
	struct VideoMode {
		VideoMode(int w, int h)
		: width(w), height(h) { }

		int width;
		int height;
	};

	extern bool shadersAvailable;
	extern bool shadersEnabled;
	extern Material *vtxColorMaterial;

	extern Settings settings;
	int GetScreenWidth();
	int GetScreenHeight();

	float GetFov();
	void SetFov(float);
	float GetFovFactor(); //cached 2*tan(fov/2) for LOD

	// does SDL video init, constructs appropriate Renderer
	Renderer* Init(Settings);
	void Uninit();
	bool AreShadersEnabled();
	std::vector<VideoMode> GetAvailableVideoModes();
}

#endif /* _RENDER_H */
