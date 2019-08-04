
#include "statickdtree.h"
#include "interlocked.h"
#include <assert.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <list>
#include <set>
#include "threadinghelper.h"
#include <vector>
using namespace std;

#ifdef __MSVC__
#include <hash_map>
#define HashMap stdext::hash_map
#else
#include <ext/hash_map>
#define HashMap __gnu_cxx::hash_map
#endif

namespace rapido
{
	#define ENABLEBACKTRACKING
	const int BacktrackingAttempts = 8;

	//#define RESTRICT_SURFACE_AREA
	const float MinSurfaceArea = 0.00001f; // in percent relative to whole scene aabb

	const float TraversalCosts = 1.0f;
	const float IntersectionCosts = 0.5f * (1.0f + sqrtf(5.0f)); // golden cut ;-)

	#define PROMOTEEMPTYSPLITS // uses CalcEmptySplitScale to scale costs of empty splits
	const float DepthBasedScaleBase = (float)exp(log(0.05) / StaticKdTree::MaxDepth); // 0.05 is the scale of depth == MaxKdtDepth
	inline static float GetDepthBasedScale(int depth) { return powf(DepthBasedScaleBase, depth); }
	inline static float CalcEmptySplitScale(float emptyPercentage, int depth) { return 1.0f - emptyPercentage * GetDepthBasedScale(depth); }

	inline static bool IsSplitBeneficial(float splitCosts, float leafCosts, int depth) { return splitCosts < leafCosts * GetDepthBasedScale(depth); }
	inline static float CalcInnerNodeCosts(float sa, float leftCosts, float rightCosts) { return sa * TraversalCosts + leftCosts + rightCosts; }
	inline static float CalcLeafCosts(float sa, int numberOfTris) { return sa * IntersectionCosts * numberOfTris; }

	enum PlaneSide
	{
		LeftPlaneSideOnly = 1, RightPlaneSideOnly
	};

	struct triangle
	{
		float3 points[3];

		inline triangle() { }
		inline triangle(const float3 &a, const float3 &b, const float3 &c) { points[0] = a; points[1] = b; points[2] = c; }
	};

	struct Event
	{
		enum EventType
		{
			EndEventType = 0, PlanarEventType, StartEventType
		} type;

		float pos;
		int tri;

		inline Event(float pos, EventType type, int tri)
			: type(type), pos(pos), tri(tri) { }

		inline bool operator <(const Event &other) const
		{
			return (pos == other.pos) ? (type < other.type) : (pos < other.pos);
		}
	};

	struct SplittingPlane
	{
		int axis;
		float pos, costs;
		bool planarsGoLeft;
	};

	union fatnode
	{
		struct
		{
			int numberOfIndices; // number of triangle indices
			int offsetToIndices; // offset to array of integers
			vector<int> *indices; // storage for triangle indices
		} leaf;

		struct
		{
			int offsetAndAxis; // 0..2 axis, 2..30 offset to children, 31 node marker
			float splitPos;	// position of the splitting plane
		} inner;

		inline bool IsLeaf() const { return (inner.offsetAndAxis & (1 << 31)) == 0; }

		inline void MakeLeaf(const vector<int> &indices, volatile long *numberOfIndices)
		{
			if(indices.size() > 0)
			{
				leaf.numberOfIndices = indices.size();
				leaf.indices = new vector<int>(indices);
				Interlocked::ExchangeAdd(numberOfIndices, leaf.numberOfIndices);
			}
			else
				leaf.offsetToIndices = 0;
		}

		inline void ClearLeaf(volatile long *numberOfIndices)
		{
			if(leaf.numberOfIndices > 0)
			{
				delete leaf.indices;
				Interlocked::ExchangeAdd(numberOfIndices, -leaf.numberOfIndices);
			}
		}
	};

	inline static float EvaluateSurfaceArea(const float3 &bbMin, const float3 &bbMax)
	{
		float3 delta = bbMax - bbMin;
		return /*2.0 */(delta.x * delta.y + (delta.x + delta.y) * delta.z);
	}

