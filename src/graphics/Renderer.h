// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_H
#define _RENDERER_H

#include "Graphics.h"
#include "Light.h"
#include "Stats.h"
#include "Types.h"
#include "core/StringHash.h"
#include "graphics/BufferCommon.h"
#include "matrix4x4.h"
#include <map>
#include <memory>

struct SDL_Window;

namespace Graphics {

	/*
	 * Renderer base class. A Renderer draws points, lines, triangles.
	 * It is also used to create render states, materials and vertex/index buffers.
	 */

	class IndexBuffer;
	class InstanceBuffer;
	class Material;
	class MaterialDescriptor;
	class MeshObject;
	class RenderState;
	class RenderTarget;
	class Texture;
	class TextureDescriptor;
	class UniformBuffer;
	class VertexArray;
	class VertexBuffer;

	struct VertexBufferDesc;
	struct RenderStateDesc;
	struct RenderTargetDesc;

	// Renderer base, functions return false if
	// failed/unsupported
	class Renderer {
	private:
		typedef std::pair<std::string, std::string> TextureCacheKey;
		typedef std::map<TextureCacheKey, RefCountedPtr<Texture> *> TextureCacheMap;

	public:
		using TextureCache = TextureCacheMap;

		Renderer(SDL_Window *win, int width, int height);
		virtual ~Renderer();

		virtual const char *GetName() const = 0;
		virtual RendererType GetRendererType() const = 0;

		virtual void WriteRendererInfo(std::ostream &out) const {}

		virtual void CheckRenderErrors(const char *func = nullptr, const int line = -1) const {}

		virtual bool SupportsInstancing() = 0;

		SDL_Window *GetSDLWindow() const { return m_window; }
		virtual void OnWindowResized() {};

		float GetDisplayAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
		int GetWindowWidth() const { return m_width; }
		int GetWindowHeight() const { return m_height; }
		virtual int GetMaximumNumberAASamples() const = 0;

		virtual void SetVSyncEnabled(bool enabled) = 0;

		//get supported minimum for z near and maximum for z far values
		virtual bool GetNearFarRange(float &near_, float &far_) const = 0;

		virtual bool BeginFrame() = 0;
		virtual bool EndFrame() = 0;
		//traditionally gui happens between endframe and swapbuffers
		virtual bool SwapBuffers() = 0;

		// returns currently bound render target (if any)
		virtual RenderTarget *GetRenderTarget() = 0;
		//set 0 to render to screen
		virtual bool SetRenderTarget(RenderTarget *) = 0;

		// Copy a portion of one render target to another, optionally scaling the target
		virtual void CopyRenderTarget(RenderTarget *src, RenderTarget *dst, ViewportExtents srcRect, ViewportExtents dstRect, bool linearFilter = true) = 0;

		// Perform an MSAA resolve from a multisampled render target to regular render target
		// No scaling can be performed.
		virtual void ResolveRenderTarget(RenderTarget *src, RenderTarget *dst, ViewportExtents rect) = 0;

		// Set the scissor extents. This has no effect if not drawing with a renderstate using scissorTest.
		// In particular, the scissor state will not affect clearing the screen.
		virtual bool SetScissor(ViewportExtents scissor) = 0;

		//clear color and depth buffer
		virtual bool ClearScreen(const Color &c = Color::BLACK, bool depthBuffer = true) = 0;

		//clear depth buffer
		virtual bool ClearDepthBuffer() = 0;

		virtual bool SetViewport(ViewportExtents vp) = 0;
		virtual ViewportExtents GetViewport() const = 0;

		//set the model view matrix
		virtual bool SetTransform(const matrix4x4f &m) = 0;
		virtual matrix4x4f GetTransform() const = 0;

