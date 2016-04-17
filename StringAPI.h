#pragma once
// NOTE: At this point this is basically a shell

#include "Types.h"

/*
 * Length counts the terminating character.
 */
inline size_t Length(const char* s)
{
	size_t count = 1;

	const char* sT = s;
	while (*sT)
	{
		++count;
		++sT;
	}

	return count;
}

// NOTE: If you miss the which string was different and at which character add it back later.
inline bool Compare(const char* s1, const char* s2)
{
	bool match = true;
	size_t position = 0;

	const char* s1T = s1;
	const char* s2T = s2;
	while (*s1T && *s2T)
	{
		if (*s1T != *s2T)
		{
			match = false;
			break;
		}

		++position;
		++s1T;
		++s2T;
	}
	if (match && (*s1T != NULL || *s2T != NULL))
	{
		match = false;
	}

	return match;
}

inline bool Compare(const char* s1, size_t s1Length, const char* s2, size_t s2Length)
{
	bool match = false;

	if (s1Length == s2Length)
	{
		match = Compare(s1, s2);
	}

	return match;
}

inline char* Copy(const char* s, size_t sLength)
{
	char* copy = (char*)malloc(sizeof(char)* sLength);

	for (size_t i = 0; i < sLength; ++i)
	{
		copy[i] = s[i];
	}

	return copy;
}

inline char* Copy(const char* s)
{
	size_t sLength = Length(s);
	char* copy = Copy(s, sLength);
	return copy;
}

inline char* Concat(const char* s1, size_t s1Length, const char* s2, size_t s2Length)
{
	char* result;

	size_t combinedLength = s1Length + s2Length - 1;
	result = (char*)malloc(sizeof(char) * combinedLength);

	size_t s1LengthMinusOne = s1Length - 1;
	for (size_t i = 0; i < s1LengthMinusOne; ++i)
	{
		result[i] = s1[i];
	}
	for (size_t i = 0; i < s2Length; ++i)
	{
		result[s1LengthMinusOne + i] = s2[i];
	}

	return result;
}

inline char* Concat(const char* s1, const char* s2)
{
	size_t s1Length = Length(s1);
	size_t s2Length = Length(s2);
	char* result = Concat(s1, s1Length, s2, s2Length);
	return result;
}

/*
 * Inserts AFTER
 * counting starts at 0
 */
inline char* Insert(const char* s, size_t sLength, size_t insertionIndex, const char* s2, size_t s2Length)
{
	size_t combinedLength = sLength + s2Length - 1;
	char* result = (char*)malloc(sizeof(char) * combinedLength);

	size_t insertionIndexPlusOne = insertionIndex + 1;
	size_t s2LengthMinusOne = s2Length - 1;

	for (size_t i = 0; i < insertionIndexPlusOne; ++i)
	{
		result[i] = s[i];
	}
	for (size_t i = 0; i < s2Length - 1; ++i)
	{
		result[insertionIndexPlusOne + i] = s2[i];
	}
	for (size_t i = 0; i < sLength - insertionIndex - 1; ++i)
	{
		result[insertionIndexPlusOne + s2LengthMinusOne + i] = s[insertionIndexPlusOne + i];
	}

	return result;
}

inline char* Insert(const char* s, size_t insertionPosition, const char* s2)
{
	size_t sLength = Length(s);
	size_t s2Length = Length(s2);
	char* result = Insert(s, sLength, insertionPosition, s2, s2Length);
	return result;
}


/*
 * Erases AFTER
 * counting starts at 0
 * if the amount to erase is greater than the remaining length of the string
 *	the rest of the string will be erased.
 */
inline char* Erase(const char* s, size_t sLength, size_t erasePosition, size_t eraseCount)
{
	if (erasePosition + eraseCount > sLength - 1)
	{
		eraseCount = sLength - 2 - erasePosition;
	}
	char* result = (char*)malloc(sizeof(char) * (sLength - eraseCount));

	size_t erasePositionPlusOne = erasePosition + 1;

	for (size_t i = 0; i < erasePositionPlusOne; ++i)
	{
		result[i] = s[i];
	}
	for (size_t i = 0; i < sLength - erasePositionPlusOne - eraseCount; ++i)
	{
		result[erasePositionPlusOne + i] = s[erasePositionPlusOne + eraseCount + i];
	}

	return result;
}

inline char* Erase(const char* s, size_t erasePosition, size_t eraseCount)
{
	size_t sLength = Length(s);
	char* result = Erase(s, sLength, erasePosition, eraseCount);
	return result;
}

/*
 * Starting from the beginning of the string converts the string 
 *	to an integer.
 * Stops when it hits its first non numeric value.
 * If the first character encountered is non numeric returns 0
 */
inline I32 ToI32(const char* s)
{
	I32 value = 0;

	bool isNegative = false;
	const char* sT = s;
	if (s[0] == '-')
	{
		isNegative = true;
		++sT;
	}

	while (*sT && *sT >= '0' && *sT <= '9')
	{
		value = (10 * value) + (*sT - '0');
		++sT;
	}

	if (isNegative)
	{
		value *= -1;
	}

	return value;
}

