// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Random.h"
#include "Random64.h"
#include <string>

namespace NameGenerator {
	void GetSystemName(std::string& name, Random& rng, Random64 &rng64);
}

namespace FrontierNames {
	void GetName(std::string& name, Random& rng);
}

namespace HybridNames {
	void GetName(std::string& name, Random& rng);
}

namespace Doomdark {
	void GetName(std::string& name, Random& rng);
}

namespace Katakana {
	void GetName(std::string& name, Random& rng);
}

namespace FrenchPhonetics {
	void GetName(std::string& name, Random& rng);
}


namespace PersianPhonetics {
	void GetName(std::string& name, Random& rng);
}

namespace AborigineePhonetics {
	void GetName(std::string& name, Random& rng);
}

namespace MaoriPhonetics {
	void GetName(std::string& name, Random& rng);
}

namespace Spanish {
	void GetName(std::string& name, Random& rng);
}

namespace Chinese {
	void GetName(std::string& name, Random& rng);
}

namespace Xhosa {
	void GetName(std::string& name, Random& rng);
}

namespace Quechua {
	void GetName(std::string& name, Random& rng);
}

namespace Kurmanji {
	void GetName(std::string& name, Random& rng);
}

namespace Korean {
	void GetName(std::string& name, Random& rng);
}

namespace Westernees {
	void GetName(std::string& name, Random& rng);
}
