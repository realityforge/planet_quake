
#ifndef DYNAMICBVH_INL
#define DYNAMICBVH_INL

#include <algorithm>
#include "interlocked.h"
using namespace std;

//#define NO_EXACT_LEAF_MASK
//#define INTERPOLATE_SAMPLES
//#define USE_SWEEPING

namespace rapido { namespace internalbvh
{
	// Data Management ----------------------------------------------------

	template<class ObjGetterType, int MaxDepth>
	Bvh<ObjGetterType, MaxDepth>::Bvh(const ObjGetterType &objGetter) : _objGetter(objGetter), _nextNode(0), lower(0.0f), upper(0.0f)
	{
		_lastNumberOfObjects = objGetter.GetSize();
		int numberOfNodes = max(2 * _lastNumberOfObjects - 1, 1);
		indices.resize(objGetter.GetSize());
		_nodes = reinterpret_cast<node *>(malloc(sizeof(node) * numberOfNodes));

		// create an empty root node
		node *root = _nodes;
		root->lower = lower; root->upper = upper;
		root->firstObj = 1; root->lastObj = 0;
		root->CreateLeaf(root, true, 0);
	}

	template<class ObjGetterType, int MaxDepth>
	Bvh<ObjGetterType, MaxDepth>::Bvh(const Bvh &other, const ObjGetterType &objGetter) :
		_objGetter(objGetter), _nextNode(other._nextNode), _lastNumberOfObjects(other._lastNumberOfObjects),
		lower(other.lower), upper(other.upper), indices(other.indices)
	{
		int numberOfNodes = max(2 * static_cast<int>(indices.size()) - 1, 1);
		_nodes = reinterpret_cast<node *>(malloc(sizeof(node) * numberOfNodes));
		memcpy(_nodes, other._nodes, sizeof(node) * numberOfNodes);
	}

	template<class ObjGetterType, int MaxDepth>
	Bvh<ObjGetterType, MaxDepth> &Bvh<ObjGetterType, MaxDepth>::operator=(const Bvh &other)
	{
		if(this != &other)
		{
			_nextNode = other._nextNode; lower = other.lower; upper = other.upper; indices = other.indices;

			int numberOfNodes = max(2 * static_cast<int>(indices.size()) - 1, 1);
			_nodes = reinterpret_cast<node *>(realloc(_nodes, sizeof(node) * numberOfNodes));
			memcpy(_nodes, other._nodes, sizeof(node) * numberOfNodes);

			_lastNumberOfObjects = other._lastNumberOfObjects;
		}
		return *this;
	}

	template<class ObjGetterType, int MaxDepth>
	Bvh<ObjGetterType, MaxDepth>::~Bvh()
	{
		if(_nodes != 0)
			free(_nodes);
	}

	template<class ObjGetterType, int MaxDepth>
	void Bvh<ObjGetterType, MaxDepth>::Reset()
	{
		int numberOfObjects = _objGetter.GetSize();
		if(_lastNumberOfObjects != numberOfObjects)
		{
			int numberOfNodes = max(2 * numberOfObjects - 1, 1);
			_nodes = reinterpret_cast<node *>(realloc(_nodes, sizeof(node) * numberOfNodes));
			_lastNumberOfObjects = numberOfObjects;
		}

		// create root node containing all geometry
		node *root = _nodes; _nextNode = 1; // get the first node and reset node allocations
		root->lower = lower; root->upper = upper;
		root->firstObj = 0; root->lastObj = numberOfObjects - 1;
		root->CreateLeaf(root, (numberOfObjects <= 1), 0);
		
		// move all objects into the root node
		for(int i = 0; i < numberOfObjects; ++i)
			_objGetter.Get(indices[i]).leaf = 0;
	}

	// Rebuilding and Refitting -----------------------------------------------

	inline static bool Equal(const float3 &a, const float3 &b)
	{
		return (a.x == b.x && a.y == b.y && a.z == b.z);
	}

