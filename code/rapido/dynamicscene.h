
#ifndef DYNAMICSCENE_H
#define DYNAMICSCENE_H

#include "rapido.h"
#include "matrix.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <assert.h>

namespace rapido
{
	template<template<class ObjGetterType, int MaxDepth> class HierarchyType, class ObjectType>
	class DynamicScene
	{
	public:
		class Geometry
		{
		public:
			typedef int IndexType;

		private:
			friend class DynamicScene;
			friend class DynamicScene::TraversalLeafOp;

			typedef ObjectType triangle;

			class TriangleGetter
			{
				std::vector<triangle> &_triangles;

			public:
				typedef triangle Type;

				inline TriangleGetter(std::vector<triangle> &triangles) : _triangles(triangles)	{ }
				inline triangle &Get(int index) const { assert(index >= 0 && index < static_cast<int>(_triangles.size())); return _triangles[index]; }
				inline int GetSize() const { return _triangles.size(); }
			};

			class TraversalLeafOp
			{
				const IndexType * const _triangleList;
				const float3 * const _vertices;
				const std::vector<int> &_indices;
				const int _hitMarker;

				inline void IntersectWithTriangle(ray &r, int triId) const
				{
					const IndexType *indices = _triangleList + 3 * triId;
					const float3 &p0 = _vertices[indices[0]], &p1 = _vertices[indices[1]], &p2 = _vertices[indices[2]];
					float3 e0 = p1 - p0, e1 = p0 - p2;
					float3 n = cross(e0, e1);

					float v = dot(n, r.direction);
					float r_ = 1.0f / v;
					float3 e2 = p0 - r.origin;
					float va = dot(n, e2);
					float t = r_ * va;
					bool m = (t < r.hit.distance) && (t > 0.0f);
					if(!m)
						return;

					float3 i = cross(e2, r.direction);
					float v1 = dot(i, e1);
					float beta = r_ * v1;
					m &= (beta >= 0.0f);
					if(!m)
						return;

					float v2 = dot(i, e0);
					m &= ((v1 + v2) * v) <= (v * v);
					float gamma = r_ * v2;
					m &= (gamma >= 0.0f);
					if(!m)
						return;

					r.hit.triId = _hitMarker | triId;
					r.hit.distance = t;
					r.hit.beta = beta;
					r.hit.gamma = gamma;		
				}

				inline void IntersectWithTriangle(packet &p, int triId) const
				{
					const IndexType *indices = _triangleList + 3 * triId;
					const float3 &p0 = _vertices[indices[0]], &p1 = _vertices[indices[1]], &p2 = _vertices[indices[2]];

					// float3 e0 = p1 - p0, e1 = p0 - p2;
					__m128 p0x = _mm_set1_ps(p0.x), p0y = _mm_set1_ps(p0.y), p0z = _mm_set1_ps(p0.z);
					__m128 e0x = _mm_sub_ps(_mm_set1_ps(p1.x), p0x), e0y = _mm_sub_ps(_mm_set1_ps(p1.y), p0y), e0z = _mm_sub_ps(_mm_set1_ps(p1.z), p0z);
					__m128 e1x = _mm_sub_ps(p0x, _mm_set1_ps(p2.x)), e1y = _mm_sub_ps(p0y, _mm_set1_ps(p2.y)), e1z = _mm_sub_ps(p0z, _mm_set1_ps(p2.z));

					// n = cross(e0, e1)
					__m128 nx = _mm_sub_ps(_mm_mul_ps(e0y, e1z), _mm_mul_ps(e0z, e1y));
					__m128 ny = _mm_sub_ps(_mm_mul_ps(e0z, e1x), _mm_mul_ps(e0x, e1z));
					__m128 nz = _mm_sub_ps(_mm_mul_ps(e0x, e1y), _mm_mul_ps(e0y, e1x));

					// float v = dot(norm, r->direction);
					__m128 v = _mm_mul_ps(nx, p.direction.x.f4);
					v = _mm_add_ps(_mm_mul_ps(ny, p.direction.y.f4), v);
					v = _mm_add_ps(_mm_mul_ps(nz, p.direction.z.f4), v);

					// float r_ = 1.0f / v;
					__m128 r_ = _mm_inverse_ps(v);

					// float3 e2 = p0 - r->origin;
					__m128 e2x = _mm_sub_ps(p0x, p.origin.x.f4);
					__m128 e2y = _mm_sub_ps(p0y, p.origin.y.f4);
					__m128 e2z = _mm_sub_ps(p0z, p.origin.z.f4);

					// float va = dot(norm, e2);
					__m128 va = _mm_mul_ps(nx, e2x);
					va = _mm_add_ps(_mm_mul_ps(ny, e2y), va);
					va = _mm_add_ps(_mm_mul_ps(nz, e2z), va);

					// float t = r_ * va;
					__m128 t = _mm_mul_ps(r_, va);

					// bool m = (t < r->hit.distance) && (t > 0.0f);
					__m128 m = _mm_and_ps(_mm_cmplt_ps(t, p.hit.distance.f4), _mm_cmpge_ps(t, Zero4));
					if(!_mm_movemask_ps(m))
						return;

					// float3 i = cross(e2, r->direction);
					__m128 ix = _mm_sub_ps(_mm_mul_ps(e2y, p.direction.z.f4), _mm_mul_ps(e2z, p.direction.y.f4));
					__m128 iy = _mm_sub_ps(_mm_mul_ps(e2z, p.direction.x.f4), _mm_mul_ps(e2x, p.direction.z.f4));
					__m128 iz = _mm_sub_ps(_mm_mul_ps(e2x, p.direction.y.f4), _mm_mul_ps(e2y, p.direction.x.f4));

					// float v1 = dot(i, e1);
					__m128 v1 = _mm_mul_ps(ix, e1x);
					v1 = _mm_add_ps(_mm_mul_ps(iy, e1y), v1);
					v1 = _mm_add_ps(_mm_mul_ps(iz, e1z), v1);

					// float beta = r_ * v1;
					__m128 beta = _mm_mul_ps(r_, v1);

					// m &= (beta >= 0.0f);
					m = _mm_and_ps(_mm_cmpge_ps(beta, Zero4), m);
					if(!_mm_movemask_ps(m))
						return;

					// float v2 = dot(i, e0);
					__m128 v2 = _mm_mul_ps(ix, e0x);
					v2 = _mm_add_ps(_mm_mul_ps(iy, e0y), v2);
					v2 = _mm_add_ps(_mm_mul_ps(iz, e0z), v2);

					// m &= ((v1 + v2) * v) <= (v * v);
					m = _mm_and_ps(_mm_cmple_ps(_mm_mul_ps(_mm_add_ps(v1, v2), v), _mm_mul_ps(v, v)), m);

					// float gamma = r_ * v2;
					__m128 gamma = _mm_mul_ps(r_, v2);

					// m &= (gamma >= 0.0f);
					m = _mm_and_ps(_mm_cmpge_ps(gamma, Zero4), m);
					if(!_mm_movemask_ps(m))
						return;

					// conditional moves based on mask m
					p.hit.triId.i4 = _mm_sel_si128_xor(p.hit.triId.i4, _mm_set1_epi32(_hitMarker | triId), m);
					p.hit.distance.f4 = _mm_sel_ps_xor(p.hit.distance.f4, t, m);
					p.hit.beta.f4 = _mm_sel_ps_xor(p.hit.beta.f4, beta, m);
					p.hit.gamma.f4 = _mm_sel_ps_xor(p.hit.gamma.f4, gamma, m);		
				}

