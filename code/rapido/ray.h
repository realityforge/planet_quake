
#ifndef RAY
#define RAY

#include <xmmintrin.h>
#include "floats.h"

//#define SHUFFLETRACERESULTS
//#define SHUFFLERAYDATA
#define RECORDRAYSTATS

#ifdef WIN32
#define ALIGNFRONT(x) __declspec(align(x))
#define ALIGNBACK(x) 
#else
#define ALIGNFRONT(x) 
#define ALIGNBACK(x)  __attribute__((aligned(x)))
#endif

namespace rapido
{
	struct ray
	{
		enum
		{
			NoHit = -1,

			MaxDynamicTriangleBits = 16,
			MaxDynamicTriangles = (1 << MaxDynamicTriangleBits),
			MaxDynamicInstanceBits = (sizeof(int) * 8 - 1/*sign*/ - MaxDynamicTriangleBits),
			MaxDynamicInstances = (1 << MaxDynamicInstanceBits)
		};

		union
		{
			struct // don't reorder fields!
			{
				int triId;
				float distance, beta, gamma;
			};
#ifdef SHUFFLETRACERESULTS
			__m128 results;
#endif
		} hit;

#ifdef SHUFFLERAYDATA
		ALIGNFRONT(16) float3 origin ALIGNBACK(16);
		ALIGNFRONT(16) float3 direction ALIGNBACK(16);
#else
		float3 origin, direction;
#endif

#ifdef RECORDRAYSTATS
		struct stats
		{
			int visitedInnerNodes, visitedLeaves;
			int intersectedTriangles;
			int splitNodes;
			stats() : visitedInnerNodes(0), visitedLeaves(0), intersectedTriangles(0), splitNodes(0) { }
		} staticStats, dynamicStats;
#endif

		struct
		{
			void *data;
		} user;

		ray() { hit.triId = NoHit; hit.distance = Infinity; }
		ray(const float3 &org, const float3 &dir) : origin(org), direction(dir) { hit.triId = NoHit; hit.distance = Infinity; }

		inline float3 GetHitPoint() const { return origin + direction * hit.distance; }

		inline ray SpawnSecondary(const float3 &dir) const { return ray(GetHitPoint(), dir); }
		inline ray SpawnSecondary(const float3 &dir, float epsilon) const { return ray(GetHitPoint() + dir * epsilon, dir); }

		inline bool HitTriangle() const { return (hit.triId != NoHit); }
		inline bool HitStaticTriangle() const { return (hit.triId & (1 << 31)) == 0; }
		inline bool HitDynamicTriangle() const { return (hit.triId & (1 << 31)) != 0; }

		inline int GetTriangleId() const { return HitDynamicTriangle() ? GetDynamicTriangleId() : GetStaticTriangleId(); }
		inline int GetStaticTriangleId() const { return hit.triId /*& 0x7fffffff*/; }
		inline int GetDynamicTriangleId() const { return hit.triId & ((1 << MaxDynamicTriangleBits) - 1); }
		inline int GetDynamicInstanceHandle() const { return (hit.triId >> MaxDynamicTriangleBits) & ((1 << MaxDynamicInstanceBits) - 1); }
	};
}

#endif // RAY
