// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "RenderTarget.h"
#include <memory>
#include <vector>

/*
 * bunch of reused 3d drawy routines.
 * XXX most of this is to be removed
 */
namespace Graphics {

	enum RendererType {
		RENDERER_DUMMY,
		RENDERER_OPENGL_3x,
		MAX_RENDERER_TYPE
	};

	const char *RendererNameFromType(const RendererType rType);

	// requested video settings
	struct Settings {
		RendererType rendererType;
		bool fullscreen;
		bool hidden;
		bool useTextureCompression;
		bool useAnisotropicFiltering;
		bool enableDebugMessages;
		bool gl3ForwardCompatible;
		int vsync;
		int requestedSamples;
		int height;
		int width;
		const char *iconFile;
		const char *title;
	};

	class Renderer;

	typedef Renderer *(*RendererCreateFunc)(const Settings &vs);
	void RegisterRenderer(RendererType type, RendererCreateFunc fn);

	//for querying available modes
	struct VideoMode {
		VideoMode(int w, int h) :
			width(w),
			height(h) {}

		int width;
		int height;
	};

	// Lightweight representation of viewport bounds to simplify viewport state management
	struct Viewport {
		Viewport() :
			x(0),
			y(0),
			w(0),
			h(0) {}

		Viewport(int32_t _x, int32_t _y, int32_t _w, int32_t _h) :
			x(_x),
			y(_y),
			w(_w),
			h(_h) {}

		int32_t x, y, w, h;
	};

	class Material;
	extern Material *vtxColorMaterial;

	int GetScreenWidth();
	int GetScreenHeight();

	float GetFov();
	void SetFov(float);
	float GetFovFactor(); //cached 2*tan(fov/2) for LOD

	// Project a point in the renderer's current coordinate system to screenspace
	// as defined by Renderer::GetViewport.
	// TODO: find a better place to hang this off of; this is too useful to be tied to a renderer object
	vector3d ProjectToScreen(const Renderer *r, const vector3d &in);
	vector3f ProjectToScreen(const Renderer *r, const vector3f &in);

	// does SDL video init, constructs appropriate Renderer
	Renderer *Init(Settings);
	void Uninit();
	std::vector<VideoMode> GetAvailableVideoModes();

	struct ScreendumpState {
		std::unique_ptr<Uint8[]> pixels;
		Uint32 width;
		Uint32 height;
		Uint32 stride;
		Uint32 bpp;
	};
} // namespace Graphics

#endif /* _RENDER_H */