				inline void IntersectWithTriangle(packet *packets, unsigned int mask, int first, int last, int triId) const
				{
					const IndexType *indices = _triangleList + 3 * triId;
					const float3 &p0 = _vertices[indices[0]], &p1 = _vertices[indices[1]], &p2 = _vertices[indices[2]];

					// float3 e0 = p1 - p0, e1 = p0 - p2;
					__m128 p0x = _mm_set1_ps(p0.x), p0y = _mm_set1_ps(p0.y), p0z = _mm_set1_ps(p0.z);
					__m128 e0x = _mm_sub_ps(_mm_set1_ps(p1.x), p0x), e0y = _mm_sub_ps(_mm_set1_ps(p1.y), p0y), e0z = _mm_sub_ps(_mm_set1_ps(p1.z), p0z);
					__m128 e1x = _mm_sub_ps(p0x, _mm_set1_ps(p2.x)), e1y = _mm_sub_ps(p0y, _mm_set1_ps(p2.y)), e1z = _mm_sub_ps(p0z, _mm_set1_ps(p2.z));

					// n = cross(e0, e1)
					__m128 nx = _mm_sub_ps(_mm_mul_ps(e0y, e1z), _mm_mul_ps(e0z, e1y));
					__m128 ny = _mm_sub_ps(_mm_mul_ps(e0z, e1x), _mm_mul_ps(e0x, e1z));
					__m128 nz = _mm_sub_ps(_mm_mul_ps(e0x, e1y), _mm_mul_ps(e0y, e1x));

					for(int i = first; i <= last; ++i)
					{
						if((mask & (1 << i)) == 0)
							continue;

						packet &p = packets[i];

						// float v = dot(norm, r->direction);
						__m128 v = _mm_mul_ps(nx, p.direction.x.f4);
						v = _mm_add_ps(_mm_mul_ps(ny, p.direction.y.f4), v);
						v = _mm_add_ps(_mm_mul_ps(nz, p.direction.z.f4), v);

						// float r_ = 1.0f / v;
						__m128 r_ = _mm_inverse_ps(v);

						// float3 e2 = p0 - r->origin;
						__m128 e2x = _mm_sub_ps(p0x, p.origin.x.f4);
						__m128 e2y = _mm_sub_ps(p0y, p.origin.y.f4);
						__m128 e2z = _mm_sub_ps(p0z, p.origin.z.f4);

						// float va = dot(norm, e2);
						__m128 va = _mm_mul_ps(nx, e2x);
						va = _mm_add_ps(_mm_mul_ps(ny, e2y), va);
						va = _mm_add_ps(_mm_mul_ps(nz, e2z), va);

						// float t = r_ * va;
						__m128 t = _mm_mul_ps(r_, va);

						// bool m = (t < r->hit.distance) && (t > 0.0f);
						__m128 m = _mm_and_ps(_mm_cmplt_ps(t, p.hit.distance.f4), _mm_cmpge_ps(t, Zero4));
						if(!_mm_movemask_ps(m))
							continue;

						// float3 i = cross(e2, r->direction);
						__m128 ix = _mm_sub_ps(_mm_mul_ps(e2y, p.direction.z.f4), _mm_mul_ps(e2z, p.direction.y.f4));
						__m128 iy = _mm_sub_ps(_mm_mul_ps(e2z, p.direction.x.f4), _mm_mul_ps(e2x, p.direction.z.f4));
						__m128 iz = _mm_sub_ps(_mm_mul_ps(e2x, p.direction.y.f4), _mm_mul_ps(e2y, p.direction.x.f4));

						// float v1 = dot(i, e1);
						__m128 v1 = _mm_mul_ps(ix, e1x);
						v1 = _mm_add_ps(_mm_mul_ps(iy, e1y), v1);
						v1 = _mm_add_ps(_mm_mul_ps(iz, e1z), v1);

						// float beta = r_ * v1;
						__m128 beta = _mm_mul_ps(r_, v1);

						// m &= (beta >= 0.0f);
						m = _mm_and_ps(_mm_cmpge_ps(beta, Zero4), m);
						if(!_mm_movemask_ps(m))
							continue;

						// float v2 = dot(i, e0);
						__m128 v2 = _mm_mul_ps(ix, e0x);
						v2 = _mm_add_ps(_mm_mul_ps(iy, e0y), v2);
						v2 = _mm_add_ps(_mm_mul_ps(iz, e0z), v2);

						// m &= ((v1 + v2) * v) <= (v * v);
						m = _mm_and_ps(_mm_cmple_ps(_mm_mul_ps(_mm_add_ps(v1, v2), v), _mm_mul_ps(v, v)), m);

						// float gamma = r_ * v2;
						__m128 gamma = _mm_mul_ps(r_, v2);

						// m &= (gamma >= 0.0f);
						m = _mm_and_ps(_mm_cmpge_ps(gamma, Zero4), m);
						if(!_mm_movemask_ps(m))
							continue;

						// conditional moves based on mask m
						p.hit.triId.i4 = _mm_sel_si128_xor(p.hit.triId.i4, _mm_set1_epi32(_hitMarker | triId), m);
						p.hit.distance.f4 = _mm_sel_ps_xor(p.hit.distance.f4, t, m);
						p.hit.beta.f4 = _mm_sel_ps_xor(p.hit.beta.f4, beta, m);
						p.hit.gamma.f4 = _mm_sel_ps_xor(p.hit.gamma.f4, gamma, m);
					}
				}

