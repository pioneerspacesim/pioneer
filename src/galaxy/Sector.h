// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTOR_H
#define _SECTOR_H

#include "ExplorationState.h"
#include "GalaxyCache.h"
#include "GalaxyEnums.h"
#include "Random.h"
#include "RefCounted.h"
#include "galaxy/SystemPath.h"
#include "vector3.h"

#include <string>
#include <vector>
#include <sigc++/sigc++.h>

class CustomSystem;
class Faction;
class Galaxy;

class Sector : public RefCounted {
	friend class GalaxyGenerator;

	friend void SetCache(RefCountedPtr<Sector> sec, SectorCache *cache);

public:
	Sector(const Sector &) = delete; // non-copyable
	Sector &operator=(const Sector &) = delete; // non-assignable
	~Sector();

	// lightyears
	static const float SIZE;

	static float DistanceBetween(RefCountedPtr<const Sector> a, int sysIdxA, RefCountedPtr<const Sector> b, int sysIdxB);

	// Sector is within a bounding rectangle - used for SectorView m_sectorCache pruning.
	bool WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const;
	bool Contains(const SystemPath &sysPath) const;

	// get the SystemPath for this sector
	SystemPath GetPath() const { return SystemPath(sx, sy, sz); }

	class System {
		friend class Sector;
		friend class SectorCustomSystemsGenerator;
		friend class SectorRandomSystemsGenerator;
		friend class SectorPersistenceGenerator;

	public:
		System(Sector *sector, int x, int y, int z, Uint32 si) :
			sx(x),
			sy(y),
			sz(z),
			idx(si),
			m_sector(sector),
			m_numStars(0),
			m_seed(0),
			m_customSys(nullptr),
			m_faction(nullptr),
			m_population(-1),
			m_explored(ExplorationState::eUNEXPLORED),
			m_exploredTime(0.0)
		{}

		static float DistanceBetween(const System *a, const System *b);

		// Check that we've had our habitation status set

		const std::string &GetName() const { return m_name; }
		const std::vector<std::string> &GetOtherNames() const { return m_other_names; }
		const vector3f &GetPosition() const { return m_pos; }
		vector3f GetFullPosition() const { return Sector::SIZE * vector3f(float(sx), float(sy), float(sz)) + m_pos; };
		unsigned GetNumStars() const { return m_numStars; }
		GalaxyEnums::BodyType GetStarType(unsigned i) const
		{
			assert(i < m_numStars);
			return m_starType[i];
		}
		Uint32 GetSeed() const { return m_seed; }
		const CustomSystem *GetCustomSystem() const { return m_customSys; }
		const Faction *GetFaction() const
		{
			if (!m_faction) AssignFaction();
			return m_faction;
		}
		fixed GetPopulation() const { return m_population; }
		void SetPopulation(fixed pop) { m_population = pop; }
		ExplorationState GetExplored() const { return m_explored; }
		double GetExploredTime() const { return m_exploredTime; }
		bool IsExplored() const { return m_explored != ExplorationState::eUNEXPLORED; }
		void SetExplored(ExplorationState e, double time);

		bool IsSameSystem(const SystemPath &b) const
		{
			return sx == b.sectorX && sy == b.sectorY && sz == b.sectorZ && idx == b.systemIndex;
		}
		bool InSameSector(const SystemPath &b) const
		{
			return sx == b.sectorX && sy == b.sectorY && sz == b.sectorZ;
		}
		SystemPath GetPath() const { return SystemPath(sx, sy, sz, idx); }

		const int sx, sy, sz;
		const Uint32 idx;

	private:
		void AssignFaction() const;

		Sector *m_sector;
		std::string m_name;
		std::vector<std::string> m_other_names;
		vector3f m_pos;
		unsigned m_numStars;
		GalaxyEnums::BodyType m_starType[4];
		Uint32 m_seed;
		const CustomSystem *m_customSys;
		mutable const Faction *m_faction; // mutable because we only calculate on demand
		fixed m_population;
		ExplorationState m_explored;
		double m_exploredTime;
	};


	std::vector<System> m_systems;
	const int sx, sy, sz;

	void Dump(FILE *file, const char *indent = "") const;

	sigc::signal<void, Sector::System *, ExplorationState, double> onSetExplorationState;

private:
	RefCountedPtr<Galaxy> m_galaxy;
	SectorCache *m_cache;

	// Only SectorCache(Job) are allowed to create sectors
	Sector(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, SectorCache *cache);

	// sets appropriate factions for all systems in the sector
};

#endif /* _SECTOR_H */
