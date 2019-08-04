
#ifndef VECTORS_H
#define VECTORS_H

#include <emmintrin.h>
#include <xmmintrin.h>
#include "floats.h"
#include "xmmhelper.h"

struct quad
{
	union
	{
		float f[4];
		int i[4];
		__m128 f4;
		__m128i i4;
	};

	inline quad() { }
	inline quad(const quad &q) : f4(q.f4) { }

	inline quad(float f) : f4(_mm_set1_ps(f)) { }
	inline quad(int i) : i4(_mm_set1_epi32(i)) { }
	inline quad(__m128 f4) : f4(f4) { }
	inline quad(__m128i i4) : i4(i4) { }

	inline quad(float f0, float f1, float f2, float f3) : f4(_mm_set_ps(f3, f2, f1, f0)) { }
	inline quad(int i0, int i1, int i2, int i3) : i4(_mm_set_epi32(i3, i2, i1, i0)) { }

	inline quad operator +() const { return *this; }
	inline quad operator -() const { return _mm_xor_ps(f4, quad(1 << 31).f4); }

	inline operator float *() { return f; }
	inline operator const float *() const { return f; }
};

inline quad operator +(const quad &q1, const quad &q2) { return _mm_add_ps(q1.f4, q2.f4); }
inline quad operator -(const quad &q1, const quad &q2) { return _mm_sub_ps(q1.f4, q2.f4); }
inline quad operator *(const quad &q1, const quad &q2) { return _mm_mul_ps(q1.f4, q2.f4); }
inline quad operator /(const quad &q1, const quad &q2) { return _mm_div_ps(q1.f4, q2.f4); }

inline quad operator <(const quad &q1, const quad &q2) { return _mm_cmplt_ps(q1.f4, q2.f4); }
inline quad operator <=(const quad &q1, const quad &q2) { return _mm_cmple_ps(q1.f4, q2.f4); }
inline quad operator ==(const quad &q1, const quad &q2) { return _mm_cmpeq_ps(q1.f4, q2.f4); }
inline quad operator !=(const quad &q1, const quad &q2) { return _mm_cmpneq_ps(q1.f4, q2.f4); }
inline quad operator >=(const quad &q1, const quad &q2) { return _mm_cmpge_ps(q1.f4, q2.f4); }
inline quad operator >(const quad &q1, const quad &q2) { return _mm_cmpgt_ps(q1.f4, q2.f4); }

inline bool all(const quad &q) { return (_mm_movemask_ps(q.f4) == 0xf); }
inline bool any(const quad &q) { return (_mm_movemask_ps(q.f4) != 0); }

inline quad clamp(const quad &q, const quad &min, const quad &max)
{
	__m128 minm = _mm_cmplt_ps(q.f4, min.f4), maxm = _mm_cmpgt_ps(q.f4, max.f4);
	return _mm_or_ps(_mm_or_ps(_mm_and_ps(minm, min.f4), _mm_and_ps(maxm, max.f4)), _mm_andnot_ps(q.f4, _mm_or_ps(minm, maxm)));
}

inline quad saturate(const quad &q) { return clamp(q, 0.0f, 1.0f); }

inline quad sign(const quad &q)
{
	__m128 neg = _mm_cmplt_ps(q.f4, _mm_set1_ps(0.0f)), pos = _mm_cmpgt_ps(q.f4, _mm_set1_ps(1.0f));
	return _mm_or_ps(_mm_and_ps(neg, _mm_set1_ps(-1.0f)), _mm_and_ps(pos, _mm_set1_ps(1.0f)));
}

inline quad lerp(const quad &q1, const quad &q2, const quad &f) { return _mm_add_ps(_mm_mul_ps(q1.f4, _mm_sub_ps(_mm_set1_ps(1.0f), f.f4)), _mm_mul_ps(q2.f4, f.f4)); }
inline quad step(const quad &q1, const quad &q2, const quad &f) { return _mm_or_ps(_mm_andnot_ps(f.f4, q1.f4), _mm_and_ps(f.f4, q2.f4)); }
inline quad min(const quad &q1, const quad &q2) { return _mm_min_ps(q1.f4, q2.f4); }
inline quad max(const quad &q1, const quad &q2) { return _mm_max_ps(q1.f4, q2.f4); }
inline quad abs(const quad &q) { return _mm_and_ps(quad(~(1 << 31)).f4, q.f4); }
inline quad inverse(const quad &q) { return _mm_inverse_ps(q.f4); }
inline quad sqrtf(const quad &q) { return _mm_sqrt_ps(q.f4); }