			public:
				TraversalLeafOp(const IndexType *triangleList, const float3 *vertices, const std::vector<int> &indices, int instance) :
					_triangleList(triangleList), _vertices(vertices), _indices(indices), _hitMarker((1 << 31) | (instance << ray::MaxDynamicTriangleBits))
				{ }

				void operator()(ray &r, const float3 &invDir, int firstObj, int lastObj, int flags) const
				{
#ifdef RECORDRAYSTATS
					++r.dynamicStats.visitedLeaves;
					r.dynamicStats.intersectedTriangles += lastObj - firstObj + 1;
#endif
					for(int i = firstObj; i <= lastObj; ++i)
						IntersectWithTriangle(r, _indices[i]);
				}

				void operator()(packet &p, int firstObj, int lastObj, int flags) const
				{
#ifdef RECORDRAYSTATS
					++p.dynamicStats.visitedLeaves;
					p.dynamicStats.intersectedTriangles += lastObj - firstObj + 1;
#endif
					for(int i = firstObj; i <= lastObj; ++i)
						IntersectWithTriangle(p, _indices[i]);
				}

				void operator()(packet packets[], int first, int last, unsigned int mask, int firstObj, int lastObj, int flags) const
				{
#ifdef RECORDRAYSTATS
					int numberOfTriangles = lastObj - firstObj + 1;
					for(int i = first; i <= last; ++i)
					{
						++packets[i].dynamicStats.visitedLeaves;
						if((mask & (1 << i)) != 0)
							packets[i].dynamicStats.intersectedTriangles += numberOfTriangles;
					}
#endif
					for(int i = firstObj; i <= lastObj; ++i)
						IntersectWithTriangle(packets, mask, first, last, _indices[i]);
				}
			};

			class TraversalLeafOpCO // common origin traversal operations
			{
				const IndexType * const _triangleList;
				const float3 * const _vertices;
				const std::vector<int> &_indices;
				const int _hitMarker;

				inline void IntersectWithTriangleCommonOrigin(packet *packets, unsigned int mask, int first, int last, int triId) const
				{
					const IndexType *indices = _triangleList + 3 * triId;
					const float3 &p0 = _vertices[indices[0]], &p1 = _vertices[indices[1]], &p2 = _vertices[indices[2]];

					// float3 e0 = p1 - p0, e1 = p0 - p2;
					__m128 p0x = _mm_set1_ps(p0.x), p0y = _mm_set1_ps(p0.y), p0z = _mm_set1_ps(p0.z);
					__m128 e0x = _mm_sub_ps(_mm_set1_ps(p1.x), p0x), e0y = _mm_sub_ps(_mm_set1_ps(p1.y), p0y), e0z = _mm_sub_ps(_mm_set1_ps(p1.z), p0z);
					__m128 e1x = _mm_sub_ps(p0x, _mm_set1_ps(p2.x)), e1y = _mm_sub_ps(p0y, _mm_set1_ps(p2.y)), e1z = _mm_sub_ps(p0z, _mm_set1_ps(p2.z));

					// n = cross(e0, e1)
					__m128 nx = _mm_sub_ps(_mm_mul_ps(e0y, e1z), _mm_mul_ps(e0z, e1y));
					__m128 ny = _mm_sub_ps(_mm_mul_ps(e0z, e1x), _mm_mul_ps(e0x, e1z));
					__m128 nz = _mm_sub_ps(_mm_mul_ps(e0x, e1y), _mm_mul_ps(e0y, e1x));

					// NOTE: common origin packets enable us to perform the following calculations only once
					// float3 e2 = p0 - r->origin;
					__m128 e2x = _mm_sub_ps(p0x, packets[0].origin.x.f4);
					__m128 e2y = _mm_sub_ps(p0y, packets[0].origin.y.f4);
					__m128 e2z = _mm_sub_ps(p0z, packets[0].origin.z.f4);

					// float va = dot(norm, e2);
					__m128 va = _mm_mul_ps(nx, e2x);
					va = _mm_add_ps(_mm_mul_ps(ny, e2y), va);
					va = _mm_add_ps(_mm_mul_ps(nz, e2z), va);

					for(int i = first; i <= last; ++i)
					{
						if((mask & (1 << i)) == 0)
							continue;

						packet &p = packets[i];

						// float v = dot(norm, r->direction);
						__m128 v = _mm_mul_ps(nx, p.direction.x.f4);
						v = _mm_add_ps(_mm_mul_ps(ny, p.direction.y.f4), v);
						v = _mm_add_ps(_mm_mul_ps(nz, p.direction.z.f4), v);

						// float r_ = 1.0f / v;
						__m128 r_ = _mm_inverse_ps(v);

						// float t = r_ * va;
						__m128 t = _mm_mul_ps(r_, va);

						// bool m = (t < r->hit.distance) && (t > 0.0f);
						__m128 m = _mm_and_ps(_mm_cmplt_ps(t, p.hit.distance.f4), _mm_cmpge_ps(t, Zero4));
						if(!_mm_movemask_ps(m))
							continue;

						// float3 i = cross(e2, r->direction);
						__m128 ix = _mm_sub_ps(_mm_mul_ps(e2y, p.direction.z.f4), _mm_mul_ps(e2z, p.direction.y.f4));
						__m128 iy = _mm_sub_ps(_mm_mul_ps(e2z, p.direction.x.f4), _mm_mul_ps(e2x, p.direction.z.f4));
						__m128 iz = _mm_sub_ps(_mm_mul_ps(e2x, p.direction.y.f4), _mm_mul_ps(e2y, p.direction.x.f4));

						// float v1 = dot(i, e1);
						__m128 v1 = _mm_mul_ps(ix, e1x);
						v1 = _mm_add_ps(_mm_mul_ps(iy, e1y), v1);
						v1 = _mm_add_ps(_mm_mul_ps(iz, e1z), v1);

						// float beta = r_ * v1;
						__m128 beta = _mm_mul_ps(r_, v1);

						// m &= (beta >= 0.0f);
						m = _mm_and_ps(_mm_cmpge_ps(beta, Zero4), m);
						if(!_mm_movemask_ps(m))
							continue;

						// float v2 = dot(i, e0);
						__m128 v2 = _mm_mul_ps(ix, e0x);
						v2 = _mm_add_ps(_mm_mul_ps(iy, e0y), v2);
						v2 = _mm_add_ps(_mm_mul_ps(iz, e0z), v2);

						// m &= ((v1 + v2) * v) <= (v * v);
						m = _mm_and_ps(_mm_cmple_ps(_mm_mul_ps(_mm_add_ps(v1, v2), v), _mm_mul_ps(v, v)), m);

						// float gamma = r_ * v2;
						__m128 gamma = _mm_mul_ps(r_, v2);

						// m &= (gamma >= 0.0f);
						m = _mm_and_ps(_mm_cmpge_ps(gamma, Zero4), m);
						if(!_mm_movemask_ps(m))
							continue;

						// conditional moves based on mask m
						p.hit.triId.i4 = _mm_sel_si128_xor(p.hit.triId.i4, _mm_set1_epi32(_hitMarker | triId), m);
						p.hit.distance.f4 = _mm_sel_ps_xor(p.hit.distance.f4, t, m);
						p.hit.beta.f4 = _mm_sel_ps_xor(p.hit.beta.f4, beta, m);
						p.hit.gamma.f4 = _mm_sel_ps_xor(p.hit.gamma.f4, gamma, m);
					}
				}

