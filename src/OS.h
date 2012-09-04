#ifndef _OS_H
#define _OS_H
/*
 * Operating system specific functionality, such as
 * raising a message dialog
 */
#include "libs.h"
#include "utils.h"

namespace OS {

	void Error(const char *format, ...)  __attribute((format(printf,1,2))) __attribute((noreturn));
	void Warning(const char *format, ...)  __attribute((format(printf,1,2)));
	void LoadWindowIcon();

}

#endif
