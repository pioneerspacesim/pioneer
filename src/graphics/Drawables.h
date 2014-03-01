// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
class Drawable {
protected:
	virtual void Draw(Renderer *r) { };
	virtual ~Drawable() { }
	Graphics::RenderState *m_renderState;
};

class Circle : public Drawable {
public:
	Circle(float radius, const Color &c, RenderState *state) : m_color(c) {
		m_renderState = state;
		for (float theta=0; theta < 2*float(M_PI); theta += 0.05f*float(M_PI)) {
			m_verts.push_back(vector3f(radius*sin(theta), radius*cos(theta), 0));
		}
	}
	Circle(float radius, float x, float y, float z, const Color &c, RenderState *state) : m_color(c) {
		m_renderState = state;
		for (float theta=0; theta < 2*float(M_PI); theta += 0.05f*float(M_PI)) {
			m_verts.push_back(vector3f(radius*sin(theta) + x, radius*cos(theta) + y, z));
		}
	}
	Circle(float radius, const vector3f &center, const Color &c, RenderState *state) : m_color(c) {
		m_renderState = state;
		for (float theta=0; theta < 2*float(M_PI); theta += 0.05f*float(M_PI)) {
			m_verts.push_back(vector3f(radius*sin(theta) + center.x, radius*cos(theta) + center.y, center.z));
		}
	}
	virtual void Draw(Renderer *renderer) {
		renderer->DrawLines(m_verts.size(), &m_verts[0], m_color, m_renderState, LINE_LOOP);
	}

private:
	std::vector<vector3f> m_verts;
	Color m_color;
};

// Two-dimensional filled circle
class Disk : public Drawable {
public:
	Disk(Graphics::Renderer *r, Graphics::RenderState*, const Color &c, float radius);
	Disk(RefCountedPtr<Material> material, Graphics::RenderState*, const int numEdges=72, const float radius=1.0f);
	virtual void Draw(Graphics::Renderer *r);

	void SetColor(const Color&);

private:
	std::unique_ptr<Graphics::VertexArray> m_vertices;
	RefCountedPtr<Material> m_material;
};

//A three dimensional line between two points
class Line3D : public Drawable {
public:
	Line3D();
	void SetStart(const vector3f &);
	void SetEnd(const vector3f &);
	void SetColor(const Color &);
	virtual void Draw(Renderer*, RenderState*);
private:
	vector3f m_points[2];
	Color m_colors[2];
	float m_width;
};

//Three dimensional sphere (subdivided icosahedron) with normals
//and spherical texture coordinates.
class Sphere3D : public Drawable {
public:
	//subdivisions must be 0-4
	Sphere3D(Renderer*, RefCountedPtr<Material> material, Graphics::RenderState*, int subdivisions=0, float scale=1.f);
	virtual void Draw(Renderer *r);

	RefCountedPtr<Material> GetMaterial() const { return m_material; }

private:
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;
	RefCountedPtr<Material> m_material;

	//std::unique_ptr<Surface> m_surface;
	//add a new vertex, return the index
	int AddVertex(VertexArray&, const vector3f &v, const vector3f &n);
	//add three vertex indices to form a triangle
	void AddTriangle(std::vector<Uint16>&, int i1, int i2, int i3);
	void Subdivide(VertexArray&, std::vector<Uint16>&,
		const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
		int i1, int i2, int i3, int depth);
};

// a textured quad with reversed winding
class TexturedQuad : public Drawable {
public:
	TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size, RenderState *state);
	virtual void Draw(Graphics::Renderer *r) {
		r->DrawTriangles(m_vertices.get(), m_renderState, m_material.get(), TRIANGLE_STRIP);
	}

	const Graphics::Texture* GetTexture() const { return m_texture.Get(); }
private:
	RefCountedPtr<Graphics::Texture> m_texture;
	std::unique_ptr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexArray> m_vertices;
};

}

}

#endif