	static void EvaluateSah(float r12, const float3 &lower, const float3 &upper, int leftCount, int planarCount, int rightCount, SplittingPlane &plane, int depth)
	{
		float3 lowerMid = lower; lowerMid[plane.axis] = plane.pos;
		float3 upperMid = upper; upperMid[plane.axis] = plane.pos;
		float r1 = EvaluateSurfaceArea(lower, upperMid); // probability of a ray traversing the left node
		float r2 = EvaluateSurfaceArea(lowerMid, upper); // probability of a ray traversing the right node

#ifdef PROMOTEEMPTYSPLITS
		float split = (plane.pos - lower[plane.axis]) / (upper[plane.axis] - lower[plane.axis]);
		if((leftCount == 0 || rightCount == 0) && planarCount > 0)
		{
			// find out where to put the planars so that we get a minimal increase of the overall costs
			float c1Left = CalcLeafCosts(r1, leftCount + planarCount); // left node's costs
			float c2Left = CalcLeafCosts(r2, rightCount); // right node's costs
			float planarsLeftCosts = CalcInnerNodeCosts(r12, c1Left, c2Left); // calculate costs of splitting the node with planars in the left child
			if(rightCount == 0) planarsLeftCosts *= CalcEmptySplitScale(1.0f - split, depth);

			float c1Right = CalcLeafCosts(r1, leftCount); // left node's costs
			float c2Right = CalcLeafCosts(r2, rightCount + planarCount); // right node's costs
			float planarsRightCosts = CalcInnerNodeCosts(r12, c1Right, c2Right); // calculate costs of splitting the node with planars in the right child
			if(leftCount == 0) planarsRightCosts *= CalcEmptySplitScale(split, depth);

			plane.planarsGoLeft = (planarsLeftCosts < planarsRightCosts);
			plane.costs = plane.planarsGoLeft ? planarsLeftCosts : planarsRightCosts;
			return;
		}
#endif

		plane.planarsGoLeft = (r1 < r2); // put planars where they result in a minimal increase of the overall costs
		int t1 = leftCount + (plane.planarsGoLeft ? planarCount : 0);
		int t2 = rightCount + (plane.planarsGoLeft ? 0 : planarCount);

		float c1 = CalcLeafCosts(r1, t1); // left node's costs
		float c2 = CalcLeafCosts(r2, t2); // right node's costs
		plane.costs = CalcInnerNodeCosts(r12, c1, c2); // calculate costs of splitting the node

#ifdef PROMOTEEMPTYSPLITS
		if(t1 == 0)
			plane.costs *= CalcEmptySplitScale(split, depth);
		else if(t2 == 0)
			plane.costs *= CalcEmptySplitScale(1.0f - split, depth);
#endif
	}

	static bool FindSplittingPlane(SplittingPlane &bestPlane, const float3 &lower, const float3 &upper, int numberOfTriangles, list<Event> events[], int depth)
	{
		float r12 = EvaluateSurfaceArea(lower, upper); // probability of a ray traversing the node
		float3 mid = (lower + upper) * 0.5f;
		SplittingPlane bestPlanes[3];

		// NOTE: cannot use parallel for because it interferes with boost's threads and causes a deadlock
		// #pragma omp parallel for
		for(int axis = 0; axis < 3; ++axis)
		{
			bestPlanes[axis].costs = Infinity;
			bestPlanes[axis].axis = -1;

			int leftCount = 0, planarCount = 0, rightCount = numberOfTriangles;
			list<Event>::const_iterator evnt = events[axis].begin(), end = events[axis].end();
			while(evnt != end)
			{
				SplittingPlane plane = { axis, evnt->pos, false };
			
				int pend = 0, pplanar = 0, pstart = 0; // count end/planar/start events in current plane
				while(evnt != end && evnt->pos == plane.pos && evnt->type == Event::EndEventType) { ++pend; ++evnt; }
				while(evnt != end && evnt->pos == plane.pos && evnt->type == Event::PlanarEventType) { ++pplanar; ++evnt; }
				while(evnt != end && evnt->pos == plane.pos && evnt->type == Event::StartEventType) { ++pstart; ++evnt; }

				// move onto current plane
				planarCount = pplanar; rightCount -= pplanar + pend;
				
				EvaluateSah(r12, lower, upper, leftCount, planarCount, rightCount, plane, depth);
				if(plane.costs < bestPlanes[axis].costs || (plane.costs == bestPlanes[axis].costs &&
					fabs(mid[axis] - plane.pos) < fabs(mid[axis] - bestPlanes[axis].pos) /* when in doubt prefer balanced split */))
				{
					bestPlanes[axis] = plane;
				}

				// move over current plane
				/*planarCount = 0;*/ leftCount += pplanar + pstart;
			}
		}

		// merge results from separate threads
		bestPlane = bestPlanes[0];
		for(int i = 1; i < 3; ++i)
		{
			if(bestPlanes[i].costs < bestPlane.costs || (bestPlanes[i].costs == bestPlane.costs &&
					fabs(mid[i] - bestPlanes[i].pos) < fabs(mid[i] - bestPlane.pos) /* when in doubt prefer balanced split */))
			{
				bestPlane = bestPlanes[i];
			}
		}
		return (bestPlane.axis != -1);
	}

