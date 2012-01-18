#ifndef _RENDERER_H
#define _RENDERER_H

#include "libs.h"
#include "vector3.h"
#include "Color.h"

/*
 * Draws points, lines, polys...
 * Terrains and LMRmodels might be too special for this now
 * Would ideally also:
 * set blending modes
 * set fill modes (solid/wireframe)
 * create and destroy context
 * toggle between fullscreen/windowed
 * prepare/endframe/swapbuf (move from Render.h)
 */

//first some data structures
struct vector2f {
	vector2f() : x(0.f), y(0.f) { }
	vector2f(float _v) : x(_v), y(_v) { }
	vector2f(float _x, float _y) : x(_x), y(_y) { }
	float x, y;
};

struct LineVertex {
	LineVertex() : position(0.f, 0.f, 0.f), color(0.f) { }
	LineVertex(const vector3f& v, const Color &c) : position(v), color(c) { }
	vector3f position;
	Color color;
};

struct LineVertex2D {
	LineVertex2D() : position(0.f), color(0.f) { }
	LineVertex2D(float x, float y, const Color &c) : position(x, y), color(c) { }
	LineVertex2D(const vector2f &pos, const Color &c) : position(pos), color(c) { }
	vector2f position;
	Color color;
};

struct ColoredVertex {
	ColoredVertex() : position(0.f), normal(0.f), color(0.f) { }
	ColoredVertex(const vector3f &v, const vector3f &n, const Color &c)
		: position(v), normal(n), color(c) { }
	vector3f position;
	vector3f normal;
	Color color;
};

enum LineType {
	LINE_SINGLE = GL_LINES, //draw one line per two vertices
	LINE_STRIP = GL_LINE_STRIP,  //connect vertices
	LINE_LOOP = GL_LINE_LOOP    //connect vertices,  connect start & end
};

enum BlendModes {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA
};

class Renderer
{
public:
	//return false if failed/unsupported
	//render state functions
	virtual bool SetBlendMode(unsigned int blendType);

	//drawing functions
	virtual bool DrawLines(int vertCount, const LineVertex *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const LineVertex2D *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawTriangleStrip(int vertCount, const ColoredVertex *vertices);
};

#endif
