#include "RenderFrustum.h"
#include "Render.h"

namespace Render {

// min/max FOV in degrees
static const float FOV_MAX = 170.0f;
static const float FOV_MIN = 20.0f;

Frustum Frustum::FromGLState()
{
	Frustum f;
	f.InitFromGLState();
	return f;
}

Frustum::Frustum() {}

Frustum::Frustum(float width, float height, float fovAng)
{
	//http://www.opengl.org/resources/faq/technical/transformations.htm
	const float fov = tan(DEG2RAD(Clamp(fovAng, FOV_MIN, FOV_MAX) / 2.0f));

	float znear, zfar;
	GetNearFarClipPlane(znear, zfar);

	const float aspect = width/height;
	const float top = znear * fov;
	const float bottom = -top;
	const float left = bottom * aspect;
	const float right = top * aspect;

	m_projMatrix = matrix4x4d::FrustumMatrix(left, right, bottom, top, znear, zfar);
	m_modelMatrix = matrix4x4d::Identity();
	InitFromMatrix(m_projMatrix);
}

void Frustum::InitFromMatrix(const matrix4x4d &m)
{
	// Left clipping plane
	m_planes[0].a = m[3] + m[0];
	m_planes[0].b = m[7] + m[4];
	m_planes[0].c = m[11] + m[8];
	m_planes[0].d = m[15] + m[12];
	// Right clipping plane
	m_planes[1].a = m[3] - m[0];
	m_planes[1].b = m[7] - m[4];
	m_planes[1].c = m[11] - m[8];
	m_planes[1].d = m[15] - m[12];
	// Top clipping plane
	m_planes[2].a = m[3] - m[1];
	m_planes[2].b = m[7] - m[5];
	m_planes[2].c = m[11] - m[9];
	m_planes[2].d = m[15] - m[13];
	// Bottom clipping plane
	m_planes[3].a = m[3] + m[1];
	m_planes[3].b = m[7] + m[5];
	m_planes[3].c = m[11] + m[9];
	m_planes[3].d = m[15] + m[13];
	// Near clipping plane
	m_planes[4].a = m[3] + m[2];
	m_planes[4].b = m[7] + m[6];
	m_planes[4].c = m[11] + m[10];
	m_planes[4].d = m[15] + m[14];
	// Far clipping plane
	m_planes[5].a = m[3] + m[2];
	m_planes[5].b = m[7] + m[6];
	m_planes[5].c = m[11] + m[10];
	m_planes[5].d = m[15] + m[14];

	// Normalize the fuckers
	for (int i=0; i<6; i++) {
		double invlen = 1.0 / sqrt(m_planes[i].a*m_planes[i].a + m_planes[i].b*m_planes[i].b + m_planes[i].c*m_planes[i].c);
		m_planes[i].a *= invlen;
		m_planes[i].b *= invlen;
		m_planes[i].c *= invlen;
		m_planes[i].d *= invlen;
	}
}

void Frustum::InitFromGLState()
{
	glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix.Data());
	glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix.Data());
	matrix4x4d m = matrix4x4d(m_projMatrix) * matrix4x4d(m_modelMatrix);
	InitFromMatrix(m);
}

bool Frustum::TestPoint(const vector3d &p, double radius) const
{
	for (int i=0; i<6; i++)
		if (m_planes[i].DistanceToPoint(p)+radius < 0)
			return false;
	return true;
}

bool Frustum::TestPointInfinite(const vector3d &p, double radius) const
{
	// check all planes except far plane
	for (int i=0; i<5; i++)
		if (m_planes[i].DistanceToPoint(p)+radius < 0)
			return false;
	return true;
}

bool Frustum::ProjectPoint(const vector3d &in, vector3d &out) const
{
	// fake viewport -- this is an identity transform when applied by gluProject
	// see, e.g., http://cgit.freedesktop.org/mesa/mesa/tree/src/glu/sgi/libutil/project.c
	// or the documentation for gluProject
	const GLint viewport[4] = { 0, 0, 1, 1 };
	return gluProject(in.x, in.y, in.z, m_modelMatrix.Data(), m_projMatrix.Data(), viewport, &out.x, &out.y, &out.z) == GL_TRUE;
}

void Frustum::Enable()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(m_projMatrix.Data());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(m_modelMatrix.Data());
}

void Frustum::Disable()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

}
