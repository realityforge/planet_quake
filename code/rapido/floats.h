
#ifndef FLOATS
#define FLOATS

#define _USE_MATH_DEFINES
#include <float.h>
#include <math.h>

inline float clamp(float f, float min, float max) { return (f < min) ? min : ((f > max) ? max : f); }
inline float saturate(float f) { return clamp(f, 0.0f, 1.0f); }

inline float lerp(float f1, float f2, float f) { return f1 + (f2 - f1) * f; }
inline float min(float f1, float f2) { return f1 < f2 ? f1 : f2; }
inline float max(float f1, float f2) { return f1 > f2 ? f1 : f2; }
inline float sign(float f) { return (f == 0.0f) ? 0.0f : ((f > 0.0f) ? 1.0f : -1.0f); }

inline int min(int i1, int i2) { return i1 < i2 ? i1 : i2; }
inline int max(int i1, int i2) { return i1 > i2 ? i1 : i2; }
inline unsigned long min(unsigned long i1, unsigned long i2) { return i1 < i2 ? i1 : i2; }
inline unsigned long max(unsigned long i1, unsigned long i2) { return i1 > i2 ? i1 : i2; }
inline long min(long i1, long i2) { return i1 < i2 ? i1 : i2; }
inline long max(long i1, long i2) { return i1 > i2 ? i1 : i2; }

const float Infinity = FLT_MAX;

// Vectors --------------------------------------------------------------------

// 2-dimensional vector -------------------------------------------------------

struct float2
{
	float x, y;

	inline float2() {}
	inline float2(float f) : x(f), y(f) {}
	inline float2(float x, float y) : x(x), y(y) {}
	float2(const struct float3 &v);
	float2(const struct float4 &v);

	inline float2 operator +() const { return *this; }
	inline float2 operator -() const { return float2(-x, -y); }

	inline operator float *() { return &x; }
	inline operator const float *() const { return &x; }
};

inline float2 operator +(const float2 &v1, const float2 &v2) { return float2(v1.x + v2.x, v1.y + v2.y); }
inline float2 operator -(const float2 &v1, const float2 &v2) { return float2(v1.x - v2.x, v1.y - v2.y); }
inline float2 operator *(const float2 &v1, const float2 &v2) { return float2(v1.x * v2.x, v1.y * v2.y); }
inline float2 operator /(const float2 &v1, const float2 &v2) { return float2(v1.x / v2.x, v1.y / v2.y); }

inline float2 operator +(const float2 &v, float f) { return float2(v.x + f, v.y + f); }
inline float2 operator +(float f, const float2 &v) { return float2(v.x + f, v.y + f); }
inline float2 operator -(const float2 &v, float f) { return float2(v.x - f, v.y - f); }
inline float2 operator -(float f, const float2 &v) { return float2(f - v.x, f - v.y); }
inline float2 operator *(const float2 &v, float f) { return float2(v.x * f, v.y * f); }
inline float2 operator *(float f, const float2 &v) { return float2(v.x * f, v.y * f); }
inline float2 operator /(const float2 &v, float f) { float inv = 1.0f / f; return float2(v.x * inv, v.y * inv); }
inline float2 operator /(float f, const float2 &v) { return float2(f / v.x, f / v.y); }

inline float dot(const float2 &v1, const float2 &v2) { return v1.x * v2.x + v1.y * v2.y; }
inline float lengthsq(const float2 &v) { return dot(v, v); }
inline float length(const float2 &v) { return sqrtf(lengthsq(v)); }
inline float2 lerp(const float2 &v1, const float2 &v2, float f) { return v1 + (v2 - v1) * f; }
inline float2 normalize(const float2 &v) { return v * (1.0f / length(v)); }
inline float2 min(const float2 &v1, const float2 &v2) { return float2(min(v1.x, v2.x), min(v1.y, v2.y)); }
inline float2 max(const float2 &v1, const float2 &v2) { return float2(max(v1.x, v2.x), max(v1.y, v2.y)); }
inline float2 abs(const float2 &v) { return float2(fabs(v.x), fabs(v.y)); }