	static HashMap<int, PlaneSide> ClassifyTriangles(int numberOfTriangles, const list<Event> events[], const SplittingPlane &bestPlane)
	{
		HashMap<int, PlaneSide> sides;
		for(list<Event>::const_iterator evnt = events[bestPlane.axis].begin(); evnt != events[bestPlane.axis].end(); ++evnt)
		{
			switch(evnt->type)
			{
			case Event::EndEventType:
				if(evnt->pos <= bestPlane.pos) sides[evnt->tri] = LeftPlaneSideOnly;
				break;
			case Event::StartEventType:
				if(bestPlane.pos <= evnt->pos) sides[evnt->tri] = RightPlaneSideOnly;
				break;
			case Event::PlanarEventType:
				if(evnt->pos < bestPlane.pos) sides[evnt->tri] = LeftPlaneSideOnly;
				else if(bestPlane.pos < evnt->pos) sides[evnt->tri] = RightPlaneSideOnly;
				else sides[evnt->tri] = bestPlane.planarsGoLeft ? LeftPlaneSideOnly : RightPlaneSideOnly;
				break;
			}
		}
		return sides;
	}

	static int ClipToPlane(int numberOfPoints, const float3 srcPoints[], float3 destPoints[], int axis, float pos, bool lower)
	{
		int numClippedPoints = 0;
		for(int i = 0; i < numberOfPoints; ++i)
		{
			int j = (i + 1) % numberOfPoints;
			float di = pos - srcPoints[i][axis], dj = pos - srcPoints[j][axis];
			if(lower) { di = -di; dj = -dj; }
			if(di >= 0.0f)
			{
				destPoints[numClippedPoints++] = srcPoints[i];
				if(!(dj >= 0.0f))
					destPoints[numClippedPoints++] = lerp(srcPoints[i], srcPoints[j], di / (di - dj));
			}
			else
			{
				if(dj >= 0.0f)
					destPoints[numClippedPoints++] = lerp(srcPoints[j], srcPoints[i], dj / (dj - di));
			}
		}
		return numClippedPoints;
	}

	static bool GenerateEventsWithClipping(list<Event> events[], const vector<triangle> &triangles, int index, const float3 &lower, const float3 &upper)
	{
		const int MaxPoints = 24;
		float3 bank0[MaxPoints] = { triangles[index].points[0], triangles[index].points[1], triangles[index].points[2] }, bank1[MaxPoints];

		// clip to one plane of the aabb at a time
		int numberOfPoints = 3;
		for(int i = 0; i < 3; ++i)
		{
			numberOfPoints = ClipToPlane(numberOfPoints, bank0, bank1, i, lower[i], true);
			numberOfPoints = ClipToPlane(numberOfPoints, bank1, bank0, i, upper[i], false);
		}

		// clipping triangles away due to numerical imprecisions shouldn't happen
		if(numberOfPoints < 3)
			return false;

		float3 triLower = bank0[0], triUpper = bank0[0];
		for(int i = 1; i < numberOfPoints; ++i)
		{
			triLower = min(triLower, bank0[i]);
			triUpper = max(triUpper, bank0[i]);
		}

		for(int i = 0; i < 3; ++i)
		{
			if(triLower[i] < triUpper[i])
			{
				events[i].push_back(Event(triLower[i], Event::StartEventType, index));
				events[i].push_back(Event(triUpper[i], Event::EndEventType, index));
			}
			else
				events[i].push_back(Event(triLower[i], Event::PlanarEventType, index));
		}

		return true;
	}	

