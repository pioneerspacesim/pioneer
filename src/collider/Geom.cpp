#include "Geom.h"

Geom::Geom(GeomTree *geomtree)
{
	m_geomtree = geomtree;
	m_orient = matrix4x4d::Identity();
	m_invOrient = matrix4x4d::Identity();
	m_active = true;
}

void Geom::SetPosition(vector3d pos)
{
	m_orient[12] = pos.x;
	m_orient[13] = pos.y;
	m_orient[14] = pos.z;
	m_invOrient = m_orient.InverseOf();
}

void Geom::SetOrientation(const matrix4x4d &rot)
{
	m_orient[0] = rot[0];
	m_orient[1] = rot[1];
	m_orient[2] = rot[2];
	m_orient[4] = rot[4];
	m_orient[5] = rot[5];
	m_orient[6] = rot[6];
	m_orient[8] = rot[8];
	m_orient[9] = rot[9];
	m_orient[10] = rot[10];
	m_invOrient = m_orient.InverseOf();
}

