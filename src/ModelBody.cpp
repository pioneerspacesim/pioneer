#include "libs.h"
#include "ModelBody.h"
#include "Space.h"
#include "matrix4x4.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"
#include "ModelCollMeshData.h"

ModelBody::ModelBody(): Body()
{
	triMeshLastMatrixIndex = 0;
}

ModelBody::~ModelBody()
{
	SetFrame(0);	// Will remove geom from frame if necessary.
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomDestroy(geoms[i]);
	}
}

void ModelBody::Disable()
{
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomDisable(geoms[i]);
	}
}

void ModelBody::Enable()
{
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomEnable(geoms[i]);
	}
}

void ModelBody::GeomsSetBody(dBodyID body)
{
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomSetBody(geoms[i], body);
	}
}

void ModelBody::SetModel(int sbreModel)
{
	assert(geoms.size() == 0);
	CollMeshSet *mset = GetModelCollMeshSet(sbreModel);
	
	geomColl.resize(mset->numMeshParts);
	geoms.resize(mset->numMeshParts);

	for (int i=0; i<mset->numMeshParts; i++) {
		geoms[i] = dCreateTriMesh(0, mset->meshParts[i], NULL, NULL, NULL);
		geomColl[i].parent = this;
		geomColl[i].flags = mset->meshInfo[i].flags;
		dGeomSetData(geoms[i], static_cast<Object*>(&geomColl[i]));
	}
}

void ModelBody::SetPosition(vector3d p)
{
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomSetPosition(geoms[i], p.x, p.y, p.z);
	}
}

vector3d ModelBody::GetPosition()
{
	const dReal *pos = dGeomGetPosition(geoms[0]);
	return vector3d(pos[0], pos[1], pos[2]);
}

double ModelBody::GetRadius() const
{
	// Calculate single AABB containing all geoms.
	dReal aabbAll[6] = {
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::min(),
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::min(),
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::min()
	};

	for(size_t i = 0; i < geoms.size(); ++i) {
		dReal aabbGeom[6];
		dGeomGetAABB(geoms[i], aabbGeom);
		aabbAll[0] = std::min(aabbAll[0], aabbGeom[0]);
		aabbAll[1] = std::max(aabbAll[1], aabbGeom[1]);
		aabbAll[2] = std::min(aabbAll[2], aabbGeom[2]);
		aabbAll[3] = std::max(aabbAll[3], aabbGeom[3]);
		aabbAll[4] = std::min(aabbAll[4], aabbGeom[4]);
		aabbAll[5] = std::max(aabbAll[5], aabbGeom[5]);
	}

	// Return size of largest dimension.
	return std::max(aabbAll[1] - aabbAll[0], std::max(aabbAll[3] - aabbAll[2], aabbAll[5] - aabbAll[4]));
}

void ModelBody::SetRotMatrix(const matrix4x4d &r)
{
	dMatrix3 _m;
	r.SaveToOdeMatrix(_m);
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomSetRotation(geoms[i], _m);
	}
}

void ModelBody::GetRotMatrix(matrix4x4d &m)
{
	m.LoadFromOdeMatrix(dGeomGetRotation(geoms[0]));
}

void ModelBody::TransformToModelCoords(const Frame *camFrame)
{
	const vector3d pos = GetPosition();
	const dReal *r = dGeomGetRotation(geoms[0]);
	matrix4x4d m, m2;
	
	Frame::GetFrameTransform(GetFrame(), camFrame, m2);
	
	m[ 0] = r[ 0];m[ 1] = r[ 4];m[ 2] = r[ 8];m[ 3] = 0;
	m[ 4] = r[ 1];m[ 5] = r[ 5];m[ 6] = r[ 9];m[ 7] = 0;
	m[ 8] = r[ 2];m[ 9] = r[ 6];m[10] = r[10];m[11] = 0;
	m[12] = pos.x; m[13] = pos.y; m[14] = pos.z; m[15] = 1;
	
	m = m2 * m;
	glMultMatrixd(&m[0]);
	
}

void ModelBody::SetFrame(Frame *f)
{
	if (GetFrame()) {
		for (unsigned int i=0; i<geoms.size(); i++) {
			GetFrame()->RemoveGeom(geoms[i]);
		}
	}
	Body::SetFrame(f);
	if (f) {
		for (unsigned int i=0; i<geoms.size(); i++) {
			f->AddGeom(geoms[i]);
		}
	}
}
	
void ModelBody::TriMeshUpdateLastPos()
{
	// ode tri mesh turd likes to know our old position
	const dReal *r = dGeomGetRotation(geoms[0]);
	vector3d pos = GetPosition();
	dReal *t = triMeshTrans + 16*triMeshLastMatrixIndex;
	t[0] = r[0]; t[1] = r[1]; t[2] = r[2]; t[3] = 0;
	t[4] = r[4]; t[5] = r[5]; t[6] = r[6]; t[7] = 0;
	t[8] = r[8]; t[9] = r[9]; t[10] = r[10]; t[11] = 0;
	t[12] = pos.x; t[13] = pos.y; t[14] = pos.z; t[15] = 1;
	triMeshLastMatrixIndex = !triMeshLastMatrixIndex;
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomTriMeshSetLastTransform(geoms[i], *(dMatrix4*)(triMeshTrans + 16*triMeshLastMatrixIndex));
	}
}

void ModelBody::RenderSbreModel(const Frame *camFrame, int model, ObjParams *params)
{
	glPushMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	// XXX reduce
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	{
		GLfloat lightCol[4];
		glGetLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
		lightCol[3] = 0;
		
		GLfloat lightDir[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, lightDir);
		lightDir[2] = -lightDir[2];
		
		sbreSetDirLight (lightCol, lightDir);
	}
	sbreSetViewport(Pi::GetScrWidth(), Pi::GetScrHeight(), Pi::GetScrWidth()*0.5, 5.0f, 100000.0f, 0.0f, 1.0f);

	matrix4x4d frameTrans;
	Frame::GetFrameTransform(GetFrame(), camFrame, frameTrans);

	vector3d pos = GetPosition();//GetPositionRelTo(camFrame);
	pos = frameTrans * pos;
	Vector p; p.x = pos.x; p.y = pos.y; p.z = -pos.z;
	matrix4x4d rot;
	rot.LoadFromOdeMatrix(dGeomGetRotation(geoms[0]));
	frameTrans.ClearToRotOnly();
	rot = frameTrans * rot;
	Matrix m;
	m.x1 = rot[0]; m.x2 = rot[4]; m.x3 = -rot[8];
	m.y1 = rot[1]; m.y2 = rot[5]; m.y3 = -rot[9];
	m.z1 = -rot[2]; m.z2 = -rot[6]; m.z3 = rot[10];

	sbreRenderModel(&p, &m, model, params);
	
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