// 3-dimensional vector -------------------------------------------------------

struct float3
{
	float x, y, z;

	inline float3() {}
	inline float3(float f) : x(f), y(f), z(f) {}
	inline float3(float x, float y, float z) : x(x), y(y), z(z) {}
	inline float3(const float2 &v, float z) : x(v.x), y(v.y), z(z) {}
	float3(const struct float4 &v);

	inline float3 operator +() const { return *this; }
	inline float3 operator -() const { return float3(-x, -y, -z); }

	inline operator float *() { return &x; }
	inline operator const float *() const { return &x; }
};

inline float3 operator +(const float3 &v1, const float3 &v2) { return float3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
inline float3 operator -(const float3 &v1, const float3 &v2) { return float3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
inline float3 operator *(const float3 &v1, const float3 &v2) { return float3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z); }
inline float3 operator /(const float3 &v1, const float3 &v2) { return float3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z); }

inline float3 operator +(const float3 &v, float f) { return float3(v.x + f, v.y + f, v.z + f); }
inline float3 operator +(float f, const float3 &v) { return float3(v.x + f, v.y + f, v.z + f); }
inline float3 operator -(const float3 &v, float f) { return float3(v.x - f, v.y - f, v.z - f); }
inline float3 operator -(float f, const float3 &v) { return float3(f - v.x, f - v.y, f - v.z); }
inline float3 operator *(const float3 &v, float f) { return float3(v.x * f, v.y * f, v.z * f); }
inline float3 operator *(float f, const float3 &v) { return float3(v.x * f, v.y * f, v.z * f); }
inline float3 operator /(const float3 &v, float f) { float inv = 1.0f / f; return float3(v.x * inv, v.y * inv, v.z * inv); }
inline float3 operator /(float f, const float3 &v) { return float3(f / v.x, f / v.y, f / v.z); }

inline float3 cross(const float3 &v1, const float3 &v2) { return float3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x); }
inline float dot(const float3 &v1, const float3 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
inline float lengthsq(const float3 &v) { return dot(v, v); }
inline float length(const float3 &v) { return sqrtf(lengthsq(v)); }
inline float3 lerp(const float3 &v1, const float3 &v2, float f) { return v1 + (v2 - v1) * f; }
inline float3 normalize(const float3 &v) { return v * (1.0f / length(v)); }
inline float3 min(const float3 &v1, const float3 &v2) { return float3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z)); }
inline float3 max(const float3 &v1, const float3 &v2) { return float3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z)); }
inline float3 abs(const float3 &v) { return float3(fabs(v.x), fabs(v.y), fabs(v.z)); }

// 4-dimensional vector -------------------------------------------------------

struct float4
{
	float x, y, z, w;

