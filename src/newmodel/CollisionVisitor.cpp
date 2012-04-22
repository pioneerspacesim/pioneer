#include "CollisionVisitor.h"
#include "StaticGeometry.h"
#include "graphics/StaticMesh.h"
#include "graphics/Surface.h"
#include "CollMesh.h"
#include "Group.h"

namespace Newmodel {

CollisionVisitor::CollisionVisitor()
{
	m_collMesh = new CollMesh();
}

void CollisionVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	m_collMesh->GetAabb().Update(g.m_boundingBox.min);
	m_collMesh->GetAabb().Update(g.m_boundingBox.max);
	m_boundingRadius = m_collMesh->GetAabb().GetBoundingRadius();
#if 0
	const Graphics::StaticMesh *mesh = g.GetMesh();
	for (Graphics::StaticMesh::SurfaceIterator surface = mesh->SurfacesBegin();
		surface != mesh->SurfacesEnd();
		++surface)
	{
		const Graphics::VertexArray *vts = (*surface)->GetVertices();
		for (unsigned int v = 0; v < vts->GetNumVerts(); v++) {
			const vector3f pos(vts->position[v]);
			m_vertices.push_back(pos.x);
			m_vertices.push_back(pos.y);
			m_vertices.push_back(pos.z);
			m_collMesh->GetAabb().Update(vector3d(pos));
		}
		const std::vector<unsigned short> &indices = (*surface)->GetIndices();
		for (std::vector<unsigned short>::const_iterator it = indices.begin();
			it != indices.end();
			++it)
		{
			m_indices.push_back(m_offset + (*it));
		}
		//XXX cheat some collision flags
		for(unsigned int i = 0; i < indices.size()/3; i++) {
			m_flags.push_back(0);
		}
		m_offset += m_vertices.size();
	}
	assert(m_flags.size() == m_indices.size()/3);
#endif
}

CollMesh *CollisionVisitor::CreateCollisionMesh()
{
	const Aabb &bb = m_collMesh->GetAabb();
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
	std::vector<vector3f> &vts = m_collMesh->m_vertices;
	vts.push_back(fbl); //0
	vts.push_back(fbr); //1
	vts.push_back(ftl); //2
	vts.push_back(ftr); //3

	vts.push_back(rtl); //4
	vts.push_back(rtr); //5
	vts.push_back(rbl); //6
	vts.push_back(rbr); //7

	//indices
	std::vector<int> &ind = m_collMesh->m_indices;
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

	for(unsigned int i = 0; i < ind.size()/3; i++) {
		m_collMesh->m_flags.push_back(0);
	}

	GeomTree *t = new GeomTree(
		vts.size(), ind.size()/3, reinterpret_cast<float*>(&vts[0]), &ind[0], &m_collMesh->m_flags[0]);
	m_collMesh->SetGeomTree(t);
	return m_collMesh;
}

}
