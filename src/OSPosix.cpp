#include "OS.h"
#include "SDLWrappers.h"
#include <SDL.h>

namespace OS {

void LoadWindowIcon()
{
	// SDL doc says "Win32 icons must be 32x32".
	SDLSurfacePtr surface = LoadSurfaceFromFile("icons/badge.png");
	if (surface) {
		SDL_WM_SetIcon(surface.Get(), 0);
	}
}

} // namespace OS