			public:
				TraversalLeafOpCO(const IndexType *triangleList, const float3 *vertices, const std::vector<int> &indices, int instance) :
					_triangleList(triangleList), _vertices(vertices), _indices(indices), _hitMarker((1 << 31) | (instance << ray::MaxDynamicTriangleBits))
				{ }

				void operator()(packet packets[], int first, int last, unsigned int mask, int firstObj, int lastObj, int flags) const
				{
#ifdef RECORDRAYSTATS
					int numberOfTriangles = lastObj - firstObj + 1;
					for(int i = first; i <= last; ++i)
					{
						++packets[i].dynamicStats.visitedLeaves;
						if((mask & (1 << i)) != 0)
							packets[i].dynamicStats.intersectedTriangles += numberOfTriangles;
					}
#endif
					for(int i = firstObj; i <= lastObj; ++i)
						IntersectWithTriangleCommonOrigin(packets, mask, first, last, _indices[i]);
				}

			};

			class GreaterEqualThan
			{
				int _val;

			public:
				inline GreaterEqualThan(int val) : _val(val) { }
				inline bool operator()(const int &value) const { return value >= _val; }
			};

			enum { MaxDepth = 64 };

			int _numberOfTriangles, _numberOfVertices;
			IndexType *_triangleList;
			float3 *_vertices;

			std::vector<triangle> _triangles;
			HierarchyType<TriangleGetter, MaxDepth> _hierarchy; // objects array contains indices into the triangles array

			std::map<DynamicScene *, std::vector<int> > _parents;

			void AddParent(DynamicScene *parent, int handle)
			{
				assert(parent != 0);
				if(_parents.find(parent) == _parents.end())
					_parents[parent] = std::vector<int>();
				_parents[parent].push_back(handle);
			}

			void RemoveParent(DynamicScene *parent, int handle)
			{
				assert(parent != 0);
				typename std::map<DynamicScene *, std::vector<int> >::iterator it = _parents.find(parent);
				assert(it != _parents.end());
				std::vector<int>::iterator it2 = find(it->second.begin(), it->second.end(), handle);
				assert(it2 != it->second.end());
				it->second.erase(it2);
				if(it->second.size() == 0)
					_parents.erase(it);
			}

			void UpdateTriangles(int first, int last)
			{
				for(int i = first, j = first * 3; i <= last; ++i, j += 3)
				{
					const float3 &v0 = _vertices[_triangleList[j]];
					const float3 &v1 = _vertices[_triangleList[j + 1]];
					const float3 &v2 = _vertices[_triangleList[j + 2]];
					triangle &tri = _triangles[i];
					tri.lower = min(min(v0, v1), v2);
					tri.upper = max(max(v0, v1), v2);
					tri.centroid = (tri.lower + tri.upper) * 0.5f;
				}
			}

			// the tracing functions are called directly by the scene to reuse invDir and/or signMask
			void Trace(ray &r, const float3 &invDir, int flags, unsigned int signMask, int instance)
			{
				_hierarchy.Traverse(r, invDir, flags, signMask,
					TraversalLeafOp(_triangleList, _vertices, _hierarchy.indices, instance));
			}

			void Trace(packet &p, int flags, unsigned int signMask, int instance)
			{
				_hierarchy.Traverse(p, flags, signMask,
					TraversalLeafOp(_triangleList, _vertices, _hierarchy.indices, instance));
			}

			inline void Trace(packet packets[], int count, int flags, unsigned int signMask, int instance)
			{
				if(flags & CommonOrigin)
				{
					_hierarchy.Traverse(packets, count, flags, signMask,
						TraversalLeafOpCO(_triangleList, _vertices, _hierarchy.indices, instance));
				}
				else
				{
					_hierarchy.Traverse(packets, count, flags, signMask,
						TraversalLeafOp(_triangleList, _vertices, _hierarchy.indices, instance));
				}
			}

		public:
			static const bool NeedsCommonDirSigns = HierarchyType<TriangleGetter, MaxDepth>::NeedsCommonDirSigns;

