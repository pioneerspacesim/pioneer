#include "RenderFrustum.h"
#include "Render.h"

namespace Render {

// min/max FOV in degrees
static const float FOV_MAX = 170.0f;
static const float FOV_MIN = 20.0f;

Frustum::Frustum(float width, float height, float fovAng) : m_width(width), m_height(height)
{
	m_shadersEnabled = AreShadersEnabled();
	SetFov(fovAng);
}

void Frustum::SetFov(float ang)
{
	m_fov = tan(DEG2RAD(Clamp(ang, FOV_MIN, FOV_MAX) / 2.0f));
	Update(true);
}

void Frustum::Update(bool force)
{
	if (!force && m_shadersEnabled == Render::AreShadersEnabled())
		return;
	m_shadersEnabled = Render::AreShadersEnabled();

	float znear, zfar;
	GetNearFarClipPlane(znear, zfar);

	m_frustumLeft = m_fov * znear;
	m_frustumTop = m_frustumLeft * m_height/m_width;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-m_frustumLeft, m_frustumLeft, -m_frustumTop, m_frustumTop, znear, zfar);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0, 0, GLsizei(m_width), GLsizei(m_height));

	glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

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
	// check all planes except far plane
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

bool Frustum::ProjectPoint(vector3d &in, vector3d &out) const
{
	GLint o = gluProject(in.x, in.y, in.z, m_modelMatrix, m_projMatrix, m_viewport, &out.x, &out.y, &out.z);
	return o == GL_TRUE && out.x*out.x <= 1e8 && out.y*out.y <= 1e8;	// x & y get converted to ints later, must be sane
}

void Frustum::Enable()
{
	float znear, zfar;
	GetNearFarClipPlane(znear, zfar);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-m_frustumLeft, m_frustumLeft, -m_frustumTop, m_frustumTop, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0, 0, GLsizei(m_width), GLsizei(m_height));
}

void Frustum::Disable()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

}
