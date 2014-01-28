// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

	const char *GetIconFilename();

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