	template<class ObjGetterType, int MaxDepth>
	bool Bvh<ObjGetterType, MaxDepth>::Refit(const std::set<int> &leaves)
	{
		map<int, set<node *> > inners; // maps from depth to a set of inner nodes
		int minDepth = MaxDepth, depth = 0;

		// evaluate bounding boxes of the leaves
		bool updatedRoot = false;
		for(set<int>::const_iterator it = leaves.begin(); it != leaves.end(); ++it)
		{
			node *n = _nodes + *it;
			assert(n->IsLeaf());

			const typename ObjGetterType::Type *obj = &_objGetter.Get(indices[n->firstObj]);
			float3 lower = obj->lower, upper = obj->upper;
			for(int i = n->firstObj + 1; i <= n->lastObj; ++i)
			{
				obj = &_objGetter.Get(indices[i]);
				lower = min(lower, obj->lower);
				upper = max(upper, obj->upper);
			}

			if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
			{
				n->lower = lower; n->upper = upper;
				int depthOfParent = n->GetDepth() - 1;
				updatedRoot = (depthOfParent == -1);
				if(depthOfParent >= 0) // violated if we're operating on triangles in the root node
				{
					assert(!n->GetParent()->IsLeaf());
					inners[depthOfParent].insert(n->GetParent());

					minDepth = min(minDepth, depthOfParent);
					depth = max(depth, depthOfParent);
				}
			}
		}

		if(depth == 0) // all triangles were in the root node or we did not perform any updates, we're done here!
		{
			if(updatedRoot) { lower = _nodes->lower; upper = _nodes->upper; } // update hierarchy bounding box
			return updatedRoot; // did we modify the roots bounding box?
		}

		// update inner nodes in level order up to the lowest common ancestor
		while(depth > minDepth || inners[depth].size() > 1)
		{
			for(typename set<node *>::iterator it = inners[depth].begin(); it != inners[depth].end(); ++it)
			{
				node *n = *it;
				assert(!n->IsLeaf());

				const node *children = n->GetChildren();
				float3 lower = min(children[0].lower, children[1].lower);
				float3 upper = max(children[0].upper, children[1].upper);
				if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
				{
					n->lower = lower; n->upper = upper;
					updatedRoot = (depth == 0);
					assert(!n->GetParent()->IsLeaf());
					inners[depth - 1].insert(n->GetParent());
				}
			}
			inners[depth].clear(); --depth;
		}

		// update the rest of the bounding boxes on the path to the root
		if(inners[depth].size() != 0)
		{
			node *n = *inners[depth].begin();
			while(true)
			{
				assert(!n->IsLeaf());
				const node *children = n->GetChildren();
				float3 lower = min(children[0].lower, children[1].lower);
				float3 upper = max(children[0].upper, children[1].upper);
				if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
				{
					n->lower = lower; n->upper = upper;
					if(depth-- == 0)
					{
						updatedRoot = true;
						break; // just updated the root node, we're done with refitting!
					}
				}
				else
					break; // definitely did not update the root node

				n = n->GetParent();
			}
		}

		if(updatedRoot) { lower = _nodes->lower; upper = _nodes->upper; } // update hierarchy bounding box
		return updatedRoot;
	}

	inline static float EvaluateSurfaceArea(const float3 &bbMin, const float3 &bbMax)
	{
		float3 delta = bbMax - bbMin;
		return /*2.0 */ (delta.x * delta.y + (delta.x + delta.y) * delta.z);
	}

	template<class ObjGetterType, int MaxDepth>
	bool Bvh<ObjGetterType, MaxDepth>::RebuildFavorable(node *n, float threshold)
	{
		assert(!n->IsLeaf()); // rebuilding does not make sense for leaves
		const node *children = n->GetChildren();
		// reevaluate costs of node configuration
		float sa = EvaluateSurfaceArea(n->lower, n->upper);
		float lsa = EvaluateSurfaceArea(children[0].lower, children[0].upper);
		float rsa = EvaluateSurfaceArea(children[1].lower, children[1].upper);
		int leftCount = children[0].lastObj - children[0].firstObj + 1;
		int rightCount = children[1].lastObj - children[1].firstObj + 1;
		float splitCosts = (lsa * leftCount + rsa * rightCount) / sa;
		return (splitCosts >= n->splitCosts * threshold);
	}

	struct IsPartOfFreeList
	{
		const vector<int> &_pairs;
		IsPartOfFreeList(const vector<int> &pairs) : _pairs(pairs) { }
		bool operator()(const int &value)
		{
			// _nodes layout: |0:root|1:left|2:right|3:left|4:right|
			bool left = (value & 0x1) != 0; // _pairs stores index of left child, if value corresponds to right child, subtract 1 for index of pair
			return find(_pairs.begin(), _pairs.end(), left ? value : (value - 1)) != _pairs.end();
		}
	};

