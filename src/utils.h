#ifndef _UTILS_H
#define _UTILS_H

#include <string>

std::string date_format(double time);

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
