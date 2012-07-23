#ifndef _TEXT_TEXTSUPPORT_H
#define _TEXT_TEXTSUPPORT_H

#include <SDL_stdinc.h>

namespace Text {

// various text-related functions. a proper home needs to be found for them.

// convert one multibyte (utf8) char to a widechar (utf32/ucs4)
//  chr: pointer to output storage
//  src: multibyte string
//  returns: number of bytes swallowed, or 0 if end of string
int utf8_decode_char(Uint32 *chr, const char *src);

// encode one Unicode code-point as UTF-8
//  chr: the Unicode code-point
//  buf: a character buffer, which must have space for at least 4 bytes
//       (i.e., assigning to buf[3] must be a valid operation)
//  returns: number of bytes in the encoded character
int utf8_encode_char(Uint32 chr, char buf[4]);

// returns true if the char c is an ASCII letter, a digit
// or an underscore.
inline bool is_alphanumunderscore(char c) {
	return (c == '_' || (c >= '0' && c <= '9') || (c  >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

}

#endif
