#pragma once

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
	bool match = true;

	if (s1Length == s2Length)
	{
		for (size_t i = 0; i < s1Length; ++i)
		{
			if (s1[i] != s2[i])
			{
				match = false;
				break;
			}
		}
	}
	else
	{
		match = false;
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

inline char* CopyAndTerminate(const char* s, size_t sLength)
{
	char* copy = (char*)malloc(sizeof(char)* (sLength + 1));

	for (size_t i = 0; i < sLength; ++i)
	{
		copy[i] = s[i];
	}
	copy[sLength] = '\0';

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
 *
 * This should probably be changed as trying to insert at the head of the string we are wrapping
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
 *
 * This should probably be changed as trying to erase at the head of the string we are wrapping
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

struct StringFindResult
{
	bool found;
	size_t position;
};
inline StringFindResult FindFirst(const char* s, char c)
{
	StringFindResult result = {false, 0};

	const char* sT = s;
	while (*sT)
	{
		if (*sT == c)
		{
			result.found = true;
			break;
		}
		++result.position;
		++sT;
	}

	return result;
}

inline StringFindResult FindLast(const char* s, const char c)
{
	StringFindResult result = {false, 0};

	size_t i = 0;
	const char* sT = s;
	while (*sT)
	{
		if (*sT == c)
		{
			result.found = true;
			result.position = i;
		}
		++i;
		++sT;
	}

	return result;
}

inline char* TrimWhitespaceFront(const char* s)
{
	char* result;

	if (s[0] == ' ')
	{
		const char* sT = s;
		size_t count = 0;
		while (*sT && *sT == ' ')
		{
			++count;
			++sT;
		}
		char* result = Erase(s, -1, count);
	}

	return result;
}

inline char* TrimWhitespaceBack(const char* s)
{
	char* result;

	size_t rLength = Length(s);
	if (s[rLength - 2] == ' ')
	{
		const char* tT = &s[rLength - 2];
		size_t endCount = 0;
		while (tT != s && *tT == ' ')
		{
			++endCount;
			--tT;
		}

		char* t = Erase(s, rLength - 2 - endCount, endCount);
	}

	return result;
}

inline char* TrimWhitespace(const char* s)
{
	char* result;

	// Find the first character that is a space
	// from that point
	//	start looping over the rest of the string until you find a non space character
	//  all the while counting the number of spaces you have encountered
	// once you encounter a non space character break out of the counting loop
	// then erase the characters in the string in the range determined in the loop starting at
	//	the first character.

	result = Copy(s);

	if (result[0] == ' ')
	{
		const char* sT = result;
		size_t count = 0;
		while (*sT && *sT == ' ')
		{
			++count;
			++sT;
		}
		char* t = Erase(result, -1, count);
		free(result);
		result = t;
	}


	size_t rLength = Length(result);
	if (result[rLength - 2] == ' ')
	{
		char* tT = &result[rLength - 2];
		size_t endCount = 0;
		while (tT != result && *tT == ' ')
		{
			++endCount;
			--tT;
		}

		char* t = Erase(result, rLength - 2 - endCount, endCount);
		free(result);
		result = t;
	}

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

/*
 * The user is responsible for freeing each string in the split result
 * and the components and componentlengths part
 */
struct SplitResult
{
	char** components;
	size_t* componentLengths;

	size_t numberOfComponents;

	SplitResult() : components(NULL), componentLengths(NULL), numberOfComponents(0) {};

	void Destroy()
	{
		for (size_t i = 0; i < this->numberOfComponents; ++i)
		{
			free(this->components[i]);
		}
		free(this->components);
		free(this->componentLengths);
		this->components = NULL;
		this->componentLengths = NULL;
		this->numberOfComponents = 0;
	}
};


inline SplitResult Split(const char* string, size_t stringLength, char splitCharacter)
{
	SplitResult result;

	char** stringComponents = (char**)malloc(sizeof(char*)* stringLength);
	size_t* stringComponentLengths = (size_t*)malloc(sizeof(size_t)* stringLength);

	const char* stringFront = string;
	size_t numberOfSplits = 0;
	for (U64 i = 0; i < stringLength; ++i)
	{
		if (string[i] == splitCharacter)
		{
			stringComponents[numberOfSplits] = CopyAndTerminate(stringFront, &(string[i]) - stringFront);
			stringComponentLengths[numberOfSplits] = Length(stringComponents[numberOfSplits]);
			stringFront = &(string[i + 1]);
			++numberOfSplits;
		}
	}

	stringComponents[numberOfSplits] = Copy(stringFront);
	stringComponentLengths[numberOfSplits] = Length(stringComponents[numberOfSplits]);
	++numberOfSplits;

	stringComponents = (char**)realloc(stringComponents, sizeof(char*) * numberOfSplits);
	stringComponentLengths = (size_t*)realloc(stringComponentLengths, sizeof(size_t) * numberOfSplits);

	result.numberOfComponents = numberOfSplits;
	result.componentLengths = stringComponentLengths;
	result.components = stringComponents;
	return result;
}

inline SplitResult Split(const char* string, char splitCharacter)
{
	size_t stringLength = Length(string);
	SplitResult result = Split(string, stringLength, splitCharacter);
	return result;
}

inline SplitResult Split(const char* string, size_t stringLength, const char* splitString, size_t splitStringLength)
{
	splitStringLength -= 1;
	SplitResult result;

	char** stringComponents = (char**)malloc(sizeof(char*)* stringLength);
	size_t* stringComponentLengths = (size_t*)malloc(sizeof(size_t)* stringLength);

	const char* stringFront = string;
	size_t numberOfSplits = 0;
	for (U64 i = 0; i + splitStringLength < stringLength; ++i)
	{
		if (Compare(&string[i], splitStringLength, splitString, splitStringLength))
		{
			stringComponents[numberOfSplits] = CopyAndTerminate(stringFront, &(string[i]) - stringFront);
			stringComponentLengths[numberOfSplits] = Length(stringComponents[numberOfSplits]);
			stringFront = &(string[i + splitStringLength]);
			++numberOfSplits;
		}
	}

	stringComponents[numberOfSplits] = Copy(stringFront);
	stringComponentLengths[numberOfSplits] = Length(stringComponents[numberOfSplits]);
	++numberOfSplits;

	stringComponents = (char**)realloc(stringComponents, sizeof(char*)* numberOfSplits);
	stringComponentLengths = (size_t*)realloc(stringComponentLengths, sizeof(size_t)* numberOfSplits);

	result.numberOfComponents = numberOfSplits;
	result.componentLengths = stringComponentLengths;
	result.components = stringComponents;
	return result;
}

inline SplitResult Split(const char* string, const char* splitString)
{
	size_t stringLength = Length(string);
	size_t splitStringLength = Length(splitString);
	SplitResult result = Split(string, stringLength, splitString, splitStringLength);
	return result;
}