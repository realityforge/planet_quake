
#ifndef STATICTRIANGLE_H
#define STATICTRIANGLE_H

#include <assert.h>
#include "packet.h"
#include "ray.h"
#include "xmmhelper.h"

namespace rapido
{
	struct statictriangle
	{
		float3 p0, e0, e1, n;

		inline void IntersectWith(ray &r, int triId) const
		{
			float v = dot(n, r.direction);
			float r_ = 1.0f / v;
			float3 e2 = p0 - r.origin;
			float va = dot(n, e2);
			float t = r_ * va;
			bool m = (t < r.hit.distance) && (t >= 0.0f);
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

			r.hit.triId = triId;
			r.hit.distance = t;
			r.hit.beta = beta;
			r.hit.gamma = gamma;
		}

		inline void IntersectWith(ray rays[], int count, int triId) const
		{
			for(int i = 0; i < count; ++i)
				IntersectWith(rays[i], triId);
		}

		inline void IntersectWithCommonOrigin(ray rays[], int count, int triId) const
		{
			assert(count > 0);
			float3 e2 = p0 - rays[0].origin;
			float va = dot(n, e2);
			for(int j = 0; j < count; ++j)
			{
				ray &r = rays[j];

				float v = dot(n, r.direction);
				float r_ = 1.0f / v;
				float t = r_ * va;
				bool m = (t < r.hit.distance) && (t >= 0.0f);
				if(!m)
					continue;

				float3 i = cross(e2, r.direction);
				float v1 = dot(i, e1);
				float beta = r_ * v1;
				m &= (beta >= 0.0f);
				if(!m)
					continue;

				float v2 = dot(i, e0);
				m &= ((v1 + v2) * v) <= (v * v);
				float gamma = r_ * v2;
				m &= (gamma >= 0.0f);
				if(!m)
					continue;

				r.hit.triId = triId;
				r.hit.distance = t;
				r.hit.beta = beta;
				r.hit.gamma = gamma;
			}
		}

