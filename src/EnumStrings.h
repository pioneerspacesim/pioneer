// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ENUMSTRINGS_H
#define ENUMSTRINGS_H

namespace EnumStrings {

	void Init();
	const char *GetString(const char *ns, int value);
	int GetValue(const char *ns, const char *name);

} // namespace EnumStrings

#endif
