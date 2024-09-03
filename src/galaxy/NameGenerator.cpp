// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NameGenerator.h"
#include "utils.h"

void NameGenerator::GetSystemName(std::string &name, Random &rng)
{
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
}

namespace FrontierNames {
	static const char *sys_names[] = {
		"en", "la", "can", "be",
		"and", "phi", "eth", "ol",
		"ve", "ho", "a", "lia",
		"an", "ar", "ur", "mi",
		"in", "ti", "qu", "so",
		"ed", "ess", "ex", "io",
		"ce", "ze", "fa", "ay",
		"wa", "de", "ack", "gre",
		"le", "du", "do", "ne"
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
		"le", "du", "do", "ne",

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
		"s", "th", "gan", "mal", "im", "var", "hag", "zar", "anv", "ber", "kah", "ash", "du"
	};
	static const unsigned int PREFIX_FRAGS = ((unsigned int)(sizeof(Prefixes) / sizeof(char *)));

	static const char *Midwords[] = {
		"ar", "or", "ir", "en", "orth", "angr", "igr", "ash", "el", "in", "ul", "atr", "orm", "udr", "is", "ildr"
	};
	static const unsigned int MIDWORD_FRAGS = ((unsigned int)(sizeof(Midwords) / sizeof(char *)));

	static const char *Suffixes[] = {
		"orn", "il", "iel", "im", "uk", "ium", "ia", "eon", "ay", "ak", "arg", "and", "ane", "esh", "ad", "un", "ne"
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
