#include "libs.h"
#include "StaticRigidBody.h"
#include "Space.h"
#include "matrix4x4.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"

StaticRigidBody::StaticRigidBody(): Body()
{
	m_geom = 0;
	sbreCollMesh = 0;
}

StaticRigidBody::~StaticRigidBody()
{
	if (sbreCollMesh) {
		free(sbreCollMesh->pVertex);
		free(sbreCollMesh->pIndex);
		free(sbreCollMesh->pFlag);
		free(sbreCollMesh);
	}
	dGeomDestroy(m_geom);
}

void StaticRigidBody::SetGeomSphere(double radius)
{
	assert(!m_geom);
	m_geom = dCreateSphere(0, radius);
}

void StaticRigidBody::SetGeomFromSBREModel(int sbreModel, ObjParams *params)
{
	assert(!m_geom);
	assert(sbreCollMesh == 0);
	sbreCollMesh = (CollMesh*)calloc(1, sizeof(CollMesh));
	sbreGenCollMesh(sbreCollMesh, sbreModel, params);
	// XXX flip Z & X because sbre is in magicspace
	for (int i=0; i<3*sbreCollMesh->nv; i+=3) {
		sbreCollMesh->pVertex[i] = -sbreCollMesh->pVertex[i];
		sbreCollMesh->pVertex[i+2] = -sbreCollMesh->pVertex[i+2];
	}
	dTriMeshDataID triMeshDataID = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle(triMeshDataID, (void*)sbreCollMesh->pVertex,
		3*sizeof(float), sbreCollMesh->nv, (void*)sbreCollMesh->pIndex,
		sbreCollMesh->ni, 3*sizeof(int));

	// XXX leaking StaticRigidBody m_geom
	m_geom = dCreateTriMesh(0, triMeshDataID, NULL, NULL, NULL);
	dGeomSetData(m_geom, static_cast<Body*>(this));
}

void StaticRigidBody::SetPosition(vector3d p)
{
	dGeomSetPosition(m_geom, p.x, p.y, p.z);
}

void StaticRigidBody::SetVelocity(vector3d v)
{
	assert(0);
}

vector3d StaticRigidBody::GetPosition()
{
	const dReal *pos = dGeomGetPosition(m_geom);
	return vector3d(pos[0], pos[1], pos[2]);
}

void StaticRigidBody::GetRotMatrix(matrix4x4d &m)
{
	m.LoadFromOdeMatrix(dGeomGetRotation(m_geom));
}

void StaticRigidBody::ViewingRotation()
{
	matrix4x4d m;
	GetRotMatrix(m);
	m = m.InverseOf();
	glMultMatrixd(&m[0]);
}

void StaticRigidBody::TransformCameraTo()
{
	const dReal *p = dGeomGetPosition(m_geom);
	matrix4x4d m;
	GetRotMatrix(m);
	m = m.InverseOf();
	glMultMatrixd(&m[0]);
	glTranslated(-p[0], -p[1], -p[2]);
}

void StaticRigidBody::TransformToModelCoords(const Frame *camFrame)
{
	vector3d fpos = GetPositionRelTo(camFrame);

	const dReal *r = dGeomGetRotation(m_geom);
	matrix4x4d m;
	m[ 0] = r[ 0];m[ 1] = r[ 4];m[ 2] = r[ 8];m[ 3] = 0;
	m[ 4] = r[ 1];m[ 5] = r[ 5];m[ 6] = r[ 9];m[ 7] = 0;
	m[ 8] = r[ 2];m[ 9] = r[ 6];m[10] = r[10];m[11] = 0;
	m[12] = fpos.x; m[13] = fpos.y; m[14] = fpos.z; m[15] = 1;
	glMultMatrixd(&m[0]);
}

void StaticRigidBody::SetFrame(Frame *f)
{
	if (GetFrame()) GetFrame()->RemoveGeom(m_geom);
	Body::SetFrame(f);
	if (f) f->AddGeom(m_geom);
}
	
void StaticRigidBody::TriMeshUpdateLastPos()
{
	// ode tri mesh turd likes to know our old position
	const dReal *r = dGeomGetRotation(m_geom);
	vector3d pos = GetPosition();
	dMatrix4 t;
	// row-major
	t[0] = r[0]; t[1] = r[1]; t[2] = r[2]; t[3] = pos.x;
	t[4] = r[4]; t[5] = r[5]; t[6] = r[6]; t[7] = pos.y;
	t[8] = r[8]; t[9] = r[9]; t[10] = r[10]; t[11] = pos.z;
	t[12] = t[13] = t[14] = t[15] = 0;
	dGeomTriMeshSetLastTransform(m_geom, t);
}

void StaticRigidBody::RenderSbreModel(const Frame *camFrame, int model, ObjParams *params)
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
	rot.LoadFromOdeMatrix(dGeomGetRotation(m_geom));
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
