// Provides the framework's linear interpolation helper.
#ifndef REL_FRAMEWORK_INTERPOLATION_H
#define REL_FRAMEWORK_INTERPOLATION_H

// Interpolates between two values with a double-precision amount.
template <class T> static inline T interpolateValue(T first, T last, double amount)
{
    return (1.0 - amount) * first + amount * last;
}

#endif
