#ifndef RANGE_H
#define RANGE_H

/* Floating-point bounds, period, and current value for wrapping ranges. */
class Range {
public:
    float minimum, maximum, period, current;
    float fn_1_1370(float);
    void fn_1_12F88(float);
    float fn_1_12FC4(float);
    void fn_1_13000(float, double);
    float fn_1_13110();
    float fn_1_1327C(float, float);
    Range(float, float, float);
};

#endif
