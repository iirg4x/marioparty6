#ifndef M654_PARAMETER_INTERPOLATION_H
#define M654_PARAMETER_INTERPOLATION_H

class ParameterInterpolation {
public:
    int duration;
    int elapsed;
    float value;
    float from;
    float to;

    // Initializes the current value and interpolation duration.
    ParameterInterpolation(float initial = 0.0f, int frames = 0) { set(initial, frames); }
    void set(float end, int frames);
    void set(float start, float end, int frames);
    float update();
    // Returns the current interpolated value.
    float getValue() const { return value; }
};

extern ParameterInterpolation lbl_1_bss_A8;

#endif
