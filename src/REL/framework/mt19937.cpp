// Generates the pseudo-random values used by minigame logic.
static const char rcsid[] = "$Id: mt19937.cpp,v 1.10 2004/02/17 09:22:00 hanamasu Exp $";

#include "REL/framework/mt19937.h"

// RandomSeed calls this to rewrite the 624 state words from seedValue without changing index.
void MersenneTwister::seed(u32 seedValue)
{
    words[0] = seedValue;
    for (index = 1; index < 624; index++) {
        words[index] = index + MT19937_SEED_UPDATE_MULTIPLIER *
            (words[index - 1] ^ (words[index - 1] >> 30));
    }
}

// Mixes the caller's seed words into state, leaves index at 624, then sets words[0] to 0x80000000.
void MersenneTwister::seedFromArray(u32 *seedValues, u32 seedCount)
{
    u32 stateIndex;
    u32 valueIndex;
    u32 remainingCount;

    words[0] = MT19937_ARRAY_INITIAL_SEED;
    index = 1;
    while (index < 624) {
        words[index] = index + MT19937_SEED_UPDATE_MULTIPLIER *
            (words[index - 1] ^ (words[index - 1] >> 30));
        index++;
    }
    stateIndex = 1;
    valueIndex = 0;
    remainingCount = 624 > seedCount ? 624 : seedCount;
    while (remainingCount != 0) {
        words[stateIndex] =
            (words[stateIndex] ^ ((words[stateIndex - 1] ^ (words[stateIndex - 1] >> 30)) *
                                  MT19937_ARRAY_SEED_MULTIPLIER)) +
            seedValues[valueIndex] + valueIndex;
        stateIndex++;
        valueIndex++;
        if (stateIndex >= 624) {
            words[0] = words[623];
            stateIndex = 1;
        }
        if (valueIndex >= seedCount) {
            valueIndex = 0;
        }
        remainingCount--;
    }
    remainingCount = 623;
    while (remainingCount != 0) {
        words[stateIndex] =
            (words[stateIndex] ^ ((words[stateIndex - 1] ^ (words[stateIndex - 1] >> 30)) *
                                  MT19937_ARRAY_STATE_MULTIPLIER)) -
            stateIndex;
        stateIndex++;
        if (stateIndex >= 624) {
            words[0] = words[623];
            stateIndex = 1;
        }
        remainingCount--;
    }
    words[0] = MT19937_WORD_HIGH_BIT_MASK;
}

// Returns one full 32-bit word from the generator.
u32 MersenneTwister::nextUnsigned32()
{
    return nextWord();
}

// Returns the next word shifted right one bit for RandomUnsigned and RandomInteger.
u32 MersenneTwister::nextUnsigned31()
{
    return nextWord() >> 1;
}

// Scales the next full random word by 2^-32.
float MersenneTwister::nextFloat()
{
    return (1.0f / MT19937_FLOAT_WORD_SCALE) * nextWord();
}

// Applies the same full-word float conversion as nextFloat().
float MersenneTwister::nextFloatAlternate()
{
    return (1.0f / MT19937_FLOAT_WORD_SCALE) * nextWord();
}

// Adds 0.5 to the next word in float precision, then scales by 2^-32; the offset may round away.
float MersenneTwister::nextFloatCentered()
{
    return (1.0f / MT19937_FLOAT_WORD_SCALE) * (0.5f + nextWord());
}

// Combines two draws into a double-precision fraction for RandomFloat(maximum).
double MersenneTwister::nextDouble()
{
    u32 upperBits = nextWord() >> 5;
    u32 lowerBits = nextWord() >> 6;
    return (1.0 / MT19937_DOUBLE_UNIT_SCALE) *
        (MT19937_DOUBLE_SIGNIFICAND_SCALE * (double)upperBits + (double)lowerBits);
}

// Keeps 23 random bits for the framework's single-precision RandomFloat().
float MersenneTwister::nextFloat23()
{
    u32 randomBits = nextWord() >> 9;
    return (1.0f / MT19937_FLOAT_23_BIT_SCALE) * (float)randomBits;
}
