// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Win32Setup.h"

#include "OS.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "TextUtils.h"
#include <SDL.h>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>

namespace OS {

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

void RedirectStdio()
{
	std::string stdout_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "stdout.txt");
	std::string stderr_path = FileSystem::JoinPath(FileSystem::GetUserDir(), "stderr.txt");
	std::wstring wstdout_path = transcode_utf8_to_utf16(stdout_path);
	std::wstring wstderr_path = transcode_utf8_to_utf16(stderr_path);

	FILE *f;

	f = _wfreopen(wstdout_path.c_str(), L"w", stdout);
	if (!f) {
		fprintf(stderr, "ERROR: Couldn't redirect stdout to '%s': %s\n", stdout_path.c_str(), strerror(errno));
	} else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
	}

	f = _wfreopen(wstderr_path.c_str(), L"w", stderr);
	if (!f) {
		fprintf(stderr, "ERROR: Couldn't redirect stderr to '%s': %s\n", stderr_path.c_str(), strerror(errno));
	} else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
	}
}

void EnableFPE()
{
	// clear any outstanding exceptions before enabling, otherwise they'll
	// trip immediately
	_clearfp();
	_controlfp(_EM_INEXACT | _EM_UNDERFLOW, _MCW_EM);
}

void DisableFPE()
{
	_controlfp(_MCW_EM, _MCW_EM);
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

} // namespace OS
