#ifdef _WIN32

#include "OS.h"
#include "SDLWrappers.h"
#include "TextUtils.h"
#include <SDL.h>

namespace OS {

// Call MessageBox with error icon and abort
void Error(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "Error: %s\n", buf);
	MessageBoxW(0, transcode_utf8_to_utf16(buf, strlen(buf)).c_str(), L"Error", MB_ICONERROR|MB_OK);
#ifndef NDEBUG
	abort();
#else
	exit(-1);
#endif
}

// Call MessageBox with warning icon
void Warning(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "Warning: %s\n", buf);
	MessageBoxW(0, transcode_utf8_to_utf16(buf, strlen(buf)).c_str(), L"Warning", MB_ICONWARNING|MB_OK);
}

void LoadWindowIcon()
{
	// SDL doc says "Win32 icons must be 32x32".
	SDLSurfacePtr surface = LoadSurfaceFromFile("icons/badge32-8b.png");
	if (surface) {
		SDL_WM_SetIcon(surface.Get(), 0);
	}
}

} // namespace OS

#endif
