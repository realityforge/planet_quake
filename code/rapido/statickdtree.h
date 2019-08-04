
#ifndef STATICKDTREE_H
#define STATICKDTREE_H

#include "rapido.h"
#include "statictriangle.h"
#include <iostream>

namespace rapido
{
	class StaticKdTree
	{
		union node
		{
			struct
			{
				int numberOfIndices; // number of triangle indices
				int offsetToIndices; // offset to array of integers
				inline const int *GetIndices() const { return reinterpret_cast<const int *>(reinterpret_cast<const char *>(this) + offsetToIndices); }
			} leaf;

			struct
			{
				int offsetAndAxis; // 0..2 axis, 2..30 offset to children, 31 node marker
				float splitPos;	// position of the splitting plane
				inline int GetOffset() const { return (offsetAndAxis & 0x7ffffffc); }
				inline int GetAxis() const { return (offsetAndAxis & 0x3); }
				inline const node *GetChildren() const { return reinterpret_cast<const node *>(reinterpret_cast<const char *>(this) + GetOffset()); }
			} inner;

			inline bool IsLeaf() const { return (inner.offsetAndAxis & (1 << 31)) == 0; }
		};

		friend class StaticKdTreeBuilder;

		float3 _sceneMin, _sceneMax;
		int _numberOfTriangles, _numberOfIndices, _numberOfNodes;
		const char *_memory;
		unsigned int _memoryLength;
		const statictriangle *_triangles;
		const int *_indices;
		const node *_nodes;

		StaticKdTree(const float3 &sceneMin, const float3 &sceneMax, int numberOfTriangles,
			int numberOfIndices, int numberOfNodes, const char *memory, unsigned int memoryLength);
		void SetPointers();

		inline bool ClipToSceneBB(float &nearDist, float &farDist, const ray &r, const float3 &invDir) const;
		inline unsigned int ClipToSceneBB(__m128 &nearDist4, __m128 &farDist4, const packet &p) const;

	public:
		enum { MaxDepth = 128, CacheLineLength = 32 };
		static const bool NeedsCommonDirSigns = true;

		StaticKdTree();
		StaticKdTree(const StaticKdTree &other);
		StaticKdTree &operator=(const StaticKdTree &other);
		~StaticKdTree();

		friend std::istream& operator >>(std::istream &is, StaticKdTree &kdtree);
		friend std::ostream& operator <<(std::ostream &os, const StaticKdTree &kdtree);

		inline int GetNumberOfTriangles() const { return _numberOfTriangles; }
		inline const statictriangle *GetTriangles() const { return _triangles; }

		void Trace(ray &r, int flags);
		inline void Trace(ray rays[], int count, int flags) { for(int i = 0; i < count; ++i) Trace(rays[i], flags); }
		void Trace(packet &p, int flags);
		inline void Trace(packet packets[], int count, int flags) { for(int i = 0; i < count; ++i) Trace(packets[i], flags); }
	};

	class StaticKdTreeBuilder
	{
		static unsigned int CalculateMemoryLength(int numberOfTriangles, int numberOfIndices, int numberOfNodes);
		static StaticKdTree BuildInternal(void *);

	public:
		typedef bool isdecalfunc(int triId);

		static StaticKdTree BuildFrom(const float3 vertices[], int numberOfVertices, const int triangleList[], int numberOfTriangles);
		static StaticKdTree BuildFrom(const float3 vertexList[], int numberOfTriangles);
		static int OffsetDecals(StaticKdTree &kdtree, isdecalfunc *func, float offset);
	};
}

#endif // STATICKDTREE_H
