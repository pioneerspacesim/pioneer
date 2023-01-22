// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SectorGenerator.h"

#include "CustomSystem.h"
#include "DateTime.h"
#include "Factions.h"
#include "Galaxy.h"
#include "GameSaveError.h"
#include "Json.h"
#include "utils.h"

#define Square(x) ((x) * (x))

namespace FrontierNames {
	static const char *sys_names[] = {
		"en", "la", "can", "be",
		"and", "phi", "eth", "ol",
		"ve", "ho", "a", "lia",
		"an", "ar", "ur", "mi",
		"in", "ti", "qu", "so",
		"ed", "ess", "ex", "io",
		"ce", "ze", "fa", "ay",
		"wa", "de", "ack", "gre"
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char *)));

	void GetName(std::string &name, Random &rng)
	{
		// add fragments to build a name
		int len = rng.Int32(2, 3);
		for (int i = 0; i < len; i++) {
			name += sys_names[rng.Int32(0, SYS_NAME_FRAGS - 1)];
		}

		name[0] = toupper(name[0]);
	}
} // namespace FrontierNames

namespace HybridNames {
	static const char *sys_names[] = {
		"en", "la", "can", "be",
		"and", "phi", "eth", "ol",
		"ve", "ho", "a", "lia",
		"an", "ar", "ur", "mi",
		"in", "ti", "qu", "so",
		"ed", "ess", "ex", "io",
		"ce", "ze", "fa", "ay",
		"wa", "de", "ack", "gre",

		//Doomdark-esque additions
		"img", "or", "ir", "dol",
		"orth", "angr", "igr", "ash",
		"el", "mor", "ul", "atr",
		"orm", "udr", "is", "ildr",
		"orn", "il", "iel", "im",
		"uk", "ium", "ia", "eon",
		"ob", "ak", "arg", "ber",
		"ane", "esh", "ad", "un",

		//WKFO
		"ank", "bur", "ist", "iz",
		"erz", "tra", "shir", "gu",
		"ant", "kon", "ya", "us",
		"esk", "ig", "kah", "zon",
		"tay", "ash", "mar", "van",
		"sus", "tar", "run", "isk",
		"hir", "gaz", "sun", "gat",
		"pi", "cis", "ele", "ova"
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char *)));

	void GetName(std::string &name, Random &rng)
	{
		// add fragments to build a name
		int len = rng.Int32(2, 3);
		for (int i = 0; i < len; i++) {
			name += sys_names[rng.Int32(0, SYS_NAME_FRAGS - 1)];
		}

		name[0] = toupper(name[0]);
	}
} // namespace HybridNames

namespace Doomdark {
	static const char *Prefixes[] = {
		"img", "dol", "lor", "ush", "mor", "tal", "car", "ulf", "as", "tor", "ob", "f", "gl",
		"s", "th", "gan", "mal", "im", "var", "hag", "zar", "anv", "ber", "kah", "ash"
	};
	static const unsigned int PREFIX_FRAGS = ((unsigned int)(sizeof(Prefixes) / sizeof(char *)));

	static const char *Midwords[] = {
		"ar", "or", "ir", "en", "orth", "angr", "igr", "ash", "el", "in", "ul", "atr", "orm", "udr", "is", "ildr"
	};
	static const unsigned int MIDWORD_FRAGS = ((unsigned int)(sizeof(Midwords) / sizeof(char *)));

	static const char *Suffixes[] = {
		"orn", "il", "iel", "im", "uk", "ium", "ia", "eon", "ay", "ak", "arg", "and", "ane", "esh", "ad", "un"
	};
	static const unsigned int SUFFIX_FRAGS = ((unsigned int)(sizeof(Suffixes) / sizeof(char *)));

	void GetName(std::string &name, Random &rng)
	{
		// Doodarken a name
		name += Prefixes[rng.Int32(0, PREFIX_FRAGS - 1)];
		name += Midwords[rng.Int32(0, MIDWORD_FRAGS - 1)];
		name += Suffixes[rng.Int32(0, SUFFIX_FRAGS - 1)];

		name[0] = toupper(name[0]);
	}
} // namespace Doomdark

