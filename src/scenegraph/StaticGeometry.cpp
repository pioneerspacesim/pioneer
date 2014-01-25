// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StaticGeometry.h"
#include "NodeVisitor.h"
#include "Model.h"
#include "BaseLoader.h"
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


typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material> > > MaterialContainer;
void StaticGeometry::Save(NodeDatabase &db)
{
    Node::Save(db);
    db.wr->Int32(m_blendMode);
    db.wr->Vector3d(m_boundingBox.min);
    db.wr->Vector3d(m_boundingBox.max);

    db.wr->Int32(m_meshes.size());

    for (auto mesh : m_meshes) {
        db.wr->Int32(mesh->GetNumSurfaces());
        for (auto surf = mesh->SurfacesBegin(); surf != mesh->SurfacesEnd(); ++surf) {
			//do ptr to material name mapping
			const std::string &matname = db.model->GetNameForMaterial((*surf)->GetMaterial().Get());
			db.wr->String(matname);

			//save positions, normals and uvs
			const auto vtxArr = (*surf)->GetVertices();

			db.wr->Int32(vtxArr->GetNumVerts());

			for (const vector3f& pos : vtxArr->position)
				db.wr->Vector3f(pos);
			for (const vector3f& nrm : vtxArr->normal)
				db.wr->Vector3f(nrm);
			for (const vector2f& uv : vtxArr->uv0) {
				db.wr->Float(uv.x);
				db.wr->Float(uv.y);
			}
			//indices
			const auto& indices = (*surf)->GetIndices();
			db.wr->Int32(indices.size());
			for (const auto idx : indices) {
				db.wr->Int16(idx);
			}
        }
    }
}

StaticGeometry *StaticGeometry::Load(NodeDatabase &db)
{
	StaticGeometry *sg = new StaticGeometry(db.loader->GetRenderer());
	Serializer::Reader &rd = *db.rd;

	sg->m_blendMode = static_cast<Graphics::BlendMode>(rd.Int32());
	sg->m_boundingBox.min = rd.Vector3d();
	sg->m_boundingBox.max = rd.Vector3d();

	const Uint32 numMeshes = rd.Int32();
	for (Uint32 mesh = 0; mesh < numMeshes; mesh++) {
		RefCountedPtr<Graphics::StaticMesh> smesh(new Graphics::StaticMesh(Graphics::TRIANGLES));
		const Uint32 numSurfs = rd.Int32();
		for (Uint32 surf = 0; surf < numSurfs; surf++) {

			RefCountedPtr<Graphics::Material> material;
			const std::string matName = rd.String();
			if (starts_with(matName, "decal_")) {
				const unsigned int di = atoi(matName.substr(6).c_str());
				material = db.loader->GetDecalMaterial(di);
			} else
				material = db.model->GetMaterialByName(matName);

			const Graphics::AttributeSet vtxAttribs =
			    Graphics::ATTRIB_POSITION |
			    Graphics::ATTRIB_NORMAL |
			    Graphics::ATTRIB_UV0;

			const Uint32 numVerts = rd.Int32();
			Graphics::VertexArray *vts = new Graphics::VertexArray(vtxAttribs, numVerts);
			for (Uint32 i = 0; i < numVerts; i++)
				vts->position.push_back(rd.Vector3f());

			for (Uint32 i = 0; i < numVerts; i++)
				vts->normal.push_back(rd.Vector3f());

			for (Uint32 i = 0; i < numVerts; i++) {
				const float x = rd.Float();
				const float y = rd.Float();
				vts->uv0.push_back(vector2f(x, y));
			}

			RefCountedPtr<Graphics::Surface> surface(new Graphics::Surface(Graphics::TRIANGLES, vts, material));
			auto& idxArr = surface->GetIndices();
			const Uint32 nidx = rd.Int32();
			idxArr.reserve(nidx);
			for (Uint32 i = 0; i < nidx; i++)
				idxArr.push_back(rd.Int16());

			smesh->AddSurface(surface);
		}
		sg->AddMesh(smesh);
	}
	return sg;
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
