// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Frustum.h"
#include "Graphics.h"
#include "MathUtil.h"

namespace Graphics {

	template<typename T>
	void FrustumT<T>::InitFromMatrix(const matrix4x4<T> &m)
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

		// Normalize the clipping planes
		for (int i = 0; i < 6; i++) {
			double invlen = 1.0 / sqrt(m_planes[i].a * m_planes[i].a + m_planes[i].b * m_planes[i].b + m_planes[i].c * m_planes[i].c);
			m_planes[i].a *= invlen;
			m_planes[i].b *= invlen;
			m_planes[i].c *= invlen;
			m_planes[i].d *= invlen;
		}
	}

	// explicitly instantiate the function templates
	template void FrustumT<float>::InitFromMatrix(const matrix4x4<float> &);
	template void FrustumT<double>::InitFromMatrix(const matrix4x4<double> &);

} // namespace Graphics
