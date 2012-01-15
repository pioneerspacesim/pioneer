#include "libs.h"
#include "mtrand.h"
#include "StringF.h"
#include "Pi.h"
#include "LuaNameGen.h"

namespace NameGenerator {
	/* in the future the western way is the only way... :-/ */
	std::vector<std::string> forenames_male;
	std::vector<std::string> forenames_female;
	std::vector<std::string> surnames;
	std::vector<std::string> planet_names;

	void Init()
	{
		FILE *f = fopen_or_die(PIONEER_DATA_DIR"/forenames_male.txt", "r");
		assert(f);
		char buf[256];
		while (fgets(buf, sizeof(buf), f)) {
			char *nl = strchr(buf, '\r');
			if (nl) *nl = 0;
			else {
				nl = strchr(buf, '\n');
				if (nl) *nl = 0;
			}
			forenames_male.push_back(std::string(buf));
		}
		fclose(f);
		f = fopen_or_die(PIONEER_DATA_DIR"/forenames_female.txt", "r");
		while (fgets(buf, sizeof(buf), f)) {
			char *nl = strchr(buf, '\r');
			if (nl) *nl = 0;
			else {
				nl = strchr(buf, '\n');
				if (nl) *nl = 0;
			}
			forenames_female.push_back(std::string(buf));
		}
		fclose(f);
		f = fopen_or_die(PIONEER_DATA_DIR"/surnames.txt", "r");
		while (fgets(buf, sizeof(buf), f)) {
			char *nl = strchr(buf, '\r');
			if (nl) *nl = 0;
			else {
				nl = strchr(buf, '\n');
				if (nl) *nl = 0;
			}
			surnames.push_back(std::string(buf));
		}
		fclose(f);
		f = fopen_or_die(PIONEER_DATA_DIR"/planet_names.txt", "r");
		while (fgets(buf, sizeof(buf), f)) {
			char *nl = strchr(buf, '\r');
			if (nl) *nl = 0;
			else {
				nl = strchr(buf, '\n');
				if (nl) *nl = 0;
			}
			planet_names.push_back(std::string(buf));
		}
		fclose(f);
	}

	std::string PlanetName(MTRand &rng)
	{
		const char *format = planet_names[rng.Int32(planet_names.size())].c_str();
		return stringf(format, Pi::luaNameGen->Surname(rng).c_str());
	}
}

