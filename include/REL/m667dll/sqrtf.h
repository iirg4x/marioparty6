/* Return an approximate square root for positive inputs; return nonpositive inputs unchanged. */
extern inline f32 fn_1_8E8C(f32 x)
{
    const double _half = 0.5;
    const double _three = 3.0;
    volatile f32 y;
    if (x > 0.0f) {
        f64 guess = __frsqrte((f64)x);
        guess = _half * guess * (_three - guess * guess * x);
        guess = _half * guess * (_three - guess * guess * x);
        guess = _half * guess * (_three - guess * guess * x);
        y = (f32)(x * guess);
        return y;
    }
    return x;
}
