// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreCount.h"

// XXX all of this belongs in Os.h

// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine

#if defined(WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/param.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#else
#error "unsupported platform"
#endif

int getNumCores() {
#if defined(WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    int nm[2];
    size_t len = 4;
    u_int count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#elif defined(__linux__)
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
