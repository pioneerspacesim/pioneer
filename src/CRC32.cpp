// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CRC32.h"

const Uint32 CRC32::s_polynomial = 0x04c11db7;

bool CRC32::s_lookupTableGenerated;
Uint32 CRC32::s_lookupTable[256];

static Uint32 crc32_reflect(Uint32 v, const int bits)
{
	Uint32 r = 0;
	for (int i = 1; i <= bits; i++) {
		if (v & 1)
			r = r | (1 << (bits - i));
		v >>= 1;
	}
	return r;
}

CRC32::CRC32() :
	m_checksum(0xffffffff)
{
	if (!s_lookupTableGenerated) {
		for (int i = 0; i <= 0xff; i++) {
			s_lookupTable[i] = crc32_reflect(i, 8) << 24;
			for (int j = 0; j < 8; j++)
				s_lookupTable[i] = (s_lookupTable[i] << 1) ^ (s_lookupTable[i] & (1 << 31) ? s_polynomial : 0);
			s_lookupTable[i] = crc32_reflect(s_lookupTable[i], 32);
		}

		s_lookupTableGenerated = true;
	}
}

void CRC32::AddData(const char *data, int length)
{
	const unsigned char *buf = reinterpret_cast<const unsigned char *>(data);
	while (length--)
		m_checksum = (m_checksum >> 8) ^ s_lookupTable[(m_checksum & 0xff) ^ *buf++];
}
