
#ifndef PACKET_H
#define PACKET_H

#include "ray.h"
#include "vectors.h"

namespace rapido
{
	struct packet
	{
		vector3 origin;
		vector3 direction;
		vector3 invdir;

		struct
		{
			quad triId;
			quad distance, beta, gamma;
		} hit;

		ray *rays[4];

#ifdef RECORDRAYSTATS
		ray::stats staticStats, dynamicStats;
#endif

		packet(ray &a, ray &b, ray &c, ray &d)
#ifndef SHUFFLERAYDATA
			: origin(a.origin, b.origin, c.origin, d.origin),
			direction(a.direction, b.direction, c.direction, d.direction),
			invdir(inverse(direction))
#endif
		{
			hit.triId = quad(ray::NoHit); hit.distance = Infinity4;
			rays[0] = &a; rays[1] = &b; rays[2] = &c; rays[3] = &d;
#ifdef SHUFFLERAYDATA
			__m128 xy10 = _mm_setzero_ps(), zw10 = _mm_setzero_ps();
			__m128 xy32 = _mm_setzero_ps(), zw32 = _mm_setzero_ps();
			xy10 = _mm_loadl_pi(xy10, reinterpret_cast<__m64 *>(&a.origin.x));
			zw10 = _mm_loadl_pi(zw10, reinterpret_cast<__m64 *>(&a.origin.z));
			xy10 = _mm_loadh_pi(xy10, reinterpret_cast<__m64 *>(&b.origin.x));
			zw10 = _mm_loadh_pi(zw10, reinterpret_cast<__m64 *>(&b.origin.z));
			xy32 = _mm_loadl_pi(xy32, reinterpret_cast<__m64 *>(&c.origin.x));
			zw32 = _mm_loadl_pi(zw32, reinterpret_cast<__m64 *>(&c.origin.z));
			xy32 = _mm_loadh_pi(xy32, reinterpret_cast<__m64 *>(&d.origin.x));
			zw32 = _mm_loadh_pi(zw32, reinterpret_cast<__m64 *>(&d.origin.z));
			__m128 ox4 = _mm_shuffle_ps(xy10, xy32, _MM_SHUFFLE(2,0,2,0));
			__m128 oy4 = _mm_shuffle_ps(xy10, xy32, _MM_SHUFFLE(3,1,3,1));
			__m128 oz4 = _mm_shuffle_ps(zw10, zw32, _MM_SHUFFLE(2,0,2,0));
			origin = vector3(ox4, oy4, oz4);

			xy10 = _mm_loadl_pi(xy10, reinterpret_cast<__m64 *>(&a.direction.x));
			zw10 = _mm_loadl_pi(zw10, reinterpret_cast<__m64 *>(&a.direction.z));
			xy10 = _mm_loadh_pi(xy10, reinterpret_cast<__m64 *>(&b.direction.x));
			zw10 = _mm_loadh_pi(zw10, reinterpret_cast<__m64 *>(&b.direction.z));
			xy32 = _mm_loadl_pi(xy32, reinterpret_cast<__m64 *>(&c.direction.x));
			zw32 = _mm_loadl_pi(zw32, reinterpret_cast<__m64 *>(&c.direction.z));
			xy32 = _mm_loadh_pi(xy32, reinterpret_cast<__m64 *>(&d.direction.x));
			zw32 = _mm_loadh_pi(zw32, reinterpret_cast<__m64 *>(&d.direction.z));
			__m128 dx4 = _mm_shuffle_ps(xy10, xy32, _MM_SHUFFLE(2,0,2,0));
			__m128 dy4 = _mm_shuffle_ps(xy10, xy32, _MM_SHUFFLE(3,1,3,1));
			__m128 dz4 = _mm_shuffle_ps(zw10, zw32, _MM_SHUFFLE(2,0,2,0));
			direction = vector3(dx4, dy4, dz4);
			invdir = inverse(direction);
#endif
		}

		~packet()
		{
#ifdef SHUFFLETRACERESULTS
			__m128 ttdd = _mm_movelh_ps(hit.triId.f4, hit.distance.f4);
			__m128 bbgg = _mm_movelh_ps(hit.beta.f4, hit.gamma.f4);
			rays[0]->hit.results = _mm_shuffle_ps(ttdd, bbgg, _MM_SHUFFLE(2, 0, 2, 0));
			rays[1]->hit.results = _mm_shuffle_ps(ttdd, bbgg, _MM_SHUFFLE(3, 1, 3, 1));

			ttdd = _mm_movehl_ps(hit.distance.f4, hit.triId.f4);
			bbgg = _mm_movehl_ps(hit.gamma.f4, hit.beta.f4);
			rays[2]->hit.results = _mm_shuffle_ps(ttdd, bbgg, _MM_SHUFFLE(2, 0, 2, 0));
			rays[3]->hit.results = _mm_shuffle_ps(ttdd, bbgg, _MM_SHUFFLE(3, 1, 3, 1));
#else
			for(int i = 0; i < 4; ++i)
			{
				rays[i]->hit.triId = hit.triId.i[i];
				rays[i]->hit.distance = hit.distance.f[i];
				rays[i]->hit.beta = hit.beta.f[i];
				rays[i]->hit.gamma = hit.gamma.f[i];
			}		
#endif
#ifdef RECORDRAYSTATS
			for(int i = 0; i < 4; ++i)
			{
				rays[i]->staticStats = staticStats;
				rays[i]->dynamicStats = dynamicStats;
			}
#endif
		}
	};
}

#endif // PACKET_H
