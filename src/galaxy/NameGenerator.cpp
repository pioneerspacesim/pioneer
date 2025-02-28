// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NameGenerator.h"
#include "utils.h"
#include "Random64.h"

void NameGenerator::GetSystemName(std::string& name, Random& rng, Random64 &rng64)
{
	const Uint64 weight = rng64.Int64(8025000000); //s
	if (weight < 30000) {
		Doomdark::GetName(name, rng);
	}
	else if (weight < 170732) {
		AborigineePhonetics::GetName(name, rng);
	}
	else if (weight < 356732) {
		MaoriPhonetics::GetName(name, rng);
	}
	else if (weight < 8356732) {
		Xhosa::GetName(name, rng);
	}
	else if (weight < 18356732) {
		Quechua::GetName(name, rng);
	}
	else if (weight < 34356732) {
		Kurmanji::GetName(name, rng);
	}
	else if (weight < 67136732) {
		FrontierNames::GetName(name, rng);
	}
	else if (weight < 145760082) {
		PersianPhonetics::GetName(name, rng);
	}
	else if (weight < 227500622) {
		Korean::GetName(name, rng);
	}
	else if (weight < 350946192) {
		Katakana::GetName(name, rng);
	}
	else if (weight < 591696192) {
		HybridNames::GetName(name, rng);
	}
	else if (weight < 901500412) {
		FrenchPhonetics::GetName(name, rng);
	}
	else if (weight < 1460579302) {
		Spanish::GetName(name, rng);
	}
	else if (weight < 2682222842) {
		Chinese::GetName(name, rng);
	}
	else {
		Westernees::GetName(name, rng);
	}
	//int nameGen = rng.Int32(0, 3);
	//switch (nameGen) {
	//case 0: FrontierNames::GetName(name, rng); break;
	//case 1: HybridNames::GetName(name, rng); break;
	//case 2: Doomdark::GetName(name, rng); break;
	//case 3: Katakana::GetName(name, rng); break;
	//default:
	//	FrontierNames::GetName(name, rng);
	//	break;
	//}
}