			Geometry(int numberOfTriangles = 0, int numberOfVertices = 0) :
				_numberOfTriangles(0), _numberOfVertices(0), _triangleList(0), _vertices(0),
				_hierarchy(TriangleGetter(_triangles))
			{
				SetNumberOfTriangles(numberOfTriangles);
				SetNumberOfVertices(numberOfVertices);
			}

			Geometry(const Geometry &other) :
				_numberOfTriangles(0), _numberOfVertices(0), _triangleList(0), _vertices(0),
				_triangles(other._triangles), _hierarchy(other._hierarchy, TriangleGetter(_triangles))
			{
				// NOTE: don't copy parents

				// copy triangle list
				SetNumberOfTriangles(other._numberOfTriangles);
				if(_numberOfTriangles > 0)
				{
					assert(_triangleList != 0);
					assert(other._triangleList != 0);
					memcpy(_triangleList, other._triangleList, sizeof(IndexType) * 3 * _numberOfTriangles);
				}

				// copy vertices
				SetNumberOfVertices(other._numberOfVertices);
				if(_numberOfVertices > 0)
				{
					assert(_vertices != 0);
					assert(other._vertices != 0);
					memcpy(_vertices, other._vertices, sizeof(float3) * _numberOfVertices);
				}
			}

			Geometry &operator=(const Geometry &other)
			{
				if(this != &other)
				{
					// NOTE: don't copy parents

					// copy triangle list
					SetNumberOfTriangles(other._numberOfTriangles);
					if(_numberOfTriangles > 0)
					{
						assert(_triangleList != 0);
						assert(other._triangleList != 0);
						memcpy(_triangleList, other._triangleList, sizeof(IndexType) * 3 * _numberOfTriangles);
					}

					// copy vertices
					SetNumberOfVertices(other._numberOfVertices);
					if(_numberOfVertices > 0)
					{
						assert(_vertices != 0);
						assert(other._vertices != 0);
						memcpy(_vertices, other._vertices, sizeof(float3) * _numberOfVertices);
					}

					_triangles = other._triangles;
					_hierarchy = other._hierarchy;			
				}
				return *this;
			}

			~Geometry()
			{
				// deregister at parents'
				while(_parents.size() > 0)
				{
					DynamicScene *parent = _parents.begin()->first;
					assert(_parents.begin()->second.size() > 0);
					int handle = *_parents.begin()->second.begin();
					parent->Remove(handle); // callback will remove parent from _parents
				}

				// release memory
				if(_vertices != 0) free(_vertices);
				if(_triangleList != 0) free(_triangleList);
			}

			void Rebuild()
			{
				UpdateTriangles(0, _numberOfTriangles - 1);

				// evaluate hierarchy's bounding box
				if(_numberOfTriangles > 0)
				{
					_hierarchy.lower = _triangles[0].lower;
					_hierarchy.upper = _triangles[0].upper;
					for(int i = 1; i < _numberOfTriangles; ++i)
					{
						_hierarchy.lower = min(_hierarchy.lower, _triangles[i].lower);
						_hierarchy.upper = max(_hierarchy.upper, _triangles[i].upper);
					}
				}
				else
				{
					_hierarchy.lower = float3(0.0f);
					_hierarchy.upper = float3(0.0f);
				}

				_hierarchy.Reset(); // clear the hierarchy so it will be rebuilt from scratch when tracing

				// propagate updated bounding box in the scene for all leaves containing instances of this geometry
				for(typename std::map<DynamicScene *, std::vector<int> >::iterator it = _parents.begin(); it != _parents.end(); ++it)
					it->first->Rebuild(this);
			}

			void Refit(int firstTriangle, int lastTriangle)
			{
				// update bounding boxes of the triangles in the specified range
				UpdateTriangles(firstTriangle, lastTriangle);

				// get a list of all leaves which contain parts of the specified triangle range
				std::set<int> leaves;
				for(int i = firstTriangle; i <= lastTriangle; ++i)
					leaves.insert(_triangles[i].leaf);

				// reevaluate the bounding boxes for the leaves and
				// propagate the updates bounding boxes up to the root node of this geometry
				if(_hierarchy.Refit(leaves))
				{
					// continue propagation in the scene for all leaves containing instances of this geometry
					for(typename std::map<DynamicScene *, std::vector<int> >::iterator it = _parents.begin(); it != _parents.end(); ++it)
						it->first->Refit(this);
				}
			}

			// threshold: if the costs of a split increase past this threshold a selective rebuild is performed
			// 1.2 means 20% cost increase over original configuration
			int SelectivelyRebuild(int firstTriangle, int lastTriangle, float threshold)
			{
				assert(threshold >= 0.0f);
		
				// update bounding boxes of the triangles in the specified range
				UpdateTriangles(firstTriangle, lastTriangle);

				// get a list of all leaves which contain parts of the specified triangle range
				std::set<int> leaves;
				for(int i = firstTriangle; i <= lastTriangle; ++i)
					leaves.insert(_triangles[i].leaf);

				// reevaluate the bounding boxes for the leaves and
				// propagate the updates bounding boxes up to the root node of this geometry
				int count = 0;
				if(_hierarchy.SelectivelyRebuild(leaves, threshold, &count))
				{
					// continue propagation in the scene for all leaves containing instances of this geometry
					for(typename std::map<DynamicScene *, std::vector<int> >::iterator it = _parents.begin(); it != _parents.end(); ++it)
						it->first->SelectivelyRebuild(this, threshold);
				}
				return count;
			}

			void SetNumberOfTriangles(int numberOfTriangles) // A full rebuild is mandatory before tracing when changing # of tris!
			{
				assert(numberOfTriangles >= 0);
				if(_numberOfTriangles != numberOfTriangles)
				{
					if(numberOfTriangles < _numberOfTriangles)
					{
						// remove indices of triangles that are no longer valid
						remove_if(_hierarchy.indices.begin(), _hierarchy.indices.end(), GreaterEqualThan(numberOfTriangles));
					}
					else
					{
						// add indices for new triangles
						_hierarchy.indices.resize(numberOfTriangles);
						for(int i = _numberOfTriangles; i < numberOfTriangles; ++i)
							_hierarchy.indices[i] = i;
					}

					_triangleList = reinterpret_cast<IndexType *>(realloc(_triangleList, sizeof(IndexType) * 3 * numberOfTriangles));
					_triangles.resize(numberOfTriangles);
					_numberOfTriangles = numberOfTriangles;
				}
			}

