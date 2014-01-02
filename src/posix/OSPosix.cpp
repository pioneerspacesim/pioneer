// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "OS.h"
#include "FileSystem.h"
#include <SDL.h>
#include <sys/time.h>
#include <fenv.h>
#if defined(__APPLE__)
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

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

const char *GetIconFilename()
{
	return "icons/badge.png";
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
#if defined(_GNU_SOURCE) && !defined(__APPLE__)
	// clear any outstanding exceptions before enabling, otherwise they'll
	// trip immediately
	feclearexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
}

void DisableFPE()
{
#if defined(_GNU_SOURCE) && !defined(__APPLE__)
	fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
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

int GetNumCores()
{
#if defined(__APPLE__)
	int nm[2];
	size_t len = 4;
	u_int count;

	nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
	sysctl(nm, 2, &count, &len, NULL, 0);

	if (count < 1) {
		nm[1] = HW_NCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);
		if(count < 1) { count = 1; }
	}
	return count;
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

} // namespace OS
