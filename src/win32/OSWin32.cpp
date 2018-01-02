// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Win32Setup.h"

#include "OS.h"
#include "FileSystem.h"
#include "TextUtils.h"
#ifdef WITH_BREAKPAD
#include "breakpad/exception_handler.h"
#endif
#include <SDL.h>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <shellapi.h>


extern "C" {
	// This is the quickest and easiest way to enable using the nVidia GPU on a Windows laptop with a dedicated nVidia GPU and Optimus tech.
	// enable optimus!
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// AMD have one too!!!
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


#ifdef RegisterClass
#undef RegisterClass
#endif

#ifdef WITH_BREAKPAD
using namespace google_breakpad;
ExceptionHandler* exceptionHandler = nullptr;
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

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
		Minimum application address: %p\n  \
		Maximum application address: %p\n  \
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

	struct OSVersion {
		DWORD major;
		DWORD minor;
		const char *name;
	};
	static const struct OSVersion osVersions[] = {
		{ 6, 3, "Windows 8.1"   },
		{ 6, 2, "Windows 8"     },
		{ 6, 1, "Windows 7"     },
		{ 6, 0, "Windows Vista" },
		{ 5, 1, "Windows XP"    },
		{ 5, 0, "Windows 2000"  },
		{ 0, 0, nullptr         }
	};

	OSVERSIONINFOA osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	GetVersionExA(&osv);

	std::string name;
	for (const OSVersion *scan = osVersions; scan->name; scan++) {
		if (osv.dwMajorVersion == scan->major && osv.dwMinorVersion == scan->minor) {
			name = scan->name;
			break;
		}
	}
	if (name.empty())
		return hwInfo + s_NoOSIdentified;

	std::string patchName(osv.szCSDVersion);

	if (patchName.empty())
		return hwInfo + name;

	return hwInfo + name + " (" + patchName + ")";
}

#ifdef WITH_BREAKPAD
/////////////////////////////////////////////////////// Google Breakpad
bool FilterCallback(void* context, EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion)
{
	return true;
}

bool MinidumpCallback(const wchar_t* dump_path,
	const wchar_t* minidump_id,
	void* context,
	EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion,
	bool succeeded)
{
	std::wstring msg = L"Unhandled exception occured.\n";
	msg.append(L"Crash information dump was written to: \n    ");
	msg.append(transcode_utf8_to_utf16(FileSystem::userFiles.GetRoot() + "\\crashdumps"));
	msg.append(L"\nMini-dump id: \n    ");
	msg.append(minidump_id);
	MessageBoxW(NULL,
		msg.c_str(), L"Game crashed!", MB_OK | MB_ICONERROR);

	return succeeded;
}
#endif

void EnableBreakpad()
{
#ifdef WITH_BREAKPAD
	CustomClientInfo cci;
	cci.count = 0;
	cci.entries = nullptr;
	std::wstring dumps_path;
	dumps_path = transcode_utf8_to_utf16(FileSystem::userFiles.GetRoot());
	FileSystem::userFiles.MakeDirectory("crashdumps");
	dumps_path.append(L"\\crashdumps");
	exceptionHandler = new ExceptionHandler(
		dumps_path,													// Dump path
		FilterCallback,												// Filter callback
		MinidumpCallback,											// Minidumps callback
		nullptr,													// Callback context
		ExceptionHandler::HandlerType::HANDLER_ALL,					// Handler types
		MINIDUMP_TYPE::MiniDumpWithDataSegs,
		L"",														// Minidump server pipe name
		&cci);														// Custom client information
#endif
}

// Open the Explorer/Finder/etc
bool SupportsFolderBrowser()
{
	return true;
}

void OpenUserFolderBrowser()
{
	std::wstring dumps_path;
	dumps_path = transcode_utf8_to_utf16(FileSystem::userFiles.GetRoot());
	ShellExecuteW(NULL, L"open", dumps_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

} // namespace OS
