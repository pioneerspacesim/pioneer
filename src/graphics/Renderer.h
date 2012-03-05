#ifndef _RENDERER_H
#define _RENDERER_H

#include "libs.h"

class Light;
class Texture;

namespace Graphics {

/*
 * Renderer base class. A Renderer draws points, lines, triangles, changes blend modes
 * and other states. Data flows mostly one way: you tell the renderer to do things, but
 * you don't get to query the current matrix mode or number of lights
 * - store that info elsewhere.
 * Performance is not a big concern right now (it hasn't really decreased), to be optimized
 * later
 *
 * To Do:
 * Move statistics collection here: fps, number of triangles etc.
 * Screenshot function (at least read framebuffer, write to file elsewhere)
 * Move SDL video init here (pass a GraphicsSettings struct or similar to Renderer's constructor)
 * ToggleFullscreen
 * The 2D varieties of DrawPoints, DrawLines might have to go - it seemed
 * like a good idea to allow the possibility for optimizing these cases but
 * right now there isn't much of a difference.
 * Renderer::RequestMaterial approach to Materials to get rid of the shader hack (see comments in Material.h)
 * LMR is tricky because it's both a model compiler and renderer. The draw ops can be quite easily
 * converted to use StaticMeshes, Materials etc. but it would lose some of the quirks
 * like changing lighting mid-mesh (without hacks which are not useful outside LMR)
 * Terrain: not necessarily tricky to convert, but let's see if it's going to be
 * rewritten first... Terrain would likely get a special DrawTerrain(GeoPatch *) function.
 * Reboot postprocessing, again
 */

class Material;
class RendererLegacy;
class StaticMesh;
class Surface;
class VertexArray;

// first some enums
enum LineType {
	LINE_SINGLE = GL_LINES, //draw one line per two vertices
	LINE_STRIP = GL_LINE_STRIP,  //connect vertices
	LINE_LOOP = GL_LINE_LOOP    //connect vertices,  connect start & end
};

//how to treat vertices
enum PrimitiveType {
	TRIANGLES = GL_TRIANGLES,
	TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
	TRIANGLE_FAN = GL_TRIANGLE_FAN,
	POINTS = GL_POINTS
};

enum BlendMode {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_ALPHA_ONE, //XXX what the hell to call this
	BLEND_ALPHA_PREMULT
};

// Renderer base, functions return false if
// failed/unsupported
class Renderer
{
public:
	Renderer(int width, int height);
	virtual ~Renderer();

	virtual const char* GetName() = 0;
	//get supported minimum for z near and maximum for z far values
	virtual bool GetNearFarRange(float &near, float &far) const = 0;

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;
	//traditionally gui happens between endframe and swapbuffers
	virtual bool SwapBuffers() = 0;

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

	//render state functions
	virtual bool SetBlendMode(BlendMode type) { return false; }
	virtual bool SetDepthTest(bool enabled) { return false; }
	//enable/disable writing to z buffer
	virtual bool SetDepthWrite(bool enabled) { return false; }
	virtual bool SetWireFrameMode(bool enabled) { return false; }

	virtual bool SetLights(int numlights, const Light *l) { return false; }
	virtual bool SetAmbientColor(const Color &c) { return false; }

	//drawing functions
	//2d drawing is generally understood to be for gui use (unlit, ortho projection)
	//per-vertex colour lines
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, LineType type=LINE_SINGLE) { return false; }
	//flat colour lines
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, LineType type=LINE_SINGLE) { return false; }
	virtual bool DrawLines2D(int vertCount, const vector2f *vertices, const Color &color, LineType type=LINE_SINGLE) { return false; }
	virtual bool DrawPoints(int count, const vector3f *points, const Color *colors, float pointSize=1.f) { return false; }
	virtual bool DrawPoints2D(int count, const vector2f *points, const Color *colors, float pointSize=1.f) { return false; }
	//unindexed triangle draw
	virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES)  { return false; }
	//indexed triangle draw
	virtual bool DrawSurface(const Surface *surface) { return false; }
	//high amount of textured quads for particles etc
	virtual bool DrawPointSprites(int count, const vector3f *positions, const Material *material, float size) { return false; }
	//complex unchanging geometry that is worthwhile to store in VBOs etc.
	virtual bool DrawStaticMesh(StaticMesh *thing) { return false; }

	// take a ticket representing the current renderer state. when the ticket
	// is deleted, the renderer state is restored
	class StateTicket {
	public:
		StateTicket(Renderer *r) : m_renderer(r) { m_renderer->PushState(); }
		virtual ~StateTicket() { m_renderer->PopState(); }
	private:
		StateTicket(const StateTicket&);
		StateTicket &operator=(const StateTicket&);
		Renderer *m_renderer;
	};


protected:
	int m_width;
	int m_height;

	virtual void PushState() = 0;
	virtual void PopState() = 0;
};

// subclass this to store renderer specific information
// See top of RendererLegacy.cpp
struct RenderInfo {
	RenderInfo() { }
	virtual ~RenderInfo() { }
};

// baseclass for a renderable thing. so far it just means that the renderer
// can store renderer-specific data in it (RenderInfo)
struct Renderable {
public:
	Renderable(): m_renderInfo(0) {}

	RenderInfo *GetRenderInfo() const { return m_renderInfo.Get(); }
	void SetRenderInfo(RenderInfo *renderInfo) { m_renderInfo.Reset(renderInfo); }

private:
	ScopedPtr<RenderInfo> m_renderInfo;
};

}

#endif
