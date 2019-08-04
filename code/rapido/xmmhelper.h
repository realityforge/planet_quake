
#ifndef XMMHELPER
#define XMMHELPER

#include <float.h>
#include <emmintrin.h>
#include <xmmintrin.h>

const __m128 Zero4 = _mm_setzero_ps();
const __m128 One4 = _mm_set1_ps(1.0f);
const __m128 Infinity4 = _mm_set1_ps(Infinity);
const unsigned int NoneMask = 0x0;
const unsigned int AllMask = 0xf;

inline void *sse_alloc(size_t size) { return _mm_malloc(size, 16); }
inline void sse_free(void *p) { return _mm_free(p); } 

inline void *sse_alignptr(void *ptr)
{
	char *mem = reinterpret_cast<char *>(ptr);
	mem += (~reinterpret_cast<long>(mem) & 0xf) + 1; // align for __m128 to 16 bytes
	return static_cast<void *>(mem);
}

#define sse_alloca(size) sse_alignptr(alloca((size) + 16))

// computes the inverse of a vector: reciprocal function + one Newton-Raphson iteration for refinement
inline __m128 _mm_inverse_ps(__m128 v)
{
	const __m128 rcp = _mm_rcp_ps(v), mul = _mm_mul_ps(_mm_mul_ps(rcp, rcp), v);
	return _mm_sub_ps(_mm_add_ps(rcp, rcp), _mm_andnot_ps(_mm_cmpeq_ps(v, Zero4), mul)); 
}

inline __m128 _mm_sel_ps_xor(__m128 a, __m128 b, __m128 mask)
{
	// (((b ^ a) & mask)^a) // Source: http://mark.santaniello.net/archives/315
	return _mm_xor_ps(a, _mm_and_ps(mask, _mm_xor_ps(b, a)));
}

inline __m128i _mm_sel_si128_xor(__m128i a, __m128i b, __m128 mask)
{
	// (((b ^ a) & mask)^a) // Source: http://mark.santaniello.net/archives/315
	union { __m128 f4; __m128i i4; } temp; temp.f4 = mask;
	return _mm_xor_si128(a, _mm_and_si128(temp.i4, _mm_xor_si128(b, a)));
}

#endif // XMMHELPER
