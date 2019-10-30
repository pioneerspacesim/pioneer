#ifndef SYSTEMBODYWRAPPER_H
#define SYSTEMBODYWRAPPER_H

#include <string>
#include <SDL_stdinc.h>

#include "Color.h"
#include "galaxy/GalaxyEnums.h"

class SystemBody;
class SystemPath;

struct AtmosphereParameters;
struct RingStyle;

class SystemBodyWrapper
{
	public:
		SystemBodyWrapper(const SystemBody *sb);

		const SystemBody *GetSystemBody() const;

		GalaxyEnums::BodyType GetSystemBodyType() const;
		GalaxyEnums::BodySuperType GetSystemBodySuperType() const;

		bool IsType(GalaxyEnums::BodyType type) const;
		bool IsSuperType(GalaxyEnums::BodySuperType type) const;

		double GetSystemBodyMass() const;
		double GetSystemBodyRadius() const;

		const std::string &GetSystemBodyName() const;
		const SystemPath &GetSystemBodyPath() const;
		Uint32 GetSystemBodySeed() const;
		bool SystemBodyHasRings() const;
		const RingStyle &GetSystemBodyRings() const;

		bool SystemBodyHasAtmosphere() const;

		int GetSystemBodyPopulation() const;

		int GetSystemBodyAverageTemp() const;

		void GetSystemBodyAtmosphereFlavor(Color &outColor, double &surfaceDensity) const; // kg / m^3

		AtmosphereParameters CalcSystemBodyAtmosphereParams() const;

	private:
		const SystemBody *m_sbody;
};

#endif // SYSTEMBODYWRAPPER_H
