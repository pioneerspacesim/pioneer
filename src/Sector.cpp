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
	MTRand rand(_init, 2);

	m_numSystems = rand(3,6);

	for (int i=0; i<m_numSystems; i++) {
		System s;
		s.p.x = rand(1.0);
		s.p.y = rand(1.0);
		s.p.z = 20.0*(rand(1.0)-0.5);
		s.name = GenName(rand);
		
		float spec = rand(1.0);
		// frequencies from wikipedia
		if (spec < 0.0000003) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_O;
		} else if (spec < 0.0013) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_B;
		} else if (spec < 0.0073) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_A;
		} else if (spec < 0.0373) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_F;
		} else if (spec < 0.1133) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_G;
		} else if (spec < 0.2343) {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_K;
		} else {
			s.primaryStarClass = StarSystem::SBody::SUBTYPE_STAR_M;
		}

		m_systems.push_back(s);
	}
}

std::string Sector::GenName(MTRand &rand)
{
	std::string name;

	int len = rand(2,4);
	for (int i=0; i<len; i++) {
		name += sys_names[rand(0,SYS_NAME_FRAGS-1)];
	}
	name[0] = toupper(name[0]);
	return name;
}

