// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sector.h"
#include "StarSystem.h"
#include "CustomSystem.h"
#include "Galaxy.h"

#include "Factions.h"
#include "utils.h"

static const unsigned int SYS_NAME_FRAGS = 32;
static const char *sys_names[SYS_NAME_FRAGS] =
{ "en", "la", "can", "be", "and", "phi", "eth", "ol", "ve", "ho", "a",
  "lia", "an", "ar", "ur", "mi", "in", "ti", "qu", "so", "ed", "ess",
  "ex", "io", "ce", "ze", "fa", "ay", "wa", "da", "ack", "gre" };

const float Sector::SIZE = 8.f;

SectorCache Sector::cache;

void Sector::GetCustomSystems(Random& rng)
{
	PROFILE_SCOPED()
	const std::vector<CustomSystem*> &systems = CustomSystem::GetCustomSystemsForSector(sx, sy, sz);
	if (systems.size() == 0) return;

	Uint32 sysIdx = 0;
	for (std::vector<CustomSystem*>::const_iterator it = systems.begin(); it != systems.end(); it++, sysIdx++) {
		const CustomSystem *cs = *it;
		System s(sx, sy, sz, sysIdx);
		s.p = SIZE*cs->pos;
		s.name = cs->name;
		for (s.numStars=0; s.numStars<cs->numStars; s.numStars++) {
			if (cs->primaryType[s.numStars] == 0) break;
			s.starType[s.numStars] = cs->primaryType[s.numStars];
		}
		s.customSys = cs;
		s.seed = cs->seed;
		if (cs->want_rand_explored) {
			/*
			 * 0 - ~500ly from sol: explored
			 * ~500ly - ~700ly (65-90 sectors): gradual
			 * ~700ly+: unexplored
			 */
			int dist = isqrt(1 + sx*sx + sy*sy + sz*sz);
			s.explored = ((dist <= 90) && ( dist <= 65 || rng.Int32(dist) <= 40)) || Faction::IsHomeSystem(SystemPath(sx, sy, sz, sysIdx));
		} else {
			s.explored = cs->explored;
		}
		m_systems.push_back(s);
	}
}

static const int CUSTOM_ONLY_RADIUS	= 4;

