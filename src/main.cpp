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
