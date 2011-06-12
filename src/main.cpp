#include "libs.h"
#include "Pi.h"
#include <signal.h>

#include "profiler/Profiler.h"

void sigsegv_handler(int signum)
{
	if (signum == SIGSEGV) {
		printf("Segfault! All is lost! Abandon ship!\n");
		SDL_Quit();
		abort();
	}
}

int main(int argc, char** argv)
{
//	signal(SIGSEGV, sigsegv_handler);
	Profiler::detect( argc, (const char**)argv );
	Pi::Init();
	for (;;) Pi::Start();
	return 0;
}
