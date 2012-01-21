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
class Light;

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

enum VertexAttribs {
	ATTRIB_POSITION = 0,
	ATTRIB_NORMAL = 1,
	ATTRIB_DIFFUSE = 2,
	ATTRIB_UV0 = 3
};
#define NUM_ATTRIBS 4

// this is a generic collection of vertex attributes. Renderers do
// whatever they need to do with regards to the attribute set.
// Presence of an attribute is checked using vector size, so users are trusted
// to provide matching number of attributes
struct VertexArray {
	VertexArray();
	//reserve space for vertice, specifying attributes to be used
	//(positions are always on)
	VertexArray(int size, bool colors, bool normals);
	~VertexArray();

	virtual unsigned int GetNumVerts() const;
	virtual void Clear();
	virtual void Add(const vector3f &v);
	virtual void Add(const vector3f &v, const Color &c);
	virtual void Add(const vector3f &v, const Color &c, const vector3f &normal);
	//common for UI
	virtual void Add(const vector3f &v, const Color &c, const vector2f &uv);
	//virtual void Reserve(unsigned int howmuch)

	std::vector<vector3f> position;
	std::vector<vector3f> normal;
	std::vector<Color> diffuse;
	// two uvs should be enough for everyone
	std::vector<vector2f> uv0;
	//vector2f* uv1;
	//future stuff
	//vector3f* tangent;
	//vector3f* binormal;
};

//shader is determined from this
//(can add shaderType or whatever is necessary)
struct Material {
	Material() { memset(this, 0, sizeof(Material)); }
	Texture *texture0;
	//Texture *texture1;
	//Color diffuse;
	//Color ambient;
	//Color specular;
	bool unlit;
	//etc
};

// surface with a material
// can have indices
struct Surface {
	std::vector<unsigned short> indices;
	VertexArray *vertices;
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
	QUADS = GL_QUADS // XXX not available in ES2, replace with strips
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

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;
	//traditionally gui happens between endframe and swapbuffers
	virtual bool SwapBuffers() = 0;

	//render state functions
	virtual bool SetBlendMode(BlendMode type) { return false; }
	//virtual bool SetState(Z_WRITE, false) or
	//virtual bool SetZWrite(false) ?

	virtual bool SetLights(int numlights, const Light *l) { return false; }

	//drawing functions
	//2d drawing is generally understood to be for gui use (unlit, ortho projection)
	virtual bool DrawLines(int vertCount, const LineVertex *vertices, LineType type=LINE_SINGLE)  { return false; }
	virtual bool DrawLines2D(int vertCount, const LineVertex2D *vertices, LineType type=LINE_SINGLE)  { return false; }
	//unindexed triangle draw
	virtual bool DrawTriangles(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES)  { return false; }
	virtual bool DrawTriangles2D(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES)  { return false; }
	//indexed triangle draw (only triangles, no strips or fans)
	virtual bool DrawSurface(const Surface *surface) { return false; }
	virtual bool DrawSurface2D(const Surface *surface) { return false; }

protected:
	int m_width;
	int m_height;
};

#endif
