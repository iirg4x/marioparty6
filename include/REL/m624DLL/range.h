#ifndef REL_M624DLL_RANGE_H
#define REL_M624DLL_RANGE_H

#include <cmath>

template <class Value> class Range {
public:
    Value minimum, maximum, period, current;
    Value distance(Value, Value);
    Value fn_1_9AE0(Value, double);
    Value wrap(Value);
    Range(Value, Value, Value);
    Value distanceTo(Value);
};

template <class Value> Value Range<Value>::distance(Value startAngle, Value endAngle)
{
    Value start = wrap(startAngle);
    Value difference = wrap(endAngle) - start;
    Value wrapDifference;
    Value absoluteDifference;

    if (difference < 0.0f) {
        absoluteDifference = -difference;
    } else {
        absoluteDifference = difference;
    }
    wrapDifference = absoluteDifference;
    return wrapDifference < period / 2.0f
               ? difference
               : (period - wrapDifference) *
                     (Value)(difference < 0.0f ? 1 : -1);
}

template <class Value> Value Range<Value>::fn_1_9AE0(Value value, double rate) {
    return current = wrap(current + rate * distanceTo(value));
}

template <class Value> Value Range<Value>::wrap(Value value) {
    if (value < this->minimum) {
        return this->maximum - (Value) std::fmod(
            (double) (this->minimum - value), (double) this->period);
    }
    if (value >= this->maximum) {
        return this->minimum + (Value) std::fmod(
            (double) (value - this->maximum), (double) this->period);
    }
    return value;
}

static inline float rangeMinimum(float a, float b) { return a < b ? a : b; }

static inline float rangeMaximum(float a, float b) { return a > b ? a : b; }

template <class Value> Range<Value>::Range(Value a, Value b, Value initial) {
    minimum = rangeMinimum(a, b);
    maximum = rangeMaximum(a, b);
    period = maximum - minimum;
    current = wrap(initial);
}

template <class Value> Value Range<Value>::distanceTo(Value value) {
    Value difference;
    Value currentValue = current;
    Value absoluteDifference;
    Value thresholdDistance;

    difference = wrap(value) - wrap(currentValue);
    if (difference < 0.0f) {
        thresholdDistance = -difference;
    } else {
        thresholdDistance = difference;
    }
    absoluteDifference = thresholdDistance;
    return absoluteDifference < period / 2.0f
        ? difference
        : (period - absoluteDifference) * (Value)(difference < 0.0f ? 1 : -1);
}

#endif
