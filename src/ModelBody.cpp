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
#include "Shader.h"

ModelBody::ModelBody(): Body()
{
	m_collMeshSet = 0;
	m_sbreModel = 0;
	m_geom = 0;
	m_isStatic = false;
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

void ModelBody::GetAabb(Aabb &aabb) const
{
	aabb = m_collMeshSet->GetAabb();
}

void ModelBody::SetModel(const char *sbreModelName, bool isStatic)
{
	m_isStatic = isStatic;
	try {
		m_sbreModel = sbreLookupModelByName(sbreModelName);
	} catch (SbreModelNotFoundException) {
		printf("Could not find model '%s'.\n", sbreModelName);
		Pi::Quit();
	}

	if (m_geom) {
		// only happens when player changes their ship
		GetFrame()->RemoveGeom(m_geom);
		delete m_geom;
	}
	const CollMeshSet *mset = GetModelCollMeshSet(m_sbreModel);
	
	m_geom = new Geom(mset->m_geomTree);
	m_geom->SetUserData((void*)this);
		
	if (GetFrame()) {
		if (m_isStatic) GetFrame()->AddStaticGeom(m_geom);
		else GetFrame()->AddGeom(m_geom);
	}

	m_collMeshSet = mset;
}

void ModelBody::SetPosition(vector3d p)
{
	matrix4x4d m;
	GetRotMatrix(m);
	m_geom->MoveTo(m, p);
	m_geom->MoveTo(m, p);
	// for rebuild of static objects in collision space
	if (m_isStatic) SetFrame(GetFrame());
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
		if (m_isStatic) GetFrame()->RemoveStaticGeom(m_geom);
		else GetFrame()->RemoveGeom(m_geom);
	}
	Body::SetFrame(f);
	if (f) {
		if (m_isStatic) f->AddStaticGeom(m_geom);
		else f->AddGeom(m_geom);
	}
}
	
void ModelBody::TriMeshUpdateLastPos(const matrix4x4d &currentTransform)
{
	m_geom->MoveTo(currentTransform);
}

void ModelBody::RenderSbreModel(const Frame *camFrame, ObjParams *params)
{
	matrix4x4d frameTrans;
	Frame::GetFrameTransform(GetFrame(), camFrame, frameTrans);

	float znear, zfar;
	Pi::worldView->GetNearFarClipPlane(&znear, &zfar);
	
	vector3d pos = frameTrans * GetPosition();

	if (pos.Length() > zfar) {
		glPointSize(1.0);
		glDisable(GL_LIGHTING);
		glColor3f(1,1,1);
		glBegin(GL_POINTS);
		pos = pos.Normalized() * 0.99*(double)zfar;
		glVertex3dv(&pos[0]);
		glEnd();
		glEnable(GL_LIGHTING);
	} else {
		glPushMatrix();
		
		sbreSetDepthRange(Pi::GetScrWidth()*0.5, 0.0f, 1.0f);

		matrix4x4d rot;
		GetRotMatrix(rot);
		frameTrans.ClearToRotOnly();
		rot = frameTrans * rot;

		Shader::EnableVertexProgram(Shader::VPROG_SBRE);
		sbreRenderModel(&pos.x, &rot[0], m_sbreModel, params);
		Shader::DisableVertexProgram();

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glDisable(GL_NORMALIZE);
		
		glPopMatrix();
	}
}
