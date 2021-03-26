// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _RENDERER_OGL_H
#define _RENDERER_OGL_H

#include "OpenGLLibs.h"
#include "RefCounted.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "graphics/opengl/UniformBuffer.h"
#include <stack>
#include <unordered_map>

namespace Graphics {

	class Texture;
	struct Settings;

	namespace OGL {
		class CachedVertexBuffer;
		class CommandList;
		class Material;
		class Shader;
		class RenderState;
		class RenderStateCache;
		class RenderTarget;
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

		virtual bool BeginFrame() override final;
		virtual bool EndFrame() override final;
		virtual bool SwapBuffers() override final;

		virtual bool SetRenderTarget(RenderTarget *) override final;
		virtual bool SetScissor(ViewportExtents) override final;

		virtual bool ClearScreen() override final;
		virtual bool ClearDepthBuffer() override final;
		virtual bool SetClearColor(const Color &c) override final;

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
		virtual bool DrawMesh(MeshObject *, Material *) override final;
		virtual bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) override final;

		virtual Material *CreateMaterial(const std::string &, const MaterialDescriptor &, const RenderStateDesc &) override final;
		virtual Material *CloneMaterial(const Material *, const MaterialDescriptor &, const RenderStateDesc &) override final;
		virtual Texture *CreateTexture(const TextureDescriptor &descriptor) override final;
		virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) override final;
		virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &) override final;
		virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) override final;
		virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) override final;
		OGL::UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage);
		virtual MeshObject *CreateMeshObject(VertexBuffer *v, IndexBuffer *i) override final;
		virtual MeshObject *CreateMeshObjectFromArray(const VertexArray *v, IndexBuffer *i = nullptr, BufferUsage u = BUFFER_USAGE_STATIC) override final;

		OGL::UniformBuffer *GetLightUniformBuffer();
		OGL::UniformLinearBuffer *GetDrawUniformBuffer(Uint32 size);
		OGL::RenderStateCache *GetStateCache() { return m_renderStateCache.get(); }

		virtual bool ReloadShaders() override final;

		virtual bool Screendump(ScreendumpState &sd) override final;
		virtual bool FrameGrab(ScreendumpState &sd) override final;

		bool DrawMeshInternal(MeshObject *, uint32_t offset, PrimitiveType type, uint32_t num);
		bool DrawMeshInstancedInternal(MeshObject *, uint32_t offset, InstanceBuffer *, PrimitiveType type, uint32_t num);

	protected:
		virtual void PushState() override final{};
		virtual void PopState() override final{};

		size_t m_frameNum;

		Uint32 m_numLights;
		Uint32 m_numDirLights;
		std::vector<GLuint> m_vertexAttribsSet;
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
		OGL::RenderTarget *m_windowRenderTarget = nullptr;
		std::unique_ptr<OGL::CommandList> m_drawCommandList;

		matrix4x4f m_modelViewMat;
		matrix4x4f m_projectionMat;
		ViewportExtents m_viewport;
		Color m_clearColor;

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

#endif
