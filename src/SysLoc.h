#ifndef _SYSLOC_H
#define _SYSLOC_H

class SysLoc {
public:
	SysLoc(): sectorX(0), sectorY(0), systemIdx(0) {}
	SysLoc(int sectorX, int sectorY, int systemIdx): sectorX(sectorX), sectorY(sectorY), systemIdx(systemIdx) {}
	int sectorX, sectorY, systemIdx;
	void Serialize() const;
	static void Unserialize(SysLoc *loc);
	friend bool operator<(const SysLoc &a, const SysLoc &b) {
		if (a.sectorX < b.sectorX) return true;
		if (a.sectorY < b.sectorY) return true;
		if ((a.sectorX == b.sectorX) && (a.sectorY == b.sectorY)) {
			return (a.systemIdx < b.systemIdx);
		}
		return false;
	}
};

#endif /* _SYSLOC_H */