namespace Katakana {
	// clang-format off
	static const char *StartFragments[] = {
		"kyo","gyo","shu","sho","chu","cho","hyu","myo",
		"ryu","chi","tsu","shi","ka","ki","ku","ke",
		"ko","ga","gi","gu","ge","go","sa","su",
		"se","so","za","zu","ze","ta","te","to",
		"da","na","ni","nu","ne","no","ha","hi",
		"fu","he","ho","ba","bi","bu","be","ma",
		"mi","mu","me","mo","ya","yu","yo","ri",
		"ru","wa","jo","a","i","u","e","o",
		
	};
	static const char *MiddleFragments[] = {
		"sshi","ppo","tto","mbo","kka","kyu","sho","chu",
		"chi","tsu","shi","ka","ki","ku","ke","ko",
		"ga","gi","gu","ge","go","sa","su","se",
		"so","za","ji","zu","ze","ta","te","to",
		"da","de","do","na","ni","nu","ne","no",
		"ha","hi","fu","ho","ba","bi","bu","be",
		"bo","ma","mi","mu","me","mo","ya","yo",
		"ra","ri","ru","re","ro","wa","ju","jo",
		"a","i","u","e","o","n",
	};
	static const char *EndFragments[] = {
		"ttsu","ppu","ssa","tto","tte","noh","mba","kko",
		"kyo","shu","chu","nyu","nyo","ryu","chi","tsu",
		"shi","ka","ki","ku","ke","ko","ga","gi",
		"gu","go","sa","su","se","so","za","ji",
		"zu","ze","zo","ta","te","to","da","de",
		"do","na","ni","ne","no","ha","hi","fu",
		"he","ho","ba","bi","bu","be","bo","ma",
		"mi","mu","me","mo","ya","yo","ra","ri",
		"ru","re","ro","wa","ja","jo","i","e",
		"o","n",
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string &name, Random &rng)
	{
		// beginning
		name += StartFragments[rng.Int32(0, NUM_START_FRAGS - 1)];

		// middle
		size_t count = rng.Int32(0, 2);
		for (size_t i = 0; i < count; i++) {
			name += MiddleFragments[rng.Int32(0, NUM_MIDDLE_FRAGS - 1)];
		}

		// end
		name += EndFragments[rng.Int32(0, NUM_END_FRAGS - 1)];

		// Capitalisation
		name[0] = toupper(name[0]);
	}
} // namespace Katakana

bool SectorCustomSystemsGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig *config)
{
	const int sx = sector->sx;
	const int sy = sector->sy;
	const int sz = sector->sz;
	const Sint64 dist = (1 + sx * sx + sy * sy + sz * sz);

	if ((sx >= -m_customOnlyRadius) && (sx <= m_customOnlyRadius - 1) &&
		(sy >= -m_customOnlyRadius) && (sy <= m_customOnlyRadius - 1) &&
		(sz >= -m_customOnlyRadius) && (sz <= m_customOnlyRadius - 1))
		config->isCustomOnly = true;

	const std::vector<const CustomSystem *> &systems = galaxy->GetCustomSystems()->GetCustomSystemsForSector(sx, sy, sz);
	if (systems.size() == 0) return true;

	Uint32 sysIdx = 0;
	for (std::vector<const CustomSystem *>::const_iterator it = systems.begin(); it != systems.end(); ++it, ++sysIdx) {
		const CustomSystem *cs = *it;
		Sector::System s(sector.Get(), sx, sy, sz, sysIdx);
		s.m_pos = Sector::SIZE * cs->pos;
		s.m_name = cs->name;
		s.m_other_names = cs->other_names;
		for (s.m_numStars = 0; s.m_numStars < cs->numStars; s.m_numStars++) {
			if (cs->primaryType[s.m_numStars] == 0) break;
			s.m_starType[s.m_numStars] = cs->primaryType[s.m_numStars];
		}
		s.m_customSys = cs;
		s.m_seed = cs->seed;
		if (cs->want_rand_explored) {
			/*
			 * 0 - ~500ly from sol: explored
			 * ~500ly - ~700ly (65-90 sectors): gradual
			 * ~700ly+: unexplored
			 */
			if (((dist <= Square(90)) && (dist <= Square(65) || rng.Int32(dist) <= Square(40))) || galaxy->GetFactions()->IsHomeSystem(SystemPath(sx, sy, sz, sysIdx)))
				s.m_explored = StarSystem::eEXPLORED_AT_START;
			else
				s.m_explored = StarSystem::eUNEXPLORED;
		} else {
			if (cs->explored)
				s.m_explored = StarSystem::eEXPLORED_AT_START;
			else
				s.m_explored = StarSystem::eUNEXPLORED;
		}
		sector->m_systems.push_back(s);
	}
	return true;
}