inline quad floor(const quad &q) { return _mm_cvttps_epi32(q.f4); }

inline quad floorf(const quad &q)
{
	__m128 q2 = _mm_sub_ps(q.f4, _mm_set1_ps(1.0f)), mask = _mm_cmplt_ps(q.f4, _mm_set1_ps(0.0f));
	__m128 f = _mm_or_ps(_mm_andnot_ps(mask, q.f4), _mm_and_ps(mask, q2));
	return _mm_cvtepi32_ps(_mm_cvttps_epi32(f));
}

inline quad mod(const quad &q, int m)
{
	// TODO: simd version
	return quad(q.i[0] % m, q.i[1] % m, q.i[2] % m, q.i[3] % m);
}

inline quad sin(const quad &q)
{
	// TODO: use _am_sin_ps (approximative maths library
	return quad(sinf(q.f[0]), sinf(q.f[1]), sinf(q.f[2]), sinf(q.f[3]));
}

// Vectors --------------------------------------------------------------------

// 2-dimensional vector -------------------------------------------------------

struct vector2
{
	quad x, y;

	inline vector2() {}
	inline vector2(const quad &q) : x(q), y(q) {}
	inline vector2(const quad &x, const quad &y) : x(x), y(y) {}
	vector2(const struct vector3 &v);
	vector2(const struct vector4 &v);

	inline vector2(const float2 &a, const float2 &b, const float2 &c, const float2 &d) :
		x(a.x, b.x, c.x, d.x), y(a.y, b.y, c.y, d.y) { }

	inline vector2 operator +() const { return *this; }
	inline vector2 operator -() const { return vector2(-x, -y); }

	inline operator quad *() { return &x; }
	inline operator const quad *() const { return &x; }
};

inline vector2 operator +(const vector2 &v1, const vector2 &v2) { return vector2(v1.x + v2.x, v1.y + v2.y); }
inline vector2 operator -(const vector2 &v1, const vector2 &v2) { return vector2(v1.x - v2.x, v1.y - v2.y); }
inline vector2 operator *(const vector2 &v1, const vector2 &v2) { return vector2(v1.x * v2.x, v1.y * v2.y); }
inline vector2 operator /(const vector2 &v1, const vector2 &v2) { return vector2(v1.x / v2.x, v1.y / v2.y); }

inline vector2 operator +(const vector2 &v, const quad &q) { return vector2(v.x + q, v.y + q); }
inline vector2 operator +(const quad &q, const vector2 &v) { return vector2(v.x + q, v.y + q); }
inline vector2 operator -(const vector2 &v, const quad &q) { return vector2(v.x - q, v.y - q); }
inline vector2 operator -(const quad &q, const vector2 &v) { return vector2(q - v.x, q - v.y); }
inline vector2 operator *(const vector2 &v, const quad &q) { return vector2(v.x * q, v.y * q); }
inline vector2 operator *(const quad &q, const vector2 &v) { return vector2(v.x * q, v.y * q); }
inline vector2 operator /(const vector2 &v, const quad &q) { quad inv = _mm_inverse_ps(q.f4); return vector2(v.x * inv, v.y * inv); }
inline vector2 operator /(const quad &q, const vector2 &v) { return vector2(q / v.x, q / v.y); }

