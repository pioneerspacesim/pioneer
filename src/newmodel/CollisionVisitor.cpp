// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionVisitor.h"
#include "CollMesh.h"
#include "Group.h"
#include "MatrixTransform.h"
#include "StaticGeometry.h"
#include "graphics/StaticMesh.h"
#include "graphics/Surface.h"

namespace Newmodel {

CollisionVisitor::CollisionVisitor()
{
	m_collMesh = new CollMesh();
}

void CollisionVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	if (m_matrixStack.empty()) {
		m_collMesh->GetAabb().Update(g.m_boundingBox.min);
		m_collMesh->GetAabb().Update(g.m_boundingBox.max);
	} else {
		//XXX should transform each corner instead?
		const matrix4x4f &matrix = m_matrixStack.back();
		vector3f min = matrix * vector3f(g.m_boundingBox.min);
		vector3f max = matrix * vector3f(g.m_boundingBox.max);
		m_collMesh->GetAabb().Update(vector3d(min));
		m_collMesh->GetAabb().Update(vector3d(max));
	}
	m_boundingRadius = m_collMesh->GetAabb().GetBoundingRadius();
}

void CollisionVisitor::ApplyMatrixTransform(MatrixTransform &m)
{
	matrix4x4f matrix = matrix4x4f::Identity();
	if (!m_matrixStack.empty()) matrix = m_matrixStack.back();

	m_matrixStack.push_back(matrix * m.GetTransform());
	m.Traverse(*this);
	m_matrixStack.pop_back();
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
	m_collMesh->SetBoundingRadius(bb.GetBoundingRadius());
	return m_collMesh;
}

}
