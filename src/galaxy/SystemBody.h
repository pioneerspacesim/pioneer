// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SYSTEMBODY_H
#define SYSTEMBODY_H

#include "Color.h"
#include "IterationProxy.h"
#include "Orbit.h"
#include "RefCounted.h"
#include "galaxy/RingStyle.h"
#include "galaxy/SystemPath.h"
#include "gameconsts.h"

class StarSystem;

struct AtmosphereParameters;

class SystemBody : public RefCounted {
public:
	SystemBody(const SystemPath &path, StarSystem *system);

	enum BodyType { // <enum scope='SystemBody' prefix=TYPE_ public>
		TYPE_GRAVPOINT = 0,
		TYPE_BROWN_DWARF = 1, //  L+T Class Brown Dwarfs
		TYPE_WHITE_DWARF = 2,
		TYPE_STAR_M = 3, //red
		TYPE_STAR_K = 4, //orange
		TYPE_STAR_G = 5, //yellow
		TYPE_STAR_F = 6, //white
		TYPE_STAR_A = 7, //blue/white
		TYPE_STAR_B = 8, //blue
		TYPE_STAR_O = 9, //blue/purple/white
		TYPE_STAR_M_GIANT = 10,
		TYPE_STAR_K_GIANT = 11,
		TYPE_STAR_G_GIANT = 12,
		TYPE_STAR_F_GIANT = 13,
		TYPE_STAR_A_GIANT = 14,
		TYPE_STAR_B_GIANT = 15,
		TYPE_STAR_O_GIANT = 16,
		TYPE_STAR_M_SUPER_GIANT = 17,
		TYPE_STAR_K_SUPER_GIANT = 18,
		TYPE_STAR_G_SUPER_GIANT = 19,
		TYPE_STAR_F_SUPER_GIANT = 20,
		TYPE_STAR_A_SUPER_GIANT = 21,
		TYPE_STAR_B_SUPER_GIANT = 22,
		TYPE_STAR_O_SUPER_GIANT = 23,
		TYPE_STAR_M_HYPER_GIANT = 24,
		TYPE_STAR_K_HYPER_GIANT = 25,
		TYPE_STAR_G_HYPER_GIANT = 26,
		TYPE_STAR_F_HYPER_GIANT = 27,
		TYPE_STAR_A_HYPER_GIANT = 28,
		TYPE_STAR_B_HYPER_GIANT = 29,
		TYPE_STAR_O_HYPER_GIANT = 30, // these various stars do exist = they are transitional states and are rare
		TYPE_STAR_M_WF = 31,		  //Wolf-Rayet star
		TYPE_STAR_B_WF = 32,		  // while you do not specifically get class M,B or O WF stars,
		TYPE_STAR_O_WF = 33,		  //  you do get red = blue and purple from the colour of the gasses = so spectral class is an easy way to define them.
		TYPE_STAR_S_BH = 34,		  //stellar blackhole
		TYPE_STAR_IM_BH = 35,		  //Intermediate-mass blackhole
		TYPE_STAR_SM_BH = 36,		  //Supermassive blackhole
		TYPE_PLANET_GAS_GIANT = 37,
		TYPE_PLANET_ASTEROID = 38,
		TYPE_PLANET_TERRESTRIAL = 39,
		TYPE_STARPORT_ORBITAL = 40,
		TYPE_STARPORT_SURFACE = 41,
		TYPE_MIN = TYPE_BROWN_DWARF,	  // <enum skip>
		TYPE_MAX = TYPE_STARPORT_SURFACE, // <enum skip>
		TYPE_STAR_MIN = TYPE_BROWN_DWARF, // <enum skip>
		TYPE_STAR_MAX = TYPE_STAR_SM_BH,  // <enum skip>
										  // XXX need larger atmosphereless thing
	};

	enum BodySuperType { // <enum scope='SystemBody' prefix=SUPERTYPE_ public>
		SUPERTYPE_NONE = 0,
		SUPERTYPE_STAR = 1,
		SUPERTYPE_ROCKY_PLANET = 2,
		SUPERTYPE_GAS_GIANT = 3,
		SUPERTYPE_STARPORT = 4,
	};

	const SystemPath &GetPath() const { return m_path; }
	SystemBody *GetParent() const { return m_parent; }

	bool IsPlanet() const;
	bool IsMoon() const { return GetSuperType() == SUPERTYPE_ROCKY_PLANET && !IsPlanet(); }
	// We allow hyperjump to any star of the system
	bool IsJumpable() const { return GetSuperType() == SUPERTYPE_STAR; }
	SystemBody *GetNearestJumpable();