inline quad dot(const vector2 &v1, const vector2 &v2) { return v1.x * v2.x + v1.y * v2.y; }
inline quad lengthsq(const vector2 &v) { return dot(v, v); }
inline quad length(const vector2 &v) { return sqrtf(lengthsq(v)); }
inline vector2 lerp(const vector2 &v1, const vector2 &v2, const quad &q) { return v1 + (v2 - v1) * q; }
inline vector2 step(const vector2 &v1, const vector2 &v2, const quad &q) { return vector2(step(v1.x, v2.x, q), step(v1.y, v2.y, q)); }
inline vector2 normalize(const vector2 &v) { return v * _mm_inverse_ps(length(v).f4); }
inline vector2 min(const vector2 &v1, const vector2 &v2) { return vector2(min(v1.x, v2.x), min(v1.y, v2.y)); }
inline vector2 max(const vector2 &v1, const vector2 &v2) { return vector2(max(v1.x, v2.x), max(v1.y, v2.y)); }
inline vector2 abs(const vector2 &v) { return vector2(abs(v.x), abs(v.y)); }
inline vector2 inverse(const vector2 &v) { return vector2(inverse(v.x), inverse(v.y)); }

// 3-dimensional vector -------------------------------------------------------

struct vector3
{
	quad x, y, z;

	inline vector3() {}
	inline vector3(const quad &q) : x(q), y(q), z(q) {}
	inline vector3(const quad &x, const quad &y, const quad &z) : x(x), y(y), z(z) {}
	inline vector3(const vector2 &v, const quad &z) : x(v.x), y(v.y), z(z) {}
	vector3(const struct vector4 &v);

	inline vector3(const float3 &a, const float3 &b, const float3 &c, const float3 &d) :
		x(a.x, b.x, c.x, d.x), y(a.y, b.y, c.y, d.y), z(a.z, b.z, c.z, d.z) { }

	inline vector3 operator +() const { return *this; }
	inline vector3 operator -() const { return vector3(-x, -y, -z); }

	inline operator quad *() { return &x; }
	inline operator const quad *() const { return &x; }
};

inline vector3 operator +(const vector3 &v1, const vector3 &v2) { return vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
inline vector3 operator -(const vector3 &v1, const vector3 &v2) { return vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
inline vector3 operator *(const vector3 &v1, const vector3 &v2) { return vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z); }
inline vector3 operator /(const vector3 &v1, const vector3 &v2) { return vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z); }

inline vector3 operator +(const vector3 &v, const quad &q) { return vector3(v.x + q, v.y + q, v.z + q); }
inline vector3 operator +(const quad &q, const vector3 &v) { return vector3(v.x + q, v.y + q, v.z + q); }
inline vector3 operator -(const vector3 &v, const quad &q) { return vector3(v.x - q, v.y - q, v.z - q); }
inline vector3 operator -(const quad &q, const vector3 &v) { return vector3(q - v.x, q - v.y, q - v.z); }
inline vector3 operator *(const vector3 &v, const quad &q) { return vector3(v.x * q, v.y * q, v.z * q); }
inline vector3 operator *(const quad &q, const vector3 &v) { return vector3(v.x * q, v.y * q, v.z * q); }
inline vector3 operator /(const vector3 &v, const quad &q) { quad inv = _mm_inverse_ps(q.f4); return vector3(v.x * inv, v.y * inv, v.z * inv); }
inline vector3 operator /(const quad &q, const vector3 &v) { return vector3(q / v.x, q / v.y, q / v.z); }

inline vector3 cross(const vector3 &v1, const vector3 &v2) { return vector3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x); }
inline quad dot(const vector3 &v1, const vector3 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
inline quad lengthsq(const vector3 &v) { return dot(v, v); }
inline quad length(const vector3 &v) { return sqrtf(lengthsq(v)); }
inline vector3 lerp(const vector3 &v1, const vector3 &v2, const quad &q) { return v1 + (v2 - v1) * q; }
inline vector3 step(const vector3 &v1, const vector3 &v2, const quad &q) { return vector3(step(v1.x, v2.x, q), step(v1.y, v2.y, q), step(v1.z, v2.z, q)); }
inline vector3 normalize(const vector3 &v) { return v * _mm_inverse_ps(length(v).f4); }
inline vector3 min(const vector3 &v1, const vector3 &v2) { return vector3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z)); }
inline vector3 max(const vector3 &v1, const vector3 &v2) { return vector3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z)); }
inline vector3 abs(const vector3 &v) { return vector3(abs(v.x), abs(v.y), abs(v.z)); }
inline vector3 inverse(const vector3 &v) { return vector3(inverse(v.x), inverse(v.y), inverse(v.z)); }

// 4-dimensional vector -------------------------------------------------------