	inline float4() {}
	inline float4(float f) : x(f), y(f), z(f), w(f) {}
	inline float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	inline float4(const float2 &v, float z, float w) : x(v.x), y(v.y), z(z), w(w) {}
	inline float4(const float3 &v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

	inline float4 operator +() const { return *this; }
	inline float4 operator -() const { return float4(-x, -y, -z, -w); }

	inline operator float *() { return &x; }
	inline operator const float *() const { return &x; }
};

inline float4 operator +(const float4 &v1, const float4 &v2) { return float4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
inline float4 operator -(const float4 &v1, const float4 &v2) { return float4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
inline float4 operator *(const float4 &v1, const float4 &v2) { return float4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w); }
inline float4 operator /(const float4 &v1, const float4 &v2) { return float4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w); }

inline float4 operator +(const float4 &v, float f) { return float4(v.x + f, v.y + f, v.z + f, v.w + f); }
inline float4 operator +(float f, const float4 &v) { return float4(v.x + f, v.y + f, v.z + f, v.w + f); }
inline float4 operator -(const float4 &v, float f) { return float4(v.x - f, v.y - f, v.z - f, v.w - f); }
inline float4 operator -(float f, const float4 &v) { return float4(f - v.x, f - v.y, f - v.z, f - v.w); }
inline float4 operator *(const float4 &v, float f) { return float4(v.x * f, v.y * f, v.z * f, v.w * f); }
inline float4 operator *(float f, const float4 &v) { return float4(v.x * f, v.y * f, v.z * f, v.w * f); }
inline float4 operator /(const float4 &v, float f) { float inv = 1.0f / f; return float4(v.x * inv, v.y * inv, v.z * inv, v.w * inv); }
inline float4 operator /(float f, const float4 &v) { return float4(f / v.x, f / v.y, f / v.z, f / v.w); }

inline float dot(const float4 &v1, const float4 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
inline float lengthsq(const float4 &v) { return dot(v, v); }
inline float length(const float4 &v) { return sqrtf(lengthsq(v)); }
inline float4 lerp(const float4 &v1, const float4 &v2, float f) { return v1 + (v2 - v1) * f; }
inline float4 normalize(const float4 &v) { return v * (1.0f / length(v)); }
inline float4 min(const float4 &v1, const float4 &v2) { return float4(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z), min(v1.w, v2.w)); }
inline float4 max(const float4 &v1, const float4 &v2) { return float4(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z), max(v1.w, v2.w)); }
inline float4 abs(const float4 &v) { return float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w)); }

// ----------------------------------------------------------------------------

inline float2::float2(const struct float3 &v) : x(v.x), y(v.y) { }
inline float2::float2(const struct float4 &v) : x(v.x), y(v.y) { }

inline float3::float3(const struct float4 &v) : x(v.x), y(v.y), z(v.z) { }

// Matrices -------------------------------------------------------------------

// 2x2-matrix -----------------------------------------------------------------

struct float2x2
{
	float2 s, t; // rows

	inline float2x2() {}
	inline float2x2(const float2 &s, const float2 &t) : s(s), t(t) {}
	inline float2x2(float sx, float sy, float tx, float ty) : s(sx, sy), t(tx, ty) {}
};

inline float2x2 transpose(const float2x2 &m) { return float2x2(float2(m.s.x, m.t.x), float2(m.s.y, m.t.y)); }

inline float2x2 operator *(const float2x2 &m1, const float2x2 &m2)
{
	float2x2 m3 = transpose(m2);
	return float2x2(float2(dot(m1.s, m3.s), dot(m1.s, m3.t)),
		float2(dot(m1.t, m3.s), dot(m1.t, m3.t)));
}

inline float2 operator *(const float2x2 &m, const float2 &v) { return float2(dot(m.s, v), dot(m.t, v)); }
inline float2x2 operator *(const float2x2 &m, float f) { return float2x2(m.s * f, m.t * f); }
inline float2x2 operator *(float f, const float2x2 &m) { return float2x2(m.s * f, m.t * f); }

inline float determinant(const float2x2 &m) { return m.s.x * m.t.y - m.s.y * m.t.x; }
inline float2x2 inverse(const float2x2 &m) { return float2x2(float2(m.t.y, -m.s.y), float2(-m.t.x, m.s.x)) * (1.0f / determinant(m)); }

// 3x3-matrix -----------------------------------------------------------------

struct float3x3
{
	float3 s, t, u; // rows

	inline float3x3() {}
	inline float3x3(const float3 &s, const float3 &t, const float3 &u) : s(s), t(t), u(u) {}
	inline float3x3(float sx, float sy, float sz, float tx, float ty, float tz,
		float ux, float uy, float uz) : s(sx, sy, sz), t(tx, ty, tz), u(ux, uy, uz) {}

	inline operator float3 *() { return &s; }
	inline operator const float3 *() const { return &s; }
};

inline float3x3 transpose(const float3x3 &m)
{
	return float3x3(float3(m.s.x, m.t.x, m.u.x), float3(m.s.y, m.t.y, m.u.y),
		float3(m.s.z, m.t.z, m.u.z));
}

