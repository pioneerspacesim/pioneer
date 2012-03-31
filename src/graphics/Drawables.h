#ifndef _DRAWABLES_H
#define _DRAWABLES_H

#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/Surface.h"
#include "graphics/VertexArray.h"

namespace Graphics {

namespace Drawables {

// A thing that can draw itself using renderer
// (circles, disks, polylines etc)
class Drawable {
protected:
	virtual void Draw(Renderer *r) = 0;
};

class Circle {
public:
	Circle(float radius, const Color &c) : m_color(c) {
		for (float theta=0; theta < 2*M_PI; theta += 0.05*M_PI) {
			m_verts.push_back(vector3f(radius*sin(theta), radius*cos(theta), 0));
		}
	}
	virtual void Draw(Renderer *renderer) {
		renderer->DrawLines(m_verts.size(), &m_verts[0], m_color, LINE_LOOP);
	}

private:
	std::vector<vector3f> m_verts;
	Color m_color;
};

//A three dimensional line between two points
class Line3D : public Drawable {
public:
	Line3D();
	void SetStart(const vector3f &);
	void SetEnd(const vector3f &);
	void SetColor(const Color &);
	virtual void Draw(Renderer *r);
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
	Sphere3D(RefCountedPtr<Material> material, int subdivisions=0, float scale=1.f);
	virtual void Draw(Renderer *r);

	RefCountedPtr<Material> GetMaterial() { return m_surface->GetMaterial(); }

private:
	ScopedPtr<Surface> m_surface;
	//add a new vertex, return the index
	int AddVertex(const vector3f &v, const vector3f &n);
	//add three vertex indices to form a triangle
	void AddTriangle(int i1, int i2, int i3);
	void Subdivide(const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
		int i1, int i2, int i3, int depth);
};

}

}

#endif