			void SetNumberOfVertices(int numberOfVertices)
			{
				assert(numberOfVertices >= 0);
				if(_numberOfVertices != numberOfVertices)
				{
					_vertices = reinterpret_cast<float3 *>(realloc(_vertices, sizeof(float3) * numberOfVertices));
					_numberOfVertices = numberOfVertices;
				}
			}

			inline int GetNumberOfTriangles() const { return _numberOfTriangles; }
			inline IndexType *GetTriangleList() { return _triangleList; }
			inline const IndexType *GetTriangleList() const { return _triangleList; }
			inline int GetNumberOfVertices() const { return _numberOfVertices; }
			inline float3 *GetVertices() { return _vertices; }
			inline const float3 *GetVertices() const { return _vertices; }

			inline const float3 &GetLower() const { return _hierarchy.lower; }
			inline const float3 &GetUpper() const { return _hierarchy.upper; }

			inline void Trace(ray &r, int flags, int instance = 0) { Trace(r, float3(1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z), flags, GetSignMask(r.direction), instance); }
			inline void Trace(ray rays[], int count, int flags, int instance = 0) { for(int i = 0; i < count; ++i) Trace(rays[i], flags, instance); }
			inline void Trace(packet &p, int flags, int instance = 0) { Trace(p, flags, GetSignMask(p.rays[0]->direction), instance); }
			inline void Trace(packet packets[], int count, int flags, int instance = 0) { Trace(packets, count, flags, GetSignMask(packets[0].rays[0]->direction), instance); }
		};

	private:
		struct Instance : public ObjectType
		{
			void *userPointer;
			Geometry *geometry;
			float4x4 transform, invtransform;
			int handle;
			bool transformed;

			inline Instance() : userPointer(0), geometry(0), handle(-1), transformed(false)
			{ }

			inline Instance(Geometry *geometry, int handle) :
				userPointer(0), geometry(geometry), handle(handle), transformed(false)
			{ }

			void Update()
			{
				if(transformed)
				{
					float3 p[2] = { geometry->GetLower(), geometry->GetUpper() };
					ObjectType::lower = ObjectType::upper = transform * float4(p[0].x, p[0].y, p[0].z, 1.0f);
					for(int i = 1; i < 8; ++i)
					{
						float3 point = transform * float4(p[i & 0x1].x, p[(i >> 1) & 0x1].y, p[(i >> 2) & 0x1].z, 1.0f);
						ObjectType::lower = min(ObjectType::lower, point);
						ObjectType::upper = max(ObjectType::upper, point);
					}
				}
				else
				{
					ObjectType::lower = geometry->GetLower();
					ObjectType::upper = geometry->GetUpper();
				}
				ObjectType::centroid = (ObjectType::lower + ObjectType::upper) * 0.5f;
			}
		};

		class InstanceGetter
		{
			std::map<int, Instance> &_instanceMap;

		public:
			typedef Instance Type;

			inline InstanceGetter(std::map<int, Instance> &instanceMap) : _instanceMap(instanceMap) { }
			inline Instance &Get(int handle) const { assert(_instanceMap.find(handle) != _instanceMap.end()); return _instanceMap[handle]; }
			inline int GetSize() const { return _instanceMap.size(); }
		};

		class TraversalLeafOp
		{
			const std::map<int, Instance> &_instanceMap;
			const std::vector<int> &_indices;
			int _flags;
			unsigned int _signMask;

			inline static void TransformPacket(packet &p, const vector3 &origin, const vector3 &direction, const float4x4 &t)
			{
				vector3 a(quad(t.s.x), quad(t.s.y), quad(t.s.z));
				vector3 b(quad(t.t.x), quad(t.t.y), quad(t.t.z));
				vector3 c(quad(t.u.x), quad(t.u.y), quad(t.u.z));

				p.origin.x = dot(origin, a) + quad(t.s.w);
				p.origin.y = dot(origin, b) + quad(t.t.w);
				p.origin.z = dot(origin, c) + quad(t.u.w);

				p.direction.x = dot(direction, a);
				p.direction.y = dot(direction, b);
				p.direction.z = dot(direction, c);

				// calculate new invdir
				p.invdir[0].f4 = _mm_inverse_ps(p.direction.x.f4);
				p.invdir[1].f4 = _mm_inverse_ps(p.direction.y.f4);
				p.invdir[2].f4 = _mm_inverse_ps(p.direction.z.f4);
			}

		public:
			TraversalLeafOp(const std::map<int, Instance> &instanceMap, const std::vector<int> &indices, int flags, unsigned int signMask) :
				_instanceMap(instanceMap), _indices(indices), _flags(flags), _signMask(signMask)
			{ }

			void operator()(ray &r, const float3 &invDir, int firstObj, int lastObj, int flags) const
			{
#ifdef RECORDRAYSTATS
				++r.dynamicStats.visitedInnerNodes; // this counts as an inner node
#endif
				for(int i = firstObj; i <= lastObj; ++i)
				{
					const Instance &inst = _instanceMap.find(_indices[i])->second;
					if(inst.transformed)
					{
						float3 org = r.origin, dir = r.direction; // backup
						r.origin = inst.invtransform * float4(org, 1); // transform origin
						r.direction = inst.invtransform * float4(dir, 0); // transform direction
						inst.geometry->Trace(r, float3(1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z),
							_flags, _signMask, inst.handle);
						r.origin = org; r.direction = dir; // restore
					}
					else
						inst.geometry->Trace(r, invDir, _flags, _signMask, inst.handle);
				}
			}

