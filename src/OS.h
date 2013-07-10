// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OS_H
#define _OS_H
/*
 * Operating system specific functionality, such as
 * raising a message dialog
 */
#include "libs.h"
#include "utils.h"

namespace OS {

	void NotifyLoadBegin();
	void NotifyLoadEnd();

	void Error(const char *format, ...)  __attribute((format(printf,1,2))) __attribute((noreturn));
	void Warning(const char *format, ...)  __attribute((format(printf,1,2)));
	void LoadWindowIcon();

	void RedirectStdio();

	// Enable and disable floating point exceptions. Disabled is usually default.
	void EnableFPE();
	void DisableFPE();

	// High frequency timer. HFTimer() returns count, HFTimerFreq() returns frequency.
	// should not be considered reliable
	Uint64 HFTimerFreq();
	Uint64 HFTimer();

	// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
	int GetNumCores();
}

#endif
