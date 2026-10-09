// Supplies the shared random-number stream used for game and board choices and effects.
#include "game/frand.h"

#define FRAND_SEED_BIT_PATTERN 0xD826BC89
#define FRAND_DIVISOR 127773
#define FRAND_QUOTIENT_MULTIPLIER 2836
#define FRAND_REMAINDER_MULTIPLIER 16807
#define FRAND_VALUE_MASK 0x7FFFFFFF
#define FRAND_FLOAT_SCALE 2147483648

static u32 frand_seed;

// frand and frandmod store the next recurrence value in the shared stream. M621 round
// initialization calls frandom(OSGetTick()) and discards the result. A zero seed uses the low 32
// bits
// of OSGetTime() XORed with FRAND_SEED_BIT_PATTERN.
u32 frandom(u32 seed)
{
    s32 quotient, remainder;

    if (seed == 0) {
        seed = OSGetTime();
        seed ^= FRAND_SEED_BIT_PATTERN;
    }

    // Split the seed at the recurrence's divisor before applying its coefficients.
    quotient = seed / (u32)FRAND_DIVISOR;
    remainder = seed - (quotient * FRAND_DIVISOR);
    seed = quotient * FRAND_QUOTIENT_MULTIPLIER;
    seed = seed - remainder * FRAND_REMAINDER_MULTIPLIER;
    return seed;
}

// Game and board code calls this whenever it needs the next integer from the shared random stream.
u32 frand(void) {
    return frand_seed = frandom(frand_seed);
}

// Game and board effects use this for a fraction from 0.0f through 1.0f; float rounding can produce
// 1.0f. Each call advances the shared stream.
f32 frandf(void) {
    u32 randomValue = frand();
    float normalizedValue;
    randomValue &= FRAND_VALUE_MASK;
    normalizedValue = (float)randomValue/FRAND_FLOAT_SCALE;
    return normalizedValue;
}

// Game and board code calls this to choose a random index or count below a positive modulus; each
// call advances the shared stream.
s32 frandmod(s32 modulus) {
    u32 randomIndex;
    frand_seed = frandom(frand_seed);
    randomIndex = (frand_seed & FRAND_VALUE_MASK)%(u32)modulus;
    return randomIndex;
}