const std::string SectorRandomSystemsGenerator::GenName(RefCountedPtr<Galaxy> galaxy, const Sector &sec, Sector::System &sys, int si, Random &rng)
{
	std::string name;
	const int sx = sec.sx;
	const int sy = sec.sy;
	const int sz = sec.sz;
	const int dist = std::max(std::max(abs(sx), abs(sy)), abs(sz));

	int chance = 100;
	switch (sys.m_starType[0]) {
	case SystemBody::TYPE_STAR_O:
	case SystemBody::TYPE_STAR_B: break;
	case SystemBody::TYPE_STAR_A: chance += dist; break;
	case SystemBody::TYPE_STAR_F: chance += 2 * dist; break;
	case SystemBody::TYPE_STAR_G: chance += 4 * dist; break;
	case SystemBody::TYPE_STAR_K: chance += 8 * dist; break;
	case SystemBody::TYPE_STAR_O_GIANT:
	case SystemBody::TYPE_STAR_B_GIANT: chance = 50; break;
	case SystemBody::TYPE_STAR_A_GIANT: chance = int(0.2 * dist); break;
	case SystemBody::TYPE_STAR_F_GIANT: chance = int(0.4 * dist); break;
	case SystemBody::TYPE_STAR_G_GIANT: chance = int(0.5 * dist); break;
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
	case SystemBody::TYPE_STAR_M_HYPER_GIANT: chance = 1; break; //Should give a nice name almost all the time
	default: chance += 16 * dist; break;
	}

	Uint32 weight = rng.Int32(chance);
	if (weight < 500 || galaxy->GetFactions()->IsHomeSystem(SystemPath(sx, sy, sz, si))) {
		// well done. you get a "real" name
		int nameGen = rng.Int32(0, 3);
		switch (nameGen) {
		case 0: FrontierNames::GetName(name, rng); break;
		case 1: HybridNames::GetName(name, rng); break;
		case 2: Doomdark::GetName(name, rng); break;
		case 3: Katakana::GetName(name, rng); break;
		default:
			FrontierNames::GetName(name, rng);
			break;
		}
		return name;
	} else if (weight < 800) {
		char buf[128];
		snprintf(buf, sizeof(buf), "MJBN %d%+d%+d", rng.Int32(10, 999), sx, sy); // MJBN -> Morton Jordan Bennett Norris
		return buf;
	} else if (weight < 1200) {
		char buf[128];
		snprintf(buf, sizeof(buf), "SC %d%+d%+d", rng.Int32(1000, 9999), sx, sy);
		return buf;
	} else {
		char buf[128];
		snprintf(buf, sizeof(buf), "DSC %d%+d%+d", rng.Int32(1000, 9999), sx, sy);
		return buf;
	}
}

