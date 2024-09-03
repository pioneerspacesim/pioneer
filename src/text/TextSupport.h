// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_TEXTSUPPORT_H
#define _TEXT_TEXTSUPPORT_H

#include <SDL_stdinc.h>
#include <cassert>

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

	// this can tell you the length of a UTF-8 character, or more generally
	// it tells you the number of bytes to move forward to get to the beginning
	// of the next character
	inline int utf8_next_char_offset(const char *str)
	{
		assert(str);
		assert(*str);
		const char *const start = str;
		if (*str & 0x80) {
			// technically, the first byte of a UTF-8 multi-byte sequence is enough
			// to determine the length of the sequence, but a loop is simpler and
			// more robust to incorrectly encoded text
			do {
				++str;
			} while ((*str & 0xC0) == 0x80);
			return (str - start);
		} else
			return 1;
	}

	// this tells you the number of bytes to move backwards to get to the
	// beginning of the previous character (or the current character if you start inside a multi-byte sequence)
	// ('begin' indicates the start of the array and is used to avoid walking off the front)
	inline int utf8_prev_char_offset(const char *str, const char *const begin)
	{
		assert(str);
		assert(str > begin);
		const char *const start = str;
		--str;
		if (*str & 0x80) {
			while ((str > begin) && ((*str & 0xC0) == 0x80)) {
				--str;
			}
			return (start - str);
		} else
			return 1;
	}

	// returns true if the char c is an ASCII letter, a digit
	// or an underscore.
	inline bool is_alphanumunderscore(char c)
	{
		return (c == '_' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
	}

} // namespace Text

#endif
