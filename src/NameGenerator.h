#ifndef _NAMEGENERATOR_H
#define _NAMEGENERATOR_H
#include <string>
#include "mtrand.h"

namespace NameGenerator {
	void Init();
	std::string FullName(MTRand &rng, bool genderFemale);
	std::string Surname(MTRand &rng);
}

#endif /* _NAMEGENERATOR_H */