/* 
 * Negative numbers are not processed so the value returned is 0
 */
inline U32 ToU32(const char* s)
{
	U32 value = 0;

	if (s[0] != '-')
	{
		const char* sT = s;
		while (*sT && *sT <= '9')
		{
			value = (10 * value) + (*sT - '0');
			++sT;
		}	
	}

	return value;
}

inline F32 ToF32(const char* s)
{
	F32 value;

	// Whole Part
	F32 wholeNumber = 0;
	bool isNegative = false;
	const char* sT = s;
	if (s[0] == '-')
	{
		isNegative = true;
		++sT;
	}

	while (*sT && *sT >= '0' && *sT <= '9')
	{
		wholeNumber = (10 * wholeNumber) + (*sT - '0');
		++sT;
	}

	// Fractional Part
	F32 fractionalNumber = 0;
	F32 divisor = 1;
	if (*sT == '.')
	{
		++sT;

		while (*sT && *sT >= '0' && *sT <= '9')
		{
			fractionalNumber = (10 * fractionalNumber) + (*sT - '0');
			divisor *= 10;
			++sT;
		}
	}

	value = wholeNumber + fractionalNumber/divisor;

	if (isNegative)
	{
		value *= -1;
	}

	return value;
}

inline char* ToString(I32 i)
{
	char* string;

	U32 iT = i;
	size_t numberOfDigits = 0;
	char reverseString[12];
	bool isNegative = false;
	if (i < 0)
	{
		isNegative = true;
		iT = i * -1;
	}

	if (i == 0)
	{
		reverseString[numberOfDigits++] = '0';
	}
	else
	{
		while (iT > 0)
		{
			reverseString[numberOfDigits++] = (iT % 10) + '0';
			iT /= 10;
		}
	}

	size_t stringLength = numberOfDigits + (isNegative ? 1 : 0) + 1;
	string = (char*)malloc(sizeof(char) * stringLength);

	size_t offset = 0;
	if (isNegative)
	{
		string[0] = '-';
		offset = 1;
	}
	for (size_t i = 0; i < numberOfDigits; ++i)
	{
		string[offset + i] = reverseString[numberOfDigits - 1 - i];
	}
	string[stringLength - 1] = '\0';

	return string;
}

inline char* ToString(U32 u)
{
	char* string;

	U32 uT = u;
	size_t numberOfDigits = 0;
	char reverseString[12];

	if (u == 0)
	{
		reverseString[numberOfDigits++] = '0';
	}
	else
	{
		while (uT > 0)
		{
			reverseString[numberOfDigits++] = (uT % 10) + '0';
			uT /= 10;
		}
	}

	size_t stringLength = numberOfDigits + 1;
	string = (char*)malloc(sizeof(char)* stringLength);

	for (size_t i = 0; i < numberOfDigits; ++i)
	{
		string[i] = reverseString[numberOfDigits - 1 - i];
	}
	string[stringLength - 1] = '\0';

	return string;
}

inline char* ToString(F32 f, U32 numberOfDecimals)
{
	char* string;

	I32 pre = (I32)f;
	char* preString = ToString(pre);
	char* prePlusDot = Concat(preString, ".");
	//string = Concat(preString, ".");

	// NOTE: post^numberOfDecimals per chunk?
	//	commented out section implements this with chunk size = 6
	//	would need to implement a way to specify the number of digits for integers
	//	for this method to work.
	F32 post = f - pre;

	/*char* tString1;
	char* tString2;
	I32 nodRemaining = numberOfDecimals;
	F32 pt = post;
	while (nodRemaining - 6 > 0)
	{
		nodRemaining -= 6;
		pt *= pow(10, 6);
		I32 intpt = (I32)pt;
		tString1 = ToString(intpt);
		tString2 = Concat(prePlusDot, tString1);
		free(prePlusDot);
		free(tString1);
		prePlusDot = tString2;
		pt = pt - intpt;
	}
	pt *= pow(10, nodRemaining);
	char* fin = ToString((I32)pt);
	string = Concat(prePlusDot, fin);
	free(fin);
	free(prePlusDot);
	return string;*/

	F32 t = post;
	char* postString = (char*)malloc(sizeof(char) * (numberOfDecimals + 1));
	for (U32 i = 0; i < numberOfDecimals; ++i)
	{
		t = t * 10;
		I32 intT = (I32)t;
		postString[i] = intT + '0';
		t = t - intT;
	}
	postString[numberOfDecimals] = '\0';


	string = Concat(prePlusDot, postString);
	free(preString);
	free(prePlusDot);
	free(postString);
	return string;
}


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

inline char* I32ToString(I32 number)
{
	char* result;
	result = (char*)malloc(sizeof(char) * 64);

	result = itoa(number, result, 10);
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