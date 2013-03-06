#include "libs.h"

#include "CoreCount.h"

#if WINDOWS_BUILD
#include <windows.h>
#elif MAC_BUILD
#include <sys/param.h>
#include <sys/sysctl.h>
#elif UNIX_BUILD
#include <unistd.h>
#endif

int getNumCores() {
#if WINDOWS_BUILD
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif MAC_BUILD
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#elif UNIX_BUILD
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
	return 1;	// unsupported platform - feel free to add it!
#endif
}
