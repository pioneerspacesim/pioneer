// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Types.h"
#ifndef _RENDERER_DUMMY_H
#define _RENDERER_DUMMY_H

#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/dummy/MaterialDummy.h"
#include "graphics/dummy/RenderTargetDummy.h"
#include "graphics/dummy/TextureDummy.h"
#include "graphics/dummy/UniformBufferDummy.h"
#include "graphics/dummy/VertexBufferDummy.h"

namespace Graphics {

	class RendererDummy : public Renderer {
	public:
		static void RegisterRenderer();

		RendererDummy() :
			Renderer(0, 0, 0),
			m_identity(matrix4x4f::Identity())
		{}

		virtual const char *GetName() const override final { return "Dummy"; }
		virtual RendererType GetRendererType() const override final { return RENDERER_DUMMY; }
		virtual bool SupportsInstancing() override final { return false; }
		virtual int GetMaximumNumberAASamples() const override final { return 0; }
		virtual bool GetNearFarRange(float &near_, float &far_) const override final { return true; }

		virtual void SetVSyncEnabled(bool) override {}

		virtual bool BeginFrame() override final { return true; }
		virtual bool EndFrame() override final { return true; }
		virtual bool SwapBuffers() override final { return true; }

		virtual RenderTarget *GetRenderTarget() override final { return m_rt; }
		virtual bool SetRenderTarget(RenderTarget *rt) override final { m_rt = rt; return true; }
		virtual bool SetScissor(ViewportExtents ext) override final { return true; }

		virtual void CopyRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents, ViewportExtents, bool) override final {}
		virtual void ResolveRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents) override final {}

		virtual bool ClearScreen(const Color &, bool) override final { return true; }
		virtual bool ClearDepthBuffer() override final { return true; }

		virtual bool SetViewport(ViewportExtents v) override final { return true; }
		virtual ViewportExtents GetViewport() const override final { return {}; }

		virtual bool SetTransform(const matrix4x4f &m) override final { return true; }
		virtual matrix4x4f GetTransform() const override final { return matrix4x4f::Identity(); }
		virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) override final { return true; }
		virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) override final { return true; }
		virtual bool SetProjection(const matrix4x4f &m) override final { return true; }
		virtual matrix4x4f GetProjection() const override final { return matrix4x4f::Identity(); }

		virtual bool SetWireFrameMode(bool enabled) override final { return true; }

		virtual bool SetLightIntensity(Uint32, const float *) override final { return true; }
		virtual bool SetLights(Uint32 numlights, const Light *l) override final { return true; }
		virtual Uint32 GetNumLights() const override final { return 1; }
		virtual bool SetAmbientColor(const Color &c) override final { return true; }

		virtual bool FlushCommandBuffers() override final { return true; }

		virtual bool DrawBuffer(const VertexArray *, Material *) override final { return true; }
		virtual bool DrawBufferDynamic(VertexBuffer *, uint32_t, IndexBuffer *, uint32_t, uint32_t, Material *) override final { return true; }
		virtual bool DrawMesh(MeshObject *, Material *) override final { return true; }
		virtual bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) override final { return true; }

		virtual Material *CreateMaterial(const std::string &s, const MaterialDescriptor &d, const RenderStateDesc &rsd) override final { return new Graphics::Dummy::Material(rsd); }
		virtual Material *CloneMaterial(const Material *m, const MaterialDescriptor &d, const RenderStateDesc &rsd) override final { return new Graphics::Dummy::Material(rsd); }
		virtual Texture *CreateTexture(const TextureDescriptor &d) override final { return new Graphics::TextureDummy(d); }
		virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &d) override final { return new Graphics::Dummy::RenderTarget(d); }
		virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &d) override final { return new Graphics::Dummy::VertexBuffer(d); }
		virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage bu, IndexBufferSize el) override final { return new Graphics::Dummy::IndexBuffer(size, bu, el); }
		virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage bu) override final { return new Graphics::Dummy::InstanceBuffer(size, bu); }
		virtual UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage bu) override final { return new Graphics::Dummy::UniformBuffer(size, bu); }
		virtual MeshObject *CreateMeshObject(VertexBuffer *v, IndexBuffer *i) override final { return new Graphics::Dummy::MeshObject(static_cast<Dummy::VertexBuffer *>(v), static_cast<Dummy::IndexBuffer *>(i)); }
		virtual MeshObject *CreateMeshObjectFromArray(const VertexArray *v, IndexBuffer *i = nullptr, BufferUsage = BUFFER_USAGE_STATIC) override final
		{
			auto desc = Graphics::VertexBufferDesc::FromAttribSet(v->GetAttributeSet());
			desc.numVertices = v->GetNumVerts();
			return new Graphics::Dummy::MeshObject(static_cast<Dummy::VertexBuffer *>(CreateVertexBuffer(desc)), static_cast<Dummy::IndexBuffer *>(i));
		}

		virtual const RenderStateDesc &GetMaterialRenderState(const Graphics::Material *m) override final { return static_cast<const Dummy::Material *>(m)->rsd; }

		virtual bool ReloadShaders() override final { return true; }

	protected:
		virtual void PushState() override final {}
		virtual void PopState() override final {}

	private:
		const matrix4x4f m_identity;
		Graphics::RenderTarget *m_rt;
	};

} // namespace Graphics

#endif
