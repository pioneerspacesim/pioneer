#include "Sector.h"
#include "StarSystem.h"
#include "custom_starsystems.h"

#define SYS_NAME_FRAGS	32
static const char *sys_names[SYS_NAME_FRAGS] =
{ "en", "la", "can", "be", "and", "phi", "eth", "ol", "ve", "ho", "a",
  "lia", "an", "ar", "ur", "mi", "in", "ti", "qu", "so", "ed", "ess",
  "ex", "io", "ce", "ze", "fa", "ay", "wa", "da", "ack", "gre" };


void Sector::GetCustomSystems()
{
	int n=0;
	for (int i=0; ; i++) {
		if (custom_systems[i].name == 0) break;
		if ((custom_systems[i].sectorX == sx) &&
		    (custom_systems[i].sectorY == sy)) {
			n++;
			const CustomSystem *sys = &custom_systems[i];

			System s;
			s.p = SIZE*sys->pos;
			s.name = custom_systems[i].name;
			s.primaryStarClass = custom_systems[i].primaryType;
			s.customDef = sys->sbodies;
			m_systems.push_back(s);
		}
	}
}

//////////////////////// Sector
Sector::Sector(int x, int y)
{
	unsigned long _init[3] = { x, y, UNIVERSE_SEED };
	sx = x; sy = y;
	MTRand rng(_init, 3);

	GetCustomSystems();

	if (m_systems.size() != 0) {
		// custom sector

	} else {
		int numSystems = rng.Int32(3,6);

		for (int i=0; i<numSystems; i++) {
			System s;
			s.p.x = rng.Double(SIZE);
			s.p.y = rng.Double(SIZE);
			s.p.z = rng.Double(2*SIZE)-SIZE;
			s.name = GenName(rng);
			s.customDef = 0;
			
			float spec = rng.Int32(1000000);
			// frequencies from wikipedia
		/*	if (spec < 1) {
				s.primaryStarClass = StarSystem::TYPE_STAR_O;
			} else*/ if (spec < 1300) {
				s.primaryStarClass = StarSystem::TYPE_STAR_B;
			} else if (spec < 7300) {
				s.primaryStarClass = StarSystem::TYPE_STAR_A;
			} else if (spec < 37300) {
				s.primaryStarClass = StarSystem::TYPE_STAR_F;
			} else if (spec < 113300) {
				s.primaryStarClass = StarSystem::TYPE_STAR_G;
			} else if (spec < 234300) {
				s.primaryStarClass = StarSystem::TYPE_STAR_K;
			} else if (spec < 250000) {
				s.primaryStarClass = StarSystem::TYPE_WHITE_DWARF;
			} else {
				s.primaryStarClass = StarSystem::TYPE_STAR_M;
			}

			m_systems.push_back(s);
		}
	}
}

float Sector::DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB)
{
	vector3f dv = a->m_systems[sysIdxA].p - b->m_systems[sysIdxB].p;
	dv += Sector::SIZE*vector3f(a->sx - b->sx, a->sy - b->sy, 0);
	return dv.Length();
}

std::string Sector::GenName(MTRand &rng)
{
	std::string name;

	int len = rng.Int32(2,3);
	for (int i=0; i<len; i++) {
		name += sys_names[rng.Int32(0,SYS_NAME_FRAGS-1)];
	}
	name[0] = toupper(name[0]);
	return name;
}

