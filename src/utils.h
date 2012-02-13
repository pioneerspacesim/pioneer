#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <GL/glew.h>
#include "libs.h"

#ifdef DEBUG
#define glError() { \
	GLenum err = glGetError(); \
	while (err != GL_NO_ERROR) { \
		fprintf(stderr, "glError: %s caught at %s:%u\n", reinterpret_cast<const char *>(gluErrorString(err)), __FILE__, __LINE__); \
		err = glGetError(); \
	} \
}
#else
#define glError() 
#endif

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */

// GCC warns when a function marked __attribute((noreturn)) actually returns a value
// but other compilers which don't see the noreturn attribute of course require that
// a function with a non-void return type should return something.
#ifndef __GNUC__
#define RETURN_ZERO_NONGNU_ONLY return 0;
#else
#define RETURN_ZERO_NONGNU_ONLY
#endif

void Error(const char *format, ...) __attribute((format(printf,1,2))) __attribute((noreturn));
void Warning(const char *format, ...) __attribute((format(printf,1,2)));
void SilentWarning(const char *format, ...) __attribute((format(printf,1,2)));

std::string GetPiUserDir(const std::string &subdir = "");
std::string GetPiDataDir();

inline std::string GetPiSavefileDir() { return GetPiUserDir("savefiles"); }

void GetDirectoryContents(const std::string &path, std::list<std::string> &files);

// joinpath("data","models","some.def") = "data/models/some.def"
std::string join_path(const char *firstbit, ...);
std::string string_join(std::vector<std::string> &v, std::string sep);
std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist);
std::string format_money(Sint64 money);

FILE *fopen_or_die(const char *filename, const char *mode);
size_t fread_or_die(void* ptr, size_t size, size_t nmemb, FILE* stream, bool allow_truncated = false);

static inline Sint64 isqrt(Sint64 a)
{
	Sint64 ret=0;
	Sint64 s;
	Sint64 ret_sq=-a-1;
	for(s=62; s>=0; s-=2){
		Sint64 b;
		ret+= ret;
		b=ret_sq + ((2*ret+1)<<s);
		if(b<0){
			ret_sq=b;
			ret++;
		}
	}
	return ret;
}

bool is_file(const std::string &filename);
bool is_dir(const std::string &filename);
/** args to callback are basename, full path */
void foreach_file_in(const std::string &directory, void (*callback)(const std::string &, const std::string &));

void Screendump(const char* destFile, const int w, const int h);

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

// find string in bigger string, ignoring case
const char *pi_strcasestr(const char *haystack, const char *needle);

// add a few things that MSVC is missing
#ifdef _MSC_VER

// round & roundf. taken from http://cgit.freedesktop.org/mesa/mesa/tree/src/gallium/auxiliary/util/u_math.h
static double round(double x)
{
   return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static inline float roundf(float x)
{
   return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}
#endif /* _MSC_VER */

void hexdump(const unsigned char *buf, int bufsz);

#endif /* _UTILS_H */
