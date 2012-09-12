// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#include "libs.h"
#include "Pi.h"
#include <signal.h>

void sigsegv_handler(int signum)
{
	if (signum == SIGSEGV) {
		printf("Segfault! All is lost! Abandon ship!\n");
		SDL_Quit();
		abort();
	}
}

int main(int argc, char**)
{
//	signal(SIGSEGV, sigsegv_handler);
	Pi::Init();
	for (;;) Pi::Start();
	return 0;
}
