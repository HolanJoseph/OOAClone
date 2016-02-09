#include "Types.h"

void setBit(U8* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

void setBit(U16* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

void setBit(U32* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

void setBit(U64* buffer, U8 code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & ((U64)1 << code));
}



bool isBitSet(U8* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

bool isBitSet(U16* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

bool isBitSet(U32* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

bool isBitSet(U64* buffer, U8 code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}