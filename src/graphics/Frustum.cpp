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

	Frustum::Frustum(float width, float height, float fovAng, float znear, float zfar)
	{
		matrix4x4d projMatrix = matrix4x4d::PerspectiveMatrix(DEG2RAD(Clamp(fovAng, FOV_MIN, FOV_MAX)), width / height, znear, zfar);
		InitFromMatrix(projMatrix);

		m_translateThresholdSqr = zfar * zfar * TRANSLATE_STEP;
	}

	Frustum::Frustum(const matrix4x4d &modelview, const matrix4x4d &projection)
	{
		const matrix4x4d m = projection * modelview;
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

} // namespace Graphics
