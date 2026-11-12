#ifndef REL_FRAMEWORK_RANGE_DECLARATION_H
#define REL_FRAMEWORK_RANGE_DECLARATION_H
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
    Value wrap(Value value);
    Value get();
    Range(Value from, Value to, Value initial);
};

#endif
