#ifndef REL_M681DLL_TRANSITION_H
#define REL_M681DLL_TRANSITION_H
template<class Value>
struct TransitionState {
    int frames;
    int elapsed;
    Value current;
    Value start;
    Value end;
    TransitionState(Value end, int frames) { set(end, frames); }
    Value advance();
    void set(Value end, int frames);
    void reset(Value start, Value end, int frames);
};
typedef TransitionState<float> Transition;

template<class Value>
Value TransitionState<Value>::advance()
{
    Value value = current;
    if (elapsed <= frames) {
        double elapsed = (double)++this->elapsed;
        double fraction = elapsed / (double)(frames + 1);
        value = (Value)(start * (1.0 - fraction) + end * fraction);
        current = value;
    }
    return value;
}

template<class Value>
void TransitionState<Value>::set(Value end, int frames)
{
    reset(current, end, frames);
}

template<class Value>
void TransitionState<Value>::reset(Value start, Value end, int frames)
{
    this->frames = frames;
    elapsed = 0;
    if (frames == 0) {
        current = end;
        this->start = end;
        this->end = end;
    } else {
        current = start;
        this->start = start;
        this->end = end;
    }
}
#endif
