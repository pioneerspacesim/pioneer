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
 */

class Texture;
class Light;
class RendererLegacy;

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
	QUADS = GL_QUADS, // XXX not available in ES2, replace with strips
	TYPE_POINTS = GL_POINTS
};

enum BlendMode {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_ALPHA_ONE, //XXX what the hell to call this
	BLEND_ALPHA_PREMULT
};

//allowed minimum of GL_MAX_VERTEX_ATTRIBS is 8 on ES2
enum VertexAttribs {
	ATTRIB_POSITION = 0,
	ATTRIB_NORMAL = 1,
	ATTRIB_DIFFUSE = 2,
	ATTRIB_UV0 = 3,
	ATTRIB_UV1 = 4,
	ATTRIB_TANGENT = 5,
	ATTRIB_BITANGENT = 6
	//ATTRIB_CUSTOM?
};
#define NUM_ATTRIBS 4

struct LineVertex {
	LineVertex() : position(0.f, 0.f, 0.f), color(0.f) { }
	LineVertex(const vector3f& v, const Color &c) : position(v), color(c) { }
	vector3f position;
	Color color;
};

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

//a bunch of renderstates and shaders are determined from this
//(can add shaderType or whatever hacks are necessary)
// Idea: to avoid if-else soup in Draw* functions, let renderers subclass Material
// with Apply() and perhaps Cleanup() methods. Users can then request
// materials with Material *mat = renderer->RequestMaterial(...)
struct Material {
	Texture *texture0;
	//Texture *texture1;
	Color diffuse;
	//Color ambient;
	//Color specular;
	bool unlit;
	bool twoSided;

	Render::Shader *shader; //custom glsl prog
	//etc
	Material() : texture0(0), unlit(false), twoSided(false), shader(0) { }
};

// surface with a material
// can have indices
struct Surface {
	Surface() : vertices(0), mat(0), primitiveType(TRIANGLES) { }
	int GetNumVerts() const;
	std::vector<unsigned short> indices;
	VertexArray *vertices;
	Material* mat;
	PrimitiveType primitiveType;

	// multiple surfaces can be buffered in one vbo so need to
	// save starting offset + amount to draw
	//int startVertex;
	//int numVertices; should be samme as vertices->GetNumVerts()
	//int startIndex;
	//int numIndices; should be same as indices.size()
};

// Geometry that changes rarely or never
// May be cached by the renderer
// Can hold multiple surfaces
class StaticMesh {
public:
	StaticMesh();
	StaticMesh(int num_surfaces);
	~StaticMesh();
	int GetNumVerts() const;

	int numSurfaces;
	Surface *surfaces;
	bool cached;

private:
	friend class Renderer;
	friend class RendererLegacy;
	friend class RendererGL2;
	// XXX gl specific hack (stores vbo id)
	unsigned int buffy;
};

// Renderer base, functions return false if
// failed/unsupported
class Renderer
{
public:
	Renderer(int width, int height);
	virtual ~Renderer();

	virtual const char* GetName() = 0;

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;
	//traditionally gui happens between endframe and swapbuffers
	virtual bool SwapBuffers() = 0;

	//set the model view matrix
	virtual bool SetTransform(const matrix4x4d &m) { return false; }

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
	virtual bool DrawTriangles2D(const VertexArray *vertices, const Material *material=0, PrimitiveType type=TRIANGLES)  { return false; }
	//indexed triangle draw
	virtual bool DrawSurface(const Surface *surface) { return false; }
	virtual bool DrawSurface2D(const Surface *surface) { return false; }
	//high amount of textured quads for particles etc
	virtual bool DrawPointSprites(int count, const vector3f *positions, const Material *material, float size) { return false; }
	//complex unchanging geometry that is worthwhile to store in VBOs etc.
	virtual bool DrawStaticMesh(StaticMesh *thing) { return false; }

protected:
	int m_width;
	int m_height;
};

#endif
