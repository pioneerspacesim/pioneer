#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"
#include "CustomSystem.h"

class SectorLoc {
public:
	SectorLoc(): sectorX(0), sectorY(0) {}
	SectorLoc(int sectorX_, int sectorY_): sectorX(sectorX_), sectorY(sectorY_) {}
	int sectorX, sectorY;
	int GetSectorX() const { return sectorX; }
	int GetSectorY() const { return sectorY; }
	friend bool operator==(const SectorLoc &a, const SectorLoc &b) {
		if (a.sectorX != b.sectorX) return false;
		if (a.sectorY != b.sectorY) return false;
		return true;
	}
	friend bool operator!=(const SectorLoc &a, const SectorLoc &b) {
		return !(a==b);
	}
	friend bool operator<(const SectorLoc &a, const SectorLoc &b) {
		if (a.sectorX != b.sectorX) return (a.sectorX < b.sectorX);
		if (a.sectorY != b.sectorY) return (a.sectorY < b.sectorY);
		return false;
	}
};


class Sector {
public:
	// lightyears
	static const float SIZE;
	Sector(int x, int y);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	static void Init();
	// Sector is within a bounding rectangle - used for SectorView m_sectorCache pruning.
	bool WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax) const;
	
	class System {
	public:
		System() : customSys(0), pStarSystem(0) {};
		~System() {
			if( NULL!=pStarSystem ) {
				pStarSystem->Release();
			}
		}
	
		std::string name;
		vector3f p;
		int numStars;
		SBody::BodyType starType[4];
		Uint32 seed;
		const CustomSystem *customSys;
		StarSystem* pStarSystem;
	};
	std::vector<System> m_systems;
private:
	void GetCustomSystems();
	std::string GenName(System &sys, MTRand &rand);
	int sx, sy;
};

#endif /* _SECTOR_H */
