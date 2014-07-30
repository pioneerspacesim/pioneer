// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionVisitor.h"
#include "CollisionGeometry.h"
#include "CollMesh.h"
#include "Group.h"
#include "MatrixTransform.h"
#include "StaticGeometry.h"

namespace SceneGraph {
CollisionVisitor::CollisionVisitor()
: m_properData(false)
, m_totalTris(0)
{
	m_collMesh.Reset(new CollMesh());
	m_vertices.reserve(300);
	m_indices.reserve(300 * 3);
	m_flags.reserve(300);
}

void CollisionVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	if (m_matrixStack.empty()) {
		m_collMesh->GetAabb().Update(g.m_boundingBox.min);
		m_collMesh->GetAabb().Update(g.m_boundingBox.max);
	} else {
		const matrix4x4f &matrix = m_matrixStack.back();
		vector3f min = matrix * vector3f(g.m_boundingBox.min);
		vector3f max = matrix * vector3f(g.m_boundingBox.max);
		m_collMesh->GetAabb().Update(vector3d(min));
		m_collMesh->GetAabb().Update(vector3d(max));
	}
}

void CollisionVisitor::ApplyMatrixTransform(MatrixTransform &m)
{
	matrix4x4f matrix = matrix4x4f::Identity();
	if (!m_matrixStack.empty()) matrix = m_matrixStack.back();

	m_matrixStack.push_back(matrix * m.GetTransform());
	m.Traverse(*this);
	m_matrixStack.pop_back();
}

void CollisionVisitor::ApplyCollisionGeometry(CollisionGeometry &cg)
{
	using std::vector;

	if (cg.IsDynamic()) return ApplyDynamicCollisionGeometry(cg);

	const matrix4x4f matrix = m_matrixStack.empty() ? matrix4x4f::Identity() : m_matrixStack.back();

	//copy data (with index offset)
	int idxOffset = m_vertices.size();
	for (vector<vector3f>::const_iterator it = cg.GetVertices().begin(); it != cg.GetVertices().end(); ++it) {
		const vector3f pos = matrix * (*it);
		m_vertices.push_back(pos);
		m_collMesh->GetAabb().Update(pos.x, pos.y, pos.z);
	}

	for (vector<Uint16>::const_iterator it = cg.GetIndices().begin(); it != cg.GetIndices().end(); ++it)
		m_indices.push_back(*it + idxOffset);

	//at least some of the geoms should be default collision
	if (cg.GetTriFlag() == 0)
		m_properData = true;

	for (unsigned int i = 0; i < cg.GetIndices().size() / 3; i++)
		m_flags.push_back(cg.GetTriFlag());
}

void CollisionVisitor::ApplyDynamicCollisionGeometry(CollisionGeometry &cg)
{
	//don't transform geometry, one geomtree per cg, create tree right away

	const int numVertices = cg.GetVertices().size();
	const int numIndices = cg.GetIndices().size();
	const int numTris = numIndices / 3;
	vector3f *vertices = new vector3f[numVertices];
	Uint16 *indices = new Uint16[numIndices];
	unsigned int *triFlags = new unsigned int[numTris];

	for (int i = 0; i < numVertices; i++)
		vertices[i] = cg.GetVertices()[i];

	for (int i = 0; i < numIndices; i++)
		indices[i] = cg.GetIndices()[i];

	for (int i = 0; i < numTris; i++)
		triFlags[i] = cg.GetTriFlag();

	//create geomtree
	//takes ownership of data
	GeomTree *gt = new GeomTree(
		numVertices, numTris,
		reinterpret_cast<float*>(vertices),
		indices, triFlags);
	cg.SetGeomTree(gt);

	m_collMesh->AddDynGeomTree(gt);

	m_totalTris += numTris;
}

void CollisionVisitor::AabbToMesh(const Aabb &bb)
{
	std::vector<vector3f> &vts = m_vertices;
	std::vector<Uint16> &ind = m_indices;
	const int offs = vts.size();

	const vector3f min(bb.min.x, bb.min.y, bb.min.z);
	const vector3f max(bb.max.x, bb.max.y, bb.max.z);
	const vector3f fbl(min.x, min.y, min.z); //front bottom left
	const vector3f fbr(max.x, min.y, min.z); //front bottom right
	const vector3f ftl(min.x, max.y, min.z); //front top left
	const vector3f ftr(max.x, max.y, min.z); //front top right
	const vector3f rtl(min.x, max.y, max.z); //rear top left
	const vector3f rtr(max.x, max.y, max.z); //rear top right
	const vector3f rbl(min.x, min.y, max.z); //rear bottom left
	const vector3f rbr(max.x, min.y, max.z); //rear bottom right

	vts.push_back(fbl); //0
	vts.push_back(fbr); //1
	vts.push_back(ftl); //2
	vts.push_back(ftr); //3

	vts.push_back(rtl); //4
	vts.push_back(rtr); //5
	vts.push_back(rbl); //6
	vts.push_back(rbr); //7

#define ADDTRI(_i1, _i2, _i3) \
	ind.push_back(offs + _i1); \
	ind.push_back(offs + _i2); \
	ind.push_back(offs + _i3);
	//indices
	//Front face
	ADDTRI(3, 1, 0);
	ADDTRI(0, 2, 3);

	//Rear face
	ADDTRI(7, 5, 6);
	ADDTRI(6, 5, 4);

	//Top face
	ADDTRI(4, 5, 3);
	ADDTRI(3, 2, 4);

	//bottom face
	ADDTRI(1, 7, 6);
	ADDTRI(6, 0, 1);

	//left face
	ADDTRI(0, 6, 4);
	ADDTRI(4, 2, 0);

	//right face
	ADDTRI(5, 7, 1);
	ADDTRI(1, 3, 5);
#undef ADDTRI

	for(unsigned int i = 0; i < ind.size()/3; i++)
		m_flags.push_back(0);
}

RefCountedPtr<CollMesh> CollisionVisitor::CreateCollisionMesh()
{
	//convert from model AABB if no collisiongeoms found
	if (!m_properData)
		AabbToMesh(m_collMesh->GetAabb());

	assert(m_collMesh->GetGeomTree() == 0);
	assert(!m_vertices.empty() && !m_indices.empty());

	//duplicate data again for geomtree...
	const int numVertices = m_vertices.size();
	const int numIndices = m_indices.size();
	const int numTris = numIndices / 3;
	vector3f *vertices = new vector3f[numVertices];
	Uint16 *indices = new Uint16[numIndices];
	unsigned int *triFlags = new unsigned int[numTris];

	m_totalTris += numTris;

	for (int i = 0; i < numVertices; i++)
		vertices[i] = m_vertices[i];

	for (int i = 0; i < numIndices; i++)
		indices[i] = m_indices[i];

	for (int i = 0; i < numTris; i++)
		triFlags[i] = m_flags[i];

	//create geomtree
	//takes ownership of data
	GeomTree *gt = new GeomTree(
		numVertices, numTris,
		reinterpret_cast<float*>(vertices),
		indices, triFlags);
	m_collMesh->SetGeomTree(gt);
	m_collMesh->SetNumTriangles(m_totalTris);
	m_boundingRadius = m_collMesh->GetAabb().GetRadius();

	m_vertices.clear();
	m_indices.clear();
	m_flags.clear();

	return m_collMesh;
}
}
