// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _RENDERER_OGL_H
#define _RENDERER_OGL_H
/*
 * OpenGL 2.X renderer (2.0, GLSL 1.10 at the moment)
 *  - no fixed function support (shaders for everything)
 *  The plan is: make this more like GL3/ES2
 *  - try to stick to bufferobjects
 *  - use glvertexattribpointer instead of glvertexpointer etc
 *  - get rid of built-in glMaterial, glMatrix use
 */
#include "graphics/Renderer.h"
#include <stack>
#include <unordered_map>

namespace Graphics {

class Texture;
struct Settings;

namespace OGL {
	class GasGiantSurfaceMaterial;
	class GeoSphereSkyMaterial;
	class GeoSphereSurfaceMaterial;
	class Material;
	class MultiMaterial;
	class LitMultiMaterial;
	class Program;
	class RenderState;
	class RenderTarget;
	class RingMaterial;
	class FresnelColourMaterial;
	class ShieldMaterial;
	class UIMaterial;
}

class RendererOGL : public Renderer
{
public:
	static void RegisterRenderer();

	RendererOGL(WindowSDL *window, const Graphics::Settings &vs);
	virtual ~RendererOGL();

	virtual const char* GetName() const { return "OpenGL 3.1, with extensions, renderer"; }

	virtual void WriteRendererInfo(std::ostream &out) const;

	virtual void CheckRenderErrors() const { CheckErrors(); }
	static void CheckErrors();

	virtual bool GetNearFarRange(float &near_, float &far_) const;

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool SetRenderState(RenderState*) override;
	virtual bool SetRenderTarget(RenderTarget*) override;

	virtual bool SetDepthRange(double near, double far) override;

	virtual bool ClearScreen();
	virtual bool ClearDepthBuffer();
	virtual bool SetClearColor(const Color &c);

	virtual bool SetViewport(int x, int y, int width, int height);

	virtual bool SetTransform(const matrix4x4d &m);
	virtual bool SetTransform(const matrix4x4f &m);
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_);
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
	virtual bool SetProjection(const matrix4x4f &m);

	virtual bool SetWireFrameMode(bool enabled);

	virtual bool SetLights(Uint32 numlights, const Light *l);
	virtual Uint32 GetNumLights() const { return m_numLights; }
	virtual bool SetAmbientColor(const Color &c);

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f));

	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES) override;
	virtual bool DrawPointSprites(int count, const vector3f *positions, RenderState *rs, Material *material, float size) override;
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType) override;
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType) override;
	virtual bool DrawBufferInstanced(VertexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType type=TRIANGLES) override;
	virtual bool DrawBufferIndexedInstanced(VertexBuffer*, IndexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType=TRIANGLES) override;

	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor) override;
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor) override;
	virtual RenderState *CreateRenderState(const RenderStateDesc &) override;
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) override;
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc&) override;
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) override;
	virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) override;

	virtual bool ReloadShaders();

	virtual const matrix4x4f& GetCurrentModelView() const { return m_modelViewStack.top(); }
	virtual const matrix4x4f& GetCurrentProjection() const { return m_projectionStack.top(); }
	virtual void GetCurrentViewport(Sint32 *vp) const {
		const Viewport &cur = m_viewportStack.top();
		vp[0] = cur.x; vp[1] = cur.y; vp[2] = cur.w; vp[3] = cur.h;
	}

	virtual void SetMatrixMode(MatrixMode mm);
	virtual void PushMatrix();
	virtual void PopMatrix();
	virtual void LoadIdentity();
	virtual void LoadMatrix(const matrix4x4f &m);
	virtual void Translate( const float x, const float y, const float z );
	virtual void Scale( const float x, const float y, const float z );

	virtual bool Screendump(ScreendumpState &sd);

protected:
	virtual void PushState();
	virtual void PopState();

	Uint32 m_numLights;
	Uint32 m_numDirLights;
	std::vector<GLuint> m_vertexAttribsSet;
	float m_minZNear;
	float m_maxZFar;
	bool m_useCompressedTextures;
	
	void SetMaterialShaderTransforms(Material *);

	matrix4x4f& GetCurrentTransform() { return m_currentTransform; }
	matrix4x4f m_currentTransform;

	OGL::Program* GetOrCreateProgram(OGL::Material*);
	friend class OGL::Material;
	friend class OGL::GasGiantSurfaceMaterial;
	friend class OGL::GeoSphereSurfaceMaterial;
	friend class OGL::GeoSphereSkyMaterial;
	friend class OGL::MultiMaterial;
	friend class OGL::LitMultiMaterial;
	friend class OGL::RingMaterial;
	friend class OGL::FresnelColourMaterial;
	friend class OGL::ShieldMaterial;
	std::vector<std::pair<MaterialDescriptor, OGL::Program*> > m_programs;
	std::unordered_map<Uint32, OGL::RenderState*> m_renderStates;
	float m_invLogZfarPlus1;
	OGL::RenderTarget *m_activeRenderTarget;
	RenderState *m_activeRenderState;

	MatrixMode m_matrixMode;
	std::stack<matrix4x4f> m_modelViewStack;
	std::stack<matrix4x4f> m_projectionStack;

	struct Viewport {
		Viewport() : x(0), y(0), w(0), h(0) {}
		Sint32 x, y, w, h;
	};
	std::stack<Viewport> m_viewportStack;

private:
	static bool initted;
};

}

#endif