	static void SplitTriangles(vector<int> &leftIndices, vector<int> &rightIndices, list<Event> events[], list<Event> rightEvents[],
		const vector<triangle> &triangles, const vector<int> &indices, const float3 &lower, const float3 &upper, const SplittingPlane &plane)
	{
		// classify triangles based on events, triangles that are on both sides are not in the map!
		HashMap<int, PlaneSide> sides = ClassifyTriangles(indices.size(), events, plane);

		// split triangles into two subsets and remember triangles that need clipping
		vector<int> clipIndices;
		for(vector<int>::const_iterator it = indices.begin(); it != indices.end(); ++it)
		{
			int index = *it;
			HashMap<int, PlaneSide>::const_iterator it = sides.find(index);
			if(it != sides.end())
			{
				switch(it->second)
				{
				case LeftPlaneSideOnly: leftIndices.push_back(index); break;
				case RightPlaneSideOnly: rightIndices.push_back(index); break;
				}
			}
			else
				clipIndices.push_back(index);
		}

		// split events into two subsets and remove events that are obsolete and will be replaced during clipping
		// NOTE: cannot use parallel for because it interferes with boost's threads and causes a deadlock
		// #pragma omp parallel for
		for(int axis = 0; axis < 3; ++axis)
		{
			list<Event>::iterator evnt = events[axis].begin();
			while(evnt != events[axis].end())
			{
				HashMap<int, PlaneSide>::const_iterator it = sides.find(evnt->tri);
				if(it != sides.end())
				{
					switch(it->second)
					{
					case LeftPlaneSideOnly: ++evnt; /* keep on the left side */ break;
					case RightPlaneSideOnly: rightEvents[axis].splice(rightEvents[axis].end(), events[axis], evnt++); break;
					}
				}
				else
					events[axis].erase(evnt++); // remove it, the event will be generated for both sides
			}
		}

		// generate new events for triangles that straddle the splitting plane
		list<Event> newLeftEvents[3], newRightEvents[3];
		float3 lupper = upper; lupper[plane.axis] = plane.pos;
		float3 rlower = lower; rlower[plane.axis] = plane.pos;
		for(vector<int>::const_iterator it = clipIndices.begin(); it != clipIndices.end(); ++it)
		{
			int index = *it;
			if(GenerateEventsWithClipping(newLeftEvents, triangles, index, lower, lupper))
				leftIndices.push_back(index);
			if(GenerateEventsWithClipping(newRightEvents, triangles, index, rlower, upper))
				rightIndices.push_back(index);
		}

		// merge event lists to a single sorted list
		for(int axis = 0; axis < 3; ++axis)
		{
			newLeftEvents[axis].sort();
			events[axis].merge(newLeftEvents[axis]);
			newRightEvents[axis].sort();
			rightEvents[axis].merge(newRightEvents[axis]);
		}
	}

	unsigned int StaticKdTreeBuilder::CalculateMemoryLength(int numberOfTriangles, int numberOfIndices, int numberOfNodes)
	{	
		unsigned int memoryLength = 0;

		// triangles
		unsigned int length = numberOfTriangles * sizeof(statictriangle);
		unsigned int padding = (StaticKdTree::CacheLineLength - length % StaticKdTree::CacheLineLength);
		if(padding == StaticKdTree::CacheLineLength) padding = 0;
		memoryLength += length + padding;

		// indices
		length = numberOfIndices * sizeof(int);
		padding = (StaticKdTree::CacheLineLength - length % StaticKdTree::CacheLineLength);
		if(padding == StaticKdTree::CacheLineLength) padding = 0;
		memoryLength += length + padding;

		// nodes
		// NOTE: offset start of nodes by one node yielding |pad|root|left|right|
		memoryLength += (numberOfNodes + 1) * sizeof(StaticKdTree::node);

		return memoryLength;
	}

