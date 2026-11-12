#ifndef RANGE_H
#define RANGE_H

/* Floating-point range and wrap operations. */
// m624's own class; its functions are ordinary functions of player_runner.cpp.
class Range {
public:
    float minimum, maximum, period, current;
    float fn_1_1370(float);
    float fn_1_9AE0(float, double);
    float fn_1_9D3C(float);
    float fn_1_999C(float, float);
    Range(float, float, float);
};

#endif
