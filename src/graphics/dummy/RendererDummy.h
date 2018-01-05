// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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

	virtual const char *GetName() const override final { return "Dummy"; }
	virtual RendererType GetRendererType() const  override final { return RENDERER_DUMMY; }
	virtual bool SupportsInstancing() override final { return false; }
	virtual int GetMaximumNumberAASamples() const override final { return 0; }
	virtual bool GetNearFarRange(float &near_, float &far_) const override final { return true; }

	virtual bool BeginFrame() override final { return true; }
	virtual bool EndFrame() override final { return true; }
	virtual bool SwapBuffers() override final { return true; }

	virtual bool SetRenderState(RenderState*) override final { return true; }
	virtual bool SetRenderTarget(RenderTarget*) override final { return true; }

	virtual bool SetDepthRange(double znear, double zfar) override final { return true; }

	virtual bool ClearScreen() override final { return true; }
	virtual bool ClearDepthBuffer() override final { return true; }
	virtual bool SetClearColor(const Color &c) override final { return true; }

	virtual bool SetViewport(int x, int y, int width, int height) override final { return true; }

	virtual bool SetTransform(const matrix4x4d &m) override final { return true; }
	virtual bool SetTransform(const matrix4x4f &m) override final { return true; }
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) override final { return true; }
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) override final { return true; }
	virtual bool SetProjection(const matrix4x4f &m) override final { return true; }

	virtual bool SetWireFrameMode(bool enabled) override final { return true; }

	virtual bool SetLights(Uint32 numlights, const Light *l) override final { return true; }
	virtual Uint32 GetNumLights() const override final { return 1; }
	virtual bool SetAmbientColor(const Color &c) override final { return true; }

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f)) override final { return true; }

	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES) override final { return true; }
	virtual bool DrawPointSprites(const Uint32 count, const vector3f *positions, RenderState *rs, Material *material, float size) override final { return true; }
	virtual bool DrawPointSprites(const Uint32 count, const vector3f *positions, const vector2f *offsets, const float *sizes, RenderState *rs, Material *material) override final { return true; }
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType) override final { return true; }
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType) override final { return true; }
	virtual bool DrawBufferInstanced(VertexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType type=TRIANGLES) override final { return true; }
	virtual bool DrawBufferIndexedInstanced(VertexBuffer*, IndexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType=TRIANGLES) override final { return true; }

	virtual Material *CreateMaterial(const MaterialDescriptor &d) override final { return new Graphics::Dummy::Material(); }
	virtual Texture *CreateTexture(const TextureDescriptor &d) override final { return new Graphics::TextureDummy(d); }
	virtual RenderState *CreateRenderState(const RenderStateDesc &d) override final { return new Graphics::Dummy::RenderState(d); }
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &d) override final { return new Graphics::Dummy::RenderTarget(d); }
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &d) override final { return new Graphics::Dummy::VertexBuffer(d); }
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage bu) override final { return new Graphics::Dummy::IndexBuffer(size, bu); }
	virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage bu) override final { return new Graphics::Dummy::InstanceBuffer(size, bu); }

	virtual bool ReloadShaders() override final { return true; }

	virtual const matrix4x4f& GetCurrentModelView() const override final { return m_identity; }
	virtual const matrix4x4f& GetCurrentProjection() const override final { return m_identity; }
	virtual void GetCurrentViewport(Sint32 *vp) const override final { }

	virtual void SetMatrixMode(MatrixMode mm) override final {}
	virtual void PushMatrix() override final {}
	virtual void PopMatrix() override final {}
	virtual void LoadIdentity() override final {}
	virtual void LoadMatrix(const matrix4x4f &m) override final {}
	virtual void Translate( const float x, const float y, const float z ) override final {}
	virtual void Scale( const float x, const float y, const float z ) override final {}

protected:
	virtual void PushState() override final {}
	virtual void PopState() override final {}

private:
	const matrix4x4f m_identity;
};

}

#endif
