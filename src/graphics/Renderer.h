// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_H
#define _RENDERER_H

#include "Graphics.h"
#include "Light.h"
#include "Stats.h"
#include "Types.h"
#include "libs.h"
#include "matrix4x4.h"
#include <map>
#include <memory>

namespace Graphics {

	/*
 * Renderer base class. A Renderer draws points, lines, triangles.
 * It is also used to create render states, materials and vertex/index buffers.
 */

	class Material;
	class MaterialDescriptor;
	class RenderState;
	class RenderTarget;
	class Texture;
	class TextureDescriptor;
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;
	class InstanceBuffer;
	class MeshObject;
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
		float GetDisplayAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
		int GetWindowWidth() const { return m_width; }
		int GetWindowHeight() const { return m_height; }
		virtual int GetMaximumNumberAASamples() const = 0;

		//get supported minimum for z near and maximum for z far values
		virtual bool GetNearFarRange(float &near_, float &far_) const = 0;

		virtual bool BeginFrame() = 0;
		virtual bool EndFrame() = 0;
		//traditionally gui happens between endframe and swapbuffers
		virtual bool SwapBuffers() = 0;

		//set 0 to render to screen
		virtual bool SetRenderTarget(RenderTarget *) = 0;

		//clear color and depth buffer
		virtual bool ClearScreen() = 0;
		//clear depth buffer
		virtual bool ClearDepthBuffer() = 0;
		virtual bool SetClearColor(const Color &c) = 0;

		virtual bool SetViewport(Viewport vp) = 0;
		virtual Viewport GetViewport() const = 0;

		//set the model view matrix
		virtual bool SetTransform(const matrix4x4f &m) = 0;
		virtual matrix4x4f GetTransform() const = 0;

		//set projection matrix
		virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) = 0;
		virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) = 0;
		virtual bool SetProjection(const matrix4x4f &m) = 0;
		virtual matrix4x4f GetProjection() const = 0;

		// XXX maybe GL-specific. maybe should be part of the render state
		virtual bool SetDepthRange(double znear, double zfar) = 0;
		virtual bool ResetDepthRange() = 0;

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

		virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f)) = 0;

		//drawing functions

		// Upload and draw the contents of this VertexArray. Should be used for highly dynamic geometry that changes per-frame.
		virtual bool DrawBuffer(const VertexArray *v, Material *m) = 0;
		// Draw a single mesh object (vertex+index buffer) using the given material.
		virtual bool DrawMesh(MeshObject *, Material *) = 0;
		// Draw multiple instances of a mesh object using the given material.
		virtual bool DrawMeshInstanced(MeshObject *, Material *, InstanceBuffer *) = 0;

		//creates a unique material based on the descriptor. It will not be deleted automatically.
		virtual Material *CreateMaterial(const MaterialDescriptor &descriptor, const RenderStateDesc &stateDescriptor) = 0;
		virtual Texture *CreateTexture(const TextureDescriptor &descriptor) = 0;
		virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) = 0; //returns nullptr if unsupported
		virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc &) = 0;
		virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) = 0;
		virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) = 0;

		// Create a new mesh object that wraps the given vertex and index buffers.
		virtual MeshObject *CreateMeshObject(VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer = nullptr) = 0;

		// Create a new mesh object and vertex buffer, and upload data from the given vertex array. Optionally associate the given index buffer.
		// This is a convenience function to avoid boilerplate needed to set up a vertex buffer and mesh object from a vertex array.
		// This function is not suitable for transient geometry; prefer DrawBuffer instead.
		virtual MeshObject *CreateMeshObjectFromArray(const VertexArray *vertexArray, IndexBuffer *indexBuffer = nullptr, BufferUsage usage = BUFFER_USAGE_STATIC) = 0;

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
			}

			virtual ~StateTicket()
			{
				m_renderer->PopState();
				m_renderer->SetViewport(m_storedVP);
				m_renderer->SetTransform(m_storedMV);
				m_renderer->SetProjection(m_storedProj);
			}

			StateTicket(const StateTicket &) = delete;
			StateTicket &operator=(const StateTicket &) = delete;

		private:
			Renderer *m_renderer;
			matrix4x4f m_storedProj;
			matrix4x4f m_storedMV;
			Viewport m_storedVP;
		};

		// Temporarily save the current transform matrix to do non-destructive drawing.
		// XXX state has died, does this need to die further?
		class MatrixTicket {
		public:
			MatrixTicket(Renderer *r) :
				MatrixTicket(r, r->GetTransform())
			{}

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
		virtual bool FrameGrab(ScreendumpState &sd) { return false; }

		Stats &GetStats() { return m_stats; }

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
