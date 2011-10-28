#include "RenderFrustum.h"
#include "Render.h"

namespace Render {

// min/max FOV in degrees
static const float FOV_MAX = 170.0f;
static const float FOV_MIN = 20.0f;

Frustum::Frustum()
{
	InitFromGLState();
}

Frustum::Frustum(float width, float height, float fovAng)
{
	float fov = tan(DEG2RAD(Clamp(fovAng, FOV_MIN, FOV_MAX) / 2.0f));

	float znear, zfar;
	GetNearFarClipPlane(znear, zfar);

	float left = fov * znear;
	float top = left * height/width;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-left, left, -top, top, znear, zfar);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0, 0, GLsizei(width), GLsizei(height));

	InitFromGLState();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Frustum::InitFromGLState()
{
	glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix);
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	matrix4x4d m = matrix4x4d(m_projMatrix) * matrix4x4d(m_modelMatrix); 

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
	GLint o = gluProject(in.x, in.y, in.z, m_modelMatrix, m_projMatrix, m_viewport, &out.x, &out.y, &out.z);
	return o == GL_TRUE && out.x*out.x <= 1e8 && out.y*out.y <= 1e8;	// x & y get converted to ints later, must be sane
}

void Frustum::Enable()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(m_projMatrix);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(m_modelMatrix);
}

void Frustum::Disable()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

}
