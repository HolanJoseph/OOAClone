#pragma once
#include "Types.h"

inline F32 ClampRange(F32 val, F32 low, F32 high)
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