	StaticKdTree StaticKdTreeBuilder::BuildFrom(const float3 vertices[], int numberOfVertices, const int triangleList[], int numberOfTriangles)
	{
		if(numberOfTriangles == 0)
			return StaticKdTree();
		assert(vertices != 0 && triangleList != 0);

		// generate the triangle objects array
		vector<triangle> triangles(numberOfTriangles);
		#pragma omp parallel for
		for(int i = 0; i < numberOfTriangles; ++i)
		{
			int a = triangleList[3 * i], b = triangleList[3 * i + 1], c = triangleList[3 * i + 2];
			if(a < 0 || a >= numberOfVertices || b < 0 || b >= numberOfVertices || b < 0 || b >= numberOfVertices)
				throw "Invalid vertex index!";
			triangles[i] = triangle(vertices[a], vertices[b], vertices[c]);
		}

		return BuildInternal(&triangles);
	}

	StaticKdTree StaticKdTreeBuilder::BuildFrom(const float3 vertexList[], int numberOfTriangles)
	{
		if(numberOfTriangles == 0)
			return StaticKdTree();
	
		assert(vertexList != 0);

		// generate the triangle objects array
		vector<triangle> triangles(numberOfTriangles);
		#pragma omp parallel for
		for(int i = 0; i < numberOfTriangles; ++i)
			triangles[i] = triangle(vertexList[3 * i], vertexList[3 * i + 1], vertexList[3 * i + 2]);

		return BuildInternal(&triangles);
	}

	class BuildJob
	{
		mutable volatile long _threadsLeft; // counter is shared across threads, use interlocked operations!
		size_t _nodeSize;
		const vector<triangle> &_triangles;
		list<fatnode> _kdtree; // cannot use vector, because pointers would become invalid on resize
		mutable volatile long _numberOfIndices; // counter is shared across threads, use interlocked operations!
		float3 _sceneMin, _sceneMax;
		int _numberOfTriangles;
#ifdef RESTRICT_SURFACE_AREA
		float _lowerSurfaceAreaBound;
#endif
		void DetermineSceneBoundsAndGenerateEvents(list<Event> events[])
		{
			#pragma omp parallel for
			for(int axis = 0; axis < 3; ++axis)
			{
				_sceneMin[axis] = _sceneMax[axis] = _triangles[0].points[0][axis];
				for(int i = 1; i < _numberOfTriangles; ++i)
				{
					const triangle &tri = _triangles[i];
					float lower = min(tri.points[0][axis], min(tri.points[1][axis], tri.points[2][axis]));
					float upper = max(tri.points[0][axis], max(tri.points[1][axis], tri.points[2][axis]));
					if(lower < upper)
					{
						events[axis].push_back(Event(lower, Event::StartEventType, i));
						events[axis].push_back(Event(upper, Event::EndEventType, i));
					}
					else
						events[axis].push_back(Event(lower, Event::PlanarEventType, i));

					_sceneMin[axis] = min(_sceneMin[axis], lower);
					_sceneMax[axis] = max(_sceneMax[axis], upper);
				}
				events[axis].sort();
			}
		}

		struct bounds { float3 lower, upper; };

