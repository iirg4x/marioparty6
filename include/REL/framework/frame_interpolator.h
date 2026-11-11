// A value that moves in a straight line from a start value to a target value over a number of
// frames.
#ifndef REL_FRAMEWORK_FRAME_INTERPOLATOR_H
#define REL_FRAMEWORK_FRAME_INTERPOLATOR_H

template <class Value> class FrameInterpolator {
public:
    FrameInterpolator(Value target, long frames = 0) { set(target, frames); }
    // Advances one frame and returns the new value; after the last frame the value stays.
    Value update();
    // Starts a new movement from the current value.
    void set(Value target, long frames);
    // With no frames the value jumps to the target.
    void set(Value start, Value target, long frames);
    bool isFinished() const { return elapsed > duration; }
    operator Value() const { return value; }

    long duration;
    long elapsed;
    Value value;
    Value startValue;
    Value targetValue;
};

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

template <class Value> void FrameInterpolator<Value>::set(Value target, long frames)
{
    set(value, target, frames);
}

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
