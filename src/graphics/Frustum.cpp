// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Frustum.h"
#include "Graphics.h"
#include "MathUtil.h"

namespace Graphics {

	// min/max FOV in degrees
	static const float FOV_MAX = 170.0f;
	static const float FOV_MIN = 20.0f;

	// step for translating to frustum space
	static const double TRANSLATE_STEP = 0.25;

	Frustum::Frustum() {}

	Frustum::Frustum(float width, float height, float fovAng, float znear, float zfar)
	{
		m_projMatrix = matrix4x4d::PerspectiveMatrix(DEG2RAD(Clamp(fovAng, FOV_MIN, FOV_MAX)), width / height, znear, zfar);
		m_modelMatrix = matrix4x4d::Identity();
		InitFromMatrix(m_projMatrix);

		m_translateThresholdSqr = zfar * zfar * TRANSLATE_STEP;
	}

	Frustum::Frustum(const matrix4x4d &modelview, const matrix4x4d &projection) :
		m_projMatrix(projection),
		m_modelMatrix(modelview)
	{
		const matrix4x4d m = m_projMatrix * m_modelMatrix;
		InitFromMatrix(m);
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
		for (int i = 0; i < 6; i++) {
			double invlen = 1.0 / sqrt(m_planes[i].a * m_planes[i].a + m_planes[i].b * m_planes[i].b + m_planes[i].c * m_planes[i].c);
			m_planes[i].a *= invlen;
			m_planes[i].b *= invlen;
			m_planes[i].c *= invlen;
			m_planes[i].d *= invlen;
		}
	}

	bool Frustum::TestPoint(const vector3d &p, double radius) const
	{
		for (int i = 0; i < 6; i++)
			if (m_planes[i].DistanceToPoint(p) + radius < 0)
				return false;
		return true;
	}

	bool Frustum::TestPointInfinite(const vector3d &p, double radius) const
	{
		// check all planes except far plane
		for (int i = 0; i < 5; i++)
			if (m_planes[i].DistanceToPoint(p) + radius < 0)
				return false;
		return true;
	}

	// Returns a vector3d in the range { 0..1, 0..1, 1..0 }
	bool Frustum::ProjectPoint(const vector3d &in, vector3d &out) const
	{
		// see the OpenGL documentation
		// or http://www.songho.ca/opengl/gl_transform.html
		// or http://cgit.freedesktop.org/mesa/glu/tree/src/libutil/project.c (gluProject implementation from Mesa)

		const double *const M = m_modelMatrix.Data();
		const double *const P = m_projMatrix.Data();

		const double vcam[4] = { // camera space
			in.x * M[0] + in.y * M[4] + in.z * M[8] + M[12],
			in.x * M[1] + in.y * M[5] + in.z * M[9] + M[13],
			in.x * M[2] + in.y * M[6] + in.z * M[10] + M[14],
			in.x * M[3] + in.y * M[7] + in.z * M[11] + M[15]
		};
		const double vclip[4] = { // clip space
			vcam[0] * P[0] + vcam[1] * P[4] + vcam[2] * P[8] + vcam[3] * P[12],
			vcam[0] * P[1] + vcam[1] * P[5] + vcam[2] * P[9] + vcam[3] * P[13],
			vcam[0] * P[2] + vcam[1] * P[6] + vcam[2] * P[10] + vcam[3] * P[14],
			vcam[0] * P[3] + vcam[1] * P[7] + vcam[2] * P[11] + vcam[3] * P[15]
		};

		if (is_zero_exact(vclip[3])) {
			return false;
		}

		const double w = vclip[3];
		out.x = (vclip[0] / w) * 0.5 + 0.5;
		out.y = (vclip[1] / w) * 0.5 + 0.5;
		out.z = (vclip[2] / w);

		return true;
	}

	void Frustum::TranslatePoint(const vector3d &in, vector3d &out) const
	{
		out = in;
		while (out.LengthSqr() > m_translateThresholdSqr) {
			out *= TRANSLATE_STEP;
		}
	}

} // namespace Graphics
