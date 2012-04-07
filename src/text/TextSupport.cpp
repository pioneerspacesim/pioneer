#include "TextSupport.h"
#include <cassert>

namespace Text {

// returns num bytes consumed, or 0 for end/bogus
int conv_mb_to_wc(Uint32 *chr, const char *src)
{
	unsigned int c = *(reinterpret_cast<const unsigned char*>(src));
	if (!c) { *chr = c; return 0; }
	if (!(c & 0x80)) { *chr = c; return 1; }
	else if (c >= 0xf0) {
		if (!src[1] || !src[2] || !src[3]) return 0;
		c = (c & 0x7) << 18;
		c |= (src[1] & 0x3f) << 12;
		c |= (src[2] & 0x3f) << 6;
		c |= src[3] & 0x3f;
		*chr = c; return 4;
	}
	else if (c >= 0xe0) {
		if (!src[1] || !src[2]) return 0;
		c = (c & 0xf) << 12;
		c |= (src[1] & 0x3f) << 6;
		c |= src[2] & 0x3f;
		*chr = c; return 3;
	}
	else {
		if (!src[1]) return 0;
		c = (c & 0x1f) << 6;
		c |= src[1] & 0x3f;
		*chr = c; return 2;
	}
}

// encode one Unicode code-point as UTF-8
//  chr: the Unicode code-point
//  buf: a character buffer, which must have space for at least 4 bytes
//       (i.e., assigning to buf[3] must be a valid operation)
//  returns: number of bytes in the encoded character
int conv_wc_to_mb(Uint32 chr, char buf[4])
{
	unsigned char *ubuf = reinterpret_cast<unsigned char*>(buf);
	if (chr <= 0x7f) {
		ubuf[0] = chr;
		return 1;
	} else if (chr <= 0x7ff) {
		ubuf[0] = 0xc0 | (chr >> 6);
		ubuf[1] = 0x80 | (chr & 0x3f);
		return 2;
	} else if (chr <= 0xffff) {
		ubuf[0] = 0xe0 | (chr >> 12);
		ubuf[1] = 0x80 | ((chr >> 6) & 0x3f);
		ubuf[2] = 0x80 | (chr & 0x3f);
		return 3;
	} else if (chr <= 0x10fff) {
		ubuf[0] = 0xf0 | (chr >> 18);
		ubuf[1] = 0x80 | ((chr >> 12) & 0x3f);
		ubuf[2] = 0x80 | ((chr >> 6) & 0x3f);
		ubuf[3] = 0x80 | (chr & 0x3f);
		return 4;
	} else {
		assert(0 && "Invalid Unicode code-point.");
		return 0;
	}
}

}
