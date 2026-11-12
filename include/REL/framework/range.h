// A value kept between two bounds: a value outside them is wrapped back in by the period
// (maximum - minimum), the way an angle is.
//
// The class and member names are descriptive.
#ifndef REL_FRAMEWORK_RANGE_H
#define REL_FRAMEWORK_RANGE_H

#include "REL/framework/range_declaration.h"

template<class Value> void Range<Value>::set(Value value)
{
    current = wrap(value);
}

template<class Value> Value Range<Value>::distanceTo(Value value)
{
    return distance(current, value);
}

template<class Value> Value Range<Value>::step(Value target, double rate)
{
    Value progress = distance(current, target);
    return current = wrap(current + rate * progress);
}

template<class Value> Value Range<Value>::wrap(Value value)
{
    if (value < minimum) {
        return maximum - (Value)fmod((double)(minimum - value), (double)period);
    }
    if (value >= maximum) {
        return minimum + (Value)fmod((double)(value - maximum), (double)period);
    }
    return value;
}

template<class Value> inline Value rangeMinimum(Value a, Value b) { return a < b ? a : b; }

template<class Value> inline Value rangeMaximum(Value a, Value b) { return a > b ? a : b; }

template<class Value> Range<Value>::Range(Value a, Value b, Value initial)
{
    minimum = rangeMinimum(a, b);
    maximum = rangeMaximum(a, b);
    period = maximum - minimum;
    current = wrap(initial);
}

template<class Value> Value Range<Value>::distance(Value first, Value second)
{
    Value wrappedSecond;
    Value wrappedFirst;
    Value difference;
    Value magnitude;
    Value thresholdDistance;

    if (second < minimum) {
        wrappedSecond = maximum - (Value)fmod((double)(minimum - second), (double)period);
    } else if (second >= maximum) {
        wrappedSecond = minimum + (Value)fmod((double)(second - maximum), (double)period);
    } else {
        wrappedSecond = second;
    }

    if (first < minimum) {
        wrappedFirst = maximum - (Value)fmod((double)(minimum - first), (double)period);
    } else if (first >= maximum) {
        wrappedFirst = minimum + (Value)fmod((double)(first - maximum), (double)period);
    } else {
        wrappedFirst = first;
    }

    difference = wrappedSecond - wrappedFirst;
    if (difference < 0.0f) {
        thresholdDistance = -difference;
    } else {
        thresholdDistance = difference;
    }
    magnitude = thresholdDistance;
    return magnitude < period / 2.0f
        ? difference
        : (period - magnitude) * (Value)(difference < 0.0f ? 1 : -1);
}

template<class Value> Value Range<Value>::get() { return current; }

#endif
