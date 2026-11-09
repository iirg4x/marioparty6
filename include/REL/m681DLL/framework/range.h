#ifndef RANGE_H
#define RANGE_H

struct Range {
    float minimum, maximum, period, current;
    float fn_1_1370(float);
    Range(float, float, float);
};

#endif
