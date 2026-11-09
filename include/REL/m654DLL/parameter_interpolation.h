#ifndef M654_PARAMETER_INTERPOLATION_H
#define M654_PARAMETER_INTERPOLATION_H

class ParameterInterpolation {
public:
    int duration;
    int elapsed;
    float value;
    float from;
    float to;

    ParameterInterpolation(float initial = 0.0f, int frames = 0) { fn_1_75F8(initial, frames); }
    void fn_1_75F8(float end, int frames);
    void fn_1_763C(float start, float end, int frames);
    float fn_1_7670();
    float getValue() const { return value; }
};

extern "C" ParameterInterpolation lbl_1_bss_A8;

#endif
