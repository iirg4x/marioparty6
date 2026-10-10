// Shared random number generator used by the minigame framework.
static const char rcsid[] = "$Id: random.cpp,v 1.8.2.1 2004/06/04 03:56:51 shohyama Exp $";

#include "REL/framework/mt19937_types.h"

// Framework random draws use this shared stream; RandomInteger(0) consumes no draw.
static MersenneTwister randomGenerator;

// Restarts the shared stream when minigame code supplies a seed.
void RandomSeed(u32 seed)
{
    randomGenerator.seed(seed);
}

// Supplies minigame random decisions with a nonnegative 31-bit integer.
u32 RandomUnsigned()
{
    return randomGenerator.nextUnsigned31();
}

// Supplies minigame choices with the next 31-bit draw modulo the requested limit.
s32 RandomInteger(s32 limit)
{
    s32 remainder;
    if (limit == 0) {
        // A zero limit returns zero without advancing the shared stream.
        remainder = 0;
    } else {
        // The draw is nonnegative, so a negative limit also gives a nonnegative remainder.
        remainder = (s32)randomGenerator.nextUnsigned31() % limit;
    }
    return remainder;
}

// Supplies a 23-bit fraction in [0, 1) when minigame code requests an unscaled float.
float RandomFloat()
{
    return randomGenerator.nextFloat23();
}

// Scales a two-draw double-precision fraction for a minigame's requested maximum.
float RandomFloat(float maximum)
{
    // Rounding the final float can include the maximum endpoint.
    return (float)((double)maximum * randomGenerator.nextDouble());
}

// Scales one draw between the endpoints requested by minigame code.
float RandomFloat(float minimum, float maximum)
{
    // The full-word float conversion can round to 1, including the maximum endpoint.
    return minimum + ((maximum - minimum) * randomGenerator.nextFloat());
}
