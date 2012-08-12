#ifndef _GL2_GEOSPHEREMATERIAL_H
#define _GL2_GEOSPHEREMATEIRAL_H
/*
 * Programs & Materials used by terrain
 */
#include "libs.h"
#include "GL2Material.h"
#include "Program.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
	namespace GL2 {
		class GeoSphereProgram : public Program {
		public:
			GeoSphereProgram(const std::string &filename, const std::string &defines);
			void SetUniforms(float radius, float scale,
				const vector3d &center, const SystemBody::AtmosphereParameters &);

			Uniform atmosColor;
			Uniform geosphereAtmosFogDensity;
			Uniform geosphereAtmosInvScaleHeight;
			Uniform geosphereAtmosTopRad; // in planet radii
			Uniform geosphereCenter;
			Uniform geosphereScale;
			Uniform geosphereScaledRadius; // (planet radius) / scale

		protected:
			virtual void InitUniforms();
		};
	}
}
#endif
