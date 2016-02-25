#pragma once
#include "Types.h"

inline void SetBit(U8* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

inline void SetBit(U16* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

inline void SetBit(U32* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

inline void SetBit(U64* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & ((U64)1 << code));
}



inline bool IsBitSet(U8* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

inline bool IsBitSet(U16* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

inline bool IsBitSet(U32* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

inline bool IsBitSet(U64* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}