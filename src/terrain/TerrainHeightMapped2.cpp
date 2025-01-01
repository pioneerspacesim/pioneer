// Copyright © 2008-2024 Pioneer Developers. 
// See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"
#include <algorithm> // for std::max clamp
#include <cmath>     // for std::pow, etc.

/**
 * \class TerrainHeightMapped2
 * \brief Specialized fractal class used primarily for the Moon-like terrain.
 *
 * This class generates terrain by first sampling a bicubic interpolation map
 * (BiCubicInterpolation) for a base scalar, then applies multiple layers of ridged noise
 * for additional detail.
 *
 * \note The pipeline:
 *   - Sample base fractal or interpolation map -> \c v
 *   - Scale and offset \c v by \c m_heightScaling, \c m_minh, etc.
 *   - Apply ridged noise with varying octaves, frequencies, and multipliers
 *   - Combine results and clamp final height to 0 if negative
 */
using namespace TerrainNoise;

// Templated fractal name retrieval
template <>
const char *TerrainHeightFractal<TerrainHeightMapped2>::GetHeightFractalName() const
{
	return "Mapped2";
}

// Constructor
template <>
TerrainHeightFractal<TerrainHeightMapped2>::TerrainHeightFractal(const SystemBody *body)
	: Terrain(body)
{
	// Optionally, you could tweak local parameters here
	// e.g., m_heightScaling = body->GetSomeValue();
	//       or load from config data, etc.
}

/**
 * \fn GetHeight
 * \brief Returns the terrain height at position \p p in planet-local coordinates.
 *
 * The process:
 *   1. BiCubicInterpolation(p) -> base \c v
 *   2. Apply scaling: \c v = v * m_heightScaling + m_minh
 *   3. Multiply by inverse planet radius: \c v *= m_invPlanetRadius
 *   4. Add a small offset (0.1) to \c v
 *   5. Calculate multiple ridged octave noises using \c v as amplitude or frequency mod
 *   6. Combine results to produce final \c h
 *   7. Subtract 0.09, then clamp to zero if negative
 *
 * \param[in] p Planet-local coordinates (normalized or actual?)
 * \return final terrain height in planet radius units or absolute units, depending on usage
 */
template <>
double TerrainHeightFractal<TerrainHeightMapped2>::GetHeight(const vector3d &p) const
{
	// Step 1: Base sampling from Bicubic Interpolation
	double v = BiCubicInterpolation(p);

	// Step 2: Scale and offset for planet's min/max
	v = (v * m_heightScaling + m_minh) * m_invPlanetRadius;

	// Step 3: Additional offset
	v += 0.1;

	// Step 4: Ridged noise calculations
	//   - The exponents (1.5, 4.0, 5.0, etc.) are presumably tuned for lunar shapes
	double h = 1.5 * std::pow(v, 3) * ridged_octavenoise(16, 4.0 * v, 4.0, p);
	h += 30000.0 * std::pow(v, 7) * ridged_octavenoise(16, 5.0 * v, 20.0 * v, p);
	h += v;

	// Adjust final offset
	h -= 0.09;

	// Step 5: Ensure non-negative result
	return std::max(h, 0.0);
}
