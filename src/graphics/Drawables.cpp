// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Drawables.h"
#include "Texture.h"

namespace Graphics {

namespace Drawables {

Disk::Disk(Graphics::Renderer *r, const Color &c, float rad)
{
	m_vertices.reset(new VertexArray(ATTRIB_POSITION));
	m_material.Reset(r->CreateMaterial(MaterialDescriptor()));
	m_material->diffuse = c;

	m_vertices->Add(vector3f(0.f, 0.f, 0.f));
	for (int i = 72; i >= 0; i--) {
		m_vertices->Add(vector3f(
			0.f+sinf(DEG2RAD(i*5.f))*rad,
			0.f+cosf(DEG2RAD(i*5.f))*rad,
			0.f));
	}
}

Disk::Disk(RefCountedPtr<Material> material, const int numEdges/*=72*/, const float radius/*=1.0f*/) : m_material(material)
{
	m_vertices.reset(new VertexArray(ATTRIB_POSITION));

	m_vertices->Add(vector3f(0.f, 0.f, 0.f));
	const float edgeStep = 360.0f / float(numEdges);
	for (int i = numEdges; i >= 0; i--) {
		m_vertices->Add(vector3f(
			0.f+sinf(DEG2RAD(i*edgeStep))*radius,
			0.f+cosf(DEG2RAD(i*edgeStep))*radius,
			0.f));
	}
}

void Disk::Draw(Renderer *r)
{
	r->DrawTriangles(m_vertices.get(), m_material.Get(), TRIANGLE_FAN);
}

void Disk::SetColor(const Color &c)
{
	m_material->diffuse = c;
}

Line3D::Line3D()
{
	m_points[0] = vector3f(0.f);
	m_points[1] = vector3f(0.f);
	m_colors[0] = Color(0);
	m_colors[1] = Color(255);
	m_width     = 2.f; // XXX bug in Radeon drivers will cause crash in glLineWidth if width >= 3
}

void Line3D::SetStart(const vector3f &s)
{
	m_points[0] = s;
}

void Line3D::SetEnd(const vector3f &e)
{
	m_points[1] = e;
}

void Line3D::SetColor(const Color &c)
{
	m_colors[0]  = c;
	m_colors[1]  = c;
	m_colors[1]  *= 0.5; //XXX hardcoded appearance
}

void Line3D::Draw(Renderer *renderer)
{
	// XXX would be nicer to draw this as a textured triangle strip
	// can't guarantee linewidth support
	glLineWidth(m_width);
	renderer->DrawLines(2, m_points, m_colors);
	glLineWidth(1.f);
}

static const float ICOSX = 0.525731112119133f;
static const float ICOSZ = 0.850650808352039f;

static const vector3f icosahedron_vertices[12] = {
	vector3f(-ICOSX, 0.0, ICOSZ), vector3f(ICOSX, 0.0, ICOSZ), vector3f(-ICOSX, 0.0, -ICOSZ), vector3f(ICOSX, 0.0, -ICOSZ),
	vector3f(0.0, ICOSZ, ICOSX), vector3f(0.0, ICOSZ, -ICOSX), vector3f(0.0, -ICOSZ, ICOSX), vector3f(0.0, -ICOSZ, -ICOSX),
	vector3f(ICOSZ, ICOSX, 0.0), vector3f(-ICOSZ, ICOSX, 0.0), vector3f(ICOSZ, -ICOSX, 0.0), vector3f(-ICOSZ, -ICOSX, 0.0)
};

static const int icosahedron_faces[20][3] = {
	{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
	{8,10,1}, {8,3,10},{5,3,8}, {5,2,3}, {2,7,3},
	{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
	{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};

Sphere3D::Sphere3D(RefCountedPtr<Material> mat, int subdivs, float scale)
{
	subdivs = Clamp(subdivs, 0, 4);
	scale = fabs(scale);
	matrix4x4f trans = matrix4x4f::Identity();
	trans.Scale(scale, scale, scale);
	m_surface.reset(new Surface(TRIANGLES, new VertexArray(ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0), mat));

	//initial vertices
	int i;
	int vi[12];
	for (i=0; i<12; i++) {
		const vector3f &v = icosahedron_vertices[i];
		vi[i] = AddVertex(trans * v, v);
	}

	//subdivide
	for (i=0; i<20; i++) {
		Subdivide(trans, icosahedron_vertices[icosahedron_faces[i][0]],
				icosahedron_vertices[icosahedron_faces[i][1]],
				icosahedron_vertices[icosahedron_faces[i][2]],
				vi[icosahedron_faces[i][0]],
				vi[icosahedron_faces[i][1]],
				vi[icosahedron_faces[i][2]],
				subdivs);
	}
}

void Sphere3D::Draw(Renderer *r)
{
	r->DrawSurface(m_surface.get());
}

int Sphere3D::AddVertex(const vector3f &v, const vector3f &n)
{
	m_surface->GetVertices()->position.push_back(v);
	m_surface->GetVertices()->normal.push_back(n);
	//http://www.mvps.org/directx/articles/spheremap.htm
	m_surface->GetVertices()->uv0.push_back(vector2f(asinf(n.x)/M_PI+0.5f, asinf(n.y)/M_PI+0.5f));
	return m_surface->GetNumVerts() - 1;
}

void Sphere3D::AddTriangle(int i1, int i2, int i3)
{
	std::vector<unsigned short> &indices = m_surface->GetIndices();
	indices.push_back(i1);
	indices.push_back(i2);
	indices.push_back(i3);
}

void Sphere3D::Subdivide(const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
		const int i1, const int i2, const int i3, int depth)
{
	if (depth == 0) {
		AddTriangle(i1, i3, i2);
		return;
	}

	const vector3f v12 = (v1+v2).Normalized();
	const vector3f v23 = (v2+v3).Normalized();
	const vector3f v31 = (v3+v1).Normalized();
	const int i12 = AddVertex(trans * v12, v12);
	const int i23 = AddVertex(trans * v23, v23);
	const int i31 = AddVertex(trans * v31, v31);
	Subdivide(trans, v1, v12, v31, i1, i12, i31, depth-1);
	Subdivide(trans, v2, v23, v12, i2, i23, i12, depth-1);
	Subdivide(trans, v3, v31, v23, i3, i31, i23, depth-1);
	Subdivide(trans, v12, v23, v31, i12, i23, i31, depth-1);
}

// a textured quad with reversed winding
TexturedQuad::TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size) : m_texture(RefCountedPtr<Graphics::Texture>(texture)) 
{
	m_vertices.reset(new VertexArray(ATTRIB_POSITION | ATTRIB_UV0));
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	m_material.reset(r->CreateMaterial(desc));
	m_material->texture0 = m_texture.Get();

	// these might need to be reversed
	const vector2f texPos = vector2f(0.0f);
	const vector2f texSize = m_texture->GetDescriptor().texSize;

	m_vertices->Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
	m_vertices->Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(texPos.x,           texPos.y));
	m_vertices->Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));
	m_vertices->Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(texPos.x+texSize.x, texPos.y));
}

}

}