	template<class ObjGetterType, int MaxDepth>
	bool Bvh<ObjGetterType, MaxDepth>::SelectivelyRebuild(const std::set<int> &leaves, float threshold, int *count)
	{
		map<int, set<node *> > inners; // maps from depth to a set of inner nodes
		int minDepth = MaxDepth, depth = 0;
		if(count) *count = 0;

		// evaluate bounding boxes of the leaves
		bool updatedRoot = false;
		for(set<int>::const_iterator it = leaves.begin(); it != leaves.end(); ++it)
		{
			node *n = _nodes + *it;
			assert(n->IsLeaf());

			const typename ObjGetterType::Type *obj = &_objGetter.Get(indices[n->firstObj]);
			float3 lower = obj->lower, upper = obj->upper;
			for(int i = n->firstObj + 1; i <= n->lastObj; ++i)
			{
				obj = &_objGetter.Get(indices[i]);
				lower = min(lower, obj->lower);
				upper = max(upper, obj->upper);
			}

			if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
			{
				n->lower = lower; n->upper = upper;
				int depthOfParent = n->GetDepth() - 1;
				updatedRoot = (depthOfParent == -1);
				if(depthOfParent >= 0) // violated if we're operating on triangles in the root node
				{
					assert(!n->GetParent()->IsLeaf());
					inners[depthOfParent].insert(n->GetParent());

					minDepth = min(minDepth, depthOfParent);
					depth = max(depth, depthOfParent);
				}
			}
		}

		// NOTE: the following code differs from refitting
		if(depth == 0) // all triangles were in the root node or we did not perform any updates, we're done here!
		{
			if(updatedRoot) { lower = _nodes->lower; upper = _nodes->upper; } // update hierarchy bounding box
			return updatedRoot; // did we modify the roots bounding box?
		}

		list<int> rebuildInners; // collection of inner nodes that are subject for rebuilding
		node *nodes = _nodes;

		// update inner nodes in level order up to the lowest common ancestor
		while(depth > minDepth || inners[depth].size() > 1)
		{
			for(typename set<node *>::iterator it = inners[depth].begin(); it != inners[depth].end(); ++it)
			{
				node *n = *it;
				assert(!n->IsLeaf());

				const node *children = n->GetChildren();
				float3 lower = min(children[0].lower, children[1].lower);
				float3 upper = max(children[0].upper, children[1].upper);
				if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
				{
					n->lower = lower; n->upper = upper;
					updatedRoot = (depth == 0);
					assert(!n->GetParent()->IsLeaf());
					inners[depth - 1].insert(n->GetParent());
				}

				if(RebuildFavorable(n, threshold))
					rebuildInners.push_back(n - nodes);
			}
			inners[depth].clear(); --depth;
		}

		// update the rest of the bounding boxes on the path to the root
		if(inners[depth].size() != 0)
		{
			node *n = *inners[depth].begin();
			while(true)
			{
				assert(!n->IsLeaf());
				const node *children = n->GetChildren();
				float3 lower = min(children[0].lower, children[1].lower);
				float3 upper = max(children[0].upper, children[1].upper);
				if(!Equal(n->lower, lower) || !Equal(n->upper, upper))
				{
					n->lower = lower; n->upper = upper;
					if(RebuildFavorable(n, threshold))
						rebuildInners.push_back(n - nodes);

					if(depth-- == 0)
					{
						updatedRoot = true;
						break; // just updated the root node, we're done with refitting!
					}
				}
				else
				{
					if(RebuildFavorable(n, threshold))
						rebuildInners.push_back(n - nodes);
					break; // definitely did not update the root node
				}

				n = n->GetParent();
			}
		}

		// perform rebuilds of nodes in rebuildInners in reverse order
		if(rebuildInners.size() > 0)
		{
			if(count) *count = rebuildInners.size();

			// always access the last element in rebuildInners: this way we process nodes that are closer
			// to the root first and reduce the number of operations by culling subtrees
			if(*rebuildInners.rbegin() == 0)
			{
				// rebuilding the root node allows for shortcuts :-)
				node *n = _nodes;
				assert(!n->IsLeaf());
				n->CreateLeaf(n, (n->lastObj <= n->firstObj), 0);
				_nextNode = 1; // reset node allocations
			}
			else
			{
				while(rebuildInners.size() > 0)
				{
					node *rebuild = nodes + *rebuildInners.rbegin();
					rebuildInners.pop_back();
					assert(rebuild != nodes); // root node should have been handled in the other branch!
					assert(!rebuild->IsLeaf());

					// collect all nodes from n's subtree for deletion
					vector<int> pairs;
					node *stack[MaxDepth], *n = rebuild; int stackIdx = 0;
					do
					{
						assert(!n->IsLeaf());
						n = n->GetChildren();
						pairs.push_back(n - nodes);
						if(n->IsLeaf())
						{
							++n;
							if(n->IsLeaf()) // pop a node from the stack if possible
								{ n = (stackIdx > 0) ? stack[--stackIdx] : 0; }
						}
						else if(!(n + 1)->IsLeaf())
							{ stack[stackIdx++] = n + 1; }
					}
					while(n != 0);
					assert(stackIdx == 0);

					// remove nodes from rebuildInners
					rebuildInners.remove_if(IsPartOfFreeList(pairs));

					// determine the depth of the node we wish to rebuild
					int rebuildDepth = 0;
					n = rebuild;
					while(n != nodes)
					{
						n = n->GetParent();
						assert(!n->IsLeaf());
						++rebuildDepth;
					}

					// turns n into a leaf, effectively cutting off its subtree
					rebuild->CreateLeaf(rebuild->GetParent(), (rebuild->lastObj <= rebuild->firstObj), rebuildDepth);

					// update the leaf-fields of the objects
					int leaf = rebuild - _nodes;
					for(int i = rebuild->firstObj; i <= rebuild->lastObj; ++i)
						_objGetter.Get(indices[i]).leaf = leaf;

					// compact memory (for each pair _nextNode will be decremented by 2)
					// if we move a node in rebuildInners, we have to update the index in rebuildInners!
					sort(pairs.begin(), pairs.end()); // minimize copying of nodes by deleting from the tail
					for(vector<int>::reverse_iterator it = pairs.rbegin(); it != pairs.rend(); ++it)
					{
						_nextNode -= 2;
						if(*it >= _nextNode)
							continue;

						node *dest = nodes + *it, *src = nodes + _nextNode;

						// update src's parent's offset to its children
						assert(src[0].GetParent() == src[1].GetParent());
						node *parent = src->GetParent();
						assert(!parent->IsLeaf());
						parent->offsetToChildren = dest - parent;
						assert(parent->GetChildren() == dest);

						for(int i = 0; i < 2; ++i)
						{
							// update indices in rebuildInners
							list<int>::iterator idx = find(rebuildInners.begin(), rebuildInners.end(), &src[i] - nodes);
							if(idx != rebuildInners.end())
								*idx = &dest[i] - nodes;

							dest[i] = src[i]; // copy node
							dest[i].offsetToParent = parent - &dest[i]; // update offset to parent

							if(src[i].IsLeaf())
							{
								// update offsets of objects to their respective leaves
								int leaf = &dest[i] - _nodes;
								for(int j = src[i].firstObj; j <= src[i].lastObj; ++j)
									_objGetter.Get(indices[j]).leaf = leaf;
							}
							else
							{
								node *children = src[i].GetChildren(); // update children's offset to their parent (us)
								children[0].offsetToParent = &dest[i] - &children[0];
								children[1].offsetToParent = &dest[i] - &children[1];
								dest[i].offsetToChildren = children - &dest[i]; // update offset to our children
								assert(dest[i].GetChildren() == src[i].GetChildren());
							}
						}

					}
				}
			}
		}

		if(updatedRoot) { lower = _nodes->lower; upper = _nodes->upper; } // update hierarchy bounding box
		return updatedRoot;
	}

