// Declares the random stream used by the minigame framework.
#ifndef REL_FRAMEWORK_MT19937_TYPES_H
#define REL_FRAMEWORK_MT19937_TYPES_H

#include "dolphin/types.h"

#define MT19937_DEFAULT_SEED 5489

// Holds the Mersenne Twister state shared by the framework's random helpers.
struct MersenneTwister {
    u32 words[624]; // State words regenerated together when the stream runs out.
    u32 index; // Next state word to read; 624 requests regeneration on the next draw.

    // Starts a newly constructed stream with the default seed.
    MersenneTwister() {
        // Seeds the state words with MT19937_DEFAULT_SEED without changing index.
        seed(MT19937_DEFAULT_SEED);
    }
    void seed(u32 seedValue);
    void seedFromArray(u32 *seedValues, u32 seedCount);
    u32 nextUnsigned32();
    u32 nextUnsigned31();
    float nextFloat();
    float nextFloatAlternate();
    float nextFloatCentered();
    double nextDouble();
    float nextFloat23();

    inline u32 nextWord();
};

#endif
