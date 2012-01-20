#ifndef _RENDERER_H
#define _RENDERER_H

#include "libs.h"
#include "vector3.h"
#include "Color.h"

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
 */

class Texture;

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

enum VertexAttribs {
	ATTRIB_POSITION = 0,
	ATTRIB_NORMAL = 1,
	ATTRIB_DIFFUSE = 2
};
#define NUM_ATTRIBS 3

struct VertexArray {
	//no attribs are enabled by default
	VertexArray();
	//reserve space for vertice, specifying attributes to be used
	//(positions are always on)
	VertexArray(int size, bool colors=true);
	~VertexArray();

	virtual void Add(const vector3f &v);
	virtual void Add(const vector3f &v, const Color &c);
	//vector3f* position;
	std::vector<vector3f> position;
	//vector3f* normal;
	std::vector<Color> diffuse;
	// two uvs should be enough for everyone
	std::vector<vector2f> uv0;
	//vector2f* uv1;
	//future stuff
	//vector3f* tangent;
	//vector3f* binormal;
	//can be set by user
	int numVertices;
	//track used attributes
	bool attribs[NUM_ATTRIBS];
};

//shader is determined from this
//(can add shaderType or whatever is necessary)
struct Material {
	Material() { memset(this, 0, sizeof(Material)); }
	Texture *texture0;
	Texture *texture1;
	Color diffuse;
	Color ambient;
	Color specular;
	//etc
};

// surface with a material
// can have indices
struct Surface {
	unsigned short* indices;
	unsigned int numIndices;
	VertexArray vertices;
	Material* mat;
	//Texture separately or part of material?
};

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
	QUADS = GL_QUADS
};

enum BlendModes {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_ALPHA_ONE, //XXX what the hell to call this
	BLEND_ALPHA_PREMULT
};

class Renderer
{
public:
	Renderer(int width, int height);
	//return false if failed/unsupported
	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	//render state functions
	virtual bool SetBlendMode(unsigned int blendType);

	//drawing functions
	virtual bool DrawLines(int vertCount, const LineVertex *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const LineVertex2D *vertices, unsigned int lineType=LINE_SINGLE);
	virtual bool DrawTriangleStrip(int vertCount, const ColoredVertex *vertices);
	//yes, this is different, trying out what works best
	virtual bool DrawTriangleFan(int vertCount, const vector3f *vertices, const Color *colors);
	//unindexed triangle draw
	//virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, unsigned int type=TRIANGLES) { };
	//indexed triangle draw
	//virtual bool DrawSurface(const Surface *surface) { };
	virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, unsigned int type=TRIANGLES);
	virtual bool DrawTriangles2D(const VertexArray *vertices, const Material *material=0, unsigned int type=TRIANGLES);

protected:
	int m_width;
	int m_height;
};

#endif