	// Construction -------------------------------------------------------

#ifdef INTERPOLATE_SAMPLES
	inline static float FindSahMinimum(float al0, int cl0, float ar0, int cr0, float al1, int cl1, float ar1, int cr1)
	{
		double idenom = 2 * ((al0-al1) * (cl0-cl1) + (ar0-ar1) * (cr0-cr1));
		if(idenom == 0.0) return 0.5f;
		double t = (al0*(2*cl0-cl1) - al1*cl0 + ar0*(2*cr0-cr1) - ar1*cr0)/idenom;
		return max(min(t, 1.0), 0.0);
	}
#endif

	template<class T>
	class CompareObjectsByAxis
	{
		const T &_objGetter;
		int _axis;

	public:
		inline CompareObjectsByAxis(const T &objGetter, int axis) :
			_objGetter(objGetter), _axis(axis)
		{ }

		inline bool operator()(const int &a, const int &b) const
		{
			return _objGetter.Get(a).centroid[_axis] < _objGetter.Get(b).centroid[_axis];
		}
	};

	template<class ObjGetterType, int MaxDepth>
	int Bvh<ObjGetterType, MaxDepth>::SplitNode(node *n)
	{
		// try to lock this node for the current thread
		if(!_nodeLock.Lock(n)) { /* another thread is currently busy splitting this node */ return InProgress; }
		if(!n->IsLeaf()) { /* another thread split this into a node */ _nodeLock.Unlock(n); return Split; }
		else if(!n->CanSplit()) { /* another thread marked this node as a final leaf */ _nodeLock.Unlock(n); return Finalized; }

		int first = n->firstObj, last = n->lastObj;
		int numberOfObjects = (last - first + 1), depth = n->GetDepth();

		int mid, bestAxis = -1;
		float3 bestLowers[2] = { 0.0f, 0.0 }, bestUppers[2] = { 0.0f, 0.0 }; // NOTE: initialize to shut up the compiler

#ifdef USE_SWEEPING
		const int SweepTriangles = 16;
		if(numberOfObjects <= SweepTriangles)
		{
			float bestCosts = EvaluateSurfaceArea(n->lower, n->upper) * numberOfObjects;
			mid = -1;

			for(int axis = 0; axis < 3; ++axis)
			{
				// sort object range [first;last] by axis
				std::sort(indices.begin() + first, indices.begin() + (last + 1),
					CompareObjectsByAxis<T>(_objGetter, axis));

				float3 rightLowers[SweepTriangles], rightUppers[SweepTriangles];
				const typename ObjGetterType::Type *obj = &_objGetter.Get(indices[last]);
				rightLowers[0] = obj->lower; rightUppers[0] = obj->upper;
				for(int i = last - 1, j = 1; i > first; --i, ++j)
				{
					ObjGetterTypeobj = &_objGetter.Get(indices[i]);
					rightLowers[j] = min(rightLowers[j - 1], obj->lower);
					rightUppers[j] = max(rightUppers[j - 1], obj->upper);
				}

				obj = &_objGetter.Get(indices[first]);
				float3 lower = obj->lower, upper = obj->upper;
				for(int i = first, j = numberOfObjects - 2; i < last; ++i, --j)
				{
					obj = &_objGetter.Get(indices[i]);
					lower = min(lower, obj->lower);
					upper = max(upper, obj->upper);

					int counts[2] = { i - first + 1, last - i };
					float leftArea = EvaluateSurfaceArea(lower, upper);
					float rightArea = EvaluateSurfaceArea(rightLowers[j], rightUppers[j]);
					float costs = leftArea * counts[0] + rightArea * counts[1];
					if(costs < bestCosts)
					{
						bestCosts = costs; mid = i + 1; bestAxis = axis;
						bestLowers[0] = lower; bestLowers[1] = rightLowers[j];
						bestUppers[0] = upper; bestUppers[1] = rightUppers[j];
					}
				}
			}

			if(mid == -1)
			{
				n->MarkAsFinalLeaf();
				_nodeLock.Unlock(n);
				return Finalized;
			}

			if(bestAxis < 2) // don't need to sort by z-axis, because that was the last axis we sorted by
			{
				// sort object range [first;last] by axis
				std::sort(indices.begin() + first, indices.begin() + (last + 1),
					CompareObjectsByAxis<T>(_objGetter, bestAxis));
			}
		}
		else
#endif
		{
			float bestCosts = EvaluateSurfaceArea(n->lower, n->upper) * numberOfObjects;
			float bestSplit = 0.0f /* initialize to shut up the compiler */;

			// evaluate the SAH by taking a fixed number of samples ---------------
			const int SahSamples = 16;
			for(int axis = 0; axis < 3; ++axis)
			{
				float splits[SahSamples]; // calculate positions of candidate splitting planes
				float step = (n->upper[axis] - n->lower[axis]) * (1.0f / (SahSamples + 1));
				for(int i = 0; i < SahSamples; ++i) splits[i] = n->lower[axis] + (i + 1) * step;
		
				struct sample
				{
					float3 lowers[2];
					float3 uppers[2];
					int counts[2];
					float areas[2];
				} samples[SahSamples];

				// process the first object to initialize the samples structure
				{
					const typename ObjGetterType::Type &obj = _objGetter.Get(indices[first]);
					float centroid = obj.centroid[axis];
					for(int s = 0; s < SahSamples; ++s)
					{
						if(centroid > splits[s])
						{
							// object is on the RIGHT side of the splitting plane
							samples[s].lowers[0] = n->upper;
							samples[s].lowers[1] = obj.lower;
							samples[s].uppers[0] = n->lower;
							samples[s].uppers[1] = obj.upper;
							samples[s].counts[0] = 0;
							samples[s].counts[1] = 1;
						}
						else
						{
							// object is on the LEFT side of the splitting plane
							samples[s].lowers[0] = obj.lower;
							samples[s].lowers[1] = n->upper;
							samples[s].uppers[0] = obj.upper;
							samples[s].uppers[1] = n->lower;
							samples[s].counts[0] = 1;
							samples[s].counts[1] = 0;
						}
					}

				}

				// process the rest of the geometry
				for(int i = first + 1; i <= last; ++i)
				{
					const typename ObjGetterType::Type &obj = _objGetter.Get(indices[i]);
					float centroid = obj.centroid[axis];
					for(int s = 0; s < SahSamples; ++s)
					{
						int index = (centroid > splits[s]) ? 1 : 0; // determine side of plane, left or right?
						samples[s].lowers[index] = min(samples[s].lowers[index], obj.lower);
						samples[s].uppers[index] = max(samples[s].uppers[index], obj.upper);
						++samples[s].counts[index];
					}
				}

				for(int s = 0; s < SahSamples; ++s)
				{
					samples[s].areas[0] = EvaluateSurfaceArea(samples[s].lowers[0], samples[s].uppers[0]);
					samples[s].areas[1] = EvaluateSurfaceArea(samples[s].lowers[1], samples[s].uppers[1]);				
				}

#ifdef INTERPOLATE_SAMPLES
				for(int s = 0; s < SahSamples - 1; ++s)
				{
					float al0 = samples[s].areas[0], ar0 = samples[s].areas[1];
					float cl0 = samples[s].counts[0], cr0 = samples[s].counts[1];
					float al1 = samples[s + 1].areas[0], ar1 = samples[s + 1].areas[1];
					float cl1 = samples[s + 1].counts[0], cr1 = samples[s + 1].counts[1];
					double t = FindSahMinimum(al0, cl0, ar0, cr0, al1, cl1, ar1, cr1);
					float costs = lerp(cl0, cl1, t) * lerp(al0, al1, t) + lerp(cr0, cr1, t) * lerp(ar0, ar1, t);
					if(costs < bestCosts)
						{ bestCosts = costs; bestSplit = lerp(splits[s], splits[s+1], t); bestAxis = axis; }
				}
#else
				// pick the best candidate
				for(int s = 0; s < SahSamples; ++s)
				{
					const sample &samp = samples[s];
					if(samp.counts[0] == 0 || samp.counts[1] == 0) continue; // don't allow empty leaves
					float costs = samp.areas[0] * samp.counts[0] + samp.areas[1] * samp.counts[1];
					if(costs < bestCosts)
					{
						bestCosts = costs; bestSplit = splits[s]; bestAxis = axis;
						bestLowers[0] = samp.lowers[0]; bestLowers[1] = samp.lowers[1];
						bestUppers[0] = samp.uppers[0]; bestUppers[1] = samp.uppers[1];
					}
				}
#endif
			}

			if(bestAxis == -1)
			{
				n->MarkAsFinalLeaf();
				_nodeLock.Unlock(n);
				return Finalized;
			}

			mid = first; int right = last;
			while(mid <= right)
			{
				float centroid = _objGetter.Get(indices[mid]).centroid[bestAxis];
				if(centroid > bestSplit)
				{
					// move to the right half of the list by swapping with the untested centroid at [right]
					swap(indices[mid], indices[right]);
					--right;
				}
				else
					++mid; // keep on the left side
			}

#ifdef INTERPOLATE_SAMPLES
			if(mid <= first || mid > last)
			{
				n->MarkAsFinalLeaf();
				_nodeLock.Unlock(n);
				return Finalized;
			}

			// evaluate bounding boxes
			const typename ObjGetterType::Type *obj = &_objGetter.Get(indices[first]);
			bestLowers[0] = obj->lower; bestUppers[0] = obj->upper;
			for(int i = first + 1; i < mid; ++i)
			{
				obj = &_objGetter.Get(indices[i]);
				bestLowers[0] = min(bestLowers[0], obj->lower);
				bestUppers[0] = max(bestUppers[0], obj->upper);
			}

			obj = &_objGetter.Get(indices[last]);
			bestLowers[1] = obj->lower; bestUppers[1] = obj->upper;
			for(int i = mid; i < last; ++i)
			{
				obj = &_objGetter.Get(indices[i]);
				bestLowers[1] = min(bestLowers[1], obj->lower);
				bestUppers[1] = max(bestUppers[1], obj->upper);
			}
#endif
		}

		// split into intervals [first;mid[ and [mid;last] --------------------

		// allocate two nodes
		node *left = _nodes + Interlocked::ExchangeAdd(&_nextNode, 2);
		node *right = left + 1;

		// create left node
		left->lower = bestLowers[0]; left->upper = bestUppers[0];
		left->firstObj = first; left->lastObj = mid - 1;
		left->CreateLeaf(n, first >= (mid - 1) || depth >= MaxDepth, depth + 1);

		// create right node
		right->lower = bestLowers[1]; right->upper = bestUppers[1];
		right->firstObj = mid; right->lastObj = last;
		right->CreateLeaf(n, mid >= last || depth >= MaxDepth, depth + 1);

		// determine nearest node for all possible cases (signMask, zyx)
		int nearmask = 0; // reset
		nearmask |= length(left->lower - n->lower) < length(right->lower - n->lower) ? 0 : 1; // 000
		nearmask |= (bestAxis == 0) ? 2 : 0; // 001
		nearmask |= (bestAxis == 1) ? 4 : 0; // 010
		nearmask |= (bestAxis == 2) ? 0 : 8; // 011
		nearmask |= (bestAxis == 2) ? 16 : 0; // 100
		nearmask |= (bestAxis == 1) ? 0 : 32; // 101
		nearmask |= (bestAxis == 0) ? 0 : 64; // 110
		nearmask |= length(left->upper - n->upper) > length(right->upper - n->upper) ? 128 : 0; // 111

		// update the leaf indices for each object, we need them for refitting
		int leaf = left - _nodes;
		for(int i = first; i < mid; ++i)
			_objGetter.Get(indices[i]).leaf = leaf;
		leaf = right - _nodes;
		for(int i = mid; i <= last; ++i)
			_objGetter.Get(indices[i]).leaf = leaf;

		// store costs of current configuration
		float sa = EvaluateSurfaceArea(n->lower, n->upper);
		float lsa = EvaluateSurfaceArea(left->lower, left->upper);
		float rsa = EvaluateSurfaceArea(right->lower, right->upper);
		int leftCount = left->lastObj - left->firstObj + 1;
		int rightCount = right->lastObj - right->firstObj + 1;
		n->splitCosts = (lsa * leftCount + rsa * rightCount) / sa;

		// this marks the node as an inner node, has to be the last operation here, because other threads will interpret it as an inner node immediately
		n->TurnIntoInner(left, nearmask);

		_nodeLock.Unlock(n);
		return Split;
	}

