// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Win32Setup.h"

#include "OS.h"
#include "FileSystem.h"
#include "TextUtils.h"
#include <SDL.h>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <VersionHelpers.h>

namespace OS {

	namespace {
		static const std::string s_NoOSIdentified("No OS Identified\n");
	};

// Notify Windows that the window may become unresponsive
void NotifyLoadBegin()
{
	// XXX MinGW doesn't know this function
#ifndef __MINGW32__
	// XXX Remove the following call when loading is moved to a background thread
	DisableProcessWindowsGhosting(); // Prevent Windows from whiting out the screen for "not responding"
#endif
}

// Since there's no way to re-enable Window ghosting, do nothing
void NotifyLoadEnd()
{
}

const char *GetIconFilename()
{
	// SDL doc says "Win32 icons must be 32x32".
	return "icons/badge32-8b.png";
}

void RedirectStdio()
{
	std::string output_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "output.txt");
	std::wstring woutput_path = transcode_utf8_to_utf16(output_path);

	FILE *f;

	f = _wfreopen(woutput_path.c_str(), L"w", stderr);
	if (!f) {
		Output("ERROR: Couldn't redirect output to '%s': %s\n", output_path.c_str(), strerror(errno));
	} else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
	}
}

void EnableFPE()
{
	// clear any outstanding exceptions before enabling, otherwise they'll
	// trip immediately
#ifdef _MCW_EM
	_clearfp();
	_controlfp(_EM_INEXACT | _EM_UNDERFLOW, _MCW_EM);
#endif
}

void DisableFPE()
{
#ifdef _MCW_EM
	_controlfp(_MCW_EM, _MCW_EM);
#endif
}

Uint64 HFTimerFreq()
{
	LARGE_INTEGER i;
	QueryPerformanceFrequency(&i);
	return i.QuadPart;
}

Uint64 HFTimer()
{
	LARGE_INTEGER i;
	QueryPerformanceCounter(&i);
	return i.QuadPart;
}

int GetNumCores()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

// get hardware information
const std::string GetHardwareInfo()
{
	SYSTEM_INFO siSysInfo;
 
	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&siSysInfo); 

	// Display the contents of the SYSTEM_INFO structure. 
	char infoString[2048];
	snprintf(infoString, 2048, 
		"Hardware information: \n  \
		OEM ID: %u\n  \
		Number of processors: %u\n  \
		Page size: %u\n  \
		Processor type: %u\n  \
		Minimum application address: %lx\n  \
		Maximum application address: %lx\n  \
		Active processor mask: %u\n\n", 
		siSysInfo.dwOemId, 
		siSysInfo.dwNumberOfProcessors, 
		siSysInfo.dwPageSize, 
		siSysInfo.dwProcessorType, 
		siSysInfo.lpMinimumApplicationAddress, 
		siSysInfo.lpMaximumApplicationAddress, 
		siSysInfo.dwActiveProcessorMask); 

	return std::string(infoString);
}

const std::string GetOSInfoString()
{
	const std::string hwInfo = GetHardwareInfo();
	if( IsWindows8Point1OrGreater() )
		return hwInfo + std::string("Windows 8.1 or Newer\n");
	else if( IsWindows8OrGreater() )
		return hwInfo + std::string("Windows 8\n");
	else if( IsWindows7SP1OrGreater() )
		return hwInfo + std::string("Windows 7 SP1\n");
	else if( IsWindows7OrGreater() )
		return hwInfo + std::string("Windows 7\n");
	else if( IsWindowsVistaSP2OrGreater() )
		return hwInfo + std::string("Windows Vista SP2\n");
	else if( IsWindowsVistaSP1OrGreater() )
		return hwInfo + std::string("Windows Vista SP1\n");
	else if( IsWindowsVistaOrGreater() )
		return hwInfo + std::string("Windows Vista\n");
	else if( IsWindowsXPSP3OrGreater() )
		return hwInfo + std::string("Windows XP SP3\n");
	else if( IsWindowsXPSP2OrGreater() )
		return hwInfo + std::string("Windows XP SP2\n");
	else if( IsWindowsXPSP1OrGreater() )
		return hwInfo + std::string("Windows XP SP1\n");
	else if( IsWindowsXPOrGreater() )
		return hwInfo + std::string("Windows XP\n");

	return hwInfo + s_NoOSIdentified;
}

} // namespace OS
