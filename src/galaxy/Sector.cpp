// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sector.h"
#include "CustomSystem.h"
#include "Galaxy.h"
#include "StarSystem.h"

#include "EnumStrings.h"
#include "Factions.h"

#include "core/StringUtils.h"
#include "profiler/Profiler.h"

const float Sector::SIZE = 8.f;

//////////////////////// Sector

Sector::Sector(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, SectorCache *cache) :
	sx(path.sectorX),
	sy(path.sectorY),
	sz(path.sectorZ),
	m_galaxy(galaxy),
	m_cache(cache) {}

Sector::~Sector()
{
	if (m_cache)
		m_cache->RemoveFromAttic(SystemPath(sx, sy, sz));
}

float Sector::DistanceBetween(RefCountedPtr<const Sector> a, int sysIdxA, RefCountedPtr<const Sector> b, int sysIdxB)
{
	PROFILE_SCOPED()
	vector3f dv = a->m_systems[sysIdxA].GetPosition() - b->m_systems[sysIdxB].GetPosition();
	dv += Sector::SIZE * vector3f(float(a->sx - b->sx), float(a->sy - b->sy), float(a->sz - b->sz));
	return dv.Length();
}

float Sector::DistanceBetweenSqr(const RefCountedPtr<const Sector> a, const int sysIdxA, const RefCountedPtr<const Sector> b, const int sysIdxB)
{
	PROFILE_SCOPED()
	vector3f dv = a->m_systems[sysIdxA].GetPosition() - b->m_systems[sysIdxB].GetPosition();
	dv += Sector::SIZE * vector3f(float(a->sx - b->sx), float(a->sy - b->sy), float(a->sz - b->sz));
	return dv.LengthSqr();
}

bool Sector::WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const
{
	PROFILE_SCOPED()
	if (sx >= Xmin && sx <= Xmax) {
		if (sy >= Ymin && sy <= Ymax) {
			if (sz >= Zmin && sz <= Zmax) {
				return true;
			}
		}
	}
	return false;
}

/*	answer whether the system path is in this sector
*/
bool Sector::Contains(const SystemPath &sysPath) const
{
	PROFILE_SCOPED()
	if (sx != sysPath.sectorX) return false;
	if (sy != sysPath.sectorY) return false;
	if (sz != sysPath.sectorZ) return false;
	return true;
}

void Sector::System::SetExplored(StarSystem::ExplorationState e, double time)
{
	if (e != m_explored) {
		m_sector->onSetExplorationState.emit(this, e, time);
		m_explored = e;
		m_exploredTime = time;
	}
}

void Sector::Dump(FILE *file, const char *indent) const
{
	fprintf(file, "Sector(%d,%d,%d) {\n", sx, sy, sz);
	fprintf(file, "\t" SIZET_FMT " systems\n", m_systems.size());
	for (const Sector::System &sys : m_systems) {
		assert(sx == sys.sx && sy == sys.sy && sz == sys.sz);
		fprintf(file, "\tSystem(%d,%d,%d,%u) {\n", sys.sx, sys.sy, sys.sz, sys.idx);
		fprintf(file, "\t\t\"%s\"\n", sys.GetName().c_str());
		fprintf(file, "\t\t%sEXPLORED%s\n", sys.IsExplored() ? "" : "UN", sys.GetCustomSystem() != nullptr ? ", CUSTOM" : "");
		fprintf(file, "\t\tfaction %s%s%s\n", sys.GetFaction() ? "\"" : "NONE", sys.GetFaction() ? sys.GetFaction()->name.c_str() : "", sys.GetFaction() ? "\"" : "");
		fprintf(file, "\t\tpos (%f, %f, %f)\n", double(sys.GetPosition().x), double(sys.GetPosition().y), double(sys.GetPosition().z));
		fprintf(file, "\t\tseed %u\n", sys.GetSeed());
		fprintf(file, "\t\tpopulation %.0f\n", sys.GetPopulation().ToDouble() * 1e9);
		fprintf(file, "\t\t%d stars%s\n", sys.GetNumStars(), sys.GetNumStars() > 0 ? " {" : "");
		for (unsigned i = 0; i < sys.GetNumStars(); ++i)
			fprintf(file, "\t\t\t%s\n", EnumStrings::GetString("BodyType", sys.GetStarType(i)));
		if (sys.GetNumStars() > 0) fprintf(file, "\t\t}\n");
		RefCountedPtr<const StarSystem> ssys = m_galaxy->GetStarSystem(SystemPath(sys.sx, sys.sy, sys.sz, sys.idx));
		assert(ssys->GetPath().IsSameSystem(SystemPath(sys.sx, sys.sy, sys.sz, sys.idx)));
		assert(ssys->GetNumStars() == sys.GetNumStars());
		assert(ssys->GetName() == sys.GetName());
		assert(ssys->GetUnexplored() == !sys.IsExplored());
		assert(ssys->GetFaction() == sys.GetFaction());
		assert(unsigned(ssys->GetSeed()) == sys.GetSeed());
		assert(ssys->GetNumStars() == sys.GetNumStars());
		for (unsigned i = 0; i < sys.GetNumStars(); ++i)
			assert(sys.GetStarType(i) == ssys->GetStars()[i]->GetType());
		ssys->Dump(file, "\t\t", true);
		fprintf(file, "\t}\n");
	}
	fprintf(file, "}\n\n");
}

float Sector::System::DistanceBetween(const System *a, const System *b)
{
	vector3f dv = a->GetPosition() - b->GetPosition();
	dv += Sector::SIZE * vector3f(float(a->sx - b->sx), float(a->sy - b->sy), float(a->sz - b->sz));
	return dv.Length();
}

void Sector::System::AssignFaction() const
{
	assert(m_sector->m_galaxy->GetFactions()->MayAssignFactions());
	m_faction = m_sector->m_galaxy->GetFactions()->GetNearestClaimant(this);
}