	// Traversal ----------------------------------------------------------

	template<class ObjGetterType, int MaxDepth>
	inline bool Bvh<ObjGetterType, MaxDepth>::node::IntersectWith(const ray &r, const float3 &invDir) const
	{
		float nearDist = 0.0f, farDist = r.hit.distance;
		for(int axis = 0; axis < 3; ++axis)
		{
			float origin = r.origin[axis], direction = invDir[axis];
			float t0 = (lower[axis] - origin) * direction;
			float t1 = (upper[axis] - origin) * direction;
			nearDist = max(nearDist, min(t0, t1));
			farDist = min(farDist, max(t0, t1));
			// branch free code is perferable! if(nearDist > farDist) return false;
		}
		return (nearDist <= farDist);
	}

	template<class ObjGetterType, int MaxDepth> template<class LeafOpType>
	void Bvh<ObjGetterType, MaxDepth>::Traverse(ray &r, const float3 &invDir, int flags, unsigned int signMask, const LeafOpType &leafOp)
	{
		node *n = _nodes;
		if(n->IntersectWith(r, invDir))
		{
			int stackIdx = 0;
			node *nodes[MaxDepth];
			while(true)
			{
				if(n->IsLeaf())
				{
					if(n->CanSplit())
					{
						switch(SplitNode(n))
						{
						case Finalized:
							/* done, this is leaf */
							break;
						case Split:
							/* this has become an inner node */
#ifdef RECORDRAYSTATS
							++r.dynamicStats.splitNodes;
#endif
							goto HandleInnerNode;
						case InProgress:
							if(stackIdx > 0) // we have other nodes to visit: pop a node from the stack and resume here later on
								swap(n, nodes[stackIdx - 1]);
							continue; // try again in next iteration
						}
					}
					leafOp(r, invDir, n->firstObj, n->lastObj, flags);
				}
				else
				{
HandleInnerNode:
#ifdef RECORDRAYSTATS
					++r.dynamicStats.visitedInnerNodes;
#endif
					node *children = n->GetChildren(); // move pointer to children
					bool left = children[0].IntersectWith(r, invDir);
					bool right = children[1].IntersectWith(r, invDir);
					if(left)
					{
						if(right)
						{
							int nearIndex = n->GetNearIndex(signMask);
							nodes[stackIdx++] = children + (nearIndex ^ 1); // push far child onto the stack
							n = children + nearIndex; // continue traversal with near child
						}
						else
							n = children;
						continue;
					}
					else if(right)
						{ n = children + 1; continue; }
				}

				// we reach this point, if we're in need of a new node from the stack
				if(stackIdx > 0)
					n = nodes[--stackIdx];
				else
					break;
			}
		}
	}

