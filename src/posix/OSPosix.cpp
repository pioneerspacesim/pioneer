#include "OS.h"
#include "SDLWrappers.h"
#include <SDL.h>

namespace OS {

void Error(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "Error: %s\n", buf);
	abort();
}

void Warning(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "Warning: %s\n", buf);
}

void LoadWindowIcon()
{
	SDLSurfacePtr surface = LoadSurfaceFromFile("icons/badge.png");
	if (surface) {
		SDL_WM_SetIcon(surface.Get(), 0);
	}
}

} // namespace OS
