// Scales the effect inputs that are stored at a hundred times their supplied value.

#ifndef REL_SEER_TERROR_SCALE_HUNDRED_H
#define REL_SEER_TERROR_SCALE_HUNDRED_H

// Returns the value times one hundred.
template <class T>
static inline T scaleHundred(T value)
{
    return 100 * value;
}

#endif
