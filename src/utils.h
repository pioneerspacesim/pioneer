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

void Error(const char *format, ...) __attribute((format(printf,1,2))) __attribute((noreturn));
void Warning(const char *format, ...) __attribute((format(printf,1,2)));
void SilentWarning(const char *format, ...) __attribute((format(printf,1,2)));

std::string GetPiUserDir(const std::string &subdir = "");
std::string GetPiDataDir();

// joinpath("data","models","some.def") = "data/models/some.def"
std::string join_path(const char *firstbit, ...);
std::string string_join(std::vector<std::string> &v, std::string sep);
std::string string_subst(const char *format, const unsigned int num_args, std::string args[]);
std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist);
std::string format_money(Sint64 money);
void strip_cr_lf(char *string);

GLuint util_load_tex_rgba(const char *filename);

FILE *fopen_or_die(const char *filename, const char *mode);
size_t fread_or_die(void* ptr, size_t size, size_t nmemb, FILE* stream, bool allow_truncated = false);

static inline std::string stringf(int maxlen, const char *format, ...)
		__attribute((format(printf,2,3)));

static inline std::string stringf(int maxlen, const char *format, ...)
{
	char *buf = reinterpret_cast<char*>(alloca(maxlen));
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buf, maxlen, format, argptr);
	va_end(argptr);
	return std::string(buf);
}

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

struct Plane {
	double a, b, c, d;
	double DistanceToPoint(const vector3d &p) {
		return a*p.x + b*p.y + c*p.z + d;
	}
};

/* from current GL modelview*projection matrix */
void GetFrustum(Plane planes[6]);

bool is_file(const std::string &filename);
bool is_dir(const std::string &filename);
/** args to callback are basename, full path */
void foreach_file_in(const std::string &directory, void (*callback)(const std::string &, const std::string &));

Uint32 ceil_pow2(Uint32 v);

void Screendump(const char* destFile, const int w, const int h);

// convert one multibyte (utf8) char to a widechar (utf32/ucs4)
//  chr: pointer to output storage
//  src: multibyte string
//  returns: number of bytes swallowed, or 0 if end of string
int conv_mb_to_wc(Uint32 *chr, const char *src);

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

#endif /* _UTILS_H */
