#ifndef RANGE_H
#define RANGE_H

/* A bounded value with a period used to wrap updates. */
template<class T> class RangeT {
public:
    T minimum, maximum, period, current;
    inline T fn_1_1370(T);
    T fn_1_7CE0(T first, T second) const;
    RangeT(T, T, T);
    void SetCurrent(T);
    T GetCurrent();
};

typedef RangeT<float> Range;
#endif