		void ProcessNode(float *costs, list<fatnode> *tree, fatnode *node, const bounds &b,
			vector<int> &indices, list<Event> events[], int depth = 0
#ifdef ENABLEBACKTRACKING
			, int backtrackingAttempts = BacktrackingAttempts
#endif
			) const
		{
			// decide whether to turn the current configuration into a leaf or to keep splitting
			float r12 = EvaluateSurfaceArea(b.lower, b.upper); // probability of a ray traversing the node
			*costs = CalcLeafCosts(r12, indices.size()); // costs of not splitting this node

			SplittingPlane plane;
			if(indices.size() == 0 || depth >= StaticKdTree::MaxDepth ||
#ifdef RESTRICT_SURFACE_AREA
				r12 <= _lowerSurfaceAreaBound ||
#endif
				!FindSplittingPlane(plane, b.lower, b.upper, indices.size(), events, depth))
			{
				node->MakeLeaf(indices, &_numberOfIndices);
				return;
			}
#ifdef ENABLEBACKTRACKING
			bool backtrackingPoint = false;
			if(!IsSplitBeneficial(plane.costs, *costs, depth))
			{
				if(backtrackingAttempts > 0)
				{
					--backtrackingAttempts;
					backtrackingPoint = true;
				}
				else
				{
					node->MakeLeaf(indices, &_numberOfIndices);
					return;
				}
			}
#else
			if(!IsSplitBeneficial(plane.costs, *costs, depth))
			{
				node->MakeLeaf(indices, &_numberOfIndices);
				return;
			}
#endif
			vector<int> leftIndices, rightIndices;
			list<Event> rightEvents[3];
			SplitTriangles(leftIndices, rightIndices, events, rightEvents, _triangles, indices, b.lower, b.upper, plane);
#ifdef ENABLEBACKTRACKING
			if(!backtrackingPoint)
				indices.clear(); // free some memory, if we definitely do not need it anymore
#else
			indices.clear(); // free some memory
#endif
			node->inner.offsetAndAxis = (1 << 31) | static_cast<int>(plane.axis); // offset to children is set by caller of this function
			node->inner.splitPos = plane.pos;

			list<fatnode> leftTree, rightTree;
			float leftCosts = 0.0f, rightCosts = 0.0f;
			bounds leftBounds = b; leftBounds.upper[plane.axis] = plane.pos;
			bounds rightBounds = b; rightBounds.lower[plane.axis] = plane.pos;
			tree->push_back(fatnode()); fatnode *leftNode = &*tree->rbegin();
			tree->push_back(fatnode()); fatnode *rightNode = &*tree->rbegin();

			// try to spawn a new thread
			bool recursed = false;
			if(_threadsLeft > 0)
			{
				if(Interlocked::ExchangeAdd(&_threadsLeft, -1) > 0)
				{
#ifdef ENABLEBACKTRACKING
					boost::thread lt(boost::bind(&BuildJob::ProcessNode, this, &leftCosts, &leftTree, leftNode, leftBounds, leftIndices, events, depth + 1, backtrackingAttempts));
					ProcessNode(&rightCosts, &rightTree, rightNode, rightBounds, rightIndices, rightEvents, depth + 1, backtrackingAttempts);
#else
					boost::thread lt(boost::bind(&BuildJob::ProcessNode, this, &leftCosts, &leftTree, leftNode, leftBounds, leftIndices, events, depth + 1));
					ProcessNode(&rightCosts, &rightTree, rightNode, rightBounds, rightIndices, rightEvents, depth + 1);
#endif
					lt.join();
					recursed = true;
				}
				Interlocked::ExchangeAdd(&_threadsLeft, 1);
			}
			
			// out of threads, recurse normally
			if(!recursed)
			{
#ifdef ENABLEBACKTRACKING
				ProcessNode(&leftCosts, &leftTree, leftNode, leftBounds, leftIndices, events, depth + 1, backtrackingAttempts);
				ProcessNode(&rightCosts, &rightTree, rightNode, rightBounds, rightIndices, rightEvents, depth + 1, backtrackingAttempts);
#else
				ProcessNode(&leftCosts, &leftTree, leftNode, leftBounds, leftIndices, events, depth + 1);
				ProcessNode(&rightCosts, &rightTree, rightNode, rightBounds, rightIndices, rightEvents, depth + 1);
#endif
			}

			if(!leftNode->IsLeaf())
				leftNode->inner.offsetAndAxis |= 2 * _nodeSize; // hop over the left and right node
			if(!rightNode->IsLeaf())
				rightNode->inner.offsetAndAxis |= _nodeSize * (leftTree.size() + 1); // hop over right node plus left subtree

			float innerNodeCosts = CalcInnerNodeCosts(r12, leftCosts, rightCosts);
#ifdef ENABLEBACKTRACKING
			if(backtrackingPoint && innerNodeCosts > *costs)
			{
				// clean up the left and right subtree
				if(rightNode->IsLeaf()) rightNode->ClearLeaf(&_numberOfIndices); tree->pop_back();
				if(leftNode->IsLeaf()) leftNode->ClearLeaf(&_numberOfIndices); tree->pop_back();
				for(list<fatnode>::iterator it = leftTree.begin(); it != leftTree.end(); ++it)
					{ if(it->IsLeaf()) it->ClearLeaf(&_numberOfIndices); }
				for(list<fatnode>::iterator it = rightTree.begin(); it != rightTree.end(); ++it)
					{ if(it->IsLeaf()) it->ClearLeaf(&_numberOfIndices); }
			
				// build a leaf node
				node->MakeLeaf(indices, &_numberOfIndices);
				return;
			}
#endif
			*costs = innerNodeCosts;
			tree->splice(tree->end(), leftTree);
			tree->splice(tree->end(), rightTree);
		}

