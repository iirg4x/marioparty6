#ifndef REL_FRAMEWORK_PARAMETER_INTERPOLATION_H
#define REL_FRAMEWORK_PARAMETER_INTERPOLATION_H

// Moves a value from one number to another over a count of frames.
class ParameterInterpolation {
public:
    int duration;
    int elapsed;
    float value;
    float from;
    float to;

    // Initializes the current value and interpolation duration.
    ParameterInterpolation(float initial = 0.0f, int frames = 0) { set(initial, frames); }
    // Each minigame that uses the class defines update() and set() in one of its own files.
    void set(float end, int frames);
    void set(float start, float end, int frames);
    float update();
    // Returns the current interpolated value.
    float getValue() const { return value; }
};

#endif
