#include "libs.h"
#include "Pi.h"
//#include <signal.h>
#include "NewModelViewer.h"

/*void sigsegv_handler(int signum)
{
	if (signum == SIGSEGV) {
		printf("Segfault! All is lost! Abandon ship!\n");
		SDL_Quit();
		abort();
	}
}*/

int main(int argc, char** argv)
{
//	signal(SIGSEGV, sigsegv_handler);
	if (argc <= 1) {
		Pi::Init();
		for (;;) Pi::Start();
	} else {
		ModelViewer::Run(argc, argv);
	}
	return 0;
}