			void operator()(packet &p, int firstObj, int lastObj, int flags) const
			{
#ifdef RECORDRAYSTATS
				++p.dynamicStats.visitedInnerNodes; // this counts as an inner node
#endif	
				bool transformed = false;
				for(int i = firstObj; i <= lastObj; ++i)
				{
					const Instance &inst = _instanceMap.find(_indices[i])->second;
					transformed |= inst.transformed;
					if(!inst.transformed) // handle untransformed geometry
						inst.geometry->Trace(p, flags, inst.handle);
				}

				if(transformed) // handle transformed geometry
				{
					vector3 origin = p.origin, direction = p.direction, invdir = p.invdir; // backup
					for(int i = firstObj; i <= lastObj; ++i)
					{
						const Instance &inst = _instanceMap.find(_indices[i])->second;
						if(inst.transformed)
						{
							TransformPacket(p, origin, direction, inst.invtransform); // transform
							inst.geometry->Trace(p, flags, _signMask, inst.handle);
						}
					}			
					p.origin = origin; p.direction = direction; p.invdir = invdir; // restore
				}
			}

			void operator()(packet packets[], int first, int last, unsigned int mask, int firstObj, int lastObj, int flags) const
			{
				int count = last - first + 1;
				packets += first;
#ifdef RECORDRAYSTATS
				for(int i = 0; i < count; ++i)
					++packets[i].dynamicStats.visitedInnerNodes; // this counts as an inner node
#endif
				bool transformed = false;
				for(int i = firstObj; i <= lastObj; ++i)
				{
					const Instance &inst = _instanceMap.find(_indices[i])->second;
					transformed |= inst.transformed;
					if(!inst.transformed) // handle untransformed geometry
						inst.geometry->Trace(packets, count, flags, inst.handle);
				}

				if(transformed) // handle transformed geometry
				{
					// allocate memory on the stack for backing up the untransformed packet
					void *mem = sse_alloca(count * 3 * sizeof(vector3));
					vector3 *origins = reinterpret_cast<vector3 *>(mem);
					vector3 *directions = origins + count;
					vector3 *invdirs = directions + count;
					for(int i = 0; i < count; ++i)
					{
						origins[i] = packets[i].origin;
						directions[i] = packets[i].direction;
						invdirs[i] = packets[i].invdir;
					}

					for(int i = firstObj; i <= lastObj; ++i)
					{
						const Instance &inst = _instanceMap.find(_indices[i])->second;
						if(inst.transformed)
						{
							for(int j = 0; j < count; ++j)
								TransformPacket(packets[j], origins[j], directions[j], inst.invtransform);
							inst.geometry->Trace(packets, count, flags, _signMask, inst.handle);
						}
					}

					// restore
					for(int i = 0; i < count; ++i)
					{
						packets[i].origin = origins[i];
						packets[i].direction = directions[i];
						packets[i].invdir = invdirs[i];
					}
				}
			}
		};

		enum { MaxDepth = 32 };

		std::map<int, Instance> _instanceMap;
		int _nextHandle;
		HierarchyType<InstanceGetter, MaxDepth> _hierarchy; // object array contains handles into the instance map

		int GetNextFreeHandle()
		{
			if(_instanceMap.size() >= ray::MaxDynamicInstances) // reached the maximum
				throw "Out of handles!";

			// find the next free handle, [0;MaxDynamicInstances-1]
			while(_instanceMap.find(_nextHandle) != _instanceMap.end()) // find next free handle
				_nextHandle = (_nextHandle + 1) & ((1 << ray::MaxDynamicTriangleBits) - 1); // increment and handle wrap around

			int handle = _nextHandle;
			_nextHandle = (_nextHandle + 1) & ((1 << ray::MaxDynamicTriangleBits) - 1); // increment and handle wrap around
			return handle;
		}

		void UpdateInstances()
		{
			// update the bounding boxes of all instances
			if(_instanceMap.size() > 0)
			{
				typename std::map<int, Instance>::iterator it = _instanceMap.begin();
				it->second.Update();
				_hierarchy.lower = it->second.lower;
				_hierarchy.upper = it->second.upper;
				for(++it; it != _instanceMap.end(); ++it)
				{
					it->second.Update();
					_hierarchy.lower = min(_hierarchy.lower, it->second.lower);
					_hierarchy.upper = max(_hierarchy.upper, it->second.upper);
				}
			}
			else
			{
				_hierarchy.lower = 0.0f;
				_hierarchy.upper = 0.0f;
			}
		}

		void UpdateInstancesOf(Geometry *geometry)
		{
			// update the bounding boxes of instances of the specified geometry
			if(_instanceMap.size() > 0)
			{
				typename std::map<int, Instance>::iterator it = _instanceMap.begin();
				if(it->second.geometry == geometry)
					it->second.Update();
				_hierarchy.lower = it->second.lower;
				_hierarchy.upper = it->second.upper;
				for(++it; it != _instanceMap.end(); ++it)
				{
					if(it->second.geometry == geometry)
						it->second.Update();
					_hierarchy.lower = min(_hierarchy.lower, it->second.lower);
					_hierarchy.upper = max(_hierarchy.upper, it->second.upper);
				}
			}
			else
			{
				_hierarchy.lower = 0.0f;
				_hierarchy.upper = 0.0f;
			}
		}

		void UpdateBoundsAfterRemovalOf(const Instance &instance)
		{
			if(_instanceMap.size() > 0)
			{
				if(_hierarchy.lower.x >= instance.lower.x || _hierarchy.lower.y >= instance.lower.y || _hierarchy.lower.z >= instance.lower.z ||
					_hierarchy.upper.x <= instance.upper.x || _hierarchy.upper.y <= instance.upper.y || _hierarchy.upper.z <= instance.upper.z)
				{
					// have to reevaluate the scene's bounding box
					typename std::map<int, Instance>::iterator it = _instanceMap.begin();
					_hierarchy.lower = it->second.lower; _hierarchy.upper = it->second.upper;
					for(++it; it != _instanceMap.end(); ++it)
					{
						_hierarchy.lower = min(_hierarchy.lower, it->second.lower);
						_hierarchy.upper = max(_hierarchy.upper, it->second.upper);
					}
				}
			}
			else
			{
				_hierarchy.lower = 0.0f;
				_hierarchy.upper = 0.0f;
			}
		}

		std::set<int> GetLeavesContaining(Geometry *geometry)
		{
			// update the bounding boxes of instances of the specified geometry
			std::set<int> leaves;
			if(_instanceMap.size() > 0)
			{
				for(typename std::map<int, Instance>::iterator it = _instanceMap.begin(); it != _instanceMap.end(); ++it)
				{
					if(it->second.geometry == geometry)
						leaves.insert(it->second.leaf);
				}
			}
			return leaves;
		}

