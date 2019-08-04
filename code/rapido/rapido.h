
#ifndef RAPIDO
#define RAPIDO

#include "floats.h"
#include "ray.h"
#include "packet.h"
#include <assert.h>
#include <new>

namespace rapido
{
	enum TraceFlags
	{
		CommonOrigin = 0x1,		// all rays handed over to Trace share the same origin
		ShadowRays = 0x2,		// enable early-out on first hit < distance
		NoRayPackets = 0x4		// forces mono tracing by disallowing ray packets
	};

	inline unsigned int GetSignMask(float3 v)
	{
#if 1
		// use sign bits from floats
		union { float f; unsigned int i; } tmpx, tmpy, tmpz;
		tmpx.f = v.x; tmpy.f = v.y; tmpz.f = v.z;
		return ((tmpx.i >> 31) & 0x1) | ((tmpy.i >> 30) & 0x2) | ((tmpz.i >> 29) & 0x4);
#else
		return (lt0(v.x) ? 1 : 0) | (lt0(v.y) ? 2 : 0) | (lt0(v.z) ? 4 : 0);
#endif
	}

	template<class T>
	struct Tracer
	{
		T *scene;

		static const bool NeedsCommonDirSigns = T::NeedsCommonDirSigns;

		Tracer() : scene(0) { }
		Tracer(T *scene) : scene(scene) { }

		inline void Trace(ray &r, int flags)
		{
			assert(scene != 0);
			scene->Trace(r, flags);
		}

		inline void Trace(ray rays[], int count, int flags)
		{
			assert(scene != 0);
			if(count >= 4 && !(flags & NoRayPackets))
			{
				assert(rays != 0);

				// packetize rays
				if(NeedsCommonDirSigns)
				{
					ray *dircube[8][16];
					int counts[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
					for(int i = 0; i < count; ++i)
					{
						int idx = GetSignMask(rays[i].direction);
						int cnt = counts[idx]++;
						dircube[idx][cnt] = &rays[i];
						if(cnt == 15)
						{
							counts[idx] = 0; // array is full, process the rays
							ray **drays = dircube[idx];
							packet packets[4] = {
								packet(*drays[0], *drays[1], *drays[2], *drays[3]),
								packet(*drays[4], *drays[5], *drays[6], *drays[7]),
								packet(*drays[8], *drays[9], *drays[10], *drays[11]),
								packet(*drays[12], *drays[13], *drays[14], *drays[15])
							};
							scene->Trace(packets, 4, flags);
						}
					}

					for(int idx = 0; idx < 8; ++idx)
					{
						int cnt = counts[idx];
						if(cnt > 0)
						{
							ray **drays = dircube[idx];
							while(cnt >= 4)
							{
								packet p(*drays[0], *drays[1], *drays[2], *drays[3]);
								scene->Trace(p, flags);
								drays += 4; cnt -= 4;
							}

							// trace the rest using the mono tracer
							for(int i = 0; i < cnt; ++i)
								scene->Trace(*drays[i], flags);
						}
					}
				}
				else
				{
					int numberOfPackets = count >> 2;
					if(numberOfPackets > 1)
					{
						// allocate packets on the stack
						packet *packets = reinterpret_cast<packet *>(sse_alloca(sizeof(packet) * numberOfPackets));
						for(int i = 0, j = 0; i < numberOfPackets; ++i, j += 4)
							new(&packets[i]) packet(rays[j], rays[j + 1], rays[j + 2], rays[j + 3]);

						scene->Trace(packets, numberOfPackets, flags);

						// have to call the destructor explicitly due to placement new on the stack
						for(int i = 0; i < numberOfPackets; ++i)
							packets[i].~packet();
					}
					else
					{
						packet p(rays[0], rays[1], rays[2], rays[3]);
						scene->Trace(p, flags);
					}

					for(int i = numberOfPackets << 2; i < count; ++i)
						scene->Trace(rays[i], flags);
				}
			}
			else
				scene->Trace(rays, count, flags);
		}

		inline void Trace(packet &p, int flags)
		{
			assert(scene != 0);
			scene->Trace(p, flags);
		}

		inline void Trace(packet packets[], int count, int flags)
		{
			assert(scene != 0);
			scene->Trace(packets, count, flags);
		}
	};
}

#include "statickdtree.h"
#include "dynamicbvh.h"
#include "scenegroup.h"

#endif // RAPIDO