struct vector4
{
	quad x, y, z, w;

	inline vector4() {}
	inline vector4(const quad &q) : x(q), y(q), z(q), w(q) {}
	inline vector4(const quad &x, const quad &y, const quad &z, const quad &w) : x(x), y(y), z(z), w(w) {}
	inline vector4(const vector2 &v, const quad &z, const quad &w) : x(v.x), y(v.y), z(z), w(w) {}
	inline vector4(const vector3 &v, const quad &w) : x(v.x), y(v.y), z(v.z), w(w) {}

	inline vector4(const float4 &a, const float4 &b, const float4 &c, const float4 &d) :
		x(a.x, b.x, c.x, d.x), y(a.y, b.y, c.y, d.y), z(a.z, b.z, c.z, d.z), w(a.w, b.w, c.w, d.w) { }

	inline vector4 operator +() const { return *this; }
	inline vector4 operator -() const { return vector4(-x, -y, -z, -w); }

	inline operator quad *() { return &x; }
	inline operator const quad *() const { return &x; }
};

inline vector4 operator +(const vector4 &v1, const vector4 &v2) { return vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
inline vector4 operator -(const vector4 &v1, const vector4 &v2) { return vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
inline vector4 operator *(const vector4 &v1, const vector4 &v2) { return vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w); }
inline vector4 operator /(const vector4 &v1, const vector4 &v2) { return vector4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w); }

inline vector4 operator +(const vector4 &v, const quad &q) { return vector4(v.x + q, v.y + q, v.z + q, v.w + q); }
inline vector4 operator +(const quad &q, const vector4 &v) { return vector4(v.x + q, v.y + q, v.z + q, v.w + q); }
inline vector4 operator -(const vector4 &v, const quad &q) { return vector4(v.x - q, v.y - q, v.z - q, v.w - q); }
inline vector4 operator -(const quad &q, const vector4 &v) { return vector4(q - v.x, q - v.y, q - v.z, q - v.w); }
inline vector4 operator *(const vector4 &v, const quad &q) { return vector4(v.x * q, v.y * q, v.z * q, v.w * q); }
inline vector4 operator *(const quad &q, const vector4 &v) { return vector4(v.x * q, v.y * q, v.z * q, v.w * q); }
inline vector4 operator /(const vector4 &v, const quad &q) { quad inv = _mm_inverse_ps(q.f4); return vector4(v.x * inv, v.y * inv, v.z * inv, v.w * inv); }
inline vector4 operator /(const quad &q, const vector4 &v) { return vector4(q / v.x, q / v.y, q / v.z, q / v.w); }

inline quad dot(const vector4 &v1, const vector4 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
inline quad lengthsq(const vector4 &v) { return dot(v, v); }
inline quad length(const vector4 &v) { return sqrtf(lengthsq(v)); }
inline vector4 lerp(const vector4 &v1, const vector4 &v2, const quad &q) { return v1 + (v2 - v1) * q; }
inline vector4 step(const vector4 &v1, const vector4 &v2, const quad &q) { return vector4(step(v1.x, v2.x, q), step(v1.y, v2.y, q), step(v1.z, v2.z, q), step(v1.w, v2.w, q)); }
inline vector4 normalize(const vector4 &v) { return v * _mm_inverse_ps(length(v).f4); }
inline vector4 min(const vector4 &v1, const vector4 &v2) { return vector4(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z), min(v1.w, v2.w)); }
inline vector4 max(const vector4 &v1, const vector4 &v2) { return vector4(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z), max(v1.w, v2.w)); }
inline vector4 abs(const vector4 &v) { return vector4(abs(v.x), abs(v.y), abs(v.z), abs(v.w)); }
inline vector4 inverse(const vector4 &v) { return vector4(inverse(v.x), inverse(v.y), inverse(v.z), inverse(v.w)); }

// ----------------------------------------------------------------------------

inline vector2::vector2(const struct vector3 &v) : x(v.x), y(v.y) { }
inline vector2::vector2(const struct vector4 &v) : x(v.x), y(v.y) { }

inline vector3::vector3(const struct vector4 &v) : x(v.x), y(v.y), z(v.z) { }

#endif // VECTORS_H