	public:
		BuildJob(const vector<triangle> &triangles, size_t nodeSize) :
			_threadsLeft(GetNumberOfProcessors()), _nodeSize(nodeSize),
			_triangles(triangles), _numberOfIndices(0), _numberOfTriangles(triangles.size())
		{
			assert(_numberOfTriangles > 0);

			// create the root node
			_kdtree.push_back(fatnode());
			fatnode *root = &*_kdtree.rbegin();

			// create the array of indices
			vector<int> indices(_numberOfTriangles);
			for(int i = 0, e = _numberOfTriangles; i < e; ++i)
				indices[i] = i;

			// generate sorted event lists for each axis and find bounding box of the scene
			list<Event> events[3];
			DetermineSceneBoundsAndGenerateEvents(events);
#ifdef RESTRICT_SURFACE_AREA
			_lowerSurfaceAreaBound = MinSurfaceArea * EvaluateSurfaceArea(_sceneMin, _sceneMax);
#endif
			bounds sceneBounds = { _sceneMin, _sceneMax };
			float costs; ProcessNode(&costs, &_kdtree, root, sceneBounds, indices, events);
			if(!root->IsLeaf()) // the offset from the root to its children is exactly one node
				root->inner.offsetAndAxis |= nodeSize;
		}

		inline list<fatnode> &GetKdTree() { return _kdtree; }
		inline const float3 &GetSceneMin() const { return _sceneMin; }
		inline const float3 &GetSceneMax() const { return _sceneMax; }
		inline int GetNumberOfIndices() const { return static_cast<int>(_numberOfIndices); }
	};

