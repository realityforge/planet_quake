
#include "statickdtree.h"
#include "mailbox.h"
using namespace std;

namespace rapido
{
	const int StaticKdTreeFileMarker = ('s' << 24) | ('K' << 16) | ('d' << 8) | 't';

	StaticKdTree::StaticKdTree() : _sceneMin(0.0f), _sceneMax(0.0f),
		_numberOfTriangles(0), _numberOfIndices(0), _numberOfNodes(0),
		_memory(0), _memoryLength(0), _triangles(0), _indices(0), _nodes(0)
	{ }

	StaticKdTree::StaticKdTree(const float3 &sceneMin, const float3 &sceneMax, int numberOfTriangles,
		int numberOfIndices, int numberOfNodes, const char *memory, unsigned int memoryLength) :
		_sceneMin(sceneMin), _sceneMax(sceneMax), _numberOfTriangles(numberOfTriangles),
		_numberOfIndices(numberOfIndices), _numberOfNodes(numberOfNodes),
		_memory(memory), _memoryLength(memoryLength)
	{
		SetPointers();		
	}

	StaticKdTree::StaticKdTree(const StaticKdTree &other)
	{
		_sceneMin = other._sceneMin; _sceneMax = other._sceneMax;
		_numberOfTriangles = other._numberOfTriangles;
		_numberOfIndices = other._numberOfIndices;
		_numberOfNodes = other._numberOfNodes;
		_memoryLength = other._memoryLength;
		if(other._memoryLength != 0)
		{
			_memory = reinterpret_cast<const char *>(sse_alloc(other._memoryLength));
			memcpy(const_cast<char *>(_memory), other._memory, other._memoryLength);
		}
		else
			_memory = 0;

		SetPointers();
	}

	StaticKdTree &StaticKdTree::operator=(const StaticKdTree &other)
	{
		if(this != &other)
		{
			if(_memory != 0) { sse_free(const_cast<char *>(_memory)); _memory = 0; }

			_sceneMin = other._sceneMin; _sceneMax = other._sceneMax;
			_numberOfTriangles = other._numberOfTriangles;
			_numberOfIndices = other._numberOfIndices;
			_numberOfNodes = other._numberOfNodes;
			_memoryLength = other._memoryLength;
			if(other._memoryLength != 0)
			{
				_memory = reinterpret_cast<const char *>(sse_alloc(other._memoryLength));
				memcpy(const_cast<char *>(_memory), other._memory, other._memoryLength);
			}

			SetPointers();
		}
		return *this;
	}

	void StaticKdTree::SetPointers()
	{
		const char *cur = _memory;

		_triangles = reinterpret_cast<const statictriangle *>(cur);
		unsigned int length = _numberOfTriangles * sizeof(statictriangle);
		unsigned int padding = (CacheLineLength - length % CacheLineLength);
		if(padding == CacheLineLength) padding = 0;
		cur += length + padding;

		_indices = reinterpret_cast<const int *>(cur);
		length = _numberOfIndices * sizeof(int);
		padding = (CacheLineLength - length % CacheLineLength);
		if(padding == CacheLineLength) padding = 0;
		cur += length + padding;

		// NOTE: offset start of nodes by one node yielding |pad|root|left|right|
		_nodes = reinterpret_cast<const node *>(cur) + 1;
	}

	StaticKdTree::~StaticKdTree()
	{
		if(_memory != 0) { sse_free(const_cast<char *>(_memory)); _memory = 0; }
	}

	istream& operator >>(istream &is, StaticKdTree &kdtree)
	{
		int marker = 0;
		is.read(reinterpret_cast<char *>(&marker), sizeof(marker));
		if(marker != StaticKdTreeFileMarker)
		{
			is.seekg(-static_cast<int>(sizeof(marker)), ios_base::cur);
			throw "Cannot read StaticKdTree from stream!";
		}

		float3 sceneMin, sceneMax;
		int numberOfTriangles, numberOfIndices, numberOfNodes;
		unsigned int memoryLength;
		is.read(reinterpret_cast<char *>(&sceneMin), sizeof(sceneMin));
		is.read(reinterpret_cast<char *>(&sceneMax), sizeof(sceneMax));
		is.read(reinterpret_cast<char *>(&numberOfTriangles), sizeof(numberOfTriangles));
		is.read(reinterpret_cast<char *>(&numberOfIndices), sizeof(numberOfIndices));
		is.read(reinterpret_cast<char *>(&numberOfNodes), sizeof(numberOfNodes));
		is.read(reinterpret_cast<char *>(&memoryLength), sizeof(memoryLength));

		char *memory = 0;
		try
		{
			if(memoryLength != 0)
			{
				memory = reinterpret_cast<char *>(sse_alloc(memoryLength));
				is.read(memory, memoryLength);
			}

			kdtree = StaticKdTree(sceneMin, sceneMax, numberOfTriangles, numberOfIndices,
				numberOfNodes, memory, memoryLength);
			return is;
		}
		catch(...)
		{
			if(memory != 0) sse_free(memory);
			throw;
		}
	}

