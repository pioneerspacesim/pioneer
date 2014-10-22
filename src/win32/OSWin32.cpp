// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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

#include <algorithm> // std::remove

#include "Logger/sha1.h" // SHA-1 hashing algorithm
#include "curl/curl.h" // libcurl, for sending http requests
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <iptypes.h>

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


static std::unique_ptr<Analytics> s_logger;
Analytics* GetLogger( const bool bUseLogging /*= false*/ )
{
#ifdef USE_GAME_ANALYTICS_LOGGING
	if(!s_logger.get()) {
		if( bUseLogging ) {
			// The GameAnalytics class that sends data to our GameAnalytics.com service
			s_logger.reset( new GameAnalytics );
		} else {
			// this is the NULL device that does nothing, it just provides empty methods you can safely call
			s_logger.reset( new Analytics );
		}
	}
#else
	if(!s_logger.get()) {
		// this is the NULL device that does nothing, it just provides empty methods you can safely call
		s_logger.reset( new Analytics );
	}
#endif
	return s_logger.get();
}

// Destroy the analytics logger
void ShutdownLogger()
{
	s_logger.reset();
}

std::string GetGUID(void)
{
#ifdef USE_GAME_ANALYTICS_LOGGING
    // Get the GUID.
    GUID guid;
    CoCreateGuid(&guid);

    // Turn the GUID into an ASCII string.
    RPC_CSTR str;
    UuidToStringA((UUID*)&guid, &str);

    // Get a standard string out of that.
    std::string result = (LPSTR)str;

    // Remove any hyphens.
    result.erase(std::remove(result.begin(), result.end(), '-'), result.end());

    return result;
#else
	return std::string();
#endif
}

std::string GetUniqueUserID(void)
{
#ifdef USE_GAME_ANALYTICS_LOGGING
    std::string result;

    // Prepare an array to get info for up to 16 network adapters.
    // It's a little hacky, but it's way complicated to do otherwise,
    //   and the chances of someone having more than 16 are pretty crazy.
    IP_ADAPTER_INFO info[16];

    // Get info for all the network adapters.
    DWORD dwBufLen = sizeof(info);
    DWORD dwStatus = GetAdaptersInfo(info, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
    {
        // ---------------------- //
        // ------INCOMPLETE------ //
        // ---------------------- //
        /* deal with error */
    }
    else
    {
        PIP_ADAPTER_INFO pAdapterInfo = info;

        // Iterate through the adapter array until we find one.
        // (Will most likely be the first.)
        while (pAdapterInfo && pAdapterInfo == 0)
        {
            pAdapterInfo = pAdapterInfo->Next;
        }

        if (!pAdapterInfo)
        {
            // Can't get the network adapter, using a GUID instead.
            result = GetGUID();
        }
        else
        {
            // Get a hash of the MAC address using SHA1.
            unsigned char hash[20];
            char hexstring[41];
            sha1::calc(pAdapterInfo->Address, pAdapterInfo->AddressLength, hash);
            sha1::toHexString(hash, hexstring);
            result = hexstring;

            // Remove any hyphens.
            result.erase(std::remove(result.begin(), result.end(), '-'), result.end());
        }
    }

    return result;
#else
	return std::string();
#endif
}

} // namespace OS
