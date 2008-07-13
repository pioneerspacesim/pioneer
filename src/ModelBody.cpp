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

void ModelBody::SetGeomFromSBREModel(int sbreModel, ObjParams *params)
{
	assert(geoms.size() == 0);
	CollMeshSet *mset = GetModelCollMeshSet(sbreModel);
	
	geomColl.resize(mset->numMeshParts);
	geoms.resize(mset->numMeshParts);

	for (unsigned int i=0; i<mset->numMeshParts; i++) {
		geoms[i] = dCreateTriMesh(0, mset->meshParts[i], NULL, NULL, NULL);
		geomColl[i].parent = this;
		geomColl[i].flags = mset->meshFlags[i];
		dGeomSetData(geoms[i], static_cast<Object*>(&geomColl[i]));
	}
}

void ModelBody::SetPosition(vector3d p)
{
	for (unsigned int i=0; i<geoms.size(); i++) {
		dGeomSetPosition(geoms[i], p.x, p.y, p.z);
	}
}

void ModelBody::SetVelocity(vector3d v)
{
	assert(0);
}

vector3d ModelBody::GetPosition()
{
	const dReal *pos = dGeomGetPosition(geoms[0]);
	return vector3d(pos[0], pos[1], pos[2]);
}

void ModelBody::GetRotMatrix(matrix4x4d &m)
{
	m.LoadFromOdeMatrix(dGeomGetRotation(geoms[0]));
}

void ModelBody::ViewingRotation()
{
	matrix4x4d m;
	GetRotMatrix(m);
	m = m.InverseOf();
	glMultMatrixd(&m[0]);
}

void ModelBody::TransformCameraTo()
{
	const dReal *p = dGeomGetPosition(geoms[0]);
	matrix4x4d m;
	GetRotMatrix(m);
	m = m.InverseOf();
	glMultMatrixd(&m[0]);
	glTranslated(-p[0], -p[1], -p[2]);
}

void ModelBody::TransformToModelCoords(const Frame *camFrame)
{
	vector3d fpos = GetPositionRelTo(camFrame);

	const dReal *r = dGeomGetRotation(geoms[0]);
	matrix4x4d m;
	m[ 0] = r[ 0];m[ 1] = r[ 4];m[ 2] = r[ 8];m[ 3] = 0;
	m[ 4] = r[ 1];m[ 5] = r[ 5];m[ 6] = r[ 9];m[ 7] = 0;
	m[ 8] = r[ 2];m[ 9] = r[ 6];m[10] = r[10];m[11] = 0;
	m[12] = fpos.x; m[13] = fpos.y; m[14] = fpos.z; m[15] = 1;
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
		// XXX need to use correct starlight colour
		float lightCol[3] = { 1,1,1 };
		float lightDir[3];
		vector3d _lightDir = Frame::GetFramePosRelativeToOther(Space::GetRootFrame(), camFrame);

		matrix4x4d poo = Pi::world_view->viewingRotation;
		poo[2] = -poo[2];
		poo[6] = -poo[6];
		poo[8] = -poo[8];
		poo[9] = -poo[9];
		_lightDir = poo * _lightDir;

		lightDir[0] = _lightDir.x;
		lightDir[1] = _lightDir.y;
		lightDir[2] = _lightDir.z;

		sbreSetDirLight (lightCol, lightDir);
	}
	sbreSetViewport(Pi::GetScrWidth(), Pi::GetScrHeight(), Pi::GetScrWidth()*0.5, 5.0f, 100000.0f, 0.0f, 1.0f);
	vector3d pos = GetPositionRelTo(camFrame);
	pos = Pi::world_view->viewingRotation * pos;
	Vector p; p.x = pos.x; p.y = pos.y; p.z = -pos.z;
	matrix4x4d rot;
	rot.LoadFromOdeMatrix(dGeomGetRotation(geoms[0]));
	rot = Pi::world_view->viewingRotation * rot;
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
