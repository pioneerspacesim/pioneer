// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_GL2_H
#define _RENDERER_GL2_H
/*
 * OpenGL 2.X renderer (2.0, GLSL 1.10 at the moment)
 *  - no fixed function support (shaders for everything)
 *  The plan is: make this more like GL3/ES2
 *  - try to stick to bufferobjects
 *  - use glvertexattribpointer instead of glvertexpointer etc
 *  - get rid of built-in glMaterial, glMatrix use
 */
#include "Renderer.h"
#include "RendererLegacy.h"

namespace Graphics {

namespace GL2 {
	class GeoSphereSkyMaterial;
	class GeoSphereSurfaceMaterial;
	class Material;
	class MultiMaterial;
	class LitMultiMaterial;
	class Program;
	class RenderTarget;
	class RingMaterial;
	class FresnelColourMaterial;
}

class RendererGL2 : public RendererLegacy
{
public:
	RendererGL2(const Graphics::Settings &vs);
	virtual ~RendererGL2();

	virtual const char* GetName() const { return "GL2 renderer"; }

	virtual bool BeginFrame();

	virtual bool SetRenderTarget(RenderTarget*);

	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far);

	virtual bool SetAmbientColor(const Color &c);

	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, LineType type=LINE_SINGLE);
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, LineType type=LINE_SINGLE);

	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor);
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &);

	virtual bool ReloadShaders();

private:
	GL2::Program* GetOrCreateProgram(GL2::Material*);
	friend class GL2::GeoSphereSurfaceMaterial;
	friend class GL2::GeoSphereSkyMaterial;
	friend class GL2::MultiMaterial;
	friend class GL2::LitMultiMaterial;
	friend class GL2::RingMaterial;
	friend class GL2::FresnelColourMaterial;
	std::vector<std::pair<MaterialDescriptor, GL2::Program*> > m_programs;
	float m_invLogZfarPlus1;
	GL2::RenderTarget *m_activeRenderTarget;
};

}

#endif