	public:
		static const bool NeedsCommonDirSigns = HierarchyType<InstanceGetter, MaxDepth>::NeedsCommonDirSigns;

		DynamicScene() :
			_nextHandle(0), _hierarchy(InstanceGetter(_instanceMap))
		{ }

		DynamicScene(const DynamicScene &other) :
		_instanceMap(other._instanceMap), _nextHandle(other._nextHandle), _hierarchy(other._hierarchy, InstanceGetter(_instanceMap))
		{
			// add this as a parent to the new geometries
			for(typename std::map<int, Instance>::iterator it = _instanceMap.begin(); it != _instanceMap.end(); ++it)
				it->second.geometry->AddParent(this, it->first);
		}

		DynamicScene &operator=(const DynamicScene &other)
		{
			if(this != &other)
			{
				// remove this as a parent from all geometries
				for(typename std::map<int, Instance>::iterator it = _instanceMap.begin(); it != _instanceMap.end(); ++it)
					it->second.geometry->RemoveParent(this, it->first);

				_instanceMap = other._instanceMap;
				_nextHandle = other._nextHandle;
				_hierarchy = other._hierarchy;

				// add this as a parent to the new geometries
				for(typename std::map<int, Instance>::iterator it = _instanceMap.begin(); it != _instanceMap.end(); ++it)
					it->second.geometry->AddParent(this, it->first);
			}
			return *this;
		}

		inline void Rebuild()
		{
			UpdateInstances();
			_hierarchy.Reset(); // clear the hierarchy so it will be rebuilt from scratch when tracing
		}

		inline void Rebuild(Geometry *geometry)
		{
			assert(geometry != 0);
			UpdateInstancesOf(geometry);
			_hierarchy.Reset(); // clear the hierarchy so it will be rebuilt from scratch when tracing
		}

		inline void Refit(Geometry *geometry)
		{
			assert(geometry != 0);
			UpdateInstancesOf(geometry);
			_hierarchy.Refit(GetLeavesContaining(geometry));
		}

		// threshold: if the costs of a split increase past this threshold a selective rebuild is performed
		// 1.2 means 20% cost increase over original configuration
		inline void SelectivelyRebuild(Geometry *geometry, float threshold)
		{
			assert(geometry != 0);
			assert(threshold >= 0.0f);
			UpdateInstancesOf(geometry);
			_hierarchy.SelectivelyRebuild(GetLeavesContaining(geometry), threshold);
		}

		int Add(Geometry *geometry) // will take effect after rebuilding
		{
			assert(geometry != 0);
			int handle = GetNextFreeHandle();
			_instanceMap[handle] = Instance(geometry, handle);
			geometry->AddParent(this, handle);
			_hierarchy.indices.push_back(handle);
			return handle;
		}

		void Remove(int handle) // removing instances clears the hierarchy
		{
			typename std::map<int, Instance>::iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
			{
				// NOTE: could remember to remove this instance on the next rebuild and don't do it now!
				Instance inst = it->second;
				it->second.geometry->RemoveParent(this, handle);
				_instanceMap.erase(it);

				UpdateBoundsAfterRemovalOf(inst);
				_hierarchy.indices.erase(find(_hierarchy.indices.begin(), _hierarchy.indices.end(), handle));
				_hierarchy.Reset();
			}
			else
				throw "Invalid handle!";
		}

		inline Geometry *Get(int handle) const
		{
			typename std::map<int, Instance>::const_iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
				return it->second.geometry;
			else
				throw "Invalid handle!";
		}

		// take care of rebuilding the structure after changing transforms!
		inline void ClearTransform(int handle)
		{
			typename std::map<int, Instance>::iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
				// NOTE: could remember to apply changes on the next rebuild and don't do it now
				it->second.transformed = false;
			else
				throw "Invalid handle!";
		}

		inline void SetTransform(int handle, const float4x4 &matrix)
		{
			typename std::map<int, Instance>::iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
			{
				// NOTE: could remember to apply changes on the next rebuild and don't do it now
				it->second.transform = matrix;
				it->second.invtransform = inverse(matrix);
				it->second.transformed = true;
			}
			else
				throw "Invalid handle!";
		}

		inline const float4x4 &GetTransform(int handle) const
		{
			static float4x4 identity = identity4x4();
			typename std::map<int, Instance>::const_iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
				return it->second.transformed ? it->second.transform : identity;
			else
				throw "Invalid handle!";
		}

		inline void SetUserPointer(int handle, void *pointer)
		{
			typename std::map<int, Instance>::iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
				it->second.userPointer = pointer;
			else
				throw "Invalid handle!";
		}

		inline void *GetUserPointer(int handle) const
		{
			typename std::map<int, Instance>::const_iterator it = _instanceMap.find(handle);
			if(it != _instanceMap.end())
				return it->second.userPointer;
			else
				throw "Invalid handle!";
		}

		inline void Trace(ray &r, int flags)
		{
			const float3 invDir(1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z);
			unsigned int signMask = GetSignMask(r.direction);
			_hierarchy.Traverse(r, invDir, flags, signMask,
				TraversalLeafOp(_instanceMap, _hierarchy.indices, flags, signMask));
		}

		inline void Trace(ray rays[], int count, int flags)
		{
			for(int i = 0; i < count; ++i)
				Trace(rays[i], flags);
		}

		inline void Trace(packet &p, int flags)
		{
			unsigned int signMask = GetSignMask(p.rays[0]->direction);
			_hierarchy.Traverse(p, flags, signMask,
				TraversalLeafOp(_instanceMap, _hierarchy.indices, flags, signMask));
		}

		inline void Trace(packet packets[], int count, int flags)
		{
			unsigned int signMask = GetSignMask(packets[0].rays[0]->direction);
			_hierarchy.Traverse(packets, count, flags, signMask,
				TraversalLeafOp(_instanceMap, _hierarchy.indices, flags, signMask));
		}
	};
}

#endif // DYNAMICSCENE_H
