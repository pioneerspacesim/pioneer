// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Random.h"
#include <string>

namespace NameGenerator {
	void GetSystemName(std::string &name, Random &rng);
}

namespace FrontierNames {
	void GetName(std::string &name, Random &rng);
}

namespace HybridNames {
	void GetName(std::string &name, Random &rng);
}

namespace Doomdark {
	void GetName(std::string &name, Random &rng);
}

namespace Katakana {
	void GetName(std::string &name, Random &rng);
}
