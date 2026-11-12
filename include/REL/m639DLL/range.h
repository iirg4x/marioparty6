// A float value kept inside [minimum, maximum), wrapping round like an angle, and the signed
// shortest step between two such values. The class and member names are descriptive; the members
// have the names of include/REL/framework/range.h. This module's text of the class is its own:
// toudai.o does not match with the framework's.

#ifndef REL_M639DLL_RANGE_H
#define REL_M639DLL_RANGE_H

#include <cmath>

template <class Value> class Range {
public:
    Value minimum, maximum, period, current;
    double distanceTo(Value value);
    Value distance(Value startAngle, Value endAngle);
    Value wrap(Value value);
    Range(Value first, Value second, Value initial);
};

template <class Value> inline Value rangeMinimum(Value first, Value second)
{
    return first < second ? first : second;
}

template <class Value> inline Value rangeMaximum(Value first, Value second)
{
    return first > second ? first : second;
}

template <class Value> inline Value rangeMagnitude(Value value)
{
    if (value < 0.0f) {
        return -value;
    }
    return value;
}

// The step from the current value to the given one.
template <class Value> double Range<Value>::distanceTo(Value value)
{
    return distance(current, value);
}

// Stores the ordered bounds and width, then wraps the initial value into them.
template <class Value> Range<Value>::Range(Value first, Value second, Value initial)
{
    minimum = rangeMinimum(first, second);
    maximum = rangeMaximum(first, second);
    period = maximum - minimum;
    current = wrap(initial);
}

// The signed shortest step from the first value to the second, going round the range when that is shorter.
template <class Value> Value Range<Value>::distance(Value startAngle, Value endAngle)
{
    Value start = wrap(startAngle);
    Value difference = wrap(endAngle) - start;
    Value magnitude = rangeMagnitude(difference);

    return magnitude < period / 2.0f ? difference : (period - magnitude) * (Value)(difference < 0.0f ? 1 : -1);
}

// Wraps a value into [minimum, maximum).
template <class Value> Value Range<Value>::wrap(Value value)
{
    if (value < minimum) {
        return maximum - (Value)std::fmod((double)(minimum - value), (double)period);
    }
    if (value >= maximum) {
        return minimum + (Value)std::fmod((double)(value - maximum), (double)period);
    }
    return value;
}

#endif
