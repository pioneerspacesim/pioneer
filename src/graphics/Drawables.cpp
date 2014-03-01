// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Drawables.h"
#include "Texture.h"

namespace Graphics {

namespace Drawables {

Disk::Disk(Graphics::Renderer *r, Graphics::RenderState *state, const Color &c, float rad)
{
	m_renderState = state;

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

Disk::Disk(RefCountedPtr<Material> material, Graphics::RenderState *state, const int numEdges/*=72*/, const float radius/*=1.0f*/)
	: m_material(material)
{
	m_renderState = state;

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
	r->DrawTriangles(m_vertices.get(), m_renderState, m_material.Get(), TRIANGLE_FAN);
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

void Line3D::Draw(Renderer *renderer, RenderState *rs)
{
	// XXX would be nicer to draw this as a textured triangle strip
	// can't guarantee linewidth support
	glLineWidth(m_width);
	renderer->DrawLines(2, m_points, m_colors, rs);
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

struct Sphere3DVertex {
	vector3f pos;
	vector3f nrm;
	vector2f uv;
};

Sphere3D::Sphere3D(Renderer *renderer, RefCountedPtr<Material> mat, Graphics::RenderState *state, int subdivs, float scale)
{
	m_material = mat;
	m_renderState = state;

	subdivs = Clamp(subdivs, 0, 4);
	scale = fabs(scale);
	matrix4x4f trans = matrix4x4f::Identity();
	trans.Scale(scale, scale, scale);

	//m_surface.reset(new Surface(TRIANGLES, new VertexArray(ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0), mat));
	//reserve some data
	VertexArray vts(ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0, 256);
	std::vector<Uint16> indices;

	//initial vertices
	int vi[12];
	for (int i=0; i<12; i++) {
		const vector3f &v = icosahedron_vertices[i];
		vi[i] = AddVertex(vts, trans * v, v);
	}

	//subdivide
	for (int i=0; i<20; i++) {
		Subdivide(vts, indices, trans, icosahedron_vertices[icosahedron_faces[i][0]],
				icosahedron_vertices[icosahedron_faces[i][1]],
				icosahedron_vertices[icosahedron_faces[i][2]],
				vi[icosahedron_faces[i][0]],
				vi[icosahedron_faces[i][1]],
				vi[icosahedron_faces[i][2]],
				subdivs);
	}

	//Create vtx & index buffers and copy data
	VertexBufferDesc vbd;
	vbd.attrib[0].semantic = ATTRIB_POSITION;
	vbd.attrib[0].format   = ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[0].offset   = offsetof(Sphere3DVertex, pos);
	vbd.attrib[1].semantic = ATTRIB_NORMAL;
	vbd.attrib[1].format   = ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].offset   = offsetof(Sphere3DVertex, nrm);
	vbd.attrib[2].semantic = ATTRIB_UV0;
	vbd.attrib[2].format   = ATTRIB_FORMAT_FLOAT2;
	vbd.attrib[2].offset   = offsetof(Sphere3DVertex, uv);
	vbd.stride = sizeof(Sphere3DVertex);
	vbd.numVertices = vts.GetNumVerts();
	vbd.usage = BUFFER_USAGE_STATIC;
	m_vertexBuffer.reset(renderer->CreateVertexBuffer(vbd));

	auto vtxPtr = m_vertexBuffer->Map<Sphere3DVertex>(Graphics::BUFFER_MAP_WRITE);
	for (Uint32 i = 0; i < vts.GetNumVerts(); i++) {
		vtxPtr->pos = vts.position[i];
		vtxPtr->nrm = vts.normal[i];
		vtxPtr->uv = vts.uv0[i];
		vtxPtr++;
	}
	m_vertexBuffer->Unmap();

	m_indexBuffer.reset(renderer->CreateIndexBuffer(indices.size(), BUFFER_USAGE_STATIC));
	Uint16 *idxPtr = m_indexBuffer->Map(Graphics::BUFFER_MAP_WRITE);
	for (auto it : indices) {
		*idxPtr = it;
		idxPtr++;
	}
	m_indexBuffer->Unmap();
}

void Sphere3D::Draw(Renderer *r)
{
	r->DrawBufferIndexed(m_vertexBuffer.get(), m_indexBuffer.get(), m_renderState, m_material.Get());
}

int Sphere3D::AddVertex(VertexArray &vts, const vector3f &v, const vector3f &n)
{
	vts.position.push_back(v);
	vts.normal.push_back(n);
	//http://www.mvps.org/directx/articles/spheremap.htm
	vts.uv0.push_back(vector2f(asinf(n.x)/M_PI+0.5f, asinf(n.y)/M_PI+0.5f));
	return vts.GetNumVerts() - 1;
}

void Sphere3D::AddTriangle(std::vector<Uint16> &indices, int i1, int i2, int i3)
{
	indices.push_back(i1);
	indices.push_back(i2);
	indices.push_back(i3);
}

void Sphere3D::Subdivide(VertexArray &vts, std::vector<Uint16> &indices,
		const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
		const int i1, const int i2, const int i3, int depth)
{
	if (depth == 0) {
		AddTriangle(indices, i1, i3, i2);
		return;
	}

	const vector3f v12 = (v1+v2).Normalized();
	const vector3f v23 = (v2+v3).Normalized();
	const vector3f v31 = (v3+v1).Normalized();
	const int i12 = AddVertex(vts, trans * v12, v12);
	const int i23 = AddVertex(vts, trans * v23, v23);
	const int i31 = AddVertex(vts, trans * v31, v31);
	Subdivide(vts, indices, trans, v1, v12, v31, i1, i12, i31, depth-1);
	Subdivide(vts, indices, trans, v2, v23, v12, i2, i23, i12, depth-1);
	Subdivide(vts, indices, trans, v3, v31, v23, i3, i31, i23, depth-1);
	Subdivide(vts, indices, trans, v12, v23, v31, i12, i23, i31, depth-1);
}

// a textured quad with reversed winding
TexturedQuad::TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size, RenderState *state)
	: m_texture(RefCountedPtr<Graphics::Texture>(texture))
{
	assert(state);
	m_renderState = state;

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
