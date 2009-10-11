#include "SysLoc.h"
#include "Serializer.h"

void SysLoc::Serialize() const
{
	using namespace Serializer::Write;
	wr_int(sectorX);
	wr_int(sectorY);
	wr_int(systemIdx);
}

void SysLoc::Unserialize(SysLoc *loc)
{
	using namespace Serializer::Read;
	loc->sectorX = rd_int();
	loc->sectorY = rd_int();
	loc->systemIdx = rd_int();
}
