#include "CRC32.h"

const Uint32 CRC32::s_polynomial = 0x04c11db7;

bool CRC32::s_lookupTableGenerated;
Uint32 CRC32::s_lookupTable[256];

CRC32::CRC32() : m_checksum(0xffffffff)
{
	if (s_lookupTableGenerated) return;

	for (int i = 0; i <= 0xff; i++) { 
		s_lookupTable[i] = Reflect(i,8) << 24; 
		for (int j = 0; j < 8; j++) 
			s_lookupTable[i] = (s_lookupTable[i] << 1) ^ (s_lookupTable[i] & (1 << 31) ? s_polynomial : 0); 
		s_lookupTable[i] = Reflect(s_lookupTable[i], 32); 
	}
}

Uint32 CRC32::Reflect(Uint32 v, const int bits)
{
	Uint32 r = 0;
	for (int i = 1; i <= bits; i++) {
		if (v & 1)
			r = r | (1<<(bits-i));
		v >>= 1;
	}
	return r;
}

void CRC32::AddData(const char *data, int length)
{
	const unsigned char *buf = reinterpret_cast<const unsigned char *>(data);
	while (length--)
		m_checksum = (m_checksum >> 8) ^ s_lookupTable[(m_checksum & 0xff) ^ *buf++];
}

Uint32 CRC32::GetChecksum() const
{
	return m_checksum & 0xffffffff;
}
