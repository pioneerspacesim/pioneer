// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "RenderTarget.h"
#include "matrix4x4.h"
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
		bool canBeResized;
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
	struct ViewportExtents {
		ViewportExtents() :
			x(0),
			y(0),
			w(0),
			h(0) {}

		ViewportExtents(int32_t _x, int32_t _y, int32_t _w, int32_t _h) :
			x(_x),
			y(_y),
			w(_w),
			h(_h) {}

		int32_t x, y, w, h;
		bool operator!=(const ViewportExtents &rhs) const { return !(*this == rhs); }
		bool operator==(const ViewportExtents &rhs) const
		{
			return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
		}
	};

	class Material;
	extern Material *vtxColorMaterial;

	float GetFov();
	void SetFov(float);
	float GetFovFactor(); //cached 2*tan(fov/2) for LOD

	// Project a point in the renderer's current coordinate system to
	// screenspace as defined by Renderer::GetViewport.
	// This function applies the current renderer transform to the input point
	// TODO: find a better place to hang this off of; this is too useful to be tied to a renderer object
	vector3d ProjectToScreen(const Renderer *r, const vector3d &in);
	vector3f ProjectToScreen(const Renderer *r, const vector3f &in);

	// ProjectToScreen handles projecting a point in view-space to 2d viewport space.
	// It returns the { X, Y } window coordinates and the NDC depth value as Z.
	// Implements gluProject (see the OpenGL documentation or the Mesa implementation of gluProject)
	// This implementation is tailored to understand Reverse-Z and our data structures.
	template <typename T>
	vector3<T> ProjectToScreen(const vector3<T> &vcam, const matrix4x4<T> &proj, const ViewportExtents &vp)
	{
		// compute the effective W component for perspective divide.
		// This code assumes that it's being passed a 'standard' perspective or ortho matrix.
		const double w = vcam.z * proj[11] + proj[15];

		// convert view coordinates -> homogeneous coordinates -> NDC
		// perspective divide is applied last (left-to-right associativity)
		const vector3<T> vNDC = proj * vcam / w;

		// convert -1..1 NDC to 0..1 viewport coordinates
		const vector3<T> vVP = {
			vNDC.x * T(0.5) + T(0.5),
			vNDC.y * T(0.5) + T(0.5),
			// FIXME: this isn't a proper linearization into viewspace (needs -znear / zNDC)
			// but it accomplishes the goal of positions behind the camera having z > 0.
			-vNDC.z // undo reverse-Z coordinate flip
		};

		// viewport coord * size + position
		return vector3<T>{
			vVP.x * vp.w + vp.x,
			vVP.y * vp.h + vp.y,
			vVP.z
		};
	}

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
