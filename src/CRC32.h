#ifndef _CRC32_H
#define _CRC32_H

#include <SDL_stdinc.h>
#include <vector>

class CRC32 {
public:
	CRC32();

	void AddData(const char *data, int length);
	Uint32 GetChecksum() const;

private:
	Uint32 m_checksum;

	Uint32 Reflect(Uint32 value, const int bits);

	static const Uint32 s_polynomial;

	static bool s_lookupTableGenerated;
	static Uint32 s_lookupTable[256];
};

#endif
