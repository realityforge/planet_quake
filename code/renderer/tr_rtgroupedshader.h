
#ifndef __TR_RTGROUPEDSHADER_H__
#define __TR_RTGROUPEDSHADER_H__

#include <rapido.h>
#include <algorithm>

namespace GroupedShader
{
	typedef void ShadingFunc(rapido::ray **rays, int count, int depth);

	inline static void Shade(rapido::ray *rays, int count, int depth, ShadingFunc *func, ShadingFunc *nohitFunc)
	{
		rapido::ray **r = reinterpret_cast<rapido::ray **>(alloca(sizeof(rapido::ray *) * count));
		for(int i = 0; i < count; ++i)
			r[i] = rays + i;

		// perform a bubble sort on the rays
		for(int j = count - 1; j > 0; --j)
		{
			bool swapped = false;
			for(int i = 0; i < j; ++i)
			{
				if(r[i]->hit.triId > r[i + 1]->hit.triId)
				{
					std::swap(r[i], r[i + 1]);
					swapped = true;
				}
			}
			if(!swapped) break;
		}

		int start = 0;
		while(start < count)
		{
			int end = start + 1;
			while(end < count && r[start]->hit.triId == r[end]->hit.triId) ++end;
			ShadingFunc *shader = (r[start]->hit.triId == rapido::ray::NoHit) ? nohitFunc : func;
			shader(r + start, end - start, depth);
			start = end;
		}
	}
};

#endif // __TR_RTGROUPEDSHADER_H__

