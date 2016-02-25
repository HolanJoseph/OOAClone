#pragma once
#include "Types.h"

#include <random>

struct RandomNumberGenerator
{
	std::mt19937 randomNumberGenerator;
	std::uniform_real_distribution<F32> getF32from0To1Exclusive;
};

void SeedRandomNumberGenerator(RandomNumberGenerator *generator, U32 seed)
{
	generator->randomNumberGenerator.seed(seed);
}

// [lowValue, highValue)
inline F32 RandomF32Between(RandomNumberGenerator *generator, F32 lowValue = 0.0f, F32 highValue = 1.0f)
{
	F32 result = generator->getF32from0To1Exclusive(generator->randomNumberGenerator);
	
	// NOTE: Adjust for provided range
	result = (result * (highValue - lowValue)) + lowValue;

	return result;
}

