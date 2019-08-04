
#ifndef THREADINGHELPER_H
#define THREADINGHELPER_H

#if defined(WIN32)
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#import <sys/param.h>
#import <sys/sysctl.h>
#else
#import <unistd.h>
#import <sys/utsname.h>
#endif

namespace rapido
{
	inline int GetNumberOfProcessors()
	{
#if defined(WIN32)
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);
		return siSysInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
		int count; size_t size = sizeof(count);
		return sysctlbyname("hw.ncpu", &count, &size, 0, 0) ? 1 : count;  
#else
		return sysconf(_SC_NPROCESSORS_ONLN);
#endif
	}
}

#endif // THREADINGHELPER_H

