#include "SysLoc.h"

void SysLoc::Serialize(Serializer::Writer &wr) const
{
	wr.Int32(sectorX);
	wr.Int32(sectorY);
	wr.Int32(systemIdx);
}

void SysLoc::Unserialize(Serializer::Reader &rd, SysLoc *loc)
{
	loc->sectorX = rd.Int32();
	loc->sectorY = rd.Int32();
	loc->systemIdx = rd.Int32();
}
