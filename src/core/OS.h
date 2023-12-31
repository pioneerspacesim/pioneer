// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OS_H
#define _OS_H
/*
 * Operating system specific functionality, such as
 * raising a message dialog
 */

#include <string>

namespace OS {

	void NotifyLoadBegin();
	void NotifyLoadEnd();

	const char *GetIconFilename();

	// Enable and disable floating point exceptions. Disabled is usually default.
	void EnableFPE();
	void DisableFPE();

	// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
	uint32_t GetNumCores();

	// return a string describing the operating system that the game is running on, useful!
	const std::string GetOSInfoString();

	// return a two-character language code detected from the OS environment
	// If the OS environment language cannot be detected, returns "en"
	const std::string GetUserLangCode();

	// Enable Google breakpad for crash minidumps
	void EnableBreakpad();

	// Open the Explorer/Finder/etc
	bool SupportsFolderBrowser();
	void OpenUserFolderBrowser();

	// Mark application as DPI-aware
	void SetDPIAware();
} // namespace OS

#endif
