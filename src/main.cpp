#include "libs.h"
#include "Pi.h"
#include <signal.h>
#include "lua_model_compiler.h"

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
	printf("Pioneer ultra high tech tech demo dude!\n");
	LuaModelCompilerInit();
	exit(0);
//	signal(SIGSEGV, sigsegv_handler);
	IniConfig cfg("config.ini");
	Pi::Init(cfg);
	for (;;) Pi::Start();
	return 0;
}
