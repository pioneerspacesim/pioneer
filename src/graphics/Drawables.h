// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DRAWABLES_H
#define _DRAWABLES_H

#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/RenderState.h"
#include "graphics/VertexBuffer.h"
#include "graphics/Material.h"

namespace Graphics {

namespace Drawables {

// A thing that can draw itself using renderer
// (circles, disks, polylines etc)
//------------------------------------------------------------

class Circle {
public:
	Circle(Renderer *renderer, const float radius, const Color &c, RenderState *state);
	Circle(Renderer *renderer, const float radius, const float x, const float y, const float z, const Color &c, RenderState *state);
	Circle(Renderer *renderer, const float radius, const vector3f &center, const Color &c, RenderState *state);
	virtual void Draw(Renderer *renderer);

private:
	void SetupVertexBuffer(const Graphics::VertexArray&, Graphics::Renderer *);
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	RefCountedPtr<Material> m_material;
	Color m_color;
	Graphics::RenderState *m_renderState;
};
//------------------------------------------------------------

// Two-dimensional filled circle
class Disk {
public:
	Disk(Graphics::Renderer *r, Graphics::RenderState*, const Color &c, float radius);
	Disk(Graphics::Renderer *r, RefCountedPtr<Material>, Graphics::RenderState*, const int edges=72, const float radius=1.0f);
	virtual void Draw(Graphics::Renderer *r);

	void SetColor(const Color&);

private:
	void SetupVertexBuffer(const Graphics::VertexArray&, Graphics::Renderer *);
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	RefCountedPtr<Material> m_material;
	Graphics::RenderState *m_renderState;
};
//------------------------------------------------------------

// A three dimensional line between two points
class Line3D {
public:
	Line3D();
	Line3D(const Line3D& b); // this needs an explicit copy constructor due to the std::unique_ptr below
	virtual ~Line3D() {}
	void SetStart(const vector3f &);
	void SetEnd(const vector3f &);
	void SetColor(const Color &);
	virtual void Draw(Renderer*, RenderState*);
private:
	void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);
	void Dirty();

	bool m_refreshVertexBuffer;
	float m_width;
	RefCountedPtr<Material> m_material;
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<Graphics::VertexArray> m_va;
};
//------------------------------------------------------------

// Three dimensional line segments between two points
class Lines {
public:
	Lines();
	void SetData(const Uint32 vertCount, const vector3f *vertices, const Color &color);
	void SetData(const Uint32 vertCount, const vector3f *vertices, const Color *colors);
	void Draw(Renderer*, RenderState*, const PrimitiveType pt = Graphics::LINE_SINGLE);
private:
	void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);

	bool m_refreshVertexBuffer;
	float m_width;
	RefCountedPtr<Material> m_material;
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<VertexArray> m_va;
};
//------------------------------------------------------------

// Screen aligned quad / billboard / pointsprite
class PointSprites {
public:
	PointSprites();
	void SetData(const int count, const vector3f *positions, const matrix4x4f &trans, const float size);
	void Draw(Renderer*, RenderState*, Material*);
private:
	void CreateVertexBuffer(Graphics::Renderer *r, Material *mat, const Uint32 size);

	bool m_refreshVertexBuffer;
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<VertexArray> m_va;
};
//------------------------------------------------------------

// Screen aligned quad / billboard / pointsprite
class Points {
public:
	Points();
	void SetData(Renderer*, const int count, const vector3f *positions, const matrix4x4f &trans, const Color &color, const float size);
	void SetData(Renderer*, const int count, const vector3f *positions, const Color *color, const matrix4x4f &trans, const float size);
	void Draw(Renderer*, RenderState*);
private:
	void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);
	
	bool m_refreshVertexBuffer;
	RefCountedPtr<Material> m_material;
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<VertexArray> m_va;
};
//------------------------------------------------------------

// Three dimensional sphere (subdivided icosahedron) with normals
// and spherical texture coordinates.
class Sphere3D {
public:
	//subdivisions must be 0-4
	Sphere3D(Renderer*, RefCountedPtr<Material> material, Graphics::RenderState*, int subdivisions=0, float scale=1.f);
	virtual void Draw(Renderer *r);

	RefCountedPtr<Material> GetMaterial() const { return m_material; }

private:
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;
	RefCountedPtr<Material> m_material;
	Graphics::RenderState *m_renderState;

	//std::unique_ptr<Surface> m_surface;
	//add a new vertex, return the index
	int AddVertex(VertexArray&, const vector3f &v, const vector3f &n);
	//add three vertex indices to form a triangle
	void AddTriangle(std::vector<Uint16>&, int i1, int i2, int i3);
	void Subdivide(VertexArray&, std::vector<Uint16>&,
		const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
		int i1, int i2, int i3, int depth);
};
//------------------------------------------------------------

// a textured quad with reversed winding
class TexturedQuad {
public:
	TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size, RenderState *state);
	virtual void Draw(Graphics::Renderer *r);
	const Graphics::Texture* GetTexture() const { return m_texture.Get(); }
private:
	RefCountedPtr<Graphics::Texture> m_texture;
	std::unique_ptr<Graphics::Material> m_material;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	Graphics::RenderState *m_renderState;
};
//------------------------------------------------------------

//industry-standard red/green/blue XYZ axis indicator
class Axes3D {
public:
	Axes3D(Graphics::Renderer *r, Graphics::RenderState *state = nullptr);
	virtual void Draw(Graphics::Renderer *r);
private:
	RefCountedPtr<Graphics::Material> m_material;
	RefCountedPtr<VertexBuffer> m_vertexBuffer;
	Graphics::RenderState *m_renderState;
};

Axes3D* GetAxes3DDrawable(Graphics::Renderer *r);

}

}

#endif
