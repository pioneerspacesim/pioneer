#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <stdio.h>

std::string string_join(std::vector<std::string> &v, std::string sep);
std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist);

FILE *fopen_or_die(const char *filename, const char *mode);

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */
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
std::string make_random_ship_registration();

#endif /* _UTILS_H */