bool SectorRandomSystemsGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig *config)
{
	/* Always place random systems outside the core custom-only region */
	if (config->isCustomOnly)
		return true;

	const int sx = sector->sx;
	const int sy = sector->sy;
	const int sz = sector->sz;
	const int customCount = static_cast<Uint32>(sector->m_systems.size());
	const Sint64 dist = (1 + sx * sx + sy * sy + sz * sz);
	const Sint64 freq = (1 + sx * sx + sy * sy);

	const int numSystems = (rng.Int32(4, 20) * galaxy->GetSectorDensity(sx, sy, sz)) >> 8;
	sector->m_systems.reserve(numSystems);

	for (int i = 0; i < numSystems; i++) {
		Sector::System s(sector.Get(), sx, sy, sz, customCount + i);

		switch (rng.Int32(15)) {
		case 0:
			s.m_numStars = 4;
			break;
		case 1:
		case 2:
			s.m_numStars = 3;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			s.m_numStars = 2;
			break;
		default:
			s.m_numStars = 1;
			break;
		}

		s.m_pos.x = rng.Double(Sector::SIZE);
		s.m_pos.y = rng.Double(Sector::SIZE);
		s.m_pos.z = rng.Double(Sector::SIZE);

		/*
		 * 0 - ~500ly from sol: explored
		 * ~500ly - ~700ly (65-90 sectors): gradual
		 * ~700ly+: unexplored
		 */
		if (((dist <= Square(90)) && (dist <= Square(65) || rng.Int32(dist) <= Square(40))) || galaxy->GetFactions()->IsHomeSystem(SystemPath(sx, sy, sz, customCount + i)))
			s.m_explored = StarSystem::eEXPLORED_AT_START;
		else
			s.m_explored = StarSystem::eUNEXPLORED;

		// Frequencies are low enough that we probably don't need this anymore.
		if (freq > Square(10)) {
			const Uint32 weight = rng.Int32(1000000);
			if (weight < 1) {
				s.m_starType[0] = SystemBody::TYPE_STAR_IM_BH; // These frequencies are made up
			} else if (weight < 3) {
				s.m_starType[0] = SystemBody::TYPE_STAR_S_BH;
			} else if (weight < 5) {
				s.m_starType[0] = SystemBody::TYPE_STAR_O_WF;
			} else if (weight < 8) {
				s.m_starType[0] = SystemBody::TYPE_STAR_B_WF;
			} else if (weight < 12) {
				s.m_starType[0] = SystemBody::TYPE_STAR_M_WF;
			} else if (weight < 15) {
				s.m_starType[0] = SystemBody::TYPE_STAR_K_HYPER_GIANT;
			} else if (weight < 18) {
				s.m_starType[0] = SystemBody::TYPE_STAR_G_HYPER_GIANT;
			} else if (weight < 23) {
				s.m_starType[0] = SystemBody::TYPE_STAR_O_HYPER_GIANT;
			} else if (weight < 28) {
				s.m_starType[0] = SystemBody::TYPE_STAR_A_HYPER_GIANT;
			} else if (weight < 33) {
				s.m_starType[0] = SystemBody::TYPE_STAR_F_HYPER_GIANT;
			} else if (weight < 41) {
				s.m_starType[0] = SystemBody::TYPE_STAR_B_HYPER_GIANT;
			} else if (weight < 48) {
				s.m_starType[0] = SystemBody::TYPE_STAR_M_HYPER_GIANT;
			} else if (weight < 58) {
				s.m_starType[0] = SystemBody::TYPE_STAR_K_SUPER_GIANT;
			} else if (weight < 68) {
				s.m_starType[0] = SystemBody::TYPE_STAR_G_SUPER_GIANT;
			} else if (weight < 78) {
				s.m_starType[0] = SystemBody::TYPE_STAR_O_SUPER_GIANT;
			} else if (weight < 88) {
				s.m_starType[0] = SystemBody::TYPE_STAR_A_SUPER_GIANT;
			} else if (weight < 98) {
				s.m_starType[0] = SystemBody::TYPE_STAR_F_SUPER_GIANT;
			} else if (weight < 108) {
				s.m_starType[0] = SystemBody::TYPE_STAR_B_SUPER_GIANT;
			} else if (weight < 158) {
				s.m_starType[0] = SystemBody::TYPE_STAR_M_SUPER_GIANT;
			} else if (weight < 208) {
				s.m_starType[0] = SystemBody::TYPE_STAR_K_GIANT;
			} else if (weight < 250) {
				s.m_starType[0] = SystemBody::TYPE_STAR_G_GIANT;
			} else if (weight < 300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_O_GIANT;
			} else if (weight < 350) {
				s.m_starType[0] = SystemBody::TYPE_STAR_A_GIANT;
			} else if (weight < 400) {
				s.m_starType[0] = SystemBody::TYPE_STAR_F_GIANT;
			} else if (weight < 500) {
				s.m_starType[0] = SystemBody::TYPE_STAR_B_GIANT;
			} else if (weight < 700) {
				s.m_starType[0] = SystemBody::TYPE_STAR_M_GIANT;
			} else if (weight < 800) {
				s.m_starType[0] = SystemBody::TYPE_STAR_O; // should be 1 but that is boring
			} else if (weight < 2000) {					   // weight < 1300 / 20500
				s.m_starType[0] = SystemBody::TYPE_STAR_B;
			} else if (weight < 8000) { // weight < 7300
				s.m_starType[0] = SystemBody::TYPE_STAR_A;
			} else if (weight < 37300) { // weight < 37300
				s.m_starType[0] = SystemBody::TYPE_STAR_F;
			} else if (weight < 113300) { // weight < 113300
				s.m_starType[0] = SystemBody::TYPE_STAR_G;
			} else if (weight < 234300) { // weight < 234300
				s.m_starType[0] = SystemBody::TYPE_STAR_K;
			} else if (weight < 250000) { // weight < 250000
				s.m_starType[0] = SystemBody::TYPE_WHITE_DWARF;
			} else if (weight < 900000) { //weight < 900000
				s.m_starType[0] = SystemBody::TYPE_STAR_M;
			} else {
				s.m_starType[0] = SystemBody::TYPE_BROWN_DWARF;
			}
		} else {
			const Uint32 weight = rng.Int32(1000000);
			if (weight < 100) { // should be 1 but that is boring
				s.m_starType[0] = SystemBody::TYPE_STAR_O;
			} else if (weight < 1300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_B;
			} else if (weight < 7300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_A;
			} else if (weight < 37300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_F;
			} else if (weight < 113300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_G;
			} else if (weight < 234300) {
				s.m_starType[0] = SystemBody::TYPE_STAR_K;
			} else if (weight < 250000) {
				s.m_starType[0] = SystemBody::TYPE_WHITE_DWARF;
			} else if (weight < 900000) {
				s.m_starType[0] = SystemBody::TYPE_STAR_M;
			} else {
				s.m_starType[0] = SystemBody::TYPE_BROWN_DWARF;
			}
		}
		//Output("%d: %d%\n", sx, sy);

		if (s.m_numStars > 1) {
			s.m_starType[1] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.m_starType[0]));
			if (s.m_numStars > 2) {
				s.m_starType[2] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.m_starType[0]));
				s.m_starType[3] = SystemBody::BodyType(rng.Int32(SystemBody::TYPE_STAR_MIN, s.m_starType[2]));
			}
		}

		if ((s.m_starType[0] <= SystemBody::TYPE_STAR_A) && (rng.Int32(10) == 0)) {
			// make primary a giant. never more than one giant in a system
			if (freq > Square(10)) {
				const Uint32 weight = rng.Int32(1000);
				if (weight >= 999) {
					s.m_starType[0] = SystemBody::TYPE_STAR_B_HYPER_GIANT;
				} else if (weight >= 998) {
					s.m_starType[0] = SystemBody::TYPE_STAR_O_HYPER_GIANT;
				} else if (weight >= 997) {
					s.m_starType[0] = SystemBody::TYPE_STAR_K_HYPER_GIANT;
				} else if (weight >= 995) {
					s.m_starType[0] = SystemBody::TYPE_STAR_B_SUPER_GIANT;
				} else if (weight >= 993) {
					s.m_starType[0] = SystemBody::TYPE_STAR_O_SUPER_GIANT;
				} else if (weight >= 990) {
					s.m_starType[0] = SystemBody::TYPE_STAR_K_SUPER_GIANT;
				} else if (weight >= 985) {
					s.m_starType[0] = SystemBody::TYPE_STAR_B_GIANT;
				} else if (weight >= 980) {
					s.m_starType[0] = SystemBody::TYPE_STAR_O_GIANT;
				} else if (weight >= 975) {
					s.m_starType[0] = SystemBody::TYPE_STAR_K_GIANT;
				} else if (weight >= 950) {
					s.m_starType[0] = SystemBody::TYPE_STAR_M_HYPER_GIANT;
				} else if (weight >= 875) {
					s.m_starType[0] = SystemBody::TYPE_STAR_M_SUPER_GIANT;
				} else {
					s.m_starType[0] = SystemBody::TYPE_STAR_M_GIANT;
				}
			} else if (freq > Square(5))
				s.m_starType[0] = SystemBody::TYPE_STAR_M_GIANT;
			else
				s.m_starType[0] = SystemBody::TYPE_STAR_M;

			//Output("%d: %d%\n", sx, sy);
		}

		s.m_name = GenName(galaxy, *sector, s, customCount + i, rng);
		//Output("%s: \n", s.m_name.c_str());

		sector->m_systems.push_back(s);
	}
	return true;
}

