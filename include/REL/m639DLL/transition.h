#ifndef M639_TRANSITION_H
#define M639_TRANSITION_H
// A value that eases to a target over a number of frames.
template <class T> class Transition {
public:
    int frames, elapsed;
    T current, start, end;
    Transition(T value = 0.0f) { fn_1_50AC(value, 0); }
    void fn_1_50AC(T end, int frames);
    void fn_1_50F0(T start, T end, int frames);
    T fn_1_5124();
};

template <class T> void Transition<T>::fn_1_50AC(T end, int frames)
{
    fn_1_50F0(current, end, frames);
}

template <class T> void Transition<T>::fn_1_50F0(T start, T end, int frames)
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

template <class T> T Transition<T>::fn_1_5124()
{
    T value = current;
    if (elapsed <= frames) {
        double nextFrame = (double)++elapsed;
        double fraction = nextFrame / (double)(frames + 1);
        value = (T)(start * (1.0 - fraction) + end * fraction);
        current = value;
    }
    return value;
}

#endif