		//set projection matrix
		virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) = 0;
		virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) = 0;
		virtual bool SetProjection(const matrix4x4f &m) = 0;
		virtual matrix4x4f GetProjection() const = 0;

		virtual bool SetWireFrameMode(bool enabled) = 0;

		virtual bool SetLightIntensity(Uint32 numlights, const float *intensity) = 0;
		virtual bool SetLights(Uint32 numlights, const Light *l) = 0;
		const Light &GetLight(const Uint32 idx) const
		{
			assert(idx < 4);
			return m_lights[idx];
		}
		virtual Uint32 GetNumLights() const { return 0; }
		virtual bool SetAmbientColor(const Color &c) = 0;
		const Color &GetAmbientColor() const { return m_ambient; }

		//drawing functions
		// TODO: placeholder API; here until CommandLists are exposed
		// and all code can safely deal with async drawing
		virtual bool FlushCommandBuffers() = 0;

		// All drawing commands are assumed to defer execution of the command
		// until the next commandlist flush. This is to batch GPU data updates
		// and ensure state changes are minimal and internally consistent.
		// If the calling code really needs all pending draw commands to be
		// executed before making state changes, call FlushCommandBuffers to
		// manually synchronize.

		// Upload and draw the contents of this VertexArray. Should be used for highly dynamic geometry that changes per-frame.
		// The contents of the VertexArray will be cached internally by the renderer and uploaded in bulk.
		virtual bool DrawBuffer(const VertexArray *v, Material *m) = 0;
		// Draw a subregion from an existing vertex+index buffer. Should be used for drawing aggregated vertex streams
		// generated by middleware (e.g. UI buffers) that are updated once or twice during the frame.
		// vtxOffset, idxOffset specify the starting element, not the starting byte offset in the buffer
		virtual bool DrawBufferDynamic(VertexBuffer *v, uint32_t vtxOffset, IndexBuffer *i, uint32_t idxOffset, uint32_t numElems, Material *m) = 0;
		// Draw a single mesh object (vertex+index buffer) using the given material.
		virtual bool DrawMesh(MeshObject *, Material *) = 0;
		// Draw multiple instances of a mesh object using the given material.
		virtual bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) = 0;

		//creates a unique material based on the descriptor. It will not be deleted automatically.
		virtual Material *CreateMaterial(const std::string &shader, const MaterialDescriptor &descriptor, const RenderStateDesc &stateDescriptor) = 0;
		// Make a copy of the given material with a possibly new descriptor or render state.
		virtual Material *CloneMaterial(const Material *mat, const MaterialDescriptor &descriptor, const RenderStateDesc &stateDescriptor) = 0;
		virtual Texture *CreateTexture(const TextureDescriptor &descriptor) = 0;
		virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) = 0; //returns nullptr if unsupported
		virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &) = 0;
		virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage, IndexBufferSize = INDEX_BUFFER_32BIT) = 0;
		virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) = 0;
		virtual UniformBuffer *CreateUniformBuffer(Uint32 size, BufferUsage) = 0;

		// Create a new mesh object that wraps the given vertex and index buffers.
		virtual MeshObject *CreateMeshObject(VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer = nullptr) = 0;

		// Create a new mesh object and vertex buffer, and upload data from the given vertex array. Optionally associate the given index buffer.
		// This is a convenience function to avoid boilerplate needed to set up a vertex buffer and mesh object from a vertex array.
		// This function is not suitable for transient geometry; prefer DrawBuffer instead.
		virtual MeshObject *CreateMeshObjectFromArray(const VertexArray *vertexArray, IndexBuffer *indexBuffer = nullptr, BufferUsage usage = BUFFER_USAGE_STATIC) = 0;

		// Return a reference to the render state desc that is used by the given material.
		virtual const RenderStateDesc &GetMaterialRenderState(const Material *mat) = 0;

		Texture *GetCachedTexture(const std::string &type, const std::string &name);
		void AddCachedTexture(const std::string &type, const std::string &name, Texture *texture);
		void RemoveCachedTexture(const std::string &type, const std::string &name);
		void RemoveAllCachedTextures();

		const TextureCache &GetTextureCache() { return m_textureCache; }

		virtual bool ReloadShaders() = 0;

		// take a ticket representing the current renderer state. when the ticket
		// is deleted, the renderer state is restored
		// XXX state must die
		class StateTicket {
		public:
			StateTicket(Renderer *r) :
				m_renderer(r)
			{
				m_renderer->PushState();
				m_storedVP = m_renderer->GetViewport();
				m_storedProj = m_renderer->GetProjection();
				m_storedMV = m_renderer->GetTransform();
				m_storedRT = m_renderer->GetRenderTarget();
			}

			virtual ~StateTicket()
			{
				m_renderer->PopState();
				m_renderer->SetRenderTarget(m_storedRT);
				m_renderer->SetViewport(m_storedVP);
				m_renderer->SetTransform(m_storedMV);
				m_renderer->SetProjection(m_storedProj);
			}

			StateTicket(const StateTicket &) = delete;
			StateTicket &operator=(const StateTicket &) = delete;

		private:
			Renderer *m_renderer;
			RenderTarget *m_storedRT;
			matrix4x4f m_storedProj;
			matrix4x4f m_storedMV;
			ViewportExtents m_storedVP;
		};

		// Temporarily save the current transform matrix to do non-destructive drawing.
		class MatrixTicket {
		public:
			MatrixTicket(Renderer *r) :
				MatrixTicket(r, r->GetTransform())
			{
			}

			MatrixTicket(Renderer *r, const matrix4x4f &newMat) :
				m_renderer(r)
			{
				m_storedMat = m_renderer->GetTransform();
				m_renderer->SetTransform(newMat);
			}

			virtual ~MatrixTicket()
			{
				m_renderer->SetTransform(m_storedMat);
			}

			MatrixTicket(const MatrixTicket &) = delete;
			MatrixTicket &operator=(const MatrixTicket &) = delete;

		private:
			Renderer *m_renderer;
			matrix4x4f m_storedMat;
		};

		virtual bool Screendump(ScreendumpState &sd) { return false; }

		Stats &GetStats() { return m_stats; }

		// Returns a hashed name for referring to material constant slots and other constant-size string names
		static constexpr size_t GetName(std::string_view s) { return hash_64_fnv1a(s.data(), s.size()); }

	protected:
		int m_width;
		int m_height;
		Color m_ambient;
		Light m_lights[4];
		Stats m_stats;
		SDL_Window *m_window;

		virtual void PushState() = 0;
		virtual void PopState() = 0;

	private:
		TextureCacheMap m_textureCache;
	};

} // namespace Graphics

#endif
