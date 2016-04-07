#pragma once
// NOTE: At this point this is basically a shell

#include "Types.h"

#include <cstdlib>

inline I32 StringToI32(char* string)
{
	I32 result = atoi(string);
	return result;
}

inline U32 StringToU32(char* string)
{
	U32 result = strtoul(string, NULL, 0);
	return result;
}

inline F32 StringToF32(char* string)
{
	F32 result = strtof(string, NULL);
	return result;
}

inline char** SplitStringOnCharacter(char* string, size_t stringLength, char splitCharacter, U64* out_numberOfSplits, U64** out_lineLengths)
{
	char** stringComponents = (char**)malloc(sizeof(char*) * stringLength);
	U64* stringComponentLengths = (U64*)malloc(sizeof(U64) * stringLength);

	char* stringFront = string;
	size_t numberOfSplits = 0;
	for (U64 i = 0; i < stringLength; ++i)
	{
		if (string[i] == splitCharacter)
		{
			string[i] = '\0';
			stringComponents[numberOfSplits] = stringFront;
			stringComponentLengths[numberOfSplits] = (&(string[i]) - stringFront + 1) / sizeof(char);
			stringFront = &(string[i + 1]);
			++numberOfSplits;
		}
	}

	stringComponents[numberOfSplits] = stringFront;
	stringComponentLengths[numberOfSplits] = (&(string[stringLength]) - stringFront) / sizeof(char);
	++numberOfSplits;

	stringComponents = (char**)realloc(stringComponents, sizeof(char*) * numberOfSplits);
 	stringComponentLengths = (U64*)realloc(stringComponentLengths, sizeof(U64) * numberOfSplits);

	*out_numberOfSplits = numberOfSplits;
	*out_lineLengths = stringComponentLengths;
	return stringComponents;
}