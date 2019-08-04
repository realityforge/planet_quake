
#ifndef DYNAMICBVH_H
#define DYNAMICBVH_H

#include "dynamicscene.h"
#include "lockbook.h"

// TODO: potential bug in single packet traversal -> crash in bart demo

namespace rapido
{
	namespace internalbvh
	{
		struct BvhObject
		{
			float3 lower, upper, centroid;
			int leaf; // index of the hosting leaf
		};

		template<class ObjGetterType, int MaxDepth>
		class Bvh
		{
			struct node
			{
				int offsetToParent;
				int offsetToChildren; // INNER: offset to the children, stored as a pair
				float3 lower, upper;
				int firstObj, lastObj;
				int nearmask; // INNER: mask for determining which child to visit first, LEAF: 31 .. leaf node, 30 .. final leaf, depth
				float splitCosts; // INNER: original costs for splitting

				// common accessors
				inline node *GetParent() { return (this + offsetToParent); }

				// inner node accessors
				inline bool IsInnerNode() const { return (nearmask & (1 << 31)) == 0; }
				inline node *GetChildren() { return (this + offsetToChildren); }
				inline int GetNearIndex(unsigned int signmask) const { return (nearmask >> signmask) & 0x1; }

				// leaf accessors
				inline bool IsLeaf() const { return (nearmask & (1 << 31)) != 0; }
				inline bool CanSplit() const { return (nearmask & (1 << 30)) == 0; }
				inline int GetDepth() const { return nearmask & 0x1fffffff; }

				// setters
				inline void TurnIntoInner(node *children, int nearmask)
				{
					offsetToChildren = children - this;
					this->nearmask = nearmask & ((1 << 29) | 0xff);
				}

				inline void CreateLeaf(node *parent, bool final, int depth)
				{
					offsetToParent = parent - this;
					nearmask = (1 << 31) | (final ? (1 << 30) : 0) | (depth & 0x3fffffff);
				}

				inline void MarkAsFinalLeaf() { nearmask |= (1 << 30); }

				// intersection test
				inline bool IntersectWith(const ray &r, const float3 &invDir) const;
				inline unsigned int IntersectWith(const packet &p) const;
				inline unsigned int IntersectWith(const packet packets[], int &first, int &last) const;
			};

			enum SplitResult { Finalized, Split, InProgress };

			ObjGetterType _objGetter;
			node *_nodes;
			LockBook<node> _nodeLock;
			volatile long _nextNode;
			int _lastNumberOfObjects;

			Bvh(const Bvh<ObjGetterType, MaxDepth> &other); // not allowed, need local object getter

			bool RebuildFavorable(node *n, float threshold);
			int SplitNode(node *n); // returns a value from the SplitResult enumeration

		public:
			static const bool NeedsCommonDirSigns = false;

			float3 lower, upper;
			std::vector<int> indices;

			Bvh(const ObjGetterType &objGetter);
			Bvh(const Bvh<ObjGetterType, MaxDepth> &other, const ObjGetterType &objGetter);
			Bvh<ObjGetterType, MaxDepth> &operator=(const Bvh<ObjGetterType, MaxDepth> &other);
			~Bvh();

			void Reset();
			bool Refit(const std::set<int> &leaves);
			bool SelectivelyRebuild(const std::set<int> &leaves, float threshold, int *count = 0);

			template<class LeafOpType>
			void Traverse(ray &r, const float3 &invDir, int flags, unsigned int signMask, const LeafOpType &leafOp);
			template<class LeafOpType>
			void Traverse(packet &p, int flags, unsigned int signMask, const LeafOpType &leafOp);
			template<class LeafOpType>
			void Traverse(packet packets[], int count, int flags, unsigned int signMask, const LeafOpType &leafOp);
		};
	}

	typedef DynamicScene<internalbvh::Bvh, internalbvh::BvhObject> DynamicBvh;
}

#include "dynamicbvh.inl"

#endif // DYNAMICBVH_H
