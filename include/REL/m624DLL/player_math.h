#ifndef M624_PLAYER_MATH_H
#define M624_PLAYER_MATH_H

template <class T> inline T Square(const T &value)
{
    return value * value;
}

template <class T> inline T Clamp(T value, T minimum, T maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

#endif