	ostream& operator <<(ostream &os, const StaticKdTree &kdtree)
	{
		os.write(reinterpret_cast<const char *>(&StaticKdTreeFileMarker), sizeof(StaticKdTreeFileMarker));
		os.write(reinterpret_cast<const char *>(&kdtree._sceneMin), sizeof(kdtree._sceneMin));
		os.write(reinterpret_cast<const char *>(&kdtree._sceneMax), sizeof(kdtree._sceneMax));
		os.write(reinterpret_cast<const char *>(&kdtree._numberOfTriangles), sizeof(kdtree._numberOfTriangles));
		os.write(reinterpret_cast<const char *>(&kdtree._numberOfIndices), sizeof(kdtree._numberOfIndices));
		os.write(reinterpret_cast<const char *>(&kdtree._numberOfNodes), sizeof(kdtree._numberOfNodes));
		os.write(reinterpret_cast<const char *>(&kdtree._memoryLength), sizeof(kdtree._memoryLength));
		os.write(kdtree._memory, kdtree._memoryLength);
		return os;
	}

	inline bool StaticKdTree::ClipToSceneBB(float &nearDist, float &farDist, const ray &r, const float3 &invDir) const
	{
		for(int axis = 0; axis < 3; ++axis)
		{
			float origin = r.origin[axis], direction = invDir[axis];
			float t0 = (_sceneMin[axis] - origin) * direction;
			float t1 = (_sceneMax[axis] - origin) * direction;
			nearDist = max(nearDist, min(t0, t1));
			farDist = min(farDist, max(t0, t1));
			// branch free code is perferable! if(nearDist > farDist) return false;
		}
		return (nearDist <= farDist);
	}

	void StaticKdTree::Trace(ray &r, int flags)
	{
		float3 invDir = 1.0f / r.direction;
		float nearDist = 0.0f, farDist = (flags & ShadowRays) ? 1.0f : r.hit.distance; // initialize near/far distance
		if(!ClipToSceneBB(nearDist, farDist, r, invDir))
			return;

		unsigned int signMask = GetSignMask(r.direction);
		const node *nodes[MaxDepth]; float nears[MaxDepth], fars[MaxDepth]; int stackIdx = 0;
		Mailbox mailbox;

		const node *cur = _nodes;
		while(true)
		{
traverse:	while(!cur->IsLeaf()) // traverse the tree down to a leaf
			{
#ifdef RECORDRAYSTATS
				++r.staticStats.visitedInnerNodes;
#endif
				int axis = cur->inner.GetAxis();
				unsigned int nearIndex = (signMask >> axis) & 0x1;
				float d = (cur->inner.splitPos - r.origin[axis]) * invDir[axis];
				const node *children = cur->inner.GetChildren();
				if(d >= nearDist)
				{
					if(d <= farDist)
					{
						nodes[stackIdx] = children + (nearIndex ^ 1); // push far child onto the stack
						nears[stackIdx] = d; fars[stackIdx++] = farDist;
						farDist = d; // continue traversal with near child
					}
					cur = children + nearIndex;
				}
				else
					cur = children + (nearIndex ^ 1);
			}

			// intersect triangles in the leaf with the ray
#ifdef RECORDRAYSTATS
			++r.staticStats.visitedLeaves;
#endif
			const int *indices = cur->leaf.GetIndices();
			for(int i = 0; i < cur->leaf.numberOfIndices; ++i)
			{
				int index = indices[i];
				if(mailbox.SkipIntersectionTestWith(index))
					continue;
#ifdef RECORDRAYSTATS
				++r.staticStats.intersectedTriangles;
#endif
				_triangles[index].IntersectWith(r, index);
			}

			if(r.hit.distance <= ((flags & ShadowRays) ? 1.0f : farDist)) // early out test
				break;

			// pop the next node from the stack
			while(stackIdx > 0)
			{
				nearDist = nears[--stackIdx]; farDist = min(r.hit.distance, fars[stackIdx]);
				if(nearDist <= farDist) { cur = nodes[stackIdx]; goto traverse; }
			}
			break;
		}
	}

