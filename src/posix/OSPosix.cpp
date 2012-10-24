// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "OS.h"
#include "FileSystem.h"
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

void RedirectStdio()
{
	std::string stdout_file = FileSystem::JoinPath(FileSystem::GetUserDir(), "stdout.txt");
	std::string stderr_file = FileSystem::JoinPath(FileSystem::GetUserDir(), "stderr.txt");

	FILE *f;

	f = freopen(stdout_file.c_str(), "w", stdout);
	if (!f)
		f = fopen(stdout_file.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stdout to '%s': %s\n", stdout_file.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stdout = *f;
	}

	f = freopen(stderr_file.c_str(), "w", stderr);
	if (!f)
		f = fopen(stderr_file.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stderr to '%s': %s\n", stderr_file.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stderr = *f;
	}
}

} // namespace OS