	bool HasChildren() const { return !m_children.empty(); }
	Uint32 GetNumChildren() const { return static_cast<Uint32>(m_children.size()); }
	IterationProxy<std::vector<SystemBody *>> GetChildren() { return MakeIterationProxy(m_children); }
	const IterationProxy<const std::vector<SystemBody *>> GetChildren() const { return MakeIterationProxy(m_children); }

	const std::vector<SystemBody *> CollectAllChildren();

	inline const std::string &GetName() const { return m_name; }
	std::string GetAstroDescription() const;
	const char *GetIcon() const;
	BodyType GetType() const { return m_type; }
	BodySuperType GetSuperType() const;
	bool IsCustomBody() const { return m_isCustomBody; }
	bool IsCoOrbitalWith(const SystemBody *other) const; //this and other form a binary pair
	bool IsCoOrbital() const;							 //is part of any binary pair
	fixed GetRadiusAsFixed() const { return m_radius; }

	// the aspect ratio adjustment is converting from equatorial to polar radius to account for ellipsoid bodies, used for calculating terrains etc
	inline double GetRadius() const
	{
		if (GetSuperType() <= SUPERTYPE_STAR) // polar radius
			return (m_radius.ToDouble() / m_aspectRatio.ToDouble()) * SOL_RADIUS;
		else
			return m_radius.ToDouble() * EARTH_RADIUS;
	}

	// the un-adjusted equatorial radius is necessary for calculating the radius of frames, see Space.cpp `MakeFrameFor`
	inline double GetEquatorialRadius() const
	{
		if (GetSuperType() <= SUPERTYPE_STAR) // equatorial radius
			return m_radius.ToDouble() * SOL_RADIUS;
		else
			return m_radius.ToDouble() * EARTH_RADIUS;
	}

	double GetAspectRatio() const { return m_aspectRatio.ToDouble(); }
	fixed GetMassAsFixed() const { return m_mass; }
	double GetMass() const
	{
		if (GetSuperType() <= SUPERTYPE_STAR)
			return m_mass.ToDouble() * SOL_MASS;
		else
			return m_mass.ToDouble() * EARTH_MASS;
	}
	fixed GetMassInEarths() const
	{
		if (GetSuperType() <= SUPERTYPE_STAR)
			return m_mass * 332998;
		else
			return m_mass;
	}
	bool IsRotating() const { return m_rotationPeriod != fixed(0); }
	// returned in seconds
	double GetRotationPeriodInDays() const { return m_rotationPeriod.ToDouble(); }
	fixed GetRotationPeriodAsFixed() const { return m_rotationPeriod; }
	double GetRotationPeriod() const
	{
		return m_rotationPeriod.ToDouble() * 60 * 60 * 24;
	}
	bool HasRotationPhase() const { return m_rotationalPhaseAtStart != fixed(0); }
	double GetRotationPhaseAtStart() const { return m_rotationalPhaseAtStart.ToDouble(); }
	double GetAxialTilt() const { return m_axialTilt.ToDouble(); }
	fixed GetAxialTiltAsFixed() const { return m_axialTilt; }

	const Orbit &GetOrbit() const { return m_orbit; }
	double GetEccentricity() const { return m_eccentricity.ToDouble(); }
	fixed GetEccentricityAsFixed() const { return m_eccentricity; }
	double GetOrbMin() const { return m_orbMin.ToDouble(); }
	double GetOrbMax() const { return m_orbMax.ToDouble(); }
	fixed GetOrbMinAsFixed() const { return m_orbMin; }
	fixed GetOrbMaxAsFixed() const { return m_orbMax; }
	double GetSemiMajorAxis() const { return m_semiMajorAxis.ToDouble(); }
	fixed GetSemiMajorAxisAsFixed() const { return m_semiMajorAxis; }
	fixed GetInclinationAsFixed() const { return m_inclination; }
	void SetOrbitPlane(const matrix3x3d &orient) { m_orbit.SetPlane(orient); }

	int GetAverageTemp() const { return m_averageTemp; }
	inline const std::string &GetHeightMapFilename() const { return m_heightMapFilename; }
	unsigned int GetHeightMapFractal() const { return m_heightMapFractal; }

	Uint32 GetSeed() const { return m_seed; }