	StaticKdTree StaticKdTreeBuilder::BuildInternal(void *trianglesPtr)
	{
		assert(trianglesPtr != 0);

		const vector<triangle> &triangles = *reinterpret_cast<const vector<triangle> *>(trianglesPtr);

		BuildJob job(triangles, sizeof(StaticKdTree::node));
		list<fatnode> &kdtree = job.GetKdTree();
		const float3 &sceneMin = job.GetSceneMin();
		const float3 &sceneMax = job.GetSceneMax();

		int numberOfTriangles = triangles.size();
		int numberOfIndices = job.GetNumberOfIndices();
		int numberOfNodes = kdtree.size();

		// allocate memory and pointers for the optimized memory representation
		unsigned int memoryLength = CalculateMemoryLength(numberOfTriangles, numberOfIndices, numberOfNodes);
		char *memory = reinterpret_cast<char *>(sse_alloc(memoryLength));
		try
		{
			char *cur = memory;

			statictriangle *tris = reinterpret_cast<statictriangle *>(cur);
			unsigned int length = numberOfTriangles * sizeof(statictriangle);
			unsigned int padding = (StaticKdTree::CacheLineLength - length % StaticKdTree::CacheLineLength);
			if(padding == StaticKdTree::CacheLineLength) padding = 0;
			cur += length + padding;

			int *indices = reinterpret_cast<int *>(cur);
			length = numberOfIndices * sizeof(int);
			padding = (StaticKdTree::CacheLineLength - length % StaticKdTree::CacheLineLength);
			if(padding == StaticKdTree::CacheLineLength) padding = 0;
			cur += length + padding;

			// NOTE: offset start of nodes by one node yielding |pad|root|left|right|
			StaticKdTree::node *nodes = reinterpret_cast<StaticKdTree::node *>(cur) + 1;

			// write triangles
			#pragma omp parallel for
			for(int i = 0; i < numberOfTriangles; ++i)
			{
				statictriangle &dest = tris[i];
				const triangle &tri = triangles[i];
				// TODO: could optimize for precision by selecting best p0
				dest.p0 = tri.points[0];
				dest.e0 = tri.points[1] - dest.p0;
				dest.e1 = dest.p0 - tri.points[2];
				dest.n = cross(dest.e0, dest.e1); // must not normalize for fast ray/tri test
			}

			// write indices and nodes
			// TODO: should probably use some sort of index list compression here
			for(list<fatnode>::const_iterator it = kdtree.begin(); it != kdtree.end(); ++it, ++nodes)
			{
				if(it->IsLeaf())
				{
					if(it->leaf.numberOfIndices > 0)
					{
						nodes->leaf.numberOfIndices = it->leaf.numberOfIndices;
						nodes->leaf.offsetToIndices = reinterpret_cast<char *>(indices) - reinterpret_cast<char *>(nodes);
						for(vector<int>::iterator idx = it->leaf.indices->begin(); idx != it->leaf.indices->end(); ++idx)
							*indices++ = *idx;
						delete it->leaf.indices;
					}
					else
					{
						nodes->leaf.numberOfIndices = 0;
						nodes->leaf.offsetToIndices = 0;
					}
				}
				else
				{
					nodes->inner.offsetAndAxis = it->inner.offsetAndAxis;
					nodes->inner.splitPos = it->inner.splitPos;
				}
			}
		
			return StaticKdTree(sceneMin, sceneMax, numberOfTriangles, numberOfIndices, numberOfNodes, memory, memoryLength);
		}
		catch(...)
		{
			sse_free(memory);
			throw;
		}
	}

	inline static bool AreTrianglesPlanar(const statictriangle *a, const statictriangle *b)
	{
		const float Epsilon = 0.001f;
		float3 an = normalize(a->n);
		return (dot(an, normalize(b->n)) >= 1.0f - Epsilon) &&
			(dot(an, b->p0 - a->p0) < Epsilon);
	} 

	int StaticKdTreeBuilder::OffsetDecals(StaticKdTree &kdtree, isdecalfunc *func, float offset)
	{
		assert(func != 0);

		set<int> planars;
		for(const StaticKdTree::node *n = kdtree._nodes, *end = kdtree._nodes + kdtree._numberOfNodes; n < end; ++n)
		{
			if(n->IsLeaf())
			{
				const int *indices = n->leaf.GetIndices();
				for(int i = 0; i < n->leaf.numberOfIndices; ++i)
				{
					// check if triangle indices[i] lies in the same plane with any of the other triangles
					const statictriangle *me = kdtree._triangles + indices[i];
					for(int j = 0; j < n->leaf.numberOfIndices; ++j)
					{
						if(AreTrianglesPlanar(me, kdtree._triangles + indices[j]))
						{
							planars.insert(indices[i]);
							planars.insert(indices[j]);
						}
					}
				}
			}
		}

		int count = 0;
		for(set<int>::iterator it = planars.begin(); it != planars.end(); ++it)
		{
			if(func(*it))
			{
				statictriangle *tri = const_cast<statictriangle *>(&kdtree._triangles[*it]);
				tri->p0 = tri->p0 + normalize(tri->n) * offset;
				++count;
			}
		}
		return count;
	}
}