inline float3x3 operator *(const float3x3 &m1, const float3x3 &m2)
{
	float3x3 m3 = transpose(m2);
	return float3x3(float3(dot(m1.s, m3.s), dot(m1.s, m3.t), dot(m1.s, m3.u)),
		float3(dot(m1.t, m3.s), dot(m1.t, m3.t), dot(m1.t, m3.u)),
		float3(dot(m1.u, m3.s), dot(m1.u, m3.t), dot(m1.u, m3.u)));
}

inline float3 operator *(const float3x3 &m, const float3 &v) { return float3(dot(m.s, v), dot(m.t, v), dot(m.u, v)); }
inline float3x3 operator *(const float3x3 &m, float f) { return float3x3(m.s * f, m.t * f, m.u * f); }
inline float3x3 operator *(float f, const float3x3 &m) { return float3x3(m.s * f, m.t * f, m.u * f); }

inline float determinant(const float3x3 &m)
{
	return m.s.x * determinant(float2x2(m.t.y, m.t.z, m.u.y, m.u.z))
		- m.s.y * determinant(float2x2(m.t.x, m.t.z, m.u.x, m.u.z))
		+ m.s.z * determinant(float2x2(m.t.x, m.t.y, m.u.x, m.u.y));
}

inline float3x3 inverse(const float3x3 &m)
{
	return float3x3(
		determinant(float2x2(m.t.y, m.t.z, m.u.y, m.u.z)),
		determinant(float2x2(m.s.z, m.s.y, m.u.z, m.u.y)),
		determinant(float2x2(m.s.y, m.s.z, m.t.y, m.t.z)),
		
		determinant(float2x2(m.t.z, m.t.x, m.u.z, m.u.x)),
		determinant(float2x2(m.s.x, m.s.z, m.u.x, m.u.z)),
		determinant(float2x2(m.s.z, m.s.x, m.t.z, m.t.x)),

		determinant(float2x2(m.t.x, m.t.y, m.u.x, m.u.y)),
		determinant(float2x2(m.s.y, m.s.x, m.u.y, m.u.x)),
		determinant(float2x2(m.s.x, m.s.y, m.t.x, m.t.y)))
		
		* (1.0f / determinant(m));
}

// 4x4-matrix -----------------------------------------------------------------

struct float4x4
{
	float4 s, t, u, v; // rows

	inline float4x4() {}
	inline float4x4(const float4 &s, const float4 &t, const float4 &u, const float4 &v) : s(s), t(t), u(u), v(v) {}
	inline float4x4(float sx, float sy, float sz, float sw, float tx, float ty, float tz, float tw,
		float ux, float uy, float uz, float uw, float vx, float vy, float vz, float vw) :
		s(sx, sy, sz, sw), t(tx, ty, tz, tw), u(ux, uy, uz, uw), v(vx, vy, vz, vw) {}

	inline operator float4 *() { return &s; }
	inline operator const float4 *() const { return &s; }
};

inline float4x4 transpose(const float4x4 &m)
{
	return float4x4(float4(m.s.x, m.t.x, m.u.x, m.v.x), float4(m.s.y, m.t.y, m.u.y, m.v.y),
		float4(m.s.z, m.t.z, m.u.z, m.v.z), float4(m.s.w, m.t.w, m.u.w, m.v.w));
}

inline float4x4 operator *(const float4x4 &m1, const float4x4 &m2)
{
	float4x4 m3 = transpose(m2);
	return float4x4(float4(dot(m1.s, m3.s), dot(m1.s, m3.t), dot(m1.s, m3.u), dot(m1.s, m3.v)),
		float4(dot(m1.t, m3.s), dot(m1.t, m3.t), dot(m1.t, m3.u), dot(m1.t, m3.v)),
		float4(dot(m1.u, m3.s), dot(m1.u, m3.t), dot(m1.u, m3.u), dot(m1.u, m3.v)),
		float4(dot(m1.v, m3.s), dot(m1.v, m3.t), dot(m1.v, m3.u), dot(m1.v, m3.v)));
}