	fixed GetMetallicityAsFixed() const { return m_metallicity; }
	double GetMetallicity() const { return m_metallicity.ToDouble(); }
	fixed GetVolatileGasAsFixed() const { return m_volatileGas; }
	double GetVolatileGas() const { return m_volatileGas.ToDouble(); }
	fixed GetVolatileLiquidAsFixed() const { return m_volatileLiquid; }
	double GetVolatileLiquid() const { return m_volatileLiquid.ToDouble(); }
	fixed GetVolatileIcesAsFixed() const { return m_volatileIces; }
	double GetVolatileIces() const { return m_volatileIces.ToDouble(); }
	fixed GetVolcanicityAsFixed() const { return m_volcanicity; }
	double GetVolcanicity() const { return m_volcanicity.ToDouble(); }
	double GetAtmosOxidizing() const { return m_atmosOxidizing.ToDouble(); }
	fixed GetLifeAsFixed() const { return m_life; }
	double GetLife() const { return m_life.ToDouble(); }

	fixed GetAgriculturalAsFixed() const { return m_agricultural; }
	double GetAgricultural() const { return m_agricultural.ToDouble(); }
	fixed GetPopulationAsFixed() const { return m_population; }
	double GetPopulation() const { return m_population.ToDouble(); }

	double CalcSurfaceGravity() const;

	double GetMaxChildOrbitalDistance() const;

	bool HasRings() const { return bool(m_rings.maxRadius.v); }
	const RingStyle &GetRings() const { return m_rings; }

	// XXX merge all this atmosphere stuff
	bool HasAtmosphere() const;

	Color GetAlbedo() const
	{
		// XXX suggestions about how to determine a sensible albedo colour would be welcome
		// Currently (2014-03-24) this is just used as the colour for the body billboard
		// which is rendered when the body has a small screen size
		return Color(200, 200, 200, 255);
	}

	void GetAtmosphereFlavor(Color *outColor, double *outDensity) const
	{
		*outColor = m_atmosColor;
		*outDensity = m_atmosDensity;
	}

	AtmosphereParameters CalcAtmosphereParams() const;

	bool IsScoopable() const;

	void Dump(FILE *file, const char *indent = "") const;

	StarSystem *GetStarSystem() const { return m_system; }

	const std::string &GetSpaceStationType() const { return m_space_station_type; }

private:
	friend class StarSystem;
	friend class ObjectViewerView;
	friend class StarSystemLegacyGeneratorBase;
	friend class StarSystemCustomGenerator;
	friend class StarSystemRandomGenerator;
	friend class PopulateStarSystemGenerator;

	void ClearParentAndChildPointers();

	SystemBody *m_parent;				  // these are only valid if the StarSystem
	std::vector<SystemBody *> m_children; // that create them still exists

	SystemPath m_path;
	Orbit m_orbit;
	Uint32 m_seed; // Planet.cpp can use to generate terrain
	std::string m_name;
	fixed m_radius;					// in earth radii for planets, sol radii for stars. equatorial radius in case of bodies which are flattened at the poles
	fixed m_aspectRatio;			// ratio between equatorial and polar radius for bodies with eqatorial bulges
	fixed m_mass;					// earth masses if planet, solar masses if star
	fixed m_orbMin, m_orbMax;		// periapsism, apoapsis in AUs
	fixed m_rotationPeriod;			// in days
	fixed m_rotationalPhaseAtStart; // 0 to 2 pi
	fixed m_humanActivity;			// 0 - 1
	fixed m_semiMajorAxis;			// in AUs
	fixed m_eccentricity;
	fixed m_orbitalOffset;
	fixed m_orbitalPhaseAtStart; // 0 to 2 pi
	fixed m_axialTilt;			 // in radians
	fixed m_inclination;		 // in radians, for surface bodies = latitude
	int m_averageTemp;
	BodyType m_type;
	bool m_isCustomBody;

	/* composition */
	fixed m_metallicity;	// (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
	fixed m_volatileGas;	// 1.0 = earth atmosphere density
	fixed m_volatileLiquid; // 1.0 = 100% ocean cover (earth = 70%)
	fixed m_volatileIces;	// 1.0 = 100% ice cover (earth = 3%)
	fixed m_volcanicity;	// 0 = none, 1.0 = fucking volcanic
	fixed m_atmosOxidizing; // 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
	fixed m_life;			// 0.0 = dead, 1.0 = teeming

	RingStyle m_rings;

	/* economy type stuff */
	fixed m_population;
	fixed m_agricultural;

	std::string m_heightMapFilename;
	unsigned int m_heightMapFractal;

	Color m_atmosColor;
	double m_atmosDensity;

	StarSystem *m_system;

	std::string m_space_station_type;
};

#endif // SYSTEMBODY_H
