#include "CollisionVisitor.h"
#include "StaticGeometry.h"
#include "graphics/StaticMesh.h"
#include "CollMesh.h"
#include "Group.h"

namespace Newmodel {

CollisionVisitor::CollisionVisitor()
{
	m_collMesh = new CollMesh();
}

void CollisionVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	//XXX assumes one surface
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
			m_indices.push_back((*it));
		}
	}
	//cheat some collision flags
	for(unsigned int i = 0; i < m_indices.size()/3; i++) {
		m_flags.push_back(0);
	}
	assert(m_flags.size() == m_indices.size()/3);
}

CollMesh *CollisionVisitor::CreateCollisionMesh()
{
	GeomTree *t = new GeomTree(
		m_vertices.size()/3, m_indices.size()/3, &m_vertices[0], &m_indices[0], &m_flags[0]);
	m_collMesh->SetGeomTree(t);
	return m_collMesh;
}

}