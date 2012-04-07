#ifndef _TEXT_TEXTSUPPORT_H
#define _TEXT_TEXTSUPPORT_H

#include <SDL_stdinc.h>

namespace Text {

// various text-related functions. a proper home needs to be found for them.

// convert one multibyte (utf8) char to a widechar (utf32/ucs4)
//  chr: pointer to output storage
//  src: multibyte string
//  returns: number of bytes swallowed, or 0 if end of string
int conv_mb_to_wc(Uint32 *chr, const char *src);

// encode one Unicode code-point as UTF-8
//  chr: the Unicode code-point
//  buf: a character buffer, which must have space for at least 4 bytes
//       (i.e., assigning to buf[3] must be a valid operation)
//  returns: number of bytes in the encoded character
int conv_wc_to_mb(Uint32 chr, char buf[4]);

}

#endif
