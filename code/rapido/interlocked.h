
#ifndef INTERLOCKED_H
#define INTERLOCKED_H

#ifdef WIN32

namespace Interlocked
{
	namespace Internal
	{
		extern "C" long __stdcall _InterlockedExchangeAdd(volatile long *addend, long value); // kernel32.lib
		#pragma intrinsic(_InterlockedExchangeAdd)
	}

	inline long ExchangeAdd(volatile long *addend, long value)
	{
		return Internal::_InterlockedExchangeAdd(addend, value);
	}
}

#elif defined(__APPLE__)

#include <libkern/OSAtomic.h>

namespace Interlocked
{
	inline long ExchangeAdd(volatile long *addend, long value)
	{
#if 1
		//sizeof(long) == sizeof(int32_t)
		return OSAtomicAdd32(value, (int32_t *)addend) - value;
#else
		//sizeof(long) == sizeof(int64_t)
		return OSAtomicAdd64(value, (int64_t *)addend) - value;
#endif
	}
}

#else

namespace Interlocked
{
	inline long ExchangeAdd(volatile long *addend, long value)
	{
		return __sync_fetch_and_add(addend, value); // make sure to return the old value, therefore don't use __sync_add_and_fetch
	}
}

#endif

#endif // INTERLOCKED_H
