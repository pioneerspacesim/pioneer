#include "../libs.h"
#include "Mods.h"

/* yes, truly dynamic my friend */

namespace Mods {

extern void InitModPirates();

void Init()
{
	InitModPirates();

	printf("Modules initialised!\n");
}

}
