// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CRC32_H
#define _CRC32_H

#include <SDL_stdinc.h>

class CRC32 {
public:
	CRC32();

	void AddData(const char *data, int length);
	Uint32 GetChecksum() const { return m_checksum; }

private:
	Uint32 m_checksum;

	static const Uint32 s_polynomial;
	static bool s_lookupTableGenerated;
	static Uint32 s_lookupTable[256];
};

#endif
