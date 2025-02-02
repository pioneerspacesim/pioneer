// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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
		~RendererOGL() final;

		const char *GetName() const final { return "OpenGL 3.1, with extensions, renderer"; }
		RendererType GetRendererType() const final { return RENDERER_OPENGL_3x; }

		void WriteRendererInfo(std::ostream &out) const final;

		void CheckRenderErrors(const char *func = nullptr, const int line = -1) const final { CheckErrors(func, line); }
		static void CheckErrors(const char *func = nullptr, const int line = -1);

		bool SupportsInstancing() final { return true; }

		int GetMaximumNumberAASamples() const final;
		bool GetNearFarRange(float &near_, float &far_) const final;

		void SetVSyncEnabled(bool) override;

		void OnWindowResized() override;

		bool BeginFrame() final;
		bool EndFrame() final;
		bool SwapBuffers() final;

		RenderTarget *GetRenderTarget() final;
		bool SetRenderTarget(RenderTarget *) final;
		bool SetScissor(ViewportExtents) final;

		void CopyRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents, ViewportExtents, bool) final;
		void ResolveRenderTarget(RenderTarget *, RenderTarget *, ViewportExtents) final;

		bool ClearScreen(const Color &c, bool) final;
		bool ClearDepthBuffer() final;

		bool SetViewport(ViewportExtents v) final;
		ViewportExtents GetViewport() const final { return m_viewport; }

		bool SetTransform(const matrix4x4f &m) final;
		matrix4x4f GetTransform() const final { return m_modelViewMat; }

		bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) final;
		bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) final;
		bool SetProjection(const matrix4x4f &m) final;
		matrix4x4f GetProjection() const final { return m_projectionMat; }

		bool SetWireFrameMode(bool enabled) final;

		bool SetLightIntensity(Uint32 numlights, const float *intensity) final;
		bool SetLights(Uint32 numlights, const Light *l) final;
		Uint32 GetNumLights() const final { return m_numLights; }
		bool SetAmbientColor(const Color &c) final;

		bool FlushCommandBuffers() final;

		bool DrawBuffer(const VertexArray *v, Material *m) final;
		bool DrawBufferDynamic(VertexBuffer *v, uint32_t vtxOffset, IndexBuffer *i, uint32_t idxOffset, uint32_t numElems, Material *m) final;
		bool DrawMesh(MeshObject *, Material *) final;
		bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) final;

		Material *CreateMaterial(const std::string &, const MaterialDescriptor &, const RenderStateDesc &) final;
		Material *CloneMaterial(const Material *, const MaterialDescriptor &, const RenderStateDesc &) final;
		Texture *CreateTexture(const TextureDescriptor &descriptor) final;
		RenderTarget *CreateRenderTarget(const RenderTargetDesc &) final;
		VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &) final;
		IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage, IndexBufferSize) final;
		InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) final;
		UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage) final;
		MeshObject *CreateMeshObject(VertexBuffer *v, IndexBuffer *i) final;
		MeshObject *CreateMeshObjectFromArray(const VertexArray *v, IndexBuffer *i = nullptr, BufferUsage u = BUFFER_USAGE_STATIC) final;

		const RenderStateDesc &GetMaterialRenderState(const Graphics::Material *m) final;

		const BufferBinding<UniformBuffer> &GetLightUniformBuffer();
		OGL::UniformLinearBuffer *GetDrawUniformBuffer(Uint32 size);
		OGL::RenderStateCache *GetStateCache() { return m_renderStateCache.get(); }

		bool ReloadShaders() final;

		bool Screendump(ScreendumpState &sd) final;

		bool DrawMeshInternal(OGL::MeshObject *, PrimitiveType type);
		bool DrawMeshInstancedInternal(OGL::MeshObject *, OGL::InstanceBuffer *, PrimitiveType type);
		bool DrawMeshDynamicInternal(BufferBinding<OGL::VertexBuffer> vtxBind, BufferBinding<OGL::IndexBuffer> idxBind, PrimitiveType type);

	protected:
		void PushState() final{};
		void PopState() final{};

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
		BufferBinding<UniformBuffer> m_lightUniformBuffer;
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
