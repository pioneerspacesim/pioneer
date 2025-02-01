// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "math/Plane.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace Graphics {

	// Frustum can be used for testing if a point lies inside the visible area
	template<typename T>
	class FrustumT {
	public:
		FrustumT() = delete;

		// create for specified values
		FrustumT(float width, float height, float fovAng, float nearClip, float farClip)
		{
			matrix4x4<T> projMatrix = matrix4x4<T>::PerspectiveMatrix(fovAng, width / height, nearClip, farClip);
			InitFromMatrix(projMatrix);
		}

		FrustumT(const matrix4x4<T> &modelview, const matrix4x4<T> &projection)
		{
			const matrix4x4<T> m = projection * modelview;
			InitFromMatrix(m);
		}

		// test if sphere is in the frustum
		bool TestSphere(const vector3<T> &p, T radius) const
		{
			for (int i = 0; i < 6; i++)
				if (m_planes[i].DistanceToPoint(p) + radius < 0)
					return false;
			return true;
		}

		// test if sphere is in the frustum, ignoring the far plane
		bool TestSphereInfinite(const vector3<T> &p, T radius) const
		{
			// check all planes except far plane
			for (int i = 0; i < 5; i++)
				if (m_planes[i].DistanceToPoint(p) + radius < 0)
					return false;
			return true;
		}

	private:
		void InitFromMatrix(const matrix4x4<T> &m);

		Plane<T> m_planes[6];
	};

	// Backwards-compatibility typedef
	using Frustum = FrustumT<double>;

	using FrustumF = FrustumT<float>;
	using FrustumD = FrustumT<double>;

} // namespace Graphics

#endif
