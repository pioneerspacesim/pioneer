#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <stdio.h>

std::string string_join(std::vector<std::string> &v, std::string sep);
std::string format_date(double time);
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


#endif /* _UTILS_H */
