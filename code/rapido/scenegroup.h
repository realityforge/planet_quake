
#ifndef SCENEGROUP_H
#define SCENEGROUP_H

#include "ray.h"
#include "packet.h"
#include <assert.h>

namespace rapido
{
	template<class S, class T>
	struct SceneGroup
	{
		S *first;
		T *second;

		static const bool NeedsCommonDirSigns = S::NeedsCommonDirSigns | T::NeedsCommonDirSigns;

		SceneGroup() : first(0), second(0) { }
		SceneGroup(S *first, T *second) : first(first), second(second) { }

		inline void Trace(ray &r, int flags)
		{
			assert(first != 0 && second != 0);
			first->Trace(r, flags);
			second->Trace(r, flags);
		}

		inline void Trace(ray rays[], int count, int flags)
		{
			assert(first != 0 && second != 0);
			first->Trace(rays, count, flags);
			second->Trace(rays, count, flags);
		}

		inline void Trace(packet &p, int flags)
		{
			assert(first != 0 && second != 0);
			first->Trace(p, flags);
			second->Trace(p, flags);
		}

		inline void Trace(packet packets[], int count, int flags)
		{
			assert(first != 0 && second != 0);
			first->Trace(packets, count, flags);
			second->Trace(packets, count, flags);
		}
	};
}

#endif // SCENEGROUP_H