	inline unsigned int StaticKdTree::ClipToSceneBB(__m128 &nearDist4, __m128 &farDist4, const packet &p) const
	{
		for(int axis = 0; axis < 3; ++axis)
		{
			__m128 lower = _mm_set1_ps(_sceneMin[axis]), upper = _mm_set1_ps(_sceneMax[axis]);
			__m128 origin4 = p.origin[axis].f4, direction4 = p.invdir[axis].f4;
			__m128 t0 = _mm_mul_ps(_mm_sub_ps(lower, origin4), direction4);
			__m128 t1 = _mm_mul_ps(_mm_sub_ps(upper, origin4), direction4);
			nearDist4 = _mm_max_ps(nearDist4, _mm_min_ps(t0, t1));
			farDist4 = _mm_min_ps(farDist4, _mm_max_ps(t0, t1));
		}
		return _mm_movemask_ps(_mm_cmple_ps(nearDist4, farDist4));
	}

	void StaticKdTree::Trace(packet &p, int flags)
	{
		__m128 nearDist4 = Zero4, farDist4 = (flags & ShadowRays) ? One4 : p.hit.distance.f4; // initialize near/far distance
		unsigned int activeMask = ClipToSceneBB(nearDist4, farDist4, p);
		if(activeMask == 0)
			return;

		unsigned int terminationMask = activeMask ^ AllMask;
		unsigned int signMask = GetSignMask(p.rays[0]->direction);
		const node *nodes[MaxDepth]; __m128 nears[MaxDepth], fars[MaxDepth]; int stackIdx = 0;
		Mailbox mailbox;

		const node *cur = _nodes;
		while(true)
		{
traverse:		while(!cur->IsLeaf()) // traverse the tree down to a leaf
			{
#ifdef RECORDRAYSTATS
				++p.staticStats.visitedInnerNodes;
#endif
				int axis = cur->inner.GetAxis();
				unsigned int nearIndex = (signMask >> axis) & 0x1;
				__m128 d = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(cur->inner.splitPos), p.origin[axis].f4), p.invdir[axis].f4);
				const node *children = cur->inner.GetChildren();
				unsigned int near = _mm_movemask_ps(_mm_cmpge_ps(d, nearDist4)) & activeMask;
				if(near != 0)
				{
					unsigned int far = _mm_movemask_ps(_mm_cmple_ps(d, farDist4)) & activeMask;
					if(far != 0)
					{
						nodes[stackIdx] = children + (nearIndex ^ 1); // push far child onto the stack
						nears[stackIdx] = d; fars[stackIdx++] = farDist4;
						farDist4 = d; // continue traversal with near child
					}
					cur = children + nearIndex; activeMask = near;
				}
				else
					cur = children + (nearIndex ^ 1);
			}

			// intersect triangles in the leaf with the ray
#ifdef RECORDRAYSTATS
			++p.staticStats.visitedLeaves;
#endif
			const int *indices = cur->leaf.GetIndices();
			for(int i = 0; i < cur->leaf.numberOfIndices; ++i)
			{
				int index = indices[i];
				if(mailbox.SkipIntersectionTestWith(index))
					continue;
#ifdef RECORDRAYSTATS
				++p.staticStats.intersectedTriangles;
#endif
				_triangles[index].IntersectWith(p, index);
			}

			// compute the new termination mask
			terminationMask |= _mm_movemask_ps(_mm_cmple_ps(p.hit.distance.f4, (flags & ShadowRays) ? One4 : farDist4)) & activeMask;
			if(terminationMask == AllMask) // early out test
				break;

			// pop the next node from the stack
			while(stackIdx > 0)
			{
				nearDist4 = nears[--stackIdx]; farDist4 = _mm_min_ps(p.hit.distance.f4, fars[stackIdx]);
				activeMask = _mm_movemask_ps(_mm_cmple_ps(nearDist4, farDist4)) & ~terminationMask;
				if(activeMask != 0) { cur = nodes[stackIdx]; goto traverse; }
			}
			break;
		}
	}
}
