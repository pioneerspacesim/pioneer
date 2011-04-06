#include "Sector.h"
#include "StarSystem.h"
#include "CustomSystem.h"
#include "Galaxy.h"

#define SYS_NAME_FRAGS	32
static const char *sys_names[SYS_NAME_FRAGS] =
{ "en", "la", "can", "be", "and", "phi", "eth", "ol", "ve", "ho", "a",
  "lia", "an", "ar", "ur", "mi", "in", "ti", "qu", "so", "ed", "ess",
  "ex", "io", "ce", "ze", "fa", "ay", "wa", "da", "ack", "gre" };

const float Sector::SIZE = 8;

void Sector::GetCustomSystems()
{
	const std::list<const CustomSystem*> systems = CustomSystem::GetCustomSystemsForSector(sx, sy);
	if (systems.size() == 0) return;

	for (std::list<const CustomSystem*>::const_iterator i = systems.begin(); i != systems.end(); i++) {
		const CustomSystem *cs = *i;
		System s;
		s.p = SIZE*cs->pos;
		s.name = cs->name;
		for (s.numStars=0; s.numStars<4; s.numStars++) {
			if (cs->primaryType[s.numStars] == 0) break;
			s.starType[s.numStars] = cs->primaryType[s.numStars];
		}
		s.customSys = cs;
		s.seed = cs->seed;
		m_systems.push_back(s);
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
		int numSystems = (rng.Int32(4,20) * Galaxy::GetSectorDensity(x, y)) >> 8;

		for (int i=0; i<numSystems; i++) {
			System s;
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
			s.p.z = rng.Double(2*SIZE)-SIZE;
			s.seed = 0;
			s.customSys = 0;
			
			float spec = rng.Int32(1000000);
			// frequencies from wikipedia
			/*if (spec < 100) { // should be 1 but that is boring
				s.starType[0] = SBody::TYPE_STAR_O;
			} else if (spec < 1300) {
				s.starType[0] = SBody::TYPE_STAR_B;
			} else if (spec < 7300) {
				s.starType[0] = SBody::TYPE_STAR_A;
			} else if (spec < 37300) {
				s.starType[0] = SBody::TYPE_STAR_F;
			} else if (spec < 113300) {
				s.starType[0] = SBody::TYPE_STAR_G;
			} else if (spec < 234300) {
				s.starType[0] = SBody::TYPE_STAR_K;
			} else if (spec < 250000) {
				s.starType[0] = SBody::TYPE_WHITE_DWARF;
			} else if (spec < 900000) {
				s.starType[0] = SBody::TYPE_STAR_M;
			} else {
				s.starType[0] = SBody::TYPE_BROWN_DWARF;
			}*/
			if (spec < 2) { 
				s.starType[0] = SBody::TYPE_STAR_IM_BH;  // These frequencies are made up
			} else if (spec < 10) {
				s.starType[0] = SBody::TYPE_STAR_S_BH; 
			} else if (spec < 45) {
				s.starType[0] = SBody::TYPE_STAR_K_HYPER_GIANT; 
			} else if (spec < 80) {
				s.starType[0] = SBody::TYPE_STAR_G_HYPER_GIANT;
			} else if (spec < 120) {
				s.starType[0] = SBody::TYPE_STAR_O_HYPER_GIANT; 
			} else if (spec < 200) {
				s.starType[0] = SBody::TYPE_STAR_A_HYPER_GIANT;
			} else if (spec < 300) {
				s.starType[0] = SBody::TYPE_STAR_F_HYPER_GIANT;
			} else if (spec < 500) {
				s.starType[0] = SBody::TYPE_STAR_B_HYPER_GIANT; 
			} else if (spec < 1000) {
				s.starType[0] = SBody::TYPE_STAR_M_HYPER_GIANT; 
			} else if (spec < 1100) {
				s.starType[0] = SBody::TYPE_STAR_K_SUPER_GIANT; 
			} else if (spec < 1200) {
				s.starType[0] = SBody::TYPE_STAR_G_SUPER_GIANT;
			} else if (spec < 1300) {
				s.starType[0] = SBody::TYPE_STAR_O_SUPER_GIANT; 
			} else if (spec < 1500) {
				s.starType[0] = SBody::TYPE_STAR_A_SUPER_GIANT;
			} else if (spec < 1700) {
				s.starType[0] = SBody::TYPE_STAR_F_SUPER_GIANT;
			} else if (spec < 2000) {
				s.starType[0] = SBody::TYPE_STAR_B_SUPER_GIANT; 
			} else if (spec < 3000) {
				s.starType[0] = SBody::TYPE_STAR_M_SUPER_GIANT; 
			} else if (spec < 3200) {
				s.starType[0] = SBody::TYPE_STAR_K_GIANT; 
			} else if (spec < 3400) {
				s.starType[0] = SBody::TYPE_STAR_G_GIANT;
			} else if (spec < 3600) {
				s.starType[0] = SBody::TYPE_STAR_O_GIANT; 
			} else if (spec < 3800) {
				s.starType[0] = SBody::TYPE_STAR_A_GIANT;
			} else if (spec < 4000) {
				s.starType[0] = SBody::TYPE_STAR_F_GIANT;
			} else if (spec < 4500) {
				s.starType[0] = SBody::TYPE_STAR_B_GIANT; 
			} else if (spec < 8000) {
				s.starType[0] = SBody::TYPE_STAR_M_GIANT; 
			} else if (spec < 9000) {
				s.starType[0] = SBody::TYPE_STAR_O;  // should be 1 but that is boring
			} else if (spec < 12500) { // spec < 1300 / 20500
				s.starType[0] = SBody::TYPE_STAR_B;  
			} else if (spec < 38000) { // spec < 7300
				s.starType[0] = SBody::TYPE_STAR_A;  
			} else if (spec < 64300) { // spec < 37300
				s.starType[0] = SBody::TYPE_STAR_F;  
			} else if (spec < 128300) { // spec < 113300
				s.starType[0] = SBody::TYPE_STAR_G;  
			} else if (spec < 247300) { // spec < 234300
				s.starType[0] = SBody::TYPE_STAR_K;  
			} else if (spec < 261000) { // spec < 250000
				s.starType[0] = SBody::TYPE_WHITE_DWARF;
			} else if (spec < 911000) {  //spec < 900000
				s.starType[0] = SBody::TYPE_STAR_M;
			} else {
				s.starType[0] = SBody::TYPE_BROWN_DWARF;
			}

			if (s.numStars > 1) {
				s.starType[1] = (SBody::BodyType)rng.Int32(SBody::TYPE_STAR_MIN, s.starType[0]);
				if (s.numStars > 2) {
					s.starType[2] = (SBody::BodyType)rng.Int32(SBody::TYPE_STAR_MIN, s.starType[0]);
					s.starType[3] = (SBody::BodyType)rng.Int32(SBody::TYPE_STAR_MIN, s.starType[2]);
				}
			}

			if ((s.starType[0] <= SBody::TYPE_STAR_A) && (rng.Int32(10)==0)) {
				// make primary a giant. never more than one giant in a system
				// while 
				if (rng.Int32(0,100) >= 95) {
					s.starType[0] = SBody::TYPE_STAR_B_HYPER_GIANT;
				} else if (rng.Int32(0,100) >= 92) {
					s.starType[0] = SBody::TYPE_STAR_O_HYPER_GIANT;
				} else if (rng.Int32(0,100) >= 88) {
					s.starType[0] = SBody::TYPE_STAR_K_HYPER_GIANT;
				} else if (rng.Int32(0,100) >= 82) {
					s.starType[0] = SBody::TYPE_STAR_B_SUPER_GIANT;
				} else if (rng.Int32(0,100) >= 78) {
					s.starType[0] = SBody::TYPE_STAR_O_SUPER_GIANT;
				} else if (rng.Int32(0,100) >= 73) {
					s.starType[0] = SBody::TYPE_STAR_K_SUPER_GIANT;
				} else if (rng.Int32(0,100) >= 69) {
					s.starType[0] = SBody::TYPE_STAR_B_GIANT;
				} else if (rng.Int32(0,100) >= 65) {
					s.starType[0] = SBody::TYPE_STAR_O_GIANT;
				} else if (rng.Int32(0,100) >= 60) {
					s.starType[0] = SBody::TYPE_STAR_K_GIANT;
				} else if (rng.Int32(0,100) >= 50) {
					s.starType[0] = SBody::TYPE_STAR_M_HYPER_GIANT;
				} else if (rng.Int32(0,100) >= 35) {
					s.starType[0] = SBody::TYPE_STAR_M_SUPER_GIANT;
				} else {
					s.starType[0] = SBody::TYPE_STAR_M_GIANT;
				}  
			}

			s.name = GenName(s, rng);

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

std::string Sector::GenName(System &sys, MTRand &rng)
{
	std::string name;
	const int dist = std::max(abs(sx),abs(sy));

	int chance = 100;
	switch (sys.starType[0]) {
		case SBody::TYPE_STAR_O:
		case SBody::TYPE_STAR_B: break;
		case SBody::TYPE_STAR_A: chance += dist; break;
		case SBody::TYPE_STAR_F: chance += 2*dist; break;
		case SBody::TYPE_STAR_G: chance += 4*dist; break;
		case SBody::TYPE_STAR_K: chance += 8*dist; break;
		default: chance += 16*dist; break;
	}
	if (rng.Int32(chance) < 100) {
		/* well done. you get a real name */
		int len = rng.Int32(2,3);
		for (int i=0; i<len; i++) {
			name += sys_names[rng.Int32(0,SYS_NAME_FRAGS-1)];
		}
		name[0] = toupper(name[0]);
		return name;
	} else {
		char buf[128];
		snprintf(buf, sizeof(buf), "SC %d%+d%+d", rng.Int32(1000,9999),sx,sy);
		return buf;
	}
}