	template<class ObjGetterType, int MaxDepth>
	inline unsigned int Bvh<ObjGetterType, MaxDepth>::node::IntersectWith(const packet &p) const
	{
		__m128 nearDist4 = Zero4, farDist4 = p.hit.distance.f4;
		for(int axis = 0; axis < 3; ++axis)
		{
			__m128 l = _mm_set1_ps(lower[axis]), u = _mm_set1_ps(upper[axis]);
			__m128 origin4 = p.origin[axis].f4, direction4 = p.invdir[axis].f4;
			__m128 t0 = _mm_mul_ps(_mm_sub_ps(l, origin4), direction4);
			__m128 t1 = _mm_mul_ps(_mm_sub_ps(u, origin4), direction4);
			nearDist4 = _mm_max_ps(nearDist4, _mm_min_ps(t0, t1));
			farDist4 = _mm_min_ps(farDist4, _mm_max_ps(t0, t1));
		}
		return _mm_movemask_ps(_mm_cmple_ps(nearDist4, farDist4));
	}

	template<class ObjGetterType, int MaxDepth> template<class LeafOpType>
	void Bvh<ObjGetterType, MaxDepth>::Traverse(packet &p, int flags, unsigned int signMask, const LeafOpType &leafOp)
	{
		node *n = _nodes;
		if(n->IntersectWith(p))
		{
			int stackIdx = 0;
			node *nodes[MaxDepth];
			while(true)
			{
				if(n->IsLeaf())
				{
					if(n->CanSplit())
					{
						switch(SplitNode(n))
						{
						case Finalized:
							/* done, this is leaf */
							break;
						case Split:
							/* this has become an inner node */
#ifdef RECORDRAYSTATS
							++p.dynamicStats.splitNodes;
#endif
							goto HandleInnerNode;
						case InProgress:
							if(stackIdx > 0) // we have other nodes to visit: pop a node from the stack and resume here later on
								swap(n, nodes[stackIdx - 1]);
							continue; // try again in next iteration
						}
					}
					leafOp(p, n->firstObj, n->lastObj, flags);
				}
				else
				{
HandleInnerNode:
#ifdef RECORDRAYSTATS
					++p.dynamicStats.visitedInnerNodes;
#endif
					node *children = n->GetChildren(); // move pointer to children
					unsigned int left = children[0].IntersectWith(p);
					unsigned int right = children[1].IntersectWith(p);
					if(left)
					{
						if(right)
						{
							int nearIndex = n->GetNearIndex(signMask);
							nodes[stackIdx++] = children + (nearIndex ^ 1); // push far child onto the stack
							n = children + nearIndex; // continue traversal with near child
						}
						else
							n = children;
						continue;
					}
					else if(right)
						{ n = children + 1; continue; }
				}

				// we reach this point, if we're in need of a new node from the stack
				if(stackIdx > 0)
					n = nodes[--stackIdx];
				else
					break;
			}
		}
	}

