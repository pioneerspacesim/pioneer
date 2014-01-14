// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StaticGeometry.h"
#include "NodeVisitor.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/Surface.h"
#include "graphics/Material.h"

namespace SceneGraph {

StaticGeometry::StaticGeometry(Graphics::Renderer *r)
: Node(r, NODE_SOLID)
, m_blendMode(Graphics::BLEND_SOLID)
, m_bDisableDepthWrite(false)
{
}

StaticGeometry::~StaticGeometry()
{
}

StaticGeometry::StaticGeometry(const StaticGeometry &sg, NodeCopyCache *cache)
: Node(sg, cache)
, m_boundingBox(sg.m_boundingBox)
, m_blendMode(sg.m_blendMode)
, m_meshes(sg.m_meshes)
, m_bDisableDepthWrite(sg.m_bDisableDepthWrite)
{
}

Node* StaticGeometry::Clone(NodeCopyCache *cache)
{
	return this; //geometries are shared
}

void StaticGeometry::Accept(NodeVisitor &nv)
{
	nv.ApplyStaticGeometry(*this);
}

void StaticGeometry::Render(const matrix4x4f &trans, const RenderData *rd)
{
	assert(rd);
	Graphics::Renderer *r = GetRenderer();
	r->SetTransform(trans);
	if (m_blendMode != Graphics::BLEND_SOLID)
		r->SetBlendMode(m_blendMode);

	if (m_bDisableDepthWrite) {
		m_renderer->SetDepthWrite(false);
		for (MeshContainer::iterator it = m_meshes.begin(), itEnd=m_meshes.end(); it != itEnd; ++it) {
			r->DrawStaticMesh(it->Get());
		}
		m_renderer->SetDepthWrite(true);
	} else {
		for (MeshContainer::iterator it = m_meshes.begin(), itEnd = m_meshes.end(); it != itEnd; ++it) {
			r->DrawStaticMesh(it->Get());
		}
	}

	//DrawBoundingBox(r, m_boundingBox);
}

void StaticGeometry::AddMesh(RefCountedPtr<Graphics::StaticMesh> mesh)
{
	m_meshes.push_back(mesh);
}

void StaticGeometry::DrawBoundingBox(const Aabb &bb)
{
	// TODO check entire function, because Color is now Color4ub
	vector3f min(bb.min.x, bb.min.y, bb.min.z);
	vector3f max(bb.max.x, bb.max.y, bb.max.z);
	vector3f fbl(min.x, min.y, min.z); //front bottom left
	vector3f fbr(max.x, min.y, min.z); //front bottom right
	vector3f ftl(min.x, max.y, min.z); //front top left
	vector3f ftr(max.x, max.y, min.z); //front top right
	vector3f rtl(min.x, max.y, max.z); //rear top left
	vector3f rtr(max.x, max.y, max.z); //rear top right
	vector3f rbl(min.x, min.y, max.z); //rear bottom left
	vector3f rbr(max.x, min.y, max.z); //rear bottom right

	Graphics::VertexArray *vts = new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);
	Color c(Color::WHITE);
	//vertices
	vts->Add(fbl, c); //0
	vts->Add(fbr, c); //1
	vts->Add(ftl, c); //2
	vts->Add(ftr, c); //3

	vts->Add(rtl, c); //4
	vts->Add(rtr, c); //5
	vts->Add(rbl, c); //6
	vts->Add(rbr, c); //7

	RefCountedPtr<Graphics::Material> mat(Graphics::vtxColorMaterial);
	Graphics::Surface surf(Graphics::TRIANGLES, vts, mat);

	//indices
	std::vector<unsigned short> &ind = surf.GetIndices();
	//Front face
	ind.push_back(3);
	ind.push_back(1);
	ind.push_back(0);

	ind.push_back(0);
	ind.push_back(2);
	ind.push_back(3);

	//Rear face
	ind.push_back(7);
	ind.push_back(5);
	ind.push_back(6);

	ind.push_back(6);
	ind.push_back(5);
	ind.push_back(4);

	//Top face
	ind.push_back(4);
	ind.push_back(5);
	ind.push_back(3);

	ind.push_back(3);
	ind.push_back(2);
	ind.push_back(4);

	//bottom face
	ind.push_back(1);
	ind.push_back(7);
	ind.push_back(6);

	ind.push_back(6);
	ind.push_back(0);
	ind.push_back(1);

	//left face
	ind.push_back(0);
	ind.push_back(6);
	ind.push_back(4);

	ind.push_back(4);
	ind.push_back(2);
	ind.push_back(0);

	//right face
	ind.push_back(5);
	ind.push_back(7);
	ind.push_back(1);

	ind.push_back(1);
	ind.push_back(3);
	ind.push_back(5);

	Graphics::Renderer *r = GetRenderer();
	r->SetWireFrameMode(true);
	r->DrawSurface(&surf);
	r->SetWireFrameMode(false);
}

}
