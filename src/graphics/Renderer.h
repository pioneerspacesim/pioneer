// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_H
#define _RENDERER_H

#include "WindowSDL.h"
#include "libs.h"
#include "graphics/Types.h"
#include "graphics/Light.h"
#include "Stats.h"
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
struct VertexBufferDesc;
struct RenderStateDesc;
struct RenderTargetDesc;

enum class MatrixMode {
	MODELVIEW,
	PROJECTION
};


// Renderer base, functions return false if
// failed/unsupported
class Renderer
{
public:
	Renderer(WindowSDL *win, int width, int height);
	virtual ~Renderer();

	virtual const char* GetName() const = 0;

	virtual void WriteRendererInfo(std::ostream &out) const {}

	virtual void CheckRenderErrors() const {}

	WindowSDL *GetWindow() const { return m_window.get(); }
	float GetDisplayAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	//get supported minimum for z near and maximum for z far values
	virtual bool GetNearFarRange(float &near_, float &far_) const = 0;

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;
	//traditionally gui happens between endframe and swapbuffers
	virtual bool SwapBuffers() = 0;

	//set 0 to render to screen
	virtual bool SetRenderTarget(RenderTarget*) = 0;

	//clear color and depth buffer
	virtual bool ClearScreen() = 0;
	//clear depth buffer
	virtual bool ClearDepthBuffer() = 0;
	virtual bool SetClearColor(const Color &c) = 0;

	virtual bool SetViewport(int x, int y, int width, int height) = 0;

	//set the model view matrix
	virtual bool SetTransform(const matrix4x4d &m) = 0;
	virtual bool SetTransform(const matrix4x4f &m) = 0;
	//set projection matrix
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near_, float far_) = 0;
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) = 0;
	virtual bool SetProjection(const matrix4x4f &m) = 0;

	virtual bool SetRenderState(RenderState*) = 0;

	// XXX maybe GL-specific. maybe should be part of the render state
	virtual bool SetDepthRange(double near, double far) = 0;

	virtual bool SetWireFrameMode(bool enabled) = 0;

	virtual bool SetLights(Uint32 numlights, const Light *l) = 0;
	const Light& GetLight(const Uint32 idx) const { assert(idx<4); return m_lights[idx]; }
	virtual Uint32 GetNumLights() const { return 0; }
	virtual bool SetAmbientColor(const Color &c) = 0;
	const Color &GetAmbientColor() const { return m_ambient; }

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f)) = 0;

	//drawing functions
	//2d drawing is generally understood to be for gui use (unlit, ortho projection)
	//unindexed triangle draw
	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES) = 0;
	//high amount of textured quads for particles etc
	virtual bool DrawPointSprites(int count, const vector3f *positions, RenderState *rs, Material *material, float size) = 0;
	//complex unchanging geometry that is worthwhile to store in VBOs etc.
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType type=TRIANGLES) = 0;
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType=TRIANGLES) = 0;
	// instanced variations of the above
	virtual bool DrawBufferInstanced(VertexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType type=TRIANGLES) = 0;
	virtual bool DrawBufferIndexedInstanced(VertexBuffer*, IndexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType=TRIANGLES) = 0;

	//creates a unique material based on the descriptor. It will not be deleted automatically.
	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor) = 0;
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor) = 0;
	virtual RenderState *CreateRenderState(const RenderStateDesc &) = 0;
	//returns 0 if unsupported
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) { return 0; }
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc&) = 0;
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) = 0;
	virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) = 0;

	Texture *GetCachedTexture(const std::string &type, const std::string &name);
	void AddCachedTexture(const std::string &type, const std::string &name, Texture *texture);
	void RemoveCachedTexture(const std::string &type, const std::string &name);
	void RemoveAllCachedTextures();

	virtual bool ReloadShaders() = 0;

	// our own matrix stack
	// XXX state must die
	virtual const matrix4x4f& GetCurrentModelView() const  = 0;
	virtual const matrix4x4f& GetCurrentProjection() const  = 0;
	virtual void GetCurrentViewport(Sint32 *vp) const  = 0;

	// XXX all quite GL specific. state must die!
	virtual void SetMatrixMode(MatrixMode mm) = 0;
	virtual void PushMatrix() = 0;
	virtual void PopMatrix() = 0;
	virtual void LoadIdentity() = 0;
	virtual void LoadMatrix(const matrix4x4f &m) = 0;
	virtual void Translate( const float x, const float y, const float z ) = 0;
	virtual void Scale( const float x, const float y, const float z ) = 0;

	// take a ticket representing the current renderer state. when the ticket
	// is deleted, the renderer state is restored
	// XXX state must die
	class StateTicket {
	public:
		StateTicket(Renderer *r) : m_renderer(r) { m_renderer->PushState(); }
		virtual ~StateTicket() { m_renderer->PopState(); }
	private:
		StateTicket(const StateTicket&);
		StateTicket &operator=(const StateTicket&);
		Renderer *m_renderer;
	};

	// take a ticket representing a single state matrix. when the ticket is
	// deleted, the previous matrix state is restored
	// XXX state must die
	class MatrixTicket {
	public:
		MatrixTicket(Renderer *r, MatrixMode m) : m_renderer(r), m_matrixMode(m) {
			m_renderer->SetMatrixMode(m_matrixMode);
			m_renderer->PushMatrix();
		}
		virtual ~MatrixTicket() {
			m_renderer->SetMatrixMode(m_matrixMode);
			m_renderer->PopMatrix();
		}
	private:
		MatrixTicket(const MatrixTicket&);
		MatrixTicket &operator=(const MatrixTicket&);
		Renderer *m_renderer;
		MatrixMode m_matrixMode;
	};

	virtual bool Screendump(ScreendumpState &sd) { return false; }

	Stats& GetStats() { return m_stats; }

protected:
	int m_width;
	int m_height;
	Color m_ambient;
	Light m_lights[4];
	Stats m_stats;

	virtual void PushState() = 0;
	virtual void PopState() = 0;

private:
	typedef std::pair<std::string,std::string> TextureCacheKey;
	typedef std::map<TextureCacheKey,RefCountedPtr<Texture>*> TextureCacheMap;
	TextureCacheMap m_textures;

	std::unique_ptr<WindowSDL> m_window;
};

}

#endif
