// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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

		const char *GetName() const final { return "Dummy"; }
		RendererType GetRendererType() const final { return RENDERER_DUMMY; }
		bool SupportsInstancing() final { return false; }
		int GetMaximumNumberAASamples() const final { return 0; }
		bool GetNearFarRange(float &near_, float &far_) const final { return true; }

		void SetVSyncEnabled(bool) override {}

		bool BeginFrame() final { return true; }
		bool EndFrame() final { return true; }
		bool SwapBuffers() final { return true; }

		RenderTarget *GetRenderTarget() final { return m_rt; }
		bool SetRenderTarget(RenderTarget *rt) final { m_rt = rt; return true; }
		bool SetScissor(ViewportExtents ext) final { return true; }

		void CopyRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents, ViewportExtents, bool) final {}
		void ResolveRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents) final {}

		bool ClearScreen(const Color &, bool) final { return true; }
		bool ClearDepthBuffer() final { return true; }

		bool SetViewport(ViewportExtents v) final { return true; }
		ViewportExtents GetViewport() const final { return {}; }

		bool SetTransform(const matrix4x4f &m) final { return true; }
		matrix4x4f GetTransform() const final { return matrix4x4f::Identity(); }
		bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) final { return true; }
		bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) final { return true; }
		bool SetProjection(const matrix4x4f &m) final { return true; }
		matrix4x4f GetProjection() const final { return matrix4x4f::Identity(); }

		bool SetWireFrameMode(bool enabled) final { return true; }

		bool SetLightIntensity(Uint32, const float *) final { return true; }
		bool SetLights(Uint32 numlights, const Light *l) final { return true; }
		Uint32 GetNumLights() const final { return 1; }
		bool SetAmbientColor(const Color &c) final { return true; }

		bool FlushCommandBuffers() final { return true; }

		bool DrawBuffer(const VertexArray *, Material *) final { return true; }
		bool DrawBufferDynamic(VertexBuffer *, uint32_t, IndexBuffer *, uint32_t, uint32_t, Material *) final { return true; }
		bool DrawMesh(MeshObject *, Material *) final { return true; }
		bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) final { return true; }

		Material *CreateMaterial(const std::string &s, const MaterialDescriptor &d, const RenderStateDesc &rsd) final { return new Graphics::Dummy::Material(rsd); }
		Material *CloneMaterial(const Material *m, const MaterialDescriptor &d, const RenderStateDesc &rsd) final { return new Graphics::Dummy::Material(rsd); }
		Texture *CreateTexture(const TextureDescriptor &d) final { return new Graphics::TextureDummy(d); }
		RenderTarget *CreateRenderTarget(const RenderTargetDesc &d) final { return new Graphics::Dummy::RenderTarget(d); }
		VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &d) final { return new Graphics::Dummy::VertexBuffer(d); }
		IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage bu, IndexBufferSize el) final { return new Graphics::Dummy::IndexBuffer(size, bu, el); }
		InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage bu) final { return new Graphics::Dummy::InstanceBuffer(size, bu); }
		UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage bu) final { return new Graphics::Dummy::UniformBuffer(size, bu); }
		MeshObject *CreateMeshObject(VertexBuffer *v, IndexBuffer *i) final { return new Graphics::Dummy::MeshObject(static_cast<Dummy::VertexBuffer *>(v), static_cast<Dummy::IndexBuffer *>(i)); }
		MeshObject *CreateMeshObjectFromArray(const VertexArray *v, IndexBuffer *i = nullptr, BufferUsage = BUFFER_USAGE_STATIC) final
		{
			auto desc = Graphics::VertexBufferDesc::FromAttribSet(v->GetAttributeSet());
			desc.numVertices = v->GetNumVerts();
			return new Graphics::Dummy::MeshObject(static_cast<Dummy::VertexBuffer *>(CreateVertexBuffer(desc)), static_cast<Dummy::IndexBuffer *>(i));
		}

		const RenderStateDesc &GetMaterialRenderState(const Graphics::Material *m) final { return static_cast<const Dummy::Material *>(m)->rsd; }

		bool ReloadShaders() final { return true; }

	protected:
		void PushState() final {}
		void PopState() final {}

	private:
		const matrix4x4f m_identity;
		Graphics::RenderTarget *m_rt;
	};

} // namespace Graphics

#endif
