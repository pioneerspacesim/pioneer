// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "graphics/UniformBuffer.h"

#include "OpenGLLibs.h"
#include "RefCounted.h"
#include <stack>
#include <unordered_map>

typedef void *SDL_GLContext;

namespace Graphics {

	class Texture;
	struct Settings;

	namespace OGL {
		class CachedVertexBuffer;
		class CommandList;
		class InstanceBuffer;
		class IndexBuffer;
		class Material;
		class MeshObject;
		class RenderState;
		class RenderStateCache;
		class RenderTarget;
		class Shader;
		class UniformBuffer;
		class UniformLinearBuffer;
		class VertexBuffer;
	} // namespace OGL

	class RendererOGL final : public Renderer {
	public:
		static void RegisterRenderer();

		RendererOGL(SDL_Window *window, const Graphics::Settings &vs, SDL_GLContext &glContext);
		virtual ~RendererOGL() override final;

		virtual const char *GetName() const override final { return "OpenGL 3.1, with extensions, renderer"; }
		virtual RendererType GetRendererType() const override final { return RENDERER_OPENGL_3x; }

		virtual void WriteRendererInfo(std::ostream &out) const override final;

		virtual void CheckRenderErrors(const char *func = nullptr, const int line = -1) const override final { CheckErrors(func, line); }
		static void CheckErrors(const char *func = nullptr, const int line = -1);

		virtual bool SupportsInstancing() override final { return true; }

		virtual int GetMaximumNumberAASamples() const override final;
		virtual bool GetNearFarRange(float &near_, float &far_) const override final;

		virtual void SetVSyncEnabled(bool) override;

		virtual void OnWindowResized() override;

		virtual bool BeginFrame() override final;
		virtual bool EndFrame() override final;
		virtual bool SwapBuffers() override final;

		virtual RenderTarget *GetRenderTarget() override final;
		virtual bool SetRenderTarget(RenderTarget *) override final;
		virtual bool SetScissor(ViewportExtents) override final;

		virtual void CopyRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents, ViewportExtents, bool) override final;
		virtual void ResolveRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents) override final;

		virtual bool ClearScreen(const Color &c, bool) override final;
		virtual bool ClearDepthBuffer() override final;

		virtual bool SetViewport(ViewportExtents v) override final;
		virtual ViewportExtents GetViewport() const override final { return m_viewport; }

		virtual bool SetTransform(const matrix4x4f &m) override final;
		virtual matrix4x4f GetTransform() const override final { return m_modelViewMat; }

		virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) override final;
		virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) override final;
		virtual bool SetProjection(const matrix4x4f &m) override final;
		virtual matrix4x4f GetProjection() const override final { return m_projectionMat; }

		virtual bool SetWireFrameMode(bool enabled) override final;

		virtual bool SetLightIntensity(Uint32 numlights, const float *intensity) override final;
		virtual bool SetLights(Uint32 numlights, const Light *l) override final;
		virtual Uint32 GetNumLights() const override final { return m_numLights; }
		virtual bool SetAmbientColor(const Color &c) override final;

		virtual bool FlushCommandBuffers() override final;

		virtual bool DrawBuffer(const VertexArray *v, Material *m) override final;
		virtual bool DrawBufferDynamic(VertexBuffer *v, uint32_t vtxOffset, IndexBuffer *i, uint32_t idxOffset, uint32_t numElems, Material *m) override final;
		virtual bool DrawMesh(MeshObject *, Material *) override final;
		virtual bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) override final;

		virtual Material *CreateMaterial(const std::string &, const MaterialDescriptor &, const RenderStateDesc &) override final;
		virtual Material *CloneMaterial(const Material *, const MaterialDescriptor &, const RenderStateDesc &) override final;
		virtual Texture *CreateTexture(const TextureDescriptor &descriptor) override final;
		virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) override final;
		virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &) override final;
		virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage, IndexBufferSize) override final;
		virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) override final;
		virtual UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage) override final;
		virtual MeshObject *CreateMeshObject(VertexBuffer *v, IndexBuffer *i) override final;
		virtual MeshObject *CreateMeshObjectFromArray(const VertexArray *v, IndexBuffer *i = nullptr, BufferUsage u = BUFFER_USAGE_STATIC) override final;

		virtual const RenderStateDesc &GetMaterialRenderState(const Graphics::Material *m) override final;

		OGL::UniformBuffer *GetLightUniformBuffer();
		OGL::UniformLinearBuffer *GetDrawUniformBuffer(Uint32 size);
		OGL::RenderStateCache *GetStateCache() { return m_renderStateCache.get(); }

		virtual bool ReloadShaders() override final;

		virtual bool Screendump(ScreendumpState &sd) override final;

		bool DrawMeshInternal(OGL::MeshObject *, PrimitiveType type);
		bool DrawMeshInstancedInternal(OGL::MeshObject *, OGL::InstanceBuffer *, PrimitiveType type);
		bool DrawMeshDynamicInternal(BufferBinding<OGL::VertexBuffer> vtxBind, BufferBinding<OGL::IndexBuffer> idxBind, PrimitiveType type);

	protected:
		virtual void PushState() override final{};
		virtual void PopState() override final{};

		size_t m_frameNum;

		Uint32 m_numLights;
		Uint32 m_numDirLights;
		float m_minZNear;
		float m_maxZFar;
		bool m_useCompressedTextures;
		bool m_useAnisotropicFiltering;

		// TODO: iterate shaderdef files on startup and cache by Shader name directive rather than filename fragment
		std::vector<std::pair<std::string, OGL::Shader *>> m_shaders;
		std::vector<std::unique_ptr<OGL::UniformLinearBuffer>> m_drawUniformBuffers;
		std::unique_ptr<OGL::RenderStateCache> m_renderStateCache;
		RefCountedPtr<OGL::UniformBuffer> m_lightUniformBuffer;
		bool m_useNVDepthRanged;
		OGL::RenderTarget *m_activeRenderTarget = nullptr;
		std::unique_ptr<OGL::CommandList> m_drawCommandList;

		matrix4x4f m_modelViewMat;
		matrix4x4f m_projectionMat;
		ViewportExtents m_viewport;

	private:
		static bool initted;

		struct DynamicBufferData {
			AttributeSet attrs;
			OGL::CachedVertexBuffer *vtxBuffer;
			RefCountedPtr<MeshObject> mesh;
		};

		using DynamicBufferMap = std::vector<DynamicBufferData>;
		static DynamicBufferMap s_DynamicDrawBufferMap;

		SDL_GLContext m_glContext;
	};
#define CHECKERRORS() RendererOGL::CheckErrors(__FUNCTION__, __LINE__)

} // namespace Graphics