		inline void IntersectWith(packet &p, int triId) const
		{
			// float v = dot(n, r->direction);
			__m128 v = _mm_mul_ps(_mm_set1_ps(n.x), p.direction.x.f4);
			v = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.y), p.direction.y.f4), v);
			v = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.z), p.direction.z.f4), v);

			// float r_ = 1.0f / v;
			__m128 r_ = _mm_inverse_ps(v);

			// float3 e2 = p0 - r->origin;
			__m128 e2x = _mm_sub_ps(_mm_set1_ps(p0.x), p.origin.x.f4);
			__m128 e2y = _mm_sub_ps(_mm_set1_ps(p0.y), p.origin.y.f4);
			__m128 e2z = _mm_sub_ps(_mm_set1_ps(p0.z), p.origin.z.f4);

			// float va = dot(n, e2);
			__m128 va = _mm_mul_ps(_mm_set1_ps(n.x), e2x);
			va = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.y), e2y), va);
			va = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.z), e2z), va);

			// float t = r_ * va;
			__m128 t = _mm_mul_ps(r_, va);

			// bool m = (t < r->hit.distance) && (t >= 0.0f);
			__m128 m = _mm_and_ps(_mm_cmplt_ps(t, p.hit.distance.f4), _mm_cmpge_ps(t, Zero4));
			if(!_mm_movemask_ps(m))
				return;

			// float3 i = cross(e2, r->direction);
			__m128 ix = _mm_sub_ps(_mm_mul_ps(e2y, p.direction.z.f4), _mm_mul_ps(e2z, p.direction.y.f4));
			__m128 iy = _mm_sub_ps(_mm_mul_ps(e2z, p.direction.x.f4), _mm_mul_ps(e2x, p.direction.z.f4));
			__m128 iz = _mm_sub_ps(_mm_mul_ps(e2x, p.direction.y.f4), _mm_mul_ps(e2y, p.direction.x.f4));

			// float v1 = dot(i, e1);
			__m128 v1 = _mm_mul_ps(ix, _mm_set1_ps(e1.x));
			v1 = _mm_add_ps(_mm_mul_ps(iy, _mm_set1_ps(e1.y)), v1);
			v1 = _mm_add_ps(_mm_mul_ps(iz, _mm_set1_ps(e1.z)), v1);

			// float beta = r_ * v1;
			__m128 beta = _mm_mul_ps(r_, v1);

			// m &= (beta >= 0.0f);
			m = _mm_and_ps(_mm_cmpge_ps(beta, Zero4), m);
			if(!_mm_movemask_ps(m))
				return;

			// float v2 = dot(i, e0);
			__m128 v2 = _mm_mul_ps(ix, _mm_set1_ps(e0.x));
			v2 = _mm_add_ps(_mm_mul_ps(iy, _mm_set1_ps(e0.y)), v2);
			v2 = _mm_add_ps(_mm_mul_ps(iz, _mm_set1_ps(e0.z)), v2);

			// m &= ((v1 + v2) * v) <= (v * v);
			m = _mm_and_ps(_mm_cmple_ps(_mm_mul_ps(_mm_add_ps(v1, v2), v), _mm_mul_ps(v, v)), m);

			// float gamma = r_ * v2;
			__m128 gamma = _mm_mul_ps(r_, v2);

			// m &= (gamma >= 0.0f);
			m = _mm_and_ps(_mm_cmpge_ps(gamma, Zero4), m);
			if(!_mm_movemask_ps(m))
				return;

			// conditional moves based on mask m
			p.hit.triId.i4 = _mm_sel_si128_xor(p.hit.triId.i4, _mm_set1_epi32(triId), m);
			p.hit.distance.f4 = _mm_sel_ps_xor(p.hit.distance.f4, t, m);
			p.hit.beta.f4 = _mm_sel_ps_xor(p.hit.beta.f4, beta, m);
			p.hit.gamma.f4 = _mm_sel_ps_xor(p.hit.gamma.f4, gamma, m);
		}

		inline void IntersectWith(packet packets[], int count, int triId) const
		{
			for(int i = 0; i < count; ++i)
				IntersectWith(packets[i], triId);
		}

		inline void IntersectWithCommonOrigin(packet packets[], int count, int triId) const
		{
			assert(count > 0);

			// float3 e2 = p0 - r->origin;
			__m128 e2x = _mm_sub_ps(_mm_set1_ps(p0.x), packets[0].origin.x.f4);
			__m128 e2y = _mm_sub_ps(_mm_set1_ps(p0.y), packets[0].origin.y.f4);
			__m128 e2z = _mm_sub_ps(_mm_set1_ps(p0.z), packets[0].origin.z.f4);

			// float va = dot(n, e2);
			__m128 va = _mm_mul_ps(_mm_set1_ps(n.x), e2x);
			va = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.y), e2y), va);
			va = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.z), e2z), va);

			for(int i = 0; i < count; ++i)
			{
				packet &p = packets[i];

				// float v = dot(n, r->direction);
				__m128 v = _mm_mul_ps(_mm_set1_ps(n.x), p.direction.x.f4);
				v = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.y), p.direction.y.f4), v);
				v = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(n.z), p.direction.z.f4), v);

				// float r_ = 1.0f / v;
				__m128 r_ = _mm_inverse_ps(v);

				// float t = r_ * va;
				__m128 t = _mm_mul_ps(r_, va);

				// bool m = (t < r->hit.distance) && (t >= 0.0f);
				__m128 m = _mm_and_ps(_mm_cmplt_ps(t, p.hit.distance.f4), _mm_cmpge_ps(t, Zero4));
				if(!_mm_movemask_ps(m))
					continue;

				// float3 i = cross(e2, r->direction);
				__m128 ix = _mm_sub_ps(_mm_mul_ps(e2y, p.direction.z.f4), _mm_mul_ps(e2z, p.direction.y.f4));
				__m128 iy = _mm_sub_ps(_mm_mul_ps(e2z, p.direction.x.f4), _mm_mul_ps(e2x, p.direction.z.f4));
				__m128 iz = _mm_sub_ps(_mm_mul_ps(e2x, p.direction.y.f4), _mm_mul_ps(e2y, p.direction.x.f4));

				// float v1 = dot(i, e1);
				__m128 v1 = _mm_mul_ps(ix, _mm_set1_ps(e1.x));
				v1 = _mm_add_ps(_mm_mul_ps(iy, _mm_set1_ps(e1.y)), v1);
				v1 = _mm_add_ps(_mm_mul_ps(iz, _mm_set1_ps(e1.z)), v1);

				// float beta = r_ * v1;
				__m128 beta = _mm_mul_ps(r_, v1);

				// m &= (beta >= 0.0f);
				m = _mm_and_ps(_mm_cmpge_ps(beta, Zero4), m);
				if(!_mm_movemask_ps(m))
					continue;

				// float v2 = dot(i, e0);
				__m128 v2 = _mm_mul_ps(ix, _mm_set1_ps(e0.x));
				v2 = _mm_add_ps(_mm_mul_ps(iy, _mm_set1_ps(e0.y)), v2);
				v2 = _mm_add_ps(_mm_mul_ps(iz, _mm_set1_ps(e0.z)), v2);

				// m &= ((v1 + v2) * v) <= (v * v);
				m = _mm_and_ps(_mm_cmple_ps(_mm_mul_ps(_mm_add_ps(v1, v2), v), _mm_mul_ps(v, v)), m);

				// float gamma = r_ * v2;
				__m128 gamma = _mm_mul_ps(r_, v2);

				// m &= (gamma >= 0.0f);
				m = _mm_and_ps(_mm_cmpge_ps(gamma, Zero4), m);
				if(!_mm_movemask_ps(m))
					continue;

				// conditional moves based on mask m
				p.hit.triId.i4 = _mm_sel_si128_xor(p.hit.triId.i4, _mm_set1_epi32(triId), m);
				p.hit.distance.f4 = _mm_sel_ps_xor(p.hit.distance.f4, t, m);
				p.hit.beta.f4 = _mm_sel_ps_xor(p.hit.beta.f4, beta, m);
				p.hit.gamma.f4 = _mm_sel_ps_xor(p.hit.gamma.f4, gamma, m);
			}
		}
	};
}

#endif // STATICTRIANGLE_H

