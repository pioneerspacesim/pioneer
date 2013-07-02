// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "OS.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include <SDL.h>
#include <sys/time.h>
#include <fenv.h>

namespace OS {

void NotifyLoadBegin()
{
}

void NotifyLoadEnd()
{
}

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
	std::string stdout_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "stdout.txt");
	std::string stderr_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "stderr.txt");

	FILE *f;

	f = freopen(stdout_path.c_str(), "w", stdout);
	if (!f)
		f = fopen(stdout_path.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stdout to '%s': %s\n", stdout_path.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stdout = *f;
	}

	f = freopen(stderr_path.c_str(), "w", stderr);
	if (!f)
		f = fopen(stderr_path.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stderr to '%s': %s\n", stderr_path.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stderr = *f;
	}
}

void EnableFPE()
{
#ifndef __APPLE__
#ifdef _GNU_SOURCE
	// clear any outstanding exceptions before enabling, otherwise they'll
	// trip immediately
	feclearexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
}

void DisableFPE()
{
#ifndef __APPLE__
#ifdef _GNU_SOURCE
	fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
}

Uint64 HFTimerFreq()
{
	return 1000000;
}

Uint64 HFTimer()
{
	timeval t;
	gettimeofday(&t, 0);
	return Uint64(t.tv_sec)*1000000 + Uint64(t.tv_usec);
}

} // namespace OS
