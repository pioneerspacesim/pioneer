// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_H
#define _RENDERER_H

#include "WindowSDL.h"
#include "libs.h"
#include "graphics/Types.h"
#include <map>
#include <memory>

namespace Graphics {

/*
 * Renderer base class. A Renderer draws points, lines, triangles.
 * It is also used to create render states, materials and vertex/index buffers.
 */

class Light;
class Material;
class MaterialDescriptor;
class RenderState;
class RenderTarget;
class Texture;
class TextureDescriptor;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
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

	WindowSDL *GetWindow() const { return m_window.get(); }
	float GetDisplayAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	//get supported minimum for z near and maximum for z far values
	virtual bool GetNearFarRange(float &near, float &far) const = 0;

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;
	//traditionally gui happens between endframe and swapbuffers
	virtual bool SwapBuffers() = 0;

	//set 0 to render to screen
	virtual bool SetRenderTarget(RenderTarget*) { return false; }

	//clear color and depth buffer
	virtual bool ClearScreen() { return false; }
	//clear depth buffer
	virtual bool ClearDepthBuffer() { return false; }
	virtual bool SetClearColor(const Color &c) { return false; }

	virtual bool SetViewport(int x, int y, int width, int height) { return false; }

	//set the model view matrix
	virtual bool SetTransform(const matrix4x4d &m) { return false; }
	virtual bool SetTransform(const matrix4x4f &m) { return false; }
	//set projection matrix
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far) { return false; }
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) { return false; }
	virtual bool SetProjection(const matrix4x4f &m) { return false; }

	virtual bool SetRenderState(RenderState*) { return false; }

	virtual bool SetWireFrameMode(bool enabled) { return false; }

	virtual bool SetLights(int numlights, const Light *l) { return false; }
	virtual bool SetAmbientColor(const Color &c) { return false; }
	const Color &GetAmbientColor() const { return m_ambient; }

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f)) { return false; }

	//drawing functions
	//2d drawing is generally understood to be for gui use (unlit, ortho projection)
	//per-vertex colour lines
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, RenderState*, LineType type=LINE_SINGLE) { return false; }
	//flat colour lines
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, RenderState*, LineType type=LINE_SINGLE) { return false; }
	virtual bool DrawLines2D(int vertCount, const vector2f *vertices, const Color &color, RenderState*, LineType type=LINE_SINGLE) { return false; }
	virtual bool DrawPoints(int count, const vector3f *points, const Color *colors, RenderState*, float pointSize=1.f) { return false; }
	//unindexed triangle draw
	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES)  { return false; }
	//high amount of textured quads for particles etc
	virtual bool DrawPointSprites(int count, const vector3f *positions, RenderState *rs, Material *material, float size) { return false; }
	//complex unchanging geometry that is worthwhile to store in VBOs etc.
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType type=TRIANGLES) { return false; }
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType=TRIANGLES) { return false; }

	//creates a unique material based on the descriptor. It will not be deleted automatically.
	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor) = 0;
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor) = 0;
	virtual RenderState *CreateRenderState(const RenderStateDesc &) = 0;
	//returns 0 if unsupported
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) { return 0; }
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc&) = 0;
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) = 0;

	Texture *GetCachedTexture(const std::string &type, const std::string &name);
	void AddCachedTexture(const std::string &type, const std::string &name, Texture *texture);
	void RemoveCachedTexture(const std::string &type, const std::string &name);
	void RemoveAllCachedTextures();

	// output human-readable debug info to the given stream
	virtual bool PrintDebugInfo(std::ostream &out) { return false; }

	virtual bool ReloadShaders() { return false; }

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

protected:
	int m_width;
	int m_height;
	Color m_ambient;

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
