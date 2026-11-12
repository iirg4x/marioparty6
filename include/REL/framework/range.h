// A value kept between two bounds: a value outside them is wrapped back in by the period
// (maximum - minimum), the way an angle is.
//
// The class and member names are descriptive.
#ifndef REL_FRAMEWORK_RANGE_H
#define REL_FRAMEWORK_RANGE_H

template <class Value> class Range {
public:
    Value minimum, maximum, period, current;
    // The shorter signed way from `from` to `to` around the range.
    Value distance(Value from, Value to);
    // Sets the current value, wrapped into the range.
    void set(Value value);
    Value distanceTo(Value value);
    // Moves the current value `rate` of the shorter way towards `target`; returns the new value.
    Value step(Value target, double rate);
    // The value wrapped into [minimum, maximum).
    void add(Value change);
    Value wrap(Value value);
    Value get();
    Range(Value from, Value to, Value initial);
};

template<class Value> void Range<Value>::set(Value value)
{
    current = wrap(value);
}

template<class Value> void Range<Value>::add(Value change)
{
    current = wrap(current + change);
}

template<class Value> Value Range<Value>::distanceTo(Value value)
{
    return distance(current, value);
}

template<class Value> Value Range<Value>::step(Value target, double rate)
{
    return current = wrap(current + rate * distanceTo(target));
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
    Value difference;
    Value magnitude;
    Value thresholdDistance;

    difference = wrap(second) - wrap(first);
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
