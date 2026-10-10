#ifndef REL_FRAMEWORK_MT19937_H
#define REL_FRAMEWORK_MT19937_H

#include "dolphin/types.h"

#define MT19937_WORD_HIGH_BIT_MASK 0x80000000UL
#define MT19937_TWIST_MATRIX_BIT_PATTERN 0x9908B0DFUL
#define MT19937_SEED_UPDATE_MULTIPLIER 1812433253UL
#define MT19937_ARRAY_INITIAL_SEED 19650218UL
#define MT19937_ARRAY_SEED_MULTIPLIER 1664525UL
#define MT19937_ARRAY_STATE_MULTIPLIER 1566083941UL
#define MT19937_FLOAT_WORD_SCALE 4294967296.0f
#define MT19937_DOUBLE_SIGNIFICAND_SCALE 67108864.0
#define MT19937_DOUBLE_UNIT_SCALE 9007199254740992.0
#define MT19937_FLOAT_23_BIT_SCALE 8388608.0f

static const u32 twistMatrix[2] = { 0, MT19937_TWIST_MATRIX_BIT_PATTERN };

#include "REL/framework/mt19937_types.h"

// Supplies a draw method with the next state word, twisting the state when exhausted.
inline u32 MersenneTwister::nextWord()
{
    u32 combinedBits;
    u32 wordIndex;
    if (index >= 624) {
        for (wordIndex = 0; wordIndex < 227; wordIndex++) {
            combinedBits = (words[wordIndex] & MT19937_WORD_HIGH_BIT_MASK) |
                           (words[wordIndex + 1] & 0x7FFFFFFFUL);
            words[wordIndex] =
                twistMatrix[combinedBits & 1] ^ ((combinedBits >> 1) ^ words[wordIndex + 397]);
        }
        for (; wordIndex < 623; wordIndex++) {
            combinedBits = (words[wordIndex] & MT19937_WORD_HIGH_BIT_MASK) |
                           (words[wordIndex + 1] & 0x7FFFFFFFUL);
            words[wordIndex] =
                twistMatrix[combinedBits & 1] ^ ((combinedBits >> 1) ^ words[wordIndex - 227]);
        }
        combinedBits = (words[623] & MT19937_WORD_HIGH_BIT_MASK) | (words[0] & 0x7FFFFFFFUL);
        words[623] = (combinedBits >> 1) ^ words[396] ^ twistMatrix[combinedBits & 1];
        index = 0;
    }
    combinedBits = words[index++];
    combinedBits ^= combinedBits >> 11;
    combinedBits ^= (combinedBits << 7) & 0x9D2C5680UL;
    combinedBits ^= (combinedBits << 15) & 0xEFC60000UL;
    combinedBits ^= combinedBits >> 18;
    return combinedBits;
}

#endif