//////////////////////// Sector
Sector::Sector(const SystemPath& path) : m_factionsAssigned(false)
{
	PROFILE_SCOPED()
	Uint32 _init[4] = { Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rng(_init, 4);

	sx = path.sectorX; sy = path.sectorY; sz = path.sectorZ;

	GetCustomSystems(rng);
	int customCount = m_systems.size();

	/* Always place random systems outside the core custom-only region */
	if ((path.sectorX < -CUSTOM_ONLY_RADIUS) || (path.sectorX > CUSTOM_ONLY_RADIUS-1) ||
	    (path.sectorY < -CUSTOM_ONLY_RADIUS) || (path.sectorY > CUSTOM_ONLY_RADIUS-1) ||
	    (path.sectorZ < -CUSTOM_ONLY_RADIUS) || (path.sectorZ > CUSTOM_ONLY_RADIUS-1)) {
		int numSystems = (rng.Int32(4,20) * Galaxy::GetSectorDensity(path.sectorX, path.sectorY, path.sectorZ)) >> 8;

		for (int i=0; i<numSystems; i++) {
			System s(sx, sy, sz, customCount + i);

			switch (rng.Int32(15)) {
				case 0:
					s.numStars = 4; break;
				case 1: case 2:
					s.numStars = 3; break;
				case 3: case 4: case 5: case 6:
					s.numStars = 2; break;
				default:
					s.numStars = 1; break;
			}

			s.p.x = rng.Double(SIZE);
			s.p.y = rng.Double(SIZE);
			s.p.z = rng.Double(SIZE);

			s.seed = 0;
			s.customSys = 0;

			/*
			 * 0 - ~500ly from sol: explored
			 * ~500ly - ~700ly (65-90 sectors): gradual
			 * ~700ly+: unexplored
			 */
			int dist = isqrt(1 + sx*sx + sy*sy + sz*sz);
			s.explored = ((dist <= 90) && ( dist <= 65 || rng.Int32(dist) <= 40)) || Faction::IsHomeSystem(SystemPath(sx, sy, sz, customCount + i));

			Uint32 weight = rng.Int32(1000000);

			// Frequencies are low enough that we probably don't need this anymore.
			if (isqrt(1+sx*sx+sy*sy) > 10)
			{
				if (weight < 1) {
					s.starType[0] = SystemBody::TYPE_STAR_IM_BH;  // These frequencies are made up
				} else if (weight < 3) {
					s.starType[0] = SystemBody::TYPE_STAR_S_BH;
				} else if (weight < 5) {
					s.starType[0] = SystemBody::TYPE_STAR_O_WF;
				} else if (weight < 8) {
					s.starType[0] = SystemBody::TYPE_STAR_B_WF;
				} else if (weight < 12) {
					s.starType[0] = SystemBody::TYPE_STAR_M_WF;
				} else if (weight < 15) {
					s.starType[0] = SystemBody::TYPE_STAR_K_HYPER_GIANT;
				} else if (weight < 18) {
					s.starType[0] = SystemBody::TYPE_STAR_G_HYPER_GIANT;
				} else if (weight < 23) {
					s.starType[0] = SystemBody::TYPE_STAR_O_HYPER_GIANT;
				} else if (weight < 28) {
					s.starType[0] = SystemBody::TYPE_STAR_A_HYPER_GIANT;
				} else if (weight < 33) {
					s.starType[0] = SystemBody::TYPE_STAR_F_HYPER_GIANT;
				} else if (weight < 41) {
					s.starType[0] = SystemBody::TYPE_STAR_B_HYPER_GIANT;
				} else if (weight < 48) {
					s.starType[0] = SystemBody::TYPE_STAR_M_HYPER_GIANT;
				} else if (weight < 58) {
					s.starType[0] = SystemBody::TYPE_STAR_K_SUPER_GIANT;
				} else if (weight < 68) {
					s.starType[0] = SystemBody::TYPE_STAR_G_SUPER_GIANT;
				} else if (weight < 78) {
					s.starType[0] = SystemBody::TYPE_STAR_O_SUPER_GIANT;
				} else if (weight < 88) {
					s.starType[0] = SystemBody::TYPE_STAR_A_SUPER_GIANT;
				} else if (weight < 98) {
					s.starType[0] = SystemBody::TYPE_STAR_F_SUPER_GIANT;
				} else if (weight < 108) {
					s.starType[0] = SystemBody::TYPE_STAR_B_SUPER_GIANT;
				} else if (weight < 158) {
					s.starType[0] = SystemBody::TYPE_STAR_M_SUPER_GIANT;
				} else if (weight < 208) {
					s.starType[0] = SystemBody::TYPE_STAR_K_GIANT;
				} else if (weight < 250) {
					s.starType[0] = SystemBody::TYPE_STAR_G_GIANT;
				} else if (weight < 300) {
					s.starType[0] = SystemBody::TYPE_STAR_O_GIANT;
				} else if (weight < 350) {
					s.starType[0] = SystemBody::TYPE_STAR_A_GIANT;
				} else if (weight < 400) {
					s.starType[0] = SystemBody::TYPE_STAR_F_GIANT;
				} else if (weight < 500) {
					s.starType[0] = SystemBody::TYPE_STAR_B_GIANT;
				} else if (weight < 700) {
					s.starType[0] = SystemBody::TYPE_STAR_M_GIANT;
				} else if (weight < 800) {
					s.starType[0] = SystemBody::TYPE_STAR_O;  // should be 1 but that is boring
				} else if (weight < 2000) { // weight < 1300 / 20500
					s.starType[0] = SystemBody::TYPE_STAR_B;
				} else if (weight < 8000) { // weight < 7300
					s.starType[0] = SystemBody::TYPE_STAR_A;
				} else if (weight < 37300) { // weight < 37300
					s.starType[0] = SystemBody::TYPE_STAR_F;
				} else if (weight < 113300) { // weight < 113300
					s.starType[0] = SystemBody::TYPE_STAR_G;
				} else if (weight < 234300) { // weight < 234300
					s.starType[0] = SystemBody::TYPE_STAR_K;
				} else if (weight < 250000) { // weight < 250000
					s.starType[0] = SystemBody::TYPE_WHITE_DWARF;
				} else if (weight < 900000) {  //weight < 900000
					s.starType[0] = SystemBody::TYPE_STAR_M;
				} else {
					s.starType[0] = SystemBody::TYPE_BROWN_DWARF;
				}
			} else {
				if (weight < 100) { // should be 1 but that is boring
					s.starType[0] = SystemBody::TYPE_STAR_O;
				} else if (weight < 1300) {
					s.starType[0] = SystemBody::TYPE_STAR_B;
				} else if (weight < 7300) {
					s.starType[0] = SystemBody::TYPE_STAR_A;
				} else if (weight < 37300) {
					s.starType[0] = SystemBody::TYPE_STAR_F;
				} else if (weight < 113300) {
					s.starType[0] = SystemBody::TYPE_STAR_G;
				} else if (weight < 234300) {
					s.starType[0] = SystemBody::TYPE_STAR_K;
				} else if (weight < 250000) {
					s.starType[0] = SystemBody::TYPE_WHITE_DWARF;
				} else if (weight < 900000) {
					s.starType[0] = SystemBody::TYPE_STAR_M;
				} else {
					s.starType[0] = SystemBody::TYPE_BROWN_DWARF;
				}
			}
			//Output("%d: %d%\n", sx, sy);

			if (s.numStars > 1) {
				s.starType[1] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.starType[0]));
				if (s.numStars > 2) {
					s.starType[2] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.starType[0]));
					s.starType[3] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.starType[2]));
				}
			}

			if ((s.starType[0] <= SystemBody::TYPE_STAR_A) && (rng.Int32(10)==0)) {
				// make primary a giant. never more than one giant in a system
				// while
				if (isqrt(1+sx*sx+sy*sy) > 10)
				{
					weight = rng.Int32(1000);
					if (weight >= 999) {
						s.starType[0] = SystemBody::TYPE_STAR_B_HYPER_GIANT;
					} else if (weight >= 998) {
						s.starType[0] = SystemBody::TYPE_STAR_O_HYPER_GIANT;
					} else if (weight >= 997) {
						s.starType[0] = SystemBody::TYPE_STAR_K_HYPER_GIANT;
					} else if (weight >= 995) {
						s.starType[0] = SystemBody::TYPE_STAR_B_SUPER_GIANT;
					} else if (weight >= 993) {
						s.starType[0] = SystemBody::TYPE_STAR_O_SUPER_GIANT;
					} else if (weight >= 990) {
						s.starType[0] = SystemBody::TYPE_STAR_K_SUPER_GIANT;
					} else if (weight >= 985) {
						s.starType[0] = SystemBody::TYPE_STAR_B_GIANT;
					} else if (weight >= 980) {
						s.starType[0] = SystemBody::TYPE_STAR_O_GIANT;
					} else if (weight >= 975) {
						s.starType[0] = SystemBody::TYPE_STAR_K_GIANT;
					} else if (weight >= 950) {
						s.starType[0] = SystemBody::TYPE_STAR_M_HYPER_GIANT;
					} else if (weight >= 875) {
						s.starType[0] = SystemBody::TYPE_STAR_M_SUPER_GIANT;
					} else {
						s.starType[0] = SystemBody::TYPE_STAR_M_GIANT;
					}
				} else if (isqrt(1+sx*sx+sy*sy) > 5) s.starType[0] = SystemBody::TYPE_STAR_M_GIANT;
				else s.starType[0] = SystemBody::TYPE_STAR_M;

				//Output("%d: %d%\n", sx, sy);
			}

			s.name = GenName(s, customCount + i,  rng);
			//Output("%s: \n", s.name.c_str());

			m_systems.push_back(s);
		}
	}
}

