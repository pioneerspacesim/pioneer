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
	/* These are provided mostly for the benefit of lua.
	 * You normally want to use the methods on StarSystem */
	const char *GetSystemShortDescription() const;
	const char *GetSystemName() const;
	/** Caller owns the returned pointer */
	SBodyPath *GetRandomStarportNearButNotIn() const;
	SBodyPath *GetRootSBody() const;
	double GetSystemLawlessness() const;
	double GetSystemPopulation() const;
	OOLUA::Lua_table GetCommodityBasePriceAlterations(lua_State* l) const;
	bool IsCommodityLegal(int equip_type) const;
protected:
	/** Returns a cached StarSystem object, with limited lifetime as
	 * described in StarSystem::GetCached comment. */
	const StarSystem *Sys() const;
};

OOLUA_CLASS_NO_BASES(SysLoc)
	OOLUA_TYPEDEFS
		OOLUA::Equal_op
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_3(int, int, int)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_0_CONST(double, GetSystemLawlessness);
	OOLUA_MEM_FUNC_0_CONST(double, GetSystemPopulation);
	OOLUA_MEM_FUNC_1_CONST(OOLUA::Lua_table, GetCommodityBasePriceAlterations, lua_State*);
	OOLUA_MEM_FUNC_1_CONST(bool, IsCommodityLegal, int)
	OOLUA_MEM_FUNC_0_CONST(int, GetSectorX)
	OOLUA_MEM_FUNC_0_CONST(int, GetSectorY)
	OOLUA_MEM_FUNC_0_CONST(int, GetSystemNum)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetSystemShortDescription)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetSystemName)
	OOLUA_MEM_FUNC_0_CONST(OOLUA::lua_out_p<SBodyPath*>, GetRandomStarportNearButNotIn)
	OOLUA_MEM_FUNC_0_CONST(OOLUA::lua_out_p<SBodyPath*>, GetRootSBody)
OOLUA_CLASS_END

#endif /* _SYSLOC_H */