void SectorPersistenceGenerator::SetExplored(Sector::System *sys, StarSystem::ExplorationState e, double time)
{
	if (e == StarSystem::eUNEXPLORED) {
		m_exploredSystems.erase(sys->GetPath());
	} else if (e == StarSystem::eEXPLORED_AT_START) {
		m_exploredSystems[sys->GetPath()] = 0;
	} else {
		assert(e == StarSystem::eEXPLORED_BY_PLAYER);
		Time::DateTime dt = Time::DateTime(3200, 1, 1, 0, 0, 0) + Time::TimeDelta(time, Time::Second);
		int year, month, day;
		dt.GetDateParts(&year, &month, &day);
		Sint32 date = day | month << 5 | year << 9;
		m_exploredSystems[sys->GetPath()] = date;
	}
}

bool SectorPersistenceGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig *config)
{
	if (galaxy->IsInitialized()) {
		for (Sector::System &secsys : sector->m_systems) {
			const auto iter = m_exploredSystems.find(SystemPath(secsys.sx, secsys.sy, secsys.sz, secsys.idx));
			if (iter != m_exploredSystems.end()) {
				Sint32 date = iter->second;
				if (date == 0) {
					secsys.m_explored = StarSystem::eEXPLORED_AT_START;
					secsys.m_exploredTime = 0.0;
				} else if (date > 0) {
					int year = date >> 9;
					int month = (date >> 5) & 0xf;
					int day = date & 0x1f;
					Time::DateTime dt(year, month, day);
					secsys.m_explored = StarSystem::eEXPLORED_BY_PLAYER;
					secsys.m_exploredTime = dt.ToGameTime();
				}
			}
		}
	}
	sector->onSetExplorationState.connect(sigc::mem_fun(this, &SectorPersistenceGenerator::SetExplored));
	return true;
}

