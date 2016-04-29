#pragma once
#include "Types.h"



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


inline U32 String_HashFunction(char* key, U32 numberOfIndices)
{
	U32 hash = 0;

	char* keyI = key;
	while (*keyI)
	{
		++keyI;
		// NOTE: How is this better ????
		//result += (U32)*keyI % numberOfIndices;
		hash = (31 * hash + (*keyI)) % numberOfIndices;
	}

	return hash;
}