#include "libs.h"
#include "ModelBody.h"
#include "Space.h"
#include "matrix4x4.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"
#include "ModelCollMeshData.h"
#include "Serializer.h"
#include "collider/collider.h"

ModelBody::ModelBody(): Body()
{
	m_collMeshSet = 0;
	m_geom = 0;
}

ModelBody::~ModelBody()
{
	SetFrame(0);	// Will remove geom from frame if necessary.
	delete m_geom;
}

void ModelBody::Save()
{
	using namespace Serializer::Write;
	Body::Save();
}

void ModelBody::Load()
{
	using namespace Serializer::Read;
	Body::Load();
}

void ModelBody::Disable()
{
	m_geom->Disable();
}

void ModelBody::Enable()
{
	m_geom->Enable();
}

void ModelBody::GetAabb(Aabb &aabb)
{
	aabb = m_collMeshSet->aabb;
}

void ModelBody::SetModel(int sbreModel)
{
	assert(m_geom == 0);
	CollMeshSet *mset = GetModelCollMeshSet(sbreModel);
	
	m_geom = new Geom(mset->m_geomTree);
	m_geom->SetUserData((void*)this);

	m_collMeshSet = mset;
}

void ModelBody::SetPosition(vector3d p)
{
	matrix4x4d m;
	GetRotMatrix(m);
	m_geom->MoveTo(m, p);
	m_geom->MoveTo(m, p);
}

vector3d ModelBody::GetPosition() const
{
	return m_geom->GetPosition();
}

double ModelBody::GetRadius() const
{
	Aabb aabb = m_geom->GetGeomTree()->GetAabb();
	// Return size of largest dimension.
	return std::max(aabb.max.x - aabb.min.x, std::max(aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z));
}

void ModelBody::SetRotMatrix(const matrix4x4d &r)
{
	vector3d pos = m_geom->GetPosition();
	m_geom->MoveTo(r, pos);
	m_geom->MoveTo(r, pos);
}

void ModelBody::GetRotMatrix(matrix4x4d &m) const
{
	m = m_geom->GetRotation();
}

void ModelBody::TransformToModelCoords(const Frame *camFrame)
{
	matrix4x4d m = m_geom->GetTransform();
	matrix4x4d m2;
	Frame::GetFrameTransform(GetFrame(), camFrame, m2);
	m = m2 * m;
	glMultMatrixd(&m[0]);
}

void ModelBody::SetFrame(Frame *f)
{
	if (GetFrame()) {
		GetFrame()->RemoveGeom(m_geom);
	}
	Body::SetFrame(f);
	if (f) {
		f->AddGeom(m_geom);
	}
}
	
void ModelBody::TriMeshUpdateLastPos(const matrix4x4d &currentTransform)
{
	m_geom->MoveTo(currentTransform);
}

void ModelBody::RenderSbreModel(const Frame *camFrame, int model, ObjParams *params)
{
	glPushMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	// XXX reduce
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	for (int i=0; i<4; i++) {
		GLfloat lightDir[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, lightDir);
		lightDir[2] = -lightDir[2];
		glLightfv(GL_LIGHT0+i, GL_POSITION, lightDir);
	}
/*	{
		GLfloat lightCol[4];
		glGetLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
		lightCol[3] = 0;
		
		GLfloat lightDir[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, lightDir);
		lightDir[2] = -lightDir[2];
		
		sbreSetDirLight (lightCol, lightDir);
	}*/
	sbreSetViewport(Pi::GetScrWidth(), Pi::GetScrHeight(), Pi::GetScrWidth()*0.5, 5.0f, 100000.0f, 0.0f, 1.0f);

	matrix4x4d frameTrans;
	Frame::GetFrameTransform(GetFrame(), camFrame, frameTrans);

	vector3d pos = GetPosition();//GetPositionRelTo(camFrame);
	pos = frameTrans * pos;
	Vector p; p.x = pos.x; p.y = pos.y; p.z = -pos.z;
	matrix4x4d rot;
	GetRotMatrix(rot);
	frameTrans.ClearToRotOnly();
	rot = frameTrans * rot;
	Matrix m;
	m.x1 = rot[0]; m.x2 = rot[4]; m.x3 = -rot[8];
	m.y1 = rot[1]; m.y2 = rot[5]; m.y3 = -rot[9];
	m.z1 = -rot[2]; m.z2 = -rot[6]; m.z3 = rot[10];

	sbreRenderModel(&p, &m, model, params);
	
	for (int i=0; i<4; i++) {
		GLfloat lightDir[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, lightDir);
		lightDir[2] = -lightDir[2];
		glLightfv(GL_LIGHT0+i, GL_POSITION, lightDir);
	}
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