void SectorPersistenceGenerator::FromJson(const Json &jsonObj, RefCountedPtr<Galaxy> galaxy)
{
	m_exploredSystems.clear();
	if (m_version < 1) {
		return;
	}

	// The layout of this data is really weird for historical reasons.
	// It used to be stored by a general System-information container type called PersistSystemData<>.

	try {
		Json dictArray = jsonObj["dict"].get<Json::array_t>();
		for (unsigned int arrayIndex = 0; arrayIndex < dictArray.size(); ++arrayIndex) {
			const Json &dictArrayEl = dictArray[arrayIndex];
			SystemPath path = SystemPath::FromJson(dictArrayEl);
			StrToAuto(&m_exploredSystems[path], dictArrayEl["value"]);
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void SectorPersistenceGenerator::ToJson(Json &jsonObj, RefCountedPtr<Galaxy> galaxy)
{
	// The layout of this data is really weird for historical reasons.
	// It used to be stored by a general System-information container type called PersistSystemData<>.

	Json dictArray = Json::array(); // Create JSON array to contain dict data.
	for (const auto &element : m_exploredSystems) {
		Json dictArrayEl({}); // Create JSON object to contain dict element.
		element.first.ToJson(dictArrayEl);
		dictArrayEl["value"] = AutoToStr(element.second);
		dictArray.push_back(dictArrayEl); // Append dict object to array.
	}
	jsonObj["dict"] = dictArray; // Add dict array to supplied object.
}
