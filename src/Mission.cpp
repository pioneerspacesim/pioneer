#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "Mission.h"
//#include "missions/register.h"
#include "Serializer.h"
#include "Sector.h"

std::string Mission::NaturalSystemName(const SBodyPath &p)
{
	Sector s(p.sectorX, p.sectorY);
	return stringf(512, "the %s system", s.m_systems[p.systemIdx].name.c_str());
}

std::string Mission::NaturalSpaceStationName(const SBodyPath &p)
{
	Sector s(p.sectorX, p.sectorY);
	StarSystem sys(p.sectorX, p.sectorY, p.systemIdx);
	SBody *starport = sys.GetBodyByPath(&p);
	return stringf(512, "%s in the %s system [%d,%d]",
			starport->name.c_str(),
			s.m_systems[p.systemIdx].name.c_str(),
			p.sectorX, p.sectorY);
}

