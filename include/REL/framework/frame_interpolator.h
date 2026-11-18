// A value that moves in a straight line from a start value to a target value over a number of
// frames.
#ifndef REL_FRAMEWORK_FRAME_INTERPOLATOR_H
#define REL_FRAMEWORK_FRAME_INTERPOLATOR_H

template <class Value> class FrameInterpolator {
public:
    // Moves to target over frames, starting from value, which nothing has set yet; with no frames
    // (the default), the value starts at target.
    FrameInterpolator(Value target, long frames = 0) { set(target, frames); }
    // Advances one frame and returns the new value; after the last frame the value stays.
    Value update();
    // Starts a new movement from the current value.
    void set(Value target, long frames);
    // With no frames the value jumps to the target.
    void set(Value start, Value target, long frames);
    // Tests whether interpolation has advanced beyond its duration.
    bool isFinished() const { return elapsed > duration; }
    // Returns the current interpolated value.
    operator Value() const { return value; }

    long duration;
    long elapsed;
    Value value;
    Value startValue;
    Value targetValue;
};

// Advances interpolation by one frame and returns the resulting value.
template <class Value> Value FrameInterpolator<Value>::update()
{
    Value result = value;

    if (!isFinished()) {
        double frames = (double)++elapsed;
        double fraction = frames / (double)(duration + 1);
        result = startValue * (1.0 - fraction) + targetValue * fraction;
        value = result;
    }
    return result;
}

// Starts interpolation from the current value to the target.
template <class Value> void FrameInterpolator<Value>::set(Value target, long frames)
{
    set(value, target, frames);
}

// Sets both endpoints and the duration, selecting the target immediately for zero frames.
template <class Value> void FrameInterpolator<Value>::set(Value start, Value target, long frames)
{
    duration = frames;
    elapsed = 0;
    if (frames == 0) {
        value = target;
        startValue = target;
        targetValue = target;
    } else {
        value = start;
        startValue = start;
        targetValue = target;
    }
}

#endif
