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
#include <sys/utsname.h>

namespace OS {

	namespace {
		static const std::string s_NoOSIdentified("No OS Identified\n");
	}

void NotifyLoadBegin()
{
}

void NotifyLoadEnd()
{
}

const char *GetIconFilename()
{
	return "icons/badge.png";
}

void RedirectStdio()
{
	std::string output_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "output.txt");

	FILE *f;

	f = freopen(output_path.c_str(), "w", stderr);
	if (!f)
		f = fopen(output_path.c_str(), "w");
	if (!f)
		Output("ERROR: Couldn't redirect output to '%s': %s\n", output_path.c_str(), strerror(errno));
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

const std::string GetOSInfoString()
{
	int z;
	struct utsname uts;
	z = uname(&uts);

	if ( z == -1 ) {
		return s_NoOSIdentified;
	}

	char infoString[2048];
#if defined(__APPLE__)
	snprintf(infoString, 2048, "System Name: %s\nHost Name: %s\nRelease(Kernel) Version: %s\nKernel Build Timestamp: %s\nMachine Arch: %s\n",
		uts.sysname, uts.nodename, uts.release, uts.version, uts.machine);
#else
	snprintf(infoString, 2048, "System Name: %s\nHost Name: %s\nRelease(Kernel) Version: %s\nKernel Build Timestamp: %s\nMachine Arch: %s\nDomain Name: %s\n",
		uts.sysname, uts.nodename, uts.release, uts.version, uts.machine, uts.domainname);
#endif

	return std::string(infoString);
}

} // namespace OS