Sector::~Sector()
{
	cache.RemoveFromAttic(SystemPath(sx, sy, sz));
}

float Sector::DistanceBetween(RefCountedPtr<const Sector> a, int sysIdxA, RefCountedPtr<const Sector> b, int sysIdxB)
{
	PROFILE_SCOPED()
	vector3f dv = a->m_systems[sysIdxA].p - b->m_systems[sysIdxB].p;
	dv += Sector::SIZE*vector3f(float(a->sx - b->sx), float(a->sy - b->sy), float(a->sz - b->sz));
	return dv.Length();
}

const std::string Sector::GenName(System &sys, int si, Random &rng)
{
	PROFILE_SCOPED()
	std::string name;
	const int dist = std::max(std::max(abs(sx),abs(sy)),abs(sz));

	int chance = 100;
	switch (sys.starType[0]) {
		case SystemBody::TYPE_STAR_O:
		case SystemBody::TYPE_STAR_B: break;
		case SystemBody::TYPE_STAR_A: chance += dist; break;
		case SystemBody::TYPE_STAR_F: chance += 2*dist; break;
		case SystemBody::TYPE_STAR_G: chance += 4*dist; break;
		case SystemBody::TYPE_STAR_K: chance += 8*dist; break;
		case SystemBody::TYPE_STAR_O_GIANT:
		case SystemBody::TYPE_STAR_B_GIANT: chance = 50; break;
		case SystemBody::TYPE_STAR_A_GIANT: chance = int(0.2*dist); break;
		case SystemBody::TYPE_STAR_F_GIANT: chance = int(0.4*dist); break;
		case SystemBody::TYPE_STAR_G_GIANT: chance = int(0.5*dist); break;
		case SystemBody::TYPE_STAR_K_GIANT:
		case SystemBody::TYPE_STAR_M_GIANT: chance = dist; break;
		case SystemBody::TYPE_STAR_O_SUPER_GIANT:
		case SystemBody::TYPE_STAR_B_SUPER_GIANT: chance = 10; break;
		case SystemBody::TYPE_STAR_A_SUPER_GIANT:
		case SystemBody::TYPE_STAR_F_SUPER_GIANT:
		case SystemBody::TYPE_STAR_G_SUPER_GIANT:
		case SystemBody::TYPE_STAR_K_SUPER_GIANT: chance = 15; break;
		case SystemBody::TYPE_STAR_M_SUPER_GIANT: chance = 20; break;
		case SystemBody::TYPE_STAR_O_HYPER_GIANT:
		case SystemBody::TYPE_STAR_B_HYPER_GIANT:
		case SystemBody::TYPE_STAR_A_HYPER_GIANT:
		case SystemBody::TYPE_STAR_F_HYPER_GIANT:
		case SystemBody::TYPE_STAR_G_HYPER_GIANT:
		case SystemBody::TYPE_STAR_K_HYPER_GIANT:
		case SystemBody::TYPE_STAR_M_HYPER_GIANT: chance = 1; break;  //Should give a nice name almost all the time
		default: chance += 16*dist; break;
	}

	Uint32 weight = rng.Int32(chance);
	if (weight < 500 || Faction::IsHomeSystem(SystemPath(sx, sy, sz, si))) {
		/* well done. you get a real name  */
		int len = rng.Int32(2,3);
		for (int i=0; i<len; i++) {
			name += sys_names[rng.Int32(0,SYS_NAME_FRAGS-1)];
		}
		name[0] = toupper(name[0]);
		return name;
	} else if (weight < 800) {
		char buf[128];
		snprintf(buf, sizeof(buf), "MJBN %d%+d%+d", rng.Int32(10,999),sx,sy); // MJBN -> Morton Jordan Bennett Norris
		return buf;
	} else if (weight < 1200) {
		char buf[128];
		snprintf(buf, sizeof(buf), "SC %d%+d%+d", rng.Int32(1000,9999),sx,sy);
		return buf;
	} else {
		char buf[128];
		snprintf(buf, sizeof(buf), "DSC %d%+d%+d", rng.Int32(1000,9999),sx,sy);
		return buf;
	}
}

bool Sector::WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const {
	PROFILE_SCOPED()
	if(sx >= Xmin && sx <= Xmax) {
		if(sy >= Ymin && sy <= Ymax) {
			if(sz >= Zmin && sz <= Zmax) {
				return true;
			}
		}
	}
	return false;
}

void Sector::AssignFactions()
{
	PROFILE_SCOPED()

	assert(!m_factionsAssigned);

	Uint32 index = 0;
	for (std::vector<Sector::System>::iterator system = m_systems.begin(); system != m_systems.end(); ++system, ++index ) {
		(*system).faction = Faction::GetNearestFaction(RefCountedPtr<const Sector>(this), index);
	}
	m_factionsAssigned = true;
}

/*	answer whether the system path is in this sector
*/
bool Sector::Contains(const SystemPath sysPath) const
{
	PROFILE_SCOPED()
	if (sx != sysPath.sectorX) return false;
	if (sy != sysPath.sectorY) return false;
	if (sz != sysPath.sectorZ) return false;
	return true;
}
