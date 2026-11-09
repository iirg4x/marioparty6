#ifndef REL_M654DLL_FRAMEWORK_RANGE_IMPL_H
#define REL_M654DLL_FRAMEWORK_RANGE_IMPL_H

#include "REL/m654DLL/framework/range.h"

template<class Value> void Range<Value>::fn_1_4D94(Value value)
{
    current = fn_1_1370(value);
}

template<class Value> Value Range<Value>::fn_1_4DD0(Value value)
{
    return fn_1_5C30(current, value);
}

template<class Value> Value Range<Value>::fn_1_4E0C(Value step, double rate)
{
    Value progress = fn_1_5C30(current, step);
    return current = fn_1_1370(current + rate * progress);
}

template<class Value> Value Range<Value>::fn_1_1370(Value value)
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
    current = fn_1_1370(initial);
}

template<class Value> Value Range<Value>::fn_1_5C30(Value first, Value second)
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

#endif
