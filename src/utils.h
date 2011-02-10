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
		fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
		err = glGetError(); \
	} \
}
#else
#define glError() 
#endif

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */

void Error(const char *format, ...) __attribute((format(printf,1,2)));
void Warning(const char *format, ...) __attribute((format(printf,1,2)));
void SilentWarning(const char *format, ...) __attribute((format(printf,1,2)));

std::string GetPiUserDir(const std::string &subdir = "");
std::string GetPiDataDir();

struct MissingFileException {};

// joinpath("data","models","some.def") = "data/models/some.def"
std::string join_path(const char *firstbit, ...);
std::string string_join(std::vector<std::string> &v, std::string sep);
std::string string_subst(const char *format, const unsigned int num_args, std::string args[]);
std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist);
std::string format_money(int money);
void strip_cr_lf(char *string);

GLuint util_load_tex_rgba(const char *filename);

FILE *fopen_or_die(const char *filename, const char *mode);

static inline std::string stringf(int maxlen, const char *format, ...)
		__attribute((format(printf,2,3)));

static inline std::string stringf(int maxlen, const char *format, ...)
{
	char *buf = (char*)alloca(maxlen);
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buf, maxlen, format, argptr);
	va_end(argptr);
	return std::string(buf);
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

#endif /* _UTILS_H */
