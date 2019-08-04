
#ifndef MATRIX_H
#define MATRIX_H

#include "floats.h"

// Identity functions ---------------------------------------------------------

inline float2x2 identity2x2()
{
	return float2x2(1.0f, 0.0f,
			0.0f, 1.0f);
}

inline float3x3 identity3x3()
{
	return float3x3(1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f);
}

inline float4x4 identity4x4()
{
	return float4x4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}

// ----------------------------------------------------------------------------

inline float3x3 rotatex3x3(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float3x3(1.0f, 0.0f, 0.0f,
			0.0f, cosr, -sinr,
			0.0f, sinr, cosr);
}
inline float3x3 rotatey3x3(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float3x3(cosr, 0.0f, sinr,
			0.0f, 1.0f, 0.0f,
			-sinr, 0.0f, cosr);
}
inline float3x3 rotatez3x3(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float3x3(cosr, -sinr, 0.0f,
			sinr, cosr, 0.0f,
			0.0f, 0.0f, 1.0f);
}

inline float3x3 rotate3x3(float yaw, float pitch, float roll)
{
	return rotatey3x3(yaw) * rotatex3x3(pitch) * rotatez3x3(roll);
}

inline float3x3 rotateaxis3x3(const float3 &a, float r)
{
	//float3 a = normalize(axis);
	float sinr = sinf(r), cosr = cosf(r), invcosr = 1.0f - cosr;
	return float3x3(invcosr * a.x * a.x + cosr, invcosr * a.x * a.y + a.z * sinr, invcosr * a.x * a.z - a.y * sinr,
			invcosr * a.x * a.y - a.z * sinr, invcosr * a.y * a.y + cosr, invcosr * a.y * a.z + a.x * sinr,
			invcosr * a.x * a.z + a.y * sinr, invcosr * a.y * a.z - a.x * sinr, invcosr * a.z * a.z + cosr);
}

inline float3x3 scale3x3(const float3 &s)
{
	return float3x3(s.x, 0.0f, 0.0f,
			0.0f, s.y, 0.0f,
			0.0f, 0.0f, s.z);
}

// ----------------------------------------------------------------------------

inline float4x4 rotatex4x4(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float4x4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cosr, -sinr, 0.0f,
			0.0f, sinr, cosr, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}
inline float4x4 rotatey4x4(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float4x4(cosr, 0.0f, sinr, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-sinr, 0.0f, cosr, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}
inline float4x4 rotatez4x4(float r)
{
	float sinr = sinf(r), cosr = cosf(r);
	return float4x4(cosr, -sinr, 0.0f, 0.0f,
			sinr, cosr, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 rotate4x4(float yaw, float pitch, float roll)
{
	return rotatey4x4(yaw) * rotatex4x4(pitch) * rotatez4x4(roll);
}

inline float4x4 rotateaxis4x4(const float3 &a, float r)
{
	//float3 a = normalize(axis);
	float sinr = sinf(r), cosr = cosf(r), invcosr = 1.0f - cosr;
	return float4x4(invcosr * a.x * a.x + cosr, invcosr * a.x * a.y + a.z * sinr, invcosr * a.x * a.z - a.y * sinr, 0.0f,
			invcosr * a.x * a.y - a.z * sinr, invcosr * a.y * a.y + cosr, invcosr * a.y * a.z + a.x * sinr, 0.0f,
			invcosr * a.x * a.z + a.y * sinr, invcosr * a.y * a.z - a.x * sinr, invcosr * a.z * a.z + cosr, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 scale4x4(const float3 &s)
{
	return float4x4(s.x, 0.0f, 0.0f, 0.0f,
			0.0f, s.y, 0.0f, 0.0f,
			0.0f, 0.0f, s.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 translate4x4(const float3 &t)
{
	return float4x4(1.0f, 0.0f, 0.0f, t.x,
			0.0f, 1.0f, 0.0f, t.y,
			0.0f, 0.0f, 1.0f, t.z,
			0.0f, 0.0f, 0.0f, 1.0f);
}

// ----------------------------------------------------------------------------

inline float4x4 lookatlh4x4(const float3 &eye, const float3 &at, const float3 &up)
{
	float3 za = normalize(at - eye), xa = normalize(cross(up, za)), ya = cross(za, xa);
	return float4x4(xa.x, xa.y, xa.z, -dot(xa, eye),
			ya.x, ya.y, ya.z, -dot(ya, eye),
			za.x, za.y, za.z, -dot(za, eye),
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 lookatrh4x4(const float3 &eye, const float3 &at, const float3 &up)
{
	float3 za = normalize(eye - at), xa = normalize(cross(up, za)), ya = cross(za, xa);
	return float4x4(xa.x, xa.y, xa.z, -dot(xa, eye),
			ya.x, ya.y, ya.z, -dot(ya, eye),
			za.x, za.y, za.z, -dot(za, eye),
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 perspectivefovlh4x4(float fov, float aspect, float znear, float zfar)
{
	float h = 1.0f / tanf(fov * 0.5f), w = h / aspect, d = zfar / (zfar - znear);
	return float4x4(w, 0.0f, 0.0f, 0.0f,
			0.0f, h, 0.0f, 0.0f,
			0.0f, 0.0f, d, -znear * d,
			0.0f, 0.0f, 1.0f, 0.0f);
}

inline float4x4 perspectivefovrh4x4(float fov, float aspect, float znear, float zfar)
{
	float h = 1.0f / tanf(fov * 0.5f), w = h / aspect, d = zfar / (znear - zfar);
	return float4x4(w, 0.0f, 0.0f, 0.0f,
			0.0f, h, 0.0f, 0.0f,
			0.0f, 0.0f, d, znear * d,
			0.0f, 0.0f, -1.0f, 0.0f);
}

inline float4x4 orthooffcenterlh4x4(float left, float right, float bottom, float top, float near, float far)
{
	return float4x4(2.0f / (right - left), 0.0f, 0.0f, (left + right) / (left - right),
			0.0f, 2.0f / (top - bottom), 0.0f, (bottom + top) / (bottom - top),
			0.0f, 0.0f, 1.0f / (far - near), near / (near - far),
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 orthooffcenterrh4x4(float left, float right, float bottom, float top, float near, float far)
{
	return float4x4(2.0f / (right - left), 0.0f, 0.0f, (left + right) / (left - right),
			0.0f, 2.0f / (top - bottom), 0.0f, (bottom + top) / (bottom - top),
			0.0f, 0.0f, 1.0f / (near - far), near / (near - far),
			0.0f, 0.0f, 0.0f, 1.0f);
}

inline float4x4 ortholh4x4(float width, float height, float near, float far)
{
	return orthooffcenterlh4x4(width * -0.5f, height * 0.5f, height * -0.5f, height * 0.5f, near, far);
}

inline float4x4 orthorh4x4(float width, float height, float near, float far)
{
	return orthooffcenterrh4x4(width * -0.5f, height * 0.5f, height * -0.5f, height * 0.5f, near, far);
}

#endif // MATRIX_H