	#define LOWESTBITPOS2(x) (x & 0x1 ? 0 : 1)
	#define LOWESTBITPOS4(x) (x & 0x3 ? LOWESTBITPOS2(x) : 2 + LOWESTBITPOS2(x >> 2))
	#define LOWESTBITPOS8(x) (x & 0xF ? LOWESTBITPOS4(x) : 4 + LOWESTBITPOS4(x >> 4))
	#define LOWESTBITPOS16(x) (x & 0xFF ? LOWESTBITPOS8(x) : 8 + LOWESTBITPOS8(x >> 8))
	#define LOWESTBITPOS32(x) (x & 0xFFFF ? LOWESTBITPOS16(x) : 16 + LOWESTBITPOS16(x >> 16))

	#define HIGHESTBITPOS2(x) (x & 0x2 ? 1 : 0)
	#define HIGHESTBITPOS4(x) (x & 0xC ? 2 + HIGHESTBITPOS2(x >> 2) : HIGHESTBITPOS2(x))
	#define HIGHESTBITPOS8(x) (x & 0xF0 ? 4 + HIGHESTBITPOS4(x >> 4) : HIGHESTBITPOS4(x))
	#define HIGHESTBITPOS16(x) (x & 0xFF00 ? 8 + HIGHESTBITPOS8(x >> 8) : HIGHESTBITPOS8(x))
	#define HIGHESTBITPOS32(x) (x & 0xFFFF0000 ? 16 + HIGHESTBITPOS16(x >> 16) : HIGHESTBITPOS16(x))

