// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "galaxy/StarSystem.h"
#include "galaxy/CustomSystem.h"
#include "GalaxyCache.h"
#include "RefCounted.h"
#include <string>
#include <vector>

class Faction;

class Sector : public RefCounted {
	friend class GalaxyObjectCache<Sector, SystemPath::LessSectorOnly>;

public:
	// lightyears
	static const float SIZE;
	~Sector();

	static float DistanceBetween(RefCountedPtr<const Sector> a, int sysIdxA, RefCountedPtr<const Sector> b, int sysIdxB);
	static void Init();

	// Sector is within a bounding rectangle - used for SectorView m_sectorCache pruning.
	bool WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const;
	bool Contains(const SystemPath &sysPath) const;

	// get the SystemPath for this sector
	SystemPath GetPath() const { return SystemPath(sx, sy, sz); }

	class System {
	public:
		System(int x, int y, int z, Uint32 si): sx(x), sy(y), sz(z), idx(si), m_numStars(0), m_seed(0), m_customSys(nullptr), m_faction(nullptr), m_population(-1),
			m_explored(false) {};
		~System() {};

		static float DistanceBetween(const System* a, const System* b);

		// Check that we've had our habitation status set

		const std::string& GetName() const { return m_name; }
		const vector3f& GetPosition() const { return m_pos; }
		vector3f GetFullPosition() const { return Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + m_pos; };
		unsigned GetNumStars() const { return m_numStars; }
		SystemBody::BodyType GetStarType(unsigned i) const { assert(i < m_numStars); return m_starType[i]; }
		Uint32 GetSeed() const { return m_seed; }
		const CustomSystem* GetCustomSystem() const { return m_customSys; }
		const Faction* GetFaction() const { if (!m_faction) AssignFaction(); return m_faction; }
		fixed GetPopulation() const { return m_population; }
		void SetPopulation(fixed pop) { m_population = pop; }
		bool IsExplored() const { return m_explored; }

		bool IsSameSystem(const SystemPath &b) const {
			return sx == b.sectorX && sy == b.sectorY && sz == b.sectorZ && idx == b.systemIndex;
		}
		bool InSameSector(const SystemPath &b) const {
			return sx == b.sectorX && sy == b.sectorY && sz == b.sectorZ;
		}

		const int sx, sy, sz;
		const Uint32 idx;

	private:
		friend class Sector;

		void AssignFaction() const;

		std::string m_name;
		vector3f m_pos;
		unsigned m_numStars;
		SystemBody::BodyType m_starType[4];
		Uint32 m_seed;
		const CustomSystem* m_customSys;
		mutable Faction* m_faction; // mutable because we only calculate on demand
		fixed m_population;
		bool m_explored;
	};
	std::vector<System> m_systems;

	void Dump(FILE* file, const char* indent = "") const;

private:
	Sector(const Sector&); // non-copyable
	Sector& operator=(const Sector&); // non-assignable

	int sx, sy, sz;
	SectorCache* m_cache;

	Sector(const SystemPath& path, SectorCache* cache); // Only SectorCache(Job) are allowed to create sectors
	void SetCache(SectorCache* cache) { assert(!m_cache); m_cache = cache; }
	void GetCustomSystems(Random& rng);
	const std::string GenName(System &sys, int si, Random &rand);
	// sets appropriate factions for all systems in the sector
};

#endif /* _SECTOR_H */
