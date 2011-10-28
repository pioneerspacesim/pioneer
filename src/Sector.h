#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"
#include "CustomSystem.h"

class Sector {
public:
	// lightyears
	static const float SIZE;
	Sector(int x, int y, int z);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	static void Init();
	// Sector is within a bounding rectangle - used for SectorView m_sectorCache pruning.
	bool WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const;
		
	class System {
	public:
		System() : customSys(0), m_queriedStarSystem(false), m_isInhabited(false) {};
		~System() {};

		// Check that we've had our habitation status set
		bool IsSetInhabited() const { return m_queriedStarSystem; }
		void SetInhabited(bool inhabited) { m_isInhabited = inhabited; m_queriedStarSystem = true; }
		bool IsInhabited() const { return m_isInhabited; }
	
		// public members
		std::string name;
		vector3f p;
		int numStars;
		SBody::BodyType starType[4];
		Uint32 seed;
		const CustomSystem *customSys;

	private:
		bool m_queriedStarSystem;
		bool m_isInhabited;
	};
	std::vector<System> m_systems;
private:
	void GetCustomSystems();
	std::string GenName(System &sys, MTRand &rand);
	int sx, sy, sz;
};

#endif /* _SECTOR_H */
