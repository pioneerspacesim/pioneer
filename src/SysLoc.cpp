#include "SysLoc.h"
#include "StarSystem.h"
#include "Pi.h"
#include "EquipType.h"

void SysLoc::Serialize(Serializer::Writer &wr) const
{
	wr.Int32(sectorX);
	wr.Int32(sectorY);
	wr.Int32(systemNum);
}

void SysLoc::Unserialize(Serializer::Reader &rd, SysLoc *loc)
{
	loc->sectorX = rd.Int32();
	loc->sectorY = rd.Int32();
	loc->systemNum = rd.Int32();
}
