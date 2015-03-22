// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _RENDERER_DUMMY_H
#define _RENDERER_DUMMY_H

#include "graphics/Renderer.h"
#include "graphics/dummy/MaterialDummy.h"
#include "graphics/dummy/TextureDummy.h"
#include "graphics/dummy/RenderStateDummy.h"
#include "graphics/dummy/RenderTargetDummy.h"
#include "graphics/dummy/VertexBufferDummy.h"

namespace Graphics {

class RendererDummy : public Renderer
{
public:
	static void RegisterRenderer();

	RendererDummy() : Renderer(0, 0, 0),
	m_identity(matrix4x4f::Identity())
	{}

	virtual const char *GetName() const { return "Dummy"; }
	virtual bool GetNearFarRange(float &near_, float &far_) const { return true; }

	virtual bool BeginFrame() { return true; }
	virtual bool EndFrame() { return true; }
	virtual bool SwapBuffers() { return true; }

	virtual bool SetRenderState(RenderState*) override { return true; }
	virtual bool SetRenderTarget(RenderTarget*) override { return true; }

	virtual bool SetDepthRange(double near, double far) override { return true; }

	virtual bool ClearScreen() { return true; }
	virtual bool ClearDepthBuffer() { return true; }
	virtual bool SetClearColor(const Color &c) { return true; }

	virtual bool SetViewport(int x, int y, int width, int height) { return true; }

	virtual bool SetTransform(const matrix4x4d &m) { return true; }
	virtual bool SetTransform(const matrix4x4f &m) { return true; }
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) { return true; }
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) { return true; }
	virtual bool SetProjection(const matrix4x4f &m) { return true; }

	virtual bool SetWireFrameMode(bool enabled) { return true; }

	virtual bool SetLights(Uint32 numlights, const Light *l) { return true; }
	virtual Uint32 GetNumLights() const { return 1; }
	virtual bool SetAmbientColor(const Color &c) { return true; }

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f)) { return true; }

	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES) override { return true; }
	virtual bool DrawPointSprites(int count, const vector3f *positions, RenderState *rs, Material *material, float size) override { return true; }
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType) override { return true; }
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType) override { return true; }
	virtual bool DrawBufferInstanced(VertexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType type=TRIANGLES) override { return true; }
	virtual bool DrawBufferIndexedInstanced(VertexBuffer*, IndexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType=TRIANGLES) override { return true; }

	virtual Material *CreateMaterial(const MaterialDescriptor &d) override { return new Graphics::Dummy::Material(); }
	virtual Texture *CreateTexture(const TextureDescriptor &d) override { return new Graphics::TextureDummy(d); }
	virtual RenderState *CreateRenderState(const RenderStateDesc &d) override { return new Graphics::Dummy::RenderState(d); }
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &d) override { return new Graphics::Dummy::RenderTarget(d); }
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &d) override { return new Graphics::Dummy::VertexBuffer(d); }
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage bu) override { return new Graphics::Dummy::IndexBuffer(size, bu); }
	virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage bu) override { return new Graphics::Dummy::InstanceBuffer(size, bu); }

	virtual bool ReloadShaders() { return true; }

	virtual const matrix4x4f& GetCurrentModelView() const { return m_identity; }
	virtual const matrix4x4f& GetCurrentProjection() const { return m_identity; }
	virtual void GetCurrentViewport(Sint32 *vp) const { }

	virtual void SetMatrixMode(MatrixMode mm) {}
	virtual void PushMatrix() {}
	virtual void PopMatrix() {}
	virtual void LoadIdentity() {}
	virtual void LoadMatrix(const matrix4x4f &m) {}
	virtual void Translate( const float x, const float y, const float z ) {}
	virtual void Scale( const float x, const float y, const float z ) {}

protected:
	virtual void PushState() {}
	virtual void PopState() {}

private:
	const matrix4x4f m_identity;
};

}

#endif
