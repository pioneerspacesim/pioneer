#include "Sector.h"
#include "StarSystem.h"

#define SYS_NAME_FRAGS	32
static const char *sys_names[SYS_NAME_FRAGS] =
{ "en", "la", "can", "be", "and", "phi", "eth", "ol", "ve", "ho", "a",
  "lia", "an", "ar", "ur", "mi", "in", "ti", "qu", "so", "ed", "ess",
  "ex", "io", "ce", "ze", "fa", "ay", "wa", "da", "ack", "gre" };


//////////////////////// Sector
Sector::Sector(int x, int y)
{
	unsigned long _init[2] = { x, y };
	sx = x; sy = y;
	MTRand rng(_init, 2);

	m_numSystems = rng.Int32(3,6);

	for (int i=0; i<m_numSystems; i++) {
		System s;
		s.p.x = rng.Double(SIZE);
		s.p.y = rng.Double(SIZE);
		s.p.z = rng.Double(2*SIZE)-SIZE;
		s.name = GenName(rng);
		
		float spec = rng.Double(1.0);
		// frequencies from wikipedia
		if (spec < 0.0000003) {
			s.primaryStarClass = StarSystem::TYPE_STAR_O;
		} else if (spec < 0.0013) {
			s.primaryStarClass = StarSystem::TYPE_STAR_B;
		} else if (spec < 0.0073) {
			s.primaryStarClass = StarSystem::TYPE_STAR_A;
		} else if (spec < 0.0373) {
			s.primaryStarClass = StarSystem::TYPE_STAR_F;
		} else if (spec < 0.1133) {
			s.primaryStarClass = StarSystem::TYPE_STAR_G;
		} else if (spec < 0.2343) {
			s.primaryStarClass = StarSystem::TYPE_STAR_K;
		} else {
			s.primaryStarClass = StarSystem::TYPE_STAR_M;
		}

		m_systems.push_back(s);
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

