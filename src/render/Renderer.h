#ifndef _RENDERER_H
#define _RENDERER_H

#include "libs.h"

/*
 * Don't mind the mess! Experiments are happening.
 *
 * Draws points, lines, polys...
 * Terrains and LMRmodels might be too special for this now
 * Would ideally also:
 * set blending modes
 * set fill modes (solid/wireframe)
 * create and destroy context
 * toggle between fullscreen/windowed
 * prepare/endframe/swapbuf (move from Render.h)
 * take screenshot (at least read framebuffer)
 */

class Light;
class Material;
class RendererLegacy;
class StaticMesh;
class Surface;
class Texture;
class VertexArray;

namespace Render {
	class Shader;
}

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
	TYPE_POINTS = GL_POINTS
};

enum BlendMode {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_ALPHA_ONE, //XXX what the hell to call this
	BLEND_ALPHA_PREMULT
};

struct LineVertex {
	LineVertex() : position(0.f, 0.f, 0.f), color(0.f) { }
	LineVertex(const vector3f& v, const Color &c) : position(v), color(c) { }
	vector3f position;
	Color color;
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

	//clear depth and/or color buffer
	virtual bool ClearScreen(bool color=true, bool depth=true) { return false; }
	virtual bool SetClearColor(const Color &c) { return false; }

	//set the model view matrix
	virtual bool SetTransform(const matrix4x4d &m) { return false; }
	virtual bool SetTransform(const matrix4x4f &m) { return false; }
	//set projection matrix
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far) { return false; }
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) { return false; }

	//render state functions
	virtual bool SetBlendMode(BlendMode type) { return false; }
	//virtual bool SetState(Z_WRITE, false) or
	//virtual bool SetZWrite(false) ?

	virtual bool SetLights(int numlights, const Light *l) { return false; }
	virtual bool SetAmbientColor(const Color &c) { return false; }

	//drawing functions
	//2d drawing is generally understood to be for gui use (unlit, ortho projection)
	virtual bool DrawLines(int vertCount, const LineVertex *vertices, LineType type=LINE_SINGLE)  { return false; }
	//flat colour lines (implement multicolour drawing when you need it)
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

protected:
	int m_width;
	int m_height;
};

#endif
