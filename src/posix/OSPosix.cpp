// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSystem.h"
#include "buildopts.h"
#include "core/OS.h"
#include "utils.h"

#include <SDL.h>
#include <fenv.h>
#include <sys/time.h>
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

	void EnableFPE()
	{
#if HAS_FPE_OPS
		// clear any outstanding exceptions before enabling, otherwise they'll
		// trip immediately
		feclearexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
		feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
	}

	void DisableFPE()
	{
#if HAS_FPE_OPS
		fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
	}

	uint32_t GetNumCores()
	{
#if defined(__APPLE__)
		int nm[2];
		size_t len = 4;
		u_int count;

		nm[0] = CTL_HW;
		nm[1] = HW_AVAILCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);

		if (count < 1) {
			nm[1] = HW_NCPU;
			sysctl(nm, 2, &count, &len, NULL, 0);
			if (count < 1) {
				count = 1;
			}
		}
		return count;
#else
		// sysconf can return -1 if _SC_NPROCESSORS_ONLN is not supported
		int count = sysconf(_SC_NPROCESSORS_ONLN);
		// There's definitely at least one core.
		// (What are you running Pioneer on otherwise, a potato battery?)
		return count > 0 ? uint32_t(count) : 1;
#endif
	}

	const std::string GetOSInfoString()
	{
		int z;
		struct utsname uts;
		z = uname(&uts);

		if (z == -1) {
			return s_NoOSIdentified;
		}

		char infoString[2048];
#if !defined(_GNU_SOURCE)
		snprintf(infoString, 2048, "System Name: %s\nHost Name: %s\nRelease(Kernel) Version: %s\nKernel Build Timestamp: %s\nMachine Arch: %s\n",
			uts.sysname, uts.nodename, uts.release, uts.version, uts.machine);
#else
		snprintf(infoString, 2048, "System Name: %s\nHost Name: %s\nRelease(Kernel) Version: %s\nKernel Build Timestamp: %s\nMachine Arch: %s\nDomain Name: %s\n",
			uts.sysname, uts.nodename, uts.release, uts.version, uts.machine, uts.domainname);
#endif

		return std::string(infoString);
	}

	const std::string GetUserLangCode()
	{
		// assume $LANG is in the format en_US.utf8
		const char *env_lang = getenv("LANG");
		return std::string(env_lang ? env_lang : "en").substr(0, 2);
	}

	void EnableBreakpad()
	{
		// Support for Mac and Linux should be added
	}

	// Open the Explorer/Finder/etc
	bool SupportsFolderBrowser()
	{
		return false;
	}

	void OpenUserFolderBrowser()
	{
		// Support for Mac and Linux should be added
		// Display the path instead for now
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Pioneer", FileSystem::userFiles.GetRoot().c_str(), 0);
	}

	void SetDPIAware()
	{
	}

} // namespace OS