namespace FrontierNames {
	static const char* sys_names[] = {
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
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
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
	static const char* sys_names[] = {
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
		"pi", "cis", "ele", "ova",

		//Xhosa
		"a", "aba", "abe", "ama", "ba", "bu", "e", "eli",
		"em", "emi", "en", "endi", "endim", "eni", "eniba",
		"esi", "esiba", "ezi", "ezim", "ezin", "i",
		"iim", "iin", "ili", "im", "imi", "in",
		"isi", "izi", "izim", "izin", "ka",
		"ku", "kwa", "la", "li", "lu", "lwa", "m",
		"ma", "ndi", "ni", "o", "obu", "oku", "olu",
		"om", "oo", "sa", "si", "u", "ub", "ubu",
		"uku", "ukw", "ulu", "um", "utyw", "wa", "wu",
		"ya", "yi", "za", "zi",

		//Persian
		"a", "aa" "e", "i", "ee", "oo",
		"o", "u", "ey", "ow",
		"sab", "bad",
		"dil","sir", "ser", "kay" "gul",
		"nur", "roz", "naw", "zal", "dal",
		"khe", "he", "che", "Jim",
		"se", "te", "pe", "be",
		"hamze", "alef", "fe", "ghayn",
		"ayn", "zaa", "taa", "zaad",
		"saad", "shin", "sin", "zhe",
		"ze", "re", "ye", "vav",
		"he", "noon", "mim", "lam",
		"gaaf", "kaaf", "ghaf"

		//ARGH
		"al", "sea", "joh", "blu", "bla", "lif",
		"sta", "pul", "kol", "eve", "de", "eav",
		"ar", "fig", "bey", "di", "co", "ima",
		"eli", "bar", "leb", "wa", "ri", "as",
		"tra", "ink", "ske", "spa", "lib",

		//Aurek Besh
		"aurek", "besh", "cresh", "cherek",
		"dorn", "esk", "enth", "onith", "forn",
		"grek", "herf", "isk", "jenth", "krill",
		"krenth", "leth", "mern", "nern", "nen",
		"osk", "orenth", "peth", "qek", "resh",
		"senth", "sen", "trill", "thesh", "usk",
		"vev", "wesk", "xesh", "yirt", "zerek",

		//Sebacean
		"al", "tex", "miv", "am", "miox", "nex", "us",
		"arn", "tra", "fres", "lin", "sker", "nak",
		"bar", "ken", "bar", "tan", "tic", "bas", "sim",
		"bran", "dar", "met", "caf", "cha", "mik", "gam",
		"chel", "sik", "chol", "ivara", "dag", "yo",
		"drad", "dran", "nit", "drex", "im", "drib",
		"eema", "etal", "fahr", "bot", "fahz", "far",
		"ko", "fek", "fell", "ip", "ped", "rark", "frak",
		"nik", "fro", "tash", "gap", "pa", "gar", "anta",
		"brax", "tal", "gro", "lash", "lack", "ham", "man",
		"haz", "mot", "hen", "ta", "hez", "mana", "horo",
		"dalay", "jane", "ray", "jeli", "fan", "jil", "it",
		"jin", "ka", "ju", "kal", "nega", "chi", "voko",
		"nish", "kar", "jik", "kaz", "nick", "maia", "kay",
		"visha", "kosa", "laan", "kha", "vic", "kel", "kij",
		"klend", "kord", "la", "kras", "krawl", "krell", "kron",
		"zelta", "pemno", "lerg", "loo", "mas", "mag", "ra",
		"nessk"
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
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
	static const char* Prefixes[] = {
		"img", "dol", "lor", "ush", "mor", "tal", "car", "ulf", "as", "tor", "ob", "f", "gl",
		"s", "th", "gan", "mal", "im", "var", "hag", "zar", "anv", "ber", "kah", "ash", "du"
	};
	static const unsigned int PREFIX_FRAGS = ((unsigned int)(sizeof(Prefixes) / sizeof(char*)));

	static const char* Midwords[] = {
		"ar", "or", "ir", "en", "orth", "angr", "igr", "ash", "el", "in", "ul", "atr", "orm", "udr", "is", "ildr"
	};
	static const unsigned int MIDWORD_FRAGS = ((unsigned int)(sizeof(Midwords) / sizeof(char*)));

	static const char* Suffixes[] = {
		"orn", "il", "iel", "im", "uk", "ium", "ia", "eon", "ay", "ak", "arg", "and", "ane", "esh", "ad", "un", "ne"
	};
	static const unsigned int SUFFIX_FRAGS = ((unsigned int)(sizeof(Suffixes) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
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
	static const char* StartFragments[] = {
		"kyo","gyo","shu","sho","chu","cho","hyu","myo",
		"ryu","chi","tsu","shi","ka","ki","ku","ke",
		"ko","ga","gi","gu","ge","go","sa","su",
		"se","so","za","zu","ze","ta","te","to",
		"da","na","ni","nu","ne","no","ha","hi",
		"fu","he","ho","ba","bi","bu","be","ma",
		"mi","mu","me","mo","ya","yu","yo","ri",
		"ru","wa","jo","a","i","u","e","o",

	};
	static const char* MiddleFragments[] = {
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
	static const char* EndFragments[] = {
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

	void GetName(std::string& name, Random& rng)
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

namespace FrenchPhonetics {

	// clang-format off
	static const char* StartFragments[] = {
		"a", "an", "aceto", "actino", "adeno", "aero", "afro",
		"agri", "agro", "allo", "alter", "aix",
		"ambi", "amino", "amphi", "an",
		"ana", "andro", "anemo", "angio",
		"anglo", "aniso", "ante",
		"apico", "apo", "arachno", "araneo", "archeo",
		"archi", "aristo", "arriere", "astro", "atmo",
		"atto", "audio", "auto", "balano", "baro",
		"beau", "belgo", "belle", "benzo", "bi", "bor", "ber", "bes", "bre",
		"biblio", "bio", "brady", "bucco", "caco", "car", "can",
		"canado", "carcino", "cardio", "caryo", "cata", "chal",
		"centi", "ceto", "chimio", "chloro", "chromo", "cham", "char", "cler", "clich",
		"chrono", "cine", "cleido", "co", "con", "contra", "col", "cog",
		"contre", "cosmo", "cryo", "cyano",
		"cyber", "cyclo", "cyto", "dano", "de", "deca",
		"deci", "demi", "demo", "dendro", "des", "dij",
		"dextro", "di", "dia", "dis",
		"dodeca", "dors", "duodeno", "dys",
		"e", "echino", "eco", "ecto", "egypto", "eka",
		"electro", "em", "en", "endo", "entr'",
		"entre", "epi", "equi", "eroto",
		"euro", "ex", "exa", "exo", "extra",
		"femto", "ferro", "finno", "for", "franco",
		"frigo", "genio", "genito", "geo",
		"germano", "geronto", "giga", "glyco", "grand", "gre",
		"greco", "gyro", "hagio", "hecto", "heli",
		"helio", "helleno", "hemato", "hemi", "hemo",
		"hepato", "hepta", "hexa", "hippo",
		"hispano", "histo", "hollando", "holo",
		"homeo", "homo", "hydr", "hydro", "hygro",
		"hyper", "hypo", "hystero", "iatro", "ichtyo",
		"ideo", "igni", "il", "im", "in",
		"indo", "infra", "inter", "intra", "ir",
		"is", "ischio", "islamo", "iso", "italo",
		"japano", "judeo", "kilo", "lact", "lacto", "ly", "lil", "lour", "li",
		"lal.", "libano", "libro", "litho", "livro", "le ",
		"luso", "macro", "magneto", "mal", "malaco", "mar", "mai",
		"maroco", "me", "mega", "megalo", "melano",
		"melo", "meno", "mero", "mes", "meso",
		"meta", "metallo", "methyl", "metro", "mi",
		"micro", "milli", "mini", "Mme." "mnemo", "mono", "mont", "mor",
		"morpho", "multi", "musico", "myco", "myelo",
		"myria", "myrio", "myrmeco", "mytho", "n", "nan", "ni", "ni",
		"nano", "narco", "ne", "necro", "neo", "neuro",
		"nitro", "nona", "norvego", "octo", "odonto",
		"oeno", "oleo", "omni", "onycho", "ophio",
		"organo", "oro", "ortho", "osteo", "oxy", "or",
		"pan", "par", "para",
		"penta", "per", "peri", "peta",
		"petit", "petro", "phono", "phospho",
		"phyto", "pico", "piezo", "pluri",
		"pneumo", "poly", "post", "pre", "pro", "proto", "ptero",
		"ptil", "pyro", "quadri", "quasi", "quint",
		"r", "radio", "re", "retro", "rhino", "ren",
		"russo", "sai",  "schizo", "scytho", "seleno", "semi",
		"sero", "sesqui", "sino",  "sou", "sous",
		"stego", "stereo", "strepto", "sub", "stras",
		"sur", "sus", "sym", "syn",
		"syro", "tachy", "taxo",
		"tera", "tetra", "thanato", "tou",
		"theo", "trans", "tri", "tribo", "troy",
		"trideca", "turco", "ultra", "uni", "uro", "ver",
		"xeno", "xero", "yotta"

	};
	static const char* MiddleFragments[] = {
		"er","I","n","o","t", "cas", "vig", "-les-", "mem", " d\'", "mo", "le",
		"ge", "r", "an", "l", ""

	};
	static const char* EndFragments[] = {
		"able", "ace", "acee", "acees", "a", "ac", "ade", "adique", "age", " Arc",
		"ai", "aie", "aient", "ail", "aille", "ailler", "aillon",
		"ain", "aine", "aines", "aire", "ais", "aison", "ait",
		"al", "ale", "ales", "ames", "amment", "ance", "ane",
		"ange", "ant", "ante", "antes",
		"ants", "archie", "ard", "arde", "asse", "assent",
		"asses", "assiez", "assions", "at", "ate", "stes",
		"ateur", "atif", "ation", "atre", "atrice", "aud", "arve", "avre", "aux",
		"ax", "bourg", "bougie", " Bougie", "carpe", "cele", "cendre", " Cendre", "centrique", "cephale", "bren", "brin",
		"cephalie", "chorie", "chrome", "cois", "croix", "cide", "claste", "cole", "chateau", " Chateau"
		"coque", "crate", "cratie", "culteur", "cyte", "court", "cor", "cour",
		"dingue", "drome", "doire", "durance", "doron", "dun", "e", "e", "e", "eau", "eaux",
		"ectomie", "edre", "edrique", "ee", "el", "elet", "elette", "dons", "dens", "rande",
		"elle", "elles", "els", "eme", "emie", "ence", "ene", "en",
		"ent", "er", "era", "erai", "eraie", "eraient", "erais", "etincelle", " Etincelle",
		"erait", "erent", "eresse", "erez", "ergique", "erie", "eres"
		"erole", "erolle", "eron", "erons", "eront", "es", "etoile", " Etoile",
		"esque", "esse", "et", "ete", "ette", "eur", "euse", "euil", "ueil", "jouls", "jols",
		"eux", "ez", "feu", " Feu", "fere", "fier", "flamme", "forme", "fort", " Fort", "game", "gamie",
		"garou", "gate", "gene", "genese", "genie", "genique",
		"glosse", "gone", "gonie", "gramme", "graphe",
		"graphie", "graphique", "hem","heim", "ible", "iche", "ide", "indiquer", " Indiquer",
		"ide", "ie", "iel", "ieme", "ien", "ix", "ill", "ille", "elle", "ellez",
		"ienne", "ier", "iere", "iez", "if", "ificateur", "ification",
		"ifier", "illiard", "illion", "in", "ine", "ing", "ion", "iouls", "iols",
		"ions", "ique", "ir", "is", "isation", "ise", "iser", "isme",
		"isme", "isse", "issime", "iste", "it", "ite", "ite",
		"ites", "itude", "ive", "kini", "latre", "latrie", "lecithe",
		"lecte", "lingue", "lithe", "logie", "logique", "logiste", "loire",
		"logue", "lumiere", " Lumiere", "lyse", "man", "mancie", "mane", "manie", "me", "ment", "mer",
		"mere", "metre", "metrie", "metrique", "mnesie", "mont", "morphe", "muche", "mesnil", "menil",
		"naute", "nome", "nomie", "o", "oche", "odonte", "oide", "oient",
		"oique", "oir", "oire", "ois", "oison", "ol", "ole", "ome", "olles",
		"on", "onium", "ons", "onyme", "os", "ose", "osite", "ot",
		"ote", "oter", "otte", "ouse", "ouze", "oyer", "pathe", "pathie",
		"pause", "phage", "phagie", "phile", "philie", "phobie", "pleu", "plou",
		"phone", "phore", "plastie", "pole", "pont", "point", " Point", "porte", " Porte", "port", " Port", "ptere", "ptile", "punk",
		"quin", "re", "rice", "rrhee", "roanne", "rhone", "s", "saure", "scope", "scopie",
		"sphere", "srice", "te", "theque", "thermique", "tion", "tomie", "torche", " Torche", "tua",
		"trice", "trophe", "trophie", "u", "ueux", "ule", "unieme", "ure", "val",
		"ville", "viller", "vore", "wihr", "willer", "x", "yne",
		"carnac", "arradon", "arzal", "arles", "aravis", "arize", "ares", "armenaz",
		"archamps", "cassis", "galibier", "calanque", "chalet", " La Chaux",
		"chaumes", "montcalm", "cantal", "cucq", "Cocumont", "montcuq",
		"alisia", "vandoeuvres", "vendeuvre", "brive", " Brive", " Chabris", "chavre",
		"briare", "brioude", " Brioude", " Le Chasnay", "bois", "conde", " Conde", "verdun", "autun",
		"nanterre", " Nanterre", "bayeux", "chamarande", " Chamarande"
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
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
} //namespace FrenchPhonetics

namespace PersianPhonetics {
	static const char* sys_names[] = {
		"a", "aa" "e", "i", "ee", "oo",
		"o", "u", "ey", "ow",
		"sab", "bad",
		"dil","sir", "ser", "kay" "gul",
		"nur", "roz", "naw", "zal", "dal",
		"khe", "he", "che", "Jim",
		"se", "te", "pe", "be",
		"hamze", "alef", "fe", "ghayn",
		"ayn", "zaa", "taa", "zaad",
		"saad", "shin", "sin", "zhe",
		"ze", "re", "ye", "vav",
		"he", "noon", "mim", "lam",
		"gaaf", "kaaf", "ghaf"
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
	{
		// add fragments to build a name
		int len = rng.Int32(2, 3);
		for (int i = 0; i < len; i++) {
			name += sys_names[rng.Int32(0, SYS_NAME_FRAGS - 1)];
		}

		name[0] = toupper(name[0]);
	}
} //namespace PersianPhonetics

namespace AborigineePhonetics {
	static const char* sys_names[] = {
		"a", "i", "u", "aa",
		"ay", "aay", "uy", "ii",
		"dh", "dy", "gi", "ra",
		"ba","lu", "gan", "wam" "bi",
		"ga", "mang", "dan", "ngarra", "dan",
		"bud", "yaan", "ngun", "wang",
		"bil", "bi", "bal", "bu",
		"barru", "gid", "yay", "wirr",
		"ang", "barr", "bay", "waa",
		"wii", "wee", "see", "ma",
		"ya", "gurru", "gan", "balang",
		"wari", "yan", "wala", "ruu",
		"yula", "ma", "yu", "gay", "warra",
		"gul", "wagga", "ding", "gu", "dawa",
		"rang", "garin", "gali", "li", "mirri",
		"wan", "day", "ali", "dha", "yirra",
		"gina", "gin", "baany", "baan", "guwan",
		"yala", "wambi", "gulaa", "ngga",
		"yarra", "man", "wambu", "wuny", "daay",
		"barran", "dhang", "gura", "gugu", "bara",
		"yand", "rang", "ruu", "ngu", "gug",
		"bila", "durang", "du", "wi",
		"lay", "band", "haa", "di", "daa", "ny",
		"dhun", "dhu", "bab", "ila", "wam", "bad",
		"bir", "naa", "nhi", "nga", "ma", "wurru",
		"many", "min", "gaan", "gaa", "gang",
		"gaga", "birra", "gud", "hiin", "hina",
		"rrung", "mar", "gar", "buu", "rruu", "bin",
		"ur", "wal", "narr", " "
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
	{
		// add fragments to build a name
		int len = rng.Int32(2, 3);
		for (int i = 0; i < len; i++) {
			name += sys_names[rng.Int32(0, SYS_NAME_FRAGS - 1)];
		}

		name[0] = toupper(name[0]);
	}
} //namespace AborigineePhonetics

namespace MaoriPhonetics {
	static const char* sys_names[] = {
		"kai", "ma", "tua", "whaka",
		"o", "a", "anga", "hanga",
		"hia", "ko", "ia", "ina",
		"inga","kia", "kina", "ku" "mia",
		"na", "ranga", "ria", "tanga", "tia",
		"roa", "u", "nga", "ngia",
		"whia", "whina", "koa", "kao",
		"ae", "ake nei", "aua", "hei",
		"i", "ka", "kei", "ki",
		"ko", "me", "noa", "pohehe",
		"pa", "ho", "ha", "ma",
		"ri", "to", "hin", "ga",
		"ika", "ra", "tan", "ta", "ira",
		"ne", "ta", "hu", "gi", "uri", "wai",
		"po", "ro", "tua", "ke", "rau", "rea",
		"po", "te",
	};
	static const unsigned int SYS_NAME_FRAGS = ((unsigned int)(sizeof(sys_names) / sizeof(char*)));

	void GetName(std::string& name, Random& rng)
	{
		// add fragments to build a name
		int len = rng.Int32(2, 3);
		for (int i = 0; i < len; i++) {
			name += sys_names[rng.Int32(0, SYS_NAME_FRAGS - 1)];
		}

		name[0] = toupper(name[0]);
	}
} //namespace MaoriPhonetics

namespace Spanish {
	// clang-format off
	static const char* IntroFragments[] = {
		"los ", "santa ", "beni ", "san ", "agua "
		"aliso ", "alameda ", "ajo ", "alta ",
		"bonita ", "buena ", "bonanza ", "cerritos ",
		"chico ", "el ", "la ", "sierra ", "pila ",
		"estrella ", "vera ", "ceniza ", "llama ", "fuego ",
		"encendido ", "vela ", "cirio "
	};

	static const char* StartFragments[] = {
		"a","acro","adeno","adreno","aero","afro","agro","algo",
		"alo","an","ana","sobre","intra","sub","re","ben",
		"bene","bien","mal","des","dis","im","in","pro",
		"contra","anti","ante","super","hiper","hipo","homo","uni",
		"mono","bi","bis","tri","semi","poli","equi","cent",
		"centi","auto","retro","para","trans","suedo","co",
		"con","com","extra","anarco","an","anfi","angio","anglo",
		"antero","antropo","autico","aortico","arbori","archi","arqui","artro",
		"astragalo", "astro", "atero", "atmo", "atto", "auriculo",
		"azo", "baterio", "balano", "baro", "bati", "bi", "bio",
		"braqui", "braquio", "bronco", "cachi", "calcaneo", "cardio",
		"centro", "cervico", "ciano", "ciclo", "cis", "circun", "cito",
		"clado", "cloro", "co", "condra", "coraco", "cosmo", "costo",
		"crio", "cripto", "crono", "cuneo", "dactilo", "de",
		"dendro", "di", "duo", "eco", "ecto", "electro", "em",
		"en", "endo", "eno", "entero", "ento", "entomo",
		"entre", "epi", "equi", "eritro", "es", "esfeno",
		"espleno", "espondilo", "esterno", "estilo",
		"etno", "euro", "ex", "exo", "extra", "feno",
		"ferro", "filo", "fisio", "fito", "fono",
		"fosfo", "foto", "franco", "gastro", "genito",
		"geo", "geronto", "gineco", "grafo", "greco",
		"halo", "hecto", "helio", "hemato", "hemi",
		"hemo", "hepato", "hetero", "hidro",
		"hidroxi", "hiper", "hipo", "hispano", "histero",
		"histo", "holo", "homeo", "homo", "humero",
		"icno", "ideo", "ileo", "ilio", "infero", "infra",
		"inmuno", "inter", "iso", "isquio", "italo",
		"kilo", "lacto", "laparo", "leuco", "linfo", "lipo", "lito",
		"lumbo", "luso", "macro", "mal", "mega", "meso",
		"meta", "metil", "micro", "mielo", "mili", "mini", "mio", "mono",
		"morfo", "moto", "multi", "musculo", "nano", "narco", "naso",
		"necro", "nefro", "neo", "neumo", "neuro", "nipo", "nitro",
		"normo", "occipito", "oculo", "oligo", "omni", "omo", "onco",
		"onto", "orbito", "organo", "ornito", "orto", "osteo", "oxi", "paleo",
		"pan", "para", "parieto", "pato", "penta", "per", "peri", "piro",
		"pluri", "podo", "poli", "pos", "post", "postero", "pre", "pro",
		"proto", "psico", "pterigo", "quilo", "quimio", "quiro", "re", "recontra",
		"recto", "requete", "rete", "retro", "rino", "ruso", "sacro",
		"semi", "sero", "seudo", "sico", "sin", "sino", "so", "sobre",
		"son", "sub", "talamo", "tardo", "tele", "teno", "teo",
		"terato", "termo", "tetra", "tibio", "tiro", "toraco", "trans",
		"traqueo", "tras", "tri", "uni", "uro",  "utero", "vaso",
		"vestibulo", "vice", "xeno", "xero", "xilo", "zoo"

	};

	static const char* EndFragments[] = {
		"a",	"amento",	"aza",	"cilla",	"ecilla",	"ente",	"ezno",	"grafia",	"iego",	"io",	"lisis",	"miento",	"ota",	"tecnia",	"ura",
		"able",	"amiento",	"azgo",	"cillo",	"ecillo",	"ento",	"faga",	"grafo",	"iente",	"ir",	"lita",	"morfico",	"ote",	"termia",	"uria",
		"aca",	"ana",	"azo",	"cion",	"ecita",	"enza",	"fagia",	"grama",	"iento",	"isco",	"litico",	"morfo",	"pata",	"tico",	"uro",
		"aceo",	"ancia",	"bilidad",	"cita",	"ecito",	"ena",	"fago",	"Guadal",	"ifero",	"isimo",	"lito",	"nada",	"patia",	"tomia",	"usco",
		"acha",	"anga",	"biosis",	"cito",	"eco",	"eno",	"fila",	"i",	"ificar",	"ismo",	"loga",	"nauta",	"patico",	"tomo",	"uzco",
		"acho",	"ano",	"blasto",	"clasa",	"ectomia",	"eo",	"filia",	"ia",	"il",	"ista",	"logia",	"o",	"plastia",	"torio",	"x",
		"acion",	"ante",	"blema",	"cola",	"eda",	"er",	"filo",	"ia",	"illa",	"istan",	"logico",	"odo",	"podo",	"trofia",	"xion",
		"aco",	"anza",	"cardio",	"cracia",	"edad",	"era",	"fito",	"iano",	"illo",	"istico",	"logo",	"oico", "polio",	"trofo",	"zon",
		"ada",	"ar",	"cecilla",	"crata",	"edo",	"eria",	"fobia",	"iar",	"ilo",	"ita",	"lotodo",	"oide", "rragia",	"tropico", "zoo",
		"adgo",	"arca",	"cecillo",	"dad",	"edor",	"eriza",	"fobico",	"iatra",	"imento",	"itico",	"mana",	"oma", "saurio",	"tropo",	"zuela",
		"ado",	"ario",	"cecita",	"dermo",	"edora",	"erizo",	"fobo",	"iatria",	"imiento",	"itis",	"mancia",	"on", "sca",	"ucha",
		"ador",	"arquia",	"cecito",	"dero",	"edura",	"ero",	"fono",	"ible",	"in",	"ito",	"mancia",	"on",	"sco", "ucho",
		"adora",	"arraco",	"cefalia",	"dor",	"ega",	"es",	"forme",	"ichuela",	"in",	"itud",	"mania",	"ona",	"scopia",	"uco",
		"adura",	"arro",	"centrico",	"dora",	"ego",	"esa",	"foro",	"ico",	"ina",	"ivo",	"mano",	"onimo",	"scopico",	"udo",
		"aja",	"asa",	"centrismo",	"dromo",	"ejo",	"esco",	"gamia",	"idad",	"ing",	"izacion",	"megalia",	"or", "scopio",	"uela",
		"aje",	"astro",	"ceta",	"dumbre",	"ema",	"eta",	"genesis",	"ido",	"inga",	"izar",	"mente",	"orro",	"sfera",	"uelo",
		"ajo",	"ata",	"cete",	"dura",	"emia",	"ete",	"genia",	"idor",	"ingo",	"izo",	"mento",	"osa",	"sis",	"uja",
		"al",	"atico",	"cida",	"e",	"encia",	"ez",	"genico",	"idora",	"ino",	"landia",	"metria",	"osis",	"statico", "ujo",
		"algia",	"ato",	"cidio",	"ear",	"eno",	"eza",	"geno",	"idura",	"ino",	"latria",	"metrico",	"oso", "tad",	"umbre",
		"ambre",	"avo",	"cigotico",	"ecer",	"ense",	"ezna",	"gono",	"iega",	"io",	"lecto",	"metro",	"ostomia", "teca",	"uno",

	};
	// clang-format on

	static const unsigned int NUM_INTRO_FRAGS = COUNTOF(IntroFragments);
	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	//static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
	{

		Uint32 nameGen = rng.Int32(1000000000);
		if (nameGen < 219576720) {
			// intro
			name += IntroFragments[rng.Int32(0, NUM_INTRO_FRAGS - 1)];

			// beginning
			name += StartFragments[rng.Int32(0, NUM_START_FRAGS - 1)];

			// end
			name += EndFragments[rng.Int32(0, NUM_END_FRAGS - 1)];
		}
		else {
			// beginning
			name += StartFragments[rng.Int32(0, NUM_START_FRAGS - 1)];


			// end
			name += EndFragments[rng.Int32(0, NUM_END_FRAGS - 1)];
		}

		// Capitalisation
		name[0] = toupper(name[0]);
	}
} // namespace Spanish

namespace Chinese {
	// clang-format off
	static const char* StartFragments[] = {
		"lao","xiao","a","di","chu","ke","hao","nan",
		"chong","bo","chhiau","chun","e","sin","te","zhe",
		"zha", "shi", "hou", "fang", "wa", "pi", "ma", "gu",
		"ni", "ti", "nu", "che", "hu", "ying", "yin", "zu",
		"za", "bei", "shang", "wu", "dong", "xi", "jiang",
		"she", "an", "ning"

	};
	static const char* MiddleFragments[] = {
		"zho","gyu","ng","ji","ka", "sho", "n", "an", "a",
		"wo", "i", "ge", "yao", "jia", "", " "
	};
	static const char* EndFragments[] = {
		"zi", "tou", "\'r", "bian", "men", "xue", "zhuyi",
		"de", "ye", "u", "ting", "kan", "shou", "wan", "chi",
		"i", "tou", "shu", "gong", "po", "wang", "jing", "hai",
		"han", "kou", "xian", "du", "guan", "pu", "bao", "cun"
		"yi" "zhan", "du", "\'an", "ping", "he", "bo", "sang",
		"xin"
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
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
} // namespace Chinese

namespace Xhosa {
	// clang-format off
	static const char* StartFragments[] = {
		"a", "aba", "abe", "ama", "ba", "bu", "e", "eli",
		"em", "emi", "en", "endi", "endim", "eni", "eniba",
		"esi", "esiba", "ezi", "ezim", "ezin", "i",
		"iim", "iin", "ili", "im", "imi", "in",
		"isi", "izi", "izim", "izin", "ka",
		"ku", "kwa", "la", "li", "lu", "lwa", "m",
		"ma", "ndi", "ni", "o", "obu", "oku", "olu",
		"om", "oo", "sa", "si", "u", "ub", "ubu",
		"uku", "ukw", "ulu", "um", "utyw", "wa", "wu",
		"ya", "yi", "za", "zi"

	};

	static const char* EndFragments[] = {
		"ana", "eka", "ela", "isa", "isisa", "kazi",
		"wa"
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);

	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
	{
		// beginning
		name += StartFragments[rng.Int32(0, NUM_START_FRAGS - 1)];


		// end
		name += EndFragments[rng.Int32(0, NUM_END_FRAGS - 1)];

		// Capitalisation
		name[0] = toupper(name[0]);
	}
} // namespace Xhosa

namespace Quechua {
	// clang-format off
	static const char* StartFragments[] = {
		"cha-", "chaw-", "chi-", "chka-",
		"chra-", "chu-", "cha-", "hina-",
		"kama-", "kta-", "ku-", "kuna-",
		"lla-", "m-", "man-", "manta-",
		"mi-", "mu-", "n-", "na-",
		"naku-", "naya-", "nchik-", "nchis-",
		"ni-", "nki-", "nkichik-", "nkichis-",
		"nku-", "nnaq-", "ntin-", "p-",
		"pa-", "paq-", "pas-", "paya-",
		"pi-", "pis-", "pita-", "pti-",
		"puni-", "pura-", "q-", "qa-",
		"qti-", "raq-", "rayku-", "ri-",
		"rpa-", "rqa-", "rqu-", "s-",
		"sa-", "sh-", "sha-", "shi-",
		"si-", "spa-", "sqa-", "ta-",
		"taq-", "wan-", "y-", "ya-",
		"ykacha-", "yki-", "ykichik-", "ykichis-",
		"yku-", "yna-", "yoq-", "ysi-",
		"yuq-", "na-", "niqi-"

	};

	static const char* MiddleFragments[] = {
		"cha", "chaw", "chi", "chka",
		"chra", "chu", "cha", "hina",
		"kama", "kta", "ku", "kuna",
		"lla", "m", "man", "manta",
		"mi", "mu", "n", "na",
		"naku", "naya", "nchik", "nchis",
		"ni", "nki", "nkichik", "nkichis",
		"nku", "nnaq", "ntin", "p",
		"pa", "paq", "pas", "paya",
		"pi", "pis", "pita", "pti",
		"puni", "pura", "q", "qa",
		"qti", "raq", "rayku", "ri",
		"rpa", "rqa", "rqu", "s",
		"sa", "sh", "sha", "shi",
		"si", "spa", "sqa", "ta",
		"taq", "wan", "y", "ya",
		"ykacha", "yki", "ykichik", "ykichis",
		"yku", "yna", "yoq", "ysi",
		"yuq", "na", "niqi"
	};

	static const char* EndFragments[] = {
		"-cha", "-chaw", "-chi", "-chka",
		"-chra", "-chu", "-cha", "-hina",
		"-kama", "-kta", "-ku", "-kuna",
		"-lla", "-m", "-man", "-manta",
		"-mi", "-mu", "-n", "-na",
		"-naku", "-naya", "-nchik", "-nchis",
		"-ni", "-nki", "-nkichik", "-nkichis",
		"-nku", "-nnaq", "-ntin", "-p",
		"-pa", "-paq", "-pas", "-paya",
		"-pi", "-pis", "-pita", "-pti",
		"-puni", "-pura", "-q", "-qa",
		"-qti", "-raq", "-rayku", "-ri",
		"-rpa", "-rqa", "-rqu", "-s",
		"-sa", "-sh", "-sha", "-shi",
		"-si", "-spa", "-sqa", "-ta",
		"-taq", "-wan", "-y", "-ya",
		"-ykacha", "-yki", "-ykichik", "-ykichis",
		"-yku", "-yna", "-yoq", "-ysi",
		"-yuq", "-na", "-niqi"
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
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
} // namespace Quechua

namespace Kurmanji {
	// clang-format off
	static const char* StartFragments[] = {
		"anti", "bak", "bi", "bin",
		"biyo", "be", "der", "dij",
		"etno", "et", "hidro", "hiper",
		"kar", "koz", "kilo", "maksi",
		"mikro", "mini", "ne", "niv",
		"mak", "mi", "ki", "hi", "an",
		"pas", "pes", "kozmo", "roj"

	};

	static const char* MiddleFragments[] = {
		"e", "di", "ter", "a", ""
	};

	static const char* EndFragments[] = {
		"iyo", "per", "dro", "lo", "kro",
		"mo", "ahi", "amin", "andin", "ane",
		"ane", "bar", "dank", "dar", "ek",
		"er", "geh", "istan", "loji", "nas",
		"pati", "stan", "ti", "van", "xane",
		"xaz", "xaz", "xwaz", "yek", "yi", "va",
		"i", "in", "ist", "izm", "qar", "y",
		"yen"
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
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
} // namespace Kurmanji

namespace Korean {
	// clang-format off
	static const char* StartFragments[] = {
		"a", "ya", "eo", "yeo", "o", "yo", "u", "yu", "eu", "i", "ae",
		"g", "n", "d", "r", "m", "b", "s", "o", "j", "ch", "k", "t",
		"p", "h", "ga", "gya", "gyeo", "go", "gyo", "gu", "gyu", "geu", "gi",
		"gae", "na", "nya", "neo", "nyeo", "no", "nyo", "nu", "nyu", "neu",
		"ni", "nae", "da", "dya", "deo", "dyeo", "do", "dyo", "du", "dyu", "deu",
		"di", "dae", "ra", "rya", "reo", "ryeo", "ro", "ro", "ryo", "ru", "ryu",
		"reu", "ri", "rae", "ma", "mya", "meo", "myeo", "mo", "myo", "mu", "myu",
		"meu", "mi", "mae", "ba", "bya", "beo", "byeo", "bo", "byo", "bu", "byu",
		"beu", "bi", "bae", "sa", "sya", "seo", "syeo", "so", "syo", "su", "syu",
		"seu", "si", "sae", "ja", "jya", "jeo", "jyeo", "jo", "jyo", "ju", "jyu",
		"jeu", "ji", "jae", "cha", "chya", "cheo", "chyeo", "cho", "chyo", "chu",
		"chyu", "cheu", "chi", "chae", "ka", "kya", "keo", "kyeo", "ko", "kyo",
		"ku", "kyu", "keu", "ki", "kae", "ta", "tya", "teo", "tyeo", "to", "tyo",
		"tu", "tyu", "teu", "ti", "tae", "pa", "pya", "peo", "pyeo", "po", "pyo",
		"pu", "pyu", "peu", "pi", "pae", "ha", "hya", "heo", "hyeo", "ho", "hyo",
		"hu", "hyu", "heu", "hi", "hae",

		"yae", "e", "ye", "wa", "wae", "oe", "wo", "we", "wi", "ui",
		"gyae", "ge", "gye", "gwa", "gwae", "goe", "gwo", "gwe", "gwi", "gui",
		"nyae", "ne", "nye", "nwa", "nwae", "noe", "nwo", "nwe", "nwi", "nui",
		"dyae", "de", "dye", "dwa", "dwae", "doe", "dwo", "dwe", "dwi", "dui",
		"ryae", "re", "rye", "rwa", "rwae", "roe", "rwo", "rwe", "rwi", "rui",
		"myae", "me", "mye", "mwa", "mwae", "moe", "mwo", "mwe", "mwi", "mui",
		"byae", "be", "bye", "bwa", "bwae", "boe", "bwo", "bwe", "bwi", "bui",
		"syae", "se", "sye", "swa", "swae", "soe", "swo", "swe", "swi", "sui",
		"oyae", "oye", "owa", "owae", "owo", "owe", "owi", "oui",
		"jyae", "je", "jye", "jwa", "jwae", "joe", "jwo", "jwe", "jwi", "jui",
		"chyae", "che", "chye", "chwa", "chwae", "choe", "chwo", "chwe", "chwi", "chui",
		"kyae", "ke", "kye", "kwa", "kwae", "koe", "kwo", "kwe", "kwi", "kui",
		"tyae", "te", "tye", "twa", "twae", "toe", "two", "twe", "twi", "tui",
		"pyae", "pe", "pye", "pwa", "pwae", "poe", "pwo", "pwe", "pwi", "pui",
		"hyae", "he", "hye", "hwa", "hwae", "hoe", "hwo", "hwe", "hwi", "hui",

	};

	static const char* MiddleFragments[] = {
		"a", "ya", "eo", "yeo", "o", "yo", "u", "yu", "eu", "i", "ae",
		"g", "n", "d", "r", "m", "b", "s", "o", "j", "ch", "k", "t",
		"p", "h", "ga", "gya", "gyeo", "go", "gyo", "gu", "gyu", "geu", "gi",
		"gae", "na", "nya", "neo", "nyeo", "no", "nyo", "nu", "nyu", "neu",
		"ni", "nae", "da", "dya", "deo", "dyeo", "do", "dyo", "du", "dyu", "deu",
		"di", "dae", "ra", "rya", "reo", "ryeo", "ro", "ro", "ryo", "ru", "ryu",
		"reu", "ri", "rae", "ma", "mya", "meo", "myeo", "mo", "myo", "mu", "myu",
		"meu", "mi", "mae", "ba", "bya", "beo", "byeo", "bo", "byo", "bu", "byu",
		"beu", "bi", "bae", "sa", "sya", "seo", "syeo", "so", "syo", "su", "syu",
		"seu", "si", "sae", "ja", "jya", "jeo", "jyeo", "jo", "jyo", "ju", "jyu",
		"jeu", "ji", "jae", "cha", "chya", "cheo", "chyeo", "cho", "chyo", "chu",
		"chyu", "cheu", "chi", "chae", "ka", "kya", "keo", "kyeo", "ko", "kyo",
		"ku", "kyu", "keu", "ki", "kae", "ta", "tya", "teo", "tyeo", "to", "tyo",
		"tu", "tyu", "teu", "ti", "tae", "pa", "pya", "peo", "pyeo", "po", "pyo",
		"pu", "pyu", "peu", "pi", "pae", "ha", "hya", "heo", "hyeo", "ho", "hyo",
		"hu", "hyu", "heu", "hi", "hae",

		"yae", "e", "ye", "wa", "wae", "oe", "wo", "we", "wi", "ui",
		"gyae", "ge", "gye", "gwa", "gwae", "goe", "gwo", "gwe", "gwi", "gui",
		"nyae", "ne", "nye", "nwa", "nwae", "noe", "nwo", "nwe", "nwi", "nui",
		"dyae", "de", "dye", "dwa", "dwae", "doe", "dwo", "dwe", "dwi", "dui",
		"ryae", "re", "rye", "rwa", "rwae", "roe", "rwo", "rwe", "rwi", "rui",
		"myae", "me", "mye", "mwa", "mwae", "moe", "mwo", "mwe", "mwi", "mui",
		"byae", "be", "bye", "bwa", "bwae", "boe", "bwo", "bwe", "bwi", "bui",
		"syae", "se", "sye", "swa", "swae", "soe", "swo", "swe", "swi", "sui",
		"oyae", "oye", "owa", "owae", "owo", "owe", "owi", "oui",
		"jyae", "je", "jye", "jwa", "jwae", "joe", "jwo", "jwe", "jwi", "jui",
		"chyae", "che", "chye", "chwa", "chwae", "choe", "chwo", "chwe", "chwi", "chui",
		"kyae", "ke", "kye", "kwa", "kwae", "koe", "kwo", "kwe", "kwi", "kui",
		"tyae", "te", "tye", "twa", "twae", "toe", "two", "twe", "twi", "tui",
		"pyae", "pe", "pye", "pwa", "pwae", "poe", "pwo", "pwe", "pwi", "pui",
		"hyae", "he", "hye", "hwa", "hwae", "hoe", "hwo", "hwe", "hwi", "hui",
	};

	static const char* EndFragments[] = {
		"a", "ya", "eo", "yeo", "o", "yo", "u", "yu", "eu", "i", "ae",
		"g", "n", "d", "r", "m", "b", "s", "o", "j", "ch", "k", "t",
		"p", "h", "ga", "gya", "gyeo", "go", "gyo", "gu", "gyu", "geu", "gi",
		"gae", "na", "nya", "neo", "nyeo", "no", "nyo", "nu", "nyu", "neu",
		"ni", "nae", "da", "dya", "deo", "dyeo", "do", "dyo", "du", "dyu", "deu",
		"di", "dae", "ra", "rya", "reo", "ryeo", "ro", "ro", "ryo", "ru", "ryu",
		"reu", "ri", "rae", "ma", "mya", "meo", "myeo", "mo", "myo", "mu", "myu",
		"meu", "mi", "mae", "ba", "bya", "beo", "byeo", "bo", "byo", "bu", "byu",
		"beu", "bi", "bae", "sa", "sya", "seo", "syeo", "so", "syo", "su", "syu",
		"seu", "si", "sae", "ja", "jya", "jeo", "jyeo", "jo", "jyo", "ju", "jyu",
		"jeu", "ji", "jae", "cha", "chya", "cheo", "chyeo", "cho", "chyo", "chu",
		"chyu", "cheu", "chi", "chae", "ka", "kya", "keo", "kyeo", "ko", "kyo",
		"ku", "kyu", "keu", "ki", "kae", "ta", "tya", "teo", "tyeo", "to", "tyo",
		"tu", "tyu", "teu", "ti", "tae", "pa", "pya", "peo", "pyeo", "po", "pyo",
		"pu", "pyu", "peu", "pi", "pae", "ha", "hya", "heo", "hyeo", "ho", "hyo",
		"hu", "hyu", "heu", "hi", "hae",

		"yae", "e", "ye", "wa", "wae", "oe", "wo", "we", "wi", "ui",
		"gyae", "ge", "gye", "gwa", "gwae", "goe", "gwo", "gwe", "gwi", "gui",
		"nyae", "ne", "nye", "nwa", "nwae", "noe", "nwo", "nwe", "nwi", "nui",
		"dyae", "de", "dye", "dwa", "dwae", "doe", "dwo", "dwe", "dwi", "dui",
		"ryae", "re", "rye", "rwa", "rwae", "roe", "rwo", "rwe", "rwi", "rui",
		"myae", "me", "mye", "mwa", "mwae", "moe", "mwo", "mwe", "mwi", "mui",
		"byae", "be", "bye", "bwa", "bwae", "boe", "bwo", "bwe", "bwi", "bui",
		"syae", "se", "sye", "swa", "swae", "soe", "swo", "swe", "swi", "sui",
		"oyae", "oye", "owa", "owae", "owo", "owe", "owi", "oui",
		"jyae", "je", "jye", "jwa", "jwae", "joe", "jwo", "jwe", "jwi", "jui",
		"chyae", "che", "chye", "chwa", "chwae", "choe", "chwo", "chwe", "chwi", "chui",
		"kyae", "ke", "kye", "kwa", "kwae", "koe", "kwo", "kwe", "kwi", "kui",
		"tyae", "te", "tye", "twa", "twae", "toe", "two", "twe", "twi", "tui",
		"pyae", "pe", "pye", "pwa", "pwae", "poe", "pwo", "pwe", "pwi", "pui",
		"hyae", "he", "hye", "hwa", "hwae", "hoe", "hwo", "hwe", "hwi", "hui",
		"cheon", "san", "seong", "byeol", "-byeol", "bulkkoch", "-bulkkoch",
		"bich", "-bich" "balgwang", "-balgwang", "gwangjeom", "-gwangjeom",
		"balg-eun", "-balg-eun", "-bichnaneun", "bichnaneun", "beondeug-im",
		"-beondeug-im", "-hae", "hae", "-geodaehan", "geodaehan", "peulleeo",
		"-peulleeo", "pogbal", "-pogbal", "bul", "-bul", "yeol", "-yeol", "guche",
		"-guche", "chegye", "-chegye",
	};
	// clang-format on

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments);

	void GetName(std::string& name, Random& rng)
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
} // namespace Korean

namespace Westernees {
	// clang-format off
	static const char* StartFragments1[] = {
		"ar", "aer", "alders", "an", "att", "assen", "ama", "anc", "asp", "ade", "ass",
		"ae", "arc", "arch", "arcen", "aspen", "atten", "aeren", "ande", "ans", "am",
		"avi", "amster", "auer", "aur", "apel" "argen",
		"bro", "blu", "beau", "bru", "bre", "bel", "bla", "bre", "bur", "bal", "bo",
		"ballac", "bally", "berx", "brug", "bou", "blok", "bred", "blau", "biesen",
		"col", "cal", "clo", "cle", "cla", "cin", "cidar", "crey", "clare", "cae", "caer",
		"car", "cara", "crin", "craag", "crei", "cras", "cam", "cour", "culem",
		"dar", "dae", "dill", "dor", "drui", "dud", "dun", "duen", "dy", "darn", "dorn",
		"drak", "drek", "dillen", "dik", "dyk", "dijk", "dal", "doet", "doetin", "dier",
		"duben", "dender",
		"ellan", "eysen", "enh", "ess", "essen", "ever", "exe", "eind", "ev", "eve",
		"fyres", "fa", "for", "fen", "far", "fol", "fil", "fal", "fur", "fir", "fan",
		"frei", "fursten",
		"gri", "gre", "gia", "glys", "gheli", "gul", "gren", "gwen", "glev", "gael", "guden",
		"glevitz", "gle", "gail", "gayl", "guth", "geisel", "gorin", "geras", "grey",
		"ha", "hag", "hout", "hoche", "hal", "hel", "har", "hav", "hor", "had", "hol", "her",
		"heil", "hen", "hutt", "hoog", "heer", "herd", "hard",
		"ill", "illen", "ir", "ira",
		"kac", "ka", "kag", "ke", "ker", "klu", "kin", "klag", "klagen", "kitz",
		"lit", "lil", "ly", "lind", "linden", "lon", "ley", "lan", "lok", "loch", "land", "lau", "laag", "lies", "ley",
		"mel", "mari", "mol", "met", "ma", "may", "mi", "mir", "mon", "mont",
		"mal", "mu", "mur", "mall", "mae", "mar", "muin", "mun", "mullin", "mauv", "maas", "muhl", "maar",
		"neun", "novi", "nol", "nen", "nor", "nas", "naam", "north", "ne", "ner", "nieu",
		"ort", "or", "ol", "old", "oxen", "ol", "oost", "op", "orrin", "oft", "olden",
		"pol", "po", "pon", "pont", "pel", "peltra", "pfullen", "poys", "porren", "pletten", "porth", "poi",
		"rogg", "rin", "riv", "rak", "ried", "ries", "roz", "riss", "rhys", "roc", "raz",
		"runs", "ruhns", "ruhn", "run", "rhun", "rot", "roch", "ron", "ro", "rost", "rom",
		"romain", "raven", "regen",
		"sa", "sar", "sal", "sor", "sle", "syd", "sca", "schwemm", "spa", "sty", "strat", "sou", "sem",
		"sli", "shae", "stock", "schar",
		"te", "teg", "ti", "tyf", "tri", "thu", "tret", "tran", "traver", "torr", "tott", "totten",
		"threl", "tun", "tra", "thur", "tur", "ter", "temple", "tem", "tour",
		"up", "upp", "var", "vvar",
		"vart", "vir", "vid", "vide", "ved", "vede", "vel", "val", "vaal", "vool", "vol", "vollen",
		"vor", "voor", "ven", "venger", "viz", "vatt", "vell", "volk", "vlissing",
		"we", "wes", "west", "when", "winne", "waid", "ware", "waal", "win", "weiben", "werden", "wall", "wol", "wallen",
		"ygg", "zelt", "zwei", "zur", "za", "zav", "zoet"

	};

	static const char* MiddleFragments[]{
		"a", "en", "in", "on", "et", "an", "el", "ie",
		"er", "ar", "un", "ul", "ur", "eu", "ue", "uae",
		"aeu", "aue", "iu", "e", "i", "u", "o", "ae", "of"
	};

	static const char* EndFragments1[] = {
		"an", "ander", "arven", "art", "ayne",
		"bar", "bourne", "bain", "bane", "bul", "bing", "burg", "barran", "baron", "bet",
		"borg", "burgh", "burk", "burn", "barn", "byrne", "buz", "berg", "bergen", "bor",
		"briggan", "born", "bon", "beek", "beck", "berge", "bourg", "bourge", "bach", "birn",
		"chester", "clair", "chor", "canell", "cht", "cano", "catou", "court", "colt", "cort",
		"coln", "cona", "cayne", "casel", "castel", "cassel", "cart",
		"don", "duhn", "durn", "drun", "dare", "dano", "drog", "dyffra", "dal", "deith", "doth",
		"dorth", "dorst", "dort", "dam", "dunn", "dalk", "der", "derry", "den", "dert", "dorf",
		"doorn", "door", "duur", "dorff", "daal", "dust", "dyst", "dist",
		"drisio", "est", "ester", "esten", "eston", "end", "enna", "egen", "ean", "ena",
		"ette", "eveen", "en", "et", "ebuz", "elogne", "erk", "elstadt", "envin", "erstad",
		"ervik", "een", "etten", "ein", "ende", "ergen", "elstel", "eore", "elding",
		"fort", "furt", "forn", "forst", "feld", "forj", "ford", "fork", "folk", "fold",
		"font", "first", "furst", "fyre", "fire", "forde", "forth", "fen", "falk", "felden", "frey", "fry", "free", "fren", "frei",
		"fell", "frost","fal", "fall", "frost",
		"gort", "gold", "gen", "gham", "glean", "gamo", "gorst", "gorth", "gor", "grad", "gran", "geis", "geist", "gein", "gram",
		"glen", "gar", "garth", "gheda", "gard", "guard", "gaard", "garm","geld", "gild", "gelden", "gilden", "geismar", "gueux",
		"haus", "holm", "heim", "helm", "haust", "hold", "haut", "hoos", "haas", "hast", "holst", "halfen", "hafen", "huizen",
		"hort", "hand", "ham", "hagen", "hogen", "hoogen", "haugen", "hoden", "hodan", "haugen", "haugan", "haudon",
		"harden", "hasten", "holdon", "hordon", "hal", "halm", "hai", "hol", "hole", "horst", "horn", "holn",
		"heln", "heist", "hest", "host", "horm", "hein", "hot", "holz", "hurn",
		"iln", "illn", "ist", "illist", "il", "is", "id", "it", "ian", "igg", "irt", "ivel",
		"ingen", "ilien", "illien", "iten", "isten", "irsten", "inne", "in", "inn", "ia",
		"islip", "ing", "ick", "icken", "iken", "irk", "irken", "int",
		"jaque", "jack", "jill", "jon", "jorst", "jort", "jaques", "jost", "jake",
		"kiln", "ken", "kerk", "kerst", "kirk", "kyrk", "korn", "koln", "king", "kryg",  "kkon",
		"kite", "kyt", "kyte", "kert", "kurt", "kurd", "klaas", "klast", "kum", "kirchen",
		"lin", "llin", "lyn", "lyne", "line", "list", "llow", "lost", "lyffe", "lith", "ling",
		"lein", "loopen", "loop", "lion", "lyne", "lyon", "lord", "land", "leen", "lun", "lud", "luud", "lost", "lust",
		"mart", "mar", "mon", "ment", "morn", "morne", "mourne", "mourn", "moll", "mont", "molsen",
		"mint", "ma", "mur", "motier", "masse", "mende", "mere", "mende", "more", "moore", "monde",
		"now", "no", "nau", "non", "nacht", "ney", "nest", "neux", "ningen", "nicht", "nesse", "noit", "noir",
		"oost", "orst", "oort", "orn", "ost", "ourne", "our", "out", "ogne",
		"port", "pont", "post", "pol", "poll", "pile", "pool", "pes", "que",
		"roost", "rest", "raux", "roux", "runn", "rain", "rayne", "raine", "rog", "rune", "rijk",
		"ra", "ren", "ray", "rachi", "ris", "rault", "refin", "rein", "reihn", "rick", "rea", "rade", "risring", "ring",
		"stein", "salvat", "sau", "stadt", "schau", "singen", "sten", "stern", "sturn", "swil", "sant", "stel", "stad", "selt", "spach", "soix",
		"stone", "stuhn", "star", "starr", "staar", "stah", "staa", "strel", "stejrne", "ster",
		"thal", "teaux", "trenk", "trench", "ton", "thold", "told", "ten", "toil", "toile",
		"vain", "vayne", "van", "vahn", "veen", "vest", "vort", "ven", "vail", "veil", "vayl", "von", "vat", "verelin", "ville", "vein", "veen", "veyn",
		"wald", "weld", "wein", "wedd", "wil", "wijk", "wick",
		"zorn", "zurn"

	};

	static const unsigned int NUM_START_FRAGS = COUNTOF(StartFragments1);
	static const unsigned int NUM_MIDDLE_FRAGS = COUNTOF(MiddleFragments);
	static const unsigned int NUM_END_FRAGS = COUNTOF(EndFragments1);

	void GetName(std::string& name, Random& rng)
	{


		Uint32 nameGen = rng.Int32(1000000000);
		if (nameGen < 219576720) {
			// intro
			name += StartFragments1[rng.Int32(0, NUM_START_FRAGS - 1)];

			// beginning
			name += MiddleFragments[rng.Int32(0, NUM_MIDDLE_FRAGS - 1)];

			// end
			name += EndFragments1[rng.Int32(0, NUM_END_FRAGS - 1)];
		}
		else {
			// beginning
			name += StartFragments1[rng.Int32(0, NUM_START_FRAGS - 1)];

			// end
			name += EndFragments1[rng.Int32(0, NUM_END_FRAGS - 1)];
		}

		// Capitalisation
		name[0] = toupper(name[0]);
	}

}
