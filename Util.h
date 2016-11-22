#pragma once

#include "Types.h"
#include "Math.h"


inline F32 ClampRange_F32(F32 val, F32 low, F32 high)
{
	F32 result = val;

	if (val < low)
	{
		result = low;
	}
	else if (val > high)
	{
		result = high;
	}

	return result;
}

inline I32 ClampRange_I32(I32 val, I32 low, I32 high)
{
	I32 result = val;

	if (val < low)
	{
		result = low;
	}
	else if (val > high)
	{
		result = high;
	}

	return result;
}

inline U32 ClampRange_U32(U32 val, U32 low, U32 high)
{
	U32 result = val;

	if (val < low)
	{
		result = low;
	}
	else if (val > high)
	{
		result = high;
	}

	return result;
}

// NOTE: This should not cause issues with the string hash function but if there is weird hashing issues check this.
inline U32 String_HashFunction(const char* key, U32 numberOfIndices)
{
	U32 hash = 0;

	const char* keyI = key;
	while (*keyI)
	{
		++keyI;
		// NOTE: How is this better ????
		//result += (U32)*keyI % numberOfIndices;
		hash = (31 * hash + (*keyI)) % numberOfIndices;
	}

	return hash;
}

inline vec2 Lerp(vec2 start, vec2 end, F32 t)
{
	vec2 result;
	result = ((1.0f - t) * start) + (t * end);
	return result;
}

inline vec2 LerpClamped(vec2 start, vec2 end, F32 t)
{
	F32 clampedt = ClampRange_F32(t, 0.0f, 1.0f);
	vec2 result;
	result = Lerp(start, end, clampedt);
	return result;
}

struct Rect_CD;
struct Rect_CHD;
struct Rect_ULD;

struct Rect_CD 
{
	vec2 center;
	vec2 dimensions;

	Rect_CD()
	{
		this->center = vec2(0.0f, 0.0f);
		this->dimensions = vec2(1.0f, 1.0f);
	}

	Rect_CD(vec2 center, vec2 dimensions)
	{
		this->center = center;
		this->dimensions = dimensions;
	}

	Rect_CD(Rect_CHD rect);
	Rect_CD(Rect_ULD rect);
};

struct Rect_CHD
{
	vec2 center;
	vec2 halfDimensions;

	Rect_CHD()
	{
		this->center = vec2(0.0f, 0.0f);
		this->halfDimensions = vec2(0.5f, 0.5f);
	}

	Rect_CHD(vec2 center, vec2 halfDimensions)
	{
		this->center = center;
		this->halfDimensions = halfDimensions;
	}

	Rect_CHD(Rect_CD rect);
	Rect_CHD(Rect_ULD rect);
};

struct Rect_ULD 
{
	vec2 upperLeft;
	vec2 dimensions;

	Rect_ULD()
	{
		this->upperLeft = vec2(-0.5f, 0.5f);
		this->dimensions = vec2(1.0f, 1.0f);
	}

	Rect_ULD(vec2 upperLeft, vec2 dimensions)
	{
		this->upperLeft = upperLeft;
		this->dimensions = dimensions;
	}

	Rect_ULD(Rect_CD rect);
	Rect_ULD(Rect_CHD rect);
};

inline Rect_CD::Rect_CD(Rect_CHD rect)
{
	this->center = rect.center;
	this->dimensions = rect.halfDimensions * 2.0f;
}

inline Rect_CD::Rect_CD(Rect_ULD rect)
{
	vec2 halfDimensions = rect.dimensions / 2.0f;

	this->center = rect.upperLeft + vec2(halfDimensions.x, -halfDimensions.y);
	this->dimensions = rect.dimensions;
}

inline Rect_CHD::Rect_CHD(Rect_CD rect)
{
	this->center = rect.center;
	this->halfDimensions = rect.dimensions / 2.0f;
} 

inline Rect_CHD::Rect_CHD(Rect_ULD rect)
{
	vec2 halfDimensions = rect.dimensions / 2.0f;

	this->center = rect.upperLeft + vec2(halfDimensions.x, -halfDimensions.y);
	this->halfDimensions = halfDimensions;
}

inline Rect_ULD::Rect_ULD(Rect_CD rect)
{
	vec2 halfDimensions = rect.dimensions / 2.0f;

	this->upperLeft = rect.center + vec2(-halfDimensions.x, halfDimensions.y);
	this->dimensions = rect.dimensions;
}

inline Rect_ULD::Rect_ULD(Rect_CHD rect)
{
	this->upperLeft = rect.center + vec2(-rect.halfDimensions.x, rect.halfDimensions.y);
	this->dimensions = rect.halfDimensions * 2.0f;
}