inline float4 operator *(const float4x4 &m, const float4 &v) { return float4(dot(m.s, v), dot(m.t, v), dot(m.u, v), dot(m.v, v)); }
inline float4x4 operator *(const float4x4 &m, float f) { return float4x4(m.s * f, m.t * f, m.u * f, m.v * f); }
inline float4x4 operator *(float f, const float4x4 &m) { return float4x4(m.s * f, m.t * f, m.u * f, m.v * f); }

inline float determinant(const float4x4 &m)
{
	return m.s.x * determinant(float3x3(m.t.y, m.u.y, m.v.y, m.t.z, m.u.z, m.v.z, m.t.w, m.u.w, m.v.w))
		- m.s.y * determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.z, m.u.z, m.v.z, m.t.w, m.u.w, m.v.w))
		+ m.s.z * determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.y, m.u.y, m.v.y, m.t.w, m.u.w, m.v.w))
		- m.s.w * determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.y, m.u.y, m.v.y, m.t.z, m.u.z, m.v.z));
}

inline float4x4 inverse(const float4x4 &m)
{
	return float4x4(
		determinant(float3x3(m.t.y, m.u.y, m.v.y, m.t.z, m.u.z, m.v.z, m.t.w, m.u.w, m.v.w)),
		-determinant(float3x3(m.s.y, m.u.y, m.v.y, m.s.z, m.u.z, m.v.z, m.s.w, m.u.w, m.v.w)),
		determinant(float3x3(m.s.y, m.t.y, m.v.y, m.s.z, m.t.z, m.v.z, m.s.w, m.t.w, m.v.w)),
		-determinant(float3x3(m.s.y, m.t.y, m.u.y, m.s.z, m.t.z, m.u.z, m.s.w, m.t.w, m.u.w)),

		-determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.z, m.u.z, m.v.z, m.t.w, m.u.w, m.v.w)),
		determinant(float3x3(m.s.x, m.u.x, m.v.x, m.s.z, m.u.z, m.v.z, m.s.w, m.u.w, m.v.w)),
		-determinant(float3x3(m.s.x, m.t.x, m.v.x, m.s.z, m.t.z, m.v.z, m.s.w, m.t.w, m.v.w)),
		determinant(float3x3(m.s.x, m.t.x, m.u.x, m.s.z, m.t.z, m.u.z, m.s.w, m.t.w, m.u.w)),

		determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.y, m.u.y, m.v.y, m.t.w, m.u.w, m.v.w)),
		-determinant(float3x3(m.s.x, m.u.x, m.v.x, m.s.y, m.u.y, m.v.y, m.s.w, m.u.w, m.v.w)),
		determinant(float3x3(m.s.x, m.t.x, m.v.x, m.s.y, m.t.y, m.v.y, m.s.w, m.t.w, m.v.w)),
		-determinant(float3x3(m.s.x, m.t.x, m.u.x, m.s.y, m.t.y, m.u.y, m.s.w, m.t.w, m.u.w)),

		-determinant(float3x3(m.t.x, m.u.x, m.v.x, m.t.y, m.u.y, m.v.y, m.t.z, m.u.z, m.v.z)),
		determinant(float3x3(m.s.x, m.u.x, m.v.x, m.s.y, m.u.y, m.v.y, m.s.z, m.u.z, m.v.z)),
		-determinant(float3x3(m.s.x, m.t.x, m.v.x, m.s.y, m.t.y, m.v.y, m.s.z, m.t.z, m.v.z)),
		determinant(float3x3(m.s.x, m.t.x, m.u.x, m.s.y, m.t.y, m.u.y, m.s.z, m.t.z, m.u.z)))

		* (1.0f / determinant(m));
}

#endif // FLOATS