	inline static void ExtractMaskBounds(int &first, int &last, unsigned int mask)
	{
		first = LOWESTBITPOS32(mask);
		last = HIGHESTBITPOS32(mask);
	}

	template<class ObjGetterType, int MaxDepth>
	inline unsigned int Bvh<ObjGetterType, MaxDepth>::node::IntersectWith(const packet packets[], int &first, int &last) const
	{
		if(IsLeaf())
		{
#ifdef NO_EXACT_LEAF_MASK
			while(first <= last)
			{
				if(IntersectWith(packets[first]))
				{
#if 1
					while(last > first)
					{
						if(IntersectWith(packets[last]))
							break;
						else
							--last;
					}
#endif
					return ((1 << (last - first + 1)) - 1) << first; // ray hits -> all following rays
				}
				else
					++first;
			}
			return 0;
#else
			// NOTE: this actually slows things down!
			unsigned int mask = 0;
			for(int i = first; i <= last; ++i)
				mask |= IntersectWith(packets[i]) ? (1 << i) : 0;
			ExtractMaskBounds(first, last, mask);
			return mask;
#endif // NO_EXACT_LEAF_MASK
		}
		else
		{
			while(first <= last)
			{
				if(IntersectWith(packets[first]))
					return ((1 << (last - first + 1)) - 1) << first; // ray hits -> all following rays
				else
					++first;
			}
			return 0;
		}
	}

	template<class ObjGetterType, int MaxDepth> template<class LeafOpType>
	void Bvh<ObjGetterType, MaxDepth>::Traverse(packet packets[], int count, int flags, unsigned int signMask, const LeafOpType &leafOp)
	{
		int first = 0, last = count - 1;
		node *n = _nodes;
		unsigned int mask = n->IntersectWith(packets, first, last);
		if(mask)
		{
			int stackIdx = 0;
			node *nodes[MaxDepth];
			unsigned int masks[MaxDepth];
			while(true)
			{
				if(n->IsLeaf())
				{
					if(n->CanSplit())
					{
						switch(SplitNode(n))
						{
						case Finalized:
							/* done, this is leaf */
							break;
						case Split:
							/* this has become an inner node */
#ifdef RECORDRAYSTATS
							for(int i = first; i <= last; ++i)
								++packets[i].dynamicStats.splitNodes;
#endif
							goto HandleInnerNode;
						case InProgress:
							if(stackIdx > 0) // we have other nodes to visit: pop a node from the stack and resume here later on
							{
								swap(n, nodes[stackIdx - 1]);
								swap(mask, masks[stackIdx - 1]);
								ExtractMaskBounds(first, last, mask);
							}
							continue; // try again in next iteration
						}
					}
					leafOp(packets, first, last, mask, n->firstObj, n->lastObj, flags);
				}
				else
				{
HandleInnerNode:
#ifdef RECORDRAYSTATS
					for(int i = first; i <= last; ++i)
						++packets[i].dynamicStats.visitedInnerNodes;
#endif
					node *children = n->GetChildren(); // move pointer to children

					int firsts[2] = { first, first }, lasts[2] = { last, last }; unsigned int cmasks[2];
					cmasks[0] = children[0].IntersectWith(packets, firsts[0], lasts[0]);
					cmasks[1] = children[1].IntersectWith(packets, firsts[1], lasts[1]);
					if(cmasks[0])
					{
						if(cmasks[1])
						{
							int nearIndex = n->GetNearIndex(signMask);
							masks[stackIdx] = cmasks[nearIndex ^ 1]; nodes[stackIdx++] = &children[nearIndex ^ 1];
							n = &children[nearIndex]; mask = cmasks[nearIndex]; first = firsts[nearIndex]; last = lasts[nearIndex];
						}
						else
							{ n = children; mask = cmasks[0]; first = firsts[0]; last = lasts[0]; }
						continue;
					}
					else if(cmasks[1])
						{  n = children + 1; mask = cmasks[1]; first = firsts[1]; last = lasts[1]; continue; }
				}

				// we reach this point, if we're in need of a new node from the stack
				if(stackIdx > 0)
					{ n = nodes[--stackIdx]; mask = masks[stackIdx]; ExtractMaskBounds(first, last, mask); }
				else
					break;
			}
		}
	}
} /* internal */ }

#endif // DYNAMICBVH_INL
