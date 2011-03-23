#ifndef _SYSLOC_H
#define _SYSLOC_H

#include "Serializer.h"
#include "mylua.h"

class SBodyPath;

class SysLoc {
public:
	SysLoc(): sectorX(0), sectorY(0), systemNum(0) {}
	SysLoc(int sectorX, int sectorY, int systemNum): sectorX(sectorX), sectorY(sectorY), systemNum(systemNum) {}
	int sectorX, sectorY, systemNum;
	int GetSectorX() const { return sectorX; }
	int GetSectorY() const { return sectorY; }
	int GetSystemNum() const { return systemNum; }
	void Serialize(Serializer::Writer &wr) const;
	static void Unserialize(Serializer::Reader &rd, SysLoc *loc);
	friend bool operator==(const SysLoc &a, const SysLoc &b) {
		if (a.sectorX != b.sectorX) return false;
		if (a.sectorY != b.sectorY) return false;
		if (a.systemNum != b.systemNum) return false;
		return true;
	}
	friend bool operator!=(const SysLoc &a, const SysLoc &b) {
		return !(a==b);
	}
	friend bool operator<(const SysLoc &a, const SysLoc &b) {
		if (a.sectorX < b.sectorX) return true;
		if (a.sectorY < b.sectorY) return true;
		if ((a.sectorX == b.sectorX) && (a.sectorY == b.sectorY)) {
			return (a.systemNum < b.systemNum);
		}
		return false;
	}
};

#endif /* _SYSLOC_H